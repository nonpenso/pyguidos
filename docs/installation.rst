Installation
============

Requirements
------------

pyGuidos requires **Python 3.8 or higher** and the following dependencies,
which are installed automatically with pip:

.. list-table::
   :header-rows: 1
   :widths: 20 15 65

   * - Package
     - Minimum Version
     - Purpose
   * - numpy
     - 1.24
     - Array operations
   * - rasterio
     - 1.3
     - GeoTIFF reading and writing
   * - scipy
     - 1.9
     - Connected component labelling
   * - matplotlib
     - 3.5
     - Histogram and ternary plot generation
   * - fiona
     - 1.9
     - Shapefile reading
   * - shapely
     - 2.0
     - Geometry validation and reprojection
   * - pyproj
     - 3.4
     - Coordinate reference system handling
   * - python-ternary
     - 1.0
     - Ternary diagram rendering

Installation
------------

**Standard Installation (not yet available)**
   Install the latest stable release from PyPI:

   .. code-block:: console

       $ pip install pyguidos

**Git Installation**
   To install the latest development version directly from the repository:

   .. code-block:: console

       $ pip install git+https://code.europa.eu/jrc-forest/guidos/pyguidos.git

**Git Editable Installation**
   If you want to use the notebooks and contribute or modify the source code, clone the repository
   and install in editable mode:

   .. code-block:: console

       $ git clone https://code.europa.eu/jrc-forest/guidos/pyguidos.git
       $ cd pyguidos
       $ pip install -e .

   To use the notebooks:
   
   .. code-block:: console

       $ cd notebooks
       $ jupyter notebook

Configuration (Execution Workspace)
-----------------------------------

pyGuidos relies on ``MSPA`` and ``Spatcon``, two high-performance C++ binary files, that require a workspace folder with **execution permissions**.

By default, pyGuidos attempts to use:
1. A ``work/`` folder in your project root (if using a Git clone).
2. A ``pyguidos_work/`` folder in your user home directory.

.. important::
   If your system (e.g., a corporate machine) restricts execution in the Home or AppData folders, you must manually set a "safe" workspace.

Run the built-in setup tool to configure your workspace:

.. code-block:: console

    $ pyguidos-setup

Follow the prompts to provide a path (e.g., ``D:/pyguidos_work``). The tool will test the folder and save the configuration.

Verify Installation
-------------------

After installation, verify everything is working correctly:

.. code-block:: python

    import pyguidos
    print(pyguidos.__version__)
    print(f"Workspace: {pg.WORK_DIR}")

Platform Support
----------------

pyGuidos is tested and supported on:

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - OS
     - Architecture
   * - **Linux**
     - x86_64, ARM64
   * - **macOS**
     - x86_64, ARM64 (Apple Silicon)
   * - **Windows**
     - x86_64

.. note::
    The underlying GuidosToolbox binaries (MSPA, Spatcon) are platform-specific
    and are bundled automatically with the package for all supported platforms.

Troubleshooting
---------------

**Execution Blocked Error**
   If you receive a permission error when running a tool like ``mspa()``, your current ``WORK_DIR`` does not allow binary execution. Run ``pyguidos-setup`` to move the workspace to a non-restricted drive.

**Installation stalls on downloading packages**
   You may be behind a corporate proxy. Try:

   .. code-block:: console

      $ pip install pyguidos --proxy http://login:password@your-proxy:port

**Import error after installation**
   Ensure your ``pip`` matches your active Python version:

   .. code-block:: console

      $ python -m pip show pyguidos