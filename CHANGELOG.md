# Changelog

All notable changes to pyGuidos are documented in this file.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).
pyGuidos uses [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [2.5.1] - 2026-07-15

### Fixed
- Fixed integer overflow in `compute_FAD_gray`, `compute_FAC_gray`, and `compute_FED_gray` Numba kernels. Accumulators now use explicit `int64` to prevent overflow when window values are multiplied by 200 in the round-half-up formula.
- Fixed test failure in `test_threshold_reduces_foreground` where output subdirectories were not created before writing GeoTIFF files.

---

## [2.5.0] - 2026-07-10

### Overview
Version 2.5.0 introduces grayscale fragmentation analysis (`frag_gray()`), extending the binary FOS approach to continuous-value rasters where pixel values represent foreground intensity from 0 to 100 (e.g., tree cover density). Three methods (FAD, FAC, FED) are available with configurable foreground thresholds and 4/8-connectivity options.

### Added
- **`frag_gray()` function**: New grayscale fragmentation analysis supporting FAD, FAC, and FED methods on continuous-value input rasters (uint8, values 0-100, >100 = NoData).
- **`frag_gray_stats()` function**: Standalone statistics computation from existing grayscale fragmentation output GeoTIFFs. Optionally reads source raster for input pixel counts.
- **`for_threshold` parameter**: Foreground threshold (1-100) allowing users to define which pixel intensities are considered foreground. Applied inside the Numba kernel for zero-copy efficiency.
- **`compute_FAD_gray()`** Numba kernel: Grayscale FAD computation with threshold integration.
- **`compute_FAC_gray()`** Numba kernel: Grayscale FAC with threshold, 4/8-connectivity.
- **`compute_FED_gray()`** Numba kernel: Grayscale FED with threshold, 4/8-connectivity.
- **`validate_fmap_gray_input()`** validation function in `checks.py`.
- **Example data**: Added `TCD2023_sardinia.tif` (Copernicus Tree Cover Density 2023, Sardinia, 100m).
- **Example notebook**: `05_grayscale_fragmentation.ipynb` demonstrating all grayscale methods with threshold comparison.
- **Documentation**: New `docs/usage/fragmentation_gray.rst` with sub-chapters per method, formulas, and figure examples.

### Changed
- Updated `docs/usage/fragmentation.rst` with method sub-chapters (FAD, FAC, FED) and individual figures per method.
- Updated `docs/usage/index.rst` to include the new grayscale fragmentation page.
- Updated `__init__.py` to export `frag_gray` and `frag_gray_stats`.
- Removed `pyproj` dependency. EPSG resolution now uses rasterio's built-in CRS method.
- Special Background 4 (SP4, value=4) is now treated as NoData/missing during fragmentation computation (FAD, FAC, FED). SP4 pixels in the moving window are excluded from the denominator and pair counting, meaning they do not fragment the foreground.

---

## [2.4.1] - 2026-07-13

### Changed
- Special Background 4 (SP4, value=4) is now treated as NoData/missing during fragmentation computation (FAD, FAC, FED). SP4 pixels in the moving window are excluded from the denominator and pair counting, meaning they do not fragment the foreground.
- Improved `docs/usage/input_format.rst` with structured "Input Map Types" chapter, detailed explanations for all pixel values (0–4), and consistent table format.

### Fixed
- Fixed incorrect parameter name `outfile=True` → `stat_files=True` in the `frag_stats()` call within Notebook 04 (Regional Analysis).

---

## [2.4.0] - 2026-07-03

### Overview
Version 2.4.0 extends the fragmentation analysis module with two significant enhancements: the Foreground Edge Density (FED) method and configurable pixel connectivity (4- or 8-connected) for both FAC and FED. These additions enable more nuanced characterisation of landscape fragmentation patterns by accounting for edge interactions and diagonal adjacency.

### Added
- **`FED` (Foreground Edge Density) method**: New fragmentation metric that scores pixel pairs based on foreground–foreground (weight 1.0), foreground–background (weight 0.5), and background–background (weight 0.0) interactions within the moving window. Available via `pg.frag(method='FED', ...)`.
- **`connectivity` parameter**: New optional parameter for `frag()` accepting values 4 (default, horizontal + vertical pairs) or 8 (adds diagonal pairs). Applies to both FAC and FED methods; ignored for FAD.
- New `compute_FED()` Numba kernel in `engine.py` with full parallel execution and pre-clamped bounds.
- **`layer` parameter** for `extract_by_polygon()`: supports multi-layer vector files (GeoPackage, FileGDB). Exits with a clear error listing available layers if a multi-layer file is detected and no layer is specified.

### Changed
- `compute_FAC()` now accepts a `connectivity` parameter (4 or 8) instead of being hardcoded to 4-connected. The 8-connected mode adds NW-SE and NE-SW diagonal edge scanning.
- `validate_frag_params()` in `checks.py` updated to accept `'FED'` as a valid method and validate the `connectivity` parameter.
- GeoTIFF metadata tag format updated to encode the connectivity value instead of the hardcoded `8`.
- `extract_by_polygon()` no longer attempts CRS reprojection. Instead, it checks that vector and raster bounding boxes overlap and exits with a clear error if they do not.
- Configured PyPI Trusted Publisher in `.gitlab-ci.yml` for automated, token-free package publishing.
- Reordered example notebooks: Fragmentation Change is now Notebook 03, Regional Analysis is Notebook 04. Updated all cross-references accordingly.

---

## [2.3.2] - 2026-06-12

### Overview
Version 2.3.2 is a documentation and project governance patch release. It explicitly defines the project's open-source infrastructure architecture, linking the official upstream European Commission platform (`code.europa.eu`) with the public community interaction ecosystem on GitHub to streamline user engagement and journal peer-review audits.

### Added
- **Developer Documentation**: Added a comprehensive, standalone `contributing.rst` guide inside the Sphinx documentation suite outlining development environment isolation, Numba-specific testing protocols, and the dual-forge contribution workflow.
- **Cross-Referencing**: Integrated deep linking between the installation guides, the main `index.rst` homepage, and the developer contribution protocols.

### Changed
- **Ecosystem Transparency**: Overhauled the core `README.md` and Sphinx `index.rst` templates to clearly differentiate the institutional role of `code.europa.eu` (powered by GitLab) from the public community portal on GitHub.
- **Issue Tracking Realignment**: Explicitly routed public bug reporting, installation troubleshooting, and community Pull Requests away from internal Commission trackers directly to the public GitHub issue matrix.

---

## [2.3.1] - 2026-06-12

### Overview
Version 2.3.1 is a patch release that fixes critical bugs in the `frag_change()` module introduced in 2.3.0, adds example data and a notebook for fragmentation change analysis, and improves overall robustness.

### Added
- **Example notebook**: New `04_fragmentation_change.ipynb` demonstrating multi-temporal fragmentation change analysis using Corsica CLC 2000 and CLC 2018 data.
- **Example data**: Added `CLC2000_corsica_FNF.tif` (Forest/Non-Forest map from Corine Land Cover 2000) to the bundled example dataset.

### Fixed
- Fixed critical indentation bug in `_get_frag_change_stats()` where the statistics dictionary was only constructed when `out_file=True`, causing the function to return `None` silently otherwise.
- Fixed `frag_change()` crash when output file already exists from a previous failed run (`TIFFReadDirectory: Cannot handle zero number of tiles`). Corrupt leftover files are now removed before writing.
- Fixed `frag_change()` crash on tiled output writing by adding explicit `blockxsize` and `blockysize` parameters to the output profile.
- Fixed `frag_change()` mutating the input profile dictionary by adding a `.copy()` before updating.

---

## [2.3.0] - 2026-06-08

### Overview
Version 2.3.0 introduces advanced temporal analysis capabilities to the `pyguidos` suite with the implementation of the "frag_change" module. This new engine allows users to perform multi-temporal fragmentation comparisons across different time periods, generating not only spatial change trajectories but also rigorous statistical reporting (including delta matrices and confusion matrices) and visual chart summaries.

### Added
- **`frag_change()` function**: Introduced a dedicated multi-temporal spatial engine to calculate fragmentation transitions between two distinct time periods using standardized input GeoTIFFs.
- New `get_colormap()` utility function for consistent colormap loading across modules.
- New `validate_fchmaps_input()` validator for temporal fragmentation change inputs.

### Changed
- Performance: Pre-clamped window bounds in compute_FAD and compute_FAC Numba kernels, eliminating per-pixel boundary checks in the inner loop.
- Refactored `save_output_geotiff()` to use the new `get_colormap()` utility, reducing code duplication.
- Removed unused `geopandas` import from `extract_by_polygon.py`.

### Fixed
- Fixed crash in `landmos()` when `statists=False` (attempted dict access on `None`).
- Fixed malformed error message string in `landmos_stats()`.
- Corrected `matplotlib` version pin from `>=3.10.9` to `>=3.10`.

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
