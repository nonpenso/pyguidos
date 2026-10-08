"""
Pure-Python engine for Morphological Spatial Pattern Analysis (MSPA).

This module is a native-Python re-implementation of the MSPA algorithm
(``segmentBinaryPatterns``) originally written in C by Pierre Soille and
Peter Vogt as ``miallib`` and available on GitHub repository:
https://github.com/ec-jrc/jeolib-miallib
It reproduces the MSPA result and removes the need for a compiled C extension.

Public entry point
------------------
``segment_binary_patterns(imin, edge_width, graphfg=8, transition=1, internal=1)``
    imin : 2D uint8, 0=nodata, 1=background, 2=foreground.
    Returns the MSPA class-coded uint8 image (core 17, edge 3, perforation 5,
    islet 9, branch 1, bridge 33, loop 65, their transition 35/37/67/69 and
    internal +100 twins, core-opening 100, border-opening 220, missing 129).

Fidelity
--------
Validated bit-for-bit against the original miallib MSPA on the interior of
binary forest/non-forest maps across edge widths, 4- and 8-connectivity, and
transition/intext on/off. 

Algorithm map (miallib source -> function here)
-----------------------------------------------
  segmentBinaryPatterns (mspa.c) -> segment_binary_patterns
  getcore / getexternalboundary / getpatch / uc_fillhole / setedges (mspa.c)
  fm_preproc / fm_preproc2 + frame handling (mspa.c)
  binOIthin_FIFO anchored order-independent thinning (skel.c)
  ced constrained geodesic Euclidean distance (ced.c)
  getconnector2core / getcorridor (mspa.c)

Original MSPA algorithm (c) Pierre Soille and Peter Vogt (miallib, GPL v3);
see http://dx.doi.org/10.1016/j.patrec.2008.10.015
"""

from __future__ import annotations

import os
from concurrent.futures import ThreadPoolExecutor

import numpy as np

try:
    from numba import njit
    _HAVE_NUMBA = True
except Exception:  # pragma: no cover - numba is a hard dependency, but be safe
    _HAVE_NUMBA = False

    def njit(*args, **kwargs):
        """No-op fallback so the module imports even without numba.

        Supports both ``@njit`` and ``@njit(...)`` usage.
        """
        if len(args) == 1 and callable(args[0]) and not kwargs:
            return args[0]

        def _decorator(func):
            return func
        return _decorator


# --- simple_pixel 6x256 LUT (htab), transcribed verbatim from skel.c -------
# Row index = stype. For MSPA with graphfg=8 -> stype=0 (simple g=8, g'=4).
_HTAB = [
    # 0: simple g=8, g'=4
    [0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,
     1,1,0,0,1,1,1,1,0,1,0,1,0,1,1,0,
     1,1,0,0,0,1,0,1,1,1,1,1,0,1,1,0,
     0,1,0,0,0,1,0,1,0,1,0,1,0,1,1,0,
     1,0,1,0,1,1,1,1,0,0,1,1,0,1,1,0,
     0,0,0,0,1,1,1,1,0,0,0,1,0,1,1,0,
     0,0,0,0,0,1,0,1,0,0,1,1,0,1,1,0,
     0,0,0,0,0,1,0,1,0,0,0,1,0,1,1,0,
     1,0,1,0,0,0,1,1,1,1,1,1,0,1,1,0,
     0,0,0,0,0,0,1,1,0,1,0,1,0,1,1,0,
     0,0,0,0,0,0,0,1,1,1,1,1,0,1,1,0,
     0,0,0,0,0,0,0,1,0,1,0,1,0,1,1,0,
     0,0,1,0,0,0,1,1,0,0,1,1,0,1,1,0,
     0,0,0,0,0,0,1,1,0,0,0,1,0,1,1,0,
     0,0,0,0,0,0,0,1,0,0,1,1,0,1,1,0,
     0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,0],
    # 1: simple g=4, g'=8
    [0,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,
     0,1,1,0,1,1,0,0,1,0,0,0,0,0,0,0,
     0,1,1,0,1,0,0,0,1,1,0,0,0,0,0,0,
     0,1,1,0,1,1,0,0,1,1,0,0,0,1,0,0,
     0,1,1,0,1,0,1,0,1,0,0,0,0,0,0,0,
     0,1,1,0,1,1,1,1,1,0,0,0,0,0,0,0,
     0,1,1,0,1,0,1,0,1,1,0,0,0,0,0,0,
     0,1,1,0,1,1,1,1,1,1,0,0,0,1,0,1,
     0,1,1,0,1,0,0,0,1,0,1,0,0,0,0,0,
     0,1,1,0,1,1,0,0,1,0,1,0,0,0,0,0,
     0,1,1,0,1,0,0,0,1,1,1,1,0,0,0,0,
     0,1,1,0,1,1,0,0,1,1,1,1,0,1,0,1,
     0,1,1,0,1,0,1,0,1,0,1,0,0,0,1,0,
     0,1,1,0,1,1,1,1,1,0,1,0,0,0,1,1,
     0,1,1,0,1,0,1,0,1,1,1,1,0,0,1,1,
     0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0],
    # 2: simple g=4, g'=4 Ronse's {4,8}
    [0,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,
     0,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,
     0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
     0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,
     0,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,
     0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,0,
     0,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,
     0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,
     0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,
     0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,0,
     0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,0,
     0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,0],
    # 3: w-simple g=8, g'=4
    [1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,
     1,1,0,0,1,1,1,1,0,1,0,1,0,1,1,0,
     1,1,0,0,0,1,0,1,1,1,1,1,0,1,1,0,
     0,1,0,0,0,1,0,1,0,1,0,1,0,1,1,0,
     1,0,1,0,1,1,1,1,0,0,1,1,0,1,1,0,
     0,0,0,0,1,1,1,1,0,0,0,1,0,1,1,0,
     0,0,0,0,0,1,0,1,0,0,1,1,0,1,1,0,
     0,0,0,0,0,1,0,1,0,0,0,1,0,1,1,0,
     1,0,1,0,0,0,1,1,1,1,1,1,0,1,1,0,
     0,0,0,0,0,0,1,1,0,1,0,1,0,1,1,0,
     0,0,0,0,0,0,0,1,1,1,1,1,0,1,1,0,
     0,0,0,0,0,0,0,1,0,1,0,1,0,1,1,0,
     0,0,1,0,0,0,1,1,0,0,1,1,0,1,1,0,
     0,0,0,0,0,0,1,1,0,0,0,1,0,1,1,0,
     0,0,0,0,0,0,0,1,0,0,1,1,0,1,1,0,
     0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,0],
    # 4: w-simple g=4, g'=8
    [1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,
     1,1,1,0,1,1,0,0,1,0,0,0,0,0,0,0,
     1,1,1,0,1,0,0,0,1,1,0,0,0,0,0,0,
     1,1,1,0,1,1,0,0,1,1,0,0,0,1,0,0,
     1,1,1,0,1,0,1,0,1,0,0,0,0,0,0,0,
     1,1,1,0,1,1,1,1,1,0,0,0,0,0,0,0,
     1,1,1,0,1,0,1,0,1,1,0,0,0,0,0,0,
     1,1,1,0,1,1,1,1,1,1,0,0,0,1,0,1,
     1,1,1,0,1,0,0,0,1,0,1,0,0,0,0,0,
     1,1,1,0,1,1,0,0,1,0,1,0,0,0,0,0,
     1,1,1,0,1,0,0,0,1,1,1,1,0,0,0,0,
     1,1,1,0,1,1,0,0,1,1,1,1,0,1,0,1,
     1,1,1,0,1,0,1,0,1,0,1,0,0,0,1,0,
     1,1,1,0,1,1,1,1,1,0,1,0,0,0,1,1,
     1,1,1,0,1,0,1,0,1,1,1,1,0,0,1,1,
     1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0],
    # 5: w-simple g=4, g'=4 Ronse's {4,8}
    [1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,
     1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,
     1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
     0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,
     1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,
     0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,0,
     1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,
     0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,
     0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,
     0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,0,
     0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,0,
     0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,0],
]

# simple_pair2[stype][code] (diagonal neighbour), skel.c:395
_SIMPLE_PAIR2 = [
    [0,1,1,1],   # simple g=8, g'=4
    [1,1,1,0],   # simple g=4, g'=8
    [0,1,1,0],   # simple g=4, g'=4 Ronse's {4,8}
    [1,1,1,1],   # w-simple g=8, g'=4
    [1,1,1,0],   # w-simple g=4, g'=8
    [1,1,1,0],   # w-simple g=4, g'=4
]

# simple_pair4[stype][code] (non-diagonal neighbour), skel.c:398
_SIMPLE_PAIR4 = [
    [0,1,1,1,1,0,0,0,1,0,0,0,1,0,0,0],   # simple g=8, g'=4
    [0,0,0,1,0,0,0,1,0,0,0,1,1,1,1,0],   # simple g=4, g'=8
    [0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0],   # simple g=4, g'=4 Ronse's {4,8}
    [1,1,1,1,1,0,0,0,1,0,0,0,1,0,0,0],   # w-simple g=8, g'=4
    [1,0,0,1,0,0,0,1,0,0,0,1,1,1,1,0],   # w-simple g=4, g'=8
    [1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0],   # w-simple g=4, g'=4
]

# NumPy versions of the LUTs for the numba-compiled thinning path.
_HTAB_A = np.asarray(_HTAB, dtype=np.int8)
_SIMPLE_PAIR2_A = np.asarray(_SIMPLE_PAIR2, dtype=np.int8)
_SIMPLE_PAIR4_A = np.asarray(_SIMPLE_PAIR4, dtype=np.int8)


# ===========================================================================
# Numba-accelerated anchored thinning (binOIthin_FIFO)
#
# binOIthin_FIFO is by far the hottest MSPA stage (~65% of the runtime on
# realistic data). The whole skel.c helper chain is re-expressed below on flat
# int32 buffers with the LUTs passed as NumPy arrays, so numba can compile it
# to native code. The logic mirrors the pure-Python helpers verbatim; the only
# differences are mechanical (array LUTs, preallocated integer queues instead
# of Python lists). If numba is unavailable the @njit decorator degrades to a
# no-op and these run as ordinary (slower) Python, preserving behaviour.
# ===========================================================================

@njit(cache=True)
def _nb_simple_pixel(p, stype, pos, nx, htab):
    if p[pos] <= 0:
        return 0
    code = (1 if p[pos - 1] == 1 else 0)
    code |= (1 if p[pos + 1] == 1 else 0) << 1
    code |= (1 if p[pos - nx] == 1 else 0) << 2
    code |= (1 if p[pos + nx] == 1 else 0) << 3
    code |= (1 if p[pos - nx - 1] == 1 else 0) << 4
    code |= (1 if p[pos + nx - 1] == 1 else 0) << 5
    code |= (1 if p[pos - nx + 1] == 1 else 0) << 6
    code |= (1 if p[pos + nx + 1] == 1 else 0) << 7
    return htab[stype, code]


@njit(cache=True)
def _nb_test_anchor(pa, atype, pos):
    if atype == 0:
        return True
    return pa[pos] == 0


@njit(cache=True)
def _nb_simple_pair(p, stype, pos, ngbpos, sh, sp2, sp4):
    """Returns (result, conftype, a, b, c, d)."""
    if ngbpos < 4:
        if ngbpos == 0:
            a = sh[2]; b = sh[4]; c = sh[3]; d = sh[5]
        elif ngbpos == 1:
            a = sh[2]; b = sh[6]; c = sh[3]; d = sh[7]
        elif ngbpos == 2:
            a = sh[0]; b = sh[4]; c = sh[1]; d = sh[6]
        else:
            a = sh[0]; b = sh[5]; c = sh[1]; d = sh[7]
        code = (1 if p[pos + d] >= p[pos] else 0)
        code |= (1 if p[pos + c] >= p[pos] else 0) << 1
        code |= (1 if p[pos + b] >= p[pos] else 0) << 2
        code |= (1 if p[pos + a] >= p[pos] else 0) << 3
        conftype = code + 5
        return sp4[stype, code], conftype, a, b, c, d
    else:
        if ngbpos == 4:
            a = sh[0]; b = sh[2]
        elif ngbpos == 5:
            a = sh[3]; b = sh[0]
        elif ngbpos == 6:
            a = sh[2]; b = sh[1]
        else:
            a = sh[1]; b = sh[3]
        code = (1 if p[pos + b] >= p[pos] else 0)
        code |= (1 if p[pos + a] >= p[pos] else 0) << 1
        conftype = code + 1
        return sp2[stype, code], conftype, a, b, 0, 0


@njit(cache=True)
def _nb_num_sngb(p, pos, sh, ngbnum):
    c = 0
    for i in range(ngbnum):
        if p[pos + sh[i]] >= p[pos]:
            c += 1
    return c


@njit(cache=True)
def _nb_testsimple(p, pa, atype, stype, pos, ngbp, sh, nx, htab):
    if (_nb_test_anchor(pa, atype, pos + ngbp)
            and _nb_simple_pixel(p, stype, pos + ngbp, nx, htab) != 0
            and p[pos] == p[pos + ngbp]):
        return 1
    return 0


@njit(cache=True)
def _nb_indep_simple(p, pa, stype, atype, pos, sh, nx, htab, sp2, sp4):
    allindep = 1
    ngbnum = 0
    aa = 0
    bb = 0
    ct = 0
    conftype = 0
    i = 0
    while i < 8 and allindep == 1:
        if _nb_testsimple(p, pa, atype, stype, pos, sh[i], sh, nx, htab) != 0:
            ngbnum += 1
            res, conftype, a, b, c, d = _nb_simple_pair(p, stype, pos, i, sh, sp2, sp4)
            if res == 0:
                allindep = 0
                aa = a; bb = b; ct = conftype
        i += 1
    if allindep == 1:
        ct = conftype

    if (allindep == 0) and (stype == 1 or stype == 2 or stype == 4 or stype == 5):
        if ct == 4:
            if (_nb_testsimple(p, pa, atype, stype, pos, aa, sh, nx, htab) != 0
                    and _nb_testsimple(p, pa, atype, stype, pos, bb, sh, nx, htab) != 0):
                zerocount = 0
                for j in range(4):
                    if p[pos + sh[j]] < p[pos]:
                        zerocount += 1
                if zerocount == 2:
                    allindep = 1
                    if stype < 3:
                        founddiff = 0
                        j = 0
                        while j < 4 and founddiff == 0:
                            if _nb_testsimple(p, pa, atype, stype, pos, sh[j], sh, nx, htab) != 0:
                                if _nb_num_sngb(p, pos + sh[j], sh, 4) != 2:
                                    founddiff = 1
                            j += 1
                        if founddiff == 0:
                            allindep = 0
        return allindep

    if (allindep == 1) and (stype < 3):
        if (((ngbnum == 2) and (ct == 2 or ct == 3 or ct == 6 or ct == 7 or ct == 9 or ct == 13))
                or ((ngbnum == 3) and (ct == 4 or ct == 8 or ct == 17))):
            founddiff = 0
            j = 0
            while j < 8 and founddiff == 0:
                if _nb_testsimple(p, pa, atype, stype, pos, sh[j], sh, nx, htab) != 0:
                    if _nb_num_sngb(p, pos + sh[j], sh, 8) != ngbnum:
                        founddiff = 1
                j += 1
            if founddiff == 0:
                allindep = 0
    return allindep


@njit(cache=True)
def _nb_binOIthin(p, pa, atype, stype, sh, nx, start, end, htab, sp2, sp4):
    """Core order-independent thinning on a flat framed buffer p (modified in
    place). Preallocated integer queues replace the Python-list FIFOs."""
    npix = p.shape[0]
    psimple = np.zeros(npix, dtype=np.uint8)
    qcurrent = np.empty(npix, dtype=np.int64)
    qnext = np.empty(npix, dtype=np.int64)
    q2 = np.empty(npix, dtype=np.int64)
    nc = 0

    # seed: all independent simple non-anchor points
    for i in range(start, end):
        if (_nb_simple_pixel(p, stype, i, nx, htab) != 0
                and _nb_test_anchor(pa, atype, i)
                and _nb_indep_simple(p, pa, stype, atype, i, sh, nx, htab, sp2, sp4) != 0):
            psimple[i] = 1
            qcurrent[nc] = i
            nc += 1
        else:
            psimple[i] = 0

    deleted = 1
    while deleted:
        deleted = 0
        n2 = 0
        # delete the whole current independent set
        for a in range(nc):
            i = qcurrent[a]
            deleted = 1
            psimple[i] = 0
            p[i] = 0
            q2[n2] = i
            n2 += 1
        # re-examine neighbours of deleted pixels
        nn = 0
        for a in range(n2):
            i = q2[a]
            for j in range(8):
                k = i + sh[j]
                if (_nb_test_anchor(pa, atype, k)
                        and _nb_simple_pixel(p, stype, k, nx, htab) != 0
                        and psimple[k] == 0
                        and _nb_indep_simple(p, pa, stype, atype, k, sh, nx, htab, sp2, sp4) != 0):
                    psimple[k] = 1
                    qnext[nn] = k
                    nn += 1
        # swap current<-next
        for a in range(nn):
            qcurrent[a] = qnext[a]
        nc = nn


def _getngbshift8(nx: int):
    """Neighbour offsets, exactly as skel.c getngbshift8 (4-conn then 8-conn)."""
    return [
        -1,       # 0 W
        1,        # 1 E
        -nx,      # 2 N
        nx,       # 3 S
        -nx - 1,  # 4 NW
        nx - 1,   # 5 SW
        -nx + 1,  # 6 NE
        nx + 1,   # 7 SE
        0,        # 8 centre
    ]


def _simple_pixel(p, stype, pos, nx):
    """Port of simple_pixel (skel.c:69). p is a flat uint8 buffer."""
    if p[pos] <= 0:
        return 0
    code = (1 if p[pos - 1] == 1 else 0)
    code |= (1 if p[pos + 1] == 1 else 0) << 1
    code |= (1 if p[pos - nx] == 1 else 0) << 2
    code |= (1 if p[pos + nx] == 1 else 0) << 3
    code |= (1 if p[pos - nx - 1] == 1 else 0) << 4
    code |= (1 if p[pos + nx - 1] == 1 else 0) << 5
    code |= (1 if p[pos - nx + 1] == 1 else 0) << 6
    code |= (1 if p[pos + nx + 1] == 1 else 0) << 7
    return _HTAB[stype][code]


def _test_anchor(panchor, atype, pos):
    """Port of test_anchor (skel.c:190): anchor pixels are never simple."""
    return (atype == 0) or (panchor[pos] == 0)


def _simple_pair(p, stype, pos, ngbpos, sh):
    """
    Port of simple_pair (skel.c:369). Returns (result, conftype, a, b, c, d).
    """
    if ngbpos < 4:  # non-diagonal neighbour
        if ngbpos == 0:
            a, b, c, d = sh[2], sh[4], sh[3], sh[5]
        elif ngbpos == 1:
            a, b, c, d = sh[2], sh[6], sh[3], sh[7]
        elif ngbpos == 2:
            a, b, c, d = sh[0], sh[4], sh[1], sh[6]
        else:  # 3
            a, b, c, d = sh[0], sh[5], sh[1], sh[7]
        code = (1 if p[pos + d] >= p[pos] else 0)
        code |= (1 if p[pos + c] >= p[pos] else 0) << 1
        code |= (1 if p[pos + b] >= p[pos] else 0) << 2
        code |= (1 if p[pos + a] >= p[pos] else 0) << 3
        conftype = code + 5
        return _SIMPLE_PAIR4[stype][code], conftype, a, b, c, d
    else:  # diagonal neighbour
        if ngbpos == 4:
            a, b = sh[0], sh[2]
        elif ngbpos == 5:
            a, b = sh[3], sh[0]
        elif ngbpos == 6:
            a, b = sh[2], sh[1]
        else:  # 7
            a, b = sh[1], sh[3]
        code = (1 if p[pos + b] >= p[pos] else 0)
        code |= (1 if p[pos + a] >= p[pos] else 0) << 1
        conftype = code + 1
        return _SIMPLE_PAIR2[stype][code], conftype, a, b, 0, 0


def _num_sngb(p, pos, sh, ngbnum):
    """Port of num_sngb (skel.c:425)."""
    return sum(1 for i in range(ngbnum) if p[pos + sh[i]] >= p[pos])


def _testsimple(p, panchor, atype, stype, pos, ngbp, sh, nx):
    """Port of testsimple (skel.c:436)."""
    if (_test_anchor(panchor, atype, pos + ngbp)
            and _simple_pixel(p, stype, pos + ngbp, nx)
            and p[pos] == p[pos + ngbp]):
        return 1
    return 0


def _indep_simple(p, panchor, stype, atype, pos, sh, nx):
    """Port of indep_simple (skel.c:444)."""
    allindep = 1
    ngbnum = 0
    aa = bb = 0
    ct = 0
    conftype = 0
    i = 0
    while i < 8 and allindep:
        if _testsimple(p, panchor, atype, stype, pos, sh[i], sh, nx):
            ngbnum += 1
            res, conftype, a, b, c, d = _simple_pair(p, stype, pos, i, sh)
            if res == 0:
                allindep = 0
                aa, bb, ct = a, b, conftype
        i += 1
    if allindep:
        ct = conftype

    if (allindep == 0) and (stype in (1, 2, 4, 5)):
        if ct == 4:  # check edge configuration
            if (_testsimple(p, panchor, atype, stype, pos, aa, sh, nx)
                    and _testsimple(p, panchor, atype, stype, pos, bb, sh, nx)):
                zerocount = sum(1 for i in range(4) if p[pos + sh[i]] < p[pos])
                if zerocount == 2:
                    allindep = 1
                    if stype < 3:
                        founddiff = 0
                        i = 0
                        while i < 4 and founddiff == 0:
                            if _testsimple(p, panchor, atype, stype, pos, sh[i], sh, nx):
                                if _num_sngb(p, pos + sh[i], sh, 4) != 2:
                                    founddiff = 1
                            i += 1
                        if founddiff == 0:
                            allindep = 0
        return allindep

    if (allindep == 1) and (stype < 3):
        # triple / quadruple of isolated simple pixels check
        if (((ngbnum == 2) and (ct in (2, 3, 6, 7, 9, 13)))
                or ((ngbnum == 3) and (ct in (4, 8, 17)))):
            founddiff = 0
            i = 0
            while i < 8 and founddiff == 0:
                if _testsimple(p, panchor, atype, stype, pos, sh[i], sh, nx):
                    if _num_sngb(p, pos + sh[i], sh, 8) != ngbnum:
                        founddiff = 1
                i += 1
            if founddiff == 0:
                allindep = 0
    return allindep


def binOIthin_FIFO(img: np.ndarray, stype: int, anchor: np.ndarray | None):
    """
    Order-independent anchored thinning, port of binOIthin_FIFO (skel.c:587).

    Parameters
    ----------
    img : 2D uint8 (values 0/1)
        Image to thin in place (a copy is made).
    stype : int
        Simpleness type (MSPA uses 0 for 8-connected foreground).
    anchor : 2D uint8 or None
        Anchor mask; anchor pixels are never deleted (atype=1 if given).

    Returns
    -------
    2D uint8 thinned image (same shape as input).
    """
    atype = 1 if anchor is not None else 0

    ny, nx0 = img.shape
    # frame of 2 px like generic_framebox(...,box={2,2,2,2,0,0})
    F = 2
    nx = nx0 + 2 * F
    nyf = ny + 2 * F
    buf = np.zeros((nyf, nx), dtype=np.int64)
    buf[F:F + ny, F:F + nx0] = (img > 0).astype(np.int64)
    p = buf.ravel()

    if atype:
        abuf = np.zeros((nyf, nx), dtype=np.int64)
        abuf[F:F + ny, F:F + nx0] = (anchor > 0).astype(np.int64)
        pa = abuf.ravel()
    else:
        # numba needs a concrete array (not None); an all-zero anchor with
        # atype=0 is never consulted (_nb_test_anchor returns True for atype=0).
        pa = np.zeros(1, dtype=np.int64)

    sh = np.asarray(_getngbshift8(nx), dtype=np.int64)
    npix = p.size
    start = 2 * nx + 2
    end = npix - 2 * nx - 2

    # The hot inner algorithm (seeding + iterative independent-set deletion)
    # runs in the numba-compiled core; p is modified in place.
    _nb_binOIthin(p, pa, atype, stype, sh, nx, start, end,
                  _HTAB_A, _SIMPLE_PAIR2_A, _SIMPLE_PAIR4_A)

    out = buf.reshape(nyf, nx)[F:F + ny, F:F + nx0]
    return (out > 0).astype(np.uint8)


# ===========================================================================
# Base morphology helpers (getcore, getexternalboundary, getpatch, setedges)
# ===========================================================================
from scipy import ndimage as _ndi

EDU = np.sqrt(2.0)


def _struct(graph: int) -> np.ndarray:
    """3x3 connectivity element: graph==8 -> full, graph==4 -> plus."""
    if graph == 8:
        return np.ones((3, 3), dtype=bool)
    s = np.zeros((3, 3), dtype=bool)
    s[1, :] = True
    s[:, 1] = True
    return s


def _edt(mask: np.ndarray) -> np.ndarray:
    """
    Exact Euclidean distance transform of a binary mask: for each nonzero
    pixel, distance to the nearest zero pixel. Equivalent to
    sqrt(sqedt(mask)) in miallib (uc_sqedt is the error-free EDT).
    """
    return _ndi.distance_transform_edt(mask.astype(bool))


def getcore(im: np.ndarray, size: float, edu: float = EDU) -> np.ndarray:
    """
    getcore (mspa.c:208): foreground pixels whose distance to the foreground
    boundary is STRICTLY greater than size*edu.
    f_threshstrict(dist, size*edu, 65535) -> size*edu < dist < 65535.
    """
    dist = _edt(im.astype(bool))               # sqrt(sqedt(im))
    core = (dist > (size * edu)) & (dist < 65535.0)
    return core.astype(np.uint8)


def getexternalboundary(im: np.ndarray, size: float, edu: float = EDU) -> np.ndarray:
    """
    getexternalboundary (mspa.c:50): background pixels within distance size*edu
    of the foreground.
      nim = negation(im)             # background mask
      dist = sqrt(sqedt(nim))        # distance to nearest background pixel
      f_thresh(dist, 0, size*edu)    # 0 <= dist <= size*edu  (inclusive)
      AND nim                        # keep only background pixels
    """
    nim = (im == 0)                            # negation: background = 1
    dist = _edt(nim)                           # distance to nearest background
    band = (dist >= 0.0) & (dist <= (size * edu))
    return (band & nim).astype(np.uint8)


def rdil(mark: np.ndarray, mask: np.ndarray, graph: int) -> np.ndarray:
    """
    Reconstruction by dilation (rdil, recons.c): grow mark under mask.
    Binary morphological reconstruction == ndi.binary_propagation.
    """
    out = _ndi.binary_propagation(mark.astype(bool), mask=mask.astype(bool),
                                  structure=_struct(graph))
    return out.astype(np.uint8)


def rero(mark: np.ndarray, mask: np.ndarray, graph: int) -> np.ndarray:
    """
    Reconstruction by erosion (rero, recons.c): the dual of rdil.
    rero(mark, mask) = NOT( rdil(NOT mark, NOT mask) ).
    """
    nmark = (mark == 0)
    nmask = (mask == 0)
    out = _ndi.binary_propagation(nmark, mask=nmask, structure=_struct(graph))
    return (~out).astype(np.uint8)


def getpatch(im: np.ndarray, size: float, graphfg: int, edu: float = EDU,
             core: np.ndarray | None = None) -> np.ndarray:
    """
    getpatch / islet (mspa.c:224):
      core = getcore(im, size)
      core = rdil(core, im, graphfg)   # reconstruct the core-bearing patches
      return im - core                 # islets = foreground w/o any core
    (SUBSWAP_op computes im - core.)

    ``core`` may be supplied if ``getcore(im, size, edu)`` was already computed
    elsewhere, to avoid a redundant (expensive) distance transform. When given
    it must equal ``getcore(im, size, edu)`` exactly; the result is identical.
    """
    if core is None:
        core = getcore(im, size, edu)
    core = rdil(core, im, graphfg)
    islet = (im.astype(bool) & ~core.astype(bool))
    return islet.astype(np.uint8)


def uc_fillhole(im: np.ndarray, graph: int) -> np.ndarray:
    """
    uc_fillhole (mspa.c:239): morphological hole filling.
      marker = blank(255); frame set to 0; marker = sup(marker, im)
      return rero(marker, im, graph)
    Equivalent to ndi.binary_fill_holes with the given connectivity.
    """
    return _ndi.binary_fill_holes(im.astype(bool),
                                  structure=_struct(graph)).astype(np.uint8)


def setedges(im: np.ndarray, size: float, graphfg: int, graphbg: int,
             edu: float = EDU, core: np.ndarray | None = None):
    """
    setedges (mspa.c:256): returns (edges, perforation).

    Mirrors the iterative peeling in the C code:
      core     = getcore(im, size)
      core1    = copy(core)
      corefill = fillhole(core, graphbg)
      loecher  = corefill - core
      edges    = 0
      loop:
        if volume(corefill)==0: break
        crt_edges = getexternalboundary(corefill, size)
        edges |= crt_edges
        i0 = fillhole(loecher, graphfg)
        corefill -= i0
        core -= corefill
        corefill = fillhole(core, graphbg)
        loecher = corefill - core
      outer = getexternalboundary(core1, size)
      edges &= negation(core1)          # external boundary only
      perforation = outer - edges
    """
    im_b = im.astype(bool)
    if core is None:
        core = getcore(im, size, edu).astype(bool)
    else:
        core = core.astype(bool)
    core1 = core.copy()

    corefill = uc_fillhole(core, graphbg).astype(bool)
    loecher = corefill & ~core
    edges = np.zeros_like(im_b)

    while True:
        if not corefill.any():                 # volume(corefill) == 0
            break
        crt_edges = getexternalboundary(corefill.astype(np.uint8), size, edu).astype(bool)
        edges |= crt_edges
        i0 = uc_fillhole(loecher.astype(np.uint8), graphfg).astype(bool)
        corefill = corefill & ~i0              # corefill -= i0
        core = core & ~corefill                # core -= corefill
        corefill = uc_fillhole(core.astype(np.uint8), graphbg).astype(bool)
        loecher = corefill & ~core

    outer = getexternalboundary(core1.astype(np.uint8), size, edu).astype(bool)
    edges &= ~core1                            # bitwise AND negation(core1)
    perforation = outer & ~edges               # outer - edges

    return edges.astype(np.uint8), perforation.astype(np.uint8)


# ===========================================================================
# Preprocessing (fm_preproc, fm_preproc2), frame handling, sizing
# ===========================================================================


def compute_size(edge_width: float) -> float:
    """size = (edge_width + 0.98) / sqrt(2)   (mspa.c:454)."""
    return (edge_width + 0.98) / np.sqrt(2.0)


def compute_bufsize(size: float) -> int:
    """bufsize = (int)((size * 1.5) + 0.5)   (mspa.c:479)."""
    return int((size * 1.5) + 0.5)


def addframebox(im: np.ndarray, w: int, value: int = 0) -> np.ndarray:
    """generic_addframebox with equal width w on all four sides, filled value."""
    return np.pad(im, ((w, w), (w, w)), mode="constant", constant_values=value)


def subframebox(im: np.ndarray, w: int) -> np.ndarray:
    """Inverse of addframebox: strip a frame of width w on all sides."""
    return im[w:im.shape[0] - w, w:im.shape[1] - w]


def fm_preproc(fm: np.ndarray, size: float, edu: float = EDU) -> np.ndarray:
    """
    fm_preproc (mspa.c:69). Input fm: 0=nodata, 1=background, 2=foreground.

      da  = (fm >= 1)                       # data mask (bg or fg)
      dda = getexternalboundary(da, size)   # data external boundary (into nodata)
      fa  = (fm >= 2)                       # foreground mask
      dfa = getexternalboundary(fa, size)   # fg external boundary
      dfa = dfa OR fa
      dfa = dfa AND dda      (INF_op = min, binary -> and)
      dfa = dfa OR  fa       (SUP_op = max, binary -> or)
      return dfa
    """
    da = (fm >= 1).astype(np.uint8)
    dda = getexternalboundary(da, size, edu).astype(bool)
    fa = (fm >= 2).astype(bool)
    dfa = getexternalboundary(fa.astype(np.uint8), size, edu).astype(bool)
    dfa = dfa | fa
    dfa = dfa & dda
    dfa = dfa | fa
    return dfa.astype(np.uint8)


def fm_preproc2(im: np.ndarray, size: int) -> np.ndarray:
    """
    fm_preproc2 (mspa.c:96): add a frame of width ``size`` and propagate each
    image border and corners outward into the frame to mitigate border effects.
    Each border row/column is replicated outward into its side strip and the 
    four corner pixels into each corner quadrant.
    """
    # ny, nx = im.shape
    # out = np.zeros((ny + 2 * size, nx + 2 * size), dtype=im.dtype)
    # # original image in the centre
    # out[size:size + ny, size:size + nx] = im
    # # left/right strips: replicate the border column across `size` cols,
    # # over the original row span only (rows [size, size+ny)).
    # left_col = im[:, 0][:, None]            # (ny,1)
    # right_col = im[:, nx - 1][:, None]
    # out[size:size + ny, 0:size] = np.repeat(left_col, size, axis=1)
    # out[size:size + ny, size + nx:] = np.repeat(right_col, size, axis=1)
    # # top/bottom strips: replicate the border row across `size` rows,
    # # over the original column span only (cols [size, size+nx)).
    # top_row = im[0, :][None, :]             # (1,nx)
    # bot_row = im[ny - 1, :][None, :]
    # out[0:size, size:size + nx] = np.repeat(top_row, size, axis=0)
    # out[size + ny:, size:size + nx] = np.repeat(bot_row, size, axis=0)
    # # corners remain 0
    # return out
    return np.pad(im,pad_width=size, mode="edge")

# ===========================================================================
# Connector generation: getconnector2core + geodesic external boundary (ced)
# ===========================================================================


def _inmaskp(pm, ncol, offset_r, x2, y2):
    """
    Port of inmaskp (ced.c:38): Bresenham line-of-sight from the reference
    pixel at flat index ``offset_r`` stepping toward relative (x2, y2).
    Returns False if any pixel along the straight line is outside the mask
    (pm == 0), True otherwise. pm is the flat mask buffer.
    """
    x2 = int(x2); y2 = int(y2)
    dx = abs(x2); dy = abs(y2)
    s1 = (1 if x2 > 0 else (-1 if x2 < 0 else 0))
    s2 = (1 if y2 > 0 else (-1 if y2 < 0 else 0))
    interchange = True
    if dy > dx:
        dx, dy = dy, dx
    else:
        interchange = False
    d = 2 * dy - dx
    incr1 = 2 * dy
    incr2 = 2 * (dy - dx)
    x = 0; y = 0
    der = dx
    while abs(y if interchange else x) < der:
        if interchange:
            y += s2
        else:
            x += s1
        if d < 0:
            d += incr1
        else:
            d += incr2
            if interchange:
                x += s1
            else:
                y += s2
        if pm[offset_r + x + y * ncol] == 0:
            return False
    return True


_SHORT_MAX = 32767


def ced(ref: np.ndarray, mask: np.ndarray) -> np.ndarray:
    """
    Constrained geodesic Euclidean distance (port of ced, ced.c:105).

    For every mask pixel, the Euclidean distance to the nearest reference
    pixel, measured along straight lines that stay inside the mask (geodesic
    constraint via the Bresenham line-of-sight test ``inmaskp``), with a +1
    geodesic fallback when the straight line is blocked.

    ref, mask: 2D uint8 binary (reference pixels / geodesic mask). Returns a
    float distance image (0 where not reached / ref). graph is 4 as in ced.c.
    """
    INQUEUE = 2
    FICT = -1
    ny0, nx0 = ref.shape
    F = 2
    nx = nx0 + 2 * F
    ny = ny0 + 2 * F
    npix = nx * ny

    pr = np.zeros(npix, dtype=np.int64)
    pm = np.zeros(npix, dtype=np.int64)
    pr.reshape(ny, nx)[F:F + ny0, F:F + nx0] = (ref > 0).astype(np.int64)
    pm.reshape(ny, nx)[F:F + ny0, F:F + nx0] = (mask > 0).astype(np.int64)

    # 4-connectivity: ced.c uses set_seq_shift (shft) and SEPARATE (dx,dy)
    # arrays whose directions are the REVERSE of shft (shft[k] points to the
    # neighbour ofsk; (dx[k],dy[k]) points from ofsk back toward ofs). This
    # pairing is what makes px[ofs]=px[ofsk]+dx[k] accumulate the reference->
    # pixel vector and org=ofsk-px land on the reference.
    #   shft:  N=-nx, W=-1, S=+nx, E=+1   (set_seq_shift, graph=4)
    #   dx/dy: (0,1)=S, (1,0)=E, (0,-1)=N, (-1,0)=W   (ced.c)
    shft = [-nx, -1, nx, 1]
    dxl = [0, 1, 0, -1]
    dyl = [1, 0, -1, 0]

    # Explicit FIFO4 replica: a Python list acting as the circular buffer, with
    # a remove index (qpr) and a look index (qpl). fifo4_add appends; look scans
    # [qpr, len) non-destructively; remove pops at qpr. This matches fifo.c
    # ordering exactly (which the ordered propagation depends on).
    q = []

    # init: reference-and-mask pixels become seeds (pm=3); their non-ref mask
    # neighbours enter the queue.
    for i in range(npix):
        if pr[i] and pm[i]:
            pm[i] = 3
            for k in range(4):
                ofsk = i + shft[k]
                if 0 <= ofsk < npix and pm[ofsk] == 1 and pr[ofsk] == 0:
                    q.append(ofsk)
                    pm[ofsk] = INQUEUE

    px = np.full(npix, _SHORT_MAX, dtype=np.int64)
    py = np.full(npix, _SHORT_MAX, dtype=np.int64)
    pb = np.zeros(npix, dtype=np.float64)
    px[pm == 3] = 0
    py[pm == 3] = 0

    FLOAT_MAX = 1e30
    qpr = 0  # remove/front index into q
    # emulate: while (!empty) { add FICT; lookreset; look-loop; remove-loop }
    while qpr < len(q):
        q.append(FICT)                       # fifo4_add(FICTITIOUS)
        dmin = FLOAT_MAX
        # ---- look loop: scan from qpr to the FICT just appended ----
        qpl = qpr                            # fifo4_lookreset: qpl = qpr
        while True:
            ofs = q[qpl]; qpl += 1
            if ofs == FICT:
                break
            if px[ofs] != _SHORT_MAX:
                continue
            dp = FLOAT_MAX
            for k in range(4):
                ofsk = ofs + shft[k]
                if not (0 <= ofsk < npix):
                    continue
                if px[ofsk] != _SHORT_MAX:
                    base_x = int(px[ofsk]); base_y = int(py[ofsk])
                    origin = ofsk - base_x - base_y * nx
                    if _inmaskp(pm, nx, origin, base_x + dxl[k], base_y + dyl[k]):
                        base_flag = False
                        dcrt = np.hypot(base_x + dxl[k], base_y + dyl[k]) + pb[ofsk]
                    else:
                        base_flag = True
                        dcrt = np.hypot(base_x, base_y) + pb[ofsk] + 1.0
                    if dcrt < dp:
                        if base_flag:
                            px[ofs] = dxl[k]; py[ofs] = dyl[k]
                            pb[ofs] = dcrt - 1.0
                        else:
                            px[ofs] = base_x + dxl[k]; py[ofs] = base_y + dyl[k]
                            pb[ofs] = pb[ofsk]
                        dp = dcrt
            if dmin > dp:
                dmin = dp
        # ---- remove loop: pop from qpr until FICT ----
        while True:
            ofs = q[qpr]; qpr += 1
            if ofs == FICT:
                break
            if np.hypot(int(px[ofs]), int(py[ofs])) + pb[ofs] > dmin:
                q.append(ofs)                # re-queue (keeps its px/py/pb)
            else:
                for k in range(4):
                    ofsk = ofs + shft[k]
                    if 0 <= ofsk < npix and pm[ofsk] == 1:
                        q.append(ofsk)
                        pm[ofsk] = INQUEUE

    out = np.zeros(npix, dtype=np.float64)
    reached = px != _SHORT_MAX
    out[reached] = pb[reached] + np.hypot(px[reached].astype(np.float64),
                                          py[reached].astype(np.float64))
    return out.reshape(ny, nx)[F:F + ny0, F:F + nx0]


def getexternalboundarygeodesic(im: np.ndarray, mask: np.ndarray,
                                size: float, edu: float = EDU) -> np.ndarray:
    """
    getexternalboundarygeodesic (mspa.c:321):
        thresh(ced(im, mask), 0.0001, size*edu, 0, 1)
    i.e. mask pixels whose GEODESIC (within-mask) Euclidean distance to the
    reference set ``im`` lies in (0.0001, size*edu].
    """
    radius = size * edu
    if radius < 1.0:
        # smallest nonzero grid distance is 1.0 -> empty band
        return np.zeros(im.shape, dtype=np.uint8)
    dist = ced(im.astype(np.uint8), mask.astype(np.uint8))
    band = (dist > 0.0001) & (dist <= radius)
    return band.astype(np.uint8)


def getconnector2core(core: np.ndarray, opening: np.ndarray, residues: np.ndarray,
                      size: float, oitype: int, graphfg: int,
                      edu: float = EDU) -> np.ndarray:
    """
    getconnector2core (mspa.c:333). Returns the connector mask.

      sk = (opening | residues)
      binOIthin_FIFO(sk, oitype, atype=1, anchor=core)   # anchored thinning
      sk = sk - core
      i0 = (opening - core) | residues
      connector = getexternalboundarygeodesic(sk, i0, size-1)
      connector = connector | sk
      connector = connector & i0
      sk_rec = rdil(sk, i0, graphfg)
      connector = connector & sk_rec
    """
    core_b = core.astype(bool)
    opening_b = opening.astype(bool)
    residues_b = residues.astype(bool)

    sk = (opening_b | residues_b).astype(np.uint8)
    sk = binOIthin_FIFO(sk, oitype, core_b.astype(np.uint8))
    sk = (sk.astype(bool) & ~core_b)                     # sk - core

    i0 = (opening_b & ~core_b) | residues_b              # (opening - core) | residues

    connector = getexternalboundarygeodesic(sk.astype(np.uint8), i0.astype(np.uint8),
                                            size - 1, edu).astype(bool)
    connector = connector | sk
    connector = connector & i0
    sk_rec = rdil(sk.astype(np.uint8), i0.astype(np.uint8), graphfg).astype(bool)
    connector = connector & sk_rec
    return connector.astype(np.uint8)


# ===========================================================================
# Bridge test: getcorridor
# ===========================================================================
from collections import deque as _deque


@njit(cache=True)
def _nb_wsfah(out, labels_flat, H, W, graph):
    """Numba core for _wsfah on a flat int64 label buffer ``out`` (walls=-1,
    domain-to-fill=0, markers>0), modified in place.

    Faithful to miallib ``wsfah``: raster-order seeding of marker neighbours
    (4-connected first then diagonals for graph==8), then a level-0 FIFO flood,
    first-come-first-served. Uses a flat-index ring queue instead of a deque.
    """
    n = H * W
    q = np.empty(n, dtype=np.int64)
    head = 0
    tail = 0
    seeded = np.zeros(n, dtype=np.uint8)

    # neighbour (dy, dx) deltas in the same order as the offsets above
    dy4 = np.array([-1, 0, 0, 1], dtype=np.int64)
    dx4 = np.array([0, -1, 1, 0], dtype=np.int64)
    dyd = np.array([-1, -1, 1, 1], dtype=np.int64)
    dxd = np.array([-1, 1, -1, 1], dtype=np.int64)

    # --- seeding: 4-connected pass, then diagonal pass (graph==8) ----------
    for pass_id in range(2):
        if pass_id == 0:
            dyv = dy4; dxv = dx4
        else:
            if graph != 8:
                continue
            dyv = dyd; dxv = dxd
        for idx in range(n):
            lab = labels_flat[idx]
            if lab <= 0:
                continue
            y = idx // W
            x = idx - y * W
            for t in range(dyv.shape[0]):
                nyy = y + dyv[t]
                nxx = x + dxv[t]
                if nyy < 0 or nyy >= H or nxx < 0 or nxx >= W:
                    continue
                nk = nyy * W + nxx
                if out[nk] == 0 and seeded[nk] == 0:
                    out[nk] = lab
                    seeded[nk] = 1
                    q[tail] = nk
                    tail += 1

    # --- level-0 BFS flood, first-come-first-served ------------------------
    if graph == 8:
        dya = np.array([-1, 0, 0, 1, -1, -1, 1, 1], dtype=np.int64)
        dxa = np.array([0, -1, 1, 0, -1, 1, -1, 1], dtype=np.int64)
    else:
        dya = dy4; dxa = dx4

    while head < tail:
        idx = q[head]
        head += 1
        lab = out[idx]
        y = idx // W
        x = idx - y * W
        for t in range(dya.shape[0]):
            nyy = y + dya[t]
            nxx = x + dxa[t]
            if nyy < 0 or nyy >= H or nxx < 0 or nxx >= W:
                continue
            nk = nyy * W + nxx
            if out[nk] == 0:
                out[nk] = lab
                q[tail] = nk
                tail += 1


def _wsfah(labels: np.ndarray, domain: np.ndarray, graph: int) -> np.ndarray:
    """
    Flat-landscape watershed-from-markers by ordered FIFO flooding, a faithful
    port of miallib ``wsfah`` (wsfah.c) as invoked by getcorridor.

    getcorridor calls ``wsfah(lbl, imref, graph, 254)`` where the reference
    image ``imref`` is 0 on the flood domain ``(opening|connector)`` and 255
    (a wall) elsewhere. With a flat (all-zero) reference the immersion reduces
    to a single level-0 breadth-first flood: every core label grows outward
    into the domain one ring at a time, and the FIFO order makes the result
    first-come-first-served (a filled pixel is never relabelled).

    Reproducing *this* flood rather than a generic distance watershed matters:
    scipy/skimage break plateau ties by distance, which can hand a whole thin
    connector to a single core and collapse the bridge test to a loop. The
    miallib flood instead lets both cores reach the connector, so a 1-px neck
    between two distinct cores keeps labels from both (-> bridge). See the
    neighbour ordering in set_shift (setshft.c) and the 4-then-8 seeding of
    uc_u32_wsfah: for graph==8 the 4-connected neighbours of every marker are
    seeded first, the diagonals second.

    Parameters
    ----------
    labels : int array, core-object ids (0 = unlabelled / to be flooded).
    domain : bool array, pixels that may be flooded (walls are outside it).
    graph  : 4 or 8 foreground connectivity.

    Returns the flooded label image (same dtype semantics as ``labels``).
    """
    H, W = labels.shape
    out = labels.astype(np.int64).copy()
    out[~domain.astype(bool)] = -1              # walls: never floodable
    labels_flat = np.ascontiguousarray(labels, dtype=np.int64).ravel()
    out_flat = np.ascontiguousarray(out).ravel()

    _nb_wsfah(out_flat, labels_flat, H, W, graph)

    out = out_flat.reshape(H, W)
    out[out < 0] = 0
    return out


def _wsfah_py_reference(labels: np.ndarray, domain: np.ndarray, graph: int) -> np.ndarray:
    """Pure-Python reference implementation of _wsfah (kept for validation /
    documentation; the production path uses the numba core _nb_wsfah)."""
    H, W = labels.shape
    out = labels.astype(np.int64).copy()
    out[~domain.astype(bool)] = -1              # walls: never floodable

    # neighbour offsets in miallib set_shift order (setshft.c)
    if graph == 8:
        # 4-connected first, diagonals second (matches uc_u32_wsfah seeding)
        n4 = [(-1, 0), (0, -1), (0, 1), (1, 0)]
        nd = [(-1, -1), (-1, 1), (1, -1), (1, 1)]
    else:
        n4 = [(-1, 0), (0, -1), (0, 1), (1, 0)]
        nd = []
    alln = n4 + nd

    q = _deque()
    seeded = np.zeros((H, W), dtype=bool)

    def _seed_pass(offsets):
        # raster-order scan over labelled markers; stamp 0-valued in-domain
        # neighbours with the marker's label and enqueue them.
        ys, xs = np.where(labels > 0)
        for y, x in zip(ys.tolist(), xs.tolist()):
            lab = int(labels[y, x])
            for dy, dx in offsets:
                ny, nx = y + dy, x + dx
                if 0 <= ny < H and 0 <= nx < W and out[ny, nx] == 0 \
                        and not seeded[ny, nx]:
                    out[ny, nx] = lab
                    seeded[ny, nx] = True
                    q.append((ny, nx))

    _seed_pass(n4)
    if nd:
        _seed_pass(nd)

    # level-0 breadth-first flood, first-come-first-served
    while q:
        y, x = q.popleft()
        lab = out[y, x]
        for dy, dx in alln:
            ny, nx = y + dy, x + dx
            if 0 <= ny < H and 0 <= nx < W and out[ny, nx] == 0:
                out[ny, nx] = lab
                q.append((ny, nx))

    out[out < 0] = 0
    return out


def getcorridor(connector: np.ndarray, core: np.ndarray, opening: np.ndarray,
                graphfg: int) -> np.ndarray:
    """
    getcorridor (mspa.c:392): the BRIDGE test.

      lbl   = label(core)                       # distinct core-object ids
      imref = (opening | connector), bg -> wall
      prop  = wsfah(lbl, imref, graph, 254)     # flood core labels over domain
      cor   = label(connector)                  # connector segments
      set_regions(cor, lbl=prop, 20)            # per-segment RANGE of core id
      corridor = (range >= 1)                   # segment spans >= 2 cores

    A connector segment whose flooded core-ids span >= 2 distinct cores joins
    different objects (BRIDGE); a segment touching a single core only loops
    back to it (LOOP). The flood is miallib's ordered FIFO ``_wsfah`` (not a
    distance watershed) so a 1-px neck between two cores is correctly bridged.
    """
    core_b = core.astype(bool)
    conn_b = connector.astype(bool)
    opening_b = opening.astype(bool)
    st = _struct(graphfg)

    core_lbl, n_core = _ndi.label(core_b, structure=st)
    domain = (opening_b | conn_b)

    # wsfah: flood core labels across the domain (background is a wall)
    prop = _wsfah(core_lbl, domain, graphfg)

    conn_lbl, n_conn = _ndi.label(conn_b, structure=st)
    if n_conn == 0:
        return np.zeros_like(core_b, dtype=np.uint8)

    # Per-connector-segment range of the underlying flooded core-id: a segment
    # spanning >= 2 distinct cores is a BRIDGE. Computed in a single pass with
    # labelled min/max over only the pixels that carry a core id (prop > 0),
    # instead of scanning the whole image once per segment (which is
    # O(segments x pixels) and dominates on large rasters).
    seg_ids = conn_lbl[conn_b]                 # segment label per connector px
    core_ids = prop[conn_b]                    # flooded core id per connector px
    valid = core_ids > 0
    seg_v = seg_ids[valid]
    val_v = core_ids[valid]

    seg_min = np.full(n_conn + 1, np.iinfo(np.int64).max, dtype=np.int64)
    seg_max = np.full(n_conn + 1, np.iinfo(np.int64).min, dtype=np.int64)
    # np.minimum.at / maximum.at perform the grouped reduction in one pass.
    np.minimum.at(seg_min, seg_v, val_v)
    np.maximum.at(seg_max, seg_v, val_v)

    is_bridge = (seg_max - seg_min) >= 1       # per-segment-label flag
    is_bridge[0] = False                       # label 0 is background
    corridor = is_bridge[conn_lbl] & conn_b
    return corridor.astype(np.uint8)


# ===========================================================================
# Driver: segmentBinaryPatterns (bit-plane assembly + transition/intext)
# ===========================================================================


def _dilate4(mask: np.ndarray) -> np.ndarray:
    """dilate4: 4-connected dilation by 1 (mspa.c size==1 refinement)."""
    return _ndi.binary_dilation(mask.astype(bool), structure=_struct(4))


def segment_binary_patterns(imin: np.ndarray, edge_width: float,
                            graphfg: int = 8, transition: int = 1,
                            internal: int = 1, n_jobs: int | None = None) -> np.ndarray:
    """
    Pure-Python port of segmentBinaryPatterns (mspa.c).

    imin: uint8 with 0=nodata, 1=background, 2=foreground.
    Returns the MSPA class-coded uint8 image (same shape as imin).

    Bit planes (OR-ed into out, base FG=bit0): edge<<1, perf<<2, islet<<3,
    core<<4, corridor(bridge)<<5, shortcut(loop)<<6. transition only affects
    colouring (not codes). internal(intext) adds core-opening (100) and the
    internal-territory fill (220) plus the +100 internal twins.

    n_jobs : int or None
        Number of worker threads for the mutually independent stages
        (``setedges``, ``getcore``, ``getpatch`` and the internal-holes
        branch). These stages read only the preprocessed image and write
        disjoint results, so they run concurrently; the underlying
        scipy.ndimage / numpy operations release the GIL, giving real
        parallel speed-up on a shared in-memory array (no copying). None
        (default) uses up to 4 threads; 1 forces the serial path. The result
        is identical regardless of ``n_jobs``.
    """
    imin = np.asarray(imin).astype(np.uint8)
    if n_jobs is None:
        n_jobs = min(4, os.cpu_count() or 1)
    n_jobs = max(1, int(n_jobs))

    if graphfg == 8:
        graphbg, oitype = 4, 0
    else:
        graphbg, oitype = 8, 1

    edu = EDU
    size = compute_size(edge_width)
    if (size == 1) and (transition == 2):
        transition = 0
    bufsize = compute_bufsize(size)
    box = int(bufsize + 1.5)
    F = bufsize + 1

    # frame the input with nodata (0)
    im = addframebox(imin, box, 0)

    # main preprocessing (serial prerequisite for the independent stages)
    i0 = fm_preproc(im, size, edu)
    i0 = subframebox(i0, box)
    i0 = fm_preproc2(i0, F)
    i0b = i0.astype(bool)

    # ------------------------------------------------------------------
    # Mutually independent stages. Each reads only i0 (or im) and writes a
    # disjoint result, so they run concurrently on a shared array. The heavy
    # work is scipy.ndimage / numpy, which releases the GIL, so threads give
    # real parallelism without copying the (potentially huge) arrays. setedges
    # is the long pole (an internal serial fillhole loop), so overlapping it
    # with core/patch/holes is where the wall-clock time is recovered.
    # ------------------------------------------------------------------
    # core is a single distance transform; compute it once up front and reuse
    # it in getpatch, setedges and opening, saving two redundant full-image
    # EDTs (the EDT is scipy's internally multi-threaded op, so running it once
    # on its own already uses many cores).
    core = getcore(i0, size, edu).astype(bool)

    def _task_patch():
        return getpatch(i0, size, graphfg, edu, core=core.astype(np.uint8)).astype(bool)

    def _task_edges():
        e, p = setedges(i0, size, graphfg, graphbg, edu, core=core)
        return e.astype(bool), p.astype(bool)

    def _task_holes():
        # internal: all holes (uses size=1 preproc) at the box-framed resolution
        i0h = fm_preproc(im, 1, edu).astype(bool)
        return (uc_fillhole(i0h.astype(np.uint8), graphbg).astype(bool) & ~i0h)

    if n_jobs > 1:
        with ThreadPoolExecutor(max_workers=n_jobs) as ex:
            f_edges = ex.submit(_task_edges)
            f_patch = ex.submit(_task_patch)
            f_holes = ex.submit(_task_holes) if internal == 1 else None
            patch = f_patch.result()
            edges, perf = f_edges.result()
            allHoles = f_holes.result() if f_holes is not None else None
    else:
        patch = _task_patch()
        edges, perf = _task_edges()
        allHoles = _task_holes() if internal == 1 else None

    # when box == F the allHoles frame already matches i0; otherwise reframe.
    if internal == 1 and allHoles.shape != i0b.shape:
        # strip box frame, re-add F frame (edge-extend not needed: holes are 0)
        ah = subframebox(allHoles.astype(np.uint8), box)
        allHoles = addframebox(ah, F, 0).astype(bool)

    out = i0.astype(np.int64)                     # FG base = bit0 = 1

    opening = (getexternalboundary(core.astype(np.uint8), size, edu).astype(bool) | core)

    # residues = i0 - core - patch - perforation - edges  (bit assembly)
    residues = i0b & ~core
    residues = residues & ~patch
    out = out | (patch.astype(np.int64) << 3)     # islet = 8
    residues = residues & ~perf
    out = out | (perf.astype(np.int64) << 2)      # perforation = 4
    residues = residues & ~edges
    out = out | (edges.astype(np.int64) << 1)     # edge = 2

    connector = getconnector2core(core.astype(np.uint8), opening.astype(np.uint8),
                                  residues.astype(np.uint8), size, oitype, graphfg,
                                  edu).astype(bool)
    corridor = getcorridor(connector.astype(np.uint8), core.astype(np.uint8),
                           opening.astype(np.uint8), graphfg).astype(bool)

    out = out | (core.astype(np.int64) << 4)      # core = 16
    shortcut = connector & ~corridor              # shortcut = connector - corridor

    if size == 1:
        corridor = corridor | (_dilate4(corridor) & residues)
        shortcut = shortcut | (_dilate4(shortcut) & residues)

    out = out | (corridor.astype(np.int64) << 5)  # bridge = 32
    out = out | (shortcut.astype(np.int64) << 6)  # loop   = 64

    if internal == 1:
        tmp = uc_fillhole(core.astype(np.uint8), graphbg).astype(bool)
        coreHoles = tmp & ~core
        # allHoles = allHoles AND NOT coreHoles
        allHoles = allHoles & ~coreHoles
        allHoles_val = np.where(allHoles, 220, 0).astype(np.int64)
        tmp = tmp & ~core                          # filled core-hole region
        tmp2 = uc_fillhole(tmp.astype(np.uint8), graphbg).astype(bool)
        tmp2_val = np.where(tmp2, 100, 0).astype(np.int64)
        out = out + tmp2_val                       # +100 internal twins / core-opening
        out = np.maximum(out, allHoles_val)        # SUP with 220 regions

    # crop frame
    out = subframebox(out, F)

    # final masking: nodata (imin==0) -> 129
    nodata = (imin == 0)
    out = out.astype(np.int64)
    out[nodata] = 129

    return out.astype(np.uint8)
