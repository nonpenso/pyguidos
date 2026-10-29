
<p align="center">
  <img src="https://code.europa.eu/jrc-forest/guidos/pyguidos/-/raw/main/assets/logo/pyguidos_logo-banner.svg" alt="pyGuidos Logo" width="300"/>
</p>


[![pipeline status](https://code.europa.eu/jrc-forest/guidos/pyguidos/badges/main/pipeline.svg?ignore_skipped=true)](https://code.europa.eu/jrc-forest/guidos/pyguidos/-/pipelines)
[![coverage report](https://code.europa.eu/jrc-forest/guidos/pyguidos/badges/main/coverage.svg?job=run_tests)](https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/coverage/index.html)
[![docs status](https://img.shields.io/badge/docs-latest-brightgreen.svg)](https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/)
[![PyPI version](https://img.shields.io/pypi/v/pyguidos.svg)](https://pypi.org/project/pyguidos/)
[![Downloads](https://static.pepy.tech/badge/pyguidos)](https://pepy.tech/project/pyguidos)
[![license](https://img.shields.io/badge/license-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0.html)
[![JOSS status](https://joss.theoj.org/papers/8a3fdc720f5b0c3f8392d33405787d79/status.svg)](https://joss.theoj.org/papers/8a3fdc720f5b0c3f8392d33405787d79)

**PyGuidos is a Python implementation of the main Guidos tools for spatial pattern analysis**


Overview
========

`pyGuidos` is a Python implementation of the core spatial pattern analysis tools found in the [GuidosToolbox](https://forest.jrc.ec.europa.eu/en/activities/lpa/gtb/) (GTB) desktop application and the [GuidosToolbox Workbench](https://forest.jrc.ec.europa.eu/en/activities/lpa/gwb/) (GWB) command-line interface. By translating key analytical tools into native Python, `pyGuidos` removes proprietary runtime restrictions and platform limits, enabling seamless and reproducible landscape analysis workflows across Python scripts, Jupyter notebooks, and automated pipelines.


### Repository Architecture & Contents
The official upstream home of `pyguidos` is hosted on [code.europa.eu](https://code.europa.eu/jrc-forest/guidos/pyguidos), the European Commission's institutional open-source repository platform (powered by a GitLab instance). To foster a completely open community, a synchronized mirror is maintained on [GitHub](https://github.com/nonpenso/pyguidos). 

If you browse the source files directly via either platform, the repository includes:
* **`/pyguidos`**: The core package directory containing the source code, geospatial monitoring algorithms, and Numba-optimized modules (This is what is installed via `pip`).
* **`/docs`**: Source files for automated Sphinx HTML documentation platform.
* **`/notebooks`**: Interactive Jupyter notebooks demonstrating data visualization and workflow examples.
* **`/tests`**: Comprehensive unit and integration test suites validating input parameters and mathematical integrity.
* **`/assets`**: Static image assets (project logo, example of output figure) referenced in the README file.

---

Available tools
===============

The python module `pyguidos` provides the following funtions.

| Function | Description |
|---|---|
| `mspa()` | Morphological Spatial Pattern Analysis |
| `spa()` | Simplified Pattern Analysis |
| `frag()` | Fragmentation analysis |
| `frag_gray()` | Grayscale Fragmentation analysis |
| `frag_change()` | Fragmentation change |
| `landmos()` | Landscape Mosaic |
| `acc()` | Foreground Patch Size Accounting |
| `rss()` | Restoration Status Summary |
| `extract_by_polygon()` | Extract raster by polygon features |

---

Documentation
=============

Full API documentation is available at https://jrc-forest.pages.code.europa.eu/guidos/pyguidos.

---


Statement of Need
=================

Landscape analysis benefits from a wide array of existing software packages, desktop applications, and libraries, ranging from classic tools like FRAGSTATS and Circuitscape to various R packages and Python modules. However, a persistent challenge has been the programmatic automation of the established [GuidosToolbox](https://forest.jrc.ec.europa.eu/en/activities/lpa/gtb/) desktop application developed at the European Commission Joint Research Centre (JRC). Historically, Guidos capabilities were confined first to the desktop application and later to the [GuidosToolbox Workbench](https://forest.jrc.ec.europa.eu/en/activities/lpa/gwb/), which provides command-line access to the core Guidos tools but is strictly constrained to Linux systems, presenting architectural limitations that hindered integration into modern workflows. `pyGuidos` addresses this gap by reimplementing the core Guidos tools for spatial pattern analysis into a high-performance Python library. Accelerated with Numba JIT compilation and parallelized across CPU cores, it matches the performance of compiled C-binaries while offering complete cross-platform portability and seamless integration into automated data pipelines and Jupyter notebooks.

This technical evolution carries direct policy relevance, particularly in the context of the European Union Nature Restoration Regulation (NRR). The regulation legally requires EU Member States to report a forest connectivity index based on the Foreground Area Density (FAD) metric, driving an immediate demand for auditable and scalable software capable of computing FAD at national and continental scales. Because legacy Guidos architectures relied on proprietary runtimes and platform-restricted binaries, they were poorly suited for automated national reporting pipelines. `pyGuidos` fills this gap by implementing the exact FAD algorithm adopted for NRR compliance in pure, accessible Python code that can be deployed on any modern infrastructure.

Consequently, the package is designed to support a broad audience of landscape ecologists, remote sensing analysts, and forest monitoring practitioners across national forest services, international organizations, and European Commission bodies who require building reproducible analysis chains applying GuidoToolbox metrics to regional or global datasets for environmental policy reporting and spatial assessment.


---

Requirements
============

- Python >= 3.10
- numpy >2.0
- rasterio >=1.4
- scipy >=1.15
- scikit-image>=0.24
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

For more in-depth use cases, a complete suite of step-by-step example notebooks is available on `/notebooks` directory of the [git repository](https://code.europa.eu/jrc-forest/guidos/pyguidos/notebooks) and also on the [Examples](https://pyguidos-d46552.pages.code.europa.eu/examples/index.html#examples) section of the online documentation. These workflows guide you from raw data acquisition to final results generation.
- [01. Getting Started](https://pyguidos-d46552.pages.code.europa.eu/notebooks/01_getting_started.html): Basic raster handling, metadata extraction, and setup.
- [02. Landscape Mosaic](https://pyguidos-d46552.pages.code.europa.eu/notebooks/02_landscape_mosaic.html): Computing core landscape mosaic metrics.
- [03. Fragmentation Change](https://pyguidos-d46552.pages.code.europa.eu/notebooks/03_fragmentation_change.html): Analyzing multi-temporal fragmentation changes.
- [04. Regional Analysis](https://pyguidos-d46552.pages.code.europa.eu/notebooks/04_regional_analysis.html): Extracting metrics by administrative subdivisions.
- [05. Grayscale Fragmentation](https://pyguidos-d46552.pages.code.europa.eu/notebooks/05_grayscale_fragmentation.html): Processing continuous raster inputs.
- [06. Morphology MSPA & SPA](https://pyguidos-d46552.pages.code.europa.eu/notebooks/06_morphology_mspa_spa.html): Executing morphological pattern analysis.

You can browse these tutorials online or click the “Open in Colab” badge at the top of any notebook page to execute them interactively in your browser.

---

Memory Usage and Large Rasters
==============================

pyGuidos loads each input raster fully into memory, so a tool's memory footprint scales with the raster size. Peak usage differs by function, expressed as a multiple of the raw input size *R* (one byte per `uint8` pixel):

| Function | Peak memory | Reason |
|---|---|---|
| `mspa` | ~25 × *R* | Multiple full-size morphological buffers in the C engine also depending on edge width |
| `spa` | ~10–15 × *R* | Several full-size morphological masks and distance transforms  depending on edge width |
| `frag`, `frag_gray`, `landmos` | ~3 × *R* | Input up-cast to int16 plus an output buffer |
| `acc`, `rss` | ~4-5 × *R* | Adds a mask and a connected-component label array |
| `frag_change` | ~2 × *R* | Processed block by block via windowed reading |

Physical RAM is not a hard limit: the effective ceiling is RAM plus swap or page-file space. Depending on the OS in use, rasters exceeding physical RAM may be transparently paged to disk and still complete processing, though at the expense of a significant performance penalty due to disk I/O. macOS comes preconfigured with compressed RAM, which transparently expands usable memory. A similar compressed-memory approach is supported natively in Linux via the standard `zram` kernel module. Moreover, configuring disk partitions or external drives as swap space provides additional virtual memory. Raster size is therefore primarily a trade-off with processing time and I/O bandwidth rather than a strict capacity ceiling.

---

Citation
========

If you use pyGuidos in your research, please cite both the GuidosToolbox
software and this package:

Software implementation: **pyGuidos**
  - Caudullo G., Vogt P., Soille P. (2026). pyGuidos: A Python implementation of Guidos spatial pattern analysis. JOSS in review.

Methodology: **GuidosToolbox**
  - Vogt P., Riitters K. (2017). GuidosToolbox: universal digital image object analysis. European Journal of Remote Sensing, 50, 1, pp. 352-361. doi: [10.1080/22797254.2017.1330650](https://doi.org/10.1080/22797254.2017.1330650)


### Interactive Citation
You can get the plain-text citations directly in your Python console:

```python
import pyguidos as pg
pg.citation()
```

---

Contributing & Issue Tracking
=============================

The primary development workspace for `pyguidos` is officially hosted on [code.europa.eu](https://code.europa.eu/jrc-forest/guidos/pyguidos). Because this institutional ecosystem requires an EU Login to post issues or submit code, we manage all public interaction transparently via our public [GitHub mirror](https://github.com/nonpenso/pyguidos) to guarantee open participation.

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
- **Pierre Soille** -- pierre.soille@ec.europa.eu

European Commission, Joint Research Centre (JRC)

---

License
=======

pyGuidos is distributed under the
[GNU General Public License v3 (GPLv3)](https://www.gnu.org/licenses/gpl-3.0.html).
This is because the package bundles the GPLv3-licensed
[`miallib`](https://github.com/ec-jrc/jeolib-miallib) MSPA engine of Soille and
Vogt, which is compiled into pyGuidos.

The original pyGuidos code (everything except the vendored `miallib` sources
under `pyguidos/_mspa/miallib/`) remains available under the
[European Union Public Licence v1.2 (EUPL-1.2)](https://joinup.ec.europa.eu/collection/eupl/eupl-text-eupl-12),
which is compatible with the GPLv3.

See the [LICENSE](LICENSE) file for the full license text and the
[NOTICE](NOTICE) file for attribution details.


