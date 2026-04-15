import sys
import time
from pathlib import Path
import csv
import matplotlib.pyplot as plt
import shutil

import rasterio
import numpy as np

from . import utils
from . import checks
from . import engine
from . import TEMPL_DIR


def frag(
    in_tiff,
    method,
    window_size,
    outdir=None,
    statists=True,
    stat_files=True,
    verb=False
    ):
    """
    Performs Fragmentation analysis on a binary raster using Spatcon moving
    window tool. Computes the proportion of foreground pixels within each
    window, classifying landscape fragmentation into 5 classes: Rare, Patchy,
    Transitional, Dominant, and Interior.

    Parameters
    ----------
    in_tiff : str or Path
        Path to the input GeoTIFF. Must be uint8 with values:
        0 = NoData, 1 = Background, 2 = Foreground.
        Optional: 3 = Background class 2, 4 = Background class 3.
    method : str
        Fragmentation method. Must be 'FAD' (Forest Area Density) or
        'FAC' (Foreground Area Clustering).
    window_size : int
        Size of the moving window in pixels. Must be an odd integer >= 3.
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
          or None if outfile=False.
        - 'input stats' (dict): pixel counts for foreground, background,
          missing and special class pixels.
        - 'output stats' (dict): per-class pixel counts for fragmentation
          classes (rare, patchy, transitional, dominant, interior) and
          fragmentation indices (FAD_av, AVcon).


    Output Files
    ------------
    - <in_name>_<method>_<window_size>.tif  : fragmentation result
    - <in_name>_<method>_<window_size>.txt  : statistics report
    - <in_name>_<method>_<window_size>.csv  : per-value pixel counts
    - <in_name>_<method>_<window_size>.png  : foreground pixel histogram
    """
    start_time = time.time()
    success = False

    # Validate parametres
    checks.validate_frag_params(window_size, method)

    # Initialize Paths and Metadata
    in_tiff = Path(in_tiff)
    outdir = Path(outdir) if outdir else in_tiff.parent
    in_name = in_tiff.stem
    out_name = f"{in_name}_frag_{method.lower()}_{window_size}"
    info = utils.get_raster_info(in_tiff)

    # Read the input Geotiff
    with rasterio.open(in_tiff) as src:
        input_data = src.read()

    # Get the pixel counting
    input_pxl_freq = utils.get_pxl_freq(input_data)

    # Input Geotiff validations
    checks.validate_fmap_input(list(input_pxl_freq.keys()), info["bands"], info['dtype'], allow_34=True)

    try:
        # Copy input tiff to temp dir
        tmpdir = utils.setup_run_dir()
        engine.write_spatcon_input(tmpdir, input_data)

        # Write Spatcon TXT files
        dims = (info['rows'], info['cols'])
        meth_codes = {"FAD": [81, 0], "FAC": [76, 2]}
        w = window_size
        r = meth_codes[method][0]
        b = meth_codes[method][1]
        a = 2
        h = 1
        m = 0
        f = 1
        engine.write_spatcon_params(tmpdir, dims, (w, r, b, a, h, m, f))

        # Execute Spatcon Binary
        engine.run_spatcon(tmpdir, verbose=verb)

        # Process Output
        spat_bin = tmpdir / "scoutput"
        out_data = np.fromfile(spat_bin, dtype=np.float32).reshape(1, info['rows'], info['cols'])
        out_data_int = np.floor(out_data * 100 + 0.5).astype(np.uint8, casting='unsafe')

        # Mapping logic (NoData, Background, Special codes)
        choices = np.array([102, 101, 105, 106], dtype=np.uint8)
        data_masked = np.select(
            [input_data == 0, input_data == 1, input_data == 3, input_data == 4],
            choices,
            default=out_data_int
        ).astype(np.uint8, casting='unsafe')

        # Save Final Geotiff with Palette and Tags
        weblink = "https://forest.jrc.ec.europa.eu/en/activities/lpa/gtb/"
        tag_descr = f"GTB_FOS, <Binary,-1,8,{method}_5,{info['resX']},{window_size}>, {weblink}"
        cmap_path = TEMPL_DIR / "frag_colormap.txt"
        out_tiff = outdir / f"{out_name}.tif"
        utils.save_output_geotiff(out_tiff, data_masked, info['profile'], cmap_path, tag_descr)

        # Statistics and Reporting
        stats_dict = None
        if statists:
            frag_pxl_freq = utils.get_pxl_freq(data_masked)
            stats_dict = frag_stats(frag_tiff = out_tiff,
                                    outfile = stat_files,
                                    outdir = outdir,
                                    source_tiff = in_tiff,
                                    frag_freq = frag_pxl_freq)

        # Computational time
        time_str = utils.running_time(start_time, time.time())
        if verb:
            print(f"\nFragmentation completed in {time_str}")
        if statists:
            txt_file = txt_file = outdir / f'{out_name}.txt'
            utils.update_time_line(txt_file, time_str)

        # Success of the process
        success = True

        return stats_dict

    except Exception as e:
        print(f"Error during run: {e}")
        raise # Still show the error

    finally:
        if success:
            shutil.rmtree(tmpdir) # Only delete if it worked
        else:
            print(f"Debug: Files preserved in {tmpdir}")


def frag_stats(frag_tiff, outfile = True, outdir = None, source_tiff=None, frag_freq=None):
    """
    Computes statistics for an existing Fragmentation result GeoTIFF. Can be
    called independently on a previously generated fragmentation output, or
    is invoked automatically by frag() when statists=True.

    Parameters
    ----------
    frag_tiff : str or Path
        Path to the fragmentation result GeoTIFF. Must contain a valid
        GTB_FOS metadata tag in the TIFFTAG_IMAGEDESCRIPTION field.
    outfile : bool, optional
        If True (default), writes statistics to .txt, .csv and .png files.
    outdir : str or Path, optional
        Directory for output files. Defaults to the input file's directory.
    source_tiff : str or Path, optional
        Path to the original input GeoTIFF used to generate the fragmentation
        result. Used only to report the source filename in the statistics
        report. Default None.
    frag_freq : Counter, optional
        Pre-computed pixel frequency Counter for the fragmentation result
        array. If provided, skips reading the GeoTIFF for pixel counting.
        Passed internally by frag() to avoid redundant disk reads.
        Default None.

    Returns
    -------
    dict
        Nested dictionary with three keys:
        - 'output paths' (dict or None): paths to generated output files
          ('path tif', 'path txt', 'path csv', 'path png'),
          or None if outfile=False.
        - 'input stats' (dict): pixel counts for foreground, background,
          missing and special class pixels.
        - 'output stats' (dict): per-class pixel counts for fragmentation
          classes (rare, patchy, transitional, dominant, interior) and
          fragmentation indices (FAD_av, AVcon).

        Note: 'output paths' is None when outfile=False. All other keys
        are always populated regardless of outfile.

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

    # Check input tag with used tool and parametres
    tool_params = utils.get_tool_parameters(minfo["tag"])
    if tool_params.get("tool_id") != "GTB_FOS":
        sys.exit(f"ERROR: Input Geotiff is labeled as '{tool_params.get('tool_id')}', "
                 "frag_stats requires a 'GTB_FOS' result file."
        )

    # Get Fragmentation parameters
    method,fclasses = tool_params["method"].split('_')
    window_size = int(tool_params["wsize"])

    # Define input and output file names
    out_name = Path(frag_tiff).stem
    outdir = Path(outdir) if outdir else frag_tiff.parent
    source_tiff = Path(source_tiff) if source_tiff else None

    # Fragmentation pixel counting
    if frag_freq:
        frag_pxl_freq = frag_freq
    else:
        with rasterio.open(frag_tiff) as src:
            frag_data = src.read()
        frag_pxl_freq = utils.get_pxl_freq(frag_data)

    # Counting pixel per Frag class
    rare = sum(frag_pxl_freq[i] for i in range(10))
    patchy = sum(frag_pxl_freq[i] for i in range(10, 40))
    trans = sum(frag_pxl_freq[i] for i in range(40, 60))
    domin = sum(frag_pxl_freq[i] for i in range(60, 90))
    inter = sum(frag_pxl_freq[i] for i in range(90, 101))

    fgrnd = rare + patchy + trans + domin + inter
    ruarea = fgrnd + frag_pxl_freq[101]
    sum_prod = sum(v * frag_pxl_freq[v] for v in range(101))

    fad_av = sum_prod / fgrnd
    avcon = sum_prod / ruarea

    if outfile:

        ### CSV Export ###
        csv_file = outdir / f'{out_name}.csv'
        with open(csv_file, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(['pixel_value', 'pixel_count', 'foreground_proportion'])
            for v in range(101):
                pct = (frag_pxl_freq[v] / fgrnd * 100)
                writer.writerow([v, frag_pxl_freq[v], f"{pct:.6f}"])

        ### Histogram PNG figure ###

        # X & Y values
        pixel_values = list(range(101))
        frag_pxl_prop = [frag_pxl_freq[i]/fgrnd * 100 for i in pixel_values]

        # Create the colormap
        cmap_path = TEMPL_DIR / "frag_colormap.txt"
        colors = {}
        with open(cmap_path, 'r') as f:
            for line in f:
                parts = line.split()
                if len(parts) >= 4:
                    val = int(parts[0])
                    # Normalize 0-255 to 0.0-1.0 for Matplotlib
                    r, g, b = int(parts[1])/255, int(parts[2])/255, int(parts[3])/255
                    colors[val] = (r, g, b)
        bar_colors = [colors.get(v) for v in pixel_values]

        # Create the figure with bar chart
        fig, ax = plt.subplots(figsize=(7, 6))
        ax.bar(pixel_values, frag_pxl_prop, color=bar_colors, width=1.0,
                      edgecolor='black', linewidth=0.4)

        # Formatting the Axes
        ax.set_xlabel(method, fontsize=14)
        ax.set_ylabel('Frequency [%]', fontsize=14)
        ax.set_title('Foreground pixel histogram', fontsize=15, pad=20)
        ax.tick_params(axis='both', which='major', labelsize=12)

        # Set X-axis limits and ticks (show every 10 units for clarity)
        ax.set_xlim(-1, 101)
        ax.set_xticks(range(0, 101, 10))

        # Plotting
        plt.tight_layout()
        png_file = outdir / f'{out_name}.png'
        plt.savefig(png_file, dpi=300, facecolor='white', transparent=False)
        plt.close()

        ### TXT Template Reporting ###
        content = {
            "input_file": source_tiff.name if source_tiff else "n/a",
            "epsg_code": minfo["epsg"],
            "unit_type": 'metres' if minfo["is_projected"] else 'degrees',
            "resolx": minfo["resX"],
            "resoly": minfo["resY"],
            "rows_val": minfo["rows"],
            "cols_val": minfo["cols"],
            "tot_pxl": minfo["rows"] * minfo["cols"],
            "foreg_pxl": fgrnd,
            "backg_pxl": frag_pxl_freq[101],
            "miss_pxl": frag_pxl_freq[102],
            "spec3_pxl": frag_pxl_freq[105],
            "spec4_pxl": frag_pxl_freq[106],

            "used_method": method,
            "window_size": window_size,
            "window_areaHA": f"{(window_size**2)*minfo['resX']*minfo['resY']/10000:.4f}" if minfo["is_projected"] else '--',
            "window_areaAC": f"{(window_size**2)*minfo['resX']*minfo['resY']*0.000247105:.4f}" if minfo["is_projected"] else '--',

            "output_file": f"{out_name}.tif",
            "rep_unit_pxl": ruarea,
            "foreg_area_rel": (fgrnd / ruarea * 100),
            "rare_val": (rare / fgrnd * 100),
            "patch_val": (patchy / fgrnd * 100),
            "trans_val": (trans / fgrnd * 100),
            "domin_val": (domin / fgrnd * 100),
            "inter_val": (inter / fgrnd * 100),
            "fad_av_idx": fad_av,
            "avcon_idx": avcon,
            "comp_time": f"{utils.running_time(start_time_stat, time.time())}"
        }

        txt_file = outdir / f'{out_name}.txt'
        utils.generate_text_report(TEMPL_DIR / 'frag_templ.txt', txt_file, content)

    # Statistic dictionaries
    path_stats_dict = None
    if outfile:
        path_stats_dict = {
            "path tif" : str(frag_tiff),
            "path txt" : str(txt_file),
            "path csv" : str(csv_file),
            "path png" : str(png_file)
            }
    input_stats_dict = {
        "foreground pxl" : fgrnd,
        "background pxl" : frag_pxl_freq[101],
        "missing pxl" : frag_pxl_freq[102],
        "backgr3 pxl" : frag_pxl_freq[105],
        "backgr4 pxl" : frag_pxl_freq[106]
        }
    class_freq = {
        "1 rare pxl" : rare,
        "2 patch pxl" : patchy,
        "3 trans pxl" : trans,
        "4 domin pxl" : domin,
        "5 inter pxl" : inter
    }
    output_stats_dict = {
        "class freq" : class_freq,
        "fad_av" : fad_av,
        "avcon" : avcon
        }
    stats_dict = {
        "output paths" : path_stats_dict,
        "input stats" : input_stats_dict,
        "output stats" : output_stats_dict
                  }

    return stats_dict