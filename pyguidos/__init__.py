from pathlib import Path
import subprocess
import os
import sys
import inspect

# Internal paths
MODULE_ROOT = Path(__file__).resolve().parent
PROJECT_ROOT = MODULE_ROOT.parent

PROGS_DIR = MODULE_ROOT / "progs"
TEMPL_DIR = MODULE_ROOT / "templates"
DATA_DIR = MODULE_ROOT / "data"
GLOBAL_CONFIG = Path.home() / ".pyguidos_config"

# Package metadata
__version__ = "2.0.0"
__author__ = "Caudullo G. & Vogt P., European Commission, Joint Research Centre"

# Workspace discovery
def _test_execution(path: Path) -> bool:
    """Tests if a directory allows writing and executing files."""
    # Use a generic filename for the test
    test_file = path / "pyguidos_exec_test"
    try:
        path.mkdir(parents=True, exist_ok=True)
        
        # Write a simple cross-platform script
        if os.name == 'nt':
            test_file = test_file.with_suffix(".bat")
            test_file.write_text("@echo off\necho success")
        else:
            # Linux/Mac script
            test_file.write_text("#!/bin/sh\necho success")
            os.chmod(test_file, 0o755) # Add executable permission
        
        # Try to run it. shell=True helps with .bat and scripts
        result = subprocess.run([str(test_file)], capture_output=True, shell=True, timeout=3)
        success = (result.returncode == 0 and "success" in result.stdout.decode().lower())
        
        if test_file.exists():
            test_file.unlink()
            
        return success
    except Exception:
        return False


def get_workspace():
    # Priority 1: Existing Config
    if GLOBAL_CONFIG.exists():
        conf_path = Path(GLOBAL_CONFIG.read_text(encoding="utf-8").strip())
        if _test_execution(conf_path):
            return conf_path

    # Priority 2: Developer Mode (Git Clone)
    if (PROJECT_ROOT / ".git").exists():
        dev_work = PROJECT_ROOT / "work"
        if _test_execution(dev_work):
            return dev_work

    # Priority 3: Default Home Directory
    home_work = Path.home() / "pyguidos_work"
    if _test_execution(home_work):
        return home_work

    # --- THE FALLBACK (INTERACTIVE ONLY) ---
    
    # Check if we are in an interactive terminal (CLI or Notebook)
    # This prevents the script from hanging in automated/server environments
    if not sys.stdin.isatty():
        raise PermissionError(
            "pyguidos: Execution is blocked in standard folders (Home/Temp) "
            "and no interactive terminal was found to ask for a custom path. "
            "Please manually create a '.pyguidos_config' file in your home directory "
            "containing a valid, writable path."
        )

    print("\n" + "="*60)
    print(" pyguidos: ACTION REQUIRED ")
    print("="*60)
    print("Your current environment prevents running binaries in standard folders.")
    print("This is common in restricted corporate or high-security systems.")
    
    while True:
        user_path = input("\nPlease paste a path with EXECUTION permissions: ").strip()
        if not user_path:
            continue
            
        candidate = Path(user_path).resolve()
        if _test_execution(candidate):
            GLOBAL_CONFIG.write_text(str(candidate), encoding="utf-8")
            print(f"Path validated and saved to {GLOBAL_CONFIG}")
            return candidate
        else:
            print(f"Execution still blocked in {candidate}.")
            print("   Please ensure the path is writable and not mounted with 'noexec'.")

# This runs once when 'import pyguidos' is called
WORK_DIR = get_workspace()

# Import Tools and Results
from .mspa import mspa, mspa_stats
from .fragmentation import frag, frag_stats
from .land_mosaic import landmos, landmos_stats
from .accounting import acc, acc_stats
from .rss import rss
from .extract_by_polygon import extract_by_polygon
from .results import MSPAResult, FragResult, LandMosResult, AccResult, RssResult
from .utils import citation


def info(tool: str = None):
    """
    Displays quick-help and JRC documentation links for pyguidos tools.
    
    Usage:
        pg.info()           # Lists all available tools
        pg.info('mspa')     # Shows details and documentation links for MSPA
    """
    registry = {
        "mspa": {
            "title": "Morphological Spatial Pattern Analysis (MSPA)",
            "desc": "Classifies binary maps into mutually exclusive morphological classes "
                    "(Core, Edge, Islet, Loop, Perforation, Branch).",
            "guide": "https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/usage/mspa.html",
            "sheet": "https://forest.jrc.ec.europa.eu/en/activities/lpa/mspa/"
        },
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
__all__ = ["mspa", "mspa_stats", 
           "frag", "frag_stats",
           "landmos", "landmos_stats",
           "acc", "acc_stats",
           "rss", 
           "extract_by_polygon", 
           "MSPAResult", "FragResult", 
           "LandMosResult", "AccResult",
           "RssResult", 
           "citation", "info"
           ]