"""
Tests for pyguidos.checks module.

All tests are pure Python — no GeoTIFF files or binaries required.
Run with: pytest tests/test_checks.py -v
"""

import pytest
import numpy as np
from unittest.mock import patch

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

    def test_valid_fed(self):
        """Valid FED method with valid window size."""
        assert checks.validate_frag_params(27, 'FED') is None

    def test_valid_fac_connectivity_8(self):
        """FAC with connectivity=8 should pass."""
        assert checks.validate_frag_params(27, 'FAC', 8) is None

    def test_valid_fed_connectivity_4(self):
        """FED with connectivity=4 should pass."""
        assert checks.validate_frag_params(27, 'FED', 4) is None

    def test_invalid_connectivity_fac(self):
        """FAC with invalid connectivity must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_frag_params(27, 'FAC', 6)

    def test_invalid_connectivity_fed(self):
        """FED with invalid connectivity must be rejected."""
        with pytest.raises(SystemExit):
            checks.validate_frag_params(27, 'FED', 3)

    def test_fad_ignores_connectivity(self):
        """FAD should pass regardless of connectivity value (it's ignored)."""
        assert checks.validate_frag_params(27, 'FAD', 8) is None


# =============================================================================
# validate_mspa_params
# =============================================================================

def test_validate_spa_params_success():
    """Verifies that validation passes silently for valid edge width and class numbers combinations."""
    # Test all valid classes options with a proper edge width
    for valid_class in [2, 3, 5, 6]:
        checks.validate_spa_params(edge_width=1, classes=valid_class)
        checks.validate_spa_params(edge_width=10, classes=valid_class)


def test_validate_spa_params_invalid_edge_width():
    """Ensures a SystemExit is thrown if edge_width is less than 1 or not an integer."""
    # Case 1: Edge width is 0 (violates >= 1)
    with pytest.raises(SystemExit) as exc_info:
        checks.validate_spa_params(edge_width=0, classes=2)
    assert "The edge width must be an integer number >= 1" in str(exc_info.value)

    # Case 2: Edge width is a float
    with pytest.raises(SystemExit) as exc_info:
        checks.validate_spa_params(edge_width=1.5, classes=2)
    assert "The edge width must be an integer number >= 1" in str(exc_info.value)


def test_validate_spa_params_invalid_classes():
    """Ensures a SystemExit is thrown if classes is not one of 2, 3, 5, or 6."""
    # Case 1: Classes out of bound (e.g., 4 or 7)
    with pytest.raises(SystemExit) as exc_info:
        checks.validate_spa_params(edge_width=2, classes=4)
    assert "The number of classes must be 2, 3, 5 or 6" in str(exc_info.value)

    # Case 2: String value instead of an expected integer option
    with pytest.raises(SystemExit) as exc_info:
        checks.validate_spa_params(edge_width=2, classes="6")
    assert "The number of classes must be 2, 3, 5 or 6" in str(exc_info.value)


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



# =============================================================================
# validate fragmentation change input
# =============================================================================

@pytest.fixture
def base_metadata():
    """Generates a valid dictionary of base spatial and structural characteristics."""
    return {
        "tag": "VALID_TAG_A",
        "rows": 100,
        "cols": 100,
        "bands": 1,
        "resX": 10.0,
        "resY": 10.0,
        "epsg": 4326,
        "bounds": (0, 0, 1000, 1000)
    }

@pytest.fixture
def base_tool_params():
    """Generates matching base tool parameters for a successful validation path."""
    return {
        "tool_id": "GTB_FOS",
        "tiftype": "1",
        "connect": "8",
        "method": "fixed",
        "wsize": "27"
    }


def test_validate_fchmaps_input_success(base_metadata, base_tool_params):
    """Verifies that validation completes silently when both metadata structures are completely identical."""
    meta1 = base_metadata.copy()
    meta2 = base_metadata.copy()
    meta2["tag"] = "VALID_TAG_B" # Tags can differ as long as parsed parameters match

    # Mock utils.get_tool_parameters to return identical valid profiles
    with patch("pyguidos.utils.get_tool_parameters") as mock_get_params:
        mock_get_params.side_effect = [base_tool_params.copy(), base_tool_params.copy()]
        
        # This should execute seamlessly without throwing any exceptions or exits
        checks.validate_fchmaps_input(meta1, meta2)


def test_validate_fchmaps_input_invalid_guidos_tag(base_metadata, base_tool_params):
    """Ensures a SystemExit is thrown if one or both inputs fail the initial Guidos output validation."""
    meta1 = base_metadata.copy()
    meta2 = base_metadata.copy()

    with patch("pyguidos.utils.get_tool_parameters") as mock_get_params:
        # Simulate that the second file doesn't have a valid Guidos tag
        mock_get_params.side_effect = [base_tool_params, "--"]
        
        with pytest.raises(SystemExit) as exc_info:
            checks.validate_fchmaps_input(meta1, meta2)
        
        assert "not Guidos outputs" in str(exc_info.value)


def test_validate_fchmaps_input_wrong_tool_id(base_metadata, base_tool_params):
    """Ensures a SystemExit is thrown if the tool_id is not 'GTB_FOS'."""
    meta1 = base_metadata.copy()
    meta2 = base_metadata.copy()
    
    wrong_tool_params = base_tool_params.copy()
    wrong_tool_params["tool_id"] = "GTB_SPA" # Wrong tool type

    with patch("pyguidos.utils.get_tool_parameters") as mock_get_params:
        mock_get_params.side_effect = [base_tool_params, wrong_tool_params]
        
        with pytest.raises(SystemExit) as exc_info:
            checks.validate_fchmaps_input(meta1, meta2)
        
        assert "Expected: 'GTB_FOS'" in str(exc_info.value)


def test_validate_fchmaps_input_mismatching_tool_param(base_metadata, base_tool_params):
    """Ensures a SystemExit is thrown if a fragmentation analysis setting (e.g., wsize) differs."""
    meta1 = base_metadata.copy()
    meta2 = base_metadata.copy()
    
    different_tool_params = base_tool_params.copy()
    different_tool_params["wsize"] = "13" # Mismatching window size parameter

    with patch("pyguidos.utils.get_tool_parameters") as mock_get_params:
        mock_get_params.side_effect = [base_tool_params, different_tool_params]
        
        with pytest.raises(SystemExit) as exc_info:
            checks.validate_fchmaps_input(meta1, meta2)
        
        assert "Parameter 'wsize' must be identical" in str(exc_info.value)


def test_validate_fchmaps_input_mismatching_spatial_param(base_metadata, base_tool_params):
    """Ensures a SystemExit is thrown if a spatial setting (e.g., bounding box) doesn't match."""
    meta1 = base_metadata.copy()
    meta2 = base_metadata.copy()
    meta2["bounds"] = (0, 0, 500, 500) # Alter spatial limits for file B

    with patch("pyguidos.utils.get_tool_parameters") as mock_get_params:
        mock_get_params.side_effect = [base_tool_params, base_tool_params]
        
        with pytest.raises(SystemExit) as exc_info:
            checks.validate_fchmaps_input(meta1, meta2)
        
        assert "Mismatch found in parameter 'bounds'" in str(exc_info.value)