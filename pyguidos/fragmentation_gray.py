import sys
import time
from pathlib import Path
import csv
import matplotlib.pyplot as plt

import rasterio
import numpy as np

from . import engine
from . import utils
from . import checks
from . import TEMPL_DIR


def frag_gray(
    in_tiff,
    method,
    window_size,
    for_threshold,
    connectivity=4,
    outdir=None,
    statists=True,
    stat_files=True,
    verb=False
    ):
    """
    Performs grayscale Fragmentation analysis on a continuous-value
    raster where pixel values represent foreground intensity from 0 to 100.
    Each foreground pixel is assigned a value within the moving window,
    classifying landscape fragmentation into 5 classes: Rare, Patchy,
    Transitional, Dominant, and Interior.

    Parameters
    ----------
    in_tiff : str or Path
        Path to the input GeoTIFF. Must be uint8 with values:
        0 = Non-foreground, 1-100 = Foreground intensity (%),
        255 = NoData.
    method : str, optional
        Fragmentation method. Default 'FAD'.
        'FAD' Foreground Area Density (grayscale)
        'FAC' Foreground Area Clustering (grayscale)
        'FED' Foreground Edge Density (grayscale)
    window_size : int
        Size of the moving window in pixels. Must be an odd integer >= 3.
    for_threshold : int, optional
        Foreground threshold value from 1 to 100. Pixels with values
        below this threshold are treated as non-foreground (0) during
        computation.
    connectivity : int, optional
        Pixel connectivity for FED method. Must be 4 or 8.
        Default 4. Ignored for FAD method.
    outdir : str or Path, optional
        Directory for output files. Defaults to the input file's directory.
    statists : bool, optional
        If True (default), computes and returns statistics.
    stat_files : bool, optional
        If True (default), writes statistics to .txt, .csv and .png files.
    verb : bool, optional
        If True, prints progress messages. Default False.

    Returns
    -------
    dict
        Nested dictionary with three keys:
        - 'output paths' (dict or None): paths to generated output files
          ('path tif', 'path txt', 'path csv', 'path png'),
          or None if stat_files=False.
        - 'input stats' (dict): pixel counts for foreground, non-foreground,
          and missing pixels.
        - 'output stats' (dict): per-class pixel counts for fragmentation
          classes (rare, patchy, transitional, dominant, interior) and
          fragmentation indices (FAD_av, AVcon).

    Output Files
    ------------
    - <in_name>_frag_gray_<method>_<window_size>.tif  : fragmentation result
    - <in_name>_frag_gray_<method>_<window_size>.txt  : statistics report
    - <in_name>_frag_gray_<method>_<window_size>.csv  : per-value pixel counts
    - <in_name>_frag_gray_<method>_<window_size>.png  : foreground pixel histogram
    """
    start_time = time.time()

    # Log
    utils.log_msg(verb, "[   START   ]  Verifying input raster...")

    # Validate parameters
    checks.validate_wsize(window_size)
    method = method.upper()
    if method not in ['FAD', 'FAC', 'FED']:
        sys.exit(f"Grayscale fragmentation method must be ['FAD', 'FAC', 'FED'] (received '{method}')")
    if method in ['FAC', 'FED'] and connectivity not in [4, 8]:
        sys.exit(f"Connectivity must be 4 or 8 (received {connectivity})")
    if not isinstance(for_threshold, int) or not (1 <= for_threshold <= 100):
        sys.exit(f"for_threshold must be an integer between 1 and 100 (received {for_threshold})")

    # Initialize Paths and Metadata
    in_tiff = Path(in_tiff)
    outdir = Path(outdir) if outdir else in_tiff.parent
    in_name = in_tiff.stem
    out_name = f"{in_name}_frag_gray_{method.lower()}_{window_size}"
    info = utils.get_raster_info(in_tiff)

    # Read the input Geotiff
    with rasterio.open(in_tiff) as src:
        input_data = src.read(1).astype(np.int16)

    # Get the pixel counting
    input_pxl_freq = utils.get_pxl_freq(input_data)

    # Validate input
    checks.validate_fmap_gray_input(list(input_pxl_freq.keys()), info["bands"], info['dtype'])

    # Count input Foreground (1-100) and Background (0) before thresholding
    inFG = sum(count for val, count in input_pxl_freq.items() if 1 <= val <= 100)
    inBG = input_pxl_freq.get(0, 0)

    # Log
    utils.log_msg(verb, "[    OK     ]  Input raster verified.")

    try:
        # Log
        utils.log_msg(verb, "[   START   ]  Computing Grayscale Fragmentation...")

        # Compute grayscale fragmentation
        if method == 'FAD':
            data_out = engine.compute_FAD_gray(input_data, window_size, 1, for_threshold)
        elif method == 'FAC':
            data_out = engine.compute_FAC_gray(input_data, window_size, 1, for_threshold, connectivity)
        elif method == 'FED':
            data_out = engine.compute_FED_gray(input_data, window_size, 1, for_threshold, connectivity)

        # Save Final Geotiff with Palette and Tags
        weblink = "https://forest.jrc.ec.europa.eu/en/activities/lpa/gtb/"
        tag_descr = f"GTB_FOS, <Gray,{for_threshold},{connectivity},{method.upper()}_5,{info['resX']},{window_size}>, {weblink}"
        cmap_path = TEMPL_DIR / "frag_colormap.txt"
        out_tiff = outdir / f"{out_name}.tif"
        utils.save_output_geotiff(out_tiff, data_out, info['profile'], cmap_path, tag_descr)

        # Log
        utils.log_msg(verb, "[    OK     ]  Grayscale Fragmentation computed.")

        # Statistics and Reporting
        stats_dict = None
        if statists:
            # Log
            utils.log_msg(verb, "[   START   ]  Generating statistics...")

            frag_pxl_freq = utils.get_pxl_freq(data_out)
            minfo = utils.get_raster_info(out_tiff)
            stats_dict = _get_frag_gray_stats(frag_freq=frag_pxl_freq,
                                              tiff_info=minfo,
                                              in_fg_bg=[inFG, inBG],
                                              outfile=stat_files,
                                              out_name=out_name,
                                              out_dir=outdir,
                                              source_tiff=in_tiff)

        # Computational time
        time_str = utils.running_time(start_time, time.time())
        if statists and stat_files:
            txt_file = outdir / f'{out_name}.txt'
            utils.update_time_line(txt_file, time_str)
            utils.log_msg(verb, "[    OK     ]  Statistics complete and files saved.")

        # Log
        utils.log_msg(verb, f"\n>>> Grayscale Fragmentation task finished in {time_str}")

        return stats_dict

    except Exception as e:
        print(f"Error during run: {e}")
        raise


def frag_gray_stats(frag_tiff, stat_files=True, outdir=None, source_tiff=None):
    """
    Computes statistics for an existing grayscale Fragmentation result GeoTIFF.
    Can be called independently on a previously generated fragmentation output,
    or is invoked automatically by frag_gray() when statists=True.

    Parameters
    ----------
    frag_tiff : str or Path
        Path to the fragmentation result GeoTIFF. Must contain a valid
        GTB_FOS metadata tag with tiftype='Gray' in the
        TIFFTAG_IMAGEDESCRIPTION field.
    stat_files : bool, optional
        If True (default), writes statistics to .txt, .csv and .png files.
    outdir : str or Path, optional
        Directory for output files. Defaults to the input file's directory.
    source_tiff : str or Path, optional
        Path to the original input GeoTIFF used to generate the fragmentation
        result. Used only to report the source filename in the statistics
        report. Default None.

    Returns
    -------
    dict
        Nested dictionary with three keys:
        - 'output paths' (dict or None): paths to generated output files
          ('path tif', 'path txt', 'path csv', 'path png'),
          or None if stat_files=False.
        - 'input stats' (dict): pixel counts for foreground, background,
          and missing pixels.
        - 'output stats' (dict): per-class pixel counts for fragmentation
          classes (rare, patchy, transitional, dominant, interior) and
          fragmentation indices (FAD_av, AVcon).

        Note: 'input stats' will show 'n/a' for input foreground/background
        counts when called standalone, as the original input file information
        is not available from the output raster alone.

    Output Files
    ------------
    - <frag_tiff_stem>.txt : statistics report
    - <frag_tiff_stem>.csv : per-value pixel counts and frequencies
    - <frag_tiff_stem>.png : foreground pixel histogram
    """
    start_time_stat = time.time()

    # Read metadata
    frag_tiff = Path(frag_tiff)
    minfo = utils.get_raster_info(frag_tiff)
    if minfo["tag"] is None:
        sys.exit("ERROR: No valid GuidosToolbox metadata found in the input Geotiff")

    # Check input tag with used tool and parameters
    tool_params = utils.get_tool_parameters(minfo["tag"])
    if tool_params.get("tool_id") != "GTB_FOS":
        sys.exit(f"ERROR: Input Geotiff is labeled as '{tool_params.get('tool_id')}', "
                 "frag_gray_stats requires a 'GTB_FOS' result file.")
    if tool_params.get("tiftype") != "Gray":
        sys.exit(f"ERROR: Input Geotiff is a binary fragmentation result (tiftype='{tool_params.get('tiftype')}'). "
                 "frag_gray_stats requires a grayscale ('Gray') result file. "
                 "Use frag_stats() for binary results.")

    # Define input and output file names
    out_name = Path(frag_tiff).stem
    outdir = Path(outdir) if outdir else frag_tiff.parent
    source_tiff = Path(source_tiff) if source_tiff else None

    # Fragmentation pixel counting
    with rasterio.open(frag_tiff) as src:
        frag_data = src.read(1)
    frag_pxl_freq = utils.get_pxl_freq(frag_data)

    # Get statistics (in_fg_bg=None since input info not available)
    stats_dict = _get_frag_gray_stats(frag_freq=frag_pxl_freq,
                                      tiff_info=minfo,
                                      in_fg_bg=None,
                                      outfile=stat_files,
                                      out_name=out_name,
                                      out_dir=outdir,
                                      source_tiff=source_tiff)

    # Computational time
    time_str = utils.running_time(start_time_stat, time.time())
    if stat_files:
        txt_file = outdir / f'{out_name}.txt'
        utils.update_time_line(txt_file, time_str)

    return stats_dict


#############

def _get_frag_gray_stats(frag_freq,
                         tiff_info,
                         in_fg_bg=None,
                         outfile=True,
                         out_name=None,
                         out_dir=None,
                         source_tiff=None):
    """
    Get the Grayscale Fragmentation statistics.
    """

    # Get Frag parameters
    tag = tiff_info["tag"]
    tool_params = utils.get_tool_parameters(tag)
    method, fclasses = tool_params["method"].split('_')
    threshold = tool_params["for_thres"]
    window_size = int(tool_params["wsize"])

    # Define input and output file names
    source_tiff = Path(source_tiff) if source_tiff else None

    # Input pixel counts (before thresholding)
    if in_fg_bg:
        inFG = in_fg_bg[0]
        inBG = in_fg_bg[1]
    elif source_tiff:
        with rasterio.open(source_tiff) as src:
            source_data = src.read(1)
        source_freq = utils.get_pxl_freq(source_data)
        inFG = sum(count for val, count in source_freq.items() if 1 <= val <= 100)
        inBG = source_freq.get(0, 0)
    else:
        inFG = "n/a"
        inBG = "n/a"

    # Output tiff classes (after thresholding)
    bgrnd = frag_freq[101]
    ndata = frag_freq[102]

    # Counting pixel per Frag class
    rare = sum(frag_freq[i] for i in range(10))
    patchy = sum(frag_freq[i] for i in range(10, 40))
    trans = sum(frag_freq[i] for i in range(40, 60))
    domin = sum(frag_freq[i] for i in range(60, 90))
    inter = sum(frag_freq[i] for i in range(90, 101))

    fgrnd = rare + patchy + trans + domin + inter
    ruarea = fgrnd + bgrnd
    sum_prod = sum(v * frag_freq[v] for v in range(101))

    fad_av = sum_prod / fgrnd if fgrnd > 0 else 0
    avcon = sum_prod / ruarea if ruarea > 0 else 0

    if outfile:

        ### CSV Export ###
        csv_file = out_dir / f'{out_name}.csv'
        with open(csv_file, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(['pixel_value', 'pixel_count', 'foreground_proportion'])
            for v in range(101):
                pct = (frag_freq[v] / fgrnd * 100) if fgrnd > 0 else 0
                writer.writerow([v, frag_freq[v], f"{pct:.6f}"])

        ### Histogram PNG figure ###

        # X & Y values
        pixel_values = list(range(101))
        frag_pxl_prop = [frag_freq[i] / fgrnd * 100 if fgrnd > 0 else 0
                         for i in pixel_values]

        # Create the colormap
        cmap_path = TEMPL_DIR / "frag_colormap.txt"
        colors, _ = utils.get_colormap(cmap_path)
        bar_colors = [colors.get(v) for v in pixel_values]

        # Create the figure with bar chart
        fig, ax = plt.subplots(figsize=(7, 6))
        ax.bar(pixel_values, frag_pxl_prop, color=bar_colors, width=1.0,
               edgecolor='black', linewidth=0.4)

        # Formatting the Axes
        ax.set_xlabel('FAD (grayscale)', fontsize=14)
        ax.set_ylabel('Frequency [%]', fontsize=14)
        ax.set_title('Foreground pixel histogram', fontsize=15, pad=20)
        ax.tick_params(axis='both', which='major', labelsize=12)

        # Set X-axis limits and ticks
        ax.set_xlim(-1, 101)
        ax.set_xticks(range(0, 101, 10))

        # Plotting
        plt.tight_layout()
        png_file = out_dir / f'{out_name}.png'
        plt.savefig(png_file, dpi=300, facecolor='white', transparent=False)
        plt.close()

        ### TXT Template Reporting ###
        content = {
            "input_file": source_tiff.name if source_tiff else "n/a",
            "epsg_code": tiff_info["epsg"],
            "unit_type": 'metres' if tiff_info["is_projected"] else 'degrees',
            "resolx": tiff_info["resX"],
            "resoly": tiff_info["resY"],
            "rows_val": tiff_info["rows"],
            "cols_val": tiff_info["cols"],
            "tot_pxl": tiff_info["rows"] * tiff_info["cols"],
            "in_foreg_pxl": inFG,
            "in_backg_pxl": inBG,
            "miss_pxl": ndata,

            "used_method": f"{method} (grayscale)",
            "for_thresh": threshold,
            "window_size": window_size,
            "window_areaHA": f"{(window_size**2)*tiff_info['resX']*tiff_info['resY']/10000:.4f}" if tiff_info["is_projected"] else '--',
            "window_areaAC": f"{(window_size**2)*tiff_info['resX']*tiff_info['resY']*0.000247105:.4f}" if tiff_info["is_projected"] else '--',

            "output_file": f"{out_name}.tif",
            "out_foreg_pxl": fgrnd,
            "out_backg_pxl": bgrnd,
            "rep_unit_pxl": ruarea,
            "foreg_area_rel": (fgrnd / ruarea * 100) if ruarea > 0 else 0,
            "rare_val": f"{rare:>9}",
            "patch_val": f"{patchy:>9}",
            "trans_val": f"{trans:>9}",
            "domin_val": f"{domin:>9}",
            "inter_val": f"{inter:>9}",
            "rare_pro": f"{(rare / fgrnd) * 100:7.4f}",
            "patch_pro": f"{(patchy / fgrnd) * 100:7.4f}",
            "trans_pro": f"{(trans / fgrnd) * 100:7.4f}",
            "domin_pro": f"{(domin / fgrnd) * 100:7.4f}",
            "inter_pro": f"{(inter / fgrnd) * 100:7.4f}",            
            "fad_av_idx": fad_av,
            "avcon_idx": avcon,
        }

        txt_file = out_dir / f'{out_name}.txt'
        utils.generate_text_report(TEMPL_DIR / 'frag_gray_templ.txt', txt_file, content)

    # Statistic dictionaries
    path_stats_dict = None
    if outfile:
        path_stats_dict = {
            "path tif": str(out_dir / f"{out_name}.tif"),
            "path txt": str(txt_file),
            "path csv": str(csv_file),
            "path png": str(png_file)
        }
    input_stats_dict = {
        "in foreground pxl": inFG,
        "in background pxl": inBG,
        "out foreground pxl": fgrnd,
        "out background pxl": bgrnd,
        "missing pxl": ndata
    }
    class_freq = {
        "1 rare pxl": rare,
        "2 patch pxl": patchy,
        "3 trans pxl": trans,
        "4 domin pxl": domin,
        "5 inter pxl": inter
    }
    output_stats_dict = {
        "class freq": class_freq,
        "fad_av": fad_av,
        "avcon": avcon
    }
    stats_dict = {
        "output paths": path_stats_dict,
        "input stats": input_stats_dict,
        "output stats": output_stats_dict
    }

    return stats_dict


