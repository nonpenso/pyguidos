Input Format
============

All pyGuidos tools operate on **single-band GeoTIFF** files with a
specific pixel value convention. Understanding this format is essential
before using any of the analysis functions.


Input Map Types
---------------

pyGuidos supports two types of input maps, each with its own pixel value
convention depending on the analysis tool being used.


Foreground/Background Binary Maps
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Binary input maps are used by the Fragmentation, Accounting, RSS, and SPA
tools. Pixels are classified as foreground (the feature of interest, e.g.,
forest) or background (everything else). The input raster must be a
single-band integer GeoTIFF (typically uint8).

**Pixel value convention:**

- **Value 0 — NoData**: Missing or invalid pixels (e.g., clouds, areas outside
  the study region). These pixels are completely excluded from all computations
  and do not influence the results.

- **Value 1 — Background**: Non-foreground land cover (e.g., non-forest,
  agricultural or urban land). Background pixels are part of the reporting unit and
  actively participate in the analysis — they fragment the foreground by
  breaking spatial continuity between foreground patches.

- **Value 2 — Foreground**: The feature of interest (e.g., forest, habitat).
  This is the class being analysed. All spatial indices are computed for and
  relative to these pixels.

- **Value 3 — Special Background 3 (SP3)**: An optional secondary background
  class that **fragments** the foreground. SP3 behaves identically to standard
  background (value 1) in all computations — adjacent SP3 pixels break
  foreground connectivity. Use for features that clearly separate foreground
  patches (e.g., inland water bodies, urban areas within a forest landscape).

- **Value 4 — Special Background 4 (SP4)**: An optional background class that
  **does not fragment** the foreground. SP4 pixels are treated as
  transparent/missing during spatial computation — they are excluded from the
  moving window denominator and pair counting. The foreground "sees through"
  SP4 pixels as if they were not there. Use for features that should not
  influence connectivity metrics (e.g., rocks, transitional woodland).

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
     - Special Backgr. 3
     - Optional
     - Fragments foreground (same behaviour as value 1)
   * - 4
     - Special Backgr. 4
     - Optional
     - Does NOT fragment foreground (excluded like NoData)

**Accounting, RSS, and SPA** — accept only values 0–2:

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
     - Non-foreground land cover
   * - 2
     - Foreground
     - Mandatory
     - The feature being analysed

.. note::
    SPA, Accounting, and RSS do not accept values 3 and 4. If your input
    contains these values, reclassify them to 0 (NoData) or 1 (Background)
    before running these tools.


Landscape Mosaic Maps
^^^^^^^^^^^^^^^^^^^^^

The Landscape Mosaic tool (``landmos()``) analyses the compositional diversity
of a land cover map within a moving window. Unlike binary tools, it requires
exactly **three land cover classes** to be present, representing the three
poles of the tri-polar landscape model. The input raster must be a single-band
integer GeoTIFF (typically uint8).

The three classes can represent any meaningful land cover trichotomy (e.g.,
Agriculture/Natural/Developed, or Forest/Grassland/Shrubland). The tool computes
the proportional composition of the three classes within each window and
classifies the result into one of 103 compositional classes.

**Pixel value convention:**

- **Value 0 — NoData**: Missing or invalid pixels. Excluded from the window
  computation (does not contribute to any class proportion).

- **Value 1 — Class 1**: First land cover class (e.g., Agriculture). By
  convention often labelled as the "blue" pole in the ternary diagram.

- **Value 2 — Class 2**: Second land cover class (e.g., Natural vegetation).
  By convention the "green" pole.

- **Value 3 — Class 3**: Third land cover class (e.g., Developed/Urban).
  By convention the "red" pole.

.. list-table::
   :header-rows: 1

   * - Value
     - Meaning
     - Required
     - Role in computation
   * - 0
     - NoData
     - Optional
     - Excluded from window computation
   * - 1
     - Class 1
     - Mandatory
     - First pole of the ternary model (e.g., Agriculture)
   * - 2
     - Class 2
     - Mandatory
     - Second pole of the ternary model (e.g., Natural vegetation)
   * - 3
     - Class 3
     - Mandatory
     - Third pole of the ternary model (e.g., Developed / Urban)

.. note::
    All three classes (1, 2, 3) must be present in the input map. The tool
    will raise an error if any class is missing. The class labels are
    arbitrary — they can represent any three-way land cover categorisation
    relevant to your study area.


Coordinate Reference System
----------------------------

pyGuidos accepts both **projected** and **geographic** coordinate
reference systems. However, area-based statistics (window area in
hectares and acres) are only computed for projected CRS.


GTB Output Format
-----------------

All pyGuidos output GeoTIFFs follow the GuidosToolbox (GTB) convention:

- **Single-band uint8 GeoTIFF** with a colour palette
- **NoData is not set** in the TIFF header — instead a specific pixel
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

The statistic functions ``*_stats`` accept only pyGuidos (or GTB) output
GeoTIFFs as input. For example, using the GeoTIFF outputs after the function
``extract_by_polygon`` to compute the statistics of extracted GeoTIFFs.
