import pytest
import numpy as np
import rasterio
from pyguidos import frag, frag_stats

@pytest.fixture(scope="module")
def frag_result(tmp_path_factory):
    """
    Fixture that runs Fragmentation once with all optional pixel values:
    0 (Missing), 1 (BG), 2 (FG), 3 (Spec BG 1), 4 (Spec BG 2).
    """
    tmp_dir = tmp_path_factory.mktemp("frag_data_extended")
    input_tif = tmp_dir / "input_frag_extended.tif"
    
    # 1. Create 11x11 dummy data
    # Start with Background (1)
    data = np.ones((11, 11), dtype=np.uint8)
    
    # Add Foreground (2) patch
    data[3:8, 3:8] = 2 
    
    # Add Optional/Special values
    data[0, 0] = 0   # Missing/NoData
    data[0, 1] = 3   # Special Background 1
    data[0, 2] = 4   # Special Background 2
    
    # 2. Write GeoTIFF with georeferencing to avoid warnings
    from rasterio.transform import from_origin
    with rasterio.open(
        input_tif, 'w', driver='GTiff',
        height=11, width=11, count=1, dtype='uint8',
        crs='EPSG:3035',
        transform=from_origin(0, 11, 1, 1)
    ) as dst:
        dst.write(data, 1)

    # 3. Run Fragmentation
    return frag(
        str(input_tif), 
        method='FAD', 
        window_size=3, 
        stat_files=True
    )

def test_frag_special_stats(frag_result):
    """
    Verifies that frag_stats correctly counted the special pixels.
    """
    input_stats = frag_result["input stats"]
    
    assert input_stats["missing pxl"] == 1
    assert input_stats["backgr3 pxl"] == 1
    assert input_stats["backgr4 pxl"] == 1
    
    # Verify these appear in the output stats dictionary too
    assert "fad_av" in frag_result["output stats"]

def test_frag_stats_standalone_files(frag_result):
    """Verifies standalone file generation."""
    from pathlib import Path
    output_tif_path = frag_result["output paths"]["path tif"]
    
    # Run standalone stats logic on the generated output
    stats = frag_stats(output_tif_path, stat_files=True)
    
    # Verify .txt report generation
    expected_txt = Path(output_tif_path).with_suffix('.txt')
    assert expected_txt.exists()
    
    # Verify the stats dictionary structure
    assert "avcon" in stats["output stats"]


# =============================================================================
# FED Method Tests
# =============================================================================

@pytest.fixture(scope="module")
def frag_fed_result(tmp_path_factory):
    """Runs Fragmentation with FED method."""
    tmp_dir = tmp_path_factory.mktemp("frag_fed_data")
    input_tif = tmp_dir / "input_frag_fed.tif"

    data = np.ones((11, 11), dtype=np.uint8)
    data[3:8, 3:8] = 2  # Foreground patch

    from rasterio.transform import from_origin
    with rasterio.open(
        input_tif, 'w', driver='GTiff',
        height=11, width=11, count=1, dtype='uint8',
        crs='EPSG:3035',
        transform=from_origin(0, 11, 1, 1)
    ) as dst:
        dst.write(data, 1)

    return frag(
        str(input_tif),
        method='FED',
        window_size=3,
        connectivity=4,
        stat_files=True
    )


def test_frag_fed_returns_dict(frag_fed_result):
    """Verifies FED method returns a valid result dictionary."""
    assert "output paths" in frag_fed_result
    assert "input stats" in frag_fed_result
    assert "output stats" in frag_fed_result


def test_frag_fed_stats(frag_fed_result):
    """Verifies FED statistics are computed."""
    assert "fad_av" in frag_fed_result["output stats"]
    assert "avcon" in frag_fed_result["output stats"]
    assert frag_fed_result["output stats"]["fad_av"] > 0


def test_frag_fed_output_files(frag_fed_result):
    """Verifies FED output files are generated."""
    from pathlib import Path
    assert frag_fed_result["output paths"]["path tif"] is not None
    assert Path(frag_fed_result["output paths"]["path tif"]).exists()


# =============================================================================
# FAC Connectivity Tests
# =============================================================================

@pytest.fixture(scope="module")
def frag_fac8_result(tmp_path_factory):
    """Runs Fragmentation with FAC method, 8-connectivity."""
    tmp_dir = tmp_path_factory.mktemp("frag_fac8_data")
    input_tif = tmp_dir / "input_frag_fac8.tif"

    data = np.ones((11, 11), dtype=np.uint8)
    data[3:8, 3:8] = 2

    from rasterio.transform import from_origin
    with rasterio.open(
        input_tif, 'w', driver='GTiff',
        height=11, width=11, count=1, dtype='uint8',
        crs='EPSG:3035',
        transform=from_origin(0, 11, 1, 1)
    ) as dst:
        dst.write(data, 1)

    return frag(
        str(input_tif),
        method='FAC',
        window_size=3,
        connectivity=8,
        stat_files=True
    )


def test_frag_fac8_returns_dict(frag_fac8_result):
    """Verifies FAC 8-conn returns a valid result dictionary."""
    assert "output paths" in frag_fac8_result
    assert "output stats" in frag_fac8_result


def test_frag_fac8_tag_contains_connectivity(frag_fac8_result):
    """Verifies the output GeoTIFF tag encodes connectivity=8."""
    from pyguidos import utils
    from pathlib import Path
    tiff_path = Path(frag_fac8_result["output paths"]["path tif"])
    info = utils.get_raster_info(tiff_path)
    assert ",8," in info["tag"]
