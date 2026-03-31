
# pyguidos v.1

*Python module to access GuidosToolbox Workbench (GWB)*


Overview
========

``pyguidos`` v.1 is a Python module that provides convenient wrappers for GuidosToolbox Workbench (GWB). It simplifies the execution of GWB executables by encapsulating their required input parameters and handling output. This module is designed for researchers and practitioners working with spatial data, particularly GeoTIFF files for tasks such as restoration status summary, morphological spatial pattern analysis, fragmentation analysis, and more.

This module acts as a bridge, allowing Python users to integrate powerful GWB functionalities into their workflows seamlessly, without directly interacting with the command-line interfaces of the original GWB tools.

### Repository Contents
* **`/data`**: source data to use in the notebook examples.
* **`/notebooks`**: Jupyter notebooks to use the modules and visualise results.
* **`/pyguidos`**: script repository.
* **`/tests`**: Unit and integration tests for the `pyguidos` module.


Features
========

`pyguidos` currently wraps the following GWB modules:
* **`gwb_acc`**: Accounting analysis (object identification and thresholding).
* **`gwb_frag`**: Fragmentation analysis with various methods (FAD, FED, FAC).
* **`gwb_mspa`**: Morphological Spatial Pattern Analysis.
* **`gwb_rss`**: Restoration Status Summary analysis.
* **`gwb_dist`**: Euclidean Distance analysis, optionally with Hypsometric Curve.
* **`gwb_lm`**: Landscape Mosaic analysis (19-class and 103-class versions).
* **`gwb_parc`**: Parcellation analysis for landcover classification.
* **`gwb_rec`**: Recoding of categorical class values in TIFF maps.
* **`gwb_sc`**: Spatial Convolution (SpatCon) for various landscape metrics.
* **`gwb_gsc`**: Gray Spatial Convolution (GraySpatCon) for grayscale image metrics.
* **`gwb_spa`**: Simplified Spatial Pattern Analysis.

All functions handle parameter file generation, command-line execution, and basic error reporting, streamlining the use of the underlying GWB executables.


Prerequisites
=============

Before using `pyguidos` v.1, you *must* have the GWB installed and accessible in your Linux system's `PATH` environment variable. This module does not include the GWB executables themselves.

- Installation instructions: https://ies-ows.jrc.ec.europa.eu/gtb/GWB/GWB_Installation.pdf


Installation
============

### 1. Installation from GitLab
`pyguidos` can be installed directly from its GitLab repository without `/data` and `/notebooks`:

```bash
pip install git+https://code.europa.eu/jrc-forest/guidos/pyguidos.git@v1.0.0
```

### 2. Development installation
To run the examples in the Jupyter notebooks in `/notebooks` (which rely on the files on in the `/data` directory), you must clone the entire repository and then install the module in "editable" mode. This is the recommended approach for development and testing.
1. Clone the repository:
```bash
git clone --branch v1.0.0 --depth 1 https://code.europa.eu/jrc-forest/guidos/pyguidos.git
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


Usage Examples
==============
To use the functions, you first need to import them. For instance, to use ``gwb_mspa``:
```python
from pyguidos import gwb_mspa
from pathlib import Path

# Define input and output directories
input_data_dir = Path("/path/to/your/input_geotiffs") # <<< REPLACE with your actual input data directory
output_results_dir = Path("/path/to/your/output_folder") # <<< REPLACE with your desired output directory (must be empty)

# Example: Run GWB_MSPA with default settings
print("Running GWB_MSPA...")
gwb_mspa(
    input_dir=input_data_dir,
    output_dir=output_results_dir,
    conn_8=True,       # 8-connectivity
    edge_width=1,      # Default edge width
    transition=True,   # Show transition pixels
    int_ext=True,      # Distinguish internal/external features
    disk=False,        # Do not save temporary maps on disk (faster processing)
    stats=True         # Generate summary statistics
)
print("GWB_MSPA processing complete (check output_results_dir for results).")

# You can similarly call other functions like gwb_rss, gwb_acc, etc.
# Refer to the function signatures in the source code or the GWB documentation for parameters.
```


Notebooks
=========
The `/notebooks` directory contains Jupyter notebooks with detailed examples demonstrating how to use `pyguidos` functions, visualize results, and interact with the provided sample data in the `/data` directory.

If you have already followed the "Installation" steps above to clone the repository and installed in editable mode (e.g., `pip install -e ".[test,notebooks]"`), you already have all necessary packages for the notebooks (e.g., jupyterlab, rasterio, matplotlib) installed. You can simply activate your virtual environment and run Jupyter.


