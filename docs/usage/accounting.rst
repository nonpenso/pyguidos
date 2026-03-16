Accounting
==========

Foreground Patch Size Accounting (ACC) labels and measures all individual
foreground patches in a binary raster, classifying them into user-defined
size classes. The result is a spatially explicit map and tabular summary
statistics describing the patch size distribution across the landscape.
The methodology is described in detail in
`LINK_TO_ACC_PAPER <https://LINK_TO_ACC_PAPER>`_.


Usage
-----

.. code-block:: python

    import pyguidos as pg

    result = pg.acc(
        in_tiff="my_map.tif",
        thresholds=[10, 100, 1000, 10000],
        outdir="output/",
        statists=True,
        stat_files=True,
        return_array=False,
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
   * - ``thresholds``
     - list, tuple or array
     - --
     - 1 to 5 unique positive integers defining size class boundaries
   * - ``outdir``
     - str or Path
     - None
     - Output directory
   * - ``statists``
     - bool
     - True
     - Compute statistics
   * - ``stat_files``
     - bool
     - True
     - Write statistics to files
   * - ``return_array``
     - bool
     - False
     - Return output array
   * - ``verb``
     - bool
     - False
     - Print progress messages


Thresholds
----------

The ``thresholds`` parameter defines the patch size class boundaries
in pixels. For example:

.. code-block:: python

    thresholds = [10, 100, 1000, 10000]

This creates 5 size classes:

.. list-table::
   :header-rows: 1

   * - Class
     - Size range
   * - 1
     - 1 -- 10 pixels
   * - 2
     - 11 -- 100 pixels
   * - 3
     - 101 -- 1000 pixels
   * - 4
     - 1001 -- 10000 pixels
   * - 5
     - > 10000 pixels

.. note::
    A minimum of 1 and a maximum of 5 thresholds are allowed.
    Duplicate values are automatically removed and the list is
    sorted before processing.


Output Files
------------

.. list-table::
   :header-rows: 1

   * - File
     - Description
   * - ``<name>_acc.tif``
     - Accounting result GeoTIFF with colour palette
   * - ``<name>_acc.txt``
     - Statistics report


Result Object
-------------

:func:`acc` returns an :class:`AccResult` dataclass:

.. code-block:: python

    result = pg.acc("my_map.tif", thresholds=[10, 100, 1000, 10000])

    # Access statistics
    print(result.stats.keys())
    # dict_keys(['output paths', 'input stats', 'output stats'])

    # Input pixel counts
    print(result.stats["input stats"])
    # {'foreground pxl': 12500, 'background pxl': 37500,
    #  'missing pxl': 0, 'backgr3 pxl': 0, 'backgr4 pxl': 0}

    # Per-class pixel and patch counts
    print(result.stats["output stats"])
    # {'pxl numb': Counter({...}), 'patch numb': Counter({...})}

    # Output file paths
    print(result.stats["output paths"])
    # {'path tif': 'output/my_map_acc.tif',
    #  'path txt': 'output/my_map_acc.txt'}

.. note::
    ``result.stats["output paths"]`` is ``None`` when ``stat_files=False``.
    All other keys are always populated regardless of ``stat_files``.


Computing Statistics Separately
--------------------------------

If you already have an accounting output GeoTIFF, you can compute
statistics without rerunning the analysis:

.. code-block:: python

    stats = pg.acc_stats(
        acc_tiff="output/my_map_acc.tif",
        outfile=True,
        outdir="output/",
        source_tiff="my_map.tif"
    )

.. note::
    :func:`acc_stats` requires the input GeoTIFF to contain a valid
    ``GTB_ACC`` metadata tag. See :doc:`input_format` for details.


References
----------

CITATION_PLACEHOLDER
