Installation
============

Requirements
------------

pyGuidos requires **Python >=3.10** and the following dependencies,
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
   * - python-ternary
     - >=1.0.8
     - Ternary diagram rendering

Installation
------------

**Standard Installation**
   Install the latest stable release from `PyPI <https://pypi.org/project/pyguidos>`_:

   .. code-block:: console

       $ pip install pyguidos

**Development installation**
   To install the latest development version directly from the official upstream repository on **code.europa.eu** without cloning:

   .. code-block:: console

       $ pip install git+https://code.europa.eu/jrc-forest/guidos/pyguidos.git

**Editable Installation (Recommended for Testing)**
   If you want to use the notebooks or modify the source code, clone the repository
   and install in editable mode:

   .. code-block:: console

       $ git clone https://code.europa.eu/jrc-forest/guidos/pyguidos.git
       $ cd pyguidos
       $ pip install -e .

   To use the notebooks:
   
   .. code-block:: console

       $ cd notebooks
       $ jupyter notebook

   .. seealso::
   
      If you installed the package in editable mode with the intention of submitting bug
      fixes, optimizing computational loops, or adding wrappers, please proceed to our
      comprehensive :doc:`contributing` guide. It outlines our dual-forge orchestration
      between ``code.europa.eu`` and GitHub, local testing parameters, and pull request
      submission criteria.


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