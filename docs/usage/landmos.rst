Landscape Mosaic
================

The Landscape Mosaic analysis classifies each pixel based on the
proportional composition of three land cover classes within a moving
window. The result describes the local landscape context of each pixel
in terms of the dominant land cover mixture, producing up to 103
compositional classes subsequently remapped to 19 aggregated classes.
The methodology is described in detail in
`LINK_TO_LM_PAPER <https://LINK_TO_LM_PAPER>`_.


Landscape Mosaic Classes
------------------------

The 19 aggregated classes describe the dominant land cover mixture
within the moving window:

.. list-table::
   :header-rows: 1

   * - Class
     - Description
   * - **A**
     - Dominated by Class 1 (> 90%)
   * - **N**
     - Dominated by Class 2 (> 90%)
   * - **D**
     - Dominated by Class 3 (> 90%)
   * - **An, Ad, Na, Nd, Dn, Da**
     - Secondary transition zones (60-90%)
   * - **Adn, Nad, Dan**
     - Tertiary transition zones (40-60%)
   * - **an, ad, dn**
     - Mid-transition zones
   * - **adn**
     - Mixed centre zone
   * - **AA, NN, DD**
     - Pure corners (> 90% single class)
   * - **NoData**
     - No data


Usage
-----

.. code-block:: python

    import pyguidos as pg

    result = pg.landmos(
        in_tiff="my_landcover.tif",
        window_size=33,
        outdir="output/",
        statists=True,
        stat_files=True,
        out_colors='bgr',
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
   * - ``out_colors``
     - str
     - ``'bgr'``
     - Color scheme for the 103-class output colormap
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
   * - ``<name>_lm_<window_size>_103class.tif``
     - 103-class landscape mosaic result GeoTIFF
   * - ``<name>_lm_<window_size>.tif``
     - 19-class remapped result GeoTIFF
   * - ``<name>_lm_<window_size>.txt``
     - Statistics report
   * - ``<name>_lm_<window_size>.csv``
     - Per-value pixel counts and frequencies
   * - ``<name>_lm_<window_size>_heatmap.csv``
     - Ternary diagram data table
   * - ``<name>_lm_<window_size>_heatmap.png``
     - Ternary diagram heatmap


Result Object
-------------

:func:`landmos` returns a :class:`LandMosResult` dataclass:

.. code-block:: python

    result = pg.landmos("my_landcover.tif", window_size=33)

    # Access statistics
    print(result.stats.keys())
    # dict_keys(['output paths', 'input stats', 'output stats'])

    # Input pixel counts
    print(result.stats["input stats"])
    # {'class1 pxl': 15000, 'class2 pxl': 20000, 'class3 pxl': 10000,
    #  'foreground pxl': 45000, 'missing pxl': 0}

    # Pixel counts for both 103-class and 19-class outputs
    print(result.stats["output stats"].keys())
    # dict_keys(['pxl numb 103cl', 'pxl numb 19cl'])

    # Output file paths
    print(result.stats["output paths"])
    # {'path tif': 'output/my_landcover_lm_33_103class.tif',
    #  'path txt': 'output/my_landcover_lm_33.txt',
    #  'path csv': 'output/my_landcover_lm_33.csv',
    #  'path csv hm': 'output/my_landcover_lm_33_heatmap.csv',
    #  'path png': 'output/my_landcover_lm_33_heatmap.png'}

.. note::
    ``result.stats["output paths"]`` is ``None`` when ``stat_files=False``.
    All other keys are always populated regardless of ``stat_files``.


Computing Statistics Separately
--------------------------------

If you already have a Landscape Mosaic output GeoTIFF, you can compute
statistics without rerunning the analysis:

.. code-block:: python

    stats = pg.landmos_stats(
        lm_tiff="output/my_landcover_lm_33_103class.tif",
        outfile=True,
        outdir="output/",
        source_tiff="my_landcover.tif"
    )

.. note::
    :func:`landmos_stats` requires the input GeoTIFF to contain a valid
    ``GTB_LM`` metadata tag. See :doc:`input_format` for details.


Window Size
-----------

The ``window_size`` parameter controls the scale of the landscape
context analysis:

- Larger windows capture broader landscape patterns but smooth out
  local variation
- Smaller windows are more sensitive to fine-scale spatial heterogeneity
- The window size should reflect a meaningful ecological neighbourhood
  for your application

.. tip::
    For landscape-scale analysis at 25m resolution, a window size of
    33 pixels corresponds to a 825m x 825m neighbourhood (approximately
    68 hectares).


References
----------

CITATION_PLACEHOLDER
