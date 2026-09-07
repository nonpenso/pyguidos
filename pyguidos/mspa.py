import sys
import time
from pathlib import Path

import numpy as np
import rasterio

from . import utils
from . import checks
from . import TEMPL_DIR

# import lazily mspa.py in case of failing wheel installation
try:
    from ._mspa import _mspa
    _MSPA_IMPORT_ERROR = None
except ImportError as _exc:  # pragma: no cover - only on a broken/source install
    _mspa = None
    _MSPA_IMPORT_ERROR = _exc

_MSPA_MISSING_MSG = (
    "The compiled MSPA extension 'pyguidos._mspa._mspa' is not available for "
    "your platform/Python version, so pg.mspa() cannot run.\n"
    "Normally `pip install pyguidos` installs a prebuilt wheel that already "
    "contains this extension - no compiler is required. This error means no "
    "matching wheel was found and pyGuidos was installed from source without "
    "building the extension.\n"
    "Fixes: (1) upgrade pip and reinstall to fetch a wheel "
    "(`pip install --upgrade pip && pip install --force-reinstall pyguidos`); "
    "(2) use a supported CPython (3.10-3.14) on a 64-bit platform; or "
    "(3) if you must build from source, install a C compiler and reinstall."
)


def _require_mspa():
    """Raise a clear error if the compiled MSPA extension is unavailable."""
    if _mspa is None:
        raise ImportError(_MSPA_MISSING_MSG) from _MSPA_IMPORT_ERROR
    return _mspa


def mspa(in_tiff,
         connectivity=8,
         edge_width=1,
         transition=True,
         intext=True,
         outdir=None,
         statists=True,
         stat_files=True,
         verb=False):
    """
    Performs Morphological Spatial Pattern Analysis (MSPA) on a binary raster.

    MSPA segments the foreground of a binary pattern into mutually exclusive
    morphological classes (core, islet, edge, perforation, bridge, loop,
    branch, and their variants). It is computed by the original miallib C
    implementation of Soille and Vogt, bundled unmodified inside pyGuidos.

    Parameters
    ----------
    in_tiff : str or Path
        Path to the input GeoTIFF. Must be uint8 with values:
        0 = NoData/missing, 1 = Background, 2 = Foreground.
    connectivity : int, optional
        Foreground connectivity, 8 (default) or 4.
    edge_width : int, optional
        Width of the edge/transition zone in pixels (>= 1). Default 1.
    transition : bool, optional
        If True (default), distinguish transition pixels in the output.
    intext : bool, optional
        If True (default), separate internal from external features
        (internal/hole analysis).
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
        - 'output paths' (dict or None): paths to generated output files.
        - 'input stats' (dict): pixel counts for foreground, background
          and missing pixels.
        - 'output stats' (dict): per-class pixel counts for the MSPA classes.

    Output Files
    ------------
    - <in_name>_<connectivity>_<edge_width>_<transition>_<intext>.tif : MSPA result
    """
    start_time = time.time()

    # Fail fast with a clear message if the compiled extension is unavailable,
    # before doing any I/O or validation work.
    _require_mspa()

    # Log
    utils.log_msg(verb, "[   START   ]  Verifying input raster...")

    # Validate parameters
    if connectivity not in (4, 8):
        sys.exit(f"ERROR: connectivity must be 4 or 8 (received {connectivity})")
    if not isinstance(edge_width, int) or edge_width < 1:
        sys.exit(f"ERROR: edge_width must be an integer >= 1 (received {edge_width})")

    # Initialize Paths and Metadata
    in_tiff = Path(in_tiff)
    outdir = Path(outdir) if outdir else in_tiff.parent
    in_name = in_tiff.stem
    trans_i = 1 if transition else 0
    intext_i = 1 if intext else 0
    out_name = f"{in_name}_{connectivity}_{edge_width}_{trans_i}_{intext_i}"
    info = utils.get_raster_info(in_tiff)

    # Read input GeoTIFF
    with rasterio.open(in_tiff) as src:
        input_data = src.read(1)

    # Get the pixel counting
    input_pxl_freq = utils.get_pxl_freq(input_data)

    # Input GeoTIFF validation (binary map: 0=missing, 1=bg, 2=fg)
    checks.validate_fmap_input(list(input_pxl_freq.keys()), info["bands"],
                               info['dtype'], allow_34=False)

    # Downcast to uint8 after validation (values are guaranteed to be 0/1/2).
    input_data = np.ascontiguousarray(input_data, dtype=np.uint8)

    # Log
    utils.log_msg(verb, "[    OK     ]  Input raster verified.")

    try:
        # Log
        utils.log_msg(verb, "[   START   ]  Computing MSPA...")

        # Compute MSPA via the embedded miallib engine.
        # segmentBinaryPatterns(imin, size, graphfg, transition, internal)
        mspa_engine = _require_mspa()
        mspa_array, _ = mspa_engine.mspa(input_data,
                                   float(edge_width),
                                   int(connectivity),
                                   trans_i,
                                   intext_i)

        # Save Final GeoTIFF with palette and tags. 
        weblink = "https://forest.jrc.ec.europa.eu/en/activities/lpa/mspa/"
        tag_descr = (f"GTB_MSPA, <{connectivity},{edge_width},"
                     f"{trans_i},{intext_i}>, {weblink}")
        cmap_name = "mspa_colormap_trans1.txt" if trans_i else "mspa_colormap_trans0.txt"
        cmap_path = TEMPL_DIR / cmap_name
        out_tiff = outdir / f"{out_name}.tif"
        utils.save_output_geotiff(out_tiff, mspa_array, info['profile'],
                                  cmap_path, tag_descr)

        # Log
        utils.log_msg(verb, "[    OK     ]  MSPA computed.")

        # Statistics and Reporting
        stats_dict = None
        if statists:
            # Log
            utils.log_msg(verb, "[   START   ]  Generating statistics and saving GeoTIFF...")

            minfo = utils.get_raster_info(out_tiff)
            mspa_pxl_freq = utils.get_pxl_freq(mspa_array)
            stats_dict = _get_mspa_stats(mspa_freq=mspa_pxl_freq,
                                         tiff_info=minfo,
                                         outfile=stat_files,
                                         out_name=out_name,
                                         out_dir=outdir,
                                         source_tiff=in_tiff)

        # Computational time
        time_str = utils.running_time(start_time, time.time())
        if statists:
            txt_file = outdir / f'{out_name}.txt'
            utils.update_time_line(txt_file, time_str)
            utils.log_msg(verb, "[    OK     ]  Statistics complete and files saved.")

        # Log
        utils.log_msg(verb, f"\n>>> MSPA task finished in {time_str}")

        return stats_dict

    except Exception as e:
        print(f"Error during run: {e}")
        raise


def mspa_stats(mspa_tiff, stat_files=True, outdir=None, source_tiff=None):
    """
    Computes statistics for an existing MSPA result GeoTIFF. Can be called
    independently on a previously generated MSPA output, or is invoked
    automatically by mspa() when statists=True.

    Parameters
    ----------
    mspa_tiff : str or Path
        Path to the MSPA result GeoTIFF. Must contain a valid GTB_MSPA
        metadata tag in the TIFFTAG_IMAGEDESCRIPTION field.
    stat_files : bool, optional
        If True (default), writes statistics to a .txt report file.
    outdir : str or Path, optional
        Directory for output files. Defaults to the input file's directory.
    source_tiff : str or Path, optional
        Path to the original input GeoTIFF used to generate the MSPA result.
        Used only to report the source filename in the statistics report.
        Default None.

    Returns
    -------
    dict
        Nested dictionary with three keys:
        - 'output paths' (dict or None): paths to generated output files
          ('path tif', 'path txt'), or None if stat_files=False.
        - 'input stats' (dict): pixel counts for foreground, background
          and missing pixels.
        - 'output stats' (dict): per-class pixel counts for the MSPA classes.

    Output Files
    ------------
    - <mspa_tiff_stem>.txt : statistics report
    """
    start_time_stat = time.time()

    # Read metadata
    mspa_tiff = Path(mspa_tiff)
    minfo = utils.get_raster_info(mspa_tiff)

    # Not a GuidosToolbox output: no tag, or a tag whose tool id is not GTB_*.
    tool_params = utils.get_tool_parameters(minfo["tag"])
    if tool_params is None:
        sys.exit("ERROR: The input Geotiff is not a GuidosToolbox output "
                 "(no valid GTB metadata found).")

    # A GTB output, but produced by a different tool (e.g. GTB_FOS, GTB_SPA).
    tool_id = tool_params.get("tool_id")
    if tool_id != "GTB_MSPA":
        sys.exit(f"ERROR: The input Geotiff is a '{tool_id}' output, "
                 "mspa_stats requires a 'GTB_MSPA' result file.")

    # Define input and output file names
    out_name = Path(mspa_tiff).stem
    outdir = Path(outdir) if outdir else mspa_tiff.parent
    source_tiff = Path(source_tiff) if source_tiff else None

    # MSPA pixel counting
    with rasterio.open(mspa_tiff) as src:
        mspa_data = src.read(1)
    mspa_pxl_freq = utils.get_pxl_freq(mspa_data)

    # Get statistics
    stats_dict = _get_mspa_stats(mspa_freq=mspa_pxl_freq,
                                 tiff_info=minfo,
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


# MSPA output pixel values, grouped by morphological class.
_MSPA_CLASS_VALUES = {
    "Core":        {"ext": [17],  "int": [117]},
    "Edge":        {"ext": [3],   "int": [103]},
    "Perforation": {"ext": [5],   "int": [105]},
    "Islet":       {"ext": [9],   "int": [109]},
    "Branch":      {"ext": [1],   "int": [101]},
    "Loop":        {"ext": [65, 67, 69],  "int": [165, 167, 169]},
    "Bridge":      {"ext": [33, 35, 37],  "int": [133, 135, 137]},
}


def _get_mspa_stats(mspa_freq,
                    tiff_info,
                    outfile=True,
                    out_name=None,
                    out_dir=None,
                    source_tiff=None):
    """
    Builds the MSPA statistics dictionary and, optionally, the .txt report
    from templates/mspa_templ.txt.
    """

    # Get MSPA parameters from the GTB tag
    tool_params = utils.get_tool_parameters(tiff_info["tag"])
    connectivity = tool_params["connectivity"]
    edge_width = tool_params["edge_width"]
    transition = tool_params["transition"]
    int_ext = tool_params["int_ext"]

    source_tiff = Path(source_tiff) if source_tiff else None

    def f(val):
        """Frequency of a single pixel value (0 if absent)."""
        return int(mspa_freq.get(val, 0))

    # Per-value counts for every class in the template
    cor_e, cor_i = f(17), f(117)
    edg_e, edg_i = f(3), f(103)
    prf_e, prf_i = f(5), f(105)
    isl_e, isl_i = f(9), f(109)
    bch_e, bch_i = f(1), f(101)
    loo_e, loE_e, loP_e = f(65), f(67), f(69)
    loo_i, loE_i, loP_i = f(165), f(167), f(169)
    brg_e, brE_e, brP_e = f(33), f(35), f(37)
    brg_i, brE_i, brP_i = f(133), f(135), f(137)

    # Background-group values
    bgr = f(0)
    brd_opn = f(220)
    cor_opn = f(100)
    ndata = f(129)

    # Aggregated foreground totals (external + internal + variants)
    agg = {}
    for name, groups in _MSPA_CLASS_VALUES.items():
        agg[name] = sum(f(v) for v in groups["ext"]) + \
                    sum(f(v) for v in groups["int"])

    # Total foreground = sum of all 22 morphological pattern pixels.
    fgrnd = sum(agg.values())

    # GTB derived indicators
    contiguous = agg["Core"] + agg["Edge"] + agg["Perforation"]
    ifrgr = fgrnd + cor_opn
    base = contiguous + cor_opn
    poros = 100.0 - (contiguous / base * 100.0) if base > 0 else 0.0

    def rel(v):
        """Percentage of the (morphological) foreground area."""
        return (v / fgrnd * 100.0) if fgrnd > 0 else 0.0

    # Full per-class frequency dictionary (pixel value -> count)
    out_freq = {
        "External": {
            "Core (17)": cor_e, "Edge (3)": edg_e, "Perforation (5)": prf_e,
            "Islet (9)": isl_e, "Branch (1)": bch_e,
            "Loop (65)": loo_e, "Loop in Edge (67)": loE_e, "Loop in Perf. (69)": loP_e,
            "Bridge (33)": brg_e, "Bridge in Edge (35)": brE_e, "Bridge in Perf. (37)": brP_e,
        },
        "Internal": {
            "Core (117)": cor_i, "Edge (103)": edg_i, "Perforation (105)": prf_i,
            "Islet (109)": isl_i, "Branch (101)": bch_i,
            "Loop (165)": loo_i, "Loop in Edge (167)": loE_i, "Loop in Perf. (169)": loP_i,
            "Bridge (133)": brg_i, "Bridge in Edge (135)": brE_i, "Bridge in Perf. (137)": brP_i,
        },
        "Background": {
            "Background (0)": bgr, "Border-Opening (220)": brd_opn,
            "Core-Opening (100)": cor_opn, "Missing (129)": ndata,
        },
    }

    if outfile:
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
            "backg_pxl": bgr,
            "miss_pxl": ndata,

            "connect": connectivity,
            "edge_w": edge_width,
            "trans": transition,
            "InEx": int_ext,

            "output_file": f"{out_name}.tif",

            # External class counts
            "cor_e_val": cor_e, "edg_e_val": edg_e, "prf_e_val": prf_e,
            "isl_e_val": isl_e, "bch_e_val": bch_e,
            "loo_e_val": loo_e, "loE_e_val": loE_e, "loP_e_val": loP_e,
            "brg_e_val": brg_e, "brE_e_val": brE_e, "brP_e_val": brP_e,
            # Internal class counts
            "cor_i_val": cor_i, "edg_i_val": edg_i, "prf_i_val": prf_i,
            "isl_i_val": isl_i, "bch_i_val": bch_i,
            "loo_i_val": loo_i, "loE_i_val": loE_i, "loP_i_val": loP_i,
            "brg_i_val": brg_i, "brE_i_val": brE_i, "brP_i_val": brP_i,
            # Background group
            "bgr_val": bgr, "brd_opn_val": brd_opn,
            "cor_opn_val": cor_opn, "ndata_val": ndata,

            # Aggregated foreground percentages
            "cor_Frel": f'{rel(agg["Core"]):6.2f}',
            "edg_Frel": f'{rel(agg["Edge"]):6.2f}',
            "prf_Frel": f'{rel(agg["Perforation"]):6.2f}',
            "isl_Frel": f'{rel(agg["Islet"]):6.2f}',
            "bch_Frel": f'{rel(agg["Branch"]):6.2f}',
            "loo_Frel": f'{rel(agg["Loop"]):6.2f}',
            "brg_Frel": f'{rel(agg["Bridge"]):6.2f}',

            "int_frgr": ifrgr,
            "poros_val": f'{poros:.4f}',

            # Placeholder replaced later by update_time_line()
            "comp_time": "",
        }

        txt_file = out_dir / f'{out_name}.txt'
        utils.generate_text_report(TEMPL_DIR / 'mspa_templ.txt', txt_file, content)

    # Statistic dictionaries
    path_stats_dict = None
    if outfile:
        path_stats_dict = {
            "path tif": str(out_dir / f"{out_name}.tif"),
            "path txt": str(txt_file)
        }
    input_stats_dict = {
        "foreground pxl": fgrnd,
        "background pxl": bgr,
        "missing pxl": ndata
    }
    output_stats_dict = {
        "class freq": out_freq,
        "aggregated foregr": {k: int(v) for k, v in agg.items()},
        "integral foregr": ifrgr,
        "porosity": poros
    }
    stats_dict = {
        "output paths": path_stats_dict,
        "input stats": input_stats_dict,
        "output stats": output_stats_dict
    }

    return stats_dict
