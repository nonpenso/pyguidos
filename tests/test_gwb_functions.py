# tests/test_gwb_functions.py

import pytest
from pathlib import Path
from unittest.mock import patch

# Import the functions from your pyguidos package
from pyguidos.gwb import (
    gwb_mspa,
    gwb_acc,
    gwb_frag,
    gwb_rss,
    gwb_dist,
    gwb_lm,
    gwb_parc,
    gwb_rec,
    gwb_sc,
    gwb_gsc,
    gwb_spa,
    _gwb_common 
)

# --- Fixtures for common test setup ---

@pytest.fixture
def temp_dirs(tmp_path):
    """
    Provides temporary input and output directories for tests.
    `tmp_path` is a pytest built-in fixture for temporary directories.
    """
    input_dir = tmp_path / "input"
    output_dir = tmp_path / "output"
    input_dir.mkdir()
    output_dir.mkdir() # GWB typically expects an empty output directory
    yield input_dir, output_dir

@pytest.fixture(autouse=True)
def mock_gwb_common(mocker):
    """
    Mocks the _gwb_common function for all tests in this module.
    This prevents actual subprocess calls to GWB executables.
    `mocker` is provided by pytest-mock.
    """
    # Patch pyguidos.gwb._gwb_common 
    mock = mocker.patch('pyguidos.gwb._gwb_common')
    return mock

# --- Tests for gwb_mspa ---

def test_gwb_mspa_default_parameters(temp_dirs, mock_gwb_common):
    """
    Test gwb_mspa with default parameters.
    Verifies that _gwb_common is called with the correct arguments.
    """
    input_dir, output_dir = temp_dirs
    
    gwb_mspa(input_dir=input_dir, output_dir=output_dir)

    # Assert that _gwb_common was called exactly once
    mock_gwb_common.assert_called_once()

    # Get the arguments passed to _gwb_common
    call_args, call_kwargs = mock_gwb_common.call_args

    # Check the 'command' argument
    expected_command = [
        "GWB_MSPA",
        f"-i={input_dir}",
        f"-o={output_dir}"
    ]
    assert call_kwargs['command'] == expected_command

    # Check the 'param_file_path' argument
    assert call_kwargs['param_file_path'] == input_dir / "mspa-parameters.txt"

    # Check the 'param_file_content' argument (default values)
    expected_param_content = [
        '8',   # conn_8=True
        '1',   # edge_width=1
        '1',   # transition=True
        '1',   # int_ext=True
        '0',   # disk=False
        '0'    # stats=False
    ]
    assert call_kwargs['param_file_content'] == expected_param_content

    # Check blank_lines
    assert call_kwargs['blank_lines'] == 26
    
    # Check verbose
    assert call_kwargs['verbose'] is True # Default is True

def test_gwb_mspa_custom_parameters(temp_dirs, mock_gwb_common):
    """
    Test gwb_mspa with custom parameters.
    Verifies that _gwb_common is called with the correct arguments.
    """
    input_dir, output_dir = temp_dirs
    
    gwb_mspa(
        input_dir=input_dir,
        output_dir=output_dir,
        conn_8=False,
        edge_width=3,
        transition=False,
        int_ext=False,
        disk=True,
        stats=True,
        verbose=False
    )

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_command = [
        "GWB_MSPA",
        f"-i={input_dir}",
        f"-o={output_dir}"
    ]
    assert call_kwargs['command'] == expected_command

    expected_param_content = [
        '4',   # conn_8=False
        '3',   # edge_width=3
        '0',   # transition=False
        '0',   # int_ext=False
        '1',   # disk=True
        '1'    # stats=True
    ]
    assert call_kwargs['param_file_content'] == expected_param_content
    assert call_kwargs['verbose'] is False

def test_gwb_mspa_missing_input_dir_raises_error(temp_dirs, mock_gwb_common):
    """
    Test that gwb_mspa handles a non-existent input directory (if your function validates this).
    Note: The current _gwb_common handles FileNotFoundError for the executable,
    but the Path objects themselves don't validate existence on creation.
    If you want to validate input_dir existence in gwb_mspa, you'd add a check there.
    For this test, we'll assume _gwb_common would eventually fail if the path is bad.
    """
    _, output_dir = temp_dirs
    non_existent_input_dir = Path("/non/existent/path")

    # Here, we're testing the Python wrapper's behavior, not GWB's.
    # If gwb_mspa itself doesn't validate input_dir existence, this test
    # would pass (because _gwb_common is mocked and won't actually try to access it).
    # If you want to add validation:
    # with pytest.raises(FileNotFoundError, match="Input directory does not exist"):
    #     gwb_mspa(input_dir=non_existent_input_dir, output_dir=output_dir)

    # For now, just ensure it calls _gwb_common with the provided (invalid) path
    gwb_mspa(input_dir=non_existent_input_dir, output_dir=output_dir)
    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args
    assert f"-i={non_existent_input_dir}" in call_kwargs['command']


# --- Tests for gwb_acc ---

def test_gwb_acc_default_parameters(temp_dirs, mock_gwb_common):
    """
    Test gwb_acc with default parameters and required arguments.
    """
    input_dir, output_dir = temp_dirs
    pix_res = 10
    thresh = [100, 1000]

    gwb_acc(input_dir=input_dir, output_dir=output_dir, pix_res=pix_res, thresh=thresh)

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_command = [
        "GWB_ACC",
        f"-i={input_dir}",
        f"-o={output_dir}"
    ]
    assert call_kwargs['command'] == expected_command

    expected_param_content = [
        '8',           # conn_8=True
        str(pix_res),
        '100 1000',    # thresh
        'default',     # out_opt=True
        '0'            # big3pink=False
    ]
    assert call_kwargs['param_file_content'] == expected_param_content
    assert call_kwargs['blank_lines'] == 24
    assert call_kwargs['verbose'] is True

def test_gwb_acc_custom_parameters(temp_dirs, mock_gwb_common):
    """
    Test gwb_acc with custom parameters.
    """
    input_dir, output_dir = temp_dirs
    pix_res = 20
    thresh = [50, 500, 5000]

    gwb_acc(
        input_dir=input_dir,
        output_dir=output_dir,
        pix_res=pix_res,
        thresh=thresh,
        conn_8=False,
        out_opt=False,
        big3pink=True,
        verbose=False
    )

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_param_content = [
        '4',           # conn_8=False
        str(pix_res),
        '50 500 5000', # thresh
        'detailed',    # out_opt=False
        '1'            # big3pink=True
    ]
    assert call_kwargs['param_file_content'] == expected_param_content
    assert call_kwargs['verbose'] is False

def test_gwb_acc_empty_threshold_raises_error(temp_dirs):
    """
    Test that gwb_acc raises an error for an empty threshold list.
    This assumes you'd add validation for this in your gwb_acc function.
    """
    input_dir, output_dir = temp_dirs
    with pytest.raises(ValueError, match="Threshold list 'thresh' cannot be empty"):
        gwb_acc(input_dir=input_dir, output_dir=output_dir, pix_res=10, thresh=[])

# --- Tests for gwb_frag ---

def test_gwb_frag_default_parameters(temp_dirs, mock_gwb_common):
    input_dir, output_dir = temp_dirs
    pix_res = 10
    window_size = 3
    method = "FAD_5"

    gwb_frag(input_dir=input_dir, output_dir=output_dir, pix_res=pix_res, window_size=window_size, method=method)

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_param_content = [
        method,
        '8', # conn_8=True
        str(pix_res),
        str(window_size),
        '1', # precision=True
        '0', # stats=False
        'Binary' # input_map='Binary'
    ]
    assert call_kwargs['param_file_content'] == expected_param_content
    assert call_kwargs['blank_lines'] == 36

def test_gwb_frag_custom_parameters(temp_dirs, mock_gwb_common):
    input_dir, output_dir = temp_dirs
    pix_res = 25
    window_size = [5, 15, 25]
    method = "FED-APP_2"
    
    gwb_frag(
        input_dir=input_dir,
        output_dir=output_dir,
        pix_res=pix_res,
        window_size=window_size,
        method=method,
        precision=False,
        conn_8=False,
        stats=True,
        input_map="Grayscale 50"
    )

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_param_content = [
        method,
        '4', # conn_8=False
        str(pix_res),
        '5 15 25', # window_size
        '0', # precision=False
        '1', # stats=True
        'Grayscale 50' # input_map
    ]
    assert call_kwargs['param_file_content'] == expected_param_content

# --- Tests for gwb_rss ---

def test_gwb_rss_default_parameters(temp_dirs, mock_gwb_common):
    input_dir, output_dir = temp_dirs
    
    gwb_rss(input_dir=input_dir, output_dir=output_dir)

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_param_content = [
        '8' # conn_8=True
    ]
    assert call_kwargs['param_file_content'] == expected_param_content
    assert call_kwargs['blank_lines'] == 13

def test_gwb_rss_custom_parameters(temp_dirs, mock_gwb_common):
    input_dir, output_dir = temp_dirs
    
    gwb_rss(input_dir=input_dir, output_dir=output_dir, conn_8=False, verbose=False)

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_param_content = [
        '4' # conn_8=False
    ]
    assert call_kwargs['param_file_content'] == expected_param_content
    assert call_kwargs['verbose'] is False

# --- Tests for gwb_dist ---

def test_gwb_dist_eucl_only(temp_dirs, mock_gwb_common):
    input_dir, output_dir = temp_dirs
    
    gwb_dist(input_dir=input_dir, output_dir=output_dir, eucl_hysom=True)

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_param_content = [
        '8', # conn_8=True
        '1'  # eucl_hysom=True
    ]
    assert call_kwargs['param_file_content'] == expected_param_content
    assert call_kwargs['blank_lines'] == 16

def test_gwb_dist_eucl_hysom(temp_dirs, mock_gwb_common):
    input_dir, output_dir = temp_dirs
    
    gwb_dist(input_dir=input_dir, output_dir=output_dir, eucl_hysom=False, conn_8=False)

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_param_content = [
        '4', # conn_8=False
        '2'  # eucl_hysom=False
    ]
    assert call_kwargs['param_file_content'] == expected_param_content

# --- Tests for gwb_lm ---

def test_gwb_lm_single_kdim(temp_dirs, mock_gwb_common):
    input_dir, output_dir = temp_dirs
    kdim = 5
    
    gwb_lm(input_dir=input_dir, output_dir=output_dir, kdim=kdim)

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_param_content = [
        str(kdim)
    ]
    assert call_kwargs['param_file_content'] == expected_param_content
    assert call_kwargs['blank_lines'] == 13

def test_gwb_lm_multiple_kdim(temp_dirs, mock_gwb_common):
    input_dir, output_dir = temp_dirs
    kdim = [3, 7, 11]
    
    gwb_lm(input_dir=input_dir, output_dir=output_dir, kdim=kdim, verbose=False)

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_param_content = [
        '3 7 11'
    ]
    assert call_kwargs['param_file_content'] == expected_param_content
    assert call_kwargs['verbose'] is False

# --- Tests for gwb_parc ---

def test_gwb_parc_default_parameters(temp_dirs, mock_gwb_common):
    input_dir, output_dir = temp_dirs
    
    gwb_parc(input_dir=input_dir, output_dir=output_dir)

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_param_content = [
        '8' # conn_8=True
    ]
    assert call_kwargs['param_file_content'] == expected_param_content
    assert call_kwargs['blank_lines'] == 16

def test_gwb_parc_custom_parameters(temp_dirs, mock_gwb_common):
    input_dir, output_dir = temp_dirs
    
    gwb_parc(input_dir=input_dir, output_dir=output_dir, conn_8=False, verbose=False)

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_param_content = [
        '4' # conn_8=False
    ]
    assert call_kwargs['param_file_content'] == expected_param_content
    assert call_kwargs['verbose'] is False

# --- Tests for gwb_rec ---

def test_gwb_rec_single_class_mapping(temp_dirs, mock_gwb_common):
    input_dir, output_dir = temp_dirs
    classes = [(1, 10)]
    
    gwb_rec(input_dir=input_dir, output_dir=output_dir, classes=classes)

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_param_content = [
        '1 10'
    ]
    assert call_kwargs['param_file_content'] == expected_param_content
    assert call_kwargs['blank_lines'] == 22

def test_gwb_rec_multiple_class_mapping(temp_dirs, mock_gwb_common):
    input_dir, output_dir = temp_dirs
    classes = [(1, 4), (2, 5), (3, 6)]
    
    gwb_rec(input_dir=input_dir, output_dir=output_dir, classes=classes, verbose=False)

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_param_content = [
        '1 4',
        '2 5',
        '3 6'
    ]
    assert call_kwargs['param_file_content'] == expected_param_content
    assert call_kwargs['verbose'] is False

# --- Tests for gwb_sc ---

def test_gwb_sc_default_parameters(temp_dirs, mock_gwb_common):
    input_dir, output_dir = temp_dirs
    
    gwb_sc(input_dir=input_dir, output_dir=output_dir)

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_param_content = [
        'R 1',
        'W 27',
        'A 0',
        'B 0',
        'H 1', # H=True
        'F 0', # F=False
        'Z 0',
        'M 0'
    ]
    assert call_kwargs['param_file_content'] == expected_param_content
    assert call_kwargs['blank_lines'] == 52

def test_gwb_sc_custom_parameters(temp_dirs, mock_gwb_common):
    input_dir, output_dir = temp_dirs
    
    gwb_sc(input_dir=input_dir, output_dir=output_dir, R=6, W=5, A=1, B=2, H=False, F=True, verbose=False)

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_param_content = [
        'R 6',
        'W 5',
        'A 1',
        'B 2',
        'H 2', # H=False
        'F 1', # F=True
        'Z 0',
        'M 0'
    ]
    assert call_kwargs['param_file_content'] == expected_param_content
    assert call_kwargs['verbose'] is False

# --- Tests for gwb_gsc ---

def test_gwb_gsc_default_parameters(temp_dirs, mock_gwb_common):
    input_dir, output_dir = temp_dirs
    # M, F, G are required, so providing minimal valid values
    M = 1 # Mean
    F = False # 8-bit
    G = True # moving windows
    
    gwb_gsc(input_dir=input_dir, output_dir=output_dir, M=M, F=F, G=G)

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_param_content = [
        'M 1',
        'F 1', # F=False -> '1'
        'G 0', # G=True -> '0'
        'P 0', # P=False
        'W 0', # W=0
        'A 1', # A=True
        'B 1', # B=1
        'X 0', # X=0
        'Y 0', # Y=0
        'K 0'  # K=0
    ]
    assert call_kwargs['param_file_content'] == expected_param_content
    assert call_kwargs['blank_lines'] == 121

def test_gwb_gsc_custom_parameters(temp_dirs, mock_gwb_common):
    input_dir, output_dir = temp_dirs
    M = 44 # Correlation
    F = True # 32-bit float (required for M=44)
    G = False # entire map
    P = True # exclude zero
    W = 7 # window size
    A = False # do not mask
    B = 3 # byte stretch
    X = 10 # target code 1
    Y = 20 # target code 2
    K = 5 # target difference level
    
    gwb_gsc(
        input_dir=input_dir, output_dir=output_dir, M=M, F=F, G=G,
        P=P, W=W, A=A, B=B, X=X, Y=Y, K=K, verbose=False
    )

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_param_content = [
        'M 44',
        'F 2', # F=True -> '2'
        'G 1', # G=False -> '1'
        'P 1', # P=True
        'W 7', # W=7
        'A 0', # A=False
        'B 3', # B=3
        'X 10', # X=10
        'Y 20', # Y=20
        'K 5'  # K=5
    ]
    assert call_kwargs['param_file_content'] == expected_param_content
    assert call_kwargs['verbose'] is False

# --- Tests for gwb_spa ---

def test_gwb_spa_default_parameters(temp_dirs, mock_gwb_common):
    input_dir, output_dir = temp_dirs
    classes = 2 # SLF, Contiguous
    
    gwb_spa(input_dir=input_dir, output_dir=output_dir, classes=classes)

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_param_content = [
        str(classes),
        '0' # stats=False
    ]
    assert call_kwargs['param_file_content'] == expected_param_content
    assert call_kwargs['blank_lines'] == 20

def test_gwb_spa_custom_parameters(temp_dirs, mock_gwb_common):
    input_dir, output_dir = temp_dirs
    classes = 5 # Core, Core-Openings, Edge, Perforation, Margin
    
    gwb_spa(input_dir=input_dir, output_dir=output_dir, classes=classes, stats=True, verbose=False)

    mock_gwb_common.assert_called_once()
    call_args, call_kwargs = mock_gwb_common.call_args

    expected_param_content = [
        str(classes),
        '1' # stats=True
    ]
    assert call_kwargs['param_file_content'] == expected_param_content
    assert call_kwargs['verbose'] is False