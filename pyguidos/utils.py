import os
import re
import stat
import shutil
import platform
import subprocess
import collections
import time
from pathlib import Path
import tempfile
import uuid

import numpy as np
from scipy.ndimage import label, generate_binary_structure
import rasterio
from rasterio.enums import ColorInterp


# def get_module_root():
#     """
#     Locates the package root directory containing the 'progs' folder.
#     Works in two contexts:
#     - Standard scripts/console: climbs from __file__
#     - Jupyter notebooks: climbs from os.getcwd() since __file__ is undefined
#     """
#     try:
#         current = Path(__file__).resolve()
#     except NameError:
#         # Fallback: For Jupyter/IPython
#         current = Path(os.getcwd()).resolve()

#     # Climb up until finds the 'progs' folder
#     for parent in [current] + list(current.parents):
#         if (parent / "progs").exists():
#             return parent
#     return current

# --- GLOBAL PATHS ---
MODULE_ROOT = Path(__file__).resolve().parent
PROGS_DIR = MODULE_ROOT / "progs"
TEMPL_DIR = MODULE_ROOT / "templates"
DATA_DIR = MODULE_ROOT / "data"
WORK_DIR = MODULE_ROOT / "work"


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


def setup_run_dir():
    """
    Creates a unique temporary working directory for a single binary
    executable run. Silently removes job folders older than
    7 days from the work directory.

    Returns
    -------
    Path
        Path to the newly created temporary run directory,
        named 'job_<uuid>' inside the package work/ folder.
    """
    # Silently clean up folders older than 1 weeks
    max_age_seconds = 7 * 24 * 3600
    now = time.time()
    for old_job in WORK_DIR.glob("job_*"):
        if old_job.is_dir():
            age = now - old_job.stat().st_mtime
            if age > max_age_seconds:
                try:
                    shutil.rmtree(old_job)
                except Exception:
                    pass  # Silent: never block a new run over cleanup

    # Create a unique subfolder for this specific run
    unique_id = str(uuid.uuid4())[:8]
    run_dir = WORK_DIR / f"job_{unique_id}"

    # 3. Create the directories
    run_dir.mkdir(parents=True, exist_ok=True)

    return run_dir


def run_guidos_tool(tool_name, tmp_dir, args, verbose=False):
    """
    Locates, copies and executes a GuidosToolbox binary (MSPA or Spatcon)
    in a temporary working directory. Selects the correct binary
    automatically based on the current OS and architecture.

    Parameters
    ----------
    tool_name : str
        Name of the tool to run. Must be 'mspa' or 'spatcon'.
    tmp_dir : str or Path
        Temporary working directory where the binary will be copied
        and executed.
    args : list
        List of command-line arguments to pass to the binary.
        Pass an empty list if no arguments are needed.
    verbose : bool, optional
        If True, prints the binary stdout output, filtering out
        progress bars and default warnings. Default False.

    Returns
    -------
    subprocess.CompletedProcess
        The result object from subprocess.run(), containing returncode,
        stdout and stderr.

    Raises
    ------
    OSError
        If the tool_name and OS combination is not supported.
    SystemExit
        If the binary returns a non-zero exit code.
    """
    current_os, arch, is_win = get_os_info()

    # 1. Binary Mapping Table
    bins = {
        "mspa": {
            "Linux": f'mspa{arch}_lin64',
            "Windows": f'mspa_win64{arch}.exe',
            "Darwin": f'mspa{arch}_mac'
        },
        "spatcon": {
            "Linux": f'spatcon{arch}_lin64',
            "Windows": f'spatcon{arch}64.exe',
            "Darwin": f'spatcon{arch}_mac'
        }
    }

    try:
        bin_filename = bins[tool_name][current_os]
    except KeyError:
        raise OSError(f"Tool '{tool_name}' not supported on {current_os}")


    # 2. Setup Execution Path
    exe_source = PROGS_DIR / bin_filename
    exe_name = "tool.exe" if is_win else "tool"
    exe_target = Path(tmp_dir) / exe_name

    # 3. Copy and Set Permissions
    shutil.copy2(exe_source, exe_target)
    if not is_win:
        os.chmod(exe_target, os.stat(exe_target).st_mode | stat.S_IEXEC)

    # 4. Execute
    exec_cmd = [str(exe_target)] + args
    try:
        result = subprocess.run(
            exec_cmd,
            cwd=str(tmp_dir),
            capture_output=True,
            text=True,
            check=True
        )
        if result.stdout:
            if verbose:
                output_lines = result.stdout.splitlines()
                for line in output_lines:
                    if "% [" in line:  # Skip MSPA progress bar
                        continue
                    if "No output given" in line: # Skip MSPA default dir wartning
                        continue
                    print(line)

    except subprocess.CalledProcessError as e:
        print(f"ERROR: {tool_name.upper()} failed. Exit code: {e.returncode}")
        if e.stdout: print(f"STDOUT: {e.stdout.strip()}")
        if e.stderr: print(f"STDERR: {e.stderr.strip()}")
        # Stop the script here
        import sys
        sys.exit(1)

    return result

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


def write_mspa_input(out_path, source_path, data_array, dtype):
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

    Returns
    -------
    Path
        Path to the written input file.
    """
    out_path = Path(out_path)
    target_path = out_path / "mspa_input.tif"

    if dtype == 'uint8':
        shutil.copy2(source_path, target_path)
    else:
        with rasterio.open(source_path) as src:
            profile = src.profile.copy()
        profile.update(dtype='uint8')
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


def get_raster_info(intiff_path):
    """
    Extracts all required metadata from a GeoTIFF file.

    Parameters
    ----------
    intiff_path : str or Path
        Path to the input GeoTIFF.

    Returns
    -------
    dict
        Dictionary containing:
        - 'profile' (dict): full rasterio profile.
        - 'rows' (int): number of rows.
        - 'cols' (int): number of columns.
        - 'bands' (int): number of bands.
        - 'resX' (float): pixel width.
        - 'resY' (float): pixel height.
        - 'dtype' (str): data type string (e.g. 'uint8').
        - 'wkt' (str): CRS as Well Known Text.
        - 'epsg' (int or str): EPSG code, or 'Unknown' if not resolvable.
        - 'is_projected' (bool): True if CRS is a projected coordinate system.
        - 'bounds' (BoundingBox): raster bounding box.
        - 'cmap' (dict or None): colormap if present, otherwise None.
        - 'tag' (str): TIFFTAG_IMAGEDESCRIPTION value, or '--' if absent.
    """
    with rasterio.open(intiff_path) as src:
        prof = src.profile.copy()

        # Get resolution
        resX, resY = src.res

        # Get projection
        wkt = src.crs.to_wkt()
        epsg = src.crs.to_epsg()
        if epsg is None:
            matches = re.findall(r'AUTHORITY\["EPSG","(\d+)"\]', wkt)
            epsg = matches[-1] if matches else "Unknown"

        # Get colormap
        try:
            cmap = src.colormap(1)
        except ValueError:
            # Input raster has no colormap
            cmap = None

        # Tags
        tags = src.tags()
        tag_descr = tags.get('TIFFTAG_IMAGEDESCRIPTION') or "--"

        return {
            "profile": prof,
            "rows": src.height,
            "cols": src.width,
            "bands": src.count,
            "resX": resX,
            "resY": resY,
            "dtype": prof['dtype'],
            "wkt": wkt,
            "epsg": epsg,
            "is_projected": src.crs.is_projected,
            "bounds": src.bounds,
            "cmap": cmap,
            "tag": tag_descr
        }


def save_output_geotiff(output_path, data, profile, colormap_input, tag_descr):
    """
    Standardised writer for all pyGuidos output GeoTIFFs.
    Handles palette color interpretation, GTB metadata tags,
    and colormap application. Accepts both 2D and 3D arrays.

    Parameters
    ----------
    output_path : str or Path
        Path where the output GeoTIFF will be written.
    data : np.ndarray
        Output data array, either 2D (rows, cols) or 3D (bands, rows, cols).
        2D arrays are automatically expanded to 3D before writing.
    profile : dict
        Rasterio profile dictionary used as the base for the output file.
    colormap_input : dict or str or Path
        Colormap source. Either:
        - dict: pre-loaded colormap in rasterio format {value: (r,g,b,a)},
          as returned by MSPA binary.
        - str or Path: path to a colormap .txt file with space-separated
          columns: value r g b, as used by Spatcon-based tools.
    tag_descr : str
        String written to TIFFTAG_IMAGEDESCRIPTION. Encodes the tool ID
        and parameters for later parsing by get_tool_parameters().
    """
    # Update profile for Palette support
    out_profile = profile.copy()
    out_profile.update({'photometric': 'palette'})

    # Define standard Guidos tags
    tags = {
        "TIFFTAG_IMAGEDESCRIPTION": tag_descr,
        "TIFFTAG_SOFTWARE": "pyGuidos"
    }

    # Resolve Colormap
    color_map = {}

    # Check if it's a dictionary (MSPA style)
    if isinstance(colormap_input, dict):
        color_map = colormap_input
    # Check if it's a Path or String (Spatcon style)
    elif isinstance(colormap_input, (str, Path)):
        cmap_path = Path(colormap_input)
        with open(cmap_path, 'r') as f:
            for line in f:
                parts = line.split()
                if len(parts) >= 4:
                    val = int(parts[0])
                    r, g, b = int(parts[1]), int(parts[2]), int(parts[3])
                    color_map[val] = (r, g, b, 255)

    # Ensure data is 3D (bands, rows, cols) for rasterio writing
    if data.ndim == 2:
        data = data[np.newaxis, :]

    # Write the file
    with rasterio.open(output_path, 'w', **out_profile) as dst:
        dst.write(data)
        dst.colorinterp = [ColorInterp.palette]
        dst.write_colormap(1, color_map)
        dst.update_tags(**tags)


def get_pxl_freq(array, chunk_size=1000):
    """
    Counts pixel value frequencies for both 2D and 3D arrays.
    Uses np.bincount for uint8 arrays (fast path), and chunked
    np.unique for larger dtypes such as int32 labeled arrays.

    Parameters
    ----------
    array : np.ndarray
        Input array, either 2D (rows, cols) or 3D (bands, rows, cols).
        For 3D arrays, only the first band is used.
    chunk_size : int, optional
        Number of rows per chunk for the general path. Only used for
        non-uint8 arrays. Default 1000.

    Returns
    -------
    Counter
        A collections.Counter mapping pixel values to their counts.
        Zero-count values are excluded.
    """
    data = array[0] if array.ndim == 3 else array

    # Fast path for uint8: bincount covers all 256 possible values instantly
    if data.dtype == np.uint8:
        counts = np.bincount(data.ravel(), minlength=256)
        return collections.Counter({i: int(c) for i, c in enumerate(counts) if c > 0})

    # General path for int32/large arrays (e.g. labeled patch arrays)
    total_counts = collections.Counter()
    for i in range(0, data.shape[0], chunk_size):
        chunk = data[i : i + chunk_size, :]
        values, counts = np.unique(chunk, return_counts=True)
        total_counts.update(dict(zip(values.tolist(), counts.tolist())))

    return total_counts


def running_time(start_time, end_time):
    """
    Formats a duration into a human-readable time string.

    Parameters
    ----------
    start_time : float
        Start timestamp from time.time().
    end_time : float
        End timestamp from time.time().

    Returns
    -------
    str
        Formatted duration string, e.g. '2m 3.4s', '1h 5m 2.1s',
        or '0.95 seconds'.
    """
    elapsed = end_time - start_time
    hours, rem = divmod(elapsed, 3600)
    minutes, seconds = divmod(rem, 60)
    if hours > 0:
        run_time = f"{int(hours)}h {int(minutes)}m {seconds:.1f}s"
    elif minutes > 0:
        run_time = f"{int(minutes)}m {seconds:.1f}s"
    else:
        run_time = f"{seconds:.2f} seconds"

    return run_time


def generate_text_report(template_path, output_path, data_dict):
    """
    Fills a .txt template file with computed metrics and writes
    the result to disk. Uses Python str.format() syntax for
    placeholders in the template (e.g. {input_file}, {rows_val}).

    Parameters
    ----------
    template_path : str or Path
        Path to the .txt template file containing format placeholders.
    output_path : str or Path
        Path where the filled report will be written.
    data_dict : dict
        Dictionary mapping placeholder names to their values.
        Missing keys are reported to stdout and the report is not written.
    """
    with open(template_path, 'r') as f:
        template_content = f.read()

    try:
        # Use .format_map to safely handle keys
        report = template_content.format(**data_dict)
        with open(output_path, 'w') as f:
            f.write(report)
    except KeyError as e:
        print(f"Error: Missing key {e} in data dictionary for template.")


def update_time_line(file_path, time_str):
    """
    Finds the line starting with 'Computational time:' in an existing
    .txt report and updates it with the final elapsed time. Called after
    the main analysis completes to replace the placeholder written during
    the stats step with the true total runtime.

    Parameters
    ----------
    file_path : str or Path
        Path to the .txt report file to update.
    time_str : str
        Formatted time string as returned by running_time().

    Returns
    -------
    bool
        True if the line was found and updated, False otherwise.
    """
    file_path = Path(file_path)

    try:
        with open(file_path, 'r') as f:
            lines = f.readlines()

        # Pattern: Starts with 'Computational time:', then matches everything to the end of the line
        pattern = r"^(Computational time:\s*).*"

        new_lines = []
        found = False

        for line in lines:
            # re.sub will replace the whole line if the pattern matches
            if re.match(pattern, line):
                new_lines.append(f"Computational time: {time_str}\n")
                found = True
            else:
                new_lines.append(line)

        if found:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.writelines(new_lines)
            return True

        return False
    except Exception:
        return False


def get_tool_parameters(tag_description):
    """
    Parses a GTB TIFFTAG_IMAGEDESCRIPTION string into a dictionary
    of tool ID and parameters. Used to identify the tool and settings
    that generated a GeoTIFF when running standalone stats functions.

    Parameters
    ----------
    tag_description : str
        Tag string in the format 'GTB_TOOLID, param1 param2 ...'.
        For example: 'GTB_MSPA, 8 1 1 1' or 'GTB_FOS, FAD 27x27'.

    Returns
    -------
    dict or None
        Dictionary with 'tool_id' and tool-specific parameter keys,
        or None if the tag is empty or not in the expected format.
        Keys vary by tool:
        - GTB_MSPA: 'connectivity', 'edge_width', 'transition', 'int_ext'
        - GTB_FOS:  'tiftype', 'for_thres', 'connect', 'method', 'pxlsize', 'wsize'
        - GTB_LM:   'wsize'
        - GTB_ACC:  'thresholds'
    """
    if not tag_description or "," not in tag_description:
        return None

    # Split into ID and Params
    tool, param_str, weblink = [s.strip() for s in tag_description.split(",")]
    params = param_str[1:-1].split(',')

    result = {"tool_id": tool, "web_link": weblink}

    if tool == "GTB_FOS":
        # Format: "GTB_FOS, <Binary,-1,8,FAD_5,100.000,31>"
        result["tiftype"] = params[0]
        result["for_thres"] = params[1]
        result["connect"] = params[2]
        result["method"] = params[3]
        result["pxlsize"] = params[4]
        result["wsize"] = params[5]

    elif tool == "GTB_MSPA":
        # Format: "GTB_MSPA, <8,1,1,1>"
        result["connectivity"] = params[0]
        result["edge_width"] = params[1]
        result["transition"] = params[2]
        result["int_ext"] = params[3]

    elif tool == "GTB_LM":
        # Format: "GTB_LM, <33>"
        result["wsize"] = params[0]

    elif tool == "GTB_ACC":
        # Format: "GTB_ACC, <1000,100000,1000000,2000000>"
        result["thresholds"] = params

    return result

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
    label_freq = get_pxl_freq(labeled_array)

    # Remove background (0) from the frequency count
    # '0' represents everything NOT in your target_values
    if 0 in label_freq:
        del label_freq[0]

    return labeled_array, label_freq

def get_gtb_nodata(tiff_path):
    """
    Resolves the correct nodata value for a given GeoTIFF using a
    three-level priority:
    1. GTB output (has GTB tag): returns the GTB convention nodata
       value for that tool.
    2. Non-GTB, nodata not set: returns 0.
    3. Non-GTB, nodata set: returns the tiff's own nodata value.

    Parameters
    ----------
    tiff_path : str or Path
        Path to the input GeoTIFF.

    Returns
    -------
    int
        The resolved nodata value to use for masking operations.
    """
    GTB_NODATA = {
        "GTB_MSPA": 129,
        "GTB_FOS":  102,
        "GTB_ACC":  129,
        "GTB_LM":   0,
    }

    info = get_raster_info(tiff_path)

    # Priority 1: GTB output
    if info["tag"] and info["tag"] != "--":
        tool_params = get_tool_parameters(info["tag"])
        if tool_params and tool_params.get("tool_id") in GTB_NODATA:
            return GTB_NODATA[tool_params["tool_id"]]

    # Priority 2 & 3: non-GTB
    return info["profile"].get("nodata") or 0