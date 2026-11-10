---
title: 'pyGuidos: A Python implementation of Guidos spatial pattern analysis'
tags: 
  - python
  - spatial analysis
  - landscape morphology
  - connectivity
  - fragmentation
authors:
  - name: Giovanni Caudullo
    orcid: 0000-0003-4061-1204
    corresponding: true
    equal-contrib: true
    affiliation: 1	
  - name: Peter Vogt
    orcid: 0000-0002-1030-4492
    equal-contrib: true
    affiliation: 2
  - name: Pierre Soille
    orcid: 0000-0002-8479-9205
    equal-contrib: true
    affiliation: 2	
affiliations:
 - name: Arcadia SIT s.r.l, Vigevano, Italy
   index: 1
 - name: European Commission, Joint Research Centre (JRC), Ispra, Italy
   index: 2
date: 06 November 2026
bibliography: paper.bib
---

# Summary

Forests and other natural habitats are increasingly threatened by fragmentation, degradation, and land-use change. Quantifying these processes requires robust, spatially explicit indicators derived from land cover raster maps. The European Commission's Joint Research Centre (JRC) developed the Guidos ecosystem to address these challenges, starting with GuidosToolbox (GTB), a desktop application, followed by the GuidosToolbox Workbench (GWB), a command-line interface designed for automated processing on Linux x86_64 systems. While powerful, both legacy applications rely on the Interactive Data Language (IDL) and compiled binary wrappers, introducing proprietary licensing constraints, limited operating system compatibility, and restricted CPU architecture support.

To overcome these barriers and provide open-source portability, we present pyGuidos, a cross-platform Python implementation of selected Guidos raster analysis modules. pyGuidos translates core analytical routines into native Python for seamless integration into modern geospatial workflows. The current version of pyGuidos (2.6.4) implements seven pattern analysis functions, six in native Python, while maintaining high-performance execution through an embedded C-module for Morphological Spatial Pattern Analysis. pyGuidos enables scalable raster analysis across diverse operating systems without proprietary runtime dependencies.

# Statement of Need

## Software Context and Relation to Existing Tools

Landscape analysis does not suffer from a lack of analytical tools. Over the past decades, numerous software packages and GIS extensions have been developed to measure landscape patterns, connectivity, and fragmentation. These range from desktop applications like [FRAGSTATS](https://www.fragstats.org/) [@mcgarigal1995], [Circuitscape](https://circuitscape.org/) [@mcrae2008], or [Graphab](https://thema.umlp.fr/productions/software/graphab/en/home.html) [@foltete2021], to R packages like [landscapemetrics](https://r-spatialecology.github.io/landscapemetrics/) [@hesselbarth2019] or [makurhini](https://connectscape.github.io/Makurhini/) [@godinez2025], and Python packages like [pyLandStat](https://github.com/martibosch/pylandstats) [@bosch2019] or [pyLM](https://github.com/ggiuliani/PyLM) [@giuliani2026].

In this context, `pyGuidos` does not introduce new analytical methodologies or duplicate single-purpose metric algorithms. Instead, it addresses the need for programmatic automation of the [Guidos ecosystem](https://code.europa.eu/jrc-forest/guidos). Developed at the European Commission Joint Research Centre (JRC), Guidos (Graphical User Interface for the Description of image Objects and their Shapes) applications consolidate diverse spatial pattern analyses, including landscape mosaic, connectivity and fragmentation. `pyGuidos` bridges these established analytical capabilities into a Python library, enabling researchers to integrate multi-functional spatial analysis directly into automated data pipelines, Jupyter notebooks, and reproducible workflows.

## From GUI and bash to native Python

The Guidos ecosystem has evolved through three primary delivery mechanisms: first, the desktop application [GuidosToolbox](https://forest.jrc.ec.europa.eu/en/activities/lpa/gtb/) [@vogt2017] (GTB), available for x86_64 architectures on Linux, macOS, and MS-Windows, which unified spatial pattern analysis and fragmentation models into an IDL-based GUI (Graphical User Interface); second, the command-line application [GuidosToolbox Workbench](https://forest.jrc.ec.europa.eu/en/activities/lpa/gwb/) [@vogt2022] (GWB), constrained to Linux x86_64; and third, [pyGuidos v1](https://code.europa.eu/jrc-forest/guidos/pyguidos/-/tree/v1.0.0), which acted as a Python wrapper around GWB bash calls to C-compiled binaries and inheriting its Linux-only constraint [@caudullo2026b]. These earlier mechanisms presented sustainability and platform limitations, as the bundled binaries are incompatible with modern architectures like arm64.

`pyGuidos` v2 reimplements the spatial engines as highly optimized Python functions accelerated with Numba JIT compilation. The sliding-window operations run in parallel across all CPU cores, achieving performance comparable to the original C-binaries while remaining fully portable. Input and output operate on individual GeoTIFF files using the `rasterio` Python package, aligning with standard Python geospatial conventions.

## Policy context: the EU Nature Restoration Regulation

The European Union Nature Restoration Regulation (NRR), which entered into force in August 2024, legally requires European Union Member States (MS) to report a forest connectivity index based on the Foreground Area Density (FAD) metric [@nrr2024]. FAD measures the proportion of forest pixels within a moving window centered on each forest pixel, providing a scale-dependent, per-pixel estimate of local forest connectivity [@riitters2002; @vogt2025frag]. This legal mandate drives immediate demand for auditable, cross-platform software capable of computing FAD at national and continental scales. Because traditional GTB/GWB architectures rely on proprietary runtimes and platform-constrained binaries, they are limited for deployment in automated national reporting pipelines.

`pyGuidos` fills this gap directly: it provides the exact FAD algorithm adopted by the NRR, implemented in pure Python code that can be deployed on any platform. This makes `pyGuidos` a tool of direct practical relevance for environmental monitoring, with an audience extending to national forest services and environmental agencies.

## Target audience

The `pyGuidos` package is primarily intended for ecologists and landscape planners to setup reproducible analysis chains, for remote sensing analysts to apply Guidos metrics to regional or global datasets, and for forest monitoring practitioners at institutions such as the FAO, national forest services, and European Commission bodies. While `pyGuidos` was conceived for geospatial applications, the geometry-based nature of its algorithms makes it applicable wherever spatial patterns in 2-dimensional raster data need to be quantified.

# State of the Field

Guidos applications provide tools for image analysis, which have been used in forest monitoring, biodiversity assessment, habitat connectivity, and land cover change detection [@vogt2017; @vogt2022]. Some algorithms have been adopted in official reporting frameworks including the FAO State of the World's Forests [@vogt2019, @FAO2020], the EU MAES ecosystem assessment [@maes2020], as well as in national and international forest monitoring programs [@Vogt2025, @ForestEurope2026, @EUROSTAT2022].
`pyGuidos` is the first and currently only Python interface enabling these analyses on all major platforms without external dependencies beyond standard Python packages.
The nine functions currently implemented and their methodological references are:
1. `frag()`: Forest fragmentation via Foreground Area Density (FAD), Foreground Area Clustering (FAC) and Foreground Edge Density (FED) metrics for binary input maps [@vogt2025frag; @riitters2002];
2. `frag_gray()`: Forest fragmentation via FAD, FAC and FED metrics with continuous-value input maps [@riitters2023; @vogt2024];
3. `frag_change()`: Pixel-level fragmentation change analysis between two points in time;
4. `landmos()`: Land cover classification within a tri-polar framework [@riitters2009; @vogt2024lm; @vogt2022lm];
5. `mspa()`: Morphological Spatial Pattern Analysis, segmenting foreground pixels into up to 23 mutually exclusive morphological classes [@soille2009; @soille2022; @vogt2022mspa];
6. `spa()`: Simplified Pattern Analysis, a streamlined variant of MSPA classifying foreground pixels into up to 6 of the most used morphological categories (Core, CoreOpening, Edge, Perforation, Islet, Margin) [@vogt2022mspa];
7. `acc()`: Patch accounting, labeling and classifying foreground patches into user-defined area size classes [@vogt2022acc];
8. `rss()`: Restoration Status Summary, computing network indicators including Equivalent Connected Area (ECA) and coherence (COH) [@vogt2022rss; @saura2011];
9. `extract_by_polygon()`: Zonal raster statistics extraction.
The `spa()` function is a native Python approximation of MSPA producing the most common classes, whereas `mspa()` delivers the complete set of classes including those related to pattern connectivity.

# Software Design

## Architecture

`pyGuidos` is structured as a Python package with eight main analysis functions (fragmentation, fragmentation gray, fragmentation change, land mosaic, mspa, spa, accounting, rss), which are complemented by a regional extraction utility (extract by polygon). It also includes modular submodules for spatial processing (engine), computational utilities (utils), and data validation (checks). To optimize performance, the Python-native operations leverage `Numba @njit` decorated functions with `parallel=True` enabled for multi-core execution.

Each analysis function validates inputs, delegates computation to engine.py, saves output GeoTIFFs with embedded Guidos metadata tags, and returns a nested dictionary containing output paths and statistics. All output GeoTIFFs embed processing parameters in `TIFFTAG_IMAGEDESCRIPTION` using a structured format, allowing GTB/GWB or standalone statistics functions to recover parameters from output files without requiring the original function call context. 

## Native Spatial Engines

Reimplementing most Guidos modules as native Python functions yields significant architectural benefits: cross-platform portability without external binaries, full algorithmic transparency for scientific audit, and multi-core parallelism via `Numba`'s thread-level prange that matches optimized C performance. Currently, MSPA is the single exception to the pure-Python design. Its full replication relies on specific segmentation steps (e.g., anchored-skeletonization [@ranwez-soille2002] and Euclidean geodesic distance function [@soille2004]) whose faithful re-implementation in Python proved impractical and prone to diverging from reference outputs produced by the open-source Morphological Image Analysis C Library [miallib]{https://github.com/ec-jrc/jeolib-miallib} [@soille2022]. To bridge this gap, `pyGuidos` embeds a dependency-free subset of `miallib` 2.0.0 (November 2026), which introduces MSPA v2.4. Cross-platform portability is ensured through pre-compiled binary wheels across major platforms (Linux, macOS, and Windows for x86_64 and arm64) with source fallbacks, while a future pure Python MSPA reimplementation is planned to eliminate binary dependencies entirely.

## Regional Analysis Workflow

To avoid edge-artifact bias near the boundaries of the study area, landscape metrics should be computed across the enlarged study area first. The post-processing function `extract_by_polygon()` clips the resulting metric raster into smaller sub-regions using vector geometries supported by `pyogrio`. From these clipped sub-rasters, summary statistics (e.g., class distributions, area proportions) can be calculated enabling accurate, object-based regional analysis without compromising spatial continuity.

## Memory Usage and Large Rasters

`pyGuidos` loads each raster fully into memory, so its footprint scales with raster size. Peak usage, as a multiple of the raw input size *R* (one byte per `uint8` pixel), is about 4× for `frag()`, `frag_gray()`, and `landmos()`, 6× for `acc()` and `rss()`, and 10–15× for `spa()`, and up to 25x for `mspa()`, the heaviest, due to its multiple full-size morphological buffers in the C engine. `frag_change()` is lightest at ~2×, as it works block by block.

Physical RAM is not a hard limit: the effective ceiling is RAM plus swap or page-file space. Depending on the OS in use, rasters exceeding physical RAM may be transparently paged to disk and still complete processing, though at the expense of a significant performance penalty due to disk I/O. macOS comes preconfigured with compressed RAM, which transparently expands usable memory. A similar compressed-memory approach is supported natively in Linux via the standard `zram` kernel module. Moreover, configuring disk partitions or external drives as swap space provides additional virtual memory. Raster size is therefore primarily a trade-off with processing time and I/O bandwidth rather than a strict capacity ceiling.

## Validation and Testing

The package includes a comprehensive test suite covering all functions, using `pytest` with mocking for I/O-intensive operations. The GitLab CI/CD pipeline runs the full suite on every commit with code coverage reporting.

## Repository and Community Access

The core development of `pyGuidos` is officially hosted by the European Commission on [code.europa.eu](https://code.europa.eu/jrc-forest/guidos/pyguidos). Access requires creating a free account for [EU Login](https://trusted-digital-identity.europa.eu/), the European Commission's sign-in service.  A fully synchronized public mirror is maintained on [GitHub](https://github.com/nonpenso/pyguidos). This mirror offers an issue tracker and *Pull Request* workflow for the wider scientific community. A dedicated [online manual](https://pyguidos-d46552.pages.code.europa.eu/usage/index.html) provides information on installation and usage of `pyGuidos`. The repository includes reproducible example notebooks demonstrating the pyGuidos functions using the Copernicus CORINE Land Cover 2000 and 2018 maps over Corsica [@clc2000; @clc2018].

# Research Impact

`pyGuidos` was developed as the computational backbone of the Global Forest Attribute Dataset (GFAD), presenting global spatial layers and country-level statistics derived from the JRC Global Forest Cover 2020 (GFC2020) map [@caudullo2026]. The GFAD processing chain leveraged `pyGuidos` to generate global, 100-meter resolution layers for fragmentation, patch accounting, morphology, and restoration status. Processing this ~0.6 terapixel dataset demonstrated operational scalability, completing the complex routine in 6.5 hours utilizing a multi-core cluster infrastructure with 8 TB RAM and 170 CPUs.

The GFAD dataset is integrated as a core component of the [EU Observatory on Deforestation and Forest Degradation]( https://forest-observatory.ec.europa.eu/forest) (EUFO) and is aligned with the EU Nature Restoration Regulation and international forest reporting frameworks. The underlying processing notebooks are publicly archived to ensure full reproducibility [@caudullo2026b].

The package can also support EU Member States in meeting NRR reporting requirements, with `frag()` providing the standard fragmentation analysis and `frag_change()` the temporal change reporting, offering a direct pathway from research software to policy-relevant operational use.

# AI Usage Disclosure

Generative AI tools (Anthropic's Claude, Google's Gemini, and Amazon's Kiro) assisted with `pyGuidos` code generation, refactoring, `Numba` kernel tuning, unit tests, documentation, and paper drafting. All AI-generated content was reviewed and validated by the authors, who remain solely responsible for the design decisions, algorithm implementations, scientific methodology, and final outputs.

# Acknowledgements

The GTB/GWB algorithms were developed by Peter Vogt in collaboration with Kurt Riitters (USDA Forest Service) within the framework of a US-EU Collaborative Research Arrangement.

# References
