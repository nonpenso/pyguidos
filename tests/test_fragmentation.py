import pytest
import numpy as np
import rasterio
from pathlib import Path
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


# =============================================================================
# v2.5.2 — Output Filename Convention Tests
# =============================================================================

def test_frag_fad_filename_no_connectivity_suffix(frag_result):
    """FAD output filename should NOT include a connectivity suffix."""
    tif_path = frag_result["output paths"]["path tif"]
    stem = Path(tif_path).stem
    # FAD pattern: <name>_frag_fad_<wsize>  (no digit between 'fad' and '_')
    assert "_frag_fad_" in stem
    # Should NOT match patterns like _frag_fad4_ or _frag_fad8_
    import re
    assert not re.search(r"_frag_fad\d+_", stem)


def test_frag_fac8_filename_includes_connectivity(frag_fac8_result):
    """FAC 8-conn output filename should include '8' as connectivity suffix."""
    tif_path = frag_fac8_result["output paths"]["path tif"]
    stem = Path(tif_path).stem
    assert "_frag_fac8_" in stem


def test_frag_fed_filename_includes_connectivity(frag_fed_result):
    """FED output filename should include connectivity suffix."""
    tif_path = frag_fed_result["output paths"]["path tif"]
    stem = Path(tif_path).stem
    assert "_frag_fed4_" in stem


# =============================================================================
# v2.5.2 — pixel_conn in Text Report
# =============================================================================

def test_frag_fad_report_pixel_conn_is_dash(frag_result):
    """FAD text report should contain pixel_conn as '-' since connectivity is not applicable."""
    txt_path = Path(frag_result["output paths"]["path txt"])
    report_text = txt_path.read_text()
    assert "-" in report_text  # FAD has no connectivity


def test_frag_fac8_report_pixel_conn_is_8connected(frag_fac8_result):
    """FAC 8-conn text report should contain '8-connected'."""
    txt_path = Path(frag_fac8_result["output paths"]["path txt"])
    report_text = txt_path.read_text()
    assert "8-connected" in report_text


def test_frag_fed_report_pixel_conn_is_4connected(frag_fed_result):
    """FED 4-conn text report should contain '4-connected'."""
    txt_path = Path(frag_fed_result["output paths"]["path txt"])
    report_text = txt_path.read_text()
    assert "4-connected" in report_text


# =============================================================================
# v2.5.2 — frag_stats() Rejects Grayscale Input
# =============================================================================

def test_frag_stats_rejects_grayscale_input(tmp_path_factory):
    """frag_stats() should exit with an error when given a grayscale frag output."""
    from rasterio.transform import from_origin

    tmp_dir = tmp_path_factory.mktemp("frag_stats_gray_reject")
    tiff_path = tmp_dir / "gray_frag.tif"

    # Create a fake grayscale fragmentation output with Gray tiftype tag
    data = np.full((5, 5), 50, dtype=np.uint8)
    with rasterio.open(
        tiff_path, 'w', driver='GTiff',
        height=5, width=5, count=1, dtype='uint8',
        crs='EPSG:3035',
        transform=from_origin(0, 5, 1, 1)
    ) as dst:
        dst.write(data, 1)
        dst.update_tags(
            TIFFTAG_IMAGEDESCRIPTION="GTB_FOS, <Gray,-1,4,FAD_5,100.0,3>, https://x"
        )

    with pytest.raises(SystemExit) as exc_info:
        frag_stats(str(tiff_path))
    assert "grayscale" in str(exc_info.value).lower() or "Gray" in str(exc_info.value)
