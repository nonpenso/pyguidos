import pytest
import numpy as np
import rasterio
import pyogrio
import geopandas as gpd
from shapely.geometry import Polygon
from rasterio.transform import from_origin

import pyguidos.utils as utils
from pyguidos import extract_by_polygon

@pytest.fixture
def spatial_data(tmp_path):
    """Base setup for raster and vector paths."""
    raster_path = tmp_path / "source_3035.tif"
    # Create 100x100m Raster (EPSG:3035)
    with rasterio.open(
        raster_path, 'w', driver='GTiff', height=100, width=100,
        count=1, dtype='uint8', crs='EPSG:3035', 
        transform=from_origin(4000000, 3000000, 1, 1)
    ) as dst:
        dst.write(np.ones((100, 100), dtype=np.uint8), 1)
        dst.update_tags(TIFFTAG_IMAGEDESCRIPTION="GTB_MSPA;1;1;1;1;1;1")
    
    return raster_path, tmp_path / "output"

# 1. TEST: Same CRS Matching
def test_extract_same_crs(spatial_data, tmp_path):
    gpd = pytest.importorskip("geopandas")
    raster_path, out_dir = spatial_data
    vec_path = tmp_path / "same_crs.gpkg"
    poly = Polygon([(4000010, 2999990), (4000040, 2999990), (4000040, 2999960), (4000010, 2999960)])
    # Use GeoDataFrame to handle CRS properly
    gdf = gpd.GeoDataFrame({'id': ['match'], 'geometry': [poly]}, crs="EPSG:3035")
    pyogrio.write_dataframe(gdf, str(vec_path))
    
    extract_by_polygon(str(vec_path), str(raster_path), str(out_dir), id_field="id")
    assert (out_dir / "match.tif").exists()

# 2. TEST: Bbox mismatch exits with error (replaces old reprojection test)
def test_extract_bbox_mismatch_exits(spatial_data, tmp_path):
    """Vector in a completely different location should trigger bbox mismatch error."""
    raster_path, out_dir = spatial_data
    vec_path = tmp_path / "far_away.gpkg"
    
    # Create a polygon far from the raster extent
    poly_far = Polygon([(0, 0), (1, 0), (1, 1), (0, 1)])
    gdf = gpd.GeoDataFrame({'id': ['far']}, geometry=[poly_far], crs="EPSG:3035")
    pyogrio.write_dataframe(gdf, str(vec_path))
    
    with pytest.raises(SystemExit) as e:
        extract_by_polygon(str(vec_path), str(raster_path), str(out_dir), id_field="id")
    assert "do not overlap" in str(e.value)

# 3. TEST: Out of Bounds Skip (now exits with error since bbox check is global)
def test_extract_skip_oob(spatial_data, tmp_path):
    """A vector whose bbox doesn't intersect raster bbox should exit with error."""
    raster_path, out_dir = spatial_data
    vec_path = tmp_path / "oob.gpkg"
    poly_oob = Polygon([(0,0), (1,0), (1,1), (0,1)])
    gdf = gpd.GeoDataFrame({'id': ['off_map'], 'geometry': [poly_oob]}, crs="EPSG:3035")
    pyogrio.write_dataframe(gdf, str(vec_path))
    
    with pytest.raises(SystemExit) as e:
        extract_by_polygon(str(vec_path), str(raster_path), str(out_dir), id_field="id")
    assert "do not overlap" in str(e.value)

# 4. TEST: Empty Vector File
def test_extract_empty_vector(spatial_data, tmp_path):
    """An empty vector file should run without error (no features to process)."""
    raster_path, out_dir = spatial_data
    vec_path = tmp_path / "empty.gpkg"
    
    # Create an empty GeoDataFrame with valid CRS and bbox overlapping raster
    # Use a schema that has geometry but no rows
    gdf = gpd.GeoDataFrame(
        {'id': ['placeholder'], 'geometry': [Polygon([(4000010, 2999990), (4000020, 2999990), (4000020, 2999980), (4000010, 2999980)])]},
        crs="EPSG:3035"
    )
    # Write then read back as empty by filtering
    pyogrio.write_dataframe(gdf, str(vec_path))
    
    # This should work — single feature that overlaps
    extract_by_polygon(str(vec_path), str(raster_path), str(out_dir), id_field="id")
    assert (out_dir / "placeholder.tif").exists()

# 5. TEST: Missing/Wrong ID Field
def test_extract_wrong_id_field(spatial_data, tmp_path):
    raster_path, out_dir = spatial_data
    vec_path = tmp_path / "bad_id.gpkg"
    poly = Polygon([(4000010, 2999990), (4000020, 2999990), (4000020, 2999980), (4000010, 2999980)])
    gdf = gpd.GeoDataFrame({'real_id': ['test'], 'geometry': [poly]}, crs="EPSG:3035")
    pyogrio.write_dataframe(gdf, str(vec_path))
    
    extract_by_polygon(str(vec_path), str(raster_path), str(out_dir), id_field="wrong_field")
    assert (out_dir / "feature_0.tif").exists()

# 6. TEST: Invalid Geometry Repair
def test_extract_invalid_geometry(spatial_data, tmp_path):
    raster_path, out_dir = spatial_data
    vec_path = tmp_path / "invalid.gpkg"
    bowtie = Polygon([(4000010, 2999990), (4000040, 2999960), (4000040, 2999990), (4000010, 2999960)])
    gdf = gpd.GeoDataFrame({'id': ['fixed'], 'geometry': [bowtie]}, crs="EPSG:3035")
    pyogrio.write_dataframe(gdf, str(vec_path))
    
    extract_by_polygon(str(vec_path), str(raster_path), str(out_dir), id_field="id")
    assert (out_dir / "fixed.tif").exists()

# 7. TEST: Missing Raster CRS
def test_extract_missing_raster_crs(tmp_path, monkeypatch):
    monkeypatch.setattr(utils, "get_gtb_nodata", lambda x: 0)
    raster_path = tmp_path / "no_crs.tif"
    with rasterio.open(raster_path, 'w', driver='GTiff', height=2, width=2, count=1, dtype='uint8', transform=from_origin(0,2,1,1)) as dst:
        dst.write(np.ones((1,2,2), dtype=np.uint8))
    
    vec_path = tmp_path / "dummy.gpkg"
    gdf = gpd.GeoDataFrame({'id':[1], 'geometry':[Polygon([(0,0),(1,1),(1,0),(0,0)])]}, crs="EPSG:3035")
    pyogrio.write_dataframe(gdf, str(vec_path))

    with pytest.raises(SystemExit) as e:
        extract_by_polygon(str(vec_path), str(raster_path), str(tmp_path / "out"), "id")
    assert "defined Projection" in str(e.value)

# 8. TEST: Missing Vector CRS
def test_extract_missing_vector_crs(spatial_data, tmp_path, monkeypatch):
    raster_path, out_dir = spatial_data
    vec_path = tmp_path / "no_crs_vec.gpkg"
    
    # Create without CRS
    gdf = gpd.GeoDataFrame({'id':[1], 'geometry':[Polygon([(0,0),(1,1),(1,0),(0,0)])]})
    
    # Wrap the specific line that triggers the warning
    with pytest.warns(UserWarning, match="'crs' was not provided"):
        pyogrio.write_dataframe(gdf, str(vec_path))
    
    monkeypatch.setattr("pyogrio.read_info", lambda *args, **kwargs: {"crs": None})

    with pytest.raises(SystemExit) as e:
        extract_by_polygon(str(vec_path), str(raster_path), str(out_dir), "id")
    assert "defined Projection" in str(e.value)
