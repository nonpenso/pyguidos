Extract by Polygon
==================

The :func:`extract_by_polygon` function extracts and saves a separate
GeoTIFF for each polygon feature in a shapefile, clipping and masking
the input raster to each polygon's extent and shape. It is particularly
useful for batch processing a pyGuidos (or GTB) output map over multiple 
study areas such as countries, administrative regions or protected areas.

The function preserves the original colour palette and metadata from the 
input GeoTIFF, so all downstream pyGuidos tools can be applied directly 
to the extracted outputs.


Usage
-----

.. code-block:: python

    import pyguidos as pg

    pg.extract_by_polygon(
        vector_path="regions.gpkg",
        geotiff_path="my_map.tif",
        output_dir="output/",
        id_field="NAME",
        name_prefix="region_",
        nodata_value=None,
        layer=None
    )


Parameters
----------

.. list-table::
   :header-rows: 1

   * - Parameter
     - Type
     - Default
     - Description
   * - ``vector_path``
     - str or Path
     - --
     - Path to input vector file (.json, .shp, .kml, .gpkg, etc.)
   * - ``geotiff_path``
     - str or Path
     - --
     - Path to input GeoTIFF raster
   * - ``output_dir``
     - str or Path
     - --
     - Output directory, created if it does not exist
   * - ``id_field``
     - str
     - --
     - Attribute field used to name output files
   * - ``name_prefix``
     - str
     - None
     - Optional prefix prepended to each output filename
   * - ``nodata_value``
     - int
     - None
     - Value for pixels outside the polygon mask
   * - ``layer``
     - str
     - None
     - Layer name for multi-layer vector files (e.g., GeoPackage, FileGDB).
       If None, reads the first layer. Exits with an error if multiple
       layers are detected and this parameter is not specified.


Output Files
------------

One GeoTIFF per polygon feature:

.. list-table::
   :header-rows: 1

   * - File
     - Description
   * - ``<output_dir>/<name_prefix><id_field_value>.tif``
     - Clipped and masked GeoTIFF for each polygon feature


NoData Handling
---------------

The ``nodata_value`` parameter controls what value is assigned to pixels
outside the polygon mask. If ``None`` (default), the value is automatically
resolved using a three-level priority:

1. **GTB output**: uses the GTB convention nodata value
   for that tool (e.g. 129 for MSPA, 102 for Fragmentation, 0 for
   Landscape Mosaic)
2. **Non-GTB, nodata not set**: uses 0
3. **Non-GTB, nodata set**: uses the tiff's own nodata value

.. note::
    pyGuidos output GeoTIFFs do not set nodata in the TIFF header --
    they use a specific pixel value by convention. The automatic
    resolution ensures the correct value is used for each tool output
    without requiring the user to know it explicitly.


Geometry Handling
-----------------

The function automatically handles several geometry issues:

- **Invalid geometries** are repaired before processing
- **Empty geometries** are skipped with a warning message
- **Geometries outside the raster extent** are skipped with a warning
  message
- **Bounding box mismatch** between vector and raster is detected
  and the function exits with an error if they do not overlap

.. warning::
    The vector file and the raster file must share the same coordinate
    reference system. If they do not overlap spatially, the function
    will exit with an error asking you to verify both CRS.

.. tip::
    The ``id_field`` value is used as the output filename. Spaces are
    replaced with underscores and forward slashes with hyphens. If the
    field is not found in a feature, the filename falls back to
    ``feature_<index>``.


Example with prefix
-------------------

.. code-block:: python

    # Extract MSPA results for each country
    # Output files: country_France.tif, country_Germany.tif, ...
    pg.extract_by_polygon(
        vector_path="countries.shp",
        geotiff_path="europe_mspa.tif",
        output_dir="output/countries/",
        id_field="NAME",
        name_prefix="country_"
    )
