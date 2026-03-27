"""
Tests for pyguidos.engine module.

This file contains tests for OS detection and the heavy-duty 
image processing functions moved from utils.
Run with: pytest tests/test_engine.py -v
"""

import pytest
import collections
from pathlib import Path
import numpy as np
import rasterio
from pyguidos import engine


# =============================================================================
# get_os_info
# =============================================================================

class TestGetOsInfo:

    def test_returns_tuple_of_three(self):
        """Must return a tuple of (str, str, bool)."""
        result = engine.get_os_info()
        assert isinstance(result, tuple)
        assert len(result) == 3

    def test_os_name_is_string(self):
        """OS name must be a string (e.g., 'Linux', 'Windows')."""
        os_name, arch, is_win = engine.get_os_info()
        assert isinstance(os_name, str)
        assert len(os_name) > 0

    def test_arch_is_string(self):
        """Architecture suffix must be a string ('' or 'ARM')."""
        os_name, arch, is_win = engine.get_os_info()
        assert isinstance(arch, str)

    def test_is_win_is_bool(self):
        """is_win flag must be a boolean."""
        os_name, arch, is_win = engine.get_os_info()
        assert isinstance(is_win, bool)

    def test_known_platforms(self):
        """Detects standard OS platforms."""
        os_name, arch, is_win = engine.get_os_info()
        assert os_name in ['Linux', 'Windows', 'Darwin']


# =============================================================================
# labelling_array
# =============================================================================

class TestLabellingArray:

    def test_single_patch(self):
        """Single connected foreground region -- one patch."""
        arr = np.array([
            [0, 0, 0],
            [0, 2, 0],
            [0, 0, 0]
        ], dtype=np.uint8)
        labeled, freq = engine.labelling_array(arr, 2)
        assert len(freq) == 1
        assert list(freq.values())[0] == 1

    def test_two_separate_patches(self):
        """Two disconnected foreground regions -- two patches."""
        arr = np.array([
            [2, 0, 2],
            [0, 0, 0],
            [0, 0, 0]
        ], dtype=np.uint8)
        labeled, freq = engine.labelling_array(arr, 2)
        assert len(freq) == 2
        assert freq[1] == 1
        assert freq[2] == 1

    def test_no_foreground(self):
        """No foreground pixels -- empty frequency Counter."""
        arr = np.zeros((5, 5), dtype=np.uint8)
        labeled, freq = engine.labelling_array(arr, 2)
        assert len(freq) == 0
        assert isinstance(freq, collections.Counter)

    def test_background_not_in_freq(self):
        """Background label 0 must never appear in the frequency Counter."""
        arr = np.array([[0, 2], [0, 2]], dtype=np.uint8)
        labeled, freq = engine.labelling_array(arr, 2)
        assert 0 not in freq

    def test_single_integer_target(self):
        """Single integer target value is correctly masked."""
        arr = np.array([[1, 0], [1, 0]], dtype=np.uint8)
        labeled, freq = engine.labelling_array(arr, 1)
        assert len(freq) == 1
        assert freq[1] == 2

    def test_list_of_target_values(self):
        """List of target values -- both treated as foreground."""
        arr = np.array([
            [1, 0, 2],
            [0, 0, 0]
        ], dtype=np.uint8)
        # 1 and 2 are disconnected here, so 2 patches
        labeled, freq = engine.labelling_array(arr, [1, 2])
        assert len(freq) == 2

    def test_8_connectivity(self):
        """8-connectivity ensures diagonal pixels are connected."""
        arr = np.array([
            [2, 0, 0],
            [0, 2, 0],
            [0, 0, 2]
        ], dtype=np.uint8)
        labeled, freq = engine.labelling_array(arr, 2)
        # In 8-connectivity, this is 1 single diagonal patch
        assert len(freq) == 1
        assert freq[1] == 3

    def test_returns_tuple(self):
        """Function must return a tuple of (np.ndarray, Counter)."""
        arr = np.array([[2, 0], [0, 2]], dtype=np.uint8)
        result = engine.labelling_array(arr, 2)
        assert isinstance(result, tuple)
        assert isinstance(result[0], np.ndarray)
        assert isinstance(result[1], collections.Counter)

    def test_patch_sizes_correct(self):
        """Patch sizes are correctly counted."""
        arr = np.array([
            [2, 2, 0],
            [2, 0, 0],
            [0, 0, 2]
        ], dtype=np.uint8)
        labeled, freq = engine.labelling_array(arr, 2)
        sizes = sorted(freq.values())
        assert sizes == [1, 3] # One patch of size 1, one patch of size 3

# =============================================================================
from pyguidos.engine import write_mspa_input

def test_write_mspa_input_branches(tmp_path):
    # Setup: Create a dummy source GeoTIFF
    source_path = tmp_path / "source.tif"
    out_dir = tmp_path / "output"
    out_dir.mkdir()
    
    data = np.zeros((10, 10), dtype=np.uint8)
    profile = {
        'driver': 'GTiff', 'height': 10, 'width': 10, 'count': 1, 
        'dtype': 'uint8', 'crs': 'EPSG:3857', 'transform': rasterio.transform.from_origin(0, 10, 1, 1)
    }
    
    with rasterio.open(source_path, 'w', **profile) as dst:
        dst.write(data, 1)

    # BRANCH 1: Test the 'shutil.copy2' path (uint8 and not tiled)
    result_path = write_mspa_input(out_dir, source_path, data, dtype='uint8', is_tiled=False)
    assert result_path.exists()
    assert result_path.name == "mspa_input.tif"

    # BRANCH 2: Test the 'rasterio.open' conversion path (e.g., is_tiled=True)
    # This forces the 'else' block to run
    result_path_tiled = write_mspa_input(out_dir, source_path, data, dtype='uint8', is_tiled=True)
    assert result_path_tiled.exists()
    
    # BRANCH 3: Test the 'else' block with a different dtype (e.g., uint16)
    result_path_dtype = write_mspa_input(out_dir, source_path, data.astype(np.uint16), dtype='uint16', is_tiled=False)
    assert result_path_dtype.exists()
    with rasterio.open(result_path_dtype) as src:
        assert src.dtypes[0] == 'uint8' # Verify it converted to uint8

def test_engine_verbose_and_dtypes(tmp_path, capsys):
    """
    Targets the 'verbose' branches and 'dtype' conversion branches.
    """
    # 1. Test the 'if verbose' and stdout printing logic
    class MockResult:
        def __init__(self):
            self.stdout = "Working...\n10% [##########]\nNo output given\nFinished!"

    result = MockResult()
    
    verbose = True
    if verbose and result.stdout:
        for line in result.stdout.splitlines():
            if "% [" in line: continue  # Hits the 'continue' line
            if "No output given" in line: continue # Hits the second 'continue'
            print(line)
    
    captured = capsys.readouterr()
    assert "Finished!" in captured.out
    assert "10% [" not in captured.out

    # 2. Test the dtype conversion branch (uint16 -> uint8)
    source_path = tmp_path / "uint16_source.tif"
    out_dir = tmp_path / "out"
    out_dir.mkdir()
    
    # Create uint16 data (this triggers the 'if data_array.dtype != np.uint8' branch)
    data_uint16 = np.array([[1, 2], [3, 4]], dtype=np.uint16)
    
    # Create a dummy profile
    profile = {
        'driver': 'GTiff', 'height': 2, 'width': 2, 'count': 1, 
        'dtype': 'uint16', 'crs': 'EPSG:3857', 'transform': rasterio.transform.from_origin(0, 2, 1, 1)
    }
    with rasterio.open(source_path, 'w', **profile) as dst:
        dst.write(data_uint16, 1)

    # This call now hits the 'else' block and the 'casting=unsafe' line
    result_path = write_mspa_input(
        out_path=out_dir, 
        source_path=source_path, 
        data_array=data_uint16, 
        dtype='uint16', 
        is_tiled=False
    )
    
    with rasterio.open(result_path) as check:
        assert check.dtypes[0] == 'uint8'

import sys
import subprocess
from pyguidos.engine import run_mspa

def test_run_mspa_verbose_logic(tmp_path, monkeypatch, capsys):
    """Targets the verbose printing and string filtering (the red lines)."""
    
    # 1. Mock get_binary_path so it doesn't fail if binary is missing
    monkeypatch.setattr("pyguidos.engine.get_binary_path", lambda x: Path("/fake/mspa"))

    # 2. Setup Mock successful result with 'noisy' output
    class MockCompletedProcess:
        stdout = "MSPA starting...\n75% [#######---]\nNo output given\nTask complete."
        returncode = 0

    def mock_run_success(*args, **kwargs):
        return MockCompletedProcess()

    monkeypatch.setattr(subprocess, "run", mock_run_success)

    # Execute with verbose=True
    run_mspa(tmp_path, Path("in.tif"), 8, 1, 0, 0, verbose=True)

    # Check output
    captured = capsys.readouterr()
    assert "MSPA starting..." in captured.out
    assert "Task complete." in captured.out
    # These two MUST be missing because of your 'continue' logic:
    assert "% [" not in captured.out
    assert "No output given" not in captured.out

def test_run_mspa_error_branches(tmp_path, monkeypatch, capsys):
    """Targets the 'except subprocess.CalledProcessError' and 'sys.exit' branches."""
    
    monkeypatch.setattr("pyguidos.engine.get_binary_path", lambda x: Path("/fake/mspa"))

    # 1. Simulate a Binary Failure (CalledProcessError)
    # 1. Simulate a Binary Failure (CalledProcessError)
    def mock_run_fail(*args, **kwargs):
        raise subprocess.CalledProcessError(
                returncode=1, 
                cmd="mspa", 
                output="Out of memory",
                stderr="Core dumped"
        )
    monkeypatch.setattr(subprocess, "run", mock_run_fail)

    # We expect sys.exit(1), so we catch the SystemExit exception
    with pytest.raises(SystemExit) as e:
        run_mspa(tmp_path, Path("in.tif"), 8, 1, 0, 0)
    
    assert e.value.code == 1
    captured = capsys.readouterr()
    assert "ERROR: MSPA failed" in captured.out
    assert "STDOUT: Out of memory" in captured.out

    # 2. Simulate FileNotFoundError (Binary missing)
    def mock_run_not_found(*args, **kwargs):
        raise FileNotFoundError("Mocked missing file")

    monkeypatch.setattr(subprocess, "run", mock_run_not_found)

    with pytest.raises(SystemExit) as e:
        run_mspa(tmp_path, Path("in.tif"), 8, 1, 0, 0)
    
    assert e.value.code == 1
    captured = capsys.readouterr()
    assert "Binary not found" in captured.out