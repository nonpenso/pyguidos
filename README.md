
# pyguidos

[![pipeline status](https://code.europa.eu/jrc-forest/guidos/pyguidos/badges/main/pipeline.svg?ignore_skipped=true)](https://code.europa.eu/jrc-forest/guidos/pyguidos/-/pipelines)
[![coverage report](https://code.europa.eu/jrc-forest/guidos/pyguidos/badges/main/coverage.svg?job=run_tests)](https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/coverage/index.html)
[![docs status](https://img.shields.io/badge/docs-latest-brightgreen.svg)](https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/)
[![PyPI version](https://img.shields.io/pypi/v/pyguidos.svg)](https://pypi.org/project/pyguidos/)
[![Downloads](https://static.pepy.tech/badge/pyguidos)](https://pepy.tech/project/pyguidos)
[![license](https://img.shields.io/badge/license-EUPL--1.2-orange.svg)](https://joinup.ec.europa.eu/collection/eupl/eupl-text-eupl-12)

**A Python interface to the main GuidosToolbox (GTB) modules for spatial pattern analysis**


Overview
========

``pyguidos`` is a Python interface to the main modules of [GuidosToolbox](https://forest.jrc.ec.europa.eu/en/activities/lpa/gtb/) (GTB), a scientific software package for pattern spatial analysis of raster images. This Python module provides programmatic access to the core GTB analytical tools, enabling reproducible landscape analysis workflows in Python scripts, Jupyter notebooks, and automated pipelines.

### Repository Architecture & Contents
The official upstream home of `pyguidos` is hosted on [code.europa.eu](https://code.europa.eu/jrc-forest/guidos/pyguidos), the European Commission's institutional open-source repository platform (powered by a GitLab instance). To foster a completely open community, a synchronized mirror is maintained on [GitHub](https://github.com/nonpenso/pyguidos). 

If you browse the source files directly via either platform, the repository includes:
* **`/pyguidos`**: The core package directory containing the source code, geospatial monitoring algorithms, and Numba-optimized modules (This is what is installed via `pip`).
* **`/docs`**: Source files for automated Sphinx HTML documentation platform.
* **`/notebooks`**: Interactive Jupyter notebooks demonstrating data visualization and workflow examples.
* **`/tests`**: Comprehensive unit and integration test suites validating input parameters and mathematical integrity.

---

Modules
=======

| Function | Description |
|---|---|
| `frag()` | Fragmentation analysis |
| `frag_gray()` | Grayscale Fragmentation analysis |
| `frag_change()` | Fragmentation change |
| `landmos()` | Landscape Mosaic |
| `spa()` | Simplified Pattern Analysis |
| `acc()` | Foreground Patch Size Accounting |
| `rss()` | Restoration Status Summary |
| `extract_by_polygon()` | Extract raster by polygon features |

---

Documentation
=============

Full API documentation is available at https://jrc-forest.pages.code.europa.eu/guidos/pyguidos.


---

Requirements
============

- Python >= 3.10
- numpy >2.0
- rasterio >=1.4
- scipy >=1.15
- scikit-image>=0.26
- matplotlib >=3.10
- pyogrio >=0.10
- geopandas >=1.1
- shapely >=2.0
- python-ternary >=1.0
- numba >0.62
- tbb >=2021.6.0; sys_platform == 'win32'
- intel-openmp; sys_platform == 'linux'

---

Installation
============

### 1. Standard Installation
For general use, install the latest stable version directly via `pip`:
```bash
pip install pyguidos
```

### 2. Development installation
To install the latest development version directly from the official upstream repository on [code.europa.eu](https://code.europa.eu/jrc-forest/guidos/pyguidos) without cloning:
```bash
pip install git+https://code.europa.eu/jrc-forest/guidos/pyguidos.git
```

### 3. Editable Installation (Recommended for Testing)
To run the example notebooks or contribute to the source code, you must clone the repository and install it in "editable" mode. This allows changes in the code to be reflected immediately.
1. Clone the repository
```bash
git clone https://code.europa.eu/jrc-forest/guidos/pyguidos.git
cd pyguidos
```
2. Create and activate a virtual environment using Python.
  - Windows
  ```bash
  py -m venv myvenv
  myvenv\Scripts\activate
  ```
  - Linux/Mac
  ```bash
  python -m venv myvenv
  source myvenv/bin/activate
  ```
3. Install in editable mode with dependencies:
```bash
pip install -e .
```
This links module's source code directly to your Python environment, so any changes you make are immediately reflected without reinstallation.


---

Quick Start
==========

Once installed, you can verify your setup and explore the available tools directly from your Python console or Jupyter Notebook.

```python
import pyguidos as pg

# List all available analytical tools and their descriptions
pg.info()

# Get detailed documentation and methodology links for a specific tool
pg.info('landmos')

# Get full technical specification of a function
help(pg.landmos)
```

---

Usage Examples
==============

```python
import pyguidos as pg

# Fragmentation analysis (FAD)
result = pg.frag(
    in_tiff="my_map.tif",
    method="FAD",
    window_size=27
)

# Fragmentation analysis (FAC with 8-connectivity)
result = pg.frag(
    in_tiff="my_map.tif",
    method="FAC",
    window_size=27,
    connectivity=8
)

# Fragmentation analysis (FED - Foreground Edge Density)
result = pg.frag(
    in_tiff="my_map.tif",
    method="FED",
    window_size=27,
    connectivity=4
)

# Grayscale Fragmentation (tree cover density)
result = pg.frag_gray(
    in_tiff="tree_cover_density.tif",
    method="FAD",
    window_size=27,
    for_threshold=30
)

# Fragmentation change
result = pg.frag_change(
    in_tiff_t1="my_map2018_frag_fad_27.tif",
    in_tiff_t2="my_map2025_frag_fad_27.tif"
)

# Landscape Mosaic
result = pg.landmos(
    in_tiff="my_landcover.tif",
    window_size=33
)

# Simplified Pattern Analysis (SPA)
result = pg.spa(
    in_tiff="my_map.tif",
    edge_width=1,
    classes=6
)

# Foreground Patch Size Accounting
result = pg.acc(
    in_tiff="my_map.tif",
    thresholds=[10, 100, 1000, 10000]
)

# Raster Spatial Statistics
result = pg.rss(in_tiff="my_map.tif")

# Extract raster by polygon
pg.extract_by_polygon(
    shapefile_path="regions.shp",
    geotiff_path="my_map.tif",
    output_dir="output/",
    id_field="NAME"
)
```

Example data and Jupyter notebooks with worked examples are available in the `/notebooks` directory of the [code.europa.eu primary repository](https://code.europa.eu/jrc-forest/guidos/pyguidos).

---

Citation
========

If you use pyGuidos in your research, please cite both the GuidosToolbox
software and this package:

**GuidosToolbox:**
  - Vogt P. and Riitters K. (2017). GuidosToolbox: universal digital image object analysis. European Journal of Remote Sensing, 50, 1, pp. 352-361. doi: [10.1080/22797254.2017.1330650](https://doi.org/10.1080/22797254.2017.1330650)

**pyGuidos:**
  - Caudullo G. and Vogt P. (2026). PyGuidos, A cross-platform Python 
interface to GuidosToolbox for landscape pattern analysis. In press.

### Interactive Citation
You can get the plain-text citations directly in your Python console:

```python
import pyguidos as pg
pg.citation()
```

---

Contributing & Issue Tracking
=============================

The primary development workspace for `pyguidos` is officially hosted on **code.europa.eu**. Because this institutional ecosystem requires an EU Login to post issues or submit code, we manage all public interaction transparently via our public **GitHub mirror** to guarantee open participation.

### Bug Reports & Feature Requests
If you encounter a bug, have an installation issue, or wish to request an analytical feature, please do not use code.europa.eu. Instead, submit a ticket via the public [GitHub Issue Tracker](https://github.com/nonpenso/pyguidos/issues). This includes the peer-review audits associated with journal submissions.

### Code Contributions
Contributions are highly welcome. To submit bug fixes, patches, or optimization code:

1. Fork the public community mirror on [GitHub](https://github.com/nonpenso/pyguidos).
2. Create a new branch for your feature or fix (`git checkout -b feature/your-feature-name`).
3. Make your changes and ensure existing tests pass cleanly (`NUMBA_DISABLE_JIT=1 pytest tests/`).
4. Submit a **Pull Request** on the GitHub mirror. 

For a complete, step-by-step developer walkthrough—including environment isolation setups, code formatting guidelines, and advanced testing parameters—please refer to our comprehensive [Official Development & Contributing Guide](https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/contributing.html).

---

Authors
=======

- **Giovanni Caudullo** -- giovanni.caudullo@ext.ec.europa.eu
- **Peter Vogt** -- peter.vogt@ec.europa.eu

European Commission, Joint Research Centre (JRC)

---

License
=======

This project is licensed under the
[European Union Public Licence v1.2 (EUPL-1.2)](https://joinup.ec.europa.eu/collection/eupl/eupl-text-eupl-12).
See the [LICENSE](LICENSE) file for details.


