Fragmentation Change
====================

Fragmentation Change analysis evaluates landscape structural transitions over time 
by performing a comparative pixel-by-pixel cross-tabulation matrix overlay using two 
existing Guidos Fragmentation maps (Time A/T1 and Time B/T2).

The analysis tracks localized connectivity variations, groups them into 7 
categorical transition tiers (ranging from high decrease to high increase), and 
compiles detailed transition matrices tracking spatial land-cover and class variations.

Further details about structural dynamics and change metrics are available in the
`Fragmentation Change product sheet
<https://ies-ows.jrc.ec.europa.eu/gtb/GTB/psheets/GTB-Fragmentation-FADFOS.pdf>`_.


Fragmentation Change Classes
-------------------------

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
     - [0-79]
     - [+21 -- +100]
   * - Medium decrease
     - Medium increase
     - [80-89]
     - [+11 -- +20]
   * - Low decrease
     - Low increase
     - [90-98]
     - [+2 -- +10]    
   * - Insign/no change
     - Insign/no change
     - [99-101]
     - [-1 -- +1]
   * - Low increase
     - Low decrease
     - [102-110]
     - [-10 -- -2]
   * - Medium increase
     - Medium decrease
     - [111-120]
     - [-20 -- -11]
   * - High increase
     - High decrease
     - [121-200]
     - [-100 -- -21]


Usage
-----

.. code-block:: python

    from pyguidos.fragmentation_change import frag_change

    result = frag_change(
        in_tiff_t1="forest_map_2015_frag_fad_27.tif",
        in_tiff_t2="forest_map_2020_frag_fad_27.tif",
        outdir="output/",
        statists=True,
        stat_files=True,
        verb=False
    )


Parameters
----------

.. list-table::
   :header-rows: 1

   * - Parameter
     - Type
     - Default
     - Description
   * - ``in_tiff_t1``
     - str or Path
     - --
     - Path to the initial time-step Fragmentation GeoTIFF (Time A / T1)
   * - ``in_tiff_t2``
     - str or Path
     - --
     - Path to the subsequent time-step Fragmentation GeoTIFF (Time B / T2)
   * - ``outdir``
     - str or Path
     - None
     - Output directory. Defaults to the directory of ``in_tiff_t1``
   * - ``statists``
     - bool
     - True
     - Compute change transition statistics
   * - ``stat_files``
     - bool
     - True
     - Write text reports, tabular metrics, and histogram images to files
   * - ``verb``
     - bool
     - False
     - Print execution pipeline progress log messages


Output Files
------------

All generated files use the default standardized naming conventions inside the designated destination folder:

.. list-table::
   :header-rows: 1

   * - File
     - Description
   * - ``FOS_change.tif``
     - Categorical fragmentation change layer grid containing custom embedded color palette
   * - ``FOS_change.txt``
     - Detailed structural analysis cross-tabulation report
   * - ``FOS_change.csv``
     - Tabular pixel count metrics per frequency index
   * - ``FOS_change.png``
     - Connectivity change distribution bar chart image


Results
-------

The :func:`frag_change` function returns a nested :class:`dict` containing comprehensive information about the processing pipeline run:

* **output paths** (:class:`dict` or :obj:`None`)
    * **path tif** (:class:`str`): Absolute path to the categorical change output GeoTIFF.
    * **path txt** (:class:`str`): Absolute path to the text report.
    * **path csv** (:class:`str`): Absolute path to the frequency statistics CSV.
    * **path png** (:class:`str`): Absolute path to the histogram chart figure.
    * *Note: This entire key is* ``None`` *if* ``stat_files=False``.

* **input stats** (:class:`dict`)
    * **A foregr pxl** (:class:`int`): Foreground count at Time A.
    * **A backgr pxl** (:class:`int`): Background count at Time A.
    * **A missing pxl** (:class:`int`): NoData count at Time A.
    * **B foregr pxl** (:class:`int`): Foreground count at Time B.
    * **B backgr pxl** (:class:`int`): Background count at Time B.
    * **B missing pxl** (:class:`int`): NoData count at Time B.
    * *(Includes special background classes counts: ``A backgr3 pxl``, ``A backgr4 pxl``, etc.)*

* **output stats** (:class:`dict`)
    * **class freq** (:class:`dict`): Individual class distributions mapping across both frames (``A1 rare pxl`` to ``B5 inter pxl``).
    * **Frag change freq** (:class:`dict`): Consolidated count grouping mapped across the 7 connectivity change classes:
        * ``1 Frag High decrease``
        * ``2 Frag Medium decrease``
        * ``3 Frag Low decrease``
        * ``4 Frag Insign/no change``
        * ``5 Frag Low increase``
        * ``6 Frag Medium increase``
        * ``7 Frag High increase``
    * **Land change matrix** (:class:`np.ndarray`): Aggregated land-cover transition grid tracking broader foreground/background changes.
    * **Class change matrix** (:class:`np.ndarray`): 6x6 class dynamics matrix monitoring movements between specific fragmentation levels.
    * **A fad_av** / **B fad_av** (:class:`float`): Average Forest Area Density for Time A and Time B.
    * **A avcon** / **B avcon** (:class:`float`): Average Connectivity index for Time A and Time B.

.. code-block:: python

    from pyguidos.fragmentation_change import frag_change

    result = frag_change("t1.tif", "t2.tif")

    # Access main tracking categories
    print(result.keys())
    # dict_keys(['output paths', 'input stats', 'output stats'])

    # Query transition distribution trends
    print(result["output stats"]["Conn change freq"])
    # {
    #   '1 Frag High decrease': 450, 
    #   '2 Frag Medium decrease': 1200, 
    #   '3 Frag Low decrease': 3400,
    #   '4 Frag Insign/no change': 45000, 
    #   '5 Frag Low increase': 5600, 
    #   '6 Frag Medium increase': 890, 
    #   '7 Frag High increase': 120
    # }

    # View absolute file locations
    print(result["output paths"]["path tif"])
    # "output/FOS_change.tif"


.. note::
    Both inputs must have been processed using identical parameter configurations 
    (same window dimensions, same connectivity rules, and identical grid geometry metrics). 
    The system runs validation routines automatically and throws an error if any structural parameter discrepancies are encountered.