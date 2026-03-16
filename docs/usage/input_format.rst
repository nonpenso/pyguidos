Input Format
============

All pyGuidos tools operate on single-band **uint8 GeoTIFF** files with a
specific pixel value convention. Understanding this format is essential
before using any of the analysis functions.


Binary and Multi-class Maps
---------------------------

Most pyGuidos tools (MSPA, Fragmentation, Accounting, RSS) expect a
binary or multi-class uint8 GeoTIFF with the following pixel values:

.. list-table::
   :header-rows: 1

   * - Value
     - Meaning
     - Required
   * - 0
     - NoData
     - Optional
   * - 1
     - Background
     - Mandatory
   * - 2
     - Foreground
     - Mandatory
   * - 3
     - Background class 2
     - Optional
   * - 4
     - Background class 3
     - Optional

.. note::
    Values 3 and 4 are only accepted by Fragmentation, Accounting and RSS.
    MSPA strictly requires only values 0, 1 and 2.


Landscape Mosaic Maps
---------------------

The Landscape Mosaic tool expects a three-class uint8 GeoTIFF with
the following pixel values:

.. list-table::
   :header-rows: 1

   * - Value
     - Meaning
     - Required
   * - 0
     - NoData
     - Optional
   * - 1
     - Class 1 (e.g. Agriculture)
     - Mandatory
   * - 2
     - Class 2 (e.g. Natural)
     - Mandatory
   * - 3
     - Class 3 (e.g. Developed)
     - Mandatory

.. note::
    All three classes must be present in the input map. The tool will
    raise an error if any of them is missing.


Coordinate Reference System
---------------------------

pyGuidos accepts both **projected** and **geographic** coordinate
reference systems. However, area-based statistics (window area in
hectares and acres) are only computed for projected CRS. For
geographic CRS these fields will report ``--`` in the output report.


GTB Output Format
-----------------

All pyGuidos output GeoTIFFs follow the GuidosToolbox (GTB) convention:

- Single-band **uint8 GeoTIFF** with a colour palette
- **NoData is not set** in the TIFF header -- instead a specific pixel
  value encodes NoData by convention (e.g. 129 for MSPA, 102 for
  Fragmentation)
- A **GTB metadata tag** is written to ``TIFFTAG_IMAGEDESCRIPTION``,
  encoding the tool and parameters used:

.. list-table::
   :header-rows: 1

   * - Tool
     - Tag format
     - Example
   * - MSPA
     - ``GTB_MSPA, connectivity edge_width transition int_ext``
     - ``GTB_MSPA, 8 1 1 1``
   * - Fragmentation
     - ``GTB_FOS, method WSsizexsize``
     - ``GTB_FOS, FAD WS27x27``
   * - Landscape Mosaic
     - ``GTB_LM, WSsizexsize``
     - ``GTB_LM, WS33x33``
   * - Accounting
     - ``GTB_ACC, (threshold list)``
     - ``GTB_ACC, (10 100 1000)``

This tag allows the standalone ``*_stats()`` functions to identify the
tool and parameters without requiring the user to pass them explicitly.


Checking Your Input
-------------------

You can verify your input file before running any tool:

.. code-block:: python

    from pyguidos import utils

    # Get raster metadata
    info = utils.get_raster_info("my_map.tif")

    print(f"Size:  {info['rows']} x {info['cols']} pixels")
    print(f"dtype: {info['dtype']}")
    print(f"EPSG:  {info['epsg']}")
    print(f"Res:   {info['resX']} x {info['resY']}")
    print(f"Tag:   {info['tag']}")

    # Check pixel value frequencies
    freq = utils.get_pxl_freq(info['profile'])
    print(f"Values: {sorted(freq.keys())}")


Using GTB Outputs as Inputs
----------------------------

Several pyGuidos tools accept GTB output maps as inputs. For example,
MSPA output can be used as input to Fragmentation or Accounting after
reclassifying the MSPA classes back to a binary map. The GTB metadata
tag is preserved through :func:`extract_by_polygon` so downstream tools
can always identify the source analysis.