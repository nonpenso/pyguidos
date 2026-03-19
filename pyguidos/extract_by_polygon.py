import os
from pathlib import Path

import fiona
import rasterio
from rasterio.mask import mask as rio_mask
from rasterio.crs import CRS
from rasterio.transform import Affine
from rasterio.enums import ColorInterp
from shapely.geometry import shape, mapping, Polygon, MultiPolygon
from pyproj import Transformer

from . import utils


SUPPORTED_VECTOR_FORMATS = {
    '.shp', '.gpkg', '.geojson', '.json', '.kml', '.fgb', '.gdb'
}

def extract_by_polygon(
    vector_path: str,
    geotiff_path: str,
    output_dir: str,
    id_field: str,
    name_prefix: str = "",
    nodata_value: int = None
    ) -> None:
    """
    Extracts and saves a separate GeoTIFF for each polygon feature in a
    shapefile, clipping and masking the input raster to each polygon's
    extent and shape. Preserves the original colormap and GTB metadata
    tags from the input GeoTIFF. Automatically reprojects polygon
    geometries to the raster CRS if needed.

    Parameters
    ----------
    vector_path : str or Path
        Path to the input vector file containing polygon features.
        Supported formats: ESRI Shapefile (.shp), GeoPackage (.gpkg),
        GeoJSON (.geojson), KML (.kml), FlatGeobuf (.fgb),
        ESRI FileGDB (.gdb).
    geotiff_path : str or Path
        Path to the input GeoTIFF raster to extract from.
    output_dir : str or Path
        Directory where output GeoTIFFs will be saved. Created if it
        does not exist.
    id_field : str
        Attribute field name used to generate output filenames
        (e.g. "NAME", "ISO3"). Falls back to "feature_<index>" if the
        field is not found in a feature's properties.
    name_prefix : str, optional
        Optional prefix string prepended to each output filename.
        Default "" (no prefix).
    nodata_value : int, optional
        Value assigned to pixels outside the polygon mask. If None
        (default), the value is automatically resolved from the GTB tag
        of the input GeoTIFF, or from the tiff nodata header, or
        defaults to 0 if neither is available.

    Returns
    -------
    None
        Output GeoTIFFs are written directly to output_dir. Skipped or
        failed features are reported to stdout.

    Output Files
    ------------
    - <output_dir>/<name_prefix><id_field_value>.tif : one per polygon feature
    """
    os.makedirs(output_dir, exist_ok=True)

    # Check the supported vector files
    vector_path = Path(vector_path)
    if vector_path.suffix.lower() not in SUPPORTED_VECTOR_FORMATS:
        sys.exit(f"ERROR: Vector format '{vector_path.suffix}' is not supported. "
                 f"Supported formats: {sorted(SUPPORTED_VECTOR_FORMATS)}")

    # Resolve nodata value
    if nodata_value is None:
        nodata_value = utils.get_gtb_nodata(geotiff_path)

    with rasterio.open(geotiff_path) as src:

        # Check raster CRS
        raster_crs = src.crs
        if raster_crs is None:
            sys.exit("ERROR: Input GeoTIFF has not a defined Projection. "
                     "Please assign a coordinate reference system before using extract_by_polygon().")

        res_x, res_y = src.res
        in_tags = src.tags()
        tag_descr = in_tags.get('TIFFTAG_IMAGEDESCRIPTION') or '--'
        try:
            cmap = src.colormap(1)
        except ValueError:
            cmap = None

        with fiona.open(str(vector_path), "r") as vector:

            # Check vector CRS
            if not vector.crs_wkt:
                sys.exit("ERROR: Input vector file has not a defined Projection. "
                         "Please assign a coordinate reference system before using extract_by_polygon().")

            vector_crs = CRS.from_wkt(vector.crs_wkt)

            # Check if reprojection is needed
            needs_reproject = raster_crs != vector_crs
            transformer = None
            if needs_reproject:
                transformer = Transformer.from_crs(vector_crs, raster_crs, always_xy=True)

            for idx, feature in enumerate(vector):
                # --- Determine output filename ---
                if id_field in feature["properties"]:
                    # Sanitize the field value for use as a filename
                    outname = str(feature["properties"][id_field]).replace(" ", "_").replace("/", "-")
                else:
                    outname = f"feature_{idx}"

                output_path = os.path.join(output_dir, f"{name_prefix}{outname}.tif")

                # --- Get polygon geometry ---
                geom = shape(feature["geometry"])

                if not geom.is_valid:
                    # Attempt the repair
                    fixed_geom = geom.buffer(0)

                    # Check if the repair resulted in a valid, non-empty geometry
                    if fixed_geom.is_valid and not fixed_geom.is_empty:
                        # Safety check: ensure it is a polygon type (not a Point or Line)
                        if isinstance(fixed_geom, (Polygon, MultiPolygon)):
                            geom = fixed_geom  # Repair worked, update geom silently
                        else:
                            print(f"  [SKIP] Feature '{outname}': Repair resulted in {fixed_geom.geom_type}. Skipping.")
                            continue
                    else:
                        print(f"  [ERROR] Feature '{outname}': Unfixable geometry or empty after repair. Skipping.")
                        continue
                else:
                    # If it was valid but empty
                    if geom.is_empty:
                        print(f"  [SKIP] Feature '{outname}': Geometry is empty. Skipping.")
                        continue

                # --- Reproject geometry to raster CRS if needed ---
                if needs_reproject:
                    geom = _reproject_geometry(geom, transformer)

                # --- Check bounding box intersects raster ---
                raster_bounds = src.bounds
                if not _bounds_intersect(geom.bounds, raster_bounds):
                    print(f"  [SKIP] Feature '{outname}': geometry does not intersect raster extent.")
                    continue

                # --- Mask raster with polygon ---
                try:
                    out_image, out_transform = rio_mask(
                        src,
                        [mapping(geom)],   # mask expects GeoJSON-like dicts
                        crop=True,         # crop to polygon bounding box
                        nodata=nodata_value,
                        all_touched=False, # only pixels whose center falls inside
                        filled=True,       # fill masked areas with nodata
                    )
                except Exception as e:
                    print(f"  [ERROR] Feature '{outname}': {e}")
                    continue

                # out_image shape: (bands, rows, cols)
                if out_image.size == 0:
                    print(f"  [SKIP] Feature '{outname}': masked result is empty.")
                    continue

                # --- Snap transform to the input grid ---
                off_x = round((out_transform[2] - src.transform[2]) / res_x) * res_x
                off_y = round((out_transform[5] - src.transform[5]) / res_y) * res_y
                snapped_transform = Affine(
                    out_transform[0], out_transform[1], src.transform[2] + off_x,
                    out_transform[3], out_transform[4], src.transform[5] + off_y
                )

                # --- Write output GeoTIFF ---
                out_meta = src.meta.copy()
                meta_params = {
                        "driver": "GTiff",
                        "height": out_image.shape[1],
                        "width": out_image.shape[2],
                        "transform": snapped_transform,
                        "nodata": None,
                        "compress": "lzw",  # lossless compression for uint8
                        "tiled": True,
                        "blockxsize": 256,
                        "blockysize": 256
                    }
                if cmap is not None:
                    meta_params["photometric"] = "palette"

                out_meta.update(meta_params)
                tags = {
                    "TIFFTAG_IMAGEDESCRIPTION": tag_descr,
                    "TIFFTAG_SOFTWARE": "pyGuidos"
                }
                with rasterio.open(output_path, "w", **out_meta) as dst:
                    dst.write(out_image)
                    if cmap:
                        dst.colorinterp = [ColorInterp.palette]
                        dst.write_colormap(1, cmap)
                    dst.update_tags(**tags)




# ---------------------------------------------------------------------------
# Helper functions
# ---------------------------------------------------------------------------

def _reproject_geometry(geom, transformer):
    """Reproject a Shapely geometry using a pyproj Transformer."""
    from shapely.ops import transform
    return transform(transformer.transform, geom)


def _bounds_intersect(bounds_a, bounds_b):
    """Return True if two bounding boxes (minx, miny, maxx, maxy) intersect."""
    return not (
        bounds_a[2] < bounds_b.left
        or bounds_a[0] > bounds_b.right
        or bounds_a[3] < bounds_b.bottom
        or bounds_a[1] > bounds_b.top
    )

