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
                    pf = int((fg_count / denom) * 100.0 + 0.5)
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
                pff = int((fg_fg_edges / denom) * 100.0 + 0.5)
                result[i, j] = np.uint8(min(pff, 100))
            else:
                result[i, j] = OUT_MISSING

    return result