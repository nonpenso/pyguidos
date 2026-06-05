import pytest
from unittest.mock import patch, MagicMock, mock_open
from pathlib import Path
import numpy as np
import rasterio

from pyguidos.fragmentation_change import frag_change, _get_frag_change_stats, aggregate_change_matrix

@pytest.fixture
def mock_raster_profiles():
    """Generates matching mock profiles and informational dictionaries for testing."""
    profile = {
        'driver': 'GTiff',
        'height': 10,
        'width': 10,
        'count': 1,
        'dtype': 'uint8',
        'crs': 'EPSG:3035',
        'transform': rasterio.transform.from_origin(0, 0, 1, 1)
    }
    
    info = {
        "tag": "GTB_FOS, <Binary,-1,8,FAD_5,100.000,7>, https:",
        "profile": profile,
        "rows": 10,
        "cols": 10,
        "bands": 1,
        "resX": 1.0,
        "resY": 1.0,
        "epsg": 4326,
        "is_projected": True,
        "bounds": (0, 0, 10, 10)
    }
    return info


@pytest.fixture
def mock_change_matrix():
    """Populates a mock 107x107 matrix with controlled data values."""
    matrix = np.zeros((107, 107), dtype=np.int64)
    # Put values within the expected tracking bands to hit matrix accumulation logic
    matrix[101, 101] = 5  # Background dynamics
    matrix[10, 20] = 3    # Class variations
    matrix[50, 50] = 12   # Foreground invariants
    return matrix


def test_aggregate_change_matrix_logic(mock_change_matrix):
    """Directly tests the aggregation matrix reshaping engine."""
    groupings = [[101], [10, 50]]
    
    new_mat, totals_a, totals_b = aggregate_change_matrix(mock_change_matrix, groupings)
    
    # Check shape translation
    assert new_mat.shape == (2, 2)
    # Check specific aggregations
    assert new_mat[0, 0] == 5   # value from matrix[101, 101]
    assert new_mat[1, 1] == 12  # value from matrix[50, 50]


@patch("pyguidos.fragmentation_change.utils.get_raster_info")
@patch("pyguidos.fragmentation_change.checks.validate_fchmaps_input")
@patch("pyguidos.fragmentation_change.utils.get_colormap")
@patch("pyguidos.fragmentation_change.rasterio.open")
@patch("pyguidos.fragmentation_change._get_frag_change_stats")
def test_frag_change_pipeline_execution(
    mock_stats_fn, mock_rasterio_open, mock_get_cmap, mock_validate, mock_raster_info, tmp_path, mock_raster_profiles
):
    """Validates the execution flow of frag_change, confirming raster reads/writes and mock operations."""
    
    # 1. Setup our mock boundaries
    mock_raster_info.return_value = mock_raster_profiles
    mock_validate.return_value = True
    mock_get_cmap.return_value = ({"0": (1,1,1)}, {0: (255, 255, 255, 255)})
    
    # Create file instance mocks for Context Managers
    mock_src_a = MagicMock()
    mock_src_b = MagicMock()
    mock_dst = MagicMock()
    
    # Configure mock window generator loop
    mock_window = rasterio.windows.Window(0, 0, 10, 10)
    mock_src_a.block_windows.return_value = [((0, 0), mock_window)]
    mock_src_a.read.return_value = np.ones((10, 10), dtype=np.uint8)
    mock_src_b.read.return_value = np.ones((10, 10), dtype=np.uint8)
    
    # Direct the patch context routers
    mock_rasterio_open.return_value.__enter__.side_effect = [mock_src_a, mock_src_b, mock_dst]
    
    # Mock final output dictionary
    expected_response = {"output stats": "VALID_METRICS"}
    mock_stats_fn.return_value = expected_response
    
    # 2. Execute target function
    res = frag_change(
        in_tiff_t1="t1.tif",
        in_tiff_t2="t2.tif",
        outdir=tmp_path,
        statists=True,
        stat_files=True,
        verb=False
    )
    
    # 3. Structural verifications
    assert res == expected_response
    mock_validate.assert_called_once()
    mock_dst.write.assert_called_once()
    mock_dst.write_colormap.assert_called_once()


@patch("pyguidos.fragmentation_change.utils.get_raster_info")
@patch("pyguidos.fragmentation_change.utils.get_tool_parameters")
@patch("pyguidos.fragmentation_change.utils.get_pxl_freq")
@patch("pyguidos.fragmentation_change.rasterio.open")
@patch("pyguidos.fragmentation_change.plt.savefig")
@patch("pyguidos.fragmentation_change.open", new_callable=mock_open)
def test_get_frag_change_stats_reporting(
    mock_file_io, mock_save_fig, mock_rasterio_open, mock_pxl_freq, mock_tool_params, mock_raster_info, tmp_path
):
    """Tests the reporting generator, checking file saves and calculations for connectivity indices."""
    
    # FIXED: Added the "tag" key to mirror real raster metadata structures
    mock_raster_info.return_value = {
        "tag": "GTB_FOS, <Binary,-1,8,FAD_5,100.0,7>, https",
        "epsg": 3035, 
        "is_projected": False, 
        "resX": 1.0, 
        "resY": 1.0, 
        "rows": 5, 
        "cols": 5, 
        "name": "test"
    }
    
    mock_tool_params.return_value = {"method": "FAD_5", "wsize": "7"}
    
    # Create an array tracking dummy frequency pixels [0-255 values]
    freq_arr = np.zeros(256, dtype=np.int64)
    freq_arr[100] = 10  # Insignificant changes
    mock_pxl_freq.return_value = freq_arr
    
    # Create tracking matrix input setup
    ch_matrix = np.zeros((107, 107), dtype=np.int64)
    ch_matrix[5, 5] = 100  # Add elements to calculate indices without ZeroDivisionError
    
    mock_src = MagicMock()
    mock_src.read.return_value = np.ones((5,5), dtype=np.uint8)
    mock_rasterio_open.return_value.__enter__.return_value = mock_src

    # Execute metrics processing function
    stats = _get_frag_change_stats(
        frag_chan_tiff=Path("mock_in.tif"),
        ch_matrix=ch_matrix,
        tiff1=Path("t1.tif"),
        tiff2=Path("t2.tif"),
        out_file=True,
        out_dir=tmp_path
    )
    
    # Verify report dictionary generation outputs
    assert "output paths" in stats
    assert "input stats" in stats
    assert stats["output stats"]["Conn change freq"]["4 Insign/no change"] == 10
    
    # Verify reporting files are created
    mock_file_io.assert_called()
    mock_save_fig.assert_called_once()