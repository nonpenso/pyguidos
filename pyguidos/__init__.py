from pathlib import Path
import importlib
import os
import sys
import platform
import warnings
import tempfile
from typing import TYPE_CHECKING

# Internal paths
MODULE_ROOT = Path(__file__).resolve().parent
PROJECT_ROOT = MODULE_ROOT.parent
TEMPL_DIR = MODULE_ROOT / "templates"
DATA_DIR = MODULE_ROOT / "data"


# Package metadata
__version__ = "2.6.5"
__author__ = "Caudullo G., Vogt P., Soille P., European Commission, Joint Research Centre"

# Global Numba Setup
def _threading_priority(os_name):
    """
    Numba threading-layer priority for an OS (platform.system() value).
    Numba requires a permutation of ALL three layers; the first available
    one is used, so unavailable layers are skipped automatically.
    """
    if os_name == "Linux":
        return "omp tbb workqueue"
    if os_name == "Darwin":
        return "workqueue omp tbb"      # macOS: built-in workqueue first
    return "tbb omp workqueue"          # Windows: prefer TBB if available


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
    # Must be set BEFORE numba is imported (numba reads env vars at import time).
    # A priority list falls back gracefully if a layer is unavailable; user-set
    # values are respected.
    if "NUMBA_THREADING_LAYER" not in os.environ:
        os.environ.setdefault("NUMBA_THREADING_LAYER_PRIORITY",
                              _threading_priority(curr_os))

    # --- 3. GLOBAL CONFIGS ---
    warnings.filterwarnings("ignore", message=".*TBB threading layer.*")

    # --- 4. CACHE TO TEMP CONFIGS ---
    # Passed as an environment variable so that numba itself is NOT imported
    # here (it is imported lazily, with the first tool that needs it).
    # A user-defined NUMBA_CACHE_DIR is respected.
    cache_path = os.path.join(tempfile.gettempdir(), "numba_spatcon_cache")
    if not os.path.exists(cache_path):
        try:
            os.makedirs(cache_path)
        except OSError:
            cache_path = os.path.join(os.getcwd(), ".numba_cache")
    os.environ.setdefault("NUMBA_CACHE_DIR", cache_path)

    # If numba was already imported before pyguidos, the environment
    # variables come too late: apply the cache directory directly.
    if "numba" in sys.modules:
        from numba import config
        config.CACHE_DIR = os.environ["NUMBA_CACHE_DIR"]

# Execute setup upon import (cheap: environment variables only)
_setup_numba()


# ================================================================================
# Lazy loading of tools (PEP 562)
# The analysis modules and their heavy dependencies (numba, rasterio, scipy,
# matplotlib, geopandas, ...) are imported only when a function is first
# used, so `import pyguidos` stays fast. `pg.frag(...)`,
# `from pyguidos import frag` and `from pyguidos import *` work as before.

# public name -> submodule that defines it
_LAZY_FUNCTIONS = {
    "frag": "fragmentation", "frag_stats": "fragmentation",
    "frag_change": "fragmentation_change",
    "frag_gray": "fragmentation_gray", "frag_gray_stats": "fragmentation_gray",
    "landmos": "land_mosaic", "landmos_stats": "land_mosaic",
    "spa": "spa", "spa_stats": "spa",
    "mspa": "mspa", "mspa_stats": "mspa",
    "acc": "accounting", "acc_stats": "accounting",
    "rss": "rss",
    "extract_by_polygon": "extract_by_polygon",
    "citation": "utils",
}
# submodules reachable as attributes (e.g. pg.utils.get_raster_info)
_LAZY_SUBMODULES = {"utils", "checks", "engine"}


def __getattr__(name):
    if name in _LAZY_FUNCTIONS:
        module = importlib.import_module(f".{_LAZY_FUNCTIONS[name]}", __name__)
        value = getattr(module, name)
    elif name in _LAZY_SUBMODULES:
        value = importlib.import_module(f".{name}", __name__)
    else:
        raise AttributeError(f"module {__name__!r} has no attribute {name!r}")
    globals()[name] = value  # cache: later accesses skip __getattr__
    return value


def __dir__():
    return sorted(set(globals()) | set(_LAZY_FUNCTIONS) | _LAZY_SUBMODULES)


if TYPE_CHECKING:  # static analysis / IDE autocompletion only, no runtime cost
    from .fragmentation import frag, frag_stats
    from .fragmentation_change import frag_change
    from .fragmentation_gray import frag_gray, frag_gray_stats
    from .land_mosaic import landmos, landmos_stats
    from .spa import spa, spa_stats
    from .mspa import mspa, mspa_stats
    from .accounting import acc, acc_stats
    from .rss import rss
    from .extract_by_polygon import extract_by_polygon
    from . import utils, checks, engine
    from .utils import citation


def info(tool: str = None):
    """
    Displays quick-help and JRC documentation links for pyguidos tools.

    Usage:
        pg.info()          # Lists all available tools
        pg.info('acc')     # Shows details and documentation links for Accounting
    """
    registry = {
        "spa": {
            "title": "Simplified Pattern Analysis (SPA)",
            "desc": "Classifies binary maps into mutually exclusive morphological classes "
                    "(Core, Edge, Islet, Loop, Perforation, Linear).",
            "guide": "https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/usage/morph_spa.html",
            "sheet": "https://forest.jrc.ec.europa.eu/en/activities/lpa/mspa/"
        },
        "mspa": {
            "title": "Morphological Spatial Pattern Analysis (MSPA)",
            "desc": "Full morphological segmentation of binary patterns into core, islet, "
                    "edge, perforation, bridge, loop, branch and their variants (miallib).",
            "guide": "https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/usage/morph_mspa.html",
            "sheet": "https://forest.jrc.ec.europa.eu/en/activities/lpa/mspa/"
        },
        "frag": {
            "title": "Fragmentation",
            "desc": "Calculates the Fragmentation with Fixed Observation Scale (FOS) approach.",
            "guide": "https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/usage/fragmentation.html",
            "sheet": "https://ies-ows.jrc.ec.europa.eu/gtb/GTB/psheets/GTB-Fragmentation-FADFOS.pdf"
        },
        "frag_change": {
            "title": "Fragmentation change",
            "desc": "Calculates the change of two Fragmentation output tiff files.",
            "guide": "https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/usage/fragmentation_change.html",
            "sheet": "https://ies-ows.jrc.ec.europa.eu/gtb/GTB/psheets/GTB-Fragmentation-FADFOS.pdf"
        },
        "frag_gray": {
            "title": "Grayscale Fragmentation",
            "desc": "Fragmentation analysis on continuous-value rasters (e.g., tree cover density 0-100%).",
            "guide": "https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/usage/fragmentation_gray.html",
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
        print("\n" + "="*60)
        print(" pyguidos: Available Analytical Tools")
        print("="*60)
        for name, data in registry.items():
            print(f" * {name:8} : {data['title']}")
        print("\nType pg.info('tool_name') for detailed links.")
        print("="*60 + "\n")
        return

    tool = tool.lower()
    if tool in registry:
        t = registry[tool]
        # Get the function object (lazy-loads its module on first use)
        func = getattr(sys.modules[__name__], tool, None)

        print(f"\n{'-'*10} {t['title'].upper()} {'-'*10}")
        print(f"Description:  {t['desc']}")
        print(f"User Guide:   {t['guide']}")
        print(f"Method Sheet: {t['sheet']}")

        if func:
            import inspect
            # This shows the exact arguments: e.g., mspa(in_tiff, foreground=2, ...)
            signature = inspect.signature(func)
            print(f"Usage:        pg.{tool}{signature}")

        print(f"\nFull usage: help(pg.{tool})\n")

# Exported names
__all__ = [
           "spa", "spa_stats",
           "mspa", "mspa_stats",
           "frag", "frag_stats", "frag_change", "frag_gray", "frag_gray_stats",
           "landmos", "landmos_stats",
           "acc", "acc_stats",
           "rss",
           "extract_by_polygon",
           "citation", "info"
           ]
