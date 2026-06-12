Contributing to pyGuidos
========================

Thank you for your interest in contributing to ``pyguidos``! We welcome contributions from the scientific and geospatial software communities, including bug fixes, optimization enhancements, documentation updates, and new analytical wrappers.

Because this package is developed under the umbrella of the European Commission Joint Research Centre (JRC), our workflows bridge institutional requirements with open-science community standards.

.. clear-break

Development Infrastructure
--------------------------

The project uses a dual-forge system to balance institutional integrity with open accessibility:

1. **Official Upstream Workspace (``code.europa.eu``):** This is the authoritative repository platform of the European Commission. The core CI/CD pipelines, automated testing, documentation generation, and official PyPI package distributions happen here. Account registration is restricted to internal EU personnel.
2. **Community Portal Mirror (GitHub):** A fully transparent, public mirror is hosted at `GitHub <https://github.com/nonpenso/pyguidos>`_. This portal serves as our public-facing front desk. **All external code contributions, bug tracking, and journal peer-review communications happen exclusively through this mirror.**

---

Reporting Bugs and Feature Requests
-----------------------------------

If you find a bug, encounter calculation anomalies, or wish to propose an feature enhancement:

* **Do not** attempt to register an account or file an issue on ``code.europa.eu``.
* **Do** visit the public `GitHub Issue Tracker <https://github.com/nonpenso/pyguidos/issues>`_ and open a new ticket.
* Please provide a minimal reproducible example (MRE), along with your OS version, Python version, and ``pyguidos`` version string.

---

Development Workflow
--------------------

If you wish to modify the code or fix an issue, please use the following step-by-step framework:

1. Fork & Clone
~~~~~~~~~~~~~~~
Fork the public community mirror on GitHub, and then clone your fork locally:

.. code-block:: bash

   git clone https://github.com/YOUR-USERNAME/pyguidos.git
   cd pyguidos

2. Set Up the Environment
~~~~~~~~~~~~~~~~~~~~~~~~~
Create an isolated environment, install the package in editable mode (``-e``), and include the testing and compilation suites:

.. code-block:: bash

   python -m venv venv
   source venv/bin/activate  # Or venv\Scripts\activate on Windows
   pip install --upgrade pip
   pip install -e .[test,notebooks]

3. Implementing Changes
~~~~~~~~~~~~~~~~~~~~~~~
Always create a descriptive feature branch before modifying code. Do not work directly on the ``main`` branch:

.. code-block:: bash

   git checkout -b feature/your-meaningful-branch-name

Ensure your code style remains clean, structured, and properly documents functions with clear docstrings matching our existing Sphinx conventions.

4. Testing Code Integrity
~~~~~~~~~~~~~~~~~~~~~~~~~
Before submitting code, you must verify that all unit and performance tests pass completely. Because ``pyguidos`` utilizes high-performance Numba architectures, we recommend turning off JIT compilation during testing to ensure standard line-execution profiling works flawlessly:

.. code-block:: bash

   NUMBA_DISABLE_JIT=1 pytest tests/ --cov=pyguidos --cov-report term-missing

Your contribution must not degrade existing test coverage. If you are implementing a new feature, you are expected to include corresponding validation tests in the ``tests/`` folder.

5. Submitting a Pull Request
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Once your branch passes all checks, push your commits to your GitHub fork and open a **Pull Request (PR)** against the ``main`` branch of the target mirror repository.

* A project maintainer will review your modifications.
* Approved changes will be audited and brought into the ``code.europa.eu`` internal ecosystem.
* Your contributions will then be officially packed and credited into the next tagged software release cycle on PyPI!