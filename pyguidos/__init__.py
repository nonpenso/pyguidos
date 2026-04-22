from pathlib import Path
import os
import sys
import inspect
import platform
import warnings
import tempfile
from numba import config
import pyproj

# Internal paths
MODULE_ROOT = Path(__file__).resolve().parent
PROJECT_ROOT = MODULE_ROOT.parent

#PROGS_DIR = MODULE_ROOT / "progs"
TEMPL_DIR = MODULE_ROOT / "templates"
DATA_DIR = MODULE_ROOT / "data"
#GLOBAL_CONFIG = Path.home() / ".pyguidos_config"

# Package metadata
__version__ = "2.1.0"
__author__ = "Caudullo G. & Vogt P., European Commission, Joint Research Centre"

# Global Numba Setup
def _setup_numba():
    curr_os = platform.system()

    # --- 1. WINDOWS-SPECIFIC DLL FIX ---
    if curr_os == "Windows" and sys.version_info >= (3, 8):
        # 'Library\bin' folder relative to the running Python interpreter
        venv_base = sys.prefix
        paths_to_check = [
            os.path.join(venv_base, "Library", "bin"),  # Conda style
            os.path.join(venv_base, "Scripts"),         # Standard venv style
            os.path.join(venv_base, "bin")              # Some local setups
        ]

        for path in paths_to_check:
            if os.path.exists(path):
                try:
                    os.add_dll_directory(path)
                except Exception:
                    pass # Ignore errors if path is already added or inaccessible

    # --- 2. THREADING LAYER CONFIG ---
    if curr_os == "Linux":
        os.environ['NUMBA_THREADING_LAYER'] = 'omp'
    elif curr_os == "Darwin":
        os.environ['NUMBA_THREADING_LAYER'] = 'workqueue'
    else:
        # On Windows, we prefer TBB if available
        os.environ['NUMBA_THREADING_LAYER'] = 'tbb'

    # --- 3. GLOBAL CONFIGS ---
    warnings.filterwarnings("ignore", message=".*TBB threading layer.*")

     # --- 4. CACHE TO TEMP CONFIGS ---
    cache_path = os.path.join(tempfile.gettempdir(), "numba_spatcon_cache")
    if not os.path.exists(cache_path):
        try:
            os.makedirs(cache_path)
        except OSError:
            cache_path = os.path.join(os.getcwd(), ".numba_cache")

    config.CACHE_DIR = cache_path
    config.RELEASE_GIL = 1

# Execute setup upon import
_setup_numba()

### Prevent PROJ cannot find proj.db
##try:
##    proj_path = pyproj.datadir.get_data_dir()
##    if 'PROJ_LIB' not in os.environ:
##        os.environ['PROJ_LIB'] = proj_path
##except Exception:
##    pass


# ================================================================================
# Import Tools and Results
#from .mspa import mspa, mspa_stats
from .fragmentation import frag, frag_stats
from .land_mosaic import landmos, landmos_stats
from .accounting import acc, acc_stats
from .rss import rss
from .extract_by_polygon import extract_by_polygon
from .utils import citation


def info(tool: str = None):
    """
    Displays quick-help and JRC documentation links for pyguidos tools.

    Usage:
        pg.info()           # Lists all available tools
        pg.info('acc')     # Shows details and documentation links for Accounting
    """
    registry = {
        # "mspa": {
        #     "title": "Morphological Spatial Pattern Analysis (MSPA)",
        #     "desc": "Classifies binary maps into mutually exclusive morphological classes "
        #             "(Core, Edge, Islet, Loop, Perforation, Branch).",
        #     "guide": "https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/usage/mspa.html",
        #     "sheet": "https://forest.jrc.ec.europa.eu/en/activities/lpa/mspa/"
        # },
        "frag": {
            "title": "Fragmentation",
            "desc": "Calculates the Fragmentation with Fixed Observation Scale (FOS) approach.",
            "guide": "https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/usage/fragmentation.html",
            "sheet": "https://ies-ows.jrc.ec.europa.eu/gtb/GTB/psheets/GTB-Fragmentation-FADFOS.pdf"
        },
        "landmos": {
            "title": "Landscape Mosaic",
            "desc": "Tri-modal landscape classification (Agriculture, Natural, Developed).",
            "guide": "https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/usage/landmos.html",
            "sheet": "https://ies-ows.jrc.ec.europa.eu/gtb/GTB/psheets/GTB-Pattern-LM.pdf"
        },
        "acc": {
            "title": "Accounting",
            "desc": "Foreground patch size analysis.",
            "guide": "https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/usage/accounting.html",
            "sheet": "https://ies-ows.jrc.ec.europa.eu/gtb/GTB/psheets/GTB-Objects-Accounting.pdf"
        },
        "rss": {
            "title": "Restoration Status Summary",
            "desc": "Provide patch-based connectivity indices for a binary raster map.",
            "guide": "https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/usage/rss.html",
            "sheet": "https://ies-ows.jrc.ec.europa.eu/gtb/GTB/psheets/GTB-RestorationPlanner.pdf"
        }
    }

    if not tool:
        print("\n" + "═"*60)
        print(" pyguidos: Available Analytical Tools")
        print("═"*60)
        for name, data in registry.items():
            print(f" • {name:8} : {data['title']}")
        print("\nType pg.info('tool_name') for detailed links.")
        print("═"*60 + "\n")
        return

    tool = tool.lower()
    if tool in registry:
        t = registry[tool]
        # Dynamically get the function object from the module
        func = globals().get(tool)

        print(f"\n{'─'*10} {t['title'].upper()} {'─'*10}")
        print(f"Description:  {t['desc']}")
        print(f"User Guide:   {t['guide']}")
        print(f"Method Sheet: {t['sheet']}")

        if func:
            # This shows the exact arguments: e.g., mspa(in_tiff, foreground=2, ...)
            signature = inspect.signature(func)
            print(f"Usage:        pg.{tool}{signature}")

        print(f"\nFull usage: help(pg.{tool})\n")

# Exported names
__all__ = [
           # "mspa", "mspa_stats",
           "frag", "frag_stats",
           "landmos", "landmos_stats",
           "acc", "acc_stats",
           "rss",
           "extract_by_polygon",
           "citation", "info"
           ]
