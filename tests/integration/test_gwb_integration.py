import pytest
from pathlib import Path
import shutil
import os

# Import a functions from pyguidos package
from pyguidos.gwb import gwb_mspa 

# Define the absolute path to the dummy integration test data
INTEGRATION_TEST_INPUT_ROOT = Path(__file__).parent / "data"

# Define the shared dummy GeoTIFF file
DUMMY_SHARED_INPUT_FILE = INTEGRATION_TEST_INPUT_ROOT / "dummy_012_input.tif"
DUMMY_SHARED_INPUT_STEM = DUMMY_SHARED_INPUT_FILE.stem # 'dummy_012_input'

# Fixture for setting up and tearing down output directories for integration tests
@pytest.fixture
def integration_output_dir(tmp_path):
    """
    Provides a temporary output directory for integration tests and ensures cleanup.
    """
    output_dir = tmp_path / "integration_output"
    output_dir.mkdir()
    yield output_dir
    # Cleanup is handled automatically by tmp_path for directories created within it.

# Mark the entire module or individual tests as 'integration'
pytestmark = pytest.mark.integration

# --- Integration Test for gwb_mspa ---

# This test serves to confirm the full pipeline:
# Python wrapper -> _gwb_common -> GWB_MSPA executable -> Expected outputs

def test_gwb_mspa_integration(integration_output_dir):
    """
    Performs an integration test for gwb_mspa.
    Requires GWB_MSPA executable to be in PATH and DUMMY_SHARED_INPUT_FILE to exist.
    """
    # Ensure the dummy input file exists for the test to run.
    if not DUMMY_SHARED_INPUT_FILE.exists():
        pytest.skip(f"Integration test skipped: Dummy input file not found at {DUMMY_SHARED_INPUT_FILE}")

    print(f"\nRunning GWB_MSPA integration test. Input dir: {DUMMY_SHARED_INPUT_FILE.parent}, Output: {integration_output_dir}")

    try:
        gwb_mspa(
            input_dir=DUMMY_SHARED_INPUT_FILE.parent, # Pass the directory containing the input file
            output_dir=integration_output_dir,
            conn_8=True,
            edge_width=1,
            transition=True,
            int_ext=True,
            disk=False,
            stats=True
        )

        # Check for the specific output folder for this input TIFF
        expected_mspa_output_folder = integration_output_dir / f"{DUMMY_SHARED_INPUT_STEM}_mspa"
        assert expected_mspa_output_folder.is_dir(), f"Expected MSPA output folder not found: {expected_mspa_output_folder}"
        print(f"MSPA output folder found: {expected_mspa_output_folder}")

        # Check for files *inside* the output folder
        expected_tif_in_folder = expected_mspa_output_folder / f"{DUMMY_SHARED_INPUT_STEM}_8_1_1_1.tif" 
        expected_txt_in_folder = expected_mspa_output_folder / f"{DUMMY_SHARED_INPUT_STEM}_8_1_1_1.txt" 

        # Check if at least one .tif and one .txt file exist within the folder
        # A more robust check might iterate and assert on file count or specific patterns
        tif_files_found = list(expected_mspa_output_folder.glob('*.tif'))
        txt_files_found = list(expected_mspa_output_folder.glob('*.txt'))

        assert len(tif_files_found) > 0, f"No TIFF files found in {expected_mspa_output_folder}"
        assert len(txt_files_found) > 0, f"No TXT files found in {expected_mspa_output_folder}"
        
        print(f"Found {len(tif_files_found)} TIFF files and {len(txt_files_found)} TXT files in {expected_mspa_output_folder}")


    except Exception as e:
        pytest.fail(f"gwb_mspa integration test failed: {e}")
        # Print contents of the entire output directory for debugging on failure
        if integration_output_dir.exists():
            print(f"Contents of output directory {integration_output_dir}:")
            for item in integration_output_dir.iterdir():
                if item.is_dir():
                    print(f"- {item.name}/")
                    for sub_item in item.iterdir():
                        print(f"  - {sub_item.name}")
                else:
                    print(f"- {item.name}")
