User Guide
==========

This guide describes all available pyGuidos analysis tools. Each tool
operates on a single-band **uint8 GeoTIFF** input and returns a result
object containing statistics and optionally the output array.

Before using any tool, please read the :doc:`input_format` page which
describes the expected pixel value conventions and the GTB output format.


Available Tools
---------------

The easiest way to explore the available tools and access their 
scientific documentation is to use the interactive ``info()`` function:

.. code-block:: python

    import pyguidos as pg

    # List all available analytical tools
    pg.info()

    # Get detailed links and usage for a specific tool
    pg.info('mspa')
    
    # Get full technical specification of a function
    help(pg.mspa)

.. list-table::
   :header-rows: 1

   * - Function
     - Description
     - Output Files
   * - :doc:`mspa`
     - Morphological Spatial Pattern Analysis
     - ``.tif``, ``.txt``
   * - :doc:`fragmentation`
     - Fragmentation analysis
     - ``.tif``, ``.txt``, ``.csv``, ``.png``
   * - :doc:`landmos`
     - Landscape Mosaic
     - ``.tif`` (103 and 19 classes), ``.txt``, ``.csv``, ``.png``
   * - :doc:`accounting`
     - Patch Size Accounting
     - ``.tif``, ``.txt``
   * - :doc:`rss`
     - Restoration Status Summary
     - ``.txt``
   * - :doc:`extract_by_polygon`
     - Extract raster by polygon features
     - one ``.tif`` per polygon feature


Common Parameters
-----------------

All analysis functions share a common set of parameters:

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
     - Output directory. Defaults to input file directory
   * - ``statists``
     - bool
     - True
     - Compute and return statistics
   * - ``stat_files``
     - bool
     - True
     - Write statistics to output files (.txt, .png, ...)
   * - ``return_array``
     - bool
     - False
     - Include output numpy array in result object
   * - ``verb``
     - bool
     - False
     - Print progress messages to stdout


Result Objects
--------------

Each analysis function returns a result dataclass with two fields:

- **stats**: nested dictionary with three keys:

  - ``'output paths'``: paths to generated output files, or ``None``
    if ``stat_files=False``
  - ``'input stats'``: pixel counts for the input map
  - ``'output stats'``: tool-specific metrics and per-class pixel counts

- **array**: the output numpy array, only populated when
  ``return_array=True``, otherwise ``None``

.. code-block:: python

    import pyguidos as pg

    result = pg.mspa("my_map.tif", edge_width=1)

    # Access statistics dictionary
    print(result.stats.keys())
    # dict_keys(['output paths', 'input stats', 'output stats'])

    # Access output array (requires return_array=True)
    print(result.array)

    # Access a specific output path
    tif_path = result.stats['output paths']['path tif']
    print(f"Result saved at: {tif_path}")


Standalone Statistics
---------------------

All tools provide a companion ``*_stats()`` function that can be called
independently on a previously generated output GeoTIFF, without
rerunning the full analysis:

.. code-block:: python

    # Recompute statistics on an existing MSPA output
    stats = pg.mspa_stats(
        mspa_tiff="output/my_map_mspa_8_1_1_1.tif",
        outfile=True,
        source_tiff="my_map.tif"
    )

.. note::
    Standalone ``*_stats()`` functions require as input raster files to
    be output GeoTIFFs from pyGuidos or GTB. See :doc:`input_format` for details.

.. toctree::
   :hidden:

   input_format
   mspa
   fragmentation
   landmos
   accounting
   rss
   extract_by_polygon