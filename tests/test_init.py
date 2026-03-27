import pytest
from pathlib import Path
import pyguidos
import sys

def test_get_workspace_priority_1(tmp_path, monkeypatch):
    """Test Priority 1: Existing Config file."""
    fake_config = tmp_path / ".pyguidos_config"
    fake_work = tmp_path / "custom_work"
    fake_work.mkdir()
    
    # Write the path into the fake config
    fake_config.write_text(str(fake_work), encoding="utf-8")
    
    # Mock the GLOBAL_CONFIG variable itself to point to our fake file
    monkeypatch.setattr(pyguidos, "GLOBAL_CONFIG", fake_config)
    # Mock _test_execution so it always returns True
    monkeypatch.setattr(pyguidos, "_test_execution", lambda x: True)
    
    assert pyguidos.get_workspace() == fake_work

def test_get_workspace_fallback_error(monkeypatch):
    """Test the PermissionError when not in a terminal (CI environment)."""
    # 1. Mock GLOBAL_CONFIG to a non-existent path
    monkeypatch.setattr(pyguidos, "GLOBAL_CONFIG", Path("/nonexistent/config"))
    
    # 2. Mock PROJECT_ROOT so the .git check fails
    monkeypatch.setattr(pyguidos, "PROJECT_ROOT", Path("/nonexistent/root"))
    
    # 3. Mock Path.home so Priority 3 fails
    monkeypatch.setattr(Path, "home", lambda: Path("/nonexistent/home"))
    
    # 4. Force sys.stdin.isatty to False (Simulate CI)
    monkeypatch.setattr(sys.stdin, "isatty", lambda: False)
    
    with pytest.raises(PermissionError, match="pyguidos: Execution is blocked"):
        pyguidos.get_workspace()

def test_get_workspace_interactive_input(monkeypatch, tmp_path):
    """Test the interactive while loop (Priority 4)."""
    valid_path = (tmp_path / "valid_user_path").resolve()
    valid_path.mkdir()

    # Mock environment to reach the input() loop
    monkeypatch.setattr(pyguidos, "GLOBAL_CONFIG", Path("/nonexistent/config"))
    monkeypatch.setattr(pyguidos, "PROJECT_ROOT", Path("/nonexistent/root"))
    monkeypatch.setattr(Path, "home", lambda: Path("/nonexistent/home"))
    
    # Force sys.stdin.isatty to True (Simulate Terminal)
    monkeypatch.setattr(sys.stdin, "isatty", lambda: True)
    
    # Mock input() to provide one empty then one valid path
    responses = iter(["", str(valid_path)])
    monkeypatch.setattr("builtins.input", lambda _: next(responses))
    
    # Mock _test_execution to only accept our valid_path
    monkeypatch.setattr(pyguidos, "_test_execution", lambda x: str(x) == str(valid_path))
    
    # Mock write_text to prevent it from actually writing to disk
    monkeypatch.setattr(Path, "write_text", lambda *args, **kwargs: 10)

    assert pyguidos.get_workspace() == valid_path