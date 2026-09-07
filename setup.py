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

sources = [os.path.join(MSPA_DIR, "bridge.c")]
sources += [os.path.join(MIALLIB_DIR, f) for f in MIALLIB_SOURCES]

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

mspa_ext = Extension(
    name="pyguidos._mspa._mspa",
    sources=sources,
    include_dirs=[
        numpy.get_include(),
        MIALLIB_DIR,
    ],
    define_macros=define_macros,
    extra_compile_args=extra_compile_args,
    language="c",
)

setup(
    ext_modules=[mspa_ext],
)
