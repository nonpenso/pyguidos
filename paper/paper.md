---
title: 'pyGuidos: A cross-platform Python interface to GuidosToolbox Workbench for pattern analysis of raster maps'
tags:
  - Python
  - landscape ecology
  - forest monitoring
  - forest connectivity
  - spatial analysis
  - remote sensing
  - fragmentation
  - MSPA
authors:
  - name: Giovanni Caudullo
    orcid: 0000-0003-4061-1204
    affiliation: 1
  - name: Peter Vogt
    orcid: 0000-0002-1030-4492
    affiliation: 2
affiliations:
  - index: 1
    name: Arcadia SIT s.r.l, Vigevano, Italy
  - index: 2
    name: European Commission, Joint Research Centre (JRC), Ispra, Italy
    ror: 02k4b9v70
date: 12 June 2026
repository: https://github.com/nonpenso/pyguidos
archive: https://code.europa.eu/jrc-forest/guidos/pyguidos
bibliography: paper.bib
---

# Summary

Forests and other natural habitats are increasingly threatened by fragmentation, degradation, and land-use change. Quantifying these processes requires spatially explicit indicators derived from land cover raster maps. The GuidosToolbox Workbench (GWB) is a free software developed at the European Commission Joint Research Centre (JRC) that provides command-line scripts to a suite of raster image analysis modules, including forest fragmentation assessment, landscape mosaic classification, pattern analysis, patch accounting, and network coherence metrics for automated processing on Linux x86_64 systems. The GWB modules rely on compiled binary executables and Interactive Data Language (IDL) wrapper routines. IDL is bundled as a standalone runtime, but compiling modified source code requires a proprietary license. The IDL software is also constrained to a limited set of operating systems and CPU architectures, hence compatibility with future platforms cannot be guaranteed.

`pyGuidos` is a Python package that provides a direct programmatic interface to the core GWB modules across different operating systems and CPU architectures. All spatial analysis functions are coded in Python using Numba [@lam2015] JIT-compiled kernels for high-performance parallel computation, eliminating all dependencies on external binaries and the IDL runtime. The current version (2.3.2) implements six landscape analysis functions and one temporal change detection function.

# Statement of Need

## From GUI and bash to native Python

Guidos is an acronym for Graphical User Interface for the Description of Image Objects and their Shapes. The [Guidos ecosystem](https://code.europa.eu/jrc-forest/guidos) has evolved through three delivery mechanisms: first, the desktop application GuidosToolbox [@vogt2017], or [GTB](https://forest.jrc.ec.europa.eu/en/activities/lpa/gtb/) (available for x86_64 of Linux, macOS, and Windows) with IDL bundled alongside various precompiled binaries; second, the command-line application GuidosToolbox Workbench [@vogt2022], or [GWB](https://forest.jrc.ec.europa.eu/en/activities/lpa/gwb/) (constrained to Linux x86_64); and third, [pyGuidos](https://code.europa.eu/jrc-forest/guidos/pyguidos) v1, a Python wrapper around GWB bash calls inheriting the Linux-only constraint. These limitations prevented integration into Python-based scientific workflows or automated pipelines on non-Linux systems. More fundamentally, the IDL runtime creates a sustainability risk: GTB and GWB are already incompatible with arm64 versions of MS-Windows and Linux, and this incompatibility may worsen as platforms evolve.

pyGuidos v2.3 addresses these limitations by reimplementing all spatial engines as pure Python functions accelerated with Numba JIT compilation. The sliding-window operations run in parallel across all CPU cores, achieving performance comparable to the original C binaries while remaining fully portable. Input and output operate on individual GeoTIFF files using `rasterio`, aligning with standard Python geospatial conventions.

## Policy context: the EU Nature Restoration Regulation

The European Union Nature Restoration Regulation (NRR), which entered into force in August 2024, legally requires European Union Member States (MS) to report a forest connectivity index based on the Foreground Area Density (FAD) method [@nrr2024]. FAD measures the proportion of forest pixels within a moving window centered on each forest pixel, providing a scale-dependent, per-pixel estimate of local forest connectivity [@riitters2002; @vogt2025frag]. This legal mandate drives immediate demand for auditable, cross-platform software capable of computing FAD at national and continental scales. Because traditional GTB/GWB architectures rely on proprietary runtimes and platform-constrained binaries, they are unsuitable for deployment in automated national reporting pipelines.

pyGuidos fills this gap directly: it provides the exact FAD algorithm adopted by the NRR, implemented in pure Python code that can be deployed on any platform. This makes pyGuidos a tool of direct practical relevance for environmental monitoring, with an audience extending to national forest services and environmental agencies in all 27 MS and beyond.

## Target audience

The package is intended for landscape ecologists building reproducible analysis chains, remote sensing analysts applying GTB metrics to regional or global datasets, and forest monitoring practitioners at institutions such as the FAO, national forest services, and European Commission bodies. To ensure global practitioners can freely report issues and contribute code, public community interactions are managed through an open [GitHub portal](https://github.com/nonpenso/pyguidos).

# State of the Field

GTB is a widely used image analysis platform, with applications in forest monitoring, biodiversity assessment, habitat connectivity, and land cover change detection [@vogt2017; @vogt2022]. Its algorithms have been adopted in official reporting frameworks including the FAO State of the World's Forests [@vogt2019; @FAO2020], the EU MAES ecosystem assessment [@maes2020], national and international forest monitoring programs [@Vogt2025; @ForestEurope2026; @EUROSTAT2022].

pyGuidos is the first and currently only Python interface enabling GTB analyses on all major platforms without external dependencies beyond standard Python packages.

The seven functions currently implemented and their methodological references are:

1. `frag()`: Forest fragmentation via Foreground Area Density (FAD) and Foreground Area Clustering (FAC) methods [@vogt2025frag; @riitters2002]
2. `frag_change()`: Pixel-level fragmentation transition analysis across multiple time periods
3. `landmos()`: Classifies land cover heterogeneity within a tri-polar framework [@riitters2009; @vogt2024lm; @vogt2022lm]
4. `spa()`: Simplified Pattern Analysis, classifying foreground pixels into morphological categories (Core, Edge, Perforation, Islet, Linear) [@soille2009; @soille2022; @vogt2022mspa]
5. `acc()`: Patch accounting, labeling and classifying foreground patches into user-defined area size classes [@vogt2022acc]
6. `rss()`: Restoration Status Summary, computing network coherence indicators including Equivalent Connected Area (ECA) and coherence (COH) [@vogt2022rss; @saura2011]
7. `extract_by_polygon()`: Zonal raster statistics extraction with automated CRS reprojection

The SPA function implements a simplified version of the full Morphological Spatial Pattern Analysis (MSPA) algorithm. While MSPA distinguishes up to 23 mutually exclusive morphological feature classes, SPA provides up to 6 of the most used classes. A full native Python implementation of MSPA is planned for future versions.

# Software Design

## Architecture

pyGuidos is structured as a pure Python package with six main analysis functions (`fragmentation`, `fragmentation_change`, `land_mosaic`, `spa`, `accounting`, `rss`), alongside a regional extraction utility (`extract_by_polygon`). It also includes modular submodules for spatial processing (`engine`), computational utilities (`utils`), and data validation (`checks`). To optimize performance, all computationally intensive operations leverage Numba `@njit` decorated functions with `parallel=True` enabled for multi-core execution.

A key design decision was to operate on individual GeoTIFF files rather than directories. Each analysis function validates inputs, delegates computation to `engine.py`, saves output GeoTIFFs with embedded GTB metadata tags, and returns a nested dictionary containing output paths and statistics. All output GeoTIFFs embed processing parameters in `TIFFTAG_IMAGEDESCRIPTION` using a structured format, allowing standalone statistics functions to recover parameters from output files without requiring the original function call context.

## Native Spatial Engines

Reimplementing GTB modules as native Python functions yields significant architectural benefits: absolute cross-platform portability without bundling compiled binaries, full algorithmic transparency for scientific audit, and multi-core parallelism via Numba's thread-level `prange` that matches optimized C performance. The SPA function combines `scipy` distance transforms, `scikit-image` morphological reconstruction, and a Numba-parallelized assembly step to classify foreground pixels into structural categories.

## Regional Analysis Workflow

A scientifically important workflow involves computing a landscape metric over an entire study area first — to preserve correct moving-window context — and then extracting per-region statistics using `extract_by_polygon()`. This post-processing function clips output rasters to polygons from any vector format supported by `pyogrio`, handling CRS transformations and nodata masking automatically. This two-step approach ensures that metrics near administrative boundaries are not biased by edge artifacts from adjacent areas outside of the administrative boundary.

## Validation and Testing

The package includes a comprehensive test suite covering all functions, using `pytest` with mocking for I/O-intensive operations. The GitLab CI/CD pipeline runs the full suite on every commit with code coverage reporting.

## Repository and Community Access

The core development of pyGuidos is officially hosted by the European Commission on [code.europa.eu](https://code.europa.eu/jrc-forest/guidos/pyguidos). Because access to the institutional forge requires an authorized EU Login, a fully synchronized public mirror is maintained on [GitHub](https://github.com/nonpenso/pyguidos) providing an unrestricted issue tracker and Pull Request workflow for the wider scientific community. A dedicated [online manual](https://pyguidos-d46552.pages.code.europa.eu/usage/index.html) provides information on installation and usage of pyGuidos. The repository includes reproducible example notebooks demonstrating the package's functions using the Copernicus CORINE Land Cover 2000 and 2018 maps over Corsica [@clc2000; @clc2018].

# Research Impact

pyGuidos was developed as the computational backbone of the Global Forest Attribute Dataset (GFAD), presenting global spatial layers and country-level statistics derived from the JRC Global Forest Cover 2020 (GFC2020) map [@caudullo2026]. The GFAD processing chain leveraged pyGuidos to generate global, 100-meter resolution layers for fragmentation, patch accounting, morphology, and restoration status. Processing this ~0.6 terapixel dataset demonstrated operational scalability, completing the complex routine in 6.5 hours utilizing a multi-core cluster infrastructure.

The GFAD dataset is integrated as a core component of the EU Observatory on Deforestation and Forest Degradation (EUFO) and is aligned with the EU Nature Restoration Regulation and international forest reporting frameworks. The underlying processing notebooks are publicly archived to ensure full reproducibility [@caudullo2026b].

JRC is also using pyGuidos as the basis for NRR-compliant training material for EU Member States, with `frag_change()` directly supporting the temporal reporting requirement. This adoption by national monitoring programs represents a direct pathway from research software to policy-relevant operational use.

# AI Usage Disclosure

Generative AI tools assisted in the development process: Anthropic's Claude aided in module refactoring and paper drafting; Google's Gemini supported code generation, algorithm optimization, and documentation structure; and Amazon's Kiro assisted in bug identification and performance tuning for Numba kernels. All tools were utilized for writing unit tests and code reviews.

All AI-generated content was reviewed, corrected, and validated by the authors. Core design decisions, algorithm implementations, scientific methodology, and final form of all outputs are the sole responsibility of the authors.

# Acknowledgements

The authors thank the JRC Land and Climate Unit for institutional support. The GTB/GWB algorithms were developed by Peter Vogt in collaboration with Kurt Riitters (USDA Forest Service) within the framework of a US-EU Collaborative Research Arrangement. The MSPA methodology is based on the work of Soille and Vogt [@soille2009; @soille2022].

# References
