Restoration Status Summary
==========================

Restoration Status Summary (RSS) computes patch-based connectivity indices
for a binary raster map. RSS characterises the spatial structure of the
foreground by analysing the size distribution of individual patches,
providing a set of indices that quantify landscape connectivity and
restoration potential.
Further details about Restoration Status Summary analysis are
available in the `RSS product sheet
<https://ies-ows.jrc.ec.europa.eu/gtb/GTB/psheets/GTB-RestorationPlanner.pdf>`_.

.. note::
    RSS produces a statistics report only; it does not write a classified
    output map. All results are returned in the dictionary and, optionally,
    the ``.txt`` report. ``rss()`` computes its statistics as part of the
    main run; there is no separate standalone ``*_stats()`` function for
    this tool.


Functions
---------

.. py:function:: pyguidos.rss(in_tiff, outdir=None, stat_files=True, verb=False)

   Performs Restoration Status Summary (RSS) analysis on a binary or
   multi-class raster. Computes patch-based habitat network indices
   including Equivalent Connected Area (ECA), Coherence (COH) and
   Restoration Potential (REST_POT) from the patch size distribution.

   :param in_tiff: Path to input GeoTIFF (0=NoData, 1=Background, 2=Foreground; optionally 3/4 for special background classes).
   :type in_tiff: str or Path
   :param outdir: Output directory. If ``None`` (default), outputs are written to the input file's directory.
   :type outdir: str or Path, optional
   :param stat_files: If ``True``, writes a .txt report file.
   :type stat_files: bool, optional
   :param verb: If ``True``, prints progress messages.
   :type verb: bool, optional

   :returns: **dict** -- A dictionary containing three main sections:

       * ``"output paths"`` (*dict* or *None*):
           * ``"path txt"`` (*str*) -- Absolute path to the statistics report (RSS writes no GeoTIFF).
       * ``"input stats"`` (*dict*):
           * ``"foreground pxl"`` (*int*) -- Count of foreground pixels.
           * ``"background pxl"`` (*int*) -- Count of background pixels.
           * ``"missing pxl"`` (*int*) -- Count of NoData pixels.
           * ``"backgr3 pxl"`` (*int*) -- Count of special background class 3 pixels.
           * ``"backgr4 pxl"`` (*int*) -- Count of special background class 4 pixels.
       * ``"output stats"`` (*dict*):
           * ``"total patches"`` (*int*) -- Total number of discrete foreground patches.
           * ``"average patch size"`` (*float*) -- Mean patch size in pixels.
           * ``"median patch size"`` (*float*) -- Median patch size in pixels.
           * ``"largest patch size"`` (*int*) -- Size of the largest single patch in pixels.
           * ``"CNOA"`` (*int*) -- Critical New Object Area.
           * ``"ECA"`` (*int*) -- Equivalent Connected Area.
           * ``"RAC"`` (*float*) -- Reference Area Coverage (%).
           * ``"COH"`` (*float*) -- Coherence index (%).
           * ``"REST_POT"`` (*float*) -- Restoration Potential index (%), equal to ``100 - COH``.

   :outputs: The function writes the following output file with ``stat_files=True``:

       * ``<input_filename>_rss.txt``: Statistics report with all connectivity indices

   .. rubric:: Example
   Standard execution of the RSS analysis on a binary foreground/background
   raster:

   .. code-block:: python

        >>> import pyguidos as pg
        >>> rss_result = pg.rss(in_tiff="my_map.tif", outdir="output/",
        ... stat_files=True, verb=False)
        >>> rss_result['output stats']['COH']
        70.1
        >>> rss_result['output paths']['path txt']
        'output/my_map_rss.txt'


Connectivity Indices
--------------------

RSS computes the following patch-based connectivity indices:

.. list-table:: RSS connectivity indices.
   :header-rows: 1
   :widths: 35 15 15 35

   * - Full Name
     - Code
     - Unit
     - Description
   * - Critical New Object Area
     - CNOA
     - pixels
     - Minimum area of a new patch that would increase connectivity
   * - Equivalent Connected Area
     - ECA
     - pixels
     - Area of a single patch providing the same connectivity as observed
   * - Reference Area Coverage
     - RAC
     - %
     - Percentage of foreground relative to total foreground and background
   * - Coherence
     - COH
     - %
     - Percentage of foreground pixels effectively connected
   * - Restoration Potential
     - REST_POT
     - %
     - Percentage of foreground pixels that could improve connectivity (100 - COH)

In addition to these indices, RSS reports basic patch size statistics:
total number of foreground patches, and the average, median and largest
patch size (in pixels).
