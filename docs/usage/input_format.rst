Input Format
============

All pyGuidos tools operate on **single-band GeoTIFF** files with a
specific pixel value convention. Understanding this format is essential
before using any of the analysis functions.


Binary
---------------------------

Most pyGuidos tools (MSPA, Fragmentation, Accounting, RSS) expect a
binary GeoTIFF with the following pixel values:

.. list-table::
   :header-rows: 1

   * - Value
     - Meaning
     - Required
     - Example
   * - 0
     - NoData
     - Optional
     - Missing data/Cloud
   * - 1
     - Background
     - Mandatory
     - Non-forest land
   * - 2
     - Foreground
     - Mandatory
     - Forest
   * - 3
     - Background class 2
     - Optional
     - Inland water
   * - 4
     - Background class 3
     - Optional
     - Sea/Ocean

.. note::
    Values 3 and 4 are only accepted by Fragmentation, Accounting and RSS.
    MSPA strictly requires only values 0, 1 and 2.


Landscape Mosaic Maps
---------------------

The Landscape Mosaic tool expects a three-class GeoTIFF with
the following pixel values:

.. list-table::
   :header-rows: 1

   * - Value
     - Meaning
     - Required
     - Example
   * - 0
     - NoData
     - Optional
     - Missing data/Cloud
   * - 1
     - Class 1
     - Mandatory
     - Agriculture
   * - 2
     - Class 2
     - Mandatory
     - Natural
   * - 3
     - Class 3
     - Mandatory
     - Developed

.. note::
    All three classes must be present in the input map. The tool will
    raise an error if any of them is missing.


Coordinate Reference System
---------------------------

pyGuidos accepts both **projected** and **geographic** coordinate
reference systems. However, area-based statistics (window area in
hectares and acres) are only computed for projected CRS.


GTB Output Format
-----------------

All pyGuidos output GeoTIFFs follow the GuidosToolbox (GTB) convention:

- **Single-band uint8 GeoTIFF** with a colour palette
- **NoData is not set** in the TIFF header -- instead a specific pixel
  value encodes Missing/NoData by convention (e.g. 129 for MSPA, 102 for
  Fragmentation)
- Output file name includes the input file name followed by the **used tool and
  the parameters**


Checking Your Input
-------------------

You can verify your input file before running any tool:

.. code-block:: python

    from pyguidos import utils

    # Get raster metadata
    info = utils.get_raster_info("my_map.tif")

    print(f"Size:  {info['rows']} x {info['cols']} pixels")
    print(f"Bands: {info['bands']}")
    print(f"dtype: {info['dtype']}")
    print(f"EPSG:  {info['epsg']}")
    print(f"Res:   {info['resX']} x {info['resY']}")


Using GTB Outputs as Inputs
----------------------------

The statistic functions `*_stats` accept only pyGuidos (or GTB) output 
GetoTIFFs as input. For example, using the GeoTIFF outputs after the function 
`extract_by_polygon` to compute the statistics of extracted GeoTIFFs.