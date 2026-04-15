Restoration Status Summary
==========================

Restoration Status Summary (RSS) computes patch-based connectivity indices
for a binary raster map. RSS characterises the spatial structure of the
foreground by analysing the size distribution of individual patches,
providing a set of indices that quantify landscape connectivity and
restoration potential. 
Further details about Restoration Status Summary analysis are 
available in the `RSS product sheet
<https://ies-ows.jrc.ec.europa.eu/gtb/GTB/psheets/GTB-RestorationPlanner.pdf>`_.


Connectivity Indices
--------------------

RSS computes the following indices:

.. list-table:: RSS connectivity indices.
   :header-rows: 1
   :widths: 35 15 15 35

   * - Full Name
     - Code
     - Unit
     - Description
   * - Critical New Object Area
     - CNOA
     - pixels
     - Minimum area of a new patch that would increase connectivity
   * - Equivalent Connected Area
     - ECA
     - pixels
     - Area of a single patch providing the same connectivity as observed
   * - Reference Area Coverage
     - RAC
     - %
     - Percentage of foreground relative to total foreground and background
   * - Coherence
     - COH
     - %
     - Percentage of foreground pixels effectively connected
   * - Restoration Potential
     - REST_POT
     - %
     - Percentage of foreground pixels that could improve connectivity (100 - COH)


Usage
-----

.. code-block:: python

    import pyguidos as pg

    result = pg.rss(
        in_tiff="my_map.tif",
        outdir="output/",
        stat_files=True,
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
   * - ``outdir``
     - str or Path
     - None
     - Output directory
   * - ``stat_files``
     - bool
     - True
     - Write statistics to files
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
   * - ``<name>_rss.txt``
     - Statistics report with all connectivity indices


Results
-------

The :func:`rss` function returns a :class:`dict`. The structure is nested as follows:

* **output paths** (:class:`dict` or :obj:`None`)
    * **path txt** (:class:`str`): Absolute path to the comprehensive statistics report.
    * *Note: This key is* ``None`` *if* ``stat_files=False``.

* **input stats** (:class:`dict`)
    * **foreground pxl** (:class:`int`): Count of pixels with value 2 (Forest).
    * **background pxl** (:class:`int`): Count of pixels with value 1 (Background).
    * **missing pxl** (:class:`int`): Count of NoData (0) pixels.
    * **backgr3 pxl** (:class:`int`): Count of special background class 3 pixels.
    * **backgr4 pxl** (:class:`int`): Count of special background class 4 pixels.

* **output stats** (:class:`dict`)
    * **total patches** (:class:`int`): The total number of discrete patches identified in the landscape.
    * **average patch size** (:class:`float`): The mean size of patches (usually in pixel units).
    * **median patch size** (:class:`float`): The median size of patches.
    * **largest patch size** (:class:`int`): The size of the largest single patch found.
    * **CNOA** (:class:`float`): Component-wise Normalized Occupied Area.
    * **ECA** (:class:`float`): Equivalent Connected Area.
    * **RAC** (:class:`float`): Relative Area of Connectivity.
    * **COH** (:class:`float`): Cohesion index.
    * **REST_POT** (:class:`float`): Restoration Potential index.

.. code-block:: python

    result = pg.rss("my_map.tif")

    # Access statistics
    print(result.keys())
    # dict_keys(['output paths', 'input stats', 'output stats'])

    # Input pixel counts
    print(result["input stats"])
    # {'foreground pxl': 12500, 'background pxl': 37500,
    #  'missing pxl': 0, 'backgr3 pxl': 0, 'backgr4 pxl': 0}

    # Connectivity indices
    print(result["output stats"])
    # {'total patches': 142, 'average patch size': 88.0,
    #  'median patch size': 12.0, 'largest patch size': 8542,
    #  'ECA': 8764.3, 'COH': 70.1, 'CNOA': 3, 'REST_POT': 29.9, 'RAC': 25.0}

    # Output file paths
    print(result["output paths"])
    # {'path txt': 'output/my_map_rss.txt'}


Patch Statistics
----------------

In addition to the connectivity indices, RSS also reports basic patch
size statistics:

- **Total patches**: total number of foreground patches
- **Average patch size**: mean patch size in pixels
- **Median patch size**: median patch size in pixels
- **Largest patch size**: size of the largest patch in pixels
