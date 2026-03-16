
# pyguidos

*Python interface to GuidosToolbox (GTB) for spatial pattern analysis**


Overview
========

``pyguidos`` is a Python interface to [GuidosToolbox](https://forest.jrc.ec.europa.eu/en/activities/lpa/gtb/) (GTB), a scientific software package for pattern spatial analysis of raster images. This Python module provides programmatic access to the core GTB analytical tools, enabling reproducible landscape analysis workflows in Python scripts, Jupyter notebooks, and automated pipelines.

### Repository Contents
* **`/notebooks`**: Jupyter notebooks to use the module and visualise results.
* **`/pyguidos`**: script repository.
* **`/tests`**: Unit and integration tests for the `pyguidos` module.

---

Tools
=====

| Function | Description |
|---|---|
| `mspa()` | Morphological Spatial Pattern Analysis |
| `frag()` | Fragmentation analysis (FAD/FOS) |
| `landmos()` | Landscape Mosaic |
| `acc()` | Foreground Patch Size Accounting |
| `rss()` | Raster Spatial Statistics |
| `extract_by_polygon()` | Extract raster by polygon features |

---

Requirements
============

- Python >= 3.8
- numpy
- rasterio
- scipy
- matplotlib
- fiona
- shapely
- pyproj
- python-ternary

---

Installation
============

`pyguidos` can be installed directly from its GitLab repository without `/notebooks`:

```bash
pip install git+https://code.europa.eu/jrc-forest/guidos/pyguidos.git
```

To run the examples in the Jupyter notebooks in `/notebooks`, you must clone the entire repository and then install the module in "editable" mode. This is the recommended approach for development and testing.
1. Clone the repository:
```bash
git clone https://code.europa.eu/jrc-forest/guidos/pyguidos.git
cd pyguidos
```
2. Create and activate a virtual environment (highly recommended):
```bash
python3 -m venv myvenv
source myvenv/bin/activate
```
3. Install the module in editable mode, including development dependencies (for testing and notebooks):
```bash
pip install -e ".[test,notebooks]"
```
This links module's source code directly to your Python environment, so any changes you make are immediately reflected without reinstallation.

---

Usage Examples
==============

```python
import pyguidos as pg

# Morphological Spatial Pattern Analysis
result = pg.mspa(
    in_tiff="my_map.tif",
    edge_width=1,
    connectivity=8
)
print(result.stats)

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

Documentation
=============

Full API documentation is available at [LINK_TO_DOCS].

---

Citation
========

If you use pyGuidos in your research, please cite both the GuidosToolbox
software and this package:

**GuidosToolbox:**
> Vogt P. and Riitters K. (2017). GuidosToolbox: universal digital image object analysis. European Journal of Remote Sensing, 50, 1, pp. 352-361. doi: [10.1080/22797254.2017.1330650](https://doi.org/10.1080/22797254.2017.1330650)

**pyGuidos:**
> Caudullo G. and Vogt P. (2026). pyGuidos: Python interface to GuidosToolbox. In press.

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

---

Acknowledgements
================

pyGuidos is built on top of the GuidosToolbox binaries developed at the European Commission Joint Research Centre. We acknowledge all contributors to the GTB project and the open source Python ecosystem that makes this work possible.


