MSPA
====

Morphological Spatial Pattern Analysis (MSPA) is a multi-scale image
processing approach that classifies the pixels of a binary foreground/
background image into mutually exclusive morphological classes based on
their spatial context. MSPA was originally developed at the European
Commission Joint Research Centre and is described in detail in
`LINK_TO_MSPA_PAPER <https://LINK_TO_MSPA_PAPER>`_.


MSPA Classes
------------

MSPA classifies foreground pixels into 7 structural categories:

.. list-table::
   :header-rows: 1

   * - Class
     - Description
   * - **Core**
     - Interior foreground pixels, away from the background
   * - **Edge**
     - Foreground pixels at the boundary with the background
   * - **Perforation**
     - Foreground pixels at the boundary with background holes
   * - **Islet**
     - Small isolated foreground patches, too small to have core
   * - **Branch**
     - Elongated foreground connections ending at background
   * - **Loop**
     - Elongated foreground connections between edge pixels
   * - **Bridge**
     - Elongated foreground connections between core pixels

Each class is further subdivided into **external** (suffix ``_e``) and
**internal** (suffix ``_i``) subclasses when ``int_ext=True``.


Usage
-----

.. code-block:: python

    import pyguidos as pg

    result = pg.mspa(
        in_tiff="my_map.tif",
        edge_width=1,
        connectivity=8,
        transition=True,
        int_ext=True,
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
   * - ``edge_width``
     - int
     - --
     - Edge width in pixels, >= 1
   * - ``connectivity``
     - int
     - 8
     - Pixel connectivity, 4 or 8
   * - ``transition``
     - bool
     - True
     - Enable transition zones
   * - ``int_ext``
     - bool
     - True
     - Distinguish internal/external subclasses
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
   * - ``<name>_mspa_<conn>_<ew>_<tr>_<ie>.tif``
     - MSPA result GeoTIFF with colour palette
   * - ``<name>_mspa_<conn>_<ew>_<tr>_<ie>.txt``
     - Statistics report


Result Object
-------------

:func:`mspa` returns an :class:`MSPAResult` dataclass:

.. code-block:: python

    result = pg.mspa("my_map.tif", edge_width=1)

    # Access statistics
    print(result.stats.keys())
    # dict_keys(['output paths', 'input stats', 'output stats'])

    # Input pixel counts
    print(result.stats["input stats"])
    # {'foreground pxl': 12500, 'background pxl': 37500, 'missing pxl': 0}

    # Per-class pixel counts
    print(result.stats["output stats"])
    # {'core pxl': 8200, 'edge pxl': 2100, 'perforation pxl': 500, ...}

    # Output file paths
    print(result.stats["output paths"])
    # {'path tif': 'output/my_map_mspa_8_1_1_1.tif',
    #  'path txt': 'output/my_map_mspa_8_1_1_1.txt'}

    # Access array (only if return_array=True)
    print(result.array)

.. note::
    ``result.stats["output paths"]`` is ``None`` when ``stat_files=False``.
    All other keys are always populated regardless of ``stat_files``.


Computing Statistics Separately
--------------------------------

If you already have an MSPA output GeoTIFF, you can compute statistics
without rerunning the analysis:

.. code-block:: python

    stats = pg.mspa_stats(
        mspa_tiff="output/my_map_mspa_8_1_1_1.tif",
        outfile=True,
        outdir="output/",
        source_tiff="my_map.tif"
    )

.. note::
    :func:`mspa_stats` requires the input GeoTIFF to contain a valid
    ``GTB_MSPA`` metadata tag. See :doc:`input_format` for details.


Edge Width and Connectivity
---------------------------

The two most important parameters are ``edge_width`` and ``connectivity``:

- **edge_width** controls the width of the Edge and Perforation classes
  in pixels. A value of 1 means a single pixel border. Larger values
  produce wider edge zones and smaller core areas.
- **connectivity** controls how pixels are considered connected.
  8-connectivity (default) allows diagonal connections, 4-connectivity
  does not. 8-connectivity is recommended for most applications.

.. tip::
    For a given spatial resolution, choose an ``edge_width`` that corresponds
    to a meaningful ecological distance. For example, at 25m resolution,
    ``edge_width=2`` corresponds to a 50m edge zone.


References
----------

CITATION_PLACEHOLDER
