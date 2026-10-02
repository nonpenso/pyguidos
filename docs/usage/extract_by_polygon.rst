Extract by Polygon
==================

The ``extract_by_polygon()`` function extracts and saves a separate
GeoTIFF for each polygon feature in a vector file, clipping and masking
the input raster to each polygon's extent and shape. It is particularly
useful for batch processing a pyGuidos (or GTB) output map over multiple
study areas such as countries, administrative regions or protected areas.

The function preserves the original colour palette and metadata from the
input GeoTIFF, so all downstream pyGuidos tools can be applied directly
to the extracted outputs.

.. note::
    ``extract_by_polygon()`` is a utility that writes clipped rasters to
    disk; it does not return a statistics dictionary.


Functions
---------

.. py:function:: pyguidos.extract_by_polygon(vector_path, geotiff_path, output_dir, id_field, name_prefix="", nodata_value=None, layer=None)

   Extracts and saves a separate GeoTIFF for each polygon feature in a
   vector file, clipping and masking the input raster to each polygon's
   extent and shape. The original colormap and GTB metadata tags from the
   input GeoTIFF are preserved on every output.

   :param vector_path: Path to the input vector file with polygon features. Supported formats: ESRI Shapefile (``.shp``), GeoPackage (``.gpkg``), GeoJSON (``.geojson``/``.json``), KML (``.kml``), FlatGeobuf (``.fgb``), ESRI FileGDB (``.gdb``).
   :type vector_path: str or Path
   :param geotiff_path: Path to the input GeoTIFF raster to extract from.
   :type geotiff_path: str or Path
   :param output_dir: Directory where output GeoTIFFs are saved. Created if it does not exist.
   :type output_dir: str or Path
   :param id_field: Attribute field name used to generate output filenames (e.g. ``"NAME"``, ``"ISO3"``). Falls back to ``feature_<index>`` if the field is not present in a feature.
   :type id_field: str
   :param name_prefix: Optional prefix prepended to each output filename. Default ``""`` (no prefix).
   :type name_prefix: str, optional
   :param nodata_value: Value assigned to pixels outside the polygon mask. If ``None`` (default), it is resolved from the GTB tag, then the TIFF nodata header, otherwise ``0``.
   :type nodata_value: int, optional
   :param layer: Name of the layer to read from multi-layer vector files (e.g. GeoPackage, FileGDB). If ``None`` (default), the first layer is read; if multiple layers exist and no layer is given, the function exits with an error listing the available layers.
   :type layer: str, optional

   :returns: **None** -- Output GeoTIFFs are written directly to ``output_dir``. Skipped or failed features are reported to stdout.

   :outputs: The function writes one GeoTIFF per polygon feature:

       * ``<output_dir>/<name_prefix><id_field_value>.tif``: Clipped and masked GeoTIFF for each polygon feature. In each filename, spaces in the ``id_field`` value are replaced with underscores and forward slashes with hyphens.

   .. rubric:: Example
   Extract a pyGuidos output map for each country polygon in a shapefile,
   prefixing every output filename with ``country_``:

   .. code-block:: python

        >>> import pyguidos as pg
        >>> pg.extract_by_polygon(
        ... vector_path="countries.shp", geotiff_path="europe_mspa.tif",
        ... output_dir="output/countries/", id_field="NAME",
        ... name_prefix="country_", nodata_value=None, layer=None)
        >>> # Output files: output/countries/country_France.tif,
        >>> #               output/countries/country_Germany.tif, ...


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
