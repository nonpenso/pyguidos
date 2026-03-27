import os
import sys
import stat
import shutil
import platform
import subprocess
from pathlib import Path

import numpy as np
from scipy.ndimage import label, generate_binary_structure
import rasterio

from . import utils
from . import PROGS_DIR


def get_os_info():
    """
    Detects the current Operating System and CPU architecture.
    Used internally to select the correct binary executable for
    MSPA and Spatcon tools.

    Returns
    -------
    tuple
        - os_name (str): Operating system name ('Linux', 'Windows', 'Darwin').
        - arch (str): Architecture suffix ('' for x86_64, 'ARM' for arm64/aarch64,
          'Unknown' for unrecognised architectures).
        - is_win (bool): True if running on Windows.
    """
    current_os = platform.system()
    machine = platform.machine().lower()

    # Map architectures
    if machine in ['x86_64', 'amd64']:
        arch = ""
    elif machine in ['arm64', 'aarch64']:
        arch = "ARM"
    else:
        # Fallback to empty or specific handling if needed
        arch = "Unknown"

    is_win = (current_os == "Windows")
    return current_os, arch, is_win


def get_binary_path(tool_name):
    """
    Resolves the absolute path to a GTB binary executable for the
    current OS and architecture.

    Parameters
    ----------
    tool_name : str
        Name of the tool. Must be 'mspa' or 'spatcon'.

    Returns
    -------
    Path
        Absolute path to the binary executable.

    Raises
    ------
    OSError
        If the tool_name and OS combination is not supported.
    """
    current_os, arch, is_win = get_os_info()

    bins = {
        "mspa": {
            "Linux":   f'mspa{arch}_lin64',
            "Windows": f'mspa_win64{arch}.exe',
            "Darwin":  f'mspa{arch}_mac'
        },
        "spatcon": {
            "Linux":   f'spatcon{arch}_lin64',
            "Windows": f'spatcon{arch}64.exe',
            "Darwin":  f'spatcon{arch}_mac'
        }
    }

    try:
        bin_filename = bins[tool_name][current_os]
    except KeyError:
        raise OSError(f"Tool '{tool_name}' not supported on {current_os}")

    return PROGS_DIR / bin_filename


def run_mspa(run_dir, in_tiff, connectivity, edge_width, transition, internal,
             verbose=False):
    """
    Executes the MSPA binary with explicit absolute input and output
    paths. The binary is called in place from PROGS_DIR — no copy
    needed. Input and output are passed as absolute path strings to
    avoid GDAL provider path resolution issues on Windows.

    Parameters
    ----------
    run_dir : Path
        Temporary job directory (used only for verbose logging context).
    in_tiff : Path
        Absolute path to the input GeoTIFF (must be uint8, strip-based).
    connectivity : int
        Foreground connectivity rule. Must be 4 or 8.
    edge_width : int
        Edge width in pixels. Must be >= 1.
    transition : int
        Transition pixels flag. 0 or 1.
    internal : int
        Internal/external flag. 0 or 1.
    verbose : bool, optional
        If True, prints binary stdout output. Default False.

    Returns
    -------
    subprocess.CompletedProcess
        Result object from subprocess.run().

    Raises
    ------
    OSError
        If the binary is not found for the current OS/arch.
    SystemExit
        If the binary returns a non-zero exit code.
    """

    exe_path = get_binary_path("mspa")

    args = [
        str(exe_path),
        "-i", str(in_tiff),
        "-o", "mspa_output.tif",
        "-odir", str(run_dir) + os.sep,
        "-graphfg", str(connectivity),
        "-eew",     str(edge_width),
        "-transition", str(transition),
        "-internal",   str(internal)
    ]
    if verbose:
        args.append("-v")

    try:
        result = subprocess.run(
            args,
            cwd=str(run_dir),
            capture_output=True,
            text=True,
            check=True
        )
        if verbose and result.stdout:
            for line in result.stdout.splitlines():
                if "% [" in line:        # skip progress bar
                    continue
                if "No output given" in line:  # skip default dir warning
                    continue
                print(line)

    except subprocess.CalledProcessError as e:
        print(f"ERROR: MSPA failed. Exit code: {e.returncode}")
        if e.stdout: print(f"STDOUT: {e.stdout.strip()}")
        if e.stderr: print(f"STDERR: {e.stderr.strip()}")
        # Stop the script here
        sys.exit(1)
    except FileNotFoundError as e:
        print(f"ERROR: Binary not found or not executable: {e}")
        sys.exit(1) # This ensures the test passes and the user gets a clean exit

    return result


def run_spatcon(run_dir, verbose=False):
    """
    Copies the Spatcon binary into the temporary run directory and
    executes it from there. Spatcon reads its parameters and input
    data entirely from files in the working directory (scpars.txt,
    scsize.txt, scinput) and writes output to the same directory.
    No explicit input/output path arguments are passed.

    Parameters
    ----------
    run_dir : Path
        Temporary job directory containing scpars.txt, scsize.txt,
        and scinput. The binary will be copied here and executed
        with cwd set to this directory.
    verbose : bool, optional
        If True, prints binary stdout output. Default False.

    Returns
    -------
    subprocess.CompletedProcess
        Result object from subprocess.run().

    Raises
    ------
    OSError
        If the binary is not found for the current OS/arch.
    SystemExit
        If the binary returns a non-zero exit code.
    """

    _, _, is_win = get_os_info()

    exe_source = get_binary_path("spatcon")
    exe_name   = "tool.exe" if is_win else "tool"
    exe_target = run_dir / exe_name

    # Copy binary into run directory and set executable permission
    shutil.copy2(exe_source, exe_target)
    if not is_win:
        os.chmod(exe_target, os.stat(exe_target).st_mode | stat.S_IEXEC)

    try:
        result = subprocess.run(
            [str(exe_target)],
            cwd=str(run_dir),
            capture_output=True,
            text=True,
            check=True
        )
        if verbose and result.stdout:
            for line in result.stdout.splitlines():
                print(line)

    except subprocess.CalledProcessError as e:
        print(f"ERROR: Spatcon failed. Exit code: {e.returncode}")
        if e.stdout: print(f"STDOUT: {e.stdout.strip()}")
        if e.stderr: print(f"STDERR: {e.stderr.strip()}")
        sys.exit(1)
    except FileNotFoundError as e:
        print(f"ERROR: Spatcon binary not found or not executable: {e}")
        sys.exit(1)

    return result


def write_mspa_input(out_path, source_path, data_array, dtype, is_tiled):
    """
    Prepares the input GeoTIFF for the MSPA binary.
    Copies the file directly if already uint8, otherwise converts
    the data to uint8 and writes a new GeoTIFF.

    Parameters
    ----------
    out_path : str or Path
        Destination directory where the input file will be written.
    source_path : str or Path
        Path to the source GeoTIFF.
    data_array : np.ndarray
        Input data array already read from source_path (2D single band)
    dtype : str
        Data type string of the source GeoTIFF (e.g. 'uint8', 'uint16'),
        as returned by get_raster_info().
    is_tiled : bool
        True is if it is a tiled GeoTIFF.

    Returns
    -------
    Path
        Path to the written input file.
    """
    out_path = Path(out_path)
    target_path = out_path / "mspa_input.tif"

    if dtype == 'uint8' and not is_tiled:
        shutil.copy2(source_path, target_path)
    else:
        with rasterio.open(source_path) as src:
            profile = src.profile.copy()
        profile.update(
            dtype='uint8',
            nodata=None,
            tiled=False,
            blockxsize=None,
            blockysize=None
        )
        with rasterio.open(target_path, 'w', **profile) as dst:
            dst.write(data_array.astype(np.uint8), 1)

    return target_path


def write_spatcon_input(out_path, data_array):
    """
    Prepares the input binary file for the Spatcon tool.
    Converts the array to uint8 if needed and writes it as raw binary.

    Parameters
    ----------
    out_path : str or Path
        Destination directory where the input file will be written.
    data_array : np.ndarray
        Input data array to write as raw binary.

    Returns
    -------
    Path
        Path to the written input file.
    """
    out_path = Path(out_path)
    target_path = out_path / "scinput"

    if data_array.dtype != np.uint8:
        data_array = data_array.astype(np.uint8)
    data_array.tofile(target_path)

    return target_path


def write_spatcon_params(tmp_dir, dimentions, method):
    """
    Writes the Spatcon parameter files (scsize.txt and scpars.txt)
    required by the Spatcon binary before execution.

    Parameters
    ----------
    tmp_dir : str or Path
        Temporary working directory where parameter files will be written.
    dimensions : tuple
        Raster dimensions as (rows, cols).
    method : tuple
        Spatcon parameters as: w, r, b, a, h, m, f
    """
    # dims.txt
    with open(tmp_dir / 'scsize.txt', "w") as f:
       f.write(f'nrows {dimentions[0]}\nncols {dimentions[1]}\n')

    # spatcon.txt
    param_content = (
        f"w {method[0]}\n"
        f"r {method[1]}\n"
        f"b {method[2]}\n"
        f"a {method[3]}\n"
        f"h {method[4]}\n"
        f"m {method[5]}\n"
        f"f {method[6]}\n"
    )
    with open(tmp_dir / 'scpars.txt', 'w') as f:
        f.write(param_content)
        

def labelling_array(input_array, target_values):
    """
    Labels connected clusters of target pixel values using 8-connectivity
    and returns a frequency Counter of patch sizes. Used internally by
    acc() and rss() to identify and measure individual foreground patches.

    Parameters
    ----------
    input_array : np.ndarray
        2D input array to label.
    target_values : int or list of int
        Pixel value(s) to treat as foreground for labelling.
        Accepts a single integer or a list of integers.

    Returns
    -------
    tuple
        - labeled_array (np.ndarray): integer array where each connected
          patch of target_values is assigned a unique positive label ID.
          Background pixels (not in target_values) are labelled 0.
        - label_freq (Counter): mapping of patch label ID to pixel count,
          excluding background label 0.
    """
    # Ensure target_values is a list/array (handles single integer input)
    if isinstance(target_values, (int, np.integer)):
        target_values = [target_values]

    # Create the mask for specific forest classes
    foreground_mask = np.isin(input_array, target_values)

    # Define 8-connectivity (diagonal connections allowed)
    structure = generate_binary_structure(2, 2)
    labeled_array, num_patches = label(foreground_mask, structure=structure)

    # Count pixels per unique label (patch ID)
    label_freq = utils.get_pxl_freq(labeled_array)

    # Remove background (0) from the frequency count
    # '0' represents everything NOT in your target_values
    if 0 in label_freq:
        del label_freq[0]

    return labeled_array, label_freq