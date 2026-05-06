# Changelog

All notable changes to pyGuidos are documented in this file.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).
pyGuidos uses [Semantic Versioning](https://semver.org/spec/v2.0.0.html).


---

## [2.2.0] - 2026-05-06

### Overview
Version 2.2.0 achieves a major milestone with the introduction of "SPA" (Simplified Pattern Analysis), a simplified version of MSPA module, now fully reimplemented in native Python. This release also future-proofs the library by providing full compatibility with the NumPy 2.x ecosystem and official testing for Python up to version 3.14. To improve maintainability and developer access, the internal architecture has been refactored to separate raw numerical computation from statistical reporting, providing more granular access to spatial data via dedicated stats functions.

### Added
- **SPA Native Implementation**: Introduced Simplified Pattern Analysis (SPA) with the function `spa()`, a high-performance Python-native morphological engine.
- **Python 3.14 Support**: Official support and CI/CD testing for the latest Python 3.13 and 3.14 releases.
- **NumPy 2.x Compatibility**: Updated all internal array logic and C-API interactions to support NumPy 2.0+ promotion rules and metadata structures.

### Changed
- **Architectural Refactoring**: Core logic has been decoupled into "Compute" (raw array processing) and "Export" (statistics and GeoTIFF generation) layers for better performance and modularity.
- **Unified Internal Pipeline:** Standardized how `frag()`, `rss()`, `acc()`, `landmos()` and `spa()` handle data flow, ensuring that the main function and its stats counterpart call the same underlying engine.
- **Numba Optimization**: Refined JIT-compilation signatures to maintain high-speed execution across the transition to NumPy 2.x.

### Fixed
- **Statistical Consistency**: Fixed potential discrepancies between file output and dictionary results by unifying the internal calculation calls.
- **Memory Efficiency**: Optimized the `xxx_stats()` calls to reduce memory overhead when processing extremely large GeoTIFFs where only tabular data is required.


---

## [2.1.0] - 2026-04-21

### Overview
Version 2.1.0 represents a major leap in performance and maintainability. The core spatial engines for Fragmentation and Landscape Mosaic have been rewritten as native, Numba-optimized Python functions. This change eliminates the dependency on the `spatcon` binary for these modules, enabling massive parallelization and significantly faster execution on multi-core systems. This release also marks the introduction of a comprehensive automated test suite with over 90% code coverage.

### Added
- **Native Spatial Engines**: Replaced `spatcon` binary calls with high-performance Numba JIT-compiled kernels for Fragmentation FAD and FAC, and Landscape Mosaic.
- **Advanced Parallelization**: Spatial operations now utilize all available CPU cores via Numba’s `parallel=True` execution, significantly reducing processing time for large GeoTIFFs.
- **Enhanced Rounding Logic** — Implemented "Round-Half-Up" integer arithmetic in spatial kernels to ensure 100% consistency with legacy GuidosToolbox results.

### Changed
- **Numba Caching**: JIT-compiled kernels are now cached in a persistent temporary directory to eliminate "cold-start" lag on subsequent tool runs.
- **Error Reporting**: Standardized output codes (e.g., 101 for Background, 102 for Missing) across all spatial modules for better integration with GIS software.
- **Threading Control**: Intelligent OS-level threading layer selection (TBB for Windows, OpenMP for Linux, Workqueue for macOS).

### Removed
- **`mspa()` Function**: The morphological spatial pattern analysis module has been removed. A pure-Python native implementation is currently in development.

### Fixed
- **Integer Math Precision**: Resolved a discrepancy where `83.8%` would occasionally floor to `83` instead of rounding to `84`.


---

## [2.0.0] - 2026-04-16

### Overview

Version 2 is a complete architectural redesign. Rather than wrapping
GWB bash commands (which required Linux, GDAL, and a GWB installation),
pyGuidos v2 calls the `mspa` and `spatcon` binaries directly and
implements two further functions natively in Python. The binaries for
Linux, Windows, and macOS are bundled inside the package, so
installation requires only `pip install pyguidos` on any platform.

Input and output now operate on individual GeoTIFF files rather than
directories, aligning with standard Python geospatial conventions.
All functions return typed dataclass result objects carrying both the
output array and summary statistics.

Five functions are implemented in this release. The remaining GWB
modules available in v1 (`gwb_dist`, `gwb_parc`, `gwb_rec`, `gwb_sc`,
`gwb_gsc`, `gwb_spa`) are planned for future 2.x releases.

### Added

- **`frag()`** — Forest fragmentation via FAD and FAC methods, backed
  by the `spatcon` binary. Supports binary and grayscale input maps,
  5-class and 6-class reporting schemes.
- **`landmos()`** — Landscape Mosaic analysis (19-class and 103-class),
  backed by the `spatcon` binary. Returns both output rasters and a
  ternary heatmap summary.
- **`mspa()`** — Morphological Spatial Pattern Analysis, backed by the
  `mspa` binary. Supports all four MSPA parameters (connectivity,
  edge width, transition, internal/external).
- **`acc()`** — Patch accounting with up to 5 user-defined area
  thresholds, implemented natively in Python using `scipy`
  connected-component labeling.
- **`rss()`** — Restoration Status Summary, computing nine network
  coherence indicators (ECA, COH, RAC, CNOA, REST_POT, total patches,
  average/median/largest patch size), implemented natively in Python.
- **`extract_by_polygon()`** — Clips any pyGuidos output raster to
  polygons from any vector format supported by `fiona`, with automatic
  nodata handling and CRS reprojection.
- **`frag_stats()`**, **`mspa_stats()`**, **`landmos_stats()`**,
  **`acc_stats()`** — Standalone statistics functions that compute
  summary statistics from existing output GeoTIFFs without re-running
  the analysis.
- **`gtb_colormap()`** — Reads the embedded GTB color palette from an
  output GeoTIFF and returns a `matplotlib.colors.ListedColormap` and
  `Normalize` object for publication-quality map rendering.
- **Result dataclasses** — `MSPAResult`, `FragResult`, `LandMosResult`,
  `AccResult`, `RssResult` with typed `stats` dictionaries and optional
  NumPy array fields.
- **GTB metadata tags** — All output GeoTIFFs embed processing
  parameters in `TIFFTAG_IMAGEDESCRIPTION` using a structured
  `GTB_TOOLID, <params>, URL` format for later recovery by standalone
  stats functions.
- **Cross-platform binary bundling** — `mspa` and `spatcon` binaries
  for Linux x86_64, Windows x64, macOS x86_64, and macOS ARM (Apple
  Silicon) are bundled under `pyguidos/progs/`.
- **Input validation** — Dedicated `checks` module with six validator
  functions covering window size, pixel values, band count, method
  names, MSPA parameters, and accounting thresholds.
- **Tiled GeoTIFF handling** — `write_mspa_input()` automatically
  detects and rewrites tiled GeoTIFFs as strip-based TIFFs before
  passing to the MSPA binary.
- **Automatic job cleanup** — `setup_run_dir()` silently removes
  temporary job directories older than 7 days.
- **Sphinx documentation** — Full documentation with PyData theme,
  hosted at `https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/`,
  including installation guide, usage pages for all five modules, and
  this changelog.
- **Example notebooks** — Four Jupyter notebooks demonstrating all
  functions using Copernicus Land Cover data over Corsica (EPSG:3035,
  100 m resolution):
  `01_getting_started.ipynb`, `02_forest_pattern.ipynb`,
  `03_landscape_mosaic.ipynb`, `04_regional_analysis.ipynb`.
- **Unit tests** — 134 tests across `test_checks.py` (63),
  `test_utils.py` (38), `test_results.py` (33); all pure Python,
  no GeoTIFF files or binaries required.
- **pip packaging** — `pyproject.toml`, `MANIFEST.in`, `README.md`,
  `.gitignore` for standard pip distribution.

### Changed

- **Architecture** — from GWB bash wrapper (Linux-only, requires GDAL
  and GWB installation) to direct binary interface (cross-platform,
  self-contained).
- **I/O convention** — from directory-based batch processing to
  single-file function calls.
- **GDAL dependency removed** — all raster I/O now uses `rasterio`,
  which ships with self-contained GDAL binaries via Python wheels,
  eliminating system-level GDAL installation requirements.
- **Function naming** — `gwb_frag` → `frag()`, `gwb_mspa` → `mspa()`,
  `gwb_lm` → `landmos()`, `gwb_acc` → `acc()`, `gwb_rss` → `rss()`.
- **`get_module_root()`** replaced with `MODULE_ROOT = Path(__file__).resolve().parent`
  for pip compatibility.
- **`write_guidos_input()`** split into `write_mspa_input()` and
  `write_spatcon_input()` with separate signatures and responsibilities.

### Removed

- GWB dependency — `gwb_dist`, `gwb_parc`, `gwb_rec`, `gwb_sc`,
  `gwb_gsc`, `gwb_spa` are not yet available in v2 and are planned
  for future 2.x releases.
- Linux-only constraint — v2 runs on Linux, Windows, and macOS.

---

## [1.0.0] - 2025-07-11

### Overview

First public release of pyGuidos. Provides a Python interface to
GuidosToolbox Workbench (GWB) on Linux by wrapping GWB module calls
as Python functions. Each function prepares an input directory,
writes a GWB parameter file, and invokes the corresponding GWB bash
command via Python's `subprocess` module. Requires a working GWB
installation and system-level GDAL on Linux.

### Added

- **`gwb_acc()`** — Accounting analysis (object identification and
  area thresholding).
- **`gwb_frag()`** — Fragmentation analysis with FAD, FED, and FAC
  methods at user-defined observation scales.
- **`gwb_mspa()`** — Morphological Spatial Pattern Analysis.
- **`gwb_rss()`** — Restoration Status Summary analysis.
- **`gwb_dist()`** — Euclidean Distance analysis, optionally with
  Hypsometric Curve.
- **`gwb_lm()`** — Landscape Mosaic analysis (19-class and 103-class).
- **`gwb_parc()`** — Parcellation analysis for land cover
  classification.
- **`gwb_rec()`** — Recoding of categorical class values in TIFF maps.
- **`gwb_sc()`** — Spatial Convolution (SpatCon) for various landscape
  metrics.
- **`gwb_gsc()`** — Gray Spatial Convolution (GraySpatCon) for
  grayscale image metrics.
- **`gwb_spa()`** — Simplified Spatial Pattern Analysis.
- **Example notebooks** — Jupyter notebooks demonstrating key
  functions with real landscape data.
- **Unit tests** — Test suite covering core function behavior.
- **`pyproject.toml`** — Standard pip packaging configuration.

---

[Unreleased]: 
[2.0.0]: https://code.europa.eu/jrc-forest/guidos/pyguidos/-/releases/v2.0.0
[1.0.0]: https://code.europa.eu/jrc-forest/guidos/pyguidos/-/releases/v1.0.0
