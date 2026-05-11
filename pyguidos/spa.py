import sys
import time
from pathlib import Path

import rasterio

from . import engine
from . import utils
from . import checks
from . import TEMPL_DIR


def spa(in_tiff,
         edge_width,
         classes=6,
         outdir=None,
         statists=True,
         stat_files=True,
         verb=False):
    """
    Performs the Simplified Pattern Analysis (SPA) on a binary raster, a 
    streammargd version of the MSPA approach. SPA classifies foreground pixels 
    into structural categories based on their spatial context, offering 
    four different classification levels (2, 3, 5, or 6 classes).

    Parameters
    ----------
    in_tiff : str or Path
        Path to the input GeoTIFF. Must be uint8 with values:
        0 = NoData, 1 = Background, 2 = Foreground.
    edge_width : int= 6 (default)
        Width of the edge zone in pixels. Must be >= 1.
    classes : int
        Define the number of morphological classes in the output.
        2 = Contiguous (17) and Margin (1)
        3 = Core (17), Margin (1), Core Opening (100)
        5 = Core (17), Edge (3), Perforation (5), Margin (1), Core Opening (100)
        6 = Core (17), Edge (3), Perforation (5), Margin (1), Islet (9)
            Core Opening (100)
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
        - 'input stats' (dict): pixel counts for foreground, background
          and missing pixels.
        - 'output stats' (dict): per-class pixel counts for the MSPA classes.

    Output Files
    ------------
    - <in_name>_mspa_<connectivity>_<edge_width>_<trans>_<i_e>.tif : SPA result
    - <in_name>_mspa_<connectivity>_<edge_width>_<trans>_<i_e>.txt : statistics report
    """
    start_time = time.time()
    
    # Log
    utils.log_msg(verb, "[   START   ]  Verifying input raster...")

    # Validate parametres
    checks.validate_spa_params(edge_width, classes)

    # Initialize Paths and Metadata
    in_tiff = Path(in_tiff)
    outdir = Path(outdir) if outdir else in_tiff.parent
    in_name = in_tiff.stem
    out_name = in_name + f'_spa_{edge_width}_{classes}'
    info = utils.get_raster_info(in_tiff)

    # Read input Geotif
    with rasterio.open(in_tiff) as src:
        input_data = src.read(1)

    # Get the pixel counting
    input_pxl_freq = utils.get_pxl_freq(input_data)

    # Input Geotiff validations
    checks.validate_fmap_input(list(input_pxl_freq.keys()), info["bands"], info['dtype'], allow_34=False)
    
    # Log
    utils.log_msg(verb, "[    OK     ]  Input raster verified.")

    try:
        # Log
        utils.log_msg(verb, "[   START   ]  Computing SPA...")
        
        # Compute SPA
        spa_array = engine.compute_spa(input_data, edge_width, classes)
        
        # Log
        utils.log_msg(verb, "[    OK     ]  SPA computed.")
        utils.log_msg(verb, "[   START   ]  Generating statistics and saving GeoTIFF...")  

        # Save Final Geotiff with Palette and Tags
        weblink = 'https://forest.jrc.ec.europa.eu/en/activities/lpa'
        tag_descr = f"GTB_SPA, <{edge_width},{classes}>, {weblink}"
        cmap_path = TEMPL_DIR / "mspa_colormap.txt"
        out_tiff = outdir / f"{out_name}.tif"
        utils.save_output_geotiff(out_tiff, spa_array, info['profile'], cmap_path, tag_descr)

        # Statistics and Reporting
        stats_dict = None
        if statists:
            minfo = utils.get_raster_info(out_tiff)
            spa_pxl_freq = utils.get_pxl_freq(spa_array)
            stats_dict = _get_spa_stats(spa_freq = spa_pxl_freq,
                                        tiff_info = minfo,
                                        outfile = stat_files,
                                        out_name=out_name, 
                                        out_dir = outdir,
                                        source_tiff = in_tiff)

        # Computational time and log
        time_str = utils.running_time(start_time, time.time())
        utils.log_msg(verb, "[    OK     ]  Statistics complete and files saved.") 
        utils.log_msg(verb, f"\n>>> SPA task finished in {time_str}") 
        
        if statists:
            txt_file = outdir / f'{out_name}.txt'
            utils.update_time_line(txt_file, time_str)

        return stats_dict

    except Exception as e:
        print(f"Error during run: {e}")
        raise # Still show the error


def spa_stats(spa_tiff, stat_files = True, outdir = None, source_tiff=None):
    """
    Computes statistics for an existing SPA result GeoTIFF. Can be called
    independently on a previously generated MSPA output, or is invoked
    automatically by spa() when statists=True.

    Parameters
    ----------
    spa_tiff : str or Path
        Path to the SPA result GeoTIFF. Must contain a valid GTB_SPA
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
          ('path tif', 'path txt'), or None if outfile=False.
        - 'input stats' (dict): pixel counts for foreground, background
          and missing pixels.
        - 'output stats' (dict): per-class pixel counts for the SPA classes.

        Note: 'output paths' is None when outfile=False. All other keys
        are always populated regardless of outfile.

    Output Files
    ------------
    - <spa_tiff_stem>.txt : statistics report
    """

    start_time_stat = time.time()

    # Read metadata
    spa_tiff = Path(spa_tiff)
    minfo = utils.get_raster_info(spa_tiff)
    if minfo["tag"] is None:
        sys.exit("ERROR: No valid GuidosToolbox metadata found in the input Geotiff")

    # Check input tag with used tool and parametres
    tool_params = utils.get_tool_parameters(minfo["tag"])
    if tool_params.get("tool_id") != "GTB_SPA":
        sys.exit(f"ERROR: Input Geotiff is labeled as '{tool_params.get('tool_id')}', "
            "spa_stats requires a 'GTB_SPA' result file."
        )

    # Define input and output file names
    out_name = Path(spa_tiff).stem
    outdir = Path(outdir) if outdir else spa_tiff.parent
    source_tiff = Path(source_tiff) if source_tiff else None

    # MSPA pixel counting
    with rasterio.open(spa_tiff) as src:
        spa_data = src.read(1)
    spa_pxl_freq = utils.get_pxl_freq(spa_data)
    
    # Get statistics
    stats_dict = _get_spa_stats(spa_freq = spa_pxl_freq, 
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


def _get_spa_stats(spa_freq, 
                   tiff_info, 
                   outfile=True, 
                   out_name=None, 
                   out_dir=None, 
                   source_tiff=None):
    """
    Get the SPA statistics.    
    """
    
    # Get SPA parameters
    tool_params = utils.get_tool_parameters(tiff_info["tag"])
    edge_width = tool_params["edge_width"]
    classes = tool_params["classes"]
    
    # Define input and output file names
    source_tiff = Path(source_tiff) if source_tiff else None

    # Counting pixel per MSPA class
    core = spa_freq[17]
    isle = spa_freq[9]
    edge = spa_freq[3]
    perf = spa_freq[5]
    marg = spa_freq[1]

    bgrnd = spa_freq[0]
    cor_opn = spa_freq[100]

    ndata = spa_freq[129]
    fgrnd = (tiff_info["rows"] * tiff_info["cols"]) - bgrnd - ndata

    ifrgr = fgrnd + bgrnd + cor_opn
    contiguous = core + edge + perf
    base = contiguous + cor_opn
    poros = 100.0 - (contiguous/base * 100.0)

    if classes == '2':
        out_freq = {"1 Contiguous (17)" : core,
                    "2 Margin (1)" : marg,
                    "3 Background (0)" : bgrnd,
                    "4 Missing (129)" : ndata}
    elif classes == '3':
        out_freq = {"1 Core (17)" : core,
                    "2 Margin (1)" : marg,
                    "3 Core-opening (100)": cor_opn,
                    "4 Background (0)" : bgrnd,
                    "5 Missing (129)" : ndata}
    elif classes == '5':
        out_freq = {"1 Core (17)" : core,
                    "2 Edge (3)" : edge,
                    "3 Perforation (5)" : perf,
                    "4 Margin (1)" : marg,
                    "5 Core-opening (100)": cor_opn,
                    "6 Background (0)" : bgrnd,
                    "7 Missing (129)" : ndata}
    elif classes == '6':
        out_freq = {"1 Core (17)" : core,
                    "2 Edge (3)" : edge,
                    "3 Perforation (5)" : perf,
                    "4 Islet (9)" : isle,
                    "5 Margin (1)" : marg,
                    "6 Core-opening (100)": cor_opn,
                    "7 Background (0)" : bgrnd,
                    "8 Missing (129)" : ndata}  
    
    if outfile:
        ### TXT Template Reporting ###
        
        if classes == '2':
            r1 = f"Contiguous        17       {core:>10}    {core/fgrnd*100:7.2f}"
            r2 = f"Margin             1       {marg:>10}    {marg/fgrnd*100:7.2f}"
            table_body = "\n".join([r1,r2])
        elif classes == '3':
            r1 = f"Core              17       {core:>10}    {core/fgrnd*100:7.2f}"
            r2 = f"Margin             1       {marg:>10}    {marg/fgrnd*100:7.2f}"
            table_body = "\n".join([r1,r2])
        elif classes == '5':
            r1 = f"Core              17       {core:>10}    {core/fgrnd*100:7.2f}"
            r2 = f"Edge               3       {edge:>10}    {edge/fgrnd*100:7.2f}"
            r3 = f"Perforation        5       {perf:>10}    {perf/fgrnd*100:7.2f}"
            r4 = f"Margin             1       {marg:>10}    {marg/fgrnd*100:7.2f}"
            table_body = "\n".join([r1,r2,r3,r4])
        elif classes == '6':
            r1 = f"Core              17       {core:>10}    {core/fgrnd*100:7.2f}"
            r2 = f"Edge               3       {edge:>10}    {edge/fgrnd*100:7.2f}"
            r3 = f"Perforation        5       {perf:>10}    {perf/fgrnd*100:7.2f}"
            r4 = f"Islet              9       {isle:>10}    {isle/fgrnd*100:7.2f}"
            r5 = f"Margin             1       {marg:>10}    {marg/fgrnd*100:7.2f}"
            table_body = "\n".join([r1,r2,r3,r4,r5])
            
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

            "edge_w":edge_width,
            "clas_n":classes,

            "output_file": f"{out_name}.tif",
            "table_rows": table_body,

            "cor_opn_val":cor_opn,
            "bgr_val":bgrnd,
            "ndata_val":ndata,

            "core_rel":f'{core/fgrnd*100:6.2f}',
            "edge_rel":f'{edge/fgrnd*100:6.2f}',
            "perf_rel":f'{perf/fgrnd*100:6.2f}',
            "isle_rel":f'{isle/fgrnd*100:6.2f}',
            "marg_rel":f'{marg/fgrnd*100:6.2f}',

            "int_frgr": ifrgr,
            "poros_val":f'{poros:.4f}',
        }

        txt_file = out_dir / f'{out_name}.txt'
        utils.generate_text_report(TEMPL_DIR / 'spa_templ.txt', txt_file, content)

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
        "missing pxl" : ndata
        }
    output_stats_dict = {
        "class freq" : out_freq,
        "integral foregr" : ifrgr,
        "porosity" : poros
        }
    stats_dict = {
        "output paths" : path_stats_dict,
        "input stats" : input_stats_dict,
        "output stats" : output_stats_dict
                  }

    return stats_dict