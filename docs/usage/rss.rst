RSS
===

Raster Spatial Statistics (RSS) computes patch-based connectivity indices
for a binary raster map. RSS characterises the spatial structure of the
foreground by analysing the size distribution of individual patches,
providing a set of indices that quantify landscape connectivity and
restoration potential. The methodology is described in detail in
`LINK_TO_RSS_PAPER <https://LINK_TO_RSS_PAPER>`_.


Connectivity Indices
--------------------

RSS computes the following indices:

.. list-table::
   :header-rows: 1

   * - Index
     - Full Name
     - Description
   * - **ECA**
     - Equivalent Connected Area
     - Area of a single patch that would provide the same
       connectivity as the observed patch mosaic
   * - **COH**
     - Degree of Coherence
     - Percentage of foreground pixels that are effectively
       connected, relative to total foreground area
   * - **CNOA**
     - Connectivity Number of Areas
     - Equivalent number of equally sized and maximally connected
       patches that would provide the same connectivity as observed
   * - **RPOT**
     - Restoration Potential
     - Percentage of foreground pixels that could potentially
       improve connectivity through restoration (100 - COH)
   * - **RAC**
     - Relative Area of the Core
     - Percentage of foreground pixels relative to total
       foreground and background pixels


Usage
-----

.. code-block:: python

    import pyguidos as pg

    result = pg.rss(
        in_tiff="my_map.tif",
        outdir="output/",
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
   * - ``in_tiff``
     - str or Path
     - --
     - Path to input GeoTIFF
   * - ``outdir``
     - str or Path
     - None
     - Output directory
   * - ``stat_files``
     - bool
     - True
     - Write statistics to files
   * - ``verb``
     - bool
     - False
     - Print progress messages


Output Files
------------

.. list-table::
   :header-rows: 1

   * - File
     - Description
   * - ``<name>_rss.txt``
     - Statistics report with all connectivity indices


Result Object
-------------

:func:`rss` returns an :class:`RssResult` dataclass:

.. code-block:: python

    result = pg.rss("my_map.tif")

    # Access statistics
    print(result.stats.keys())
    # dict_keys(['output paths', 'input stats', 'output stats'])

    # Input pixel counts
    print(result.stats["input stats"])
    # {'foreground pxl': 12500, 'background pxl': 37500,
    #  'missing pxl': 0, 'backgr3 pxl': 0, 'backgr4 pxl': 0}

    # Connectivity indices
    print(result.stats["output stats"])
    # {'total patches': 142, 'average patch size': 88.0,
    #  'median patch size': 12.0, 'largest patch size': 8542,
    #  'ECA': 8764.3, 'COH': 70.1, 'CNOA': 3, 'RPOT': 29.9, 'RAC': 25.0}

    # Output file paths
    print(result.stats["output paths"])
    # {'path txt': 'output/my_map_rss.txt'}

.. note::
    ``result.stats["output paths"]`` is ``None`` when ``stat_files=False``.
    All other keys are always populated regardless of ``stat_files``.


Patch Statistics
----------------

In addition to the connectivity indices, RSS also reports basic patch
size statistics:

- **Total patches**: total number of foreground patches
- **Average patch size**: mean patch size in pixels
- **Median patch size**: median patch size in pixels
- **Largest patch size**: size of the largest patch in pixels


References
----------

CITATION_PLACEHOLDER