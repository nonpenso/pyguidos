import numpy as np
from numba import njit, prange


# Global Constants
MISSING_IN = 0
BACKGROUND = 1
FOREGROUND = 2
BACKGR_SP3 = 3
BACKGR_SP4 = 4


# ---- SPATCON FUNCTIONS ----

@njit("uint8[:,:](int16[:,:], int32, int32)", parallel=True, cache=True, fastmath=True)
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
    
# FOS constants
    OUT_BACKGROUND = 101
    OUT_MISSING = 102
    OUT_BACKGR_SP3 = 105
    OUT_BACKGR_SP4 = 106
    
    nrows, ncols = data.shape
    result = np.full((nrows, ncols), OUT_MISSING, dtype=np.uint8)
    radius = window_size // 2

    for i in prange(nrows):
        for j in range(ncols):
            pixel_val = data[i, j]
            
            # 1. Preserve Background (Standard and Special)
            if pixel_val == BACKGROUND:
                result[i, j] = OUT_BACKGROUND
                continue
            if pixel_val == BACKGR_SP3:
                result[i, j] = OUT_BACKGR_SP3
                continue
            if pixel_val == BACKGR_SP4:
                result[i, j] = OUT_BACKGR_SP4
                continue
            
            # 2. Process Foreground
            if pixel_val == FOREGROUND:
                fg_count = 0
                non_missing_count = 0
                
                for wi in range(i - radius, i + radius + 1):
                    for wj in range(j - radius, j + radius + 1):
                        if 0 <= wi < nrows and 0 <= wj < ncols:
                            val = data[wi, wj]
                            if val == FOREGROUND:
                                fg_count += 1
                            
                            # Treat SP3 and SP4 as valid background (non-missing)
                            if val != MISSING_IN:
                                non_missing_count += 1

                denom = non_missing_count if handle_missing == 1 else (window_size * window_size)
                
                if denom > 0:
                    #pf = int((fg_count / denom) * 100.0 + 0.5)
                    pf = (fg_count * 200 + denom) // (2 * denom)
                    result[i, j] = np.uint8(min(pf, 100))
                else:
                    result[i, j] = OUT_MISSING
                    
    return result



@njit("uint8[:,:](int16[:,:], int32, int32)", parallel=True, cache=True, fastmath=True)
def compute_FAC(data, window_size, handle_missing):
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
                     1 = 'Normalized' (denominator is count of non-missing 4-connected 
                          pairs in window).
                     2 = 'Fixed' (denominator is always 2 * window_size * (window_size - 1)).
                     
    Returns:
    --------
    result         : 2D numpy array (uint8)
                     - Values 0-100: Connectivity proportion percentage.
                     - 101: Original background pixels.
                     - 102: Missing data or invalid calculations.
    """
    
    # FOS constants
    OUT_BACKGROUND = 101
    OUT_MISSING = 102
    OUT_BACKGR_SP3 = 105
    OUT_BACKGR_SP4 = 106
    
    nrows, ncols = data.shape
    result = np.full((nrows, ncols), OUT_MISSING, dtype=np.uint8)
    radius = window_size // 2
    total_potential_edges = 2 * window_size * (window_size - 1)

    for i in prange(nrows):
        for j in range(ncols):
            pixel_val = data[i, j]

            # 1. Preserve Background (Standard and Special)
            if pixel_val == BACKGROUND:
                result[i, j] = OUT_BACKGROUND
                continue
            if pixel_val == BACKGR_SP3:
                result[i, j] = OUT_BACKGR_SP3
                continue
            if pixel_val == BACKGR_SP4:
                result[i, j] = OUT_BACKGR_SP4
                continue

            # 2. Process only Foreground pixels
            if pixel_val != FOREGROUND:
                continue

            fg_fg_edges = 0
            total_edges = 0

            r0, r1 = i - radius, i + radius + 1
            c0, c1 = j - radius, j + radius + 1

            # --- Horizontal Scan ---
            for wi in range(r0, r1):
                if wi < 0 or wi >= nrows: continue
                for wj in range(c0, c1 - 1):
                    if wj < 0 or wj + 1 >= ncols: continue 
                    
                    v1, v2 = data[wi, wj], data[wi, wj + 1]
                    if handle_missing == 1:
                        # Treat SP3/SP4 as valid for the connectivity denominator
                        if v1 != MISSING_IN and v2 != MISSING_IN:
                            total_edges += 1
                            if v1 == FOREGROUND and v2 == FOREGROUND:
                                fg_fg_edges += 1
                    else:
                        if v1 == FOREGROUND and v2 == FOREGROUND:
                            fg_fg_edges += 1

            # --- Vertical Scan ---
            for wi in range(r0, r1 - 1):
                for wj in range(c0, c1):
                    if wj < 0 or wj >= ncols: continue
                    if wi < 0 or wi + 1 >= nrows: continue

                    v1, v2 = data[wi, wj], data[wi + 1, wj]
                    if handle_missing == 1:
                        if v1 != MISSING_IN and v2 != MISSING_IN:
                            total_edges += 1
                            if v1 == FOREGROUND and v2 == FOREGROUND:
                                fg_fg_edges += 1
                    else:
                        if v1 == FOREGROUND and v2 == FOREGROUND:
                            fg_fg_edges += 1

            denom = total_edges if handle_missing == 1 else total_potential_edges
            if denom > 0:
                #pff = int((fg_fg_edges / denom) * 100.0 + 0.5)
                pff = (fg_fg_edges * 200 + denom) // (2 * denom)
                result[i, j] = np.uint8(min(pff, 100))
            else:
                result[i, j] = OUT_MISSING

    return result


@njit("uint8[:,:](int16[:,:], int32)", parallel=True, cache=True, fastmath=True)
def compute_LM(data, window_size):
    """
    Classify Landscape Mosaics (LM) using tri-polar composition.
    
    This function uses a hierarchical decision tree to classify the landscape 
    into one of 103 possible mosaic classes based on the relative proportions 
    of three specific land-cover types.
    
    Parameters
    ----------
    data : ndarray (int16)
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
    # Define internal class mapping based on your 1,2,3 requirement
    AGR_VAL = 1
    FOR_VAL = 2
    DEV_VAL = 3
    
    nrows, ncols = data.shape
    result = np.zeros((nrows, ncols), dtype=np.uint8)
    radius = window_size // 2

    for i in prange(nrows):
        for j in range(ncols):
            # Skip if center pixel is missing
            if data[i, j] == 0:
                continue

            # 1. Count pixels in window
            n_agr = 0
            n_for = 0
            n_dev = 0

            r0, r1 = i - radius, i + radius + 1
            c0, c1 = j - radius, j + radius + 1

            for wi in range(r0, r1):
                for wj in range(c0, c1):
                    # Boundary check
                    if 0 <= wi < nrows and 0 <= wj < ncols:
                        v = data[wi, wj]
                        if v == AGR_VAL:
                            n_agr += 1
                        elif v == FOR_VAL:
                            n_for += 1
                        elif v == DEV_VAL:
                            n_dev += 1
            
            n_valid = n_agr + n_for + n_dev
            if n_valid == 0:
                continue

            # 2. Integer Decision Tree Setup
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

            result[i, j] = np.uint8(code)

    return result