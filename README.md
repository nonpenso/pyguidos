
# pyguidos

[![pipeline status](https://code.europa.eu/jrc-forest/guidos/pyguidos/badges/main/pipeline.svg?ignore_skipped=true)](https://code.europa.eu/jrc-forest/guidos/pyguidos/-/pipelines)
[![coverage report](https://code.europa.eu/jrc-forest/guidos/pyguidos/badges/main/coverage.svg?job=run_tests)](https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/coverage/index.html)
[![docs status](https://img.shields.io/badge/docs-latest-brightgreen.svg)](https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/)
![version](https://img.shields.io/badge/version-2.2.0-blue.svg)
[![license](https://img.shields.io/badge/license-EUPL--1.2-orange.svg)](https://joinup.ec.europa.eu/collection/eupl/eupl-text-eupl-12)

**A Python interface to the main GuidosToolbox (GTB) modules for spatial pattern analysis**


Overview
========

``pyguidos`` is a Python interface to the main modules of [GuidosToolbox](https://forest.jrc.ec.europa.eu/en/activities/lpa/gtb/) (GTB), a scientific software package for pattern spatial analysis of raster images. This Python module provides programmatic access to the core GTB analytical tools, enabling reproducible landscape analysis workflows in Python scripts, Jupyter notebooks, and automated pipelines.

### Repository Contents
* **`/docs`**: Documentation files.
* **`/notebooks`**: Jupyter notebooks to use the module and visualise results.
* **`/pyguidos`**: script repository.
* **`/tests`**: Unit and integration tests for analytical tools, input validation (checks), and utility functions.

---

Modules
=======

| Function | Description |
|---|---|
| `frag()` | Fragmentation analysis |
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

- Python >= 3.8
- numpy >2.0
- rasterio >=1.4
- scipy >=1.15
- scikit-image>=0.26
- matplotlib >=3.10
- pyogrio >=0.10
- geopandas >=1.1
- shapely >=2.0
- pyproj >=3.4
- python-ternary >=1.0
- numba >0.62
- tbb >=2021.6.0; sys_platform == 'win32'
- intel-openmp; sys_platform == 'linux'

---

Installation
============

### 1. Standard Installation (not yet available)
For general use, install the latest stable version directly via `pip`:
```bash
pip install pyguidos
```

### 2. Development installation
To install the latest development version directly from the GitLab repository without cloning:
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

# Fragmentation analysis
result = pg.frag(
    in_tiff="my_map.tif",
    method="FAD",
    window_size=27
)

# Landscape Mosaic
result = pg.landmos(
    in_tiff="my_landcover.tif",
    window_size=33
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

Example data and Jupyter notebooks with worked examples are available in the [project repository](https://code.europa.eu/jrc-forest/guidos/pyguidos).

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

Contributing
============

Contributions are welcome. Please follow these steps:

1. Fork the repository on [GitLab](https://code.europa.eu/jrc-forest/guidos/pyguidos)
2. Create a new branch for your feature or fix:
   ```bash
   git checkout -b feature/your-feature-name
   ```
3. Make your changes and ensure existing tests pass:
   ```bash
   pytest tests/
   ```
4. Submit a merge request with a clear description of the changes and
   their motivation.

Please open an issue before starting work on significant changes, to allow discussion of the approach.

For bug reports, please include the pyGuidos version, Python version, operating system, and a minimal reproducible example.

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


