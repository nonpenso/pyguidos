from .mspa import mspa, mspa_stats
from .fragmentation import frag, frag_stats
from .land_mosaic import landmos, landmos_stats
from .accounting import acc, acc_stats
from .rss import rss
from .extract_by_polygon import extract_by_polygon

from .results import MSPAResult, FragResult, LandMosResult, AccResult, RssResult

# Package metadata
__version__ = "2.0.0"
__author__ = "European Commission, Joint Research Centre"

# from pyguidos import *
__all__ = ["mspa", "mspa_stats", 
           "frag", "frag_stats",
           "landmos", "landmos_stats"
           "acc", "acc_stats"
           "rss", 
           "extract_by_polygon", 
           "MSPAResult", "FragResult", 
           "LandMosResult", "AccResult",
           "RssResult"
           ]

