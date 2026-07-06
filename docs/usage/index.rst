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
    pg.info('frag')
    
    # Get full technical specification of a function
    help(pg.frag)

.. list-table::
   :header-rows: 1

   * - Function
     - Description
     - Output Files
   * - :doc:`morphology`
     - Simplified Pattern Analysis
     - ``.tif``, ``.txt``
   * - :doc:`fragmentation`
     - Fragmentation analysis
     - ``.tif``, ``.txt``, ``.csv``, ``.png``
   * - :doc:`fragmentation_change`
     - Fragmentation change
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


Result Structure
----------------

Each analysis function returns a nested :class:`dict`. This dictionary contains all the 
metadata, file paths, and calculated statistics generated during the run.

The dictionary follows a consistent structure across all tools:

* **output paths**: (:class:`dict` or :obj:`None`) Paths to generated output files. 
  Returns ``None`` if ``stat_files=False``.
* **input stats**: (:class:`dict`) Basic pixel counts and metadata from the source map.
* **output stats**: (:class:`dict`) Tool-specific metrics (e.g., MSPA classes, 
  Fragmentation indices, or Landscape Mosaic frequencies).

.. code-block:: python

    import pyguidos as pg

    # Run an analysis (returns a dict)
    result = pg.frag("my_map.tif", "FAD", window_size=27)

    # Access primary keys
    print(result.keys())
    # dict_keys(['output paths', 'input stats', 'output stats'])

    # Access a specific output path
    tif_path = result['output paths']['path tif']
    print(f"Result saved at: {tif_path}")

    # Access calculated metrics
    avcon = result['output stats']['avcon ']
    print(f"AVCON: {avcon}")


Standalone Statistics
---------------------

All tools provide a companion ``*_stats()`` function that can be called
independently on a previously generated output GeoTIFF, without
rerunning the full analysis:

.. code-block:: python

    # Recompute statistics on an existing MSPA output
    stats = pg.frag_stats(
        frag_tiff="output/my_map_frag_FAD_27.tif",
        outfile=True
    )

.. note::
    Standalone ``*_stats()`` functions require as input raster files to
    be output GeoTIFFs from pyGuidos or GTB. See :doc:`input_format` for details.

.. toctree::
   :hidden:

   input_format
   morphology
   fragmentation
   fragmentation_change
   landmos
   accounting
   rss
   extract_by_polygon