Accounting
==========

Foreground patch size Accounting labels and measures all individual
foreground patches in a binary raster, classifying them into user-defined
size classes. The result is a spatially explicit map and tabular summary
statistics describing the patch size distribution across the landscape.
Further details about Accounting analysis are available in the
`Accounting product sheet
<https://ies-ows.jrc.ec.europa.eu/gtb/GTB/psheets/GTB-Objects-Accounting.pdf>`_.

.. note::
    A minimum of 1 and a maximum of 5 thresholds are allowed. Duplicate
    values are automatically removed and the list is sorted before
    processing. Thresholds are expressed in pixels: to convert to area
    units, multiply by the pixel area (e.g. at 25 m resolution, 1 pixel =
    0.0625 ha, so a threshold of 200 pixels = 12.5 hectares).


Functions
---------

.. py:function:: pyguidos.acc(in_tiff, thresholds, outdir=None, statists=True, stat_files=True, verb=False)

   Performs Foreground Patch Size Accounting (ACC) on a binary or
   multi-class raster. Each foreground patch is classified into size
   categories defined by the user-provided thresholds, enabling analysis
   of the patch size distribution across the landscape.

   :param in_tiff: Path to input GeoTIFF (0=NoData, 1=Background, 2=Foreground; optionally 3/4 for special background classes).
   :type in_tiff: str or Path
   :param thresholds: Sequence of 1 to 5 unique positive integers defining the patch size class boundaries in pixels. For example, ``[10, 100, 1000]`` creates 4 classes: [1-10], [11-100], [101-1000], [>1000].
   :type thresholds: list, tuple or numpy.ndarray
   :param outdir: Output directory. If ``None`` (default), outputs are written to the input file's directory.
   :type outdir: str or Path, optional
   :param statists: If ``True``, computes and returns statistics.
   :type statists: bool, optional
   :param stat_files: If ``True``, writes a .txt report file.
   :type stat_files: bool, optional
   :param verb: If ``True``, prints progress messages.
   :type verb: bool, optional

   :returns: **dict** -- A dictionary containing three main sections:

       * ``"output paths"`` (*dict* or *None*):
           * ``"path tif"`` (*str*) -- Absolute path to the result GeoTIFF.
           * ``"path txt"`` (*str*) -- Absolute path to the statistics report.
       * ``"input stats"`` (*dict*):
           * ``"foreground pxl"`` (*int*) -- Count of foreground pixels.
           * ``"background pxl"`` (*int*) -- Count of background pixels.
           * ``"missing pxl"`` (*int*) -- Count of NoData pixels.
           * ``"backgr3 pxl"`` (*int*) -- Count of special background class 3 pixels.
           * ``"backgr4 pxl"`` (*int*) -- Count of special background class 4 pixels.
       * ``"output stats"`` (*dict*):
           * ``"class pxl"`` (*dict*) -- Pixel counts per accounting size class, keyed by class label (e.g. ``"1 [1-10]"``, ``"2 [11-100]"``, ``"3 [>1000]"``).
           * ``"class patch"`` (*dict*) -- Patch (object) counts per accounting size class, using the same keys as ``"class pxl"``.

   :outputs: The function writes the following output files with ``stat_files=True``:

       * ``<input_filename>_acc.tif``: Accounting result GeoTIFF with color palette
       * ``<input_filename>_acc.txt``: Statistics report

   .. rubric:: Example
   Standard execution of the Accounting analysis, splitting foreground
   patches into five size classes:

   .. code-block:: python

        >>> import pyguidos as pg
		>>> tiff = pg.DATA_DIR / "CLC2018_corsica_FNF.tif"
        >>> acc_result = pg.acc(in_tiff=tiff,
        ... thresholds=[10, 100, 1000, 10000], outdir="output/",
        ... statists=True, stat_files=True, verb=False)
        >>> acc_result['output stats']['class pxl']
        {'1 [1-10]': 104,
         '2 [11-100]': 11274,
         '3 [101-1000]': 38570,
         '4 [1001-10000]': 22617,
         '5 [>10000]': 228172}
        >>> acc_result['output paths']['path tif']
        'output/CLC2018_corsica_FNF_acc.tif'


.. py:function:: pyguidos.acc_stats(acc_tiff, stat_files=True, outdir=None, source_tiff=None)

   Computes statistics for an existing Accounting result GeoTIFF.

   :param acc_tiff: Path to the accounting result GeoTIFF (must carry a valid ``GTB_ACC`` metadata tag).
   :type acc_tiff: str or Path
   :param stat_files: If ``True``, writes a .txt report file.
   :type stat_files: bool, optional
   :param outdir: Directory for output files. Defaults to the input file's directory.
   :type outdir: str or Path, optional
   :param source_tiff: Path to the original input GeoTIFF used to generate the accounting result.
   :type source_tiff: str or Path, optional

   :returns: **dict** -- A dictionary containing three main sections:

       * ``"output paths"`` (*dict* or *None*):
           * ``"path tif"`` (*str*) -- Absolute path to the result GeoTIFF.
           * ``"path txt"`` (*str*) -- Absolute path to the statistics report.
       * ``"input stats"`` (*dict*):
           * ``"foreground pxl"`` (*int*) -- Count of foreground pixels.
           * ``"background pxl"`` (*int*) -- Count of background pixels.
           * ``"missing pxl"`` (*int*) -- Count of NoData pixels.
           * ``"backgr3 pxl"`` (*int*) -- Count of special background class 3 pixels.
           * ``"backgr4 pxl"`` (*int*) -- Count of special background class 4 pixels.
       * ``"output stats"`` (*dict*):
           * ``"class pxl"`` (*dict*) -- Pixel counts per accounting size class.
           * ``"class patch"`` (*dict*) -- Empty when computed from an existing GeoTIFF, since per-patch counts require the labelled array produced during the original ``acc()`` run.

   :outputs: The function writes the following output file with ``stat_files=True``:

       * ``<acc_filename>.txt``: Statistics report

   .. rubric:: Example
   After executing the Accounting analysis, pass the resulting GeoTIFF to
   ``acc_stats()`` to extract summary statistics.

   .. code-block:: python

        >>> acc_tiff = 'output/my_map_acc.tif'
        >>> acc_stats = pg.acc_stats(acc_tiff=acc_tiff, stat_files=True,
        ... outdir='output/', source_tiff="my_map.tif")
        >>> acc_stats['input stats']['foreground pxl']
        300737


Output Classes
--------------

Foreground patches are labelled and classified into up to 6 size classes
based on user-defined area thresholds. Each class groups patches whose
size in pixels falls within a specific range, from the smallest isolated
patches to the largest connected foreground areas.

The number of classes depends on the number of thresholds provided:
1 threshold produces 2 classes, 2 thresholds produce 3 classes, and so on
up to a maximum of 5 thresholds producing 6 classes. Classes are assigned
from smallest to largest and colour-coded in the output map as follows:

.. list-table:: Accounting size classes, pixel values and colors.
   :header-rows: 1
   :widths: 15 20 20 45

   * - Class
     - Pixel Value
     - Color
     - Size Range
   * - 1
     - 103
     - Black
     - Smallest patches [1 -- threshold 1]
   * - 2
     - 33
     - Red
     - [threshold 1 + 1 -- threshold 2]
   * - 3
     - 65
     - Yellow
     - [threshold 2 + 1 -- threshold 3]
   * - 4
     - 1
     - Orange
     - [threshold 3 + 1 -- threshold 4]
   * - 5
     - 9
     - Brown
     - [threshold 4 + 1 -- threshold 5]
   * - 6
     - 17
     - Green
     - Largest patches [> last threshold]

In addition to the foreground classes, the output map encodes background
and special pixel values:

.. list-table:: Accounting background and special pixel values.
   :header-rows: 1
   :widths: 20 20 60

   * - Pixel Value
     - Color
     - Meaning
   * - 0
     - Grey
     - Background (value 1 in input)
   * - 129
     - White
     - NoData (value 0 in input)
   * - 105
     - Blue
     - Special background (value 3 in input)
   * - 176
     - Light Blue
     - Special background (value 4 in input)
