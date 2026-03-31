import sys
import numpy as np


def validate_wsize(wsize):
    """
    Validates a moving window size parameter.

    Parameters
    ----------
    wsize : int
        Window size in pixels. Must be an odd integer >= 3.

    Raises
    ------
    SystemExit
        If wsize is not an integer.
    SystemExit
        If wsize is less than 3 or is an even number.
    """
    if not isinstance(wsize, int):
        sys.exit(f"The window size must be an integer number (received {type(wsize).__name__})")
    
    if wsize < 3:
        sys.exit(f"The window size must be >= 3 (received {wsize})")
    
    if wsize % 2 == 0:
        sys.exit(f"The window size must be an odd number (received {wsize}). Try {wsize+1} or {wsize-1}.")


def validate_fmap_input(present_values, bands, allow_34=False):
    """
    Validates the pixel values and data type of a binary or multi-class
    input GeoTIFF. Ensures mandatory values are present and no 
    unexpected values exist.

    Parameters
    ----------
    present_values : list
        List of unique pixel values found in the raster.
    bands : int
        Number of bands. Must be 1.
    allow_34 : bool, optional
        If True, allows optional values 3 and 4 in addition to 0, 1, 2.
        Used for Fragmentation, Accounting and RSS tools.
        If False (default), only values 0, 1 and 2 are permitted.
        Used for MSPA.

    Raises
    ------
    SystemExit
        If band are more than 1, if mandatory values 1 and 2 are missing,
        or if invalid pixel values are present.
    """
    # Check number of bands 
    if bands != 1:
        sys.exit(f"ERROR: Input GeoTIFF must be single-band, but has {bands} bands.")

    # Setup Sets
    present_set = set(present_values)
    mandatory = {1, 2}
    
    # Logic: If allow_34 is True, allowed is {0,1,2,3,4}. Otherwise {0,1,2}.
    allowed = {0, 1, 2, 3, 4} if allow_34 else {0, 1, 2}

    # Check for Mandatory Values (1 and 2)
    missing = mandatory - present_set
    if missing:
        sys.exit(f"ERROR: Input Geotiff requires values 1 and 2. Missing: {missing}")

    # Check for Invalid Values
    invalid = present_set - allowed
    if invalid:
         sys.exit(f"ERROR: Input GeoTIFF contains invalid values: {sorted(list(invalid))}. "
                 f"Allowed: {sorted(list(allowed))}")


def validate_lm_input(present_values, bands):
    """
    Validates the pixel values of an input GeoTIFF for the Landscape
    Mosaic tool. Requires all three land cover classes to be present.

    Parameters
    ----------
    present_values : list
        List of unique pixel values found in the raster.
        Mandatory: 1, 2, 3. Optional: 0 (NoData).
    bands : int
        Number of bands. Must be 1.

    Raises
    ------
    SystemExit
        If any of the mandatory values 1, 2 or 3 are missing, or if
        values other than 0, 1, 2 and 3 are present.
    """
    # Check number of bands 
    if bands != 1:
        sys.exit(f"ERROR: Input GeoTIFF must be single-band, but has {bands} bands.")

    # Setup Sets
    present_set = set(present_values)
    mandatory = {1, 2, 3}
    allowed = {0, 1, 2, 3}
    
    # Check for Mandatory Values
    missing = mandatory - present_set
    if missing:
        sys.exit(
            f"Input Geotiff requires classes 1, 2, and 3. Missing: {missing}. "
            "Ensure your map contains all three (e.g., Natural, Agriculture, Developed)."
        )
    
    # Check for Invalid Values
    invalid = present_set - allowed
    if invalid:
        sys.exit(f"Input Geotiff contains invalid values: {sorted(list(invalid))}.")


def validate_frag_params(wsize, method):
    """
    Validates the parameters for the Fragmentation tool.
    Delegates window size validation to validate_wsize().

    Parameters
    ----------
    wsize : int
        Window size in pixels. Must be an odd integer >= 3.
    method : str
        Fragmentation method. Must be 'FAD' or 'FAC'.

    Raises
    ------
    TypeError
        If wsize is not an integer.
    ValueError
        If wsize is invalid or method is not in the allowed list.
    """
    validate_wsize(wsize)
    
    allowed_methods = ['FAD', 'FAC']
    if method not in allowed_methods:
        sys.exit(f"Fragmentation method must be {allowed_methods} (received '{method}')")


def validate_mspa_params(edge_width, connectivity):
    """
    Validates the parameters for the MSPA tool.

    Parameters
    ----------
    edge_width : int
        Width of the edge zone in pixels. Must be an integer >= 1.
    connectivity : int
        Pixel connectivity. Must be 4 or 8.

    Raises
    ------
    SystemExit
        If edge_width is not a positive integer or connectivity
        is not 4 or 8.
    """
    if not isinstance(edge_width, int) or edge_width < 1:
        sys.exit(f"The edge width must be an integer number >= 1 (received {edge_width})")
        
    if connectivity not in [4, 8]:
        sys.exit(f"The connectivity must be 4 or 8 (received {connectivity})")


def validate_acc_params(thresholds):
    """
    Validates the sequence of patch size thresholds for the
    Accounting tool. Ensures values are positive, unique integers
    within the allowed count range.

    Parameters
    ----------
    thresholds : list, tuple or np.ndarray
        Sequence of 1 to 5 unique positive integers defining patch
        size class boundaries. Duplicate values are removed.
        For example, [10, 100, 1000] creates 4 size classes.

    Returns
    -------
    list
        Sorted list of unique integer thresholds, ready for use
        in acc() processing.

    Raises
    ------
    SystemExit
        If thresholds is not a list, tuple or array, is empty,
        contains non-integer values, contains values <= 0, or
        contains more than 5 unique values.
    """
    # Check for allowed types
    if not isinstance(thresholds, (list, tuple, np.ndarray)):
        sys.exit(f"ERROR: Thresholds must be a list, tuple, or array. Received: {type(thresholds).__name__}")

    # Handle empty input
    if len(thresholds) == 0:
        sys.exit("ERROR: Thresholds sequence cannot be empty. Provide between 1 and 5 values.")

    # Ensure uniqueness and convert to integers
    try:
        unique_thresholds = set(int(t) for t in thresholds)
    except (ValueError, TypeError):
        sys.exit("ERROR: All thresholds must be integers.")

    # Check the number of classes (1 to 5 thresholds)
    num_t = len(unique_thresholds)
    if not (1 <= num_t <= 5):
        sys.exit(f"ERROR: You provided {num_t} unique thresholds. Accounting allows a minimum of 1 and a maximum of 5.")

    # Check for positive values
    if any(t <= 0 for t in unique_thresholds):
        sys.exit("ERROR: All thresholds must be positive integers (greater than 0).")

    # Return sorted list for the next step of the process
    return sorted(list(unique_thresholds))
