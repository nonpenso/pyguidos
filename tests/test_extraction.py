import pytest
import numpy as np
import rasterio
import fiona
from shapely.geometry import Polygon, mapping
from rasterio.transform import from_origin
from pyproj import Transformer
from pyguidos.extract_by_polygon import extract_by_polygon

@pytest.fixture
def spatial_data(tmp_path):
    """Creates a 3035 Raster and two vectors (one 3035, one 4326)."""
    # 1. Create Raster (EPSG:3035) - 100x100m
    raster_path = tmp_path / "source_3035.tif"
    x_min, y_max = 4000000, 3000000
    transform = from_origin(x_min, y_max, 1, 1)
    
    with rasterio.open(
        raster_path, 'w', driver='GTiff', height=100, width=100,
        count=1, dtype='uint8', crs='EPSG:3035', transform=transform
    ) as dst:
        dst.write(np.ones((100, 100), dtype=np.uint8), 1)
        dst.update_tags(TIFFTAG_IMAGEDESCRIPTION="GTB_MSPA;1;1;1;1;1;1")

    # 2. Create Vector 1: EPSG:3035 (Matching CRS)
    vector_3035 = tmp_path / "zones_3035.gpkg"
    # Small 30x30m box inside the raster
    poly_in = Polygon([(4000010, 2999990), (4000040, 2999990), (4000040, 2999960), (4000010, 2999960)])
    # Polygon completely outside (at 0,0)
    poly_out = Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
    
    schema = {'geometry': 'Polygon', 'properties': {'name': 'str'}}
    with fiona.open(vector_3035, 'w', driver='GPKG', crs='EPSG:3035', schema=schema) as layer:
        layer.write({'geometry': mapping(poly_in), 'properties': {'name': 'Inside'}})
        layer.write({'geometry': mapping(poly_out), 'properties': {'name': 'FarAway'}})

    # 3. Create Vector 2: EPSG:4326 (Reprojection Case)
    vector_4326 = tmp_path / "zones_4326.gpkg"
    transformer = Transformer.from_crs("EPSG:3035", "EPSG:4326", always_xy=True)
    
    # Transform the coordinates of poly_in to WGS84
    coords_4326 = [transformer.transform(x, y) for x, y in poly_in.exterior.coords]
    poly_4326 = Polygon(coords_4326)
    
    with fiona.open(vector_4326, 'w', driver='GPKG', crs='EPSG:4326', schema=schema) as layer:
        layer.write({'geometry': mapping(poly_4326), 'properties': {'name': 'Reprojected'}})

    return raster_path, vector_3035, vector_4326, tmp_path / "output"

def test_comprehensive_extraction(spatial_data):
    """Tests all extraction logic: matching CRS, reprojection, and skipping OOB."""
    raster_path, vec_3035, vec_4326, out_dir = spatial_data
    out_dir.mkdir(exist_ok=True)

    # --- PART A: Test Same CRS & Out-of-Bounds Skip ---
    extract_by_polygon(str(vec_3035), str(raster_path), str(out_dir), id_field="name")
    
    assert (out_dir / "Inside.tif").exists(), "Inside.tif should have been created"
    assert not (out_dir / "FarAway.tif").exists(), "FarAway.tif should have been skipped"

    # --- PART B: Test Reprojection (4326 to 3035) ---
    extract_by_polygon(str(vec_4326), str(raster_path), str(out_dir), id_field="name")
    
    reproj_file = out_dir / "Reprojected.tif"
    assert reproj_file.exists(), "Reprojected.tif should have been created via CRS transformation"
    
    with rasterio.open(reproj_file) as src:
        # Output should be in the Raster's CRS
        assert src.crs.to_epsg() == 3035
        # Ensure it contains data
        assert np.max(src.read(1)) == 1

def test_extraction_edge_cases(spatial_data):
    raster_path, vec_3035, _, out_dir = spatial_data
    if not out_dir.exists():
        out_dir.mkdir()
    
    # CASE 1: Empty vector file
    empty_vec = out_dir.parent / "empty.gpkg"
    schema = {'geometry': 'Polygon', 'properties': {'name': 'str'}}
    with fiona.open(empty_vec, 'w', driver='GPKG', crs='EPSG:3035', schema=schema) as layer:
        pass 
    
    # Verify it completes without error
    extract_by_polygon(str(empty_vec), str(raster_path), str(out_dir), "name")

    # CASE 2: Wrong ID field
    extract_by_polygon(str(vec_3035), str(raster_path), str(out_dir), id_field="wrong_column_name")
    
    # Check if it created a default file (like 'None.tif' or '0.tif')
    generated_files = list(out_dir.glob("*.tif"))
    print(f"Files generated with bad ID: {[f.name for f in generated_files]}")

def test_extraction_no_overlap_logic(spatial_data):
    """Targets the skip logic for features that don't overlap the raster."""
    raster_path, _, _, out_dir = spatial_data
    
    # Ensure the directory exists so fiona can create the database file
    out_dir.mkdir(parents=True, exist_ok=True)
    
    # Create a polygon in the middle of the ocean (0,0)
    off_map_vec = out_dir / "off_map.gpkg"
    schema = {'geometry': 'Polygon', 'properties': {'id': 'int'}}
    
    with fiona.open(off_map_vec, 'w', driver='GPKG', crs='EPSG:4326', schema=schema) as layer:
        layer.write({
            'geometry': {'type': 'Polygon', 'coordinates': [[(-10,-10), (-11,-10), (-11,-11), (-10,-11), (-10,-10)]]},
            'properties': {'id': 999}
        })
    
    from pyguidos.extract_by_polygon import extract_by_polygon
    extract_by_polygon(str(off_map_vec), str(raster_path), str(out_dir), "id")
    
    # Assert that no TIF was created for the non-overlapping ID
    assert not (out_dir / "999.tif").exists()

def test_extraction_geometry_robustness(spatial_data):
    """Targets the repair and skip logic for messy geometry."""
    raster_path, _, _, out_dir = spatial_data
    out_dir.mkdir(parents=True, exist_ok=True)
    
    from pyguidos.extract_by_polygon import extract_by_polygon
    
    # --- GET REAL RASTER BOUNDS ---
    with rasterio.open(raster_path) as src:
        left, bottom, right, top = src.bounds
        # Place polygons safely inside the raster footprint
        x, y = left + 10, bottom + 10

    messy_vec = out_dir / "invalid_fixable.gpkg"
    schema = {'geometry': 'Polygon', 'properties': {'name': 'str'}}
    
    with fiona.open(messy_vec, 'w', driver='GPKG', crs='EPSG:3035', schema=schema) as layer:
        # 1. Self-intersecting "Bow-tie" (Invalid but fixable)
        layer.write({
            'geometry': {'type': 'Polygon', 'coordinates': [[(x,y), (x+50,y+50), (x+50,y), (x,y+50), (x,y)]]},
            'properties': {'name': 'FixedBowTie'}
        })
        # 2. Valid polygon (Testing naming 'else' branch)
        layer.write({
            'geometry': {'type': 'Polygon', 'coordinates': [[(x+60,y+60), (x+80,y+60), (x+80,y+80), (x+60,y+80), (x+60,y+60)]]},
            'properties': {'name': 'DefaultNameTest'}
        })

    # Trigger naming 'else' branch by looking for a column that doesn't exist
    extract_by_polygon(str(messy_vec), str(raster_path), str(out_dir), id_field="non_existent")

    # Verification: Files MUST exist now because coordinates are derived from the raster bounds
    assert (out_dir / "feature_0.tif").exists() 
    assert (out_dir / "feature_1.tif").exists()