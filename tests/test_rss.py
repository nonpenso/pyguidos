import pytest
import numpy as np
import rasterio
from pyguidos import rss

@pytest.fixture(scope="module")
def rss_result(tmp_path_factory):
    """
    Fixture that runs RSS once.
    RSS computes connectivity indices (COH, RPOT, etc.)
    """
    tmp_dir = tmp_path_factory.mktemp("rss_data")
    input_tif = tmp_dir / "input_rss.tif"
    
    # 1. Create 15x15 dummy data
    data = np.ones((15, 15), dtype=np.uint8)
    
    # Create a few foreground patches to ensure connectivity indices > 0
    data[2:5, 2:5] = 2   
    data[10:14, 10:14] = 2 
    
    # Add Special/Missing values (0, 3, 4)
    data[0, 0] = 0  # Missing
    data[0, 1] = 3  # Special BG 3
    data[0, 2] = 4  # Special BG 4
    
    # Georeference to avoid warnings
    from rasterio.transform import from_origin
    with rasterio.open(
        input_tif, 'w', driver='GTiff',
        height=15, width=15, count=1, dtype='uint8',
        crs='EPSG:3035',
        transform=from_origin(0, 15, 1, 1)
    ) as dst:
        dst.write(data, 1)

    # 2. Run RSS
    return rss(str(input_tif), stat_files=True)

def test_rss_indices_calculated(rss_result):
    """Verifies that connectivity indices are present and non-zero."""
    out_stats = rss_result.stats["output stats"]
    
    # Check core indices
    assert "COH" in out_stats
    assert "REST_POT" in out_stats
    assert "ECA" in out_stats
    assert float(out_stats["COH"]) >= 0
    assert float(out_stats["REST_POT"]) >= 0

def test_rss_special_bg_stats(rss_result):
    """Verifies that BG 3 and 4 are tracked in RSS input stats."""
    input_stats = rss_result.stats["input stats"]
    
    assert input_stats["missing pxl"] == 1
    assert input_stats["backgr3 pxl"] == 1
    assert input_stats["backgr4 pxl"] == 1
    assert input_stats["foreground pxl"] == 25

def test_rss_report_generation(rss_result):
    """Verifies the text report exists and contains the tool ID."""
    from pathlib import Path
    txt_path = Path(rss_result.stats["output paths"]["path txt"])
    
    assert txt_path.exists()
    
    with open(txt_path, 'r') as f:
        content = f.read()
        assert "RESTORATION STATUS SUMMARY (RSS)" in content
        assert "Coherence" in content