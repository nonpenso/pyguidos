import pytest
import numpy as np
import rasterio
from pyguidos import landmos, landmos_stats

@pytest.fixture(scope="module")
def lm_result(tmp_path_factory):
    """
    Fixture that runs Landscape Mosaic once.
    Requires values: 0 (NoData), 1 (Class 1), 2 (Class 2), 3 (Class 3).
    """
    tmp_dir = tmp_path_factory.mktemp("lm_data")
    input_tif = tmp_dir / "input_lm.tif"
    
    # 1. Create 11x11 dummy data with 3 classes
    # 1=Agri, 2=Natural, 3=Developed
    data = np.ones((11, 11), dtype=np.uint8)
    data[0:5, :] = 1  # Top half Class 1
    data[5:10, :] = 2 # Bottom half Class 2
    data[10, :] = 3   # Last row Class 3
    data[0, 0] = 0    # Missing pixel
    
    # Georeference to avoid warnings
    from rasterio.transform import from_origin
    with rasterio.open(
        input_tif, 'w', driver='GTiff',
        height=11, width=11, count=1, dtype='uint8',
        crs='EPSG:3035',
        transform=from_origin(0, 11, 1, 1)
    ) as dst:
        dst.write(data, 1)

    # 2. Run Land Mosaic
    return landmos(
        str(input_tif), 
        window_size=3, 
        return_array=True, 
        stat_files=True
    )

def test_lm_outputs_exist(lm_result):
    """Verifies that both 103-class and 19-class files are created."""
    paths = lm_result.stats["output paths"]
    from pathlib import Path
    
    assert Path(paths["path tif 103cl"]).exists()
    assert Path(paths["path tif 19cl"]).exists()
    assert Path(paths["path txt"]).exists()
    assert Path(paths["path png"]).exists() # Ternary Heatmap

def test_lm_array_logic(lm_result):
    """Checks the output array and mapping."""
    assert lm_result.array is not None
    # 103-class result should have values in the expected range
    assert np.max(lm_result.array) <= 240 
    # Input 0 should be mapped to 0 (NoData) in output
    assert lm_result.array[0, 0, 0] == 0

def test_lm_stats_content(lm_result):
    """Checks if the statistics dictionary captured all three input classes."""
    input_stats = lm_result.stats["input stats"]
    
    assert input_stats["class1 pxl"] > 0
    assert input_stats["class2 pxl"] > 0
    assert input_stats["class3 pxl"] > 0
    assert input_stats["missing pxl"] == 1

def test_lm_stats_standalone(lm_result):
    """Tests the independent landmos_stats call and report generation."""
    from pathlib import Path
    
    tif_103 = lm_result.stats["output paths"]["path tif 103cl"]
    
    # Run stats independently
    stats = landmos_stats(tif_103, outfile=True)
    
    assert "output stats" in stats
    assert "pxl numb 19cl" in stats["output stats"]
    
    # Check if the CSV heatmap was created (specific to Land Mosaic)
    csv_hm = Path(lm_result.stats["output paths"]["path csv hm"])
    assert csv_hm.exists()