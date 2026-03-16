Installation
============

Requirements
------------

pyGuidos requires **Python 3.8 or higher** and the following dependencies,
which are installed automatically with pip:

.. list-table::
   :header-rows: 1

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

Standard Installation
---------------------

Install the latest stable release from PyPI:

.. code-block:: console

    $ pip install pyguidos

Development Installation
------------------------

To install the latest development version directly from the repository:

.. code-block:: console

    $ pip install git+https://code.europa.eu/jrc-forest/guidos/pyguidos

Editable Installation
---------------------

If you want to contribute or modify the source code, clone the repository
and install in editable mode:

.. code-block:: console

    $ git clone https://code.europa.eu/jrc-forest/guidos/pyguidos
    $ cd pyguidos
    $ pip install -e .

Verify Installation
-------------------

After installation, verify everything is working correctly:

.. code-block:: python

    import pyguidos
    print(pyguidos.__version__)

You should see::

    2.0.0

Platform Support
----------------

pyGuidos is tested and supported on:

.. list-table::
   :header-rows: 1

   * - OS
     - Architecture
   * - Linux
     - x86_64, ARM64
   * - macOS
     - x86_64, ARM64 (Apple Silicon)
   * - Windows
     - x86_64

.. note::
    The underlying GuidosToolbox binaries (MSPA, Spatcon) are platform-specific
    and are bundled automatically with the package for all supported platforms.

Troubleshooting
---------------

**Installation stalls on downloading packages**

You may be behind a corporate proxy. Try:

.. code-block:: console

    $ pip install --proxy http://your-proxy:port pyguidos

**Permission error on Linux**

Use a virtual environment or install with the user flag:

.. code-block:: console

    $ pip install --user pyguidos

**Import error after installation**

Make sure you are using the correct Python environment:

.. code-block:: console

    $ which python
    $ pip show pyguidos