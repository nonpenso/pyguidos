import pytest
import pyguidos as pg
from pathlib import Path
import os

# =============================================================================
# Package Metadata & API Tests
# =============================================================================

def test_package_metadata():
    """Verify version and author strings are present."""
    assert hasattr(pg, "__version__")
    assert "Caudullo" in pg.__author__

def test_api_exposure():
    """Ensure key functions are accessible directly via 'pg'."""
    # These should be available via __all__ and imports in __init__
    assert hasattr(pg, "frag")
    assert hasattr(pg, "landmos")
    assert hasattr(pg, "citation")
    assert hasattr(pg, "info")

# =============================================================================
# Environment & Numba Setup Tests
# =============================================================================

def test_numba_config():
    """
    Verify Numba configuration logic.
    We check RELEASE_GIL and the presence of the threading layer.
    """
    from numba import config
    import os
    
    # 1. Verify GIL release is enabled
    assert config.RELEASE_GIL == 1
    
    # 2. Check threading layer (critical for your parallel=True functions)
    # This ensures _setup_numba() actually executed its logic branch
    assert "NUMBA_THREADING_LAYER" in os.environ
    
    # 3. Soft check on CACHE_DIR
    # Instead of asserting it exists (since Numba might keep it as '' 
    # until a function is actually jitted), we check if the variable is 
    # at least a string type.
    assert isinstance(config.CACHE_DIR, str)

def test_numba_caching_mechanism():
    """
    Verify that our custom cache directory is being prioritized
    if it is not empty.
    """
    from numba import config
    # If it is empty, Numba defaults to __pycache__. 
    # If it's NOT empty, it must be a valid path.
    if config.CACHE_DIR != '':
        assert os.path.isdir(config.CACHE_DIR)

def test_threading_layer_set():
    """Verify NUMBA_THREADING_LAYER is present in environment."""
    # Depending on OS, this should have been set by _setup_numba()
    assert "NUMBA_THREADING_LAYER" in os.environ
    # E.g., on Linux it should be 'omp', on Windows 'tbb'
    # We just check it's not empty
    assert len(os.environ["NUMBA_THREADING_LAYER"]) > 0

# =============================================================================
# Info / Registry Tests
# =============================================================================

def test_info_output(capsys):
    """Verify pg.info() prints the registry without error."""
    pg.info()
    captured = capsys.readouterr()
    assert "Available Analytical Tools" in captured.out
    assert "frag" in captured.out
    assert "landmos" in captured.out

def test_info_specific_tool(capsys):
    """Verify pg.info('frag') prints specific tool details."""
    pg.info('frag')
    captured = capsys.readouterr()
    
    # Updated to match the actual uppercase header in your __init__.py
    assert "FRAGMENTATION" in captured.out
    
    # Verify the documentation links are present
    assert "User Guide" in captured.out
    assert "Method Sheet" in captured.out
    
    # Check if the dynamic usage signature was captured correctly
    # (Matches the pg.frag(...) line seen in your error message)
    assert "pg.frag(in_tiff, method, window_size" in captured.out

# =============================================================================
# Paths Verification
# =============================================================================

def test_internal_paths():
    """Ensure internal directory variables point to existing folders."""
    # TEMPL_DIR should point to where frag_templ.txt lives
    assert pg.TEMPL_DIR.exists()
    assert pg.TEMPL_DIR.is_dir()
    
    # Check if our critical template is actually in that directory
    expected_template = pg.TEMPL_DIR / "frag_templ.txt"
    assert expected_template.exists()