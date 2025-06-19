# pyguidos/__init__.py

# By importing functions from functions.py here,
# you make them directly accessible when someone imports 'your_module_name'.
# For example, after 'import your_module_name', you can do 'your_module_name.add(2, 3)'.

from .gwb import gwb_acc, gwb_dist, gwb_frag, gwb_gsc, gwb_lm, gwb_mspa, gwb_parc, gwb_rec, gwb_rss, gwb_sc, gwb_spa

# Import the 'data' submodule to make it accessible directly from the top-level package
# This allows users to do 'from pyguidos import data'
from . import data

# Package version:
__version__ = "0.1.0"


