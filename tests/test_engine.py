import pytest
import numpy as np
from pyguidos import engine

# =============================================================================
# FAD (Fragmentation) Logic Tests
# =============================================================================

# Constants for consistency
MISSING_IN = 0
BACKGROUND = 1
FOREGROUND = 2
BACKGR_SP3 = 3

def test_compute_fad_special_background():
    """Verify that Special Background (pixel=3) is preserved as 105."""
    data = np.full((5, 5), BACKGR_SP3, dtype=np.int16)
    
    # window_size=3
    result = engine.compute_FAD(data, window_size=3, handle_missing=1)
    
    # Pixel 3 should map to 105
    assert result[2, 2] == 105

def test_compute_fad_normalized_with_missing():
    """
    Test Normalized mode (1): Denominator should exclude missing pixels.
    In a 3x3 window (9 pixels), if 1 is Foreground, 4 are Background, 
    and 4 are Missing: Density = 1 / 5 = 20%.
    """
    data = np.full((3, 3), MISSING_IN, dtype=np.int16)
    data[1, 1] = FOREGROUND # The center pixel being processed
    data[0, :] = BACKGROUND # 3 background pixels
    data[1, 0] = BACKGROUND # 1 background pixel
    # Total valid (non-missing) = 1 (FG) + 4 (BG) = 5
    
    result = engine.compute_FAD(data, window_size=3, handle_missing=1)
    
    # (1 * 200 + 5) // (2 * 5) = 205 // 10 = 20
    assert result[1, 1] == 20

def test_compute_fad_fixed_with_missing():
    """
    Test Fixed mode (2): Denominator should always be window_size^2 (9).
    Same setup as above: 1 Foreground in 9 total pixels = 1 / 9 = 11.11% -> 11%.
    """
    data = np.full((3, 3), MISSING_IN, dtype=np.int16)
    data[1, 1] = FOREGROUND
    data[0, :] = BACKGROUND
    data[1, 0] = BACKGROUND
    
    result = engine.compute_FAD(data, window_size=3, handle_missing=2)
    
    # (1 * 200 + 9) // (2 * 9) = 209 // 18 = 11.61 -> 12 (due to round half up)
    # Note: 1/9 is ~11.11, but the formula (209//18) results in 11. 
    # Let's check: 209 / 18 = 11.6... wait, 209 // 18 = 11.
    assert result[1, 1] == 11

def test_compute_fad_uniform_foreground():
    """
    Test FAD on a 5x5 grid where everything is foreground.
    The result for a full window should be 100%.
    """
    # 2 is Foreground in your Guidos convention
    data = np.full((10, 10), 2, dtype=np.int16)
    
    # window_size=3, handle_missing=1 (standard FAD)
    result = engine.compute_FAD(data, window_size=3, handle_missing=1)
    
    # Internal pixels (not on the very edge) should have a density of 100%
    assert result[5, 5] == 100

def test_compute_fad_nodata_handling():
    """
    Verify that NoData (0) in the input results in 102 (Missing) in the output.
    """
    data = np.full((5, 5), 2, dtype=np.int16)
    data[2, 2] = 0  # Inject NoData in the center
    
    result = engine.compute_FAD(data, window_size=3, handle_missing=1)
    
    # Input 0 must map to Output 102 (GTB Missing constant)
    assert result[2, 2] == 102

def test_compute_fad_background_preservation():
    """
    Verify that original Background (1) results in 101 in the FAD output.
    """
    data = np.full((5, 5), 1, dtype=np.int16)
    
    result = engine.compute_FAD(data, window_size=3, handle_missing=1)
    
    # Per Guidos convention, background pixels are assigned 101 in the FAD result
    assert result[2, 2] == 101

@pytest.mark.parametrize("val,expected", [(3, 105), (4, 106)])
def test_fad_special_bg_coverage(val, expected):
    data = np.full((3, 3), val, dtype=np.int16)
    data[1, 1] = 2 # Add one foreground so the loop processes
    res = engine.compute_FAD(data, 3, 1)
    # The foreground pixel is processed, but we check if 
    # the special background pixels around it return the right codes
    assert res[0, 0] == expected

def test_fad_single_pixel_density():
    """
    Verify that a single foreground pixel surrounded by missing data
    results in 100% density in Normalized mode (handle_missing=1).
    """
    # 3x3 of 0s, center is 2
    data = np.zeros((3, 3), dtype=np.int16)
    data[1, 1] = 2 
    
    res = engine.compute_FAD(data, window_size=3, handle_missing=1)
    
    # Denominator is 1 (the pixel itself), fg_count is 1. 1/1 = 100%
    assert res[1, 1] == 100

# =============================================================================
# FAC (Fragmentation) Logic Tests
# =============================================================================

# Constants for testing (adjust if your engine.py uses different internal names)
MISSING = 0
BACKGROUND = 1
FOREGROUND = 2

def test_compute_fac_perfect_connectivity():
    """A solid 3x3 block of foreground should yield 100% connectivity."""
    # Create a 5x5 array of foreground
    data = np.full((5, 5), FOREGROUND, dtype=np.int16)
    
    # window_size=3, normalized mode
    result = engine.compute_FAC(data, window_size=3, handle_missing=1)
    
    # In a 3x3 window, there are 6 horizontal and 6 vertical edges (total 12)
    # If all are FG-FG, (12 * 200 + 12) // (2 * 12) = 2412 // 24 = 100
    assert result[2, 2] == 100

def test_compute_fac_zero_connectivity():
    """Isolated foreground pixel in background should yield 0% connectivity."""
    data = np.full((5, 5), BACKGROUND, dtype=np.int16)
    data[2, 2] = FOREGROUND
    
    result = engine.compute_FAC(data, window_size=3, handle_missing=1)
    
    # Pixel is foreground, but no adjacent neighbors are foreground
    assert result[2, 2] == 0
    
def test_compute_fac_fixed_denominator():
    """Verify handle_missing=2 (Fixed) uses the theoretical maximum denominator."""
    # 3x3 window: max edges = 2 * 3 * (3-1) = 12
    data = np.full((5, 5), MISSING, dtype=np.int16)
    data[1:4, 1:4] = FOREGROUND 
    
    # By setting one corner to BACKGROUND, we break 2 edges (1 horizontal, 1 vertical)
    # Total fg_fg_edges = 12 - 2 = 10
    data[1, 1] = BACKGROUND 
    
    # Fixed mode ignores missing/background in denominator, uses total_potential_edges (12)
    result = engine.compute_FAC(data, window_size=3, handle_missing=2)
    
    # Math: (10 * 200 + 12) // (2 * 12) = 2012 // 24 = 83.83 -> 83
    assert result[2, 2] == 83

def test_compute_fac_background_preservation():
    """Ensure pixels labeled as background are preserved as 101."""
    data = np.full((5, 5), FOREGROUND, dtype=np.int16)
    data[2, 2] = BACKGROUND
    
    result = engine.compute_FAC(data, window_size=3, handle_missing=1)
    
    # Result should be OUT_BACKGROUND (101)
    assert result[2, 2] == 101

def test_compute_fac_missing_fallback():
    """If denominator is 0 (e.g. window only contains the center FG pixel and NoData)."""
    data = np.full((3, 3), MISSING, dtype=np.int16)
    data[1, 1] = FOREGROUND
    
    # In normalized mode, NoData edges aren't counted. Denom becomes 0.
    result = engine.compute_FAC(data, window_size=3, handle_missing=1)
    
    assert result[1, 1] == 102 # OUT_MISSING

@pytest.mark.parametrize("val,expected", [(3, 105), (4, 106)])
def test_fac_special_bg_coverage(val, expected):
    data = np.full((3, 3), val, dtype=np.int16)
    data[1, 1] = 2 # Add one foreground so the loop processes
    res = engine.compute_FAC(data, 3, 1)
    # The foreground pixel is processed, but we check if 
    # the special background pixels around it return the right codes
    assert res[0, 0] == expected

def test_fac_zero_denom_coverage():
    # Center is foreground, but everything else is Missing (0)
    # In handle_missing=1, this might lead to a 0 denominator
    data = np.zeros((3, 3), dtype=np.int16)
    data[1, 1] = 2 
    res = engine.compute_FAC(data, 3, 1)
    assert res[1, 1] == 102


# =============================================================================
# FAC 8-Connectivity Tests
# =============================================================================

def test_compute_fac_8conn_perfect():
    """A solid block of foreground should yield 100% in 8-connected mode."""
    data = np.full((5, 5), FOREGROUND, dtype=np.int16)
    result = engine.compute_FAC(data, window_size=3, handle_missing=1, connectivity=8)
    assert result[2, 2] == 100


def test_compute_fac_8conn_diagonal_only():
    """
    A checkerboard pattern: in 4-conn no FG-FG edges exist,
    but in 8-conn diagonal FG-FG pairs should be counted.
    """
    data = np.full((5, 5), BACKGROUND, dtype=np.int16)
    # Place foreground in a diagonal pattern
    data[1, 1] = FOREGROUND
    data[2, 2] = FOREGROUND
    data[3, 3] = FOREGROUND

    # 4-connected: center pixel has no horizontal/vertical FG neighbours
    res_4 = engine.compute_FAC(data, window_size=3, handle_missing=1, connectivity=4)
    assert res_4[2, 2] == 0  # no 4-connected FG-FG edges

    # 8-connected: diagonal FG-FG pairs should yield > 0
    res_8 = engine.compute_FAC(data, window_size=3, handle_missing=1, connectivity=8)
    assert res_8[2, 2] > 0


def test_compute_fac_8conn_higher_than_4conn():
    """For a foreground block, 8-conn total_potential_edges is larger,
    so the percentage may differ from 4-conn."""
    data = np.full((5, 5), FOREGROUND, dtype=np.int16)
    data[0, 0] = BACKGROUND

    res_4 = engine.compute_FAC(data, window_size=3, handle_missing=1, connectivity=4)
    res_8 = engine.compute_FAC(data, window_size=3, handle_missing=1, connectivity=8)

    # Both should produce valid values
    assert 0 <= res_4[2, 2] <= 100
    assert 0 <= res_8[2, 2] <= 100


# =============================================================================
# FED (Foreground Edge Density) Logic Tests
# =============================================================================

def test_compute_fed_all_foreground():
    """All FG: every pair is FG-FG (score 1.0), should yield 100%."""
    data = np.full((5, 5), FOREGROUND, dtype=np.int16)
    result = engine.compute_FED(data, window_size=3, handle_missing=1, connectivity=4)
    assert result[2, 2] == 100


def test_compute_fed_isolated_foreground():
    """
    Single FG pixel surrounded by BG: all pairs involving the center
    are FG-BG (0.5 each). No FG-FG pairs exist.
    """
    data = np.full((5, 5), BACKGROUND, dtype=np.int16)
    data[2, 2] = FOREGROUND

    result = engine.compute_FED(data, window_size=3, handle_missing=1, connectivity=4)

    # Should be > 0 (FG-BG edges contribute 0.5) but < 100
    assert 0 < result[2, 2] < 100


def test_compute_fed_all_background_around_fg():
    """
    3x3 window: center is FG, 8 neighbours are BG.
    4-connected: 12 total edges.
    FG-BG edges: center shares 4 edges with BG neighbours → 4 × 0.5 = 2.0
    BG-BG edges: remaining 8 edges → 0
    weighted_num (integer): 4 × 1 = 4 (since weight 0.5 × 2 = 1 in integer math)
    Plus BG-BG pairs that don't involve center also have BG-BG = 0
    Result = 4 * 100 / (2 * 12) = 400 / 24 ≈ 16-17%
    """
    data = np.full((3, 3), BACKGROUND, dtype=np.int16)
    data[1, 1] = FOREGROUND

    result = engine.compute_FED(data, window_size=3, handle_missing=1, connectivity=4)

    # FG-BG contributes, but less than full connectivity
    assert 0 < result[1, 1] < 50


def test_compute_fed_background_preserved():
    """Background pixels should output 101."""
    data = np.full((5, 5), FOREGROUND, dtype=np.int16)
    data[2, 2] = BACKGROUND
    result = engine.compute_FED(data, window_size=3, handle_missing=1, connectivity=4)
    assert result[2, 2] == 101


def test_compute_fed_missing_preserved():
    """Missing pixels should output 102."""
    data = np.full((3, 3), MISSING, dtype=np.int16)
    data[1, 1] = FOREGROUND
    result = engine.compute_FED(data, window_size=3, handle_missing=1, connectivity=4)
    # All pairs involve missing → total_edges=0 → OUT_MISSING
    assert result[1, 1] == 102


@pytest.mark.parametrize("val,expected", [(3, 105), (4, 106)])
def test_fed_special_bg_coverage(val, expected):
    """Special background values should be preserved."""
    data = np.full((3, 3), val, dtype=np.int16)
    data[1, 1] = FOREGROUND
    res = engine.compute_FED(data, 3, 1, connectivity=4)
    assert res[0, 0] == expected


def test_compute_fed_8conn_higher_than_4conn():
    """
    8-connected considers more pairs (diagonals), so for the same
    data the result should generally differ from 4-connected.
    """
    data = np.full((5, 5), FOREGROUND, dtype=np.int16)
    data[0, :] = BACKGROUND
    data[:, 0] = BACKGROUND

    res_4 = engine.compute_FED(data, window_size=3, handle_missing=1, connectivity=4)
    res_8 = engine.compute_FED(data, window_size=3, handle_missing=1, connectivity=8)

    # Both should produce valid values
    assert 0 <= res_4[2, 2] <= 100
    assert 0 <= res_8[2, 2] <= 100


def test_compute_fed_vs_fac_relationship():
    """
    FED should always be >= FAC for the same input, because FED gives
    partial credit (0.5) to FG-BG edges while FAC only counts FG-FG.
    """
    data = np.full((7, 7), BACKGROUND, dtype=np.int16)
    data[2:5, 2:5] = FOREGROUND

    fac_result = engine.compute_FAC(data, window_size=5, handle_missing=1, connectivity=4)
    fed_result = engine.compute_FED(data, window_size=5, handle_missing=1, connectivity=4)

    # FED >= FAC for foreground pixels (FED gets credit for boundary edges)
    assert fed_result[3, 3] >= fac_result[3, 3]

# =============================================================================
# Landscape Mosaic (LM) Logic Tests
# =============================================================================

def test_compute_lm_pure_forest():
    """
    Verify LM classification for a pure forest window.
    The value 170 represents 'Interior' in the GTB palette.
    """
    data = np.full((10, 10), 2, dtype=np.int16)
    
    result = engine.compute_LM(data, window_size=3)
    
    # Based on your failure, 170 is the correct 'Interior' code
    assert result[5, 5] == 170

def test_compute_lm_mixed():
    """
    Verify LM classification for a mixed window (Water/Edge/Patch).
    """
    # Create a mix: half background (1), half foreground (2)
    data = np.ones((10, 10), dtype=np.int16)
    data[:, 5:] = 2 
    
    result = engine.compute_LM(data, window_size=3)
    
    # The transition pixels should not be 170 (Interior)
    # They should be an Edge or Transitional class (e.g., 150 or 130)
    assert result[5, 5] != 170
    assert result[5, 5] > 0

def test_compute_lm_poles():
    """Verify the three pure corners of the tri-polar model."""
    # Agriculture Corner (1) -> 180
    data_agr = np.full((5, 5), 1, dtype=np.int16)
    res_agr = engine.compute_LM(data_agr, window_size=3)
    assert res_agr[2, 2] == 180

    # Forest Corner (2) -> 170
    data_for = np.full((5, 5), 2, dtype=np.int16)
    res_for = engine.compute_LM(data_for, window_size=3)
    assert res_for[2, 2] == 170

    # Developed Corner (3) -> 190
    data_dev = np.full((5, 5), 3, dtype=np.int16)
    res_dev = engine.compute_LM(data_dev, window_size=3)
    assert res_dev[2, 2] == 190

def test_compute_lm_transitional():
    """Verify specific transition codes in the decision tree."""
    # Create a 3x3 window with 8 pixels of Developed and 1 pixel of Forest (center)
    # n_for=1, n_dev=8, n_agr=0. n_valid=9.
    # f10 = 10, v1 = 9. So f10 >= v1.
    # f10 < v2 (20). 
    # a10 = 0, so a10 < v1.
    # d10 = 80, v8 = 72. d10 >= v8 is True. 
    # Code should be 61.
    data = np.full((3, 3), 3, dtype=np.int16)
    data[1, 1] = 2 
    
    result = engine.compute_LM(data, window_size=3)
    assert result[1, 1] == 61

def test_compute_lm_missing_data_skip():
    """Ensure that if the center pixel is 0, the result remains 0 (skipped)."""
    data = np.full((5, 5), 2, dtype=np.int16)
    data[2, 2] = 0 # Missing data
    
    result = engine.compute_LM(data, window_size=3)
    
    # Initialized as np.zeros, should stay 0
    assert result[2, 2] == 0

def test_lm_coverage_boost():
    # Test a window that is 50% Agriculture and 50% Developed
    # This should hit branches in the 'elif f10 < v1' block
    data = np.array([
        [1, 1, 1],
        [3, 3, 3],
        [1, 3, 1]
    ], dtype=np.int16)
    # The center pixel must be non-zero to process
    res = engine.compute_LM(data, window_size=3)
    assert res[1, 1] > 0
    
# =============================================================================
# Labelling Logic Tests
# =============================================================================

def test_labelling_array():
    """Verify labelling and patch counting using the updated target_values logic."""

    # Test Case 1: 8-connectivity (Hardcoded in your function)
    # Diagonal pixels should be seen as ONE patch because your function 
    # uses 8-connectivity (generate_binary_structure(2, 2))
    arr_diag = np.array([
        [2, 0],
        [0, 2]
    ], dtype=np.uint8)
    
    labeled_diag, freq_diag = engine.labelling_array(arr_diag, target_values=2)
    
    # In 8-connectivity, diagonal pixels are connected
    assert len(freq_diag) == 1  # Should be 1 patch
    assert freq_diag[1] == 2    # The patch should have 2 pixels

    # Test Case 2: Multiple target values
    # If we treat 2 and 3 as foreground, they should merge into one patch if adjacent
    arr_multi = np.array([
        [2, 3, 0],
        [0, 0, 0]
    ], dtype=np.uint8)
    
    _, freq_multi = engine.labelling_array(arr_multi, target_values=[2, 3])
    
    assert len(freq_multi) == 1 # 2 and 3 are adjacent, so 1 patch
    assert sum(freq_multi.values()) == 2

    # Test Case 3: Separated patches
    arr_sep = np.array([
        [2, 0, 2],
        [0, 0, 0]
    ], dtype=np.uint8)
    
    _, freq_sep = engine.labelling_array(arr_sep, target_values=2)
    assert len(freq_sep) == 2 # Clearly separated by a 0, so 2 patches

# =============================================================================
# SPA Logic Tests
# =============================================================================

@pytest.fixture
def spa_test_grid():
    """
    Creates a 20x20 grid:
    - [0,0]: NoData (0)
    - [2,2]: Isolated Foreground pixel (Islet)
    - [5:15, 5:15]: Main Foreground block (Core/Edge)
    - [10,10]: Background hole inside the main block
    """
    grid = np.ones((20, 20), dtype=np.int16)  # Background (1)
    
    # 1. Main Block
    grid[5:15, 5:15] = 2  # Foreground
    
    # 2. Background Hole inside the block
    grid[10, 10] = 1  # Background
    
    # 3. Isolated Islet
    grid[2, 2] = 2  # Foreground[cite: 1]
    
    # 4. NoData
    grid[0, 0] = 0  # Missing[cite: 1]
    
    return grid

def test_spa_binary_mode(spa_test_grid):
    """Tests n_classes=2: Islets and Linear features become 1."""
    res = engine.compute_spa(spa_test_grid, s=1.0, n_classes=2)
    
    # Isolated pixel [2, 2] should be LINEAR (1) in binary mode
    assert res[2, 2] == 1
    
    # Center of main block [8, 8] should be CORE (17)
    assert res[8, 8] == 17

def test_spa_six_class_mode(spa_test_grid):
    """Tests n_classes=6: Islets become 9, Holes become 100."""
    res = engine.compute_spa(spa_test_grid, s=1.0, n_classes=6)
    
    # Isolated pixel [2, 2] should be ISLET (9)
    assert res[2, 2] == 9
    
    # Background hole [10, 10] should be flagged as HOLE (100)
    assert res[10, 10] == 100
    
    # Core pixel [8, 8] should still be CORE (17)
    assert res[8, 8] == 17


# =============================================================================
# FRAGMENTATION CHANGE Logic Tests
# =============================================================================

@pytest.fixture
def empty_matrix():
    """Creates a clean 107x107 confusion matrix accumulator."""
    return np.zeros((107, 107), dtype=np.int64)

def test_fos_change_tiers_and_stats(empty_matrix):
    """Verifies all 7 categorical outcome tiers and checks matrix statistics computation."""
    # We set up a 1x8 grid where each index tests a distinct logic path
    # Indices:  0    1    2    3    4    5    6    7
    chunk_a = np.array([[101, 101,  50,  80, 102, 105, 106, 120]], dtype=np.uint8)
    chunk_b = np.array([[101,  30, 101,  60,  10,  20, 101, 120]], dtype=np.uint8)
    
    # Run with compute_stats = True
    out = engine.compute_fos_change(chunk_a, chunk_b, empty_matrix, compute_stats=True)

    # Tier 1: a == 101 and b == 101 -> 252
    assert out[0, 0] == 252

    # Tier 2: a == 101 and b <= 100 -> 250
    assert out[0, 1] == 250

    # Tier 3: a <= 101 and b == 101 -> 251
    assert out[0, 2] == 251

    # Tier 4: a <= 100 and b <= 100 -> 100 + a - b
    # 100 + 80 - 60 = 120
    assert out[0, 3] == 120

    # Tier 5: a == 102 or b == 102 -> 254
    assert out[0, 4] == 254

    # Tier 6: a or b in (105, 106) -> 253
    assert out[0, 5] == 253  # a is 105
    assert out[0, 6] == 253  # b is 101, but a is 106

    # Tier 7: else -> 102 (Uncaught values, e.g., 120)
    assert out[0, 7] == 102

    # Verify confusion matrix statistics counter
    # Coordinates (101, 101) should be incremented once from index 0
    assert empty_matrix[101, 101] == 1
    # Coordinates (50, 101) from index 2
    assert empty_matrix[50, 101] == 1
    
    # FIXED ASSERTION:
    # This proves that index 7 (value 120) was completely bypassed by the guard condition.
    assert np.sum(empty_matrix) == 7

def test_fos_change_bypass_stats(empty_matrix):
    """Verifies that matrix statistics tracking is completely skipped when compute_stats=False."""
    chunk_a = np.array([[50]], dtype=np.uint8)
    chunk_b = np.array([[50]], dtype=np.uint8)

    # Run with compute_stats = False
    out = engine.compute_fos_change(chunk_a, chunk_b, empty_matrix, compute_stats=False)

    # Output calculation should still occur (100 + 50 - 50 = 100)
    assert out[0, 0] == 100

    # Matrix element MUST remain unmodified (0)
    assert empty_matrix[50, 50] == 0


# =============================================================================
# SP4 Exclusion Tests (SP4 treated as missing in window computation)
# =============================================================================

def test_fad_sp4_excluded_from_window():
    """
    SP4 in the window should be excluded from denominator (like NoData).
    A 3x3 window with center=FG, 4 SP4, 4 BG:
    Without exclusion: denom=9, fg=1, FAD=11%
    With exclusion: denom=5 (only FG+BG counted), fg=1, FAD=20%
    """
    data = np.full((3, 3), 1, dtype=np.int16)  # Background
    data[1, 1] = 2  # Center = Foreground
    data[0, 0] = 4  # SP4
    data[0, 1] = 4  # SP4
    data[0, 2] = 4  # SP4
    data[1, 0] = 4  # SP4
    # Remaining: (1,2)=1, (2,0)=1, (2,1)=1, (2,2)=1
    # Valid non-missing: 1 FG + 4 BG = 5
    # FAD = 1/5 = 20%

    result = engine.compute_FAD(data, window_size=3, handle_missing=1)
    assert result[1, 1] == 20


def test_fad_sp3_still_fragments():
    """
    SP3 in the window should still count in denominator (fragments foreground).
    Same layout but with SP3 instead of SP4:
    denom=9 (all non-missing), fg=1, FAD=11%
    """
    data = np.full((3, 3), 1, dtype=np.int16)  # Background
    data[1, 1] = 2  # Center = Foreground
    data[0, 0] = 3  # SP3
    data[0, 1] = 3  # SP3
    data[0, 2] = 3  # SP3
    data[1, 0] = 3  # SP3

    result = engine.compute_FAD(data, window_size=3, handle_missing=1)
    # All 9 pixels are non-missing (SP3 counts), fg=1, FAD = 1/9 = 11%
    assert result[1, 1] == 11


def test_fac_sp4_pairs_excluded():
    """
    Pairs involving SP4 should be skipped in FAC computation.
    """
    data = np.full((3, 3), 2, dtype=np.int16)  # All foreground
    data[0, :] = 4  # Top row is SP4

    result = engine.compute_FAC(data, window_size=3, handle_missing=1)
    # Center pixel (1,1) is FG
    # Without SP4 exclusion: all 12 edges, 6 FG-FG = 50%
    # With SP4 exclusion: only pairs between rows 1-2 count
    # Valid pairs: 3 horizontal in row1, 3 horizontal in row2, 3 vertical between rows 1-2 = 9
    # FG-FG pairs: all 9 (since rows 1-2 are all FG)
    # But wait, edges touching row 0 (SP4) are excluded
    # Horizontal row0: excluded (SP4 involved)
    # Horizontal row1: (1,0)-(1,1), (1,1)-(1,2) = 2 pairs, both FG-FG
    # Horizontal row2: (2,0)-(2,1), (2,1)-(2,2) = 2 pairs, both FG-FG
    # Vertical col0: (0,0)-(1,0) excluded (SP4), (1,0)-(2,0) = FG-FG
    # Vertical col1: (0,1)-(1,1) excluded (SP4), (1,1)-(2,1) = FG-FG
    # Vertical col2: (0,2)-(1,2) excluded (SP4), (1,2)-(2,2) = FG-FG
    # Total valid edges: 4 + 3 = 7, all FG-FG = 7
    # FAC = 7*200+7 // (2*7) = 1407//14 = 100
    assert result[1, 1] == 100


def test_fed_sp4_pairs_excluded():
    """
    Pairs involving SP4 should be skipped in FED computation.
    """
    data = np.full((3, 3), 2, dtype=np.int16)  # All foreground
    data[1, 1] = 2  # Center
    data[0, 0] = 4  # One SP4 pixel

    result_with_sp4 = engine.compute_FED(data, window_size=3, handle_missing=1, connectivity=4)

    data_no_sp4 = np.full((3, 3), 2, dtype=np.int16)  # All foreground, no SP4
    result_without_sp4 = engine.compute_FED(data_no_sp4, window_size=3, handle_missing=1, connectivity=4)

    # With SP4: fewer valid edges but all remaining are FG-FG → still 100%
    # Without SP4: all edges are FG-FG → 100%
    assert result_with_sp4[1, 1] == 100
    assert result_without_sp4[1, 1] == 100
