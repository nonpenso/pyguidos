"""
Tests for pyguidos.checks module.

All tests are pure Python — no GeoTIFF files or binaries required.
Run with: pytest tests/test_checks.py -v
"""

import pytest
import numpy as np
from pyguidos import checks


# =============================================================================
# validate_wsize
# =============================================================================

class TestValidateWsize:

    def test_valid_odd_minimum(self):
        """Minimum valid window size is 3."""
        assert checks.validate_wsize(3) is None

    def test_valid_odd_larger(self):
        """Larger odd window sizes."""
        for wsize in [5, 7, 11, 27, 101]:
            assert checks.validate_wsize(wsize) is None

    def test_even_number_raises(self):
        """Even window sizes must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_wsize(4)

    def test_even_number_raises_large(self):
        """Even large window size."""
        with pytest.raises(SystemExit):
            checks.validate_wsize(28)

    def test_too_small_raises(self):
        """Window size below minimum (< 3) must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_wsize(2)

    def test_one_raises(self):
        """Window size of 1 must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_wsize(1)

    def test_zero_raises(self):
        """Zero window size must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_wsize(0)

    def test_negative_raises(self):
        """Negative window size must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_wsize(-5)

    def test_float_raises(self):
        """Float window size must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_wsize(7.0)

    def test_string_raises(self):
        """String window size must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_wsize("7")

    def test_none_raises(self):
        """None window size must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_wsize(None)


# =============================================================================
# validate_fmap_input
# =============================================================================

class TestValidateFmapInput:

    def test_valid_binary_mspa(self):
        """Minimal valid MSPA input: values 1 and 2."""
        assert checks.validate_fmap_input([1, 2], bands=1, dtype=np.uint8, allow_34=False) is None

    def test_valid_binary_with_nodata(self):
        """Valid input with NoData (0)."""
        assert checks.validate_fmap_input([0, 1, 2], bands=1, dtype='uint8', allow_34=False) is None

    def test_valid_with_special_classes(self):
        """Valid fragmentation input with special background classes 3 and 4."""
        assert checks.validate_fmap_input([0, 1, 2, 3, 4], bands=1, dtype=np.int16, allow_34=True) is None

    def test_valid_partial_special_classes(self):
        """Valid input with only one special class."""
        assert checks.validate_fmap_input([1, 2, 3], bands=1, dtype=np.uint8, allow_34=True) is None

    def test_missing_foreground_raises(self):
        """Missing foreground (value 2) must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_fmap_input([0, 1], bands=1, dtype=np.uint8, allow_34=False)

    def test_missing_background_raises(self):
        """Missing background (value 1) must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_fmap_input([0, 2], bands=1, dtype=np.uint8, allow_34=False)

    def test_invalid_value_mspa_raises(self):
        """Value 3 not allowed for MSPA (allow_34=False)."""
        with pytest.raises(SystemExit):
            checks.validate_fmap_input([1, 2, 3], bands=1, dtype=np.uint8, allow_34=False)

    def test_multiband_raises(self):
        """Multi-band raster must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_fmap_input([1, 2], bands=3, dtype=np.uint8, allow_34=False)

    def test_float_dtype_raises(self):
        """Float32 must be rejected for Fmap."""
        with pytest.raises(SystemExit):
            checks.validate_fmap_input([1, 2], bands=1, dtype=np.float32, allow_34=False)

    def test_different_int_types_pass(self):
        """Verify that various integer depths (int16, int32) are accepted."""
        assert checks.validate_fmap_input([1, 2], bands=1, dtype=np.int32, allow_34=False) is None


# =============================================================================
# validate_lm_input
# =============================================================================

class TestValidateLmInput:

    def test_valid_three_classes(self):
        """Valid Land Mosaic input with all three mandatory classes."""
        assert checks.validate_lm_input([1, 2, 3], bands=1, dtype=np.uint8) is None

    def test_valid_with_nodata(self):
        """Valid input with NoData (0)."""
        assert checks.validate_lm_input([0, 1, 2, 3], bands=1, dtype='uint8') is None

    def test_missing_class_raises(self):
        """Missing mandatory classes must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_lm_input([1, 2], bands=1, dtype=np.uint8)

    def test_invalid_value_raises(self):
        """Value 5 is not allowed."""
        with pytest.raises(SystemExit):
            checks.validate_lm_input([1, 2, 3, 5], bands=1, dtype=np.uint8)

    def test_multiband_raises(self):
        """Multi-band raster must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_lm_input([1, 2, 3], bands=3, dtype=np.uint8)

    def test_float_dtype_raises(self):
        """Float64 must be rejected for Landscape Mosaic."""
        with pytest.raises(SystemExit):
            checks.validate_lm_input([1, 2, 3], bands=1, dtype=np.float64)

    def test_int16_passes(self):
        """Int16 is a valid integer type."""
        assert checks.validate_lm_input([1, 2, 3], bands=1, dtype=np.int16) is None


# =============================================================================
# validate_frag_params
# =============================================================================

class TestValidateFragParams:

    def test_valid_fad(self):
        """Valid FAD method with valid window size."""
        assert checks.validate_frag_params(27, 'FAD') is None

    def test_valid_fac(self):
        """Valid FAC method with valid window size."""
        assert checks.validate_frag_params(27, 'FAC') is None

    def test_valid_minimum_wsize(self):
        """Minimum valid window size with FAD."""
        assert checks.validate_frag_params(3, 'FAD') is None

    def test_invalid_method_fos_raises(self):
        """FOS is not a valid method — must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_frag_params(27, 'FOS')

    def test_invalid_method_empty_raises(self):
        """Empty method string must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_frag_params(27, '')

    def test_invalid_method_lowercase_raises(self):
        """Lowercase method must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_frag_params(27, 'fad')

    def test_invalid_wsize_even_raises(self):
        """Even window size must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_frag_params(28, 'FAD')

    def test_invalid_wsize_too_small_raises(self):
        """Window size below minimum must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_frag_params(2, 'FAD')

    def test_invalid_wsize_float_raises(self):
        """Float window size must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_frag_params(27.0, 'FAD')


# =============================================================================
# validate_mspa_params
# =============================================================================

# class TestValidateMspaParams:

#     def test_valid_connectivity_8(self):
#         """Valid edge_width=1 with connectivity=8."""
#         assert checks.validate_mspa_params(1, 8) is None

#     def test_valid_connectivity_4(self):
#         """Valid edge_width=1 with connectivity=4."""
#         assert checks.validate_mspa_params(1, 4) is None

#     def test_valid_larger_edge_width(self):
#         """Valid larger edge_width values."""
#         for ew in [2, 3, 5, 10]:
#             assert checks.validate_mspa_params(ew, 8) is None

#     def test_invalid_connectivity_raises(self):
#         """Connectivity values other than 4 or 8 must be rejected."""
#         with pytest.raises(SystemExit):
#             checks.validate_mspa_params(1, 6)

#     def test_invalid_connectivity_1_raises(self):
#         """Connectivity=1 must be rejected."""
#         with pytest.raises(SystemExit):
#             checks.validate_mspa_params(1, 1)

#     def test_invalid_edge_width_zero_raises(self):
#         """Edge width of 0 must be rejected."""
#         with pytest.raises(SystemExit):
#             checks.validate_mspa_params(0, 8)

#     def test_invalid_edge_width_negative_raises(self):
#         """Negative edge width must be rejected."""
#         with pytest.raises(SystemExit):
#             checks.validate_mspa_params(-1, 8)

#     def test_invalid_edge_width_float_raises(self):
#         """Float edge width must be rejected."""
#         with pytest.raises(SystemExit):
#             checks.validate_mspa_params(1.5, 8)

#     def test_invalid_edge_width_string_raises(self):
#         """String edge width must be rejected."""
#         with pytest.raises(SystemExit):
#             checks.validate_mspa_params("1", 8)


# =============================================================================
# validate_acc_params
# =============================================================================

class TestValidateAccParams:

    def test_valid_single_threshold(self):
        """Minimum valid input: one threshold."""
        result = checks.validate_acc_params([100])
        assert result == [100]

    def test_valid_five_thresholds(self):
        """Maximum valid input: five thresholds."""
        result = checks.validate_acc_params([10, 100, 1000, 10000, 100000])
        assert result == [10, 100, 1000, 10000, 100000]

    def test_returns_sorted_list(self):
        """Thresholds must be returned sorted."""
        result = checks.validate_acc_params([1000, 10, 100])
        assert result == [10, 100, 1000]

    def test_duplicates_removed(self):
        """Duplicate thresholds must be removed."""
        result = checks.validate_acc_params([100, 100, 1000])
        assert result == [100, 1000]

    def test_tuple_input(self):
        """Tuple input must be accepted."""
        result = checks.validate_acc_params((100, 1000, 10000))
        assert result == [100, 1000, 10000]

    def test_numpy_array_input(self):
        """Numpy array input must be accepted."""
        result = checks.validate_acc_params(np.array([100, 1000, 10000]))
        assert result == [100, 1000, 10000]

    def test_empty_raises(self):
        """Empty thresholds must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_acc_params([])

    def test_too_many_thresholds_raises(self):
        """More than 5 unique thresholds must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_acc_params([1, 2, 3, 4, 5, 6])

    def test_negative_threshold_raises(self):
        """Negative thresholds must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_acc_params([-100, 1000])

    def test_zero_threshold_raises(self):
        """Zero threshold must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_acc_params([0, 100, 1000])

    def test_non_integer_raises(self):
        """Non-integer string thresholds must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_acc_params(['a', 'b'])

    def test_wrong_type_raises(self):
        """Non-sequence input must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_acc_params(100)

    def test_string_input_raises(self):
        """String input must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_acc_params("100")

    def test_six_values_with_dups_reducing_to_five_passes(self):
        """6 values with duplicates reducing to 5 unique must pass."""
        result = checks.validate_acc_params([10, 10, 100, 1000, 10000, 100000])
        assert len(result) == 5
