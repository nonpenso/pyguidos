Fragmentation
=============

Fragmentation analysis uses a Fixed Observation Scale (FOS) approach to compute
foreground pattern indices within a user-defined moving window. Each foreground
pixel is assigned a value based on the local neighbourhood composition, providing
a spatially explicit measure of landscape fragmentation.

Two methods are available:

- **FAD** (Foreground Area Density): computes within each window the proportion
  of foreground pixels relative to the total number of window pixels.
- **FAC** (Foreground Area Clustering): computes within each window the proportion
  of common adjacencies (shared vertical or horizontal pixel edges) between
  foreground pixels relative to the total number of adjacencies inside the
  moving window.

Further details about Fragmentation analysis are available in the
`Connectivity/Fragmentation product sheet
<https://ies-ows.jrc.ec.europa.eu/gtb/GTB/psheets/GTB-Fragmentation-FADFOS.pdf>`_.


.. figure:: ../_image/FOS_methods.png
    :width: 100%
    :align: center
    :alt: FOS methods

    Comparison of FAD and FAC methods exemplified for a 5x5 moving window 
    on a binary input map where black pixels are the foreground.


Fragmentation Classes
---------------------

The result of a FOS analysis is a map with the same spatial extent as the input,
where each foreground pixel receives a value in the range [0, 100] reflecting the
FAD or FAC metric in its local neighbourhood. These continuous values are then
aggregated into 5 classes and colour-coded in the output map:

.. list-table::
   :header-rows: 1

   * - Class
     - FOS range
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
    :func:`frag_stats` requires the input GeoTIFF to be an pyGuidos (or
    GTB) output raster file. See :doc:`input_format` for details.


