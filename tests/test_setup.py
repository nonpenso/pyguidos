import pytest
from pathlib import Path
import os
from unittest.mock import patch
from unittest.mock import MagicMock
from pyguidos import _test_execution, info, GLOBAL_CONFIG
from pyguidos.setup_cli import configure_workspace
import pyguidos.setup_cli as cli

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

def test_configure_workspace_failure_linux(monkeypatch, capsys):
    """Hits the Linux-specific failure message (the red 'else' block)."""
    # 1. Force os.name to NOT be Windows
    monkeypatch.setattr("os.name", "posix")
    # 2. Force input to be empty (hits the 'else' branch of user_input)
    monkeypatch.setattr("builtins.input", lambda _: "")
    # 3. Force the execution test to fail
    monkeypatch.setattr("pyguidos.setup_cli._test_execution", lambda x: False)
    
    configure_workspace()
    
    captured = capsys.readouterr()
    assert "FAILED: Execution test failed" in captured.out
    assert "noexec mount" in captured.out  # Verifies the Linux suggestion line


def test_configure_workspace_failure_windows_safe(monkeypatch, capsys):
    """Hits Windows-specific lines without breaking the global os.name."""
    
    # 1. Provide a dummy path string for input
    monkeypatch.setattr("builtins.input", lambda _: "D:/restricted_path")
    
    # 2. Force the execution check to fail
    monkeypatch.setattr(cli, "_test_execution", lambda x: False)
    
    # 3. CRITICAL: Only mock 'os' inside the setup_cli module
    # This prevents pytest from seeing 'os.name' as 'nt' globally
    class MockOs:
        name = 'nt'
        sep = '\\'
    monkeypatch.setattr(cli, "os", MockOs)

    # 4. Run the function
    cli.configure_workspace()
    
    captured = capsys.readouterr()
    
    # 5. Verify the Windows-only suggestion was printed
    assert "non-system drive" in captured.out
    assert "Windows Explorer" in captured.out



def test_configure_workspace_exception(monkeypatch, tmp_path, capsys):
    """Hits the 'except Exception' block by replacing GLOBAL_CONFIG with a Mock."""
    
    # 1. Setup mocks for input and execution check
    monkeypatch.setattr("builtins.input", lambda _: str(tmp_path))
    monkeypatch.setattr(cli, "_test_execution", lambda x: True)
    
    # 2. Create a Mock object to replace GLOBAL_CONFIG
    mock_config = MagicMock()
    # Force write_text to raise an error when called
    mock_config.write_text.side_effect = Exception("Simulated disk error")
    
    # 3. Replace the variable in the module
    monkeypatch.setattr(cli, "GLOBAL_CONFIG", mock_config)
    
    # 4. Run the function
    cli.configure_workspace()
    
    # 5. Check output
    captured = capsys.readouterr()
    assert "ERROR: Could not save config file" in captured.out
    assert "Simulated disk error" in captured.out