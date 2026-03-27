def test_package_metadata():
    import pyguidos
    # This hits lines in __init__.py related to version and path initialization
    assert hasattr(pyguidos, "__version__")