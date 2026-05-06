Installation
============

Requirements
------------

pyGuidos requires **Python >=3.8** and the following dependencies,
which are installed automatically with pip:

.. list-table::
   :header-rows: 1
   :widths: 20 15 65

   * - Package
     - Minimum Version
     - Purpose
   * - numpy
     - >=2.2
     - Array operations
   * - numba
     - >0.63
     - NumPy code into machine code
   * - rasterio
     - >=1.5
     - GeoTIFF reading and writing
   * - scipy
     - >=1.15
     - Connected component labelling
   * - scikit-image
     - >=0.26
     - Image processing     
   * - matplotlib
     - >=3.10
     - Histogram and ternary plot generation
   * - pyogrio
     - >=0.10
     - Vector file reading
   * - geopandas
     - >=1.1
     - Reading vectors as dataframe
   * - shapely
     - >=2.0
     - Geometry validation and reprojection
   * - pyproj
     - >=3.7
     - Coordinate reference system handling
   * - python-ternary
     - >=1.0.8
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


Verify Installation
-------------------

After installation, verify everything is working correctly:

.. code-block:: python

    import pyguidos
    print(pyguidos.__version__)

Troubleshooting
---------------

**Installation stalls on downloading packages**
   You may be behind a corporate proxy. Try:

   .. code-block:: console

      $ pip install pyguidos --proxy http://login:password@your-proxy:port

**Import error after installation**
   Ensure your ``pip`` matches your active Python version:

   .. code-block:: console

      $ python -m pip show pyguidos