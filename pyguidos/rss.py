import time
from pathlib import Path

import rasterio
import numpy as np

from . import utils
from . import engine
from . import checks
from .results import RssResult
from . import TEMPL_DIR


def rss(
    in_tiff,
    outdir=None,
    stat_files=True,
    verb=False
    ):
    """
    Performs Restoration Status Summary (RSS) analysis on a binary or
    multi-class raster. Computes patch-based habitat network indices
    including Degree of Coherence (COH) and Restoration Potential (RPOT),
    providing a summary of habitat network status based on patch size
    distribution.

    Parameters
    ----------
    in_tiff : str or Path
        Path to the input GeoTIFF. Must be uint8 with values:
        0 = NoData, 1 = Background, 2 = Foreground.
        Optional: 3 = Background class 2, 4 = Background class 3.
    outdir : str or Path, optional
        Directory for output files. Defaults to the input file's directory.
    stat_files : bool, optional
        If True (default), writes statistics to a .txt report file.
    verb : bool, optional
        If True, prints progress messages. Default False.

    Returns
    -------
    RssResult
        A dataclass with:
        - stats (dict): nested dictionary with output paths, input stats,
          and connectivity indices (ECA, COH, CNOA, RPOT, RAC).

    Output Files
    ------------
    - <in_name>_rss.txt : statistics report
    """
    start_time = time.time()

    # Initialize Paths and Metadata
    in_tiff = Path(in_tiff)
    outdir = Path(outdir) if outdir else in_tiff.parent
    in_name = in_tiff.stem
    out_name = f"{in_name}_rss"
    info = utils.get_raster_info(in_tiff)

    # Read the input Geotiff
    with rasterio.open(in_tiff) as src:
        input_data = src.read(1)

    # Get the pixel counting
    input_pxl_freq = utils.get_pxl_freq(input_data)

    # Input Geotiff validations
    checks.validate_fmap_input(list(input_pxl_freq.keys()), info["bands"], info['dtype'], allow_34=True)

    # Get patch size frequencies
    labeled_array, lab_pxl_freq = engine.labelling_array(input_data, 2)

    # Counting pixel per input class
    fgrnd = input_pxl_freq.get(2, 0)
    bgrnd = input_pxl_freq.get(1, 0)
    bgr3 = input_pxl_freq.get(3, 0)
    bgr4 = input_pxl_freq.get(4, 0)
    ndata = input_pxl_freq.get(0, 0)

    # Counting output pixels per patch
    patch_sizes = [count for pid, count in lab_pxl_freq.items() if pid > 0]

    # Computing indicies
    tot_pch = len(patch_sizes)
    avg_size = np.mean(patch_sizes)
    median_size = np.median(patch_sizes)
    largest_size = np.max(patch_sizes)

    sizes_array = np.array(patch_sizes)
    RAC = fgrnd / (fgrnd + bgrnd) * 100
    ECA = np.sqrt(np.sum(sizes_array**2))
    COH = (ECA / fgrnd) * 100
    CNOA = 1.0 + ((2.0 * fgrnd * ECA**2)/(fgrnd**2 - ECA**2))
    RPOT = 100 - COH

    if stat_files:

        ### TXT Template Reporting ###
        content = {
            "input_file": in_tiff.name,
            "epsg_code": info["epsg"],
            "unit_type": 'metres' if info["is_projected"] else 'degrees',
            "resolx": info["resX"],
            "resoly": info["resY"],
            "rows_val": info["rows"],
            "cols_val": info["cols"],
            "tot_pxl": info["rows"] * info["cols"],
            "foreg_pxl": fgrnd,
            "backg_pxl": bgrnd,
            "miss_pxl": ndata,
            "spec3_pxl": bgr3,
            "spec4_pxl": bgr4,

            "output_file": f'{out_name}.tif',

            "tot_pch": tot_pch,
            "avg_pch": f"{avg_size:.1f}",
            "med_pch": median_size,
            "lar_pch": largest_size,

            "CNOA_val": int(round(CNOA)),
            "ECA_val": int(round(ECA)),
            "RAC_val": f"{RAC:.2f}",
            "COH_val": f"{COH:.2f}",
            "RPOT_val":  f"{RPOT:.2f}",

            "comp_time": f"{utils.running_time(start_time, time.time())}"
        }

        txt_file = outdir / f'{out_name}.txt'
        utils.generate_text_report(TEMPL_DIR / 'rss_templ.txt', txt_file, content)

    # Statistic dictionaries
    path_stats_dict = None
    if stat_files:
        path_stats_dict = {
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
        "total patches": tot_pch,
        "average patch size": avg_size,
        "median patch size": median_size,
        "largest patch size": largest_size,
        "CNOA": CNOA,
        "ECA": ECA,
        "RAC": RAC,
        "COH": COH,
        "REST_POT": RPOT
        }
    stats_dict = {
        "output paths" : path_stats_dict,
        "input stats" : input_stats_dict,
        "output stats" : output_stats_dict
                  }

    # Completed
    if verb:
        print(fr"\RSS completed in {utils.running_time(start_time, time.time())}")

    return RssResult(stats=stats_dict)

