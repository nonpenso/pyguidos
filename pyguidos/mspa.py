import sys
import time
from pathlib import Path
import warnings
import shutil

import rasterio

from . import utils
from . import checks
from . import engine
from .results import MSPAResult


def mspa(in_tiff,
         edge_width,
         connectivity=8,
         transition=True,
         int_ext=True,
         outdir=None,
         statists=True,
         stat_files=True,
         return_array=False,
         verb=False):
    """
    Performs Morphological Spatial Pattern Analysis (MSPA) on a binary raster.
    MSPA classifies foreground pixels into structural categories such as Core,
    Edge, Perforation, Islet, Branch, Loop and Bridge, based on their spatial
    context within the landscape.

    Parameters
    ----------
    in_tiff : str or Path
        Path to the input GeoTIFF. Must be uint8 with values:
        0 = NoData, 1 = Background, 2 = Foreground.
    edge_width : int
        Width of the edge zone in pixels. Must be >= 1.
    connectivity : int, optional
        Pixel connectivity for foreground analysis. Must be 4 or 8 (default).
    transition : bool, optional
        If True (default), enables transition zones between MSPA classes.
    int_ext : bool, optional
        If True (default), distinguishes internal and external MSPA subclasses.
    outdir : str or Path, optional
        Directory for output files. Defaults to the input file's directory.
    statists : bool, optional
        If True (default), computes and returns statistics.
    stat_files : bool, optional
        If True (default), writes statistics to a .txt report file.
    return_array : bool, optional
        If True, includes the output numpy array in the result. Default False.
    verb : bool, optional
        If True, prints progress messages. Default False.

    Returns
    -------
    MSPAResult
        A dataclass with:
        - stats (dict): nested dictionary with output paths, input stats,
          and per-class pixel counts.
        - array (np.ndarray or None): the MSPA output array if return_array=True.

    Output Files
    ------------
    - <in_name>_mspa_<connectivity>_<edge_width>_<trans>_<i_e>.tif : MSPA result
    - <in_name>_mspa_<connectivity>_<edge_width>_<trans>_<i_e>.txt : statistics report
    """
    start_time = time.time()
    success = False

    # Validate parametres
    checks.validate_mspa_params(edge_width, connectivity)

    # Parametres
    trans = 1 if transition else 0
    i_e = 1 if int_ext else 0

    # Initialize Paths and Metadata
    in_tiff = Path(in_tiff)
    outdir = Path(outdir) if outdir else in_tiff.parent
    in_name = in_tiff.stem
    out_name = in_name + f'_mspa_{connectivity}_{edge_width}_{trans}_{i_e}'
    info = utils.get_raster_info(in_tiff)

    # Read input Geotif
    with rasterio.open(in_tiff) as src:
        input_data = src.read(1)

    # Get the pixel counting
    input_pxl_freq = utils.get_pxl_freq(input_data)

    # Input Geotiff validations
    checks.validate_fmap_input(list(input_pxl_freq.keys()), info["bands"], allow_34=False)

    try:
        # Create temp dir
        tmpdir = utils.setup_run_dir()
        
        # Execute MSPA
        engine.write_mspa_input(tmpdir, in_tiff, input_data, info['dtype'], info['is_tiled'])

        # Execute Binary
        engine.run_mspa(tmpdir, in_tiff, connectivity, edge_width, trans, i_e, verb)
        
        # Read Output
        out_tiff = tmpdir / "mspa_output.tif"
        with warnings.catch_warnings():
            warnings.filterwarnings("ignore", category=rasterio.errors.NotGeoreferencedWarning)
            with rasterio.open(out_tiff) as result_src:
                mspa_data = result_src.read()
                mspa_colormap = result_src.colormap(1)

        # MSPA bug correction: value = 2 to 0 (Background)
        mspa_data[mspa_data == 2] = 0

        # Save Final Geotiff with Palette and Tags
        weblink = 'https://forest.jrc.ec.europa.eu/en/activities/lpa'
        tag_descr = f"GTB_MSPA, <{connectivity},{edge_width},{trans},{i_e}>, {weblink}"
        out_tiff = outdir / f"{out_name}.tif"
        utils.save_output_geotiff(out_tiff, mspa_data, info['profile'], mspa_colormap, tag_descr)

        # Statistics and Reporting
        stats_dict = None
        if statists:
            mspa_pxl_freq = utils.get_pxl_freq(mspa_data)
            stats_dict = mspa_stats(mspa_tiff = out_tiff,
                                    outfile = stat_files,
                                    outdir = outdir,
                                    source_tiff = in_tiff,
                                    mspa_freq = mspa_pxl_freq
                                    )

        # Computational time
        time_str = utils.running_time(start_time, time.time())
        if verb:
            print(f"\nMSPA completed in {time_str}")
        if statists:
            txt_file = outdir / f'{out_name}.txt'
            utils.update_time_line(txt_file, time_str)

        # Success of the process
        success = True

        return MSPAResult(
                    stats = stats_dict,
                    array = mspa_data if return_array else None
                    )

    except Exception as e:
        print(f"Error during run: {e}")
        raise # Still show the error

    finally:
        if success:
            shutil.rmtree(tmpdir) # Only delete if it worked
        else:
            print(f"Debug: Files preserved in {tmpdir}")


def mspa_stats(mspa_tiff, outfile = True, outdir = None, source_tiff=None, mspa_freq=None):
    """
    Computes statistics for an existing MSPA result GeoTIFF. Can be called
    independently on a previously generated MSPA output, or is invoked
    automatically by mspa() when statists=True.

    Parameters
    ----------
    mspa_tiff : str or Path
        Path to the MSPA result GeoTIFF. Must contain a valid GTB_MSPA
        metadata tag in the TIFFTAG_IMAGEDESCRIPTION field.
    outfile : bool, optional
        If True (default), writes statistics to a .txt report file.
    outdir : str or Path, optional
        Directory for output files. Defaults to the input file's directory.
    source_tiff : str or Path, optional
        Path to the original input GeoTIFF used to generate the MSPA result.
        Used only to report the source filename in the statistics report.
        Default None.
    mspa_freq : Counter, optional
        Pre-computed pixel frequency Counter for the MSPA result array.
        If provided, skips reading the GeoTIFF for pixel counting.
        Passed internally by mspa() to avoid redundant disk reads.
        Default None.

    Returns
    -------
    dict
        Nested dictionary with three keys:
        - 'output paths' (dict or None): paths to generated output files
          ('path tif', 'path txt'), or None if outfile=False.
        - 'input stats' (dict): pixel counts for foreground, background
          and missing pixels.
        - 'output stats' (dict): per-class pixel counts for the 7 MSPA
          classes (core, edge, perforation, islet, branch, loop, bridge).

        Note: 'output paths' is None when outfile=False. All other keys
        are always populated regardless of outfile.

    Output Files
    ------------
    - <mspa_tiff_stem>.txt : statistics report
    """

    start_time_stat = time.time()

    # Read metadata
    mspa_tiff = Path(mspa_tiff)
    minfo = utils.get_raster_info(mspa_tiff)
    if minfo["tag"] is None:
        sys.exit("ERROR: No valid GuidosToolbox metadata found in the input Geotiff")

    # Check input tag with used tool and parametres
    tool_params = utils.get_tool_parameters(minfo["tag"])
    if tool_params.get("tool_id") != "GTB_MSPA":
        sys.exit(f"ERROR: Input Geotiff is labeled as '{tool_params.get('tool_id')}', "
            "mspa_stats requires a 'GTB_MSPA' result file."
        )

    # Get MSPA parameters
    connectivity = tool_params["connectivity"]
    edge_width = tool_params["edge_width"]
    trans = tool_params["transition"]
    i_e = tool_params["int_ext"]

    # Define input and output file names
    out_name = Path(mspa_tiff).stem
    outdir = Path(outdir) if outdir else mspa_tiff.parent
    source_tiff = Path(source_tiff) if source_tiff else None

    # MSPA pixel counting
    if mspa_freq:
        mspa_pxl_freq = mspa_freq
    else:
        with rasterio.open(mspa_tiff) as src:
            mspa_data = src.read()
        mspa_pxl_freq = utils.get_pxl_freq(mspa_data)

    # Counting pixel per MSPA class
    cor_e = mspa_pxl_freq[17]
    cor_i = mspa_pxl_freq[117]
    isl_e = mspa_pxl_freq[9]
    isl_i = mspa_pxl_freq[109]
    edg_e = mspa_pxl_freq[3]
    edg_i = mspa_pxl_freq[103]
    prf_e = mspa_pxl_freq[5]
    prf_i = mspa_pxl_freq[105]
    loo_e = mspa_pxl_freq[65]
    loo_i = mspa_pxl_freq[165]
    loE_e = mspa_pxl_freq[67]
    loE_i = mspa_pxl_freq[167]
    loP_e = mspa_pxl_freq[69]
    loP_i = mspa_pxl_freq[169]
    brg_e = mspa_pxl_freq[33]
    brg_i = mspa_pxl_freq[133]
    brE_e = mspa_pxl_freq[35]
    brE_i = mspa_pxl_freq[135]
    brP_e = mspa_pxl_freq[37]
    brP_i = mspa_pxl_freq[137]
    bch_e = mspa_pxl_freq[1]
    bch_i = mspa_pxl_freq[101]

    bgrnd = mspa_pxl_freq[0]
    brd_opn = mspa_pxl_freq[220]
    cor_opn = mspa_pxl_freq[100]

    ndata = mspa_pxl_freq[129]
    fgrnd = (minfo["rows"] * minfo["cols"]) - bgrnd - ndata

    # Counting  pixel of 7 classes and indices
    cor7cl = cor_e + cor_i
    isl7cl = isl_e + isl_i
    loo7cl = loo_e + loo_i
    brg7cl = brg_e + brg_i
    edg7cl = edg_e + edg_i + loE_e + loE_i + brE_e + brE_i
    prf7cl = prf_e + prf_i + loP_e + loP_i + brP_e + brP_i
    bch7cl = bch_e + bch_i

    ifrgr = fgrnd + brd_opn + cor_opn
    contiguous = cor7cl + edg7cl + prf7cl
    base = contiguous + cor_opn
    poros = 100.0 - (contiguous/base * 100.0)

    if outfile:
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
            "backg_pxl": bgrnd,
            "miss_pxl": ndata,

            "connect":connectivity,
            "edge_w":edge_width,
            "trans":trans,
            "InEx":i_e,

            "output_file": f"{out_name}.tif",
            "cor_e_val":cor_e,
            "edg_e_val":edg_e,
            "prf_e_val":prf_e,
            "isl_e_val":isl_e,
            "bch_e_val":bch_e,
            "loo_e_val":loo_e,
            "loE_e_val":loE_e,
            "loP_e_val":loP_e,
            "brg_e_val":brg_e,
            "brE_e_val":brE_e,
            "brP_e_val":brP_e,

            "cor_i_val":cor_i,
            "edg_i_val":edg_i,
            "prf_i_val":prf_i,
            "isl_i_val":isl_i,
            "bch_i_val":bch_i,
            "loo_i_val":loo_i,
            "loE_i_val":loE_i,
            "loP_i_val":loP_i,
            "brg_i_val":brg_i,
            "brE_i_val":brE_i,
            "brP_i_val":brP_i,

            "bgr_val":bgrnd,
            "brd_opn_val":brd_opn,
            "cor_opn_val":cor_opn,
            "ndata_val":ndata,

            "cor_Frel":f'{cor7cl/fgrnd*100:6.2f}',
            "edg_Frel":f'{edg7cl/fgrnd*100:6.2f}',
            "prf_Frel":f'{prf7cl/fgrnd*100:6.2f}',
            "isl_Frel":f'{isl7cl/fgrnd*100:6.2f}',
            "brg_Frel":f'{brg7cl/fgrnd*100:6.2f}',
            "loo_Frel":f'{loo7cl/fgrnd*100:6.2f}',
            "bch_Frel":f'{bch7cl/fgrnd*100:6.2f}',

            "int_frgr": ifrgr,
            "poros_val":f'{poros:.4f}',
            "comp_time": f"{utils.running_time(start_time_stat, time.time())}"
        }

        txt_file = outdir / f'{out_name}.txt'
        utils.generate_text_report(utils.TEMPL_DIR / 'mspa_templ.txt', txt_file, content)

    # Statistic dictionaries
    path_stats_dict = None
    if outfile:
        path_stats_dict = {
            "path tif" : str(mspa_tiff),
            "path txt" : str(txt_file)
            }
    input_stats_dict = {
        "foreground pxl" : fgrnd,
        "background pxl" : bgrnd,
        "missing pxl" : ndata
        }
    class_freq = {
        "1 core pxl" : cor7cl,
        "2 edge pxl" : edg7cl,
        "3 perforation pxl" : prf7cl,
        "4 islet pxl" : isl7cl,
        "5 branch pxl" : bch7cl,
        "6 loop pxl" : loo7cl,
        "7 bridge pxl" : brg7cl
        }
    output_stats_dict = {
        "class freq" : class_freq,
        "integral foregr" : ifrgr,
        "porosity" : poros
        }
    stats_dict = {
        "output paths" : path_stats_dict,
        "input stats" : input_stats_dict,
        "output stats" : output_stats_dict
                  }

    return stats_dict