pyGuidos
========

.. |pipeline| image:: https://code.europa.eu/jrc-forest/guidos/pyguidos/badges/main/pipeline.svg?ignore_skipped=true
   :target: https://code.europa.eu/jrc-forest/guidos/pyguidos/-/pipelines
   :alt: Pipeline Status

.. |coverage| image:: https://code.europa.eu/jrc-forest/guidos/pyguidos/badges/main/coverage.svg?job=run_tests
   :target: https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/coverage/index.html
   :alt: Coverage Report

.. |docs| image:: https://img.shields.io/badge/docs-latest-brightgreen.svg
   :target: https://jrc-forest.pages.code.europa.eu/guidos/pyguidos/
   :alt: Documentation Status

.. |pypi| image:: https://img.shields.io/pypi/v/pyguidos.svg
   :target: https://pypi.org/project/pyguidos/
   :alt: PyPI Version

.. |downloads| image:: https://static.pepy.tech/badge/pyguidos
   :target: https://pepy.tech/project/pyguidos
   :alt: Downloads Counter

.. |license| image:: https://img.shields.io/badge/license-EUPL--1.2-orange.svg
   :target: https://joinup.ec.europa.eu/collection/eupl/eupl-text-eupl-12
   :alt: EUPL-1.2 License

|pipeline| |coverage| |docs| |pypi| |downloads| |license|

**pyGuidos** is a Python interface to the main modules of `GuidosToolbox <https://forest.jrc.ec.europa.eu/en/activities/lpa/gtb/>`_ (GTB), 
a scientific software suite developed at the European Commission Joint Research Centre (JRC) for the 
spatial pattern analysis of raster images.

pyGuidos follows the logic of the GTB-based command-line modules `GuidosToolbox Workbench <https://gwbdoc.readthedocs.io/en/latest/>`_ 
providing programmatic access to the main GTB analytical tools, enabling reproducible 
landscape analysis workflows in Python scripts, Jupyter notebooks, and automated pipelines.

.. note::
   This documentation describes **pyGuidos version 2.3.2**. For older versions, 
   please refer to the legacy documentation branch.


Project Infrastructure & Community
----------------------------------

The official upstream development workspace for ``pyguidos`` is hosted on `code.europa.eu <https://code.europa.eu/jrc-forest/guidos/pyguidos>`_, the open-source repository platform of the European Commission. Because this institutional forge restricts issue creation and merge requests to authorized internal accounts (EU Login), public community collaboration is mirrored to GitHub.

.. critical::
   **Bug reports and feature fequests**
   
   To ensure an open, friction-free environment for the global scientific and GIS community, please do not use the code.europa.eu tracker for feedback. Instead, use our public **GitHub Mirror**:
   
   * To report code anomalies or file tickets, visit the public `GitHub Issue Tracker <https://github.com/nonpenso/pyguidos/issues>`_.
   
   * To submit patches or open-source optimizations, please open a Pull Request against our public `GitHub Repository Mirror <https://github.com/nonpenso/pyguidos>`_. Approved code updates will be vetted by the maintainers and integrated into the primary downstream build cycle.


Getting Started
---------------

If you are new to pyGuidos, we recommend following these steps:

1. **Installation**: Follow the :doc:`installation` guide to set up the library.
2. **Interactive Help**: Use the built-in helper to explore available tools:

   .. code-block:: python

      import pyguidos as pg
      pg.info()

3. **User Guide**: Browse the :doc:`usage/index` for detailed examples of Landscape Mosaic, Fragmentation, and other analysis tools.


Citation
--------

If you use **pyGuidos** in your research, please cite both the software implementation 
and the underlying scientific methodology:

.. important::
   **Software Implementation**
     Caudullo G. and Vogt P. (2026). *pyGuidos: A cross-platform Python interface to 
     GuidosToolbox for landscape pattern analysis*. In press.

   **Methodology (GTB)**
     Vogt P. and Riitters K. (2017). *GuidosToolbox: universal digital image object analysis*. 
     European Journal of Remote Sensing, 50, 1, pp. 352-361. 
     doi: `10.1080/22797254.2017.1330650 <https://doi.org/10.1080/22797254.2017.1330650>`_


Authors
-------

* **Giovanni Caudullo** - `giovanni.caudullo@ext.ec.europa.eu <mailto:giovanni.caudullo@ext.ec.europa.eu>`_
* **Peter Vogt** - `peter.vogt@ec.europa.eu <mailto:peter.vogt@ec.europa.eu>`_


License
-------

This project is licensed under the **European Union Public Licence (EUPL-1.2)**. 
The EUPL is a modern, copyleft free software license, providing a legal framework 
compatible with the laws of the European Union Member States.

For more details, see the `official EUPL page <https://joinup.ec.europa.eu/collection/eupl/eupl-text-eupl-12>`_.



.. toctree::
   :maxdepth: 2
   :hidden:
   :caption: Contents:

   installation
   usage/index
   contributing
   changelog