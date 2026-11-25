import numpy as np
from scipy.ndimage import binary_fill_holes, label, generate_binary_structure
from skimage.segmentation import flood_fill
import math
import gc
from numba import njit, prange, get_num_threads

from . import utils

# Global Constants
MISSING_IN = 0
BACKGROUND = 1
FOREGROUND = 2
BACKGR_SP3 = 3
BACKGR_SP4 = 4



#############################
# ---- SPATCON FUNCTIONS ----
#############################

# ---------------------------------------------------------------------------
# Shared moving-window engine (running sums, exact integer arithmetic)
# ---------------------------------------------------------------------------
# Every moving-window index (FAD, FAC, FED, their grayscale versions and the
# Landscape Mosaic) only needs a few SUMS over the window: pixel counts or
# values, or scores of adjacent pixel pairs. Instead of re-reading the whole
# window for every pixel (cost ~ window_size^2), each CPU processes a block of
# complete image rows and keeps running column sums: moving down one row adds
# the row entering the window and subtracts the row leaving it. Within a row,
# prefix sums give any window total with one subtraction. The cost per pixel
# no longer depends on the window size. Windows are clipped at the image
# border exactly as before and all arithmetic is integer, so results are
# identical to the original per-pixel loops.

_K_FAD = 1
_K_FAD_GRAY = 2
_K_FAC = 3
_K_FED = 4
_K_FAC_GRAY = 5
_K_FED_GRAY = 6
_K_LM = 7


@njit(cache=True)
def _pix_contrib(kind, v):
    """Window quantities of one pixel: (a, b, c)."""
    if kind == _K_FAD:
        a = 1 if v == FOREGROUND else 0
        b = 1 if (v != MISSING_IN and v != BACKGR_SP4) else 0   # SP4 acts as missing
        return np.int64(a), np.int64(b), np.int64(0)
    elif kind == _K_FAD_GRAY:
        if v > 100:                                              # NoData
            return np.int64(0), np.int64(0), np.int64(0)
        return np.int64(v), np.int64(1), np.int64(0)
    else:                                                        # _K_LM
        return (np.int64(1 if v == 1 else 0), np.int64(1 if v == 2 else 0),
                np.int64(1 if v == 3 else 0))


@njit(cache=True)
def _pair_contrib(kind, v1, v2):
    """Window quantities of one adjacent pixel pair: (numerator, valid pair)."""
    if kind == _K_FAC or kind == _K_FED:
        if v1 == MISSING_IN or v2 == MISSING_IN or v1 == BACKGR_SP4 or v2 == BACKGR_SP4:
            return np.int64(0), np.int64(0)
        f1 = v1 == FOREGROUND
        f2 = v2 == FOREGROUND
        if kind == _K_FAC:
            return np.int64(1 if (f1 and f2) else 0), np.int64(1)
        if f1 and f2:                                            # FED: FG-FG = 2
            return np.int64(2), np.int64(1)
        if f1 != f2:                                             # FED: FG-BG = 1
            return np.int64(1), np.int64(1)
        return np.int64(0), np.int64(1)
    else:                                                        # grayscale FAC / FED
        if v1 > 100 or v2 > 100:
            return np.int64(0), np.int64(0)
        if kind == _K_FAC_GRAY and not (v1 > 0 and v2 > 0):
            return np.int64(0), np.int64(1)
        return np.int64(v1) + np.int64(v2), np.int64(1)


@njit(cache=True)
def _needs_window(kind, v, thr):
    """True if the output of a pixel with value v depends on its window."""
    if kind == _K_FAD or kind == _K_FAC or kind == _K_FED:
        return v == FOREGROUND
    if kind == _K_LM:
        return v != 0
    return v <= 100 and v >= thr                                 # grayscale


@njit(cache=True)
def _classify(kind, v, a, b, c, num, den, handle_missing, w2, tpe, thr):
    """Output value of one pixel from its window sums (rules unchanged)."""
    if kind == _K_LM:
        if v == 0 or a + b + c == 0:
            return np.uint8(0)
        return _lm_code(a, b, c)

    if kind == _K_FAD or kind == _K_FAC or kind == _K_FED:
        if v == BACKGROUND:
            return np.uint8(101)
        if v == BACKGR_SP3:
            return np.uint8(105)
        if v == BACKGR_SP4:
            return np.uint8(106)
        if v != FOREGROUND:
            return np.uint8(102)
        if kind == _K_FAD:
            denom = b if handle_missing == 1 else w2
            numer = a * 200
        elif kind == _K_FAC:
            denom = den if handle_missing == 1 else tpe
            numer = num * 200
        else:
            denom = den if handle_missing == 1 else tpe
            numer = num * 100
    else:                                                        # grayscale
        if v > 100:
            return np.uint8(102)
        if v < thr:
            return np.uint8(101)
        if kind == _K_FAD_GRAY:
            denom = (b if handle_missing == 1 else w2) * 100
            numer = a * 200
        else:
            denom = (den if handle_missing == 1 else tpe) * 200
            numer = num * 200
    if denom > 0:
        pf = (numer + denom) // (2 * denom)
        return np.uint8(min(pf, 100))
    return np.uint8(102)


@njit(cache=True)
def _add_pixel_row(data, r, sign, kind, use_pix, use_pairs, cp, ch):
    """Add (sign=+1) or remove (sign=-1) image row r: pixel and horizontal-pair sums."""
    ncols = data.shape[1]
    for c in range(ncols):
        v = data[r, c]
        if use_pix:
            a, b, cc = _pix_contrib(kind, v)
            cp[0, c] += sign * a
            cp[1, c] += sign * b
            cp[2, c] += sign * cc
        if use_pairs and c < ncols - 1:                          # pair (r,c)-(r,c+1)
            n, d = _pair_contrib(kind, v, data[r, c + 1])
            ch[0, c] += sign * n
            ch[1, c] += sign * d


@njit(cache=True)
def _add_pair_row(data, r, sign, kind, use_diag, cv, cd1, cd2):
    """Add or remove the vertical/diagonal pairs between image rows r and r+1."""
    ncols = data.shape[1]
    for c in range(ncols):
        v1 = data[r, c]
        n, d = _pair_contrib(kind, v1, data[r + 1, c])           # (r,c)-(r+1,c)
        cv[0, c] += sign * n
        cv[1, c] += sign * d
        if use_diag:
            if c < ncols - 1:                                    # (r,c)-(r+1,c+1)
                n, d = _pair_contrib(kind, v1, data[r + 1, c + 1])
                cd1[0, c] += sign * n
                cd1[1, c] += sign * d
            if c >= 1:                                           # (r,c)-(r+1,c-1)
                n, d = _pair_contrib(kind, v1, data[r + 1, c - 1])
                cd2[0, c] += sign * n
                cd2[1, c] += sign * d


@njit(cache=True)
def _prefix(col, pre, nq):
    """pre[q, k] = sum of col[q, :k]."""
    ncols = col.shape[1]
    for q in range(nq):
        s = np.int64(0)
        pre[q, 0] = 0
        for k in range(ncols):
            s += col[q, k]
            pre[q, k + 1] = s


def _n_blocks(data):
    """
    Number of row blocks for _window_kernel: one per Numba thread. Computed
    in Python, so the compiled kernel does not call get_num_threads() and
    stays cacheable on disk.
    """
    return max(1, min(data.shape[0], get_num_threads()))


@njit(parallel=True, cache=True)
def _window_kernel(data, window_size, kind, handle_missing, connectivity, thr, nblocks):
    """
    Shared moving-window engine (see the section comment above).
    Windows: rows [i-R, i+R] and columns [j-R, j+R] clipped to the image,
    with R = window_size // 2. Pixel quantities use all window pixels; pair
    quantities use the pairs whose two pixels are both inside the window
    (horizontal, vertical and, with connectivity 8, both diagonals).
    The image is split into `nblocks` blocks of complete rows, processed in
    parallel; each block starts its running sums from scratch, so the
    result does not depend on the number of blocks.
    """
    nrows, ncols = data.shape
    R = window_size // 2
    W = np.int64(window_size)
    w2 = W * W
    if connectivity == 8:
        tpe = 2 * (W - 1) * (2 * W - 1)
    else:
        tpe = 2 * W * (W - 1)
    use_pix = kind == _K_FAD or kind == _K_FAD_GRAY or kind == _K_LM
    use_pairs = not use_pix
    use_diag = use_pairs and connectivity == 8

    result = np.empty((nrows, ncols), dtype=np.uint8)
    nblocks = max(1, min(nrows, nblocks))

    for blk in prange(nblocks):
        i0 = (blk * nrows) // nblocks
        i1 = ((blk + 1) * nrows) // nblocks
        # Column sums of the rows currently inside the window
        cp = np.zeros((3, ncols), dtype=np.int64)    # pixels: a, b, c
        ch = np.zeros((2, ncols), dtype=np.int64)    # horizontal pairs
        cv = np.zeros((2, ncols), dtype=np.int64)    # vertical pairs
        cd1 = np.zeros((2, ncols), dtype=np.int64)   # diagonal (r,c)-(r+1,c+1)
        cd2 = np.zeros((2, ncols), dtype=np.int64)   # diagonal (r,c)-(r+1,c-1)
        pp = np.zeros((3, ncols + 1), dtype=np.int64)
        ph = np.zeros((2, ncols + 1), dtype=np.int64)
        pv = np.zeros((2, ncols + 1), dtype=np.int64)
        pd1 = np.zeros((2, ncols + 1), dtype=np.int64)
        pd2 = np.zeros((2, ncols + 1), dtype=np.int64)

        # Window rows of the first row of the block: pixels [r0, r1), pairs [r0, r1-1)
        r0 = max(0, i0 - R)
        r1 = min(nrows, i0 + R + 1)
        for r in range(r0, r1):
            _add_pixel_row(data, r, 1, kind, use_pix, use_pairs, cp, ch)
        if use_pairs:
            for r in range(r0, r1 - 1):
                _add_pair_row(data, r, 1, kind, use_diag, cv, cd1, cd2)

        for i in range(i0, i1):
            if i > i0:
                # Window moves down one row
                if i + R + 1 <= nrows:                   # row r1 enters
                    _add_pixel_row(data, r1, 1, kind, use_pix, use_pairs, cp, ch)
                    if use_pairs and r1 - 1 >= r0:       # pair rows r1-1 / r1 enter
                        _add_pair_row(data, r1 - 1, 1, kind, use_diag, cv, cd1, cd2)
                    r1 += 1
                if i - R - 1 >= 0:                       # row r0 leaves
                    _add_pixel_row(data, r0, -1, kind, use_pix, use_pairs, cp, ch)
                    if use_pairs and r0 < r1 - 1:        # pair rows r0 / r0+1 leave
                        _add_pair_row(data, r0, -1, kind, use_diag, cv, cd1, cd2)
                    r0 += 1

            # Prefix sums along the row for O(1) window totals
            if use_pix:
                _prefix(cp, pp, 3)
            else:
                _prefix(ch, ph, 2)
                _prefix(cv, pv, 2)
                if use_diag:
                    _prefix(cd1, pd1, 2)
                    _prefix(cd2, pd2, 2)

            for j in range(ncols):
                v = data[i, j]
                a = b = c = num = den = np.int64(0)
                if _needs_window(kind, v, thr):
                    c0 = max(0, j - R)
                    c1 = min(ncols, j + R + 1)
                    if use_pix:
                        a = pp[0, c1] - pp[0, c0]
                        b = pp[1, c1] - pp[1, c0]
                        c = pp[2, c1] - pp[2, c0]
                    else:
                        hi = max(c1 - 1, c0)                 # pairs starting in [c0, c1-1)
                        num = (ph[0, hi] - ph[0, c0]) + (pv[0, c1] - pv[0, c0])
                        den = (ph[1, hi] - ph[1, c0]) + (pv[1, c1] - pv[1, c0])
                        if use_diag:
                            lo2 = min(c0 + 1, c1)            # pairs starting in [c0+1, c1)
                            num += (pd1[0, hi] - pd1[0, c0]) + (pd2[0, c1] - pd2[0, lo2])
                            den += (pd1[1, hi] - pd1[1, c0]) + (pd2[1, c1] - pd2[1, lo2])
                result[i, j] = _classify(kind, v, a, b, c, num, den,
                                         handle_missing, w2, tpe, thr)
    return result



@njit(cache=True)
def _lm_code(n_agr, n_for, n_dev):
    """103-class Landscape Mosaic code from the three class counts
    (Agriculture, Natural, Developed) in the window. The integer decision
    tree is unchanged from the original compute_LM loop."""
    n_valid = n_agr + n_for + n_dev

    # Integer Decision Tree Setup
    f10 = n_for * 10
    a10 = n_agr * 10
    d10 = n_dev * 10

    # Multipliers for p_x threshold checks
    v1 = n_valid;     v2 = n_valid * 2; v3 = n_valid * 3
    v4 = n_valid * 4; v5 = n_valid * 5; v6 = n_valid * 6
    v7 = n_valid * 7; v8 = n_valid * 8; v9 = n_valid * 9

    code = 0

    # --- Pure Corners ---
    if n_for == n_valid:   code = 170
    elif n_dev == n_valid: code = 190
    elif n_agr == n_valid: code = 180

    # --- Decision Tree Logic ---
    elif f10 < v1:
        if a10 < v1:    code = 191 if d10 >= v9 else 192
        elif a10 < v2:  code = 71  if d10 >= v8 else 72
        elif a10 < v3:  code = 73  if d10 >= v7 else 74
        elif a10 < v4:  code = 75  if d10 >= v6 else 131
        elif a10 < v5:  code = 132 if d10 >= v5 else 133
        elif a10 < v6:  code = 134 if d10 >= v4 else 135
        elif a10 < v7:  code = 45  if d10 >= v3 else 44
        elif a10 < v8:  code = 43  if d10 >= v2 else 42
        elif a10 < v9:  code = 41  if d10 >= v1 else 182
        else:           code = 181

    elif f10 < v2:
        if a10 < v1:    code = 61  if d10 >= v8 else 62
        elif a10 < v2:  code = 111 if d10 >= v7 else 112
        elif a10 < v3:  code = 114 if d10 >= v6 else 200
        elif a10 < v4:  code = 201 if d10 >= v5 else 202
        elif a10 < v5:  code = 203 if d10 >= v4 else 204
        elif a10 < v6:  code = 205 if d10 >= v3 else 206
        elif a10 < v7:  code = 103 if d10 >= v2 else 102
        elif a10 < v8:  code = 101 if d10 >= v1 else 52
        else:           code = 51

    elif f10 < v3:
        if a10 < v1:    code = 63  if d10 >= v7 else 64
        elif a10 < v2:  code = 113 if d10 >= v6 else 222
        elif a10 < v3:  code = 223 if d10 >= v5 else 224
        elif a10 < v4:  code = 225 if d10 >= v4 else 226
        elif a10 < v5:  code = 227 if d10 >= v3 else 228
        elif a10 < v6:  code = 207 if d10 >= v2 else 208
        elif a10 < v7:  code = 104 if d10 >= v1 else 54
        else:           code = 53

    elif f10 < v4:
        if a10 < v1:    code = 65  if d10 >= v6 else 155
        elif a10 < v2:  code = 221 if d10 >= v5 else 220
        elif a10 < v3:  code = 235 if d10 >= v4 else 234
        elif a10 < v4:  code = 236 if d10 >= v3 else 230
        elif a10 < v5:  code = 229 if d10 >= v2 else 210
        elif a10 < v6:  code = 209 if d10 >= v1 else 141
        else:           code = 55

    elif f10 < v5:
        if a10 < v1:    code = 154 if d10 >= v5 else 153
        elif a10 < v2:  code = 219 if d10 >= v4 else 218
        elif a10 < v3:  code = 233 if d10 >= v3 else 232
        elif a10 < v4:  code = 231 if d10 >= v2 else 212
        elif a10 < v5:  code = 211 if d10 >= v1 else 143
        else:           code = 142

    elif f10 < v6:
        if a10 < v1:    code = 152 if d10 >= v4 else 151
        elif a10 < v2:  code = 217 if d10 >= v3 else 216
        elif a10 < v3:  code = 215 if d10 >= v2 else 214
        elif a10 < v4:  code = 213 if d10 >= v1 else 145
        else:           code = 144

    elif f10 < v7:
        if a10 < v1:    code = 95  if d10 >= v3 else 94
        elif a10 < v2:  code = 124 if d10 >= v2 else 122
        elif a10 < v3:  code = 123 if d10 >= v1 else 84
        else:           code = 85

    elif f10 < v8:
        if a10 < v1:    code = 93  if d10 >= v2 else 92
        elif a10 < v2:  code = 121 if d10 >= v1 else 82
        else:           code = 83

    elif f10 < v9:
        if a10 < v1:    code = 91  if d10 >= v1 else 172
        else:           code = 81
    else:
        code = 171

    return np.uint8(code)


def compute_FAD(data, window_size, handle_missing):
    """
    Numba-optimized sliding window to calculate Foreground Area Density (FAD)
    within a moving window for each pixel, replicating SPATCON mapping rule 
    81 behaviour.
    
    Variables:
    ----------
    data           : 2D numpy array
                     The input raster pixels.
    window_size    : int 
                     Side length of the square window (must be odd, e.g., 3, 5, 7).
    handle_missing : int
                     Switch for denominator logic:
                     1 = 'Normalized' (denominator is count of non-missing pixels in window).
                     2 = 'Fixed' (denominator is always window_size * window_size).
                     
    Returns:
    --------
    result         : 2D numpy array (uint8)
                     - Values 0-100: Foreground proportion percentage.
                     - 101: Original background pixels.
                     - 102: Missing data or invalid calculations.
    """
    return _window_kernel(data, window_size, _K_FAD, handle_missing, 4, 0,
                          _n_blocks(data))


def compute_FAD_gray(data, window_size, handle_missing, for_threshold):
    """
    Numba-optimized sliding window to calculate Foreground Area Density (FAD)
    on a grayscale (continuous) input raster where pixel values represent
    foreground intensity from 0 to 100.

    Variables:
    ----------
    data           : 2D numpy array (uint8)
                     The input raster pixels:
                     0 = Non-foreground
                     1-100 = Foreground intensity (percentage)
                     255 = NoData (missing)
    window_size    : int
                     Side length of the square window (must be odd, >= 3).
    handle_missing : int
                     Switch for denominator logic:
                     1 = 'Normalized' (denominator is count of non-missing
                         pixels in window, scaled to 100).
                     2 = 'Fixed' (denominator is always window_size^2 * 100).
    for_threshold  : int
                     Foreground threshold (1-100). Pixels with values below
                     this threshold are treated as non-foreground (value 0)
                     during computation. If 1, all values 1-100 are valid.

    Returns:
    --------
    result         : 2D numpy array (uint8)
                     - Values 0-100: Average foreground density percentage.
                     - 101: Non-foreground pixels (input value 0 or below threshold).
                     - 102: Missing data (input value 255) or invalid.
    """
    return _window_kernel(data, window_size, _K_FAD_GRAY, handle_missing, 4,
                          for_threshold, _n_blocks(data))


def compute_FAC_gray(data, window_size, handle_missing, for_threshold, connectivity):
    """
    Numba-optimized sliding window to calculate Foreground Area Clustering (FAC)
    on a grayscale input raster. Only pairs where both pixels are >= for_threshold
    contribute to the numerator. The edge value is the average of the two pixel values.

    Variables:
    ----------
    data           : 2D numpy array (uint8)
                     0 = Non-foreground, 1-100 = Foreground intensity, 255 = NoData.
    window_size    : int
                     Side length of the square window (must be odd, >= 3).
    handle_missing : int
                     1 = 'Normalized' (denominator = non-missing edges × 100).
                     2 = 'Fixed' (denominator = total potential edges × 100).
    for_threshold  : int
                     Foreground threshold (1-100). Pixels below are treated as 0.
    connectivity   : int
                     4 = horizontal + vertical. 8 = adds diagonals.

    Returns:
    --------
    result         : 2D numpy array (uint8), values 0-100, 101, or 102.
    """
    return _window_kernel(data, window_size, _K_FAC_GRAY, handle_missing,
                          connectivity, for_threshold, _n_blocks(data))


def compute_FED_gray(data, window_size, handle_missing, for_threshold, connectivity):
    """
    Numba-optimized sliding window to calculate Foreground Edge Density (FED)
    on a grayscale input raster. The edge value is the average of the two
    adjacent pixel values (after thresholding).

    Scoring per pair (a, b are thresholded values):
        - Both >= threshold (FG-FG): edge = (a + b) / 2
        - One >= threshold (FG-nonFG): edge = (a + b) / 2  (one is 0)
        - Both < threshold (nonFG-nonFG): edge = 0

    Variables:
    ----------
    data           : 2D numpy array (uint8)
                     0 = Non-foreground, 1-100 = Foreground intensity, 255 = NoData.
    window_size    : int
                     Side length of the square window (must be odd, >= 3).
    handle_missing : int
                     1 = 'Normalized' (denominator = non-missing edges × 100).
                     2 = 'Fixed' (denominator = total potential edges × 100).
    for_threshold  : int
                     Foreground threshold (1-100). Pixels below are treated as 0.
    connectivity   : int
                     4 = horizontal + vertical. 8 = adds diagonals.

    Returns:
    --------
    result         : 2D numpy array (uint8), values 0-100, 101, or 102.
    """
    return _window_kernel(data, window_size, _K_FED_GRAY, handle_missing,
                          connectivity, for_threshold, _n_blocks(data))


def compute_FAC(data, window_size, handle_missing, connectivity):
    """
    Numba-optimized sliding window to calculate Foreground Area Clustering (FAC)
    within a moving window for each pixel, replicating SPATCON mapping rule
    76 behaviour.

    Variables:
    ----------
    data           : 2D numpy array
                     The input raster pixels (0=Missing, 1=Background, 2=Foreground).
    window_size    : int
                     Side length of the square window (must be odd).
    handle_missing : int
                     Switch for denominator logic:
                     1 = 'Normalized' (denominator is count of non-missing connected
                          pairs in window).
                     2 = 'Fixed' (denominator is always the total potential edges for
                          the chosen connectivity).
    connectivity   : int
                     4 = 4-connected (horizontal + vertical pairs).
                     8 = 8-connected (horizontal + vertical + diagonal pairs).

    Returns:
    --------
    result         : 2D numpy array (uint8)
                     - Values 0-100: Connectivity proportion percentage.
                     - 101: Original background pixels.
                     - 102: Missing data or invalid calculations.
    """
    return _window_kernel(data, window_size, _K_FAC, handle_missing,
                          connectivity, 0, _n_blocks(data))


def compute_FED(data, window_size, handle_missing, connectivity):
    """
    Numba-optimized sliding window to calculate Foreground Edge Density (FED)
    within a moving window for each pixel.

    Scores each connected pair within the window:
        - Foreground-Foreground : 1   (weight 2 in integer arithmetic)
        - Foreground-Background : 0.5 (weight 1 in integer arithmetic)
        - Background-Background : 0
        - Any pair with Missing : excluded (handle_missing=1) or counted in
                                  denominator as 0 (handle_missing=2)

    Variables:
    ----------
    data           : 2D numpy array (uint8)
                     Input raster (0=Missing, 1=Background, 2=Foreground,
                     3=BackgrSP3, 4=BackgrSP4).
    window_size    : int
                     Side length of the square window (must be odd).
    handle_missing : int
                     1 = 'Normalized': denominator is count of non-missing
                         connected pairs in window.
                     2 = 'Fixed': denominator is always the total potential
                         edges for the chosen connectivity.
    connectivity   : int
                     4 = horizontal + vertical pairs only.
                     8 = horizontal + vertical + diagonal pairs.

    Returns:
    --------
    result         : 2D numpy array (uint8)
                     - Values 0-100: FED proportion percentage.
                     - 101: Background pixels.
                     - 102: Missing data or invalid calculations.
                     - 105: BackgrSP3 pixels.
                     - 106: BackgrSP4 pixels.
    """
    return _window_kernel(data, window_size, _K_FED, handle_missing,
                          connectivity, 0, _n_blocks(data))


def compute_LM(data, window_size):
    """
    Classify Landscape Mosaics (LM) using tri-polar composition.
    
    This function uses a hierarchical decision tree to classify the landscape 
    into one of 103 possible mosaic classes based on the relative proportions 
    of three specific land-cover types.
    
    Parameters
    ----------
    data : ndarray (uint8)
        Input raster containing strictly three classes:
        1: Agriculture (AGR)
        2: Forest (FOR)
        3: Developed/Urban (DEV)
        0: Missing Data
    window_size : int
        Side length of the square observation window.
    
    Returns
    -------
    ndarray (uint8)
        Mosaic classification map. Each integer code corresponds to a specific 
        position on the tri-polar transition model.
    """
    return _window_kernel(data, window_size, _K_LM, 1, 4, 0, _n_blocks(data))


##########################
# ---- MSPA FUNCTIONS ----
##########################


def compute_spa(input_arr, s, n_classes):
    """
    Performs simplified pattern analysis SPA on a binary raster.
    
    Inputs
        input_arr: a 2D NumPy Array with a classification map 
                  0: NoData, 1: Background, 2: Foreground
        s (Float): the Edge Width parameter (e.g., 1.0, 3.0).
        n_classes (Int): The output number of classes (2, 3, 5, 6).

    Output: 2D NumPy array (uint8) with MSPA classes.
    """
    # Metrics
    threshold = s + 0.98
    size_param = (s + 0.98) / math.sqrt(2.0)
    bufsize = int(size_param * 1.5 + 0.5)
    frame = bufsize + 1

    # Pad image with 0
    original_padded = np.pad(input_arr, frame, mode='constant', constant_values=0)
    padded = original_padded.copy()

    # MASKS
    FG_mask = (padded == FOREGROUND)
    NoData_mask = (padded == MISSING_IN)

    # Distance tests below use the exact squared Euclidean distance:
    # dist <= threshold  <=>  dist^2 <= threshold^2 (computed in parallel on
    # whole image lines, see _within_sq_dist; no tiling).
    t2 = float(threshold) * float(threshold)

    # EXPAND DATA to MISSING
    near_fg = _within_sq_dist(FG_mask, t2)          # dist to FG <= threshold
    padded[NoData_mask & near_fg] = FOREGROUND
    padded[NoData_mask & ~near_fg] = BACKGROUND

    del near_fg, NoData_mask, FG_mask
    gc.collect()

    # CORE & BASIC MASKS
    FG = (padded == FOREGROUND)
    CORE = ~_within_sq_dist(~FG, t2)                # dist to non-FG > threshold

    # ISLET: Foreground patches (8-connected) that contain no Core pixel.
    # Equivalent to the binary morphological reconstruction of CORE under FG.
    ISLET = FG & ~_regions_with_seed(FG, CORE)

    # INTERNAL TERRITORY
    BG = (padded == BACKGROUND)
    BG_EXT = flood_fill(BG.astype(np.uint8), (0, 0), 2) == 2
    BG_INT = BG & ~BG_EXT
    del BG, BG_EXT

    CORE_FILL = binary_fill_holes(CORE)
    BG_INCORE = BG_INT & CORE_FILL
    del BG_INT

    BG_INCORE_DIL = _within_sq_dist(BG_INCORE, t2)
    INTERNAL = binary_fill_holes(BG_INCORE_DIL, structure=np.ones((3,3)))

    # INTERNAL CORE
    CORE_INT = CORE & INTERNAL
    CORE_INT_DIL = _within_sq_dist(CORE_INT, t2)
    del CORE_INT, INTERNAL

    # HALO
    HALO_MASK = _within_sq_dist(CORE, t2)

    # FINAL ASSEMBLY
    out_padded = _assemble_spa(
        padded, original_padded, CORE, ISLET, HALO_MASK,
        BG_INCORE_DIL, CORE_INT_DIL, BG_INCORE, n_classes
    )

    # CROP
    r0, r1 = frame, frame + input_arr.shape[0]
    c0, c1 = frame, frame + input_arr.shape[1]
    return out_padded[r0:r1, c0:c1]


@njit(parallel=True, cache=True)
def _within_sq_dist(target, t2):
    """
    Exact Euclidean distance test, computed in parallel.

    Returns a boolean array: True where the Euclidean distance from the
    pixel to the nearest True pixel of `target` is <= sqrt(t2) (target
    pixels themselves are at distance 0). Without any target pixel the
    result is all False.

    Separable exact squared EDT (Felzenszwalb & Huttenlocher 2012):
      pass 1: 1D distance along each full column (columns in parallel);
      pass 2: lower envelope of parabolas along each full row (rows in
              parallel), using the pass-1 values of all columns.
    Every line spans the whole image, so the nearest target pixel can be
    anywhere: there is no tiling and the result is identical to a serial
    computation. Squared distances are integers (int64), so the test
    d^2 <= t2 involves no floating-point rounding of the distance.
    """
    rows, cols = target.shape
    INF = np.int32(-1)                          # "no target in this column"
    g = np.empty((rows, cols), dtype=np.int32)

    # Pass 1: vertical distances, one full column per task
    for j in prange(cols):
        last = -1
        for i in range(rows):                   # top -> bottom
            if target[i, j]:
                last = i
            g[i, j] = (i - last) if last >= 0 else INF
        last = -1
        for i in range(rows - 1, -1, -1):       # bottom -> top
            if target[i, j]:
                last = i
            if last >= 0:
                d = last - i
                if g[i, j] == INF or d < g[i, j]:
                    g[i, j] = d

    out = np.zeros((rows, cols), dtype=np.bool_)

    # Pass 2: lower envelope of parabolas f(q) = g[i, q]^2, one full row per task
    for i in prange(rows):
        v = np.empty(cols, dtype=np.int64)      # parabola apex positions
        z = np.empty(cols + 1, dtype=np.float64)  # envelope boundaries
        k = -1
        for q in range(cols):
            gq = g[i, q]
            if gq == INF:
                continue
            fq = np.int64(gq) * gq + np.int64(q) * q
            if k < 0:
                k = 0
                v[0] = q
                z[0] = -np.inf
                z[1] = np.inf
                continue
            # Drop parabolas hidden by the new one (z[0] = -inf stops at k = 0)
            while True:
                p = v[k]
                gp = np.int64(g[i, p])
                fp = gp * gp + p * p
                s = (fq - fp) / (2.0 * (q - p))   # intersection with parabola p
                if s <= z[k]:
                    k -= 1
                else:
                    break
            k += 1
            v[k] = q
            z[k] = s
            z[k + 1] = np.inf
        if k < 0:
            continue                            # no target pixel in the whole image
        m = 0
        for j in range(cols):
            while z[m + 1] < j:
                m += 1
            p = v[m]
            gp = np.int64(g[i, p])
            dj = np.int64(j) - p
            out[i, j] = (dj * dj + gp * gp) <= t2
    return out


def _regions_with_seed(mask, seed):
    """
    Boolean array of the 8-connected regions of `mask` that contain at
    least one `seed` pixel (seed must lie within mask). Equivalent to the
    binary morphological reconstruction by dilation of `seed` under
    `mask` with a 3x3 neighbourhood, without its global sort.
    """
    labels, n = label(mask, structure=np.ones((3, 3), dtype=bool))
    return _keep_seeded_labels(labels, seed, n)


@njit(parallel=True, cache=True)
def _keep_seeded_labels(labels, seed, n):
    rows, cols = labels.shape
    keep = np.zeros(n + 1, dtype=np.bool_)
    for i in range(rows):                       # serial: race-free marking
        for j in range(cols):
            if seed[i, j]:
                keep[labels[i, j]] = True
    keep[0] = False
    out = np.empty((rows, cols), dtype=np.bool_)
    for i in prange(rows):
        for j in range(cols):
            out[i, j] = keep[labels[i, j]]
    return out


@njit(parallel=True, cache=True)
def _assemble_spa(healed_padded, original_padded,
                  core, islet, halo, bg_incore_dil,
                  core_int_dil, bg_incore, n_class):
    """
    It runs in parallel across all CPU cores to assign the final MSPA 
    class to every pixel.
    
    Inputs:
        - healed_padded: The foregorund with NoData gaps filled.
        - original_padded: The foreground used to restore NoData.
        - core, islet, halo, bg_incore_dil, core_int_dil, bg_incore: Boolean masks 
              representing the different morphological regions.
        - n_class: Used for conditional labeling.
    """
    # SPA constants
    OUT_BG_HOLE = 100
    OUT_MISSING = 129
    CORE = 17
    EDGE = 3
    ISLE = 9
    PERF = 5
    LINE = 1
    
    rows, cols = healed_padded.shape
    out = np.zeros((rows, cols), dtype=np.uint8)

    for i in prange(rows):
        for j in range(cols):
            # 1. TRUTH CHECK: If it was originally NoData, it stays 129
            if original_padded[i, j] == MISSING_IN:
                out[i, j] = OUT_MISSING
                continue

            # 2. BACKGROUND HOLE CHECK (Input 1 -> Output 100)
            if bg_incore[i, j] and n_class > 2:
                out[i, j] = OUT_BG_HOLE
                continue

            # 3. FOREGROUND CLASSIFICATION
            val = healed_padded[i, j]
            if val == 2:
                # --- BINARY MODE (n_class == 2) ---
                if n_class == 2:
                    if core[i, j] or halo[i, j]:
                        out[i, j] = CORE  # CORE and EDGE (including Perforation)
                    else:
                        out[i, j] = LINE   # ISLET and LINEAR
                
                # --- MULTI-CLASS MODE (n_class > 2) ---
                else:
                    if core[i, j]: 
                        out[i, j] = CORE # CORE
                    elif islet[i, j]:
                        out[i, j] = ISLE if n_class == 6 else LINE # ISLET or LINE
                    elif halo[i, j]:
                        # PERFORATION vs EDGE
                        if bg_incore_dil[i, j] and not core_int_dil[i, j]:
                            out[i, j] = PERF if n_class >= 5 else LINE # PERF or LINE
                        else:
                            out[i, j] = EDGE if n_class >= 5 else LINE # EDGE or LINE
                    else:
                        out[i, j] = LINE # LINEAR (Bridge/Loop/Branch)
    return out


###############################
# ---- LABELLING FUNCTIONS ----
###############################


def labelling_array(input_array, target_values):
    """
    Labels connected clusters of target pixel values using 8-connectivity
    and returns a frequency Counter of patch sizes. Used internally by
    acc() and rss() to identify and measure individual foreground patches.

    Parameters
    ----------
    input_array : np.ndarray
        2D input array to label.
    target_values : int or list of int
        Pixel value(s) to treat as foreground for labelling.
        Accepts a single integer or a list of integers.

    Returns
    -------
    tuple
        - labeled_array (np.ndarray): integer array where each connected
          patch of target_values is assigned a unique positive label ID.
          Background pixels (not in target_values) are labelled 0.
        - label_freq (Counter): mapping of patch label ID to pixel count,
          excluding background label 0.
    """
    # 1. Standardize targets to a NumPy array
    if isinstance(target_values, (int, np.integer)):
        targets = np.array([target_values], dtype=input_array.dtype)
    else:
        targets = np.array(target_values, dtype=input_array.dtype)

    # 2. Masking using Numba
    foreground_mask = _create_mask(input_array, targets)

    # 3. Labeling
    structure = generate_binary_structure(2, 2) # 8-connectivity
    labeled_array, _ = label(foreground_mask, structure=structure)

    # 4. Frequency counter
    label_freq = utils.get_pxl_freq(labeled_array)

    # 5. Clean up background
    if 0 in label_freq:
        del label_freq[0]

    return labeled_array, label_freq


@njit(cache=True)
def _create_mask(input_array, targets):
    """Numba-accelerated mask creation"""
    nrows, ncols = input_array.shape
    mask = np.zeros((nrows, ncols), dtype=np.bool_)
    for i in range(nrows):
        for j in range(ncols):
            val = input_array[i, j]
            # Check if val is in our targets
            for t in targets:
                if val == t:
                    mask[i, j] = True
                    break
    return mask


###############################
# ---- FOS CHANGE FUNCTION ----
###############################


@njit("uint8[:,:](uint8[:,:], uint8[:,:], int64[:,:], boolean)", parallel=True, cache=True)
def compute_fos_change(chunk_a, chunk_b, local_matrix, compute_stats):
    """
    Processes a matching spatial window chunk from two tracking rasters to 
    evaluate a 7-tier matrix overlay logic and concurrently update change statistics.

    Parameters
    ----------
    chunk_a : ndarray of shape (nrows, ncols), dtype=uint8
        A 2D window slice extracted from the initial time-step GeoTIFF (Time A).
    chunk_b : ndarray of shape (nrows, ncols), dtype=uint8
        A 2D window slice extracted from the subsequent time-step GeoTIFF (Time B).
    local_matrix : ndarray of shape (107, 107), dtype=int64
        A global or block-level confusion matrix accumulator tracking pixel-by-pixel 
        class transitions between Time A and Time B. Modified in-place.
    compute_stats : bool
        Flag indicating whether to calculate transition statistics. If False, the 
        conditional block updating `local_matrix` is bypassed completely.

    Returns
    -------
    out : ndarray of shape (nrows, ncols), dtype=uint8
        The calculated categorical change layer grid for the current window chunk, 
        where cell values represent explicit transition metrics (e.g., 250-254 for 
        exclusions/background dynamics, or localized delta values).
    """

    nrows, ncols = chunk_a.shape
    out = np.zeros((nrows, ncols), dtype=np.uint8)

    # Rows are split into chunks processed in parallel; each chunk counts its
    # transitions in its own matrix, summed into local_matrix at the end
    # (exact integer counts, independent of the number of chunks).
    nchunks = max(1, min(nrows, 64))
    part = np.zeros((nchunks, 107, 107), dtype=np.int64)

    for k in prange(nchunks):
        i_lo = (k * nrows) // nchunks
        i_hi = ((k + 1) * nrows) // nchunks
        for i in range(i_lo, i_hi):
            for j in range(ncols):
                a_val = chunk_a[i, j]
                b_val = chunk_b[i, j]

                # Track global statistics concurrently
                if compute_stats:
                    if a_val <= 106 and b_val <= 106:
                        part[k, a_val, b_val] += 1

                # Your exact 7-tier matrix overlay logic
                if a_val == 101 and b_val == 101:
                    out[i, j] = 252
                elif a_val == 101 and b_val <= 100:
                    out[i, j] = 250
                elif a_val <= 101 and b_val == 101:
                    out[i, j] = 251
                elif a_val <= 100 and b_val <= 100:
                    out[i, j] = 100 + a_val - b_val
                elif a_val == 102 or b_val == 102:
                    out[i, j] = 254
                elif a_val in (105, 106) or b_val in (105, 106):
                    out[i, j] = 253
                else:
                    out[i, j] = 102

    if compute_stats:
        for a in prange(107):
            for b in range(107):
                s = np.int64(0)
                for k in range(nchunks):
                    s += part[k, a, b]
                local_matrix[a, b] += s

    return out

