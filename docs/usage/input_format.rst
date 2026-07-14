Input Format
============

All pyGuidos tools operate on **single-band GeoTIFF** files with a
specific pixel value convention. Understanding this format is essential
before using any of the analysis functions.


Foreground/Background Binary Maps
----------------------------------

Most pyGuidos tools expect a binary GeoTIFF where pixels are classified as
foreground (the feature of interest, e.g., forest) or background (everything
else). Two optional special background classes allow finer control over how
non-foreground areas interact with the analysis:

- **Special Background 3 (SP3)**: A background class that **fragments** the
  foreground. Adjacent SP3 pixels break foreground connectivity just like
  standard background. Use for features that clearly separate foreground
  patches (e.g., inland water bodies, urban areas).

- **Special Background 4 (SP4)**: A background class that **does not fragment**
  the foreground. SP4 pixels are treated as transparent/missing during
  spatial computation — they are excluded from the moving window denominator
  and pair counting. Use for features that should not influence connectivity
  metrics (e.g., rocks, transitional vegetation).

**Fragmentation (FAD, FAC, FED)** — accepts all values 0–4:

.. list-table::
   :header-rows: 1

   * - Value
     - Meaning
     - Required
     - Role in computation
   * - 0
     - NoData
     - Optional
     - Excluded from computation
   * - 1
     - Background
     - Mandatory
     - Fragments foreground (counts in denominator)
   * - 2
     - Foreground
     - Mandatory
     - The feature being analysed
   * - 3
     - Special Background 3
     - Optional
     - Fragments foreground (same as value 1)
   * - 4
     - Special Background 4
     - Optional
     - Does NOT fragment foreground (excluded like NoData)

**Accounting, RSS, and SPA** — accept only values 0–2:

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

.. note::
    SPA, Accounting, and RSS do not accept values 3 and 4. If your input
    contains these values, reclassify them to 0 (NoData) or 1 (Background)
    before running these tools.


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
  value encodes Missing/NoData by convention (e.g. 129 for SPA, 102 for
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