"""
Tests for pyguidos.utils module.

All tests use numpy arrays only — no GeoTIFF files or binaries required.
Run with: pytest tests/test_utils.py -v
"""

import pytest
import collections
import numpy as np
from pyguidos import utils


# =============================================================================
# get_pxl_freq
# =============================================================================

class TestGetPxlFreq:

    def test_uint8_2d_basic(self):
        """Basic uint8 2D array — fast path."""
        arr = np.array([[0, 1, 2], [1, 2, 2]], dtype=np.uint8)
        freq = utils.get_pxl_freq(arr)
        assert freq[0] == 1
        assert freq[1] == 2
        assert freq[2] == 3

    def test_uint8_3d_first_band(self):
        """3D uint8 array — only first band used."""
        arr = np.zeros((3, 4, 4), dtype=np.uint8)
        arr[0, :, :] = 1   # band 0 → all 1s
        arr[1, :, :] = 2   # band 1 → all 2s (ignored)
        arr[2, :, :] = 3   # band 2 → all 3s (ignored)
        freq = utils.get_pxl_freq(arr)
        assert freq[1] == 16
        assert 2 not in freq
        assert 3 not in freq

    def test_uint8_zero_count_excluded(self):
        """Values with zero count must not appear in result."""
        arr = np.array([[1, 2], [1, 2]], dtype=np.uint8)
        freq = utils.get_pxl_freq(arr)
        assert 0 not in freq
        assert 3 not in freq

    def test_uint8_returns_counter(self):
        """Result must be a Counter instance."""
        arr = np.array([[1, 2]], dtype=np.uint8)
        freq = utils.get_pxl_freq(arr)
        assert isinstance(freq, collections.Counter)

    def test_uint8_all_same_value(self):
        """Array with all identical values."""
        arr = np.full((10, 10), 2, dtype=np.uint8)
        freq = utils.get_pxl_freq(arr)
        assert freq[2] == 100
        assert len(freq) == 1

    def test_int32_general_path(self):
        """int32 array uses chunked general path."""
        arr = np.array([[0, 1, 2, 1000], [1000, 500, 500, 0]], dtype=np.int32)
        freq = utils.get_pxl_freq(arr)
        assert freq[0] == 2
        assert freq[1] == 1
        assert freq[2] == 1
        assert freq[500] == 2
        assert freq[1000] == 2

    def test_int32_returns_counter(self):
        """int32 result must be a Counter instance."""
        arr = np.array([[1, 2, 3]], dtype=np.int32)
        freq = utils.get_pxl_freq(arr)
        assert isinstance(freq, collections.Counter)

    def test_int32_3d_first_band(self):
        """3D int32 array — only first band used."""
        arr = np.zeros((2, 3, 3), dtype=np.int32)
        arr[0, :, :] = 99
        arr[1, :, :] = 200
        freq = utils.get_pxl_freq(arr)
        assert freq[99] == 9
        assert 200 not in freq

    def test_counter_default_zero(self):
        """Accessing missing key returns 0 (Counter default)."""
        arr = np.array([[1, 2]], dtype=np.uint8)
        freq = utils.get_pxl_freq(arr)
        assert freq[255] == 0


# =============================================================================
# running_time
# =============================================================================

class TestRunningTime:

    def test_seconds_format(self):
        """Duration under 60 seconds returns seconds format."""
        result = utils.running_time(0.0, 5.5)
        assert "seconds" in result
        assert "5.50" in result

    def test_minutes_format(self):
        """Duration over 60 seconds returns minutes format."""
        result = utils.running_time(0.0, 90.0)
        assert "m" in result
        assert "30.0s" in result

    def test_hours_format(self):
        """Duration over 3600 seconds returns hours format."""
        result = utils.running_time(0.0, 3661.0)
        assert "h" in result
        assert "1m" in result

    def test_zero_duration(self):
        """Zero duration returns seconds format."""
        result = utils.running_time(0.0, 0.0)
        assert "seconds" in result
        assert "0.00" in result

    def test_returns_string(self):
        """Result must be a string."""
        result = utils.running_time(0.0, 10.0)
        assert isinstance(result, str)

    def test_exact_one_minute(self):
        """Exactly 60 seconds returns minutes format."""
        result = utils.running_time(0.0, 60.0)
        assert "m" in result

    def test_exact_one_hour(self):
        """Exactly 3600 seconds returns hours format."""
        result = utils.running_time(0.0, 3600.0)
        assert "h" in result


# =============================================================================
# get_tool_parameters
# =============================================================================

class TestGetToolParameters:

    def test_mspa_tag(self):
        """GTB_MSPA tag parsed correctly."""
        tag = "GTB_MSPA, <8,1,1,1>, https://forest.jrc.ec.europa.eu/"
        result = utils.get_tool_parameters(tag)
        assert result is not None
        assert result["tool_id"] == "GTB_MSPA"
        assert result["connectivity"] == "8"
        assert result["edge_width"] == "1"
        assert result["transition"] == "1"
        assert result["int_ext"] == "1"
        assert result["web_link"] == "https://forest.jrc.ec.europa.eu/"

    def test_fos_tag(self):
        """GTB_FOS tag parsed correctly."""
        tag = "GTB_FOS, <Binary,-1,8,FAD_5,100.000,27>, https://forest.jrc.ec.europa.eu/"
        result = utils.get_tool_parameters(tag)
        assert result is not None
        assert result["tool_id"] == "GTB_FOS"
        assert result["tiftype"] == "Binary"
        assert result["connect"] == "8"
        assert result["method"] == "FAD_5"
        assert result["wsize"] == "27"

    def test_lm_tag(self):
        """GTB_LM tag parsed correctly."""
        tag = "GTB_LM, <27,bgr>, https://forest.jrc.ec.europa.eu/"
        result = utils.get_tool_parameters(tag)
        assert result is not None
        assert result["tool_id"] == "GTB_LM"
        assert result["wsize"] == "27"
        assert result["cmap"] == "bgr"

    def test_acc_tag(self):
        """GTB_ACC tag parsed correctly."""
        tag = "GTB_ACC, <100,1000,10000>, https://forest.jrc.ec.europa.eu/"
        result = utils.get_tool_parameters(tag)
        assert result is not None
        assert result["tool_id"] == "GTB_ACC"
        assert result["thresholds"] == ["100", "1000", "10000"]

    def test_empty_tag_returns_none(self):
        """Empty tag must return None."""
        assert utils.get_tool_parameters("") is None
        assert utils.get_tool_parameters(None) is None

    def test_no_comma_returns_none(self):
        """Tag without comma must return None."""
        assert utils.get_tool_parameters("GTB_MSPA") is None

    def test_no_link(self):
        """Tag without URL still parses tool_id."""
        tag = "GTB_MSPA, <8,1,1,1>"
        result = utils.get_tool_parameters(tag)
        assert result is not None
        assert result["tool_id"] == "GTB_MSPA"
        assert result["web_link"] is None

    def test_double_dash_tag(self):
        """'--' tag (no GTB metadata) — no comma so returns None."""
        assert utils.get_tool_parameters("--") is None


# =============================================================================
# labelling_array
# =============================================================================

class TestLabellingArray:

    def test_single_patch(self):
        """Single connected foreground region — one patch."""
        arr = np.array([
            [0, 0, 0],
            [0, 2, 0],
            [0, 0, 0]
        ], dtype=np.uint8)
        labeled, freq = utils.labelling_array(arr, 2)
        assert len(freq) == 1
        assert list(freq.values())[0] == 1

    def test_two_separate_patches(self):
        """Two disconnected foreground regions — two patches."""
        arr = np.array([
            [2, 0, 2],
            [0, 0, 0],
            [0, 0, 0]
        ], dtype=np.uint8)
        labeled, freq = utils.labelling_array(arr, 2)
        assert len(freq) == 2

    def test_no_foreground(self):
        """No foreground pixels — empty freq."""
        arr = np.zeros((5, 5), dtype=np.uint8)
        labeled, freq = utils.labelling_array(arr, 2)
        assert len(freq) == 0

    def test_background_not_in_freq(self):
        """Background label 0 must not appear in freq."""
        arr = np.array([[0, 2], [0, 2]], dtype=np.uint8)
        labeled, freq = utils.labelling_array(arr, 2)
        assert 0 not in freq

    def test_single_integer_target(self):
        """Single integer target value works."""
        arr = np.array([[1, 0], [1, 0]], dtype=np.uint8)
        labeled, freq = utils.labelling_array(arr, 1)
        assert len(freq) == 1
        assert list(freq.values())[0] == 2

    def test_list_of_target_values(self):
        """List of target values — both treated as foreground."""
        arr = np.array([
            [1, 0, 2],
            [0, 0, 0]
        ], dtype=np.uint8)
        labeled, freq = utils.labelling_array(arr, [1, 2])
        assert len(freq) == 2

    def test_8_connectivity(self):
        """8-connectivity — diagonal pixels connected."""
        arr = np.array([
            [2, 0, 0],
            [0, 2, 0],
            [0, 0, 2]
        ], dtype=np.uint8)
        labeled, freq = utils.labelling_array(arr, 2)
        # With 8-connectivity all three diagonal pixels form one patch
        assert len(freq) == 1
        assert list(freq.values())[0] == 3

    def test_returns_tuple(self):
        """Function must return a tuple of (array, Counter)."""
        arr = np.array([[2, 0], [0, 2]], dtype=np.uint8)
        result = utils.labelling_array(arr, 2)
        assert isinstance(result, tuple)
        assert len(result) == 2
        assert isinstance(result[0], np.ndarray)
        assert isinstance(result[1], collections.Counter)

    def test_patch_sizes_correct(self):
        """Patch sizes correctly counted."""
        arr = np.array([
            [2, 2, 0],
            [2, 0, 0],
            [0, 0, 2]
        ], dtype=np.uint8)
        labeled, freq = utils.labelling_array(arr, 2)
        sizes = sorted(freq.values())
        assert sizes == [1, 3]


# =============================================================================
# get_os_info
# =============================================================================

class TestGetOsInfo:

    def test_returns_tuple_of_three(self):
        """Must return a tuple of three elements."""
        result = utils.get_os_info()
        assert isinstance(result, tuple)
        assert len(result) == 3

    def test_os_name_is_string(self):
        """OS name must be a string."""
        os_name, arch, is_win = utils.get_os_info()
        assert isinstance(os_name, str)

    def test_arch_is_string(self):
        """Architecture must be a string."""
        os_name, arch, is_win = utils.get_os_info()
        assert isinstance(arch, str)

    def test_is_win_is_bool(self):
        """is_win must be a boolean."""
        os_name, arch, is_win = utils.get_os_info()
        assert isinstance(is_win, bool)

    def test_known_os(self):
        """OS name must be one of the known platforms."""
        os_name, arch, is_win = utils.get_os_info()
        assert os_name in ['Linux', 'Windows', 'Darwin']
