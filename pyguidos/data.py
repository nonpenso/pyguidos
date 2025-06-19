# guidos/data.py

import os
from pathlib import Path

def test_data_dir() -> Path:
    """
    Returns the absolute path to the 'data' directory located at the package root.
    This function helps locate example/test data files relative to the installed package.

    Returns:
        Path: An absolute Path object pointing to the 'data' directory.
    """
    # Path of the current file
    current_file_path = Path(__file__)

    # Go up one level to the 'pyguidos' package directory
    package_root_dir = current_file_path.parent

    # Go up another level to the project root
    project_root_dir = package_root_dir.parent

    # Construct the path to the 'data' folder at the project root
    data_path = project_root_dir / "data"

    # Optional: Basic check to inform the user if the data directory isn't found
    if not data_path.is_dir():
        print(f"Warning: The expected 'data' directory was not found at {data_path}. "
              "Please ensure it exists and contains necessary data for examples/tests.")
    
    return data_path

