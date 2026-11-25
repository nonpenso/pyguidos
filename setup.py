"""
Build configuration for pyGuidos.

Project metadata lives in pyproject.toml; this setup.py exists to declare the
compiled ``pyguidos._mspa`` C extension, which bundles a minimal subset of the
miallib C library to compute MSPA (Morphological Spatial Pattern Analysis).

The MSPA C sources (miallib, by Pierre Soille and Peter Vogt) are vendored
unmodified under pyguidos/_mspa/miallib/ and are licensed under GPL v3.
"""
import os
import sys
from setuptools import setup, Extension

import numpy


HERE = os.path.abspath(os.path.dirname(__file__))
MSPA_DIR = os.path.join("pyguidos", "_mspa")
MIALLIB_DIR = os.path.join(MSPA_DIR, "miallib")

# The vendored miallib C files required to compile MSPA (segmentBinaryPatterns).
# This is the minimal transitive closure; see pyguidos/_mspa/miallib/ notes.
MIALLIB_SOURCES = [
    "mspa.c",
    "efedt.c",
    "ced.c",
    "skel.c",
    "recons.c",
    "label.c",
    "wsfah.c",
    "setreg.c",
    "imstat.c",
    "pointop.c",
    "format.c",
    "geom.c",
    "imem.c",
    "fifo.c",
    "fah.c",
    "pqueue.c",
    "setshft.c",
    "erodil.c",
    "miscel.c",
    "wshed.c",
]

# ---------------------------------------------------------------------------
# OpenMP (multi-core MSPA). The miallib sources contain OpenMP loops guarded by
# `#ifdef OPENMP`; they are switched on per platform, without modifying miallib:
#   - Linux (GCC):   -fopenmp and OPENMP defined for all files. The runtime
#                    (libgomp) is bundled into the wheel by auditwheel.
#   - Windows (MSVC): classic /openmp (redistributable runtime vcomp140.dll,
#                    bundled by delvewheel). MSVC's OpenMP 2.0 needs signed
#                    loop counters, so OpenMP is enabled only for the files
#                    that meet it (distance transform, erosion/dilation, ced),
#                    through the small wrappers in pyguidos/_mspa/omp/.
#   - macOS (Apple clang): no OpenMP support -> serial build.
# Set PYGUIDOS_NO_OPENMP=1 to force a serial build on any platform.
# At run time the number of threads follows OMP_NUM_THREADS (default: all cores).
# ---------------------------------------------------------------------------
OMP_DIR = os.path.join(MSPA_DIR, "omp")
WIN_OMP_FILES = {"efedt.c": "efedt_omp.c", "erodil.c": "erodil_omp.c", "ced.c": "ced_omp.c"}

use_openmp = os.environ.get("PYGUIDOS_NO_OPENMP", "") not in ("1", "true", "yes")
openmp_mode = None
if use_openmp and sys.platform == "win32":
    openmp_mode = "msvc-partial"
elif use_openmp and sys.platform.startswith("linux"):
    openmp_mode = "gcc"

sources = [os.path.join(MSPA_DIR, "bridge.c")]
for f in MIALLIB_SOURCES:
    if openmp_mode == "msvc-partial" and f in WIN_OMP_FILES:
        sources.append(os.path.join(OMP_DIR, WIN_OMP_FILES[f]))
    else:
        sources.append(os.path.join(MIALLIB_DIR, f))

# -DMSPA is REQUIRED: it gates out miallib clustering code (in setreg.c) that
# would otherwise pull in dcluster.c -> indexx.c -> GSL. With MSPA defined the
# MSPA subset links cleanly with no external scientific/geo libraries.
define_macros = [
    ("MSPA", None),
    ("NPY_NO_DEPRECATED_API", "NPY_1_7_API_VERSION"),
]

# Platform-specific compile flags.
extra_compile_args = []
if sys.platform == "win32":
    # MSVC: silence the flood of legacy-C warnings from miallib and allow the
    # older C style used throughout the library.
    extra_compile_args += ["/w"]
else:
    # GCC/Clang: miallib is legacy C; keep warnings quiet and non-fatal so the
    # vendored sources build unmodified across compilers.
    extra_compile_args += [
        "-w",
        "-fcommon",  # tolerate legacy tentative/common globals (e.g. buf)
    ]

extra_link_args = []
if openmp_mode == "msvc-partial":
    extra_compile_args += ["/openmp"]          # OPENMP is defined by the wrappers
elif openmp_mode == "gcc":
    define_macros.append(("OPENMP", None))
    extra_compile_args += ["-fopenmp"]
    extra_link_args += ["-fopenmp"]
print(f"pyguidos._mspa: OpenMP build mode = {openmp_mode or 'serial'}")

mspa_ext = Extension(
    name="pyguidos._mspa._mspa",
    sources=sources,
    include_dirs=[
        numpy.get_include(),
        MIALLIB_DIR,
    ],
    define_macros=define_macros,
    extra_compile_args=extra_compile_args,
    extra_link_args=extra_link_args,
    language="c",
)

setup(
    ext_modules=[mspa_ext],
)
