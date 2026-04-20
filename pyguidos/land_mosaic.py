import sys
import time
from pathlib import Path
import csv
from collections import Counter

import numpy as np
import rasterio
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.collections import PatchCollection
import ternary
from ternary.helpers import project_point

from . import spat
from . import utils
from . import checks
from . import TEMPL_DIR


def landmos(in_tiff,
            window_size,
            outdir=None,
            statists=True,
            stat_files=True,
            out_colors='bgr',
            verb=False):
    """
    Performs Landscape Mosaic analysis on a three-class raster using a
    moving window approach via Spatcon. Each pixel is classified based on
    the proportional composition of the three land cover classes within
    the window, resulting in up to 103 compositional classes subsequently
    remapped to 19 aggregated classes.

    Parameters
    ----------
    in_tiff : str or Path
        Path to the input GeoTIFF. Must be uint8 with values:
        0 = NoData, 1 = Class 1 (e.g. Agriculture),
        2 = Class 2 (e.g. Natural), 3 = Class 3 (e.g. Developed).
    window_size : int
        Size of the moving window in pixels. Must be an odd integer >= 3.
    outdir : str or Path, optional
        Directory for output files. Defaults to the input file's directory.
    statists : bool, optional
        If True (default), computes and returns statistics.
    stat_files : bool, optional
        If True (default), writes statistics to .txt, .csv and .png files.
    out_colors : str, optional
        Color scheme for the 103-class output colormap: 'agr', 'ant', 'bgr',
        'dev', 'div', 'nat'. Default 'bgr'.
    return_array : bool, optional
        If True, includes the 103-class output numpy array in the result.
        Default False.
    verb : bool, optional
        If True, prints progress messages. Default False.

    Returns
    -------
    dict
        Nested dictionary with three keys:
        - 'output paths' (dict or None): paths to generated output files
          ('path tif', 'path txt', 'path csv', 'path csv hm', 'path png'),
          or None if outfile=False.
        - 'input stats' (dict): pixel counts for the three input classes,
          foreground and missing pixels.
        - 'output stats' (dict): pixel counts for both the 103-class and
          19-class aggregated outputs.

    Output Files
    ------------
    - <in_name>_lm_<window_size>_103class.tif : 103-class landscape mosaic result
    - <in_name>_lm_<window_size>.tif          : 19-class remapped result
    - <in_name>_lm_<window_size>.txt          : statistics report
    - <in_name>_lm_<window_size>.csv          : per-value pixel counts
    - <in_name>_lm_<window_size>_heatmap.csv  : ternary diagram data table
    - <in_name>_lm_<window_size>_heatmap.png  : ternary diagram heatmap
    """
    start_time = time.time()

    # Validate parametres
    checks.validate_wsize(window_size)

    # Initialize Paths and Metadata
    in_tiff = Path(in_tiff)
    outdir = Path(outdir) if outdir else in_tiff.parent
    in_name = in_tiff.stem
    out_name = f"{in_name}_lm_{window_size}"
    info = utils.get_raster_info(in_tiff)

    # Read the input Geotiff
    with rasterio.open(in_tiff) as src:
        input_data = src.read(1).astype(np.int16)

    # Get the pixel counting
    input_pxl_freq = utils.get_pxl_freq(input_data)

    # Input Geotiff validations
    checks.validate_lm_input(list(input_pxl_freq.keys()), info["bands"], info['dtype'])

    try:
        # Compute Landscape Mosaic
        data_out = spat.compute_LM(input_data, window_size)

        # Save 103 classes Geotiff
        weblink = "https://forest.jrc.ec.europa.eu/en/activities/lpa/"
        tag_descr = f"GTB_LM, <{window_size},{out_colors}>, {weblink}"
        cmap_path = TEMPL_DIR / f"lm_{out_colors}_colormap.txt"
        out_tiff103c = outdir / f"{out_name}_103class_{out_colors}.tif"
        utils.save_output_geotiff(out_tiff103c, data_out, info['profile'], cmap_path, tag_descr)

        # Reclassiafy Geotiff 103 -> 19 classes
        reclass = {}
        with open(TEMPL_DIR / 'lm_103to19.txt' , 'r') as f:
            for line in f:
                l = [int(x) for x in line.split(' ')]
                reclass[l[0]]=l[1]

        mapping_array = np.zeros(256, dtype=np.uint8)
        for old_val, new_val in reclass.items():
            mapping_array[old_val] = new_val
        data_19cl = mapping_array[data_out]

        # Save 19 classes Geotiff
        cmap_path19 = TEMPL_DIR / "lm_19c_colormap.txt"
        out_tiff19c = outdir / f"{out_name}.tif"
        utils.save_output_geotiff(out_tiff19c, data_19cl, info['profile'], cmap_path19, tag_descr)

        # Statistics and Reporting
        stats_dict = None
        if statists:
            lm_pixel_freq = utils.get_pxl_freq(data_out)
            stats_dict = landmos_stats(lm_tiff=out_tiff103c,
                                       outfile = stat_files,
                                       outdir=outdir,
                                       source_tiff=in_tiff,
                                       lm_freq=lm_pixel_freq,
                                       source_freq=input_pxl_freq)

        # Add 19-class path — known here but not inside landmos_stats()
        if stats_dict["output paths"]["path tif 19cl"] is None:
            stats_dict["output paths"]["path tif 19cl"] = str(out_tiff19c)
        
        # Computational time
        time_str = utils.running_time(start_time, time.time())
        if verb:
            print(f"\nLand Mosaic completed in {time_str}")
        if statists:
            txt_file = outdir / f'{out_name}.txt'
            utils.update_time_line(txt_file, time_str)

        return stats_dict

    except Exception as e:
        print(f"Error during run: {e}")
        raise # Still show the error



def landmos_stats(lm_tiff, outfile = True, outdir = None, source_tiff=None, lm_freq=None, source_freq=None):
    """
    Computes statistics for an existing Landscape Mosaic result GeoTIFF.
    Can be called independently on a previously generated landscape mosaic
    output, or is invoked automatically by landmos() when statists=True.
    Generates a ternary diagram heatmap visualising the distribution of
    compositional classes across the three land cover types.

    Parameters
    ----------
    lm_tiff : str or Path
        Path to the 103-class Landscape Mosaic result GeoTIFF. Must contain
        a valid GTB_LM metadata tag in the TIFFTAG_IMAGEDESCRIPTION field.
    outfile : bool, optional
        If True (default), writes statistics to .txt, .csv and .png files.
    outdir : str or Path, optional
        Directory for output files. Defaults to the input file's directory.
    source_tiff : str or Path, optional
        Path to the original three-class input GeoTIFF used to generate
        the landscape mosaic result. Used to report per-class pixel counts
        in the statistics report. Default None.
    lm_freq : Counter, optional
        Pre-computed pixel frequency Counter for the 103-class landscape
        mosaic result array. If provided, skips reading the GeoTIFF for
        pixel counting. Passed internally by landmos() to avoid redundant
        disk reads. Default None.
    source_freq : Counter, optional
        Pre-computed pixel frequency Counter for the original input GeoTIFF.
        If provided, skips reading the source GeoTIFF for pixel counting.
        Passed internally by landmos() to avoid redundant disk reads.
        Default None.

    Returns
    -------
    dict
        Nested dictionary with three keys:
        - 'output paths' (dict or None): paths to generated output files
          ('path tif', 'path txt', 'path csv', 'path csv hm', 'path png'),
          or None if outfile=False.
        - 'input stats' (dict): pixel counts for the three input classes,
          foreground and missing pixels.
        - 'output stats' (dict): pixel counts for both the 103-class and
          19-class aggregated outputs.

        Note: 'output paths' is None when outfile=False. All other keys
        are always populated regardless of outfile.

    Output Files
    ------------
    - <lm_tiff_stem>.txt         : statistics report
    - <lm_tiff_stem>.csv         : per-value pixel counts and frequencies
    - <lm_tiff_stem>_heatmap.csv : ternary diagram data table
    - <lm_tiff_stem>_heatmap.png : ternary diagram heatmap
    """
    start_time_stat = time.time()

    # Read metadata
    lm_tiff = Path(lm_tiff)
    minfo = utils.get_raster_info(lm_tiff)
    if minfo["tag"] is None:
        sys.exit("ERROR: No valid GuidosToolbox metadata found in the input Geotiff")

    # Check input tag with used tool and parametres
    tool_params = utils.get_tool_parameters(minfo["tag"])
    if tool_params.get("tool_id") != "GTB_LM":
        sys.exit(f"ERROR: Input Geotiff is labeled as '{tool_params.get('tool_id')}', "
            "landmos_stats requires a 'GTB_LM' result file."
        )

    # Get Landscape Mosaic parameters
    window_size = int(tool_params["wsize"])

    # Define input and output file names
    out_name = Path(lm_tiff).stem.split('_103class')[0]
    outdir = Path(outdir) if outdir else lm_tiff.parent
    source_tiff = Path(source_tiff) if source_tiff else None

    # Counting source Geotiff pixels
    if source_freq:
        source_pxl_numb = source_freq
    else:
        if source_tiff:
            with rasterio.open(source_tiff) as src:
                source_data = src.read()
            source_pxl_numb = utils.get_pxl_freq(source_data)
        else:
            source_pxl_numb=None

    class1 = source_pxl_numb[1] if source_pxl_numb else "n/a"
    class2 = source_pxl_numb[2] if source_pxl_numb else "n/a"
    class3 = source_pxl_numb[3] if source_pxl_numb else "n/a"

    # Landscape Mosaic pixel counting
    if lm_freq:
        lm_pixel_freq = lm_freq
    else:
        with rasterio.open(lm_tiff) as src:
            lm_data = src.read()
        lm_pixel_freq = utils.get_pxl_freq(lm_data)

    NoData = lm_pixel_freq[0]
    foregr = (minfo["rows"] * minfo["cols"]) - NoData
    lm_pixel_prop = {k: v / foregr * 100 for k, v in lm_pixel_freq.items()}

    # Max value
    max_key = max((k for k in lm_pixel_prop if k > 0), key=lm_pixel_prop.get)

    # Remap to 19 classes and pixel counting
    reclass_map = {}
    with open(TEMPL_DIR / 'lm_103to19.txt' , 'r') as f:
        for line in f:
            l = [int(x) for x in line.split(' ')]
            reclass_map[l[0]]=l[1]
    lm_pixel_freq_19 = Counter()
    for orig_val, count in lm_pixel_freq.items():
        new_class = reclass_map.get(orig_val)
        if new_class is not None:
            lm_pixel_freq_19[new_class] += count

    lm_pixel_prop_19 = Counter({k: v / foregr *100
                                          for k, v in lm_pixel_freq_19.items()})
    A_rel = lm_pixel_prop_19[1]
    D_rel = lm_pixel_prop_19[2]
    N_rel = lm_pixel_prop_19[3]
    Ad_rel = lm_pixel_prop_19[4]
    An_rel = lm_pixel_prop_19[5]
    Dn_rel = lm_pixel_prop_19[6]
    Da_rel = lm_pixel_prop_19[7]
    Na_rel = lm_pixel_prop_19[8]
    Nd_rel = lm_pixel_prop_19[9]
    Adn_rel = lm_pixel_prop_19[10]
    Dan_rel = lm_pixel_prop_19[11]
    Nad_rel = lm_pixel_prop_19[12]
    ad_rel = lm_pixel_prop_19[13]
    an_rel = lm_pixel_prop_19[14]
    dn_rel = lm_pixel_prop_19[15]
    adn_rel = lm_pixel_prop_19[16]
    NN_rel = lm_pixel_prop_19[17]
    AA_rel = lm_pixel_prop_19[18]
    DD_rel = lm_pixel_prop_19[19]
    NoD_rel = lm_pixel_prop_19[0]


    if outfile:

        ### PNG HEATMAP ###
        fig = None
        try:
            value_ids = {
                        1:[171],
                        2:[91,172,81],
                        3:[93,92,121,82,83],
                        4:[95,94,124,122,123,84,85],
                        5:[152,151,217,216,215,214,213,145,144],
                        6:[154,153,219,218,233,232,231,212,211,143,142],
                        7:[65,155,221,220,235,234,236,230,229,210,209,141,55],
                        8:[63,64,113,222,223,224,225,226,227,228,207,208,104,54,53],
                        9:[61,62,111,112,114,200,201,202,203,204,205,206,103,102,101,52,51],
                        10:[191,192,71,72,73,74,75,131,132,133,134,135,45,44,43,42,41,182,181]
            }
            tri_seq = []
            for i in range(10, 0, -1):
                tri_seq += value_ids[i]

            fig, ax = plt.subplots(figsize=(10, 8))

            # Initialize the tax object
            tax = ternary.TernaryAxesSubplot(ax=ax, scale=100)
            ax.set_aspect('equal', adjustable='box')
            tax.boundary(linewidth=2)
            tax.gridlines(multiple=10, color="lightgrey", linestyle='-', linewidth=0.7)

            # Bolding the 0%, 10%, 60% lines
            # Coordinates follow: (Natural, Agriculture, Developed)
            def draw_line(p1, p2, color='black', linewidth=2.0):
                #fp1 = project_point(p1)
                #fp2 = project_point(p2)
                #tax.get_axes().plot([fp1[0], fp2[0]], [fp1[1], fp2[1]], color=color, linewidth=linewidth, zorder=3)
                xs, ys = zip(project_point(p1), project_point(p2))
                tax.get_axes().plot(xs, ys, color=color, linewidth=linewidth, zorder=3)

            for v in [0, 10, 60]:
                draw_line((v, 100-v, 0), (v, 0, 100-v))
                draw_line((100-v, v, 0), (0, v, 100-v))
                draw_line((100-v, 0, v), (0, 100-v, v))

            # # Helper function to convert (1, 2, 3) to (x, y)
            # def draw_filled_region(points, color, alpha=0.5):
            #     # Project all ternary points to cartesian x,y
            #     projected_points = [project_point(p) for p in points]
            #     # Extract x and y lists
            #     xs, ys = zip(*projected_points)
            #     # Use the underlying matplotlib axes to fill
            #     tax.get_axes().fill(xs, ys, facecolor=color, alpha=alpha, edgecolor='black', linewidth=0.5)

            # Regions
            zones = {
                # Pure Corners (>90%)
                "A":  {"poly": [(100,0,0), (90,10,0), (80,10,10), (90,0,10)], "rgb": (0, 0, 1)},
                "N":  {"poly": [(0,100,0), (10,90,0), (10,80,90), (0,90,10)], "rgb": (0, 1, 0)},
                "D":  {"poly": [(0,0,100), (10,0,90), (10,10,80), (0,10,90)], "rgb": (1, 0, 0)},

                # Secondary Transition Zones (60-90%) - Fixed Geometry
                "An": {"poly": [(90,10,0), (60,40,0), (60,30,10), (80,10,10)], "rgb": (0/255, 128/255, 255/255)},
                "Ad": {"poly": [(90,0,10), (80,10,10), (60,10,30), (60,0,40)], "rgb": (128/255, 0/255, 255/255)},
                "Na": {"poly": [(10,90,0), (10,80,10), (30,60,10), (40,60,0)], "rgb": (0/255, 255/255, 128/255)},
                "Nd": {"poly": [(0,90,10), (0,60,40), (10,60,30), (10,80,10)], "rgb": (128/255, 255/255, 0/255)},
                "Dn": {"poly": [(0,10,90), (0,40,60), (10,30,60), (10,10,80)], "rgb": (255/255, 128/255, 0/255)},
                "Da": {"poly": [(10,0,90), (40,0,60), (40,10,50), (10,10,80)], "rgb": (255/255, 0/255, 128/255)},

                # Tertiary Transition Zones (40-60%)
                "Adn": {"poly": [(60,10,30), (80,10,1), (60,30,10)], "rgb": (128/255, 128/255, 255/255)},
                "Nad": {"poly": [(10,60,30), (30,60,10), (10,80,10)], "rgb": (128/255, 255/255, 128/255)},
                "Dan": {"poly": [(10,10,80), (30,10,60), (10,30,60)], "rgb": (255/255, 128/255, 128/255)},

                # Mid-Transition Zones
                "ad": {"poly": [(60,0,40), (60,10,30), (30,10,60), (40,0,60)], "rgb": (128/255, 0, 128/255)},
                "an": {"poly": [(40,60,0), (30,60,10), (60,30,10), (60,40,0)], "rgb": (0, 128/255, 128/255)},
                "dn": {"poly": [(0,60,40), (0,40,60), (10,30,60), (10,60,30)], "rgb": (128/255, 128/255, 0)},

                # Large Center Zone (adn)
                "adn": {"poly": [(10,30,60), (30,10,60), (60,10,30), (60,30,10), (30,60,10), (10,60,30)], "rgb": (128/255, 128/255, 128/255)},
            }
            for name, zone in zones.items():
                #projected_poly = [project_point(p) for p in zone["poly"]]
                #xs, ys = zip(*projected_poly)
                xs, ys = zip(*[project_point(p) for p in zone["poly"]])
                ax.fill(xs, ys, facecolor=zone["rgb"], edgecolor='white', linewidth=0.7, zorder=1)


            # Axis Labels with Arrows (Matching the Reference)
            ax.text(50, -7, "More Blue (class=1) $\\rightarrow$",
                    ha='center', va='top', fontsize=16)
            ax.text(15, 45, r"$\leftarrow$ More Red (class=3)",
                    rotation=60, ha='center', va='center', fontsize=16)
            ax.text(86, 45, r"$\leftarrow$ More Green (class=2)",
                    rotation=-60, ha='center', va='center', fontsize=16)

            # Ticks
            tax.ticks(axis='lbr', multiple=10, linewidth=1, offset=0.018, tick_formats="%d%%",
                      fontsize=12)
            for text in ax.texts:
                label = text.get_text()
                if "%" in label:
                    x, y = text.get_position()
                    if y < 1:
                        # Decrease the second value to move it "down",
                        # or increase it to move it "up" (closer to the triangle)
                        text.set_position((x, y + 3))
                        text.set_verticalalignment('top')
            tax.get_axes().axis('off')

            # Corner Circles
            corners = {
                180: {"coord": (109, -9, 0), "color": "#0000FF", "offset": (6, -6)},
                170: {"coord": (0, 109, -9), "color": "#228B22", "offset": (0, 9)},
                190: {"coord": (-9, 0, 109), "color": "#FF0000", "offset": (-7, -7)}
            }
            for value, params in corners.items():
                # Get the XY position of the vertex
                center = project_point(params["coord"])
                prop = lm_pixel_prop.get(value)
                if prop is None:
                    prop = 0.0
                bg_col, txt_col = use_color(value, max_key)

                # Create big circle
                circle = patches.Circle((center[0], center[1]), 3.6, color=params["color"],
                        ec='black', lw=1.5, zorder=10)
                ax.add_patch(circle)

                # Create small circle
                circle = patches.Circle((center[0], center[1]), 2.2,
                                        color=bg_col,
                                        ec='none', alpha=0.9, zorder=14)
                ax.add_patch(circle)

                # Add freqeuncy value
                ax.text(center[0], center[1], frq_str(prop),
                                   color=txt_col,
                                   fontsize=9, ha='center', va='center', zorder=15)

            # Corner Arrows
            end_pos = project_point((0, 100, 0))
            start_pos = project_point((0, 106, -6))
            ax.annotate('', xy=(end_pos[0], end_pos[1]), xytext=(start_pos[0], start_pos[1]),
                        arrowprops=dict(arrowstyle='->', lw=2, mutation_scale=10))
            end_pos = project_point((100, 0, 0))
            start_pos = project_point((106, -6, 0))
            ax.annotate('', xy=(end_pos[0], end_pos[1]), xytext=(start_pos[0], start_pos[1]),
                        arrowprops=dict(arrowstyle='->', lw=2, mutation_scale=10))
            end_pos = project_point((0, 0, 100))
            start_pos = project_point((-6, 0, 106))
            ax.annotate('', xy=(end_pos[0], end_pos[1]), xytext=(start_pos[0], start_pos[1]),
                        arrowprops=dict(arrowstyle='->', lw=2, mutation_scale=10))

            # 100 Circles
            circle_patches = []
            circle_colors  = []
            text_items     = []

            step = 10
            id_counter = 1  # Placeholder for IDs
            for i in range(0, 100, step):
                for j in range(0, 100 - i, step):

                    # 1. UPWARD TRIANGLES
                    if i + step <= 100:
                        # Mid-point of the cell
                        c_a = i + 3.33
                        c_n = j + 3.33
                        c_d = 100 - c_a - c_n

                        if c_d >= 0:
                            id_cell = tri_seq[id_counter-1]
                            prop = lm_pixel_prop.get(id_cell)
                            bg_col, txt_col = use_color(id_cell, max_key)
                            pos = project_point((c_n, c_a, c_d))

                            circle_patches.append(patches.Circle(pos, 2.3))
                            circle_colors.append(bg_col)
                            text_items.append((pos[0], pos[1], frq_str(prop), txt_col))
                            id_counter += 1

                    # 2. DOWNWARD TRIANGLES
                    if i + step <= 100 and j + step <= 100 and (100 - i - j - step) >= 0:
                        # Shifted center for the inverted triangles
                        c_a_down = i + 6.66
                        c_n_down = j + 6.66
                        c_d_down = 100 - c_a_down - c_n_down

                        if c_d_down > 0:
                            id_cell = tri_seq[id_counter-1]
                            prop = lm_pixel_prop.get(id_cell)
                            bg_col, txt_col = use_color(id_cell, max_key)
                            pos_down = project_point((c_n_down, c_a_down, c_d_down))

                            circle_patches.append(patches.Circle(pos_down, 2.3))
                            circle_colors.append(bg_col)
                            text_items.append((pos_down[0], pos_down[1], frq_str(prop), txt_col))
                            id_counter += 1

            # Add all circles
            pc = PatchCollection(circle_patches, facecolors=circle_colors, edgecolors='none', alpha=0.9, zorder=14)
            ax.add_collection(pc)

            # Add all text in one batch
            for x, y, txt, col in text_items:
                ax.text(x, y, txt, color=col, fontsize=9, ha='center', va='center', zorder=15)

            ax.set_xlim(-15, 120)
            ax.set_ylim(-15, 100)

            plt.tight_layout()
            png_file = outdir / f'{out_name}_heatmap.png'
            plt.savefig(png_file, dpi=300, facecolor='white', transparent=False)
            plt.close()

        finally:
            if fig is not None:
                plt.close(fig)

        ### CSV Export ####
        csv_file = outdir / f'{out_name}.csv'
        with open(csv_file, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(['pixel_value', 'pixel_count', 'foreground_proportion'])
            for v in sorted(lm_pixel_prop.keys()):
                if v>0:
                    freq = lm_pixel_prop[v]
                    writer.writerow([v, lm_pixel_freq[v], f"{freq:.6f}"])


        ### CSV Triangle Export ###
        csv_file_hm = outdir / f'{out_name}_heatmap.csv'
        with open(csv_file_hm, 'w', newline='') as f:
            writer = csv.writer(f)
            # Title
            writer.writerow([f'Landscape Mosaic using Window size ({window_size}x{window_size})'] + [' ']*21)
            # NN
            freq170 = lm_pixel_prop.get(170, 0.0)
            writer.writerow([' ']*11 + [f"{freq170:.3f}"] + [' ']*10)
            # Triangle
            for lin in sorted(value_ids.keys()):
                frqs = []
                for p in value_ids[lin]:
                    freq_val = lm_pixel_prop.get(p, 0.0)
                    frqs.append(f'{freq_val:.3f}')
                writer.writerow([' ']*(12-lin) + frqs + [' ']*(11-lin))
            # DD and AA
            freq190 = lm_pixel_prop.get(190, 0.0)
            freq180 = lm_pixel_prop.get(180, 0.0)
            writer.writerow([' '] + [f"{freq190:.3f}"] + [' ']*19 + [f"{freq180:.3f}"])


        ### TXT Template Reporting ###

        content = {
            "input_file": source_tiff.name if source_tiff else "n/a",
            "epsg_code": minfo["epsg"],
            "unit_type": 'metres' if minfo["is_projected"] else 'degrees',
            "resolx": minfo["resX"],
            "resoly": minfo["resY"],
            "rows_val": minfo["rows"],
            "tot_pxl": minfo["rows"] * minfo["cols"],
            "cols_val": minfo["cols"],
            "class1_pxl": class1,
            "class2_pxl": class2,
            "class3_pxl": class3,
            "miss_pxl": NoData,
            "foregr_pxl": foregr,

            "w_size": window_size,
            "window_areaHA": f"{(window_size**2)*minfo['resX']*minfo['resY']/10000:.4f}" if minfo["is_projected"] else '--',
            "window_areaAC": f"{(window_size**2)*minfo['resX']*minfo['resY']*0.000247105:.4f}" if minfo["is_projected"] else '--',

            "output_file": f'{out_name}.tif',

            "A_val": f'{A_rel:6.3f}',
            "D_val": f'{D_rel:6.3f}',
            "N_val": f'{N_rel:6.3f}',
            "Ad_val": f'{Ad_rel:6.3f}',
            "An_val": f'{An_rel:6.3f}',
            "Dn_val": f'{Dn_rel:6.3f}',
            "Da_val": f'{Da_rel:6.3f}',
            "Na_val": f'{Na_rel:6.3f}',
            "Nd_val": f'{Nd_rel:6.3f}',
            "Adn_val": f'{Adn_rel:6.3f}',
            "Dan_val": f'{Dan_rel:6.3f}',
            "Nad_val": f'{Nad_rel:6.3f}',
            "ad_val": f'{ad_rel:6.3f}',
            "an_val": f'{an_rel:6.3f}',
            "dn_val": f'{dn_rel:6.3f}',
            "adn_val": f'{adn_rel:6.3f}',
            "NN_val": f'{NN_rel:6.3f}',
            "AA_val": f'{AA_rel:6.3f}',
            "DD_val": f'{DD_rel:6.3f}',
            "NoD_val": f'{NoD_rel:6.3f}',

            "comp_time": f"{utils.running_time(start_time_stat, time.time())}"
        }
        txt_file = outdir / f'{out_name}.txt'
        utils.generate_text_report(TEMPL_DIR / 'lm_templ.txt', txt_file, content)

    # Statistic dictionaries
    
    # Try to find the 19-class file in the same folder
    path_19cl = None
    tif_19cl = outdir / f"{out_name}.tif"
    if tif_19cl.exists():
        path_19cl = str(tif_19cl)
    
    path_stats_dict = None
    if outfile:
        path_stats_dict = {
            "path tif 103cl" : str(lm_tiff),
            "path tif 19cl" : path_19cl,
            "path txt" : str(txt_file),
            "path csv" : str(csv_file),
            "path csv hm" : str(csv_file_hm),
            "path png" : str(png_file)
            }
    input_stats_dict = {
        "class1 pxl": class1,
        "class2 pxl": class2,
        "class3 pxl": class3,
        "foreground pxl": foregr,
        "missing pxl": NoData
        }
    output_stats_dict = {
        "pxl numb 103cl": lm_pixel_freq,
        "pxl numb 19cl": lm_pixel_freq_19,
        }
    stats_dict = {
        "output paths" : path_stats_dict,
        "input stats" : input_stats_dict,
        "output stats" : output_stats_dict
                  }

    return stats_dict



# Frequency visialization
def frq_str(frq):
    if frq is None or frq == 0:
        return '-'
    elif frq >= 0.1:
        return f"{frq:.1f}"
    else:
        return '<0.1'

# Color visualization:
def use_color(key, max_key):
    is_max = (key == max_key)
    circle = 'black' if is_max else 'white'
    text = 'white' if is_max else 'black'
    return circle, text