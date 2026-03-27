import pytest
import numpy as np
import rasterio
from pyguidos import acc, acc_stats

@pytest.fixture(scope="module")
def acc_result(tmp_path_factory):
    """
    Fixture that runs Accounting once.
    Uses Foreground (2), Background (1), Missing (0), and Special BG (3, 4).
    """
    tmp_dir = tmp_path_factory.mktemp("acc_data")
    input_tif = tmp_dir / "input_acc.tif"
    
    # 1. Create 15x15 dummy data
    data = np.ones((15, 15), dtype=np.uint8)
    
    # Create two distinct foreground patches
    data[2:5, 2:5] = 2   # Patch of 9 pixels
    data[10:14, 10:14] = 2 # Patch of 16 pixels
    
    # Add Special/Missing values (values 0, 3, and 4)
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

    # 2. Run Accounting with a threshold of 10 pixels
    return acc(
        str(input_tif), 
        thresholds=[10], 
        return_array=True, 
        stat_files=True
    )

def test_acc_array_mapping(acc_result):
    """Verifies output array dimensions and that reclassification occurred."""
    assert acc_result.array is not None
    assert acc_result.array.shape == (15, 15) 
    
    # Verify the specific mapping at the positions defined in the fixture
    # Position [0,0] was input 0 -> should be 129
    # Position [0,1] was input 3 -> should be 105
    # Position [0,2] was input 4 -> should be 176
    assert acc_result.array[0, 0] == 129
    assert acc_result.array[0, 1] == 105
    assert acc_result.array[0, 2] == 176

def test_acc_special_bg_stats(acc_result):
    """Verifies that BG 3 and 4 are correctly tracked in the statistics."""
    input_stats = acc_result.stats["input stats"]
    
    # is correctly handled by acc_stats
    assert input_stats["missing pxl"] == 1
    assert input_stats["backgr3 pxl"] == 1
    assert input_stats["backgr4 pxl"] == 1
    assert input_stats["foreground pxl"] == 25

def test_acc_stats_standalone(acc_result):
    """Tests independent acc_stats call and report file generation."""
    from pathlib import Path
    
    output_tif = acc_result.stats["output paths"]["path tif"]
    
    # Run standalone stats logic on the generated output
    stats = acc_stats(output_tif, outfile=True)
    
    # Verify .txt report generation
    expected_txt = Path(output_tif).with_suffix('.txt')
    assert expected_txt.exists()
    
    # Verify the stats dictionary structure for patch accounting
    assert "patch numb" in stats["output stats"]
    assert "pxl numb" in stats["output stats"]