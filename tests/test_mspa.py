import pytest
import numpy as np
import rasterio
from pyguidos import mspa

def test_mspa_integration_run(tmp_path):
    """
    Test the full mspa() workflow with a valid GTB binary raster.
    Values: 1 (Background), 2 (Foreground), 0 (Optional Missing)
    """
    # 1. Prepare the dummy file path in a temporary directory
    input_tif = tmp_path / "test_input_mspa.tif"
    
    # 2. Create a 10x10 uint8 array that follows your requirements
    # Start with all 1 (Background)
    data = np.ones((10, 10), dtype=np.uint8)
    # Create a 4x4 square of 2 (Foreground) in the middle
    data[3:7, 3:7] = 2 
    # Add a single 0 (Missing) to test that it's accepted
    data[0, 0] = 0

    # 3. Write the GeoTIFF with 1 band (mandatory for your check)
    with rasterio.open(
        input_tif, 'w', driver='GTiff',
        height=10, width=10, count=1, dtype='uint8',
        crs='EPSG:3035', 
        transform=rasterio.transform.from_origin(0, 10, 1, 1)
    ) as dst:
        dst.write(data, 1)

    # 4. Call the mspa function
    try:
        # We set stat_files=False to avoid cluttering the tmp directory
        # We set return_array=True to verify the output data
        result = mspa.mspa(
            str(input_tif), 
            edge_width=1, 
            connectivity=8, 
            stat_files=False, 
            return_array=True
        )

        # 5. Verify the Result Object (MSPAResult)
        assert result.array is not None
        assert result.array.shape == (10, 10)
        
        # MSPA categories start above 2, so the max value should be higher than input
        assert np.max(result.array) > 2
        
        # Verify foreground/background counts are in the stats dict
        assert "foreground pxl" in result.stats
        assert result.stats["foreground pxl"] == 16  # 4x4 block

    except FileNotFoundError:
        pytest.skip("MSPA binary not found. Is it in the progs/linux folder?")
    except SystemExit as e:
        pytest.fail(f"MSPA exited prematurely. Check input validation: {e}")
    except Exception as e:
        pytest.fail(f"MSPA integration test failed: {e}")