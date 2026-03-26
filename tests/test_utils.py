"""
Tests for pyguidos.utils module.

All tests use numpy arrays or strings — no GeoTIFF files required.
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
        """Basic uint8 2D array."""
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
        freq = utils.get_pxl_freq(arr)
        assert freq[1] == 16
        assert 2 not in freq

    def test_uint8_returns_counter(self):
        """Result must be a collections.Counter instance."""
        arr = np.array([[1, 2]], dtype=np.uint8)
        freq = utils.get_pxl_freq(arr)
        assert isinstance(freq, collections.Counter)

    def test_int32_general_path(self):
        """Non-uint8 path (uses chunking internally)."""
        arr = np.array([[0, 1000], [1000, 0]], dtype=np.int32)
        freq = utils.get_pxl_freq(arr)
        assert freq[0] == 2
        assert freq[1000] == 2


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
        assert result["web_link"] == "https://forest.jrc.ec.europa.eu/"

    def test_fos_tag(self):
        """GTB_FOS tag parsed correctly."""
        tag = "GTB_FOS, <Binary,-1,8,FAD_5,100.000,27>, https://forest.jrc.ec.europa.eu/"
        result = utils.get_tool_parameters(tag)
        assert result["tool_id"] == "GTB_FOS"
        assert result["wsize"] == "27"

    def test_acc_tag(self):
        """GTB_ACC tag parsed correctly."""
        tag = "GTB_ACC, <100,1000,10000>, https://forest.jrc.ec.europa.eu/"
        result = utils.get_tool_parameters(tag)
        assert result["tool_id"] == "GTB_ACC"
        assert result["thresholds"] == ["100", "1000", "10000"]

    def test_empty_tag_returns_none(self):
        """Empty or invalid tag must return None."""
        assert utils.get_tool_parameters("") is None
        assert utils.get_tool_parameters(None) is None
        assert utils.get_tool_parameters("--") is None