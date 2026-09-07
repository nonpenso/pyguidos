Accounting
==========

Foreground patch size Accounting labels and measures all individual
foreground patches in a binary raster, classifying them into user-defined
size classes. The result is a spatially explicit map and tabular summary
statistics describing the patch size distribution across the landscape.
Further details about Accounting analysis are available in the
`Accounting product sheet
<https://ies-ows.jrc.ec.europa.eu/gtb/GTB/psheets/GTB-Objects-Accounting.pdf>`_.


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
   * - ``verb``
     - bool
     - False
     - Print progress messages

Example with all parameters:

.. code-block:: python

    import pyguidos as pg

    result = pg.acc(
        in_tiff="my_map.tif",
        thresholds=[10, 100, 1000, 10000],
        outdir="output/",
        statists=True,
        stat_files=True,
        verb=False
    )

The ``thresholds`` parameter defines the patch size class boundaries
in pixels. For example, ``thresholds=[10, 100, 1000, 10000]`` creates 5 size
classes:

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
    sorted before processing. Thresholds are expressed in pixels: to convert
    to area units, multiply by the pixel area (e.g. at 25 m resolution,
    1 pixel = 0.0625 ha, so a threshold of 200 pixels = 12.5 hectares).


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


Output Classes
--------------

Foreground patches are labelled and classified into up to 6 size classes
based on user-defined area thresholds. Each class groups patches whose
size in pixels falls within a specific range, from the smallest isolated
patches to the largest connected foreground areas.

The number of classes depends on the number of thresholds provided:
1 threshold produces 2 classes, 2 thresholds produce 3 classes, and so on
up to a maximum of 5 thresholds producing 6 classes. Classes are assigned
from smallest to largest and colour-coded in the output map as follows:

.. list-table:: Accounting size classes, pixel values and colors.
   :header-rows: 1
   :widths: 15 20 20 45

   * - Class
     - Pixel Value
     - Color
     - Size Range
   * - 1
     - 103
     - Black
     - Smallest patches [1 -- threshold 1]
   * - 2
     - 33
     - Red
     - [threshold 1 + 1 -- threshold 2]
   * - 3
     - 65
     - Yellow
     - [threshold 2 + 1 -- threshold 3]
   * - 4
     - 1
     - Orange
     - [threshold 3 + 1 -- threshold 4]
   * - 5
     - 9
     - Brown
     - [threshold 4 + 1 -- threshold 5]
   * - 6
     - 17
     - Green
     - Largest patches [> last threshold]

In addition to the foreground classes, the output map encodes background
and special pixel values:

.. list-table:: Accounting background and special pixel values.
   :header-rows: 1
   :widths: 20 20 60

   * - Pixel Value
     - Color
     - Meaning
   * - 0
     - Grey
     - Background (value 1 in input)
   * - 129
     - White
     - NoData (value 0 in input)
   * - 105
     - Blue
     - Special background (value 3 in input)
   * - 176
     - Light Blue
     - Special background (value 4 in input)


Statistics
----------

Result Dictionary
^^^^^^^^^^^^^^^^^^

The ``acc()`` function returns a :class:`dict` with three sections:

* **output paths** (:class:`dict` or :obj:`None`)
    * **path tif** (:class:`str`): Absolute path to the resulting Accounting GeoTIFF.
    * **path txt** (:class:`str`): Absolute path to the statistics text report.
    * *Note: This key is* ``None`` *if* ``stat_files=False``.

* **input stats** (:class:`dict`)
    * **foreground pxl** (:class:`int`): Count of pixels with value 2 (Forest).
    * **background pxl** (:class:`int`): Count of pixels with value 1 (Background).
    * **missing pxl** (:class:`int`): Count of NoData (0) pixels.
    * **backgr3 pxl** (:class:`int`): Count of special background class 3 pixels.
    * **backgr4 pxl** (:class:`int`): Count of special background class 4 pixels.

* **output stats** (:class:`dict`)
    * **pxl numb** (:class:`dict`): A dictionary where keys are class IDs and values are the total number of pixels belonging to that accounting class.
    * **patch numb** (:class:`dict`): A dictionary where keys are class IDs and values represent the total number of discrete patches identified for that accounting class.

Accessing the result:

.. code-block:: python

    result = pg.acc("my_map.tif", thresholds=[10, 100, 1000, 10000])

    # Access statistics
    print(result.keys())
    # dict_keys(['output paths', 'input stats', 'output stats'])

    # Input pixel counts
    print(result["input stats"])
    # {'foreground pxl': 12500, 'background pxl': 37500,
    #  'missing pxl': 0, 'backgr3 pxl': 0, 'backgr4 pxl': 0}

    # Per-class pixel and patch counts
    print(result["output stats"])
    # {'pxl numb': Counter({...}), 'patch numb': Counter({...})}

    # Output file paths
    print(result["output paths"])
    # {'path tif': 'output/my_map_acc.tif',
    #  'path txt': 'output/my_map_acc.txt'}


Computing Statistics Separately
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you already have an accounting output GeoTIFF, you can compute
statistics without rerunning the analysis:

.. code-block:: python

    stats = pg.acc_stats(
        acc_tiff="output/my_map_acc.tif",
        stat_files=True,
        outdir="output/",
        source_tiff="my_map.tif"
    )

.. note::
    ``acc_stats()`` requires the input GeoTIFF to be a pyGuidos (or GTB)
    Accounting output (``GTB_ACC`` tag). See :doc:`input_format` for details.
