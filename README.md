
# pyguidos


*Python module to access GuidosToolbox Workbench (GWB)*

Overview
========

``pyguidos`` is a Python module that provides convenient wrappers for GuidosToolbox Workbench (GWB). It simplifies the execution of GWB executables by encapsulating their required input parameters and handling output. This module is designed for researchers and practitioners working with spatial data, particularly GeoTIFF files for tasks such as restoration status summary, morphological spatial pattern analysis, fragmentation analysis, and more.

This module acts as a bridge, allowing Python users to integrate powerful GWB functionalities into their workflows seamlessly, without directly interacting with the command-line interfaces of the original GWB tools.


Features
========

`pyguidos` currently wraps the following GWB modules:

* **`gwb_rss`**: Restoration Status Summary analysis.

* **`gwb_mspa`**: Morphological Spatial Pattern Analysis.

* **`gwb_acc`**: Accounting analysis (object identification and thresholding).

* **`gwb_frag`**: Fragmentation analysis with various methods (FAD, FED, FAC).

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

Before using `pyguidos`, you *must* have the GWB installed and accessible in your Linux system's `PATH` environment variable. This module does not include the GWB executables themselves.

- Installation instructions: https://ies-ows.jrc.ec.europa.eu/gtb/GWB/GWB_Installation.pdf


Installation
============

`pyguidos` can be installed directly from its GitLab repository:

```bash
    pip install git+https://code.europa.eu/jrc-forest/guidos/pyguidos.git
```

Usage Examples
==============
To use the functions, you first need to import them. For instance, to use ``gwb_mspa``:
```python
# Define input and output directories
input_data_dir = Path("/path/to/your/input_geotiffs") # <<< REPLACE with your actual input data directory
output_results_dir = Path("/path/to/your/output_folder") # <<< REPLACE with your desired output directory

# Ensure the output directory exists and is empty (as required by GWB modules)
output_results_dir.mkdir(parents=True, exist_ok=True)
# You might want to add logic here to check if it's empty or clear it carefully

# Example: Run GWB_MSPA with default settings
print("Running GWB_MSPA...")
gwb_mspa(
    input_dir=input_data_dir,
    output_dir=output_results_dir,
    conn_8=True,       # 8-connectivity
    edge_width=1,      # Default edge width
    transition=True,   # Show transition pixels
    int_ext=True,      # Distinguish internal/external features
    save_ram=False,    # Do not save RAM (faster processing)
    stats=True         # Generate summary statistics
)
print("GWB_MSPA processing complete (check output_results_dir for results).")

# You can similarly call other functions like gwb_rss, gwb_acc, etc.
# Refer to the function signatures in the source code or the GWB documentation for parameters.
```

Advanced Usage & Demos
======================
For more detailed examples, including how to prepare input data, interpret outputs, and use various parameters for each GWB function, please refer to:

- ``/examples`` directory: contains Python scripts demonstrating specific use cases with sample data.
- ``/notebooks`` directory: contains Jupyter notebooks for interactive exploration, data preparation, and workflow demonstrations.


License
=======






