import pytest
import subprocess
import shutil
from pathlib import Path
from pyguidos import engine

def test_engine_binary_failure_handling(monkeypatch):
    """
    Simulates a binary failure that triggers CalledProcessError
    and verifies that the engine calls sys.exit(1).
    """
    
    def mock_run_crash(*args, **kwargs):
        # CalledProcessError to trigger your 'except' block
        raise subprocess.CalledProcessError(
            returncode=1, 
            cmd=args[0], 
            output="Error: Binary crashed", 
            stderr="Detailed error message"
        )

    monkeypatch.setattr(subprocess, "run", mock_run_crash)
    
    with pytest.raises(SystemExit):
        engine.run_mspa("/tmp/mock_dir", "fake_in.tif", 8, 1, 1, 1)

def test_engine_missing_binary(monkeypatch):
    """Tests the error handling when a binary file is not found."""
    
    # 1. Force the 'run' command to fail as if the file is missing
    def mock_run_missing(*args, **kwargs):
        raise FileNotFoundError("mspa_lin64 not found")

    monkeypatch.setattr(subprocess, "run", mock_run_missing)
    
    # 2. Verify that the engine catches this and exits with code 1
    with pytest.raises(SystemExit) as excinfo:
        engine.run_mspa("/tmp/mock_dir", "fake_in.tif", 8, 1, 1, 1)
    
    # 3. Double check the exit code is indeed 1
    assert excinfo.value.code == 1
    
def test_spatcon_binary_failure(monkeypatch, tmp_path):
    """Verifies Spatcon failure triggers SystemExit."""
    
    # Define paths
    fake_source = tmp_path / "fake_source"
    fake_source.touch()
    
    def mock_run_crash(*args, **kwargs):
        raise subprocess.CalledProcessError(
            returncode=1, 
            cmd=args[0], 
            stderr="Spatcon internal memory error"
        )

    monkeypatch.setattr("pyguidos.engine.get_binary_path", lambda x: fake_source)
    # Actually create the file so os.chmod works
    def mock_copy(src, dst):
        Path(dst).touch()
    
    monkeypatch.setattr("shutil.copy2", mock_copy)
    monkeypatch.setattr(subprocess, "run", mock_run_crash)
    
    with pytest.raises(SystemExit) as excinfo:
        engine.run_spatcon(tmp_path)
    assert excinfo.value.code == 1

def test_spatcon_missing_binary(monkeypatch, tmp_path):
    """Verifies Spatcon missing binary triggers SystemExit."""
    
    fake_source = tmp_path / "fake_source"
    fake_source.touch()

    def mock_run_missing(*args, **kwargs):
        raise FileNotFoundError("tool not found")

    monkeypatch.setattr("pyguidos.engine.get_binary_path", lambda x: fake_source)
    
    def mock_copy(src, dst):
        Path(dst).touch()
        
    monkeypatch.setattr("shutil.copy2", mock_copy)
    monkeypatch.setattr(subprocess, "run", mock_run_missing)
    
    with pytest.raises(SystemExit) as excinfo:
        engine.run_spatcon(tmp_path)
    assert excinfo.value.code == 1
    
def test_engine_utils():
    # Hits lines related to OS detection (Line 37-41)
    os_name, arch, is_win = engine.get_os_info()
    assert isinstance(is_win, bool)
    
    # Hits the binary path finder (Line 84-85)
    path = engine.get_binary_path("mspa")
    assert path.exists()

def test_engine_metadata_and_paths():
    """Hits the OS detection and binary path resolution logic."""
    from pyguidos.engine import get_os_info, get_binary_path
    
    os_name, arch, is_win = get_os_info()
    # Normalize to lowercase to match the list
    assert os_name.lower() in ['linux', 'windows', 'darwin']
    
    path = get_binary_path("mspa")
    assert path.exists()