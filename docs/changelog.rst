Changelog
=========

All notable changes to pyGuidos are documented here.
The format follows `Keep a Changelog <https://keepachangelog.com/en/1.0.0/>`_
and pyGuidos uses `Semantic Versioning <https://semver.org/spec/v2.0.0.html>`_.

----

Unreleased
----------

----

2.0.0 — 2026
------------

**Overview**

Version 2 is a complete architectural redesign. Rather than wrapping GWB bash
commands (which required Linux, GDAL, and a GWB installation), pyGuidos v2 calls
the ``mspa`` and ``spatcon`` binaries directly and implements two further functions
natively in Python. The binaries for Linux, Windows, and macOS are bundled inside
the package, so installation requires only ``pip install pyguidos`` on any platform.

Input and output now operate on individual GeoTIFF files rather than directories,
aligning with standard Python geospatial conventions. All functions return typed
dataclass result objects carrying both the output array and summary statistics.

Five functions are implemented in this release. The remaining GWB modules available
in v1 (``gwb_dist``, ``gwb_parc``, ``gwb_rec``, ``gwb_sc``, ``gwb_gsc``, ``gwb_spa``)
are planned for future 2.x releases.

**Added**

- ``frag()`` — Forest fragmentation via FAD and FAC methods, backed by the
  ``spatcon`` binary. Supports binary and grayscale input maps, 5-class and
  6-class reporting schemes.
- ``landmos()`` — Landscape Mosaic analysis (19-class and 103-class), backed
  by the ``spatcon`` binary. Returns both output rasters and a ternary heatmap
  summary.
- ``mspa()`` — Morphological Spatial Pattern Analysis, backed by the ``mspa``
  binary. Supports all four MSPA parameters (connectivity, edge width,
  transition, internal/external).
- ``acc()`` — Patch accounting with up to 5 user-defined area thresholds,
  implemented natively in Python using ``scipy`` connected-component labeling.
- ``rss()`` — Restoration Status Summary computing nine network coherence
  indicators (ECA, COH, RAC, CNOA, REST_POT, total patches,
  average/median/largest patch size), implemented natively in Python.
- ``extract_by_polygon()`` — Clips any pyGuidos output raster to polygons from
  any vector format supported by ``fiona``, with automatic nodata handling and
  CRS reprojection.
- ``frag_stats()``, ``mspa_stats()``, ``landmos_stats()``, ``acc_stats()`` —
  Standalone statistics functions that compute summary statistics from existing
  output GeoTIFFs without re-running the full analysis.
- ``gtb_colormap()`` — Reads the embedded GTB color palette from an output
  GeoTIFF and returns a ``matplotlib.colors.ListedColormap`` and ``Normalize``
  object for publication-quality map rendering.
- Result dataclasses — ``MSPAResult``, ``FragResult``, ``LandMosResult``,
  ``AccResult``, ``RssResult`` with typed ``stats`` dictionaries and optional
  NumPy array fields.
- GTB metadata tags — all output GeoTIFFs embed processing parameters in
  ``TIFFTAG_IMAGEDESCRIPTION`` for later recovery by standalone stats functions.
- Cross-platform binary bundling — ``mspa`` and ``spatcon`` binaries for
  Linux x86_64, Windows x64, macOS x86_64, and macOS ARM (Apple Silicon).
- Input validation — dedicated ``checks`` module with six validator functions.
- Tiled GeoTIFF handling — automatic detection and conversion before MSPA processing.
- Automatic job cleanup — temporary job directories older than 7 days are
  silently removed.
- Sphinx documentation with PyData theme, hosted at
  `jrc-forest.pages.code.europa.eu/guidos/pyguidos/
  <https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/>`_.
- Four example Jupyter notebooks using Copernicus Land Cover data over Corsica.
- 94% of tested statements through 14 scripts in pure Python, no GeoTIFF files or binaries required.
- Standard pip packaging via ``pyproject.toml``, ``MANIFEST.in``, ``README.md``.

**Changed**

- Architecture — from GWB bash wrapper (Linux-only, requires GDAL and GWB)
  to direct binary interface (cross-platform, self-contained).
- I/O convention — from directory-based batch processing to single-file function
  calls returning result objects.
- GDAL dependency removed — all raster I/O now uses ``rasterio``, which ships
  with self-contained GDAL binaries via Python wheels.
- Function naming — ``gwb_frag`` → ``frag()``, ``gwb_mspa`` → ``mspa()``,
  ``gwb_lm`` → ``landmos()``, ``gwb_acc`` → ``acc()``, ``gwb_rss`` → ``rss()``.
- ``write_guidos_input()`` split into ``write_mspa_input()`` and
  ``write_spatcon_input()`` with separate signatures.
- ``get_module_root()`` replaced with ``MODULE_ROOT = Path(__file__).resolve().parent``
  for pip compatibility.

**Removed**

- GWB dependency — ``gwb_dist``, ``gwb_parc``, ``gwb_rec``, ``gwb_sc``,
  ``gwb_gsc``, ``gwb_spa`` are not yet available in v2 and are planned
  for future 2.x releases.
- Linux-only constraint — v2 runs on Linux, Windows, and macOS.

----

1.0.0 — 2025-07-11
------------------

**Overview**

First public release of pyGuidos. Provides a Python interface to
GuidosToolbox Workbench (GWB) on Linux by wrapping GWB module calls as Python
functions via ``subprocess``. Requires a working GWB installation and
system-level GDAL on Linux.

**Added**

- ``gwb_acc()`` — Accounting analysis (object identification and area thresholding).
- ``gwb_frag()`` — Fragmentation analysis with FAD, FED, and FAC methods.
- ``gwb_mspa()`` — Morphological Spatial Pattern Analysis.
- ``gwb_rss()`` — Restoration Status Summary analysis.
- ``gwb_dist()`` — Euclidean Distance analysis, optionally with Hypsometric Curve.
- ``gwb_lm()`` — Landscape Mosaic analysis (19-class and 103-class).
- ``gwb_parc()`` — Parcellation analysis for land cover classification.
- ``gwb_rec()`` — Recoding of categorical class values in TIFF maps.
- ``gwb_sc()`` — Spatial Convolution (SpatCon) for various landscape metrics.
- ``gwb_gsc()`` — Gray Spatial Convolution (GraySpatCon) for grayscale metrics.
- ``gwb_spa()`` — Simplified Spatial Pattern Analysis.
- Example Jupyter notebooks demonstrating key functions with real landscape data.
- Unit test suite covering core function behavior.
- Standard pip packaging via ``pyproject.toml``.
