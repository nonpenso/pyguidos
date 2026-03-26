import pytest
import numpy as np
import rasterio
from pyguidos import mspa

def test_mspa_binary_execution(tmp_path):
    """
    Creates a valid GTB binary raster and runs the MSPA binary.
    Input: 1=Background, 2=Foreground, 0=Missing
    """
    input_tif = tmp_path / "input_mspa.tif"
    
    # 1. Create a 10x10 array
    # Fill with 1 (Background)
    data = np.ones((10, 10), dtype=np.uint8)
    
    # Add a 4x4 block of 2 (Foreground) in the center
    data[3:7, 3:7] = 2 
    
    # Optional: Add some 0 (Missing) at the corners
    data[0, 0] = 0
    data[9, 9] = 0

    # 2. Write to GeoTIFF
    with rasterio.open(
        input_tif, 'w', driver='GTiff',
        height=10, width=10, count=1, dtype='uint8',
        crs='EPSG:3035', 
        transform=rasterio.transform.from_origin(0, 10, 1, 1)
    ) as dst:
        dst.write(data, 1)

    # 3. Execute the MSPA procedure
    try:
        # Run with standard parameters
        result = mspa.mspa(str(input_tif), wsize=3, edge_width=1)
        
        # 4. Assertions to verify the procedure worked
        assert result.array is not None
        assert result.array.shape == (10, 10)
        
        # MSPA classes range from 1 to 100+ (Core, Edge, Perforated, etc.)
        # We check if the foreground (2) was actually processed into MSPA classes
        assert np.max(result.array) > 2 
        
        # Check that background (1) remained background (rendered as 129 usually)
        # or check the stats dictionary
        assert "output path" in result.stats
        print("MSPA integration test passed successfully!")

    except FileNotFoundError:
        pytest.skip("MSPA binary not found in the search path. Skipping.")
    except SystemExit as e:
        pytest.fail(f"MSPA exited with an error. Check if the binary is compatible with this OS: {e}")