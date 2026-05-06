import pytest
from pathlib import Path
import numpy as np
import rasterio
from rasterio.transform import from_origin
from pyguidos.spa import spa, spa_stats


@pytest.fixture(scope="module")
def spa_result(tmp_path_factory):
    """
    Fixture that runs SPA once and shares the result with all tests.
    'scope=module' means it runs only once for this entire file.
    """
    # Create a temporary directory that lasts for the whole session
    tmp_dir = tmp_path_factory.mktemp("data")
    input_tif = tmp_dir / "input_spa.tif"
    
    # Create dummy data: 1=BG, 2=FG
    data = np.ones((10, 10), dtype=np.uint8)
    data[3:7, 3:7] = 2 
    
    with rasterio.open(
        input_tif, 'w', 
        driver='GTiff',
        height=10, 
        width=10, 
        count=1, 
        dtype='uint8',
        crs='EPSG:3035',
        transform=from_origin(0, 10, 1, 1) 
    ) as dst:
        dst.write(data, 1)

    # Run the full SPA process
    return spa(str(input_tif), edge_width=1, classes=6, stat_files=True)


def test_spa_stats_content(spa_result):
    """Check the stats dictionary from the shared result."""
    assert "input stats" in spa_result
    assert spa_result["input stats"]["foreground pxl"] == 16

def test_spa_stats_function_directly(spa_result):
    """
    Now we test the spa_stats function using the file 
    produced by the fixture.
    """
    # Get the path of the TIF created during the fixture run
    output_tif_path = spa_result["output paths"]["path tif"]
    
    # Call spa_stats directly on that file
    stats = spa_stats(output_tif_path, stat_files=True)
    
    assert "output stats" in stats
    assert "class freq" in stats["output stats"]

def test_spa_stats_standalone(spa_result):
    """
    Tests if spa_stats can take a resulting TIF and generate 
    a text report and stats dictionary independently.
    """
    
    # 1. Get the path of the TIF produced by our fixture
    output_tif_path = spa_result["output paths"]["path tif"]
    
    # 2. Run spa_stats as a standalone call
    stats = spa_stats(output_tif_path, stat_files=True)

    # 3. Assertions for the Dictionary
    assert "output stats" in stats
    assert "input stats" in stats
    assert stats["input stats"]["foreground pxl"] == 16
    
    # 4. Assertions for the Text Report File
    expected_txt = Path(output_tif_path).with_suffix('.txt')
    assert expected_txt.exists()
    
    # 5. Verify the text file isn't empty
    with open(expected_txt, 'r') as f:
        content = f.read()
        # Check for template keywords
        assert "Porosity" in content or "Core" in content 