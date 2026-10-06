Fragmentation Change
====================

Fragmentation Change analysis evaluates landscape structural transitions over time
by performing a comparative pixel-by-pixel cross-tabulation matrix overlay using two
FOS Fragmentation maps (Time A/T1 and Time B/T2).

The analysis tracks localized connectivity variations, groups them into 7
categorical transition tiers (ranging from high decrease to high increase), and
compiles detailed transition matrices tracking spatial land-cover and class variations.

Further details about structural dynamics and change metrics are available in the
`Fragmentation Change product sheet
<https://ies-ows.jrc.ec.europa.eu/gtb/GTB/psheets/GTB-Fragmentation-FADFOS.pdf>`_.

.. note::
    ``frag_change()`` computes its statistics as part of the main run; there is
    no separate standalone ``*_stats()`` function for this tool. Both inputs
    must have been processed with identical parameters (same window size,
    connectivity rules and grid geometry); the inputs are validated
    automatically and an error is raised on any structural discrepancy.


Functions
---------

.. py:function:: pyguidos.frag_change(in_tiff_t1, in_tiff_t2, outdir=None, statists=True, stat_files=True, verb=False)

   Performs a comparative Fragmentation Change analysis using two structural
   fragmentation rasters (Time A/T1 and Time B/T2). It computes pixel-by-pixel
   class transitions across a 7-tier matrix overlay, tracks localized delta
   variations, and compiles global matrix statistics reporting land-cover and
   connectivity class dynamics.

   :param in_tiff_t1: Path to the initial time-step Fragmentation GeoTIFF (Time A/T1). Must be a valid Guidos ``GTB_FOS`` output.
   :type in_tiff_t1: str or Path
   :param in_tiff_t2: Path to the subsequent time-step Fragmentation GeoTIFF (Time B/T2). Must be a valid Guidos ``GTB_FOS`` output matching T1's spatial extent.
   :type in_tiff_t2: str or Path
   :param outdir: Output directory. If ``None`` (default), outputs are written to the directory of ``in_tiff_t1``.
   :type outdir: str or Path, optional
   :param statists: If ``True``, computes and returns change transition statistics.
   :type statists: bool, optional
   :param stat_files: If ``True`` (and ``statists=True``), writes .txt, .csv and .png report files.
   :type stat_files: bool, optional
   :param verb: If ``True``, prints progress messages.
   :type verb: bool, optional

   :returns: **dict** or **None** -- When ``statists=True``, a dictionary containing three main sections (returns ``None`` if ``statists=False``):

       * ``"output paths"`` (*dict* or *None*):
           * ``"path tif"`` (*str*) -- Absolute path to the categorical change result GeoTIFF.
           * ``"path txt"`` (*str*) -- Absolute path to the statistics report.
           * ``"path csv"`` (*str*) -- Absolute path to the per-value pixel count CSV.
           * ``"path png"`` (*str*) -- Absolute path to the connectivity change histogram.
       * ``"input stats"`` (*dict*): Raw pixel frequencies for T1 and T2 using the keys ``"A foregr pxl"``, ``"A backgr pxl"``, ``"A backgr3 pxl"``, ``"A backgr4 pxl"``, ``"A missing pxl"`` and the matching ``"B ..."`` keys.
       * ``"output stats"`` (*dict*):
           * ``"class freq"`` (*dict*) -- Per-class pixel counts for both time steps (``"A1 rare pxl"`` to ``"A5 inter pxl"`` and ``"B1 rare pxl"`` to ``"B5 inter pxl"``).
           * ``"Frag change freq"`` (*dict*) -- Pixel counts for the 7 connectivity change classes (``"1 Frag High decrease"``, ``"2 Frag Medium decrease"``, ``"3 Frag Low decrease"``, ``"4 Insign/no change"``, ``"5 Frag Low increase"``, ``"6 Frag Medium increase"``, ``"7 Frag High increase"``).
           * ``"Land change matrix"`` (*numpy.ndarray*) -- Aggregated 3x3 land-cover transition matrix (foreground / background / missing).
           * ``"Class change matrix"`` (*numpy.ndarray*) -- 6x6 fragmentation-class transition matrix.
           * ``"A fad_av"`` / ``"B fad_av"`` (*float*) -- Average Foreground Area Density index for Time A and Time B.
           * ``"A avcon"`` / ``"B avcon"`` (*float*) -- Average Connectivity index for Time A and Time B.

   :outputs: The function writes the following output files with ``stat_files=True``:

       * ``FOS_change.tif``: Categorical fragmentation change result GeoTIFF with color palette
       * ``FOS_change.txt``: Detailed cross-tabulation change matrix report
       * ``FOS_change.csv``: Per-value pixel counts with delta values
       * ``FOS_change.png``: Connectivity change frequency histogram

   .. rubric:: Example
   Compute and compare two fragmentation outputs with identical parameters at two
   different dates using the Forest/Non-Forest GeoTIFF files of Corsica stored 
   on ``pyguidos/data`` folder:

   .. code-block:: python

        >>> import pyguidos as pg
        >>> frag_t1 = pg.frag(in_tiff=pg.DATA_DIR / "CLC2000_corsica_FNF.tif",
        ... method="FAD", window_size=27, outdir="output/")
        >>> frag_t2 = pg.frag(in_tiff=pg.DATA_DIR / "CLC2018_corsica_FNF.tif",
        ... method="FAD", window_size=27, outdir="output/")
        >>> change_result = pg.frag_change(
        ... in_tiff_t1=frag_t1['output paths']['path tif'],
        ... in_tiff_t2=frag_t2['output paths']['path tif'],
        ... outdir="output/", statists=True, stat_files=True, verb=False)
        >>> change_result['output stats']['Frag change freq']
        {'1 Frag High decrease': 12017,
         '2 Frag Medium decrease': 12988,
         '3 Frag Low decrease': 39276,
         '4 Insign/no change': 177619,
         '5 Frag Low increase': 13655,
         '6 Frag Medium increase': 1876,
         '7 Frag High increase': 614}
        >>> change_result['output paths']['path tif']
        'output/FOS_change.tif'


Output Classes
--------------

The resulting map evaluates transitions and maps them into 7 distinct
categorical change classes based on the variation of Fragmentation/Connectivity:

.. list-table::
   :header-rows: 1

   * - Fragmentation
     - Connectivity
     - Pixel value
     - Delta FOS
   * - High decrease
     - High increase
     - [0, 79]
     - [+21, +100]
   * - Medium decrease
     - Medium increase
     - [80, 89]
     - [+11, +20]
   * - Low decrease
     - Low increase
     - [90, 98]
     - [+2, +10]
   * - Insign/no change
     - Insign/no change
     - [99, 101]
     - [-1, +1]
   * - Low increase
     - Low decrease
     - [102, 110]
     - [-10, -2]
   * - Medium increase
     - Medium decrease
     - [111, 120]
     - [-20, -11]
   * - High increase
     - High decrease
     - [121, 200]
     - [-100, -21]
