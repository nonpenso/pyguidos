"""
Tests for pyguidos.engine module.

This file contains tests for OS detection and the heavy-duty 
image processing functions moved from utils.
Run with: pytest tests/test_engine.py -v
"""

import pytest
import collections
import numpy as np
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