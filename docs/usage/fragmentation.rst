Fragmentation
=============

Fragmentation analysis computes the proportion of foreground pixels within
a moving window, classifying each pixel based on the local density of
foreground cover. The analysis is performed using the Spatcon moving window
tool and is described in detail in
`LINK_TO_FRAG_PAPER <https://LINK_TO_FRAG_PAPER>`_.


Fragmentation Classes
---------------------

Each foreground pixel is assigned to one of 5 fragmentation classes based
on the proportion of foreground pixels within the moving window:

.. list-table::
   :header-rows: 1

   * - Class
     - FAD range
     - Description
   * - **Rare**
     - 0 -- 10%
     - Very low foreground density
   * - **Patchy**
     - 10 -- 40%
     - Low foreground density
   * - **Transitional**
     - 40 -- 60%
     - Medium foreground density
   * - **Dominant**
     - 60 -- 90%
     - High foreground density
   * - **Interior**
     - 90 -- 100%
     - Very high foreground density


Methods
-------

Two methods are available:

- **FAD** (Forest Area Density): computes the proportion of foreground
  pixels within the moving window relative to the total window area.
- **FOS** (Forest Overall Status): similar to FAD but uses a different
  background handling convention.


Usage
-----

.. code-block:: python

    import pyguidos as pg

    result = pg.frag(
        in_tiff="my_map.tif",
        method="FAD",
        window_size=27,
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
   * - ``method``
     - str
     - --
     - Fragmentation method: ``'FAD'`` or ``'FOS'``
   * - ``window_size``
     - int
     - --
     - Moving window size in pixels, odd integer >= 3
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


Output Files
------------

.. list-table::
   :header-rows: 1

   * - File
     - Description
   * - ``<name>_<method>_<window_size>.tif``
     - Fragmentation result GeoTIFF with colour palette
   * - ``<name>_<method>_<window_size>.txt``
     - Statistics report
   * - ``<name>_<method>_<window_size>.csv``
     - Per-value pixel counts and frequencies
   * - ``<name>_<method>_<window_size>.png``
     - Foreground pixel histogram


Result Object
-------------

:func:`frag` returns a :class:`FragResult` dataclass:

.. code-block:: python

    result = pg.frag("my_map.tif", method="FAD", window_size=27)

    # Access statistics
    print(result.stats.keys())
    # dict_keys(['output paths', 'input stats', 'output stats'])

    # Input pixel counts
    print(result.stats["input stats"])
    # {'foreground pxl': 12500, 'background pxl': 37500, 'missing pxl': 0, ...}

    # Fragmentation indices and class pixel counts
    print(result.stats["output stats"])
    # {'rare pxl': 1200, 'patch pxl': 2300, 'trans pxl': 3100,
    #  'domin pxl': 4200, 'inter pxl': 1700, 'fad_av': 62.3, 'avcon': 58.1}

    # Output file paths
    print(result.stats["output paths"])
    # {'path tif': 'output/my_map_fad_27.tif',
    #  'path txt': 'output/my_map_fad_27.txt',
    #  'path csv': 'output/my_map_fad_27.csv',
    #  'path png': 'output/my_map_fad_27.png'}

.. note::
    ``result.stats["output paths"]`` is ``None`` when ``stat_files=False``.
    All other keys are always populated regardless of ``stat_files``.


Computing Statistics Separately
--------------------------------

If you already have a fragmentation output GeoTIFF, you can compute
statistics without rerunning the analysis:

.. code-block:: python

    stats = pg.frag_stats(
        frag_tiff="output/my_map_fad_27.tif",
        outfile=True,
        outdir="output/",
        source_tiff="my_map.tif"
    )

.. note::
    :func:`frag_stats` requires the input GeoTIFF to contain a valid
    ``GTB_FOS`` metadata tag. See :doc:`input_format` for details.


Window Size
-----------

The ``window_size`` parameter controls the scale of the analysis:

- Larger windows capture broader landscape context but reduce spatial
  detail
- Smaller windows are more sensitive to local variation
- The window size should reflect a meaningful ecological scale for
  your application

.. tip::
    For landscape-scale analysis at 25m resolution, a window size of
    27 pixels corresponds to a 675m x 675m neighbourhood (approximately
    45.6 hectares).


References
----------

CITATION_PLACEHOLDER
