
<p align="center">
  <img src="https://code.europa.eu/jrc-forest/guidos/pyguidos/-/raw/main/assets/logo/pyguidos_logo-banner.svg" alt="pyGuidos Logo" width="300"/>
</p>


[![pipeline status](https://code.europa.eu/jrc-forest/guidos/pyguidos/badges/main/pipeline.svg?ignore_skipped=true)](https://code.europa.eu/jrc-forest/guidos/pyguidos/-/pipelines)
[![coverage report](https://code.europa.eu/jrc-forest/guidos/pyguidos/badges/main/coverage.svg?job=run_tests)](https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/coverage/index.html)
[![docs status](https://img.shields.io/badge/docs-latest-brightgreen.svg)](https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/)
[![PyPI version](https://img.shields.io/pypi/v/pyguidos.svg)](https://pypi.org/project/pyguidos/)
[![Downloads](https://static.pepy.tech/badge/pyguidos)](https://pepy.tech/project/pyguidos)
[![license](https://img.shields.io/badge/license-EUPL--1.2-orange.svg)](https://joinup.ec.europa.eu/collection/eupl/eupl-text-eupl-12)

**PyGuidos is a Python interface to the main GuidosToolbox (GTB) modules for spatial pattern analysis**


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
===========

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

Usage Example
=============

This code computes a Forest Area Density (FAD) fragmentation analysis with a window size of 27x27 pixels of Corsica map forest derived from Corine Land Cover 2018. Then it renders both the resulting fragmentation GeoTiff map (using its embedded colormap) and the automatically generated summary chart in Matplotlib.


```python
import pyguidos as pg
from pathlib import Path
import rasterio
import matplotlib.image as mpimg
import matplotlib.pyplot as plt

# Load the Forest Non-Forest map of Corsica
fnf_tiff = pg.DATA_DIR / "CLC2018_corsica_FNF.tif"

# Compute Fragmentation analysis.
fad_corsica = pg.frag(
    in_tiff = fnf_tiff,
    method = "FAD",
    window_size = 27
)

# Read the output map and chart image
frag_tiff = fad_corsica['output paths']['path tif']
with rasterio.open(frag_tiff) as src:
    tiff_data = src.read(1)
frag_chart = fad_corsica['output paths']['path png']
png_img = mpimg.imread(frag_chart)

# Plot the output
fig, axes = plt.subplots(1, 2, figsize=(14, 7))
cmap, norm = pg.utils.get_tif_colormap(frag_tiff)
axes[0].imshow(tiff_data, cmap=cmap, norm=norm, interpolation="none")
axes[0].axis("off")
axes[1].imshow(png_img)
axes[1].axis("off")
plt.tight_layout()
plt.show()
```

![Fragmentation Analysis Output](https://code.europa.eu/jrc-forest/guidos/pyguidos/-/raw/main/assets/frag_example_output.png)


Data and Jupyter notebooks with other examples are available on `/notebooks` directory of the [git repository](https://code.europa.eu/jrc-forest/guidos/pyguidos).

---

Memory Usage and Large Rasters
==============================

pyGuidos loads each input raster fully into memory, so a tool's memory footprint scales with the raster size. Peak usage differs by function, expressed as a multiple of the raw input size *R* (one byte per `uint8` pixel):

| Function | Peak memory | Reason |
|---|---|---|
| `frag`, `frag_gray`, `landmos` | ~4 × *R* | Input up-cast to int16 plus an output buffer |
| `acc`, `rss` | ~6 × *R* | Adds a mask and a connected-component label array |
| `spa` | ~10–15 × *R* | Several morphological masks and distance transforms |
| `frag_change` | ~2 × *R* | Processed block by block via windowed reading |

Physical RAM is not a hard limit: the effective ceiling is RAM plus swap or page-file space. On Windows and macOS, rasters exceeding RAM are transparently paged to disk and still complete, only more slowly; on Linux the same holds if enough swap is configured, otherwise the process may be terminated. Raster size is therefore mainly a matter of processing time rather than capacity, and downsampling or tiling remains an option when faster turnaround is preferred.

---

Citation
========

If you use pyGuidos in your research, please cite both the GuidosToolbox
software and this package:

**GuidosToolbox:**
  - Vogt P. and Riitters K. (2017). GuidosToolbox: universal digital image object analysis. European Journal of Remote Sensing, 50, 1, pp. 352-361. doi: [10.1080/22797254.2017.1330650](https://doi.org/10.1080/22797254.2017.1330650)

**pyGuidos:**
  - Caudullo G. and Vogt P. (2026). PyGuidos, A cross-platform Python 
interface to GuidosToolbox for landscape pattern analysis. In preparation.

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


