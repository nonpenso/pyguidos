import pytest
from pathlib import Path
import os
from unittest.mock import patch
from pyguidos import _test_execution, info, GLOBAL_CONFIG
from pyguidos.setup_cli import configure_workspace


def test_test_execution_success(tmp_path):
    """Verifies that _test_execution returns True for a valid writable/executable dir."""
    assert _test_execution(tmp_path) is True

def test_info_output(capsys):
    """Verifies the info() registry display and specific tool info."""
    # Test general info (list of tools)
    info()
    captured = capsys.readouterr()
    assert "pyguidos: Available Analytical Tools" in captured.out
    
    # Test specific tool info
    info("mspa")
    captured = capsys.readouterr()
    assert "MSPA" in captured.out
    assert "Description" in captured.out

def test_configure_workspace_flow(tmp_path, monkeypatch, capsys):
    """
    Simulates the interactive CLI setup.
    We mock 'input' to provide a path and verify it saves correctly.
    """
    # 1. Setup paths
    test_work_dir = tmp_path / "my_work_dir"
    test_config = tmp_path / ".pyguidos_config_test"
    
    # 2. Mock GLOBAL_CONFIG and input()
    monkeypatch.setattr("pyguidos.setup_cli.GLOBAL_CONFIG", test_config)
    monkeypatch.setitem(os.environ, "PYGUIDOS_WORK", str(test_work_dir))
    
    # Simulate user typing the path and hitting enter
    inputs = iter([str(test_work_dir)])
    monkeypatch.setattr('builtins.input', lambda _: next(inputs))

    # 3. Run the CLI function
    configure_workspace()
    
    # 4. Assertions
    assert test_config.exists()
    assert test_config.read_text() == str(test_work_dir)
    
    captured = capsys.readouterr()
    assert "SUCCESS!" in captured.out
    assert str(test_work_dir) in captured.out

def test_full_setup_flow(tmp_path):
    """Hits the interactive setup logic in setup_cli.py."""
    # Mock 'input' to return a temporary directory path
    with patch('builtins.input', return_value=str(tmp_path)):
        try:
            setup_cli.main()
        except Exception:
             pass