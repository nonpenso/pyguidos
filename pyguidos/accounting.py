import sys
import time
from pathlib import Path
import collections

import rasterio
import numpy as np

from . import utils
from . import engine
from . import checks
from . import TEMPL_DIR


ACC_VALUES = [103, 33, 65, 1, 9, 17]


def acc(
    in_tiff,
    thresholds,
    outdir=None,
    statists=True,
    stat_files=True,
    verb=False
    ):
    """
    Performs Foreground Patch Size Accounting (ACC) on a binary or
    multi-class raster. Each foreground patch is classified into size
    categories defined by the user-provided thresholds, allowing
    analysis of patch size distribution across the landscape.

    Parameters
    ----------
    in_tiff : str or Path
        Path to the input GeoTIFF. Must be uint8 with values:
        0 = NoData, 1 = Background, 2 = Foreground.
        Optional: 3 = Background class 2, 4 = Background class 3.
    thresholds : list, tuple or np.ndarray
        Sequence of 1 to 5 unique positive integers defining the patch
        size class boundaries in pixels. For example, [10, 100, 1000]
        creates 4 classes: [1-10], [11-100], [101-1000], [>1000].
    outdir : str or Path, optional
        Directory for output files. Defaults to the input file's directory.
    statists : bool, optional
        If True (default), computes and returns statistics.
    stat_files : bool, optional
        If True (default), writes statistics to a .txt report file.
    verb : bool, optional
        If True, prints progress messages. Default False.

    Returns
    -------
    dict
        Nested dictionary with three keys:
        - 'output paths' (dict or None): paths to generated output files
          ('path tif', 'path txt'), or None if outfile=False.
        - 'input stats' (dict): pixel counts for foreground, background,
          missing and special class pixels.
        - 'output stats' (dict): per-class pixel counts and patch counts
          for each accounting size class.

    Output Files
    ------------
    - <in_name>_acc.tif : accounting result
    - <in_name>_acc.txt : statistics report
    """
    start_time = time.time()
    
    # Log
    utils.log_msg(verb, "[   START   ]  Verifying input raster...")

    # Validate parametres
    thresholds = checks.validate_acc_params(thresholds)

    # Initialize Paths and Metadata
    in_tiff = Path(in_tiff)
    outdir = Path(outdir) if outdir else in_tiff.parent
    in_name = in_tiff.stem
    out_name = f"{in_name}_acc"
    info = utils.get_raster_info(in_tiff)

    # Read the input Geotiff
    with rasterio.open(in_tiff) as src:
        input_data = src.read(1).astype(np.int16)

    # Get the pixel counting
    input_pxl_freq = utils.get_pxl_freq(input_data)

    # Input Geotiff validations
    checks.validate_fmap_input(list(input_pxl_freq.keys()), info["bands"], info['dtype'], allow_34=True)
    
    # Log
    utils.log_msg(verb, "[    OK     ]  Input raster verified.")

    try:
        # Log
        utils.log_msg(verb, "[   START   ]  Computing Accounting...")
        
        # Get patch size frequencies
        labeled_array, lab_pxl_freq = engine.labelling_array(input_data, 2)

        # Create a lookup array for high-speed mapping
        max_id = max(lab_pxl_freq.keys())
        lookup = np.zeros(int(max_id) + 1, dtype=np.uint32)

        # Fill the lookup table
        for patch_id, pixel_count in lab_pxl_freq.items():
            # Skip the Background (0)
            if patch_id == 0:
                lookup[patch_id] = 0   # Keep Background as 0
                continue
            lookup[patch_id] = get_class(pixel_count, thresholds)

        # Reclassification
        reclass_array = lookup[labeled_array.astype(np.int32)]
        reclass_array = reclass_array.astype(np.uint8, casting='unsafe')

        # Mapping logic (NoData, Special codes)
        choices = np.array([129, 105, 176], dtype=np.uint8)
        out_array = np.select(
            [input_data == 0, input_data == 3, input_data == 4],
            choices,
            default=reclass_array
        ).astype(np.uint8, casting='unsafe')

        # Log
        utils.log_msg(verb, "[    OK     ]  Accounting computed.")
        utils.log_msg(verb, "[   START   ]  Generating statistics and saving GeoTIFF...")        

        # Save Final Geotiff with Palette and Tags
        thresh_list = ",".join([str(x) for x in thresholds])
        weblink = "https://forest.jrc.ec.europa.eu/en/activities/lpa/"
        tag_descr = f"GTB_ACC, <{thresh_list}>, {weblink}"
        cmap_path = TEMPL_DIR / "acc_colormap.txt"
        out_tiff = outdir / f"{out_name}.tif"
        utils.save_output_geotiff(out_tiff, out_array, info['profile'], cmap_path, tag_descr)
        
        # Statistics and Reporting
        if statists:
            minfo = utils.get_raster_info(out_tiff)
            acc_pxl_freq = utils.get_pxl_freq(out_array)
            stats_dict = _get_acc_stats(acc_freq = acc_pxl_freq, 
                                        lab_freq = lab_pxl_freq, 
                                        tiff_info = minfo,
                                        outfile=stat_files, 
                                        out_name=out_name, 
                                        out_dir=outdir, 
                                        source_tiff=in_tiff)

        # Computational time and log
        time_str = utils.running_time(start_time, time.time())
        utils.log_msg(verb, "[    OK     ]  Statistics complete and files saved.") 
        utils.log_msg(verb, f"\n>>> Accounting task finished in {time_str}") 

        if statists:
            txt_file = outdir / f'{out_name}.txt'
            utils.update_time_line(txt_file, time_str)

        return stats_dict

    except Exception as e:
        print(f"Error during run: {e}")
        raise # Still show the error



def acc_stats(acc_tiff, stat_files=True, outdir=None, source_tiff=None):
    """
    Computes statistics for an existing Accounting GeoTIFF. Can be
    called independently on a previously generated accounting output.

    Parameters
    ----------
    acc_tiff : str or Path
        Path to the accounting result GeoTIFF. Must contain a valid GTB_ACC
        metadata tag in the TIFFTAG_IMAGEDESCRIPTION field.
    stat_files : bool, optional
        If True (default), writes statistics to a .txt report file.
    outdir : str or Path, optional
        Directory for output files. Defaults to the input file's directory.
    source_tiff : str or Path, optional
        Path to the original input GeoTIFF used to generate the accounting
        result. Used only to report the source filename in the statistics
        report. Default None.

    Returns
    -------
    dict
        Nested dictionary with three keys:
        - 'output paths' (dict or None): paths to generated output files
          ('path tif', 'path txt'), or None if outfile=False.
        - 'input stats' (dict): pixel counts for foreground, background,
          missing and special class pixels.
        - 'output stats' (dict): per-class pixel counts and patch counts
          for each accounting size class.

        Note: 'output paths' is None when outfile=False. All other keys
        are always populated regardless of outfile.

    Output Files
    ------------
    - <acc_tiff_stem>.txt : statistics report
    """
    start_time_stat = time.time()

    # Read metadata
    acc_tiff = Path(acc_tiff)
    minfo = utils.get_raster_info(acc_tiff)
    if minfo["tag"] is None:
        sys.exit("ERROR: No valid GuidosToolbox metadata found in the input Geotiff")

    # Check input tag with used tool and parametres
    tool_params = utils.get_tool_parameters(minfo["tag"])
    if tool_params.get("tool_id") != "GTB_ACC":
        sys.exit(f"ERROR: Input Geotiff is labeled as '{tool_params.get('tool_id')}', "
                 "acc_stats requires a 'GTB_ACC' result file."
        )

    # Define input and output file names
    out_name = Path(acc_tiff).stem
    outdir = Path(outdir) if outdir else acc_tiff.parent
    source_tiff = Path(source_tiff) if source_tiff else None

    # Accounting pixel and patch counting
    with rasterio.open(acc_tiff) as src:
        acc_data = src.read(1)
    acc_pxl_freq = utils.get_pxl_freq(acc_data)
    
    # Get statistics
    stats_dict = _get_acc_stats(acc_freq = acc_pxl_freq, 
                                lab_freq = None, 
                                tiff_info = minfo, 
                                outfile = stat_files, 
                                out_name = out_name, 
                                out_dir = outdir, 
                                source_tiff = source_tiff)
    
    # Computational time
    time_str = utils.running_time(start_time_stat, time.time())
    if stat_files:
        txt_file = outdir / f'{out_name}.txt'
        utils.update_time_line(txt_file, time_str)

    return stats_dict


#############

def _get_acc_stats(acc_freq, 
                   tiff_info, 
                   lab_freq=None,
                   outfile=True, 
                   out_name=None, 
                   out_dir=None, 
                   source_tiff=None):
    """
    Get the Accounting statistics.    
    """
    # Get ACC parameters
    tag = tiff_info["tag"]
    tool_params = utils.get_tool_parameters(tag)
    thre_str = tool_params["thresholds"]
    thresholds = [int(t) for t in thre_str]

    # Define input and output file names
    source_tiff = Path(source_tiff) if source_tiff else None

    # Counting pixel per ACC class
    bgrnd = acc_freq[0]
    bgr3 = acc_freq[105]
    bgr4 = acc_freq[176]
    ndata =  acc_freq[129]
    fgrnd = (tiff_info["rows"] * tiff_info["cols"]) - bgrnd - bgr3 - bgr4 - ndata

    # Counting patches and pixels per class
    tot_pch = "-"
    avg_size_txt = "-"
    median_size = "-"
    largest_size = "-"
    class_pch = None
    if lab_freq:
        class_pch = collections.Counter()
        patch_sizes = []
        for patch_id, pixel_count in lab_freq.items():
            if patch_id > 0:
                patch_class = get_class(pixel_count, thresholds)
                class_pch[patch_class] += 1
                patch_sizes.append(pixel_count)
    
        tot_pch = len(patch_sizes)
        avg_size = np.mean(patch_sizes)
        avg_size_txt = f"{avg_size:.1f}"
        median_size = np.median(patch_sizes)
        largest_size = np.max(patch_sizes)

    if outfile:
        ### TXT Template Reporting ###

        color_seq = ["black", "red", "yellow", "orange", "brown", "green"]
        rows_list = []
        num_thr = len(thresholds)

        class_pxl_out = {}
        class_pth_out = {}
        for i in range(num_thr + 1):
            # Determine Class Value and Color from the sequences
            val = ACC_VALUES[i]
            color = color_seq[i]

            # Determine the Size String
            if i == 0:
                size_raw = f"[1-{thresholds[i]}]"
            elif i < num_thr:
                size_raw = f"[{thresholds[i-1]+1}-{thresholds[i]}]"
            else:
                size_raw = f"[>{thresholds[-1]}]"

            size_col = f"{size_raw:<16}"

            # Pixel class count and freq
            p_count = acc_freq[val]
            p_pct = (p_count / fgrnd * 100) if fgrnd > 0 else 0
            class_pxl_out[f'{i+1} {size_raw}'] = p_count

            # Patch class count and freq
            o_count_txt = "        -"
            o_pct_txt = "       -"
            if lab_freq:
                o_count = class_pch.get(val, 0)
                o_pct = (o_count / tot_pch * 100) if tot_pch > 0 else 0
                o_count_txt = f"{o_count:>9}"
                o_pct_txt = f"{o_pct:>8.2f}"
                class_pth_out[f'{i+1} {size_raw}']=o_count

            # Format the row string
            row = (f"{i+1:<6} {val:<8} {color:<7} {size_col} "
                   f"{p_count:>10} {p_pct:>8.2f} {o_count_txt} {o_pct_txt}")
            rows_list.append(row)

        # Combine rows into a single string
        table_body = "\n".join(rows_list)

        content = {
            "input_file": source_tiff.name if source_tiff else "n/a",
            "epsg_code": tiff_info["epsg"],
            "unit_type": 'metres' if tiff_info["is_projected"] else 'degrees',
            "resolx": tiff_info["resX"],
            "resoly": tiff_info["resY"],
            "rows_val": tiff_info["rows"],
            "cols_val": tiff_info["cols"],
            "tot_pxl": tiff_info["rows"] * tiff_info["cols"],
            "foreg_pxl": fgrnd,
            "backg_pxl": bgrnd,
            "miss_pxl": ndata,
            "spec3_pxl": bgr3,
            "spec4_pxl": bgr4,

            "threshold_list": " ".join(thre_str),

            "output_file": f"{out_name}.tif",
            "table_rows": table_body,
            "tot_pch": tot_pch,
            "avg_pch": avg_size_txt,
            "med_pch": median_size,
            "lar_pch": largest_size,
        }

        txt_file = out_dir / f'{out_name}.txt'
        utils.generate_text_report(TEMPL_DIR / 'acc_templ.txt', txt_file, content)

    # Statistic dictionaries
    path_stats_dict = None
    if outfile:
        path_stats_dict = {
            "path tif" : str(out_dir / f"{out_name}.tif"),
            "path txt" : str(txt_file)
            }
    input_stats_dict = {
        "foreground pxl" : fgrnd,
        "background pxl" : bgrnd,
        "missing pxl" : ndata,
        "backgr3 pxl" : bgr3,
        "backgr4 pxl" : bgr4
        }
    output_stats_dict = {
        "class pxl": class_pxl_out,
        "class patch": class_pth_out
        }
    stats_dict = {
        "output paths" : path_stats_dict,
        "input stats" : input_stats_dict,
        "output stats" : output_stats_dict
        }

    return stats_dict


def get_class(count, thresholds):
    """
    Checks count against thresholds.
    """
    for i, t in enumerate(thresholds):
        if count <= t:
            return ACC_VALUES[i]
    # Return the last class
    return ACC_VALUES[len(thresholds)]