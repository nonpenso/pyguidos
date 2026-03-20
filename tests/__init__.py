"""
pyGuidos test suite.

Tests are organised by module:
- test_checks.py  : validators for input parameters and raster values
- test_utils.py   : utility functions for pixel counting, timing and metadata
- test_results.py : result dataclasses for all analysis tools

Run all tests:
    pytest tests/ -v

Run a specific file:
    pytest tests/test_checks.py -v

Run a specific class:
    pytest tests/test_checks.py::TestValidateWsize -v

Run a specific test:
    pytest tests/test_checks.py::TestValidateWsize::test_valid_odd_minimum -v
"""