# gwb.py

import subprocess
import os
from pathlib import Path
from typing import List, Union, Tuple

def _gwb_common(
    command: List[str],  
    param_file_path: Path,
    param_file_content: List[str],
    blank_lines: int
) -> None:
    """
    Common helper function to execute GWB modules and handle basic error reporting.

    Args:
        command (List[str]): The complete command line as a list of strings, ready for subprocess.
        param_file_path (Path): Full path to the parameter file to be created.
        param_file_content (List[str]): A list of string lines to write to the
                                        parameter TXT file, in the correct order.
        blank_lines (int): The number of blank lines to write at the beginning of the
                           parameter file.

    Returns:
        None: The function communicates success/failure and output through printed messages
              and writes files to the specified `output_dir`.
    """

    # Write the TXT file with the parameters
    try:
        with open(param_file_path, "w") as f:
            for _ in range(blank_lines):
                f.write(';;\n')
            for line in param_file_content:
                f.write(line + '\n')
    except IOError as e:
        print(f"Error writing parameter file '{param_file_path}': {e}")
        return

    try:
        executable_name = command[0]
		# Execute GWB module using subprocess.run
        result = subprocess.run(command, capture_output=True, text=True, check=True)

        if result.stdout:
            print(result.stdout)

        # Clean up the parameter file if the command was successful
        try:
            os.remove(param_file_path)
        except OSError as e:
            print(f"Warning: Could not remove parameter file '{param_file_path}': {e}")

    except subprocess.CalledProcessError as e:
        print(f"\nError: {executable_name} processing failed with exit code {e.returncode}.")
        print(f"Command executed: {' '.join(e.cmd)}")
        print(f"\n--- {executable_name} Standard Output (on error) ---")
        print(e.stdout)
        print(f"\n--- {executable_name} Standard Error (on error) ---")
        print(e.stderr)
        return
    except FileNotFoundError:
        print(f"\nError: {executable_name} executable not found at '{executable_name}'.")
        print(f"Please ensure {executable_name} is installed and its path is correctly set.")
        return
    except Exception as e:
        print(f"\nAn unexpected error occurred during GWB execution: {e}")
        return


def gwb_rss(
    input_dir: Union[str, Path],
    output_dir: Union[str, Path],
    conn_8: bool = True
) -> None:
    """
    Processes GeoTIFF file using the GWB_RSS module for Restoration Status Summary analysis.
    The input files are expected to be 8-bit unsigned with specific values:
            0: missing/NoData (optional)
            1: background
            2: foreground
	The resulted output is a tabular summary statistics in CSV format.

    Args:
        input_dir (str | Path): Path of the input TIFF files.
        output_dir (str | Path): Path to the directory where results will be saved.
                                 Note: the module requires this directory to be empty.
        conn_8 (bool, optional): Foreground connectivity.
                                 True = 8-connectivity (default),
                                 False = 4-connectivity.

    Returns:
        None: The function primarily communicates success/failure and output
              through printed messages and writes files to the specified `output_dir`.
    """
    
    # Paths
    input_dir_path = Path(input_dir)
    output_dir_path = Path(output_dir)

    # Parameter file
    param_file_path = input_dir_path / "rss-parameters.txt"

    # Parameter values
    conn_val = '8' if conn_8 else '4'

    # List of parameter in order for the TXY
    param_file_content = [
        conn_val
    ]

    # Build the command line here
    gwb_executable = "GWB_RSS"
    command = [
        gwb_executable,
        '-i=' + str(input_dir_path),
        '-o=' + str(output_dir_path)
    ]

    _gwb_common(
        command=command,
        param_file_path=param_file_path,
        param_file_content=param_file_content,
        blank_lines=13
    )


def gwb_mspa(
    input_dir: Union[str, Path],
    output_dir: Union[str, Path],
    conn_8: bool = True,
    edge_width: int = 1,
    transition: bool = True,
    int_ext: bool = True,
    save_ram: bool = False,
    stats: bool = False
) -> None:
    """
    Processes GeoTIFF file using the GWB_MSPA module for Morphological Spatial Pattern Analysis.
    The input files are expected to be 8-bit unsigned with specific values:
            0: missing/NoData (optional)
            1: background
            2: foreground
	The resulted outputs are:
			TIFF map: classified with MSPA classes.
			TXT file: with summary statistics.

    Args:
        input_dir (str | Path): Path of the input TIFF files.
        output_dir (str | Path): Path to the directory where results will be saved.
                                 Note: the module requires this directory to be empty.
        conn_8 (bool, optional): Foreground connectivity.
                                 True = 8-connectivity (default), False = 4-connectivity.
        edge_width (int, optional): 1 (default) or larger integer values
        transition (bool): show transition pixels.
                           True = enable (default), False = disable
        int_ext (bool): distinguish between internal and external features
                        True = enable (default), False = disable
        save_ram (bool, optional): -20% RAM but +40% processing time.
                                   True = enable, False = disable (default)
        stats (bool, optional): add summary statistics
                                True = enable, False = disable (default).

    Returns:
        None: The function primarily communicates success/failure and output
              through printed messages and writes files to the specified `output_dir`.
    """
    # Paths
    input_dir_path = Path(input_dir)
    output_dir_path = Path(output_dir)

    # Parameter file
    param_file_path = input_dir_path / "mspa-parameters.txt"

    # Parameter values
    conn_val = '8' if conn_8 else '4'
    transition_val = '1' if transition else '0'
    int_ext_val = '1' if int_ext else '0'
    save_ram_val = '1' if save_ram else '0'
    stats_val = '1' if stats else '0'

    # List of parameters in order for the TXT
    param_file_content = [
        conn_val,
        str(edge_width),
        transition_val,
        int_ext_val,
        save_ram_val,
        stats_val
    ]

    # Build the command line here
    gwb_executable = "GWB_MSPA"
    command = [
        gwb_executable,
        '-i=' + str(input_dir_path),
        '-o=' + str(output_dir_path)
    ]

    _gwb_common(
        command=command,
        param_file_path=param_file_path,
        param_file_content=param_file_content,
        blank_lines=26
    )


def gwb_acc(
    input_dir: Union[str, Path],
    output_dir: Union[str, Path],
    pix_res: int,
    thresh: List[int],
    conn_8: bool = True,
    out_opt: bool = True,
    big3pink: bool = False
) -> None:
    """
    Processes GeoTIFF files using the GWB_ACC module for Accounting analysis.
    The input files are expected to be 8-bit unsigned with specific values:
            0: missing/NoData (optional)
            1: background
            2: foreground
            3: special background 1 (optional)
            4: special background 2 (optional)
	The resulted outputs are:
			TIFF map: classified with ACC classes.
			TXT file: with summary statistics.

    Args:
        input_dir (str | Path): Path of the input TIFF files.
        output_dir (str | Path): Path to the directory where results will be saved.
                                 Note: the module requires this directory to be empty.
        pix_res (int): Spatial pixel resolution in meters.
        thresh (list[int]): Up to 5 area thresholds [unit: pixels] in increasing order.
                            E.g., [200, 2000, 20000, 100000, 200000].
        conn_8 (bool, optional): Foreground connectivity.
                                 True = 8-connectivity (default), False = 4-connectivity.
        out_opt (bool, optional): Output options.
                                  True = stats + image of viewport (default).
                                  False = stats + images of ID, area, viewport (requires much more CPU/RAM!).
        big3pink (bool, optional): Show 3 largest objects in pink color.
                                   True = enable, False = disable (default).

    Returns:
        None: The function primarily communicates success/failure and output
              through printed messages and writes files to the specified `output_dir`.
    """

    # Paths
    input_dir_path = Path(input_dir)
    output_dir_path = Path(output_dir)

    # Parameter file
    param_file_path = input_dir_path / "acc-parameters.txt"

    # Parameter values
    conn_val = '8' if conn_8 else '4'
    out_opt_val = 'default' if out_opt else 'detailed'
    big3pink_val = '1' if big3pink else '0'
    thresh_str = ' '.join(str(x) for x in thresh)

    # List of parameter in order for the TXT
    param_file_content = [
        conn_val,
        str(pix_res),
        thresh_str,
        out_opt_val,
        big3pink_val
    ]

    # Build the command line here
    gwb_executable = "GWB_ACC"
    command = [
        gwb_executable,
        '-i=' + str(input_dir_path),
        '-o=' + str(output_dir_path)
    ]

    _gwb_common(
        command=command,
        param_file_path=param_file_path,
        param_file_content=param_file_content,
        blank_lines=24
    )

def gwb_frag(
    input_dir: Union[str, Path],
    output_dir: Union[str, Path],
    pix_res: int,
    window_size: Union[str, List[int]],
    method: str, 
    precision: bool = True,
    conn_8: bool = True,
    stats: bool = False,
    input_map: str = 'Binary'
) -> None:
    """
    Processes GeoTIFF files using the GWB_FRAG module for Fragmentation analysis.
    The input files are expected to be 8-bit unsigned binary OR grayscale
        - Binary: 
            0: missing/NoData (optional)
            1: background
            2: foreground   
            3: special background 1 (optional)
            4: special background 2 (optional)
        - Grayscale: grayt = grayscale threshold in [1,100]
           [0, grayt-1]: background
           [grayt, 100]: foreground
           103: special background (optional) 
           104: non-fragmenting background (optional)
           255: missing/NoData (optional)
	The resulted outputs are:
			TIFF map: classified with FRAG classes.
			TXT file: with summary statistics.
            CSV file: with summary statistics.
            PNG file: histogram of the foreground pixels.
            SAV file: binary format of statistics.

    Args:
        input_dir (str | Path): Path of the input TIFF files.
        output_dir (str | Path): Path to the directory where results will be saved.
                                 Note: the module requires this directory to be empty.
        pix_res (int): Spatial pixel resolution in meters.
        window_size (int | list[int]): 1 to 10 window sizes (unit: pixels, uneven within [3, 501] ) 
                                       in increasing order, e.g. [3, 5, 11, 17].       
        method (str): conbination of 
		              Three methods to analyze the Foreground (FG) pixels:
                         * FAD (FG Area Density)
                         * FED (FG Edge Density)
                         * FAC (FG Area Clustering)
                      Two per-pixel reporting, color-coded into 5 or 6 fragmentation classes:
                         * _5
                         * _6
                      Or two per-patch reporting, color-coded into 2 or 5 classes
                         * -APP_2
                         * -APP_5
                      E.g.: FAD/FED/FAC_5/6, FAD-APP/FED-APP/FAC-APP_2/5
        precision (bool, optional): True = 1-float precision (default), False = 0-rounded byte
        conn_8 (bool, optional): Foreground connectivity.
                                 True = 8-connectivity (default), False = 4-connectivity.
        stats (bool, optional): add summary statistics 
                                True = enable, False = disable (default).
        input_map (str, optional): Input map type
                                   "Binary" (default), "Grayscale grayt" (e.g., "Grayscale 30")

    Returns:
        None: The function primarily communicates success/failure and output
              through printed messages and writes files to the specified `output_dir`.
    """

    # Paths
    input_dir_path = Path(input_dir)
    output_dir_path = Path(output_dir)

    # Parameter file
    param_file_path = input_dir_path / "frag-parameters.txt"

    # Parameter values
    conn_val = '8' if conn_8 else '4'
    precision_val = '1' if precision else '0'
    stats_val =  '1' if stats else '0'

    if isinstance(window_size, int):
        window_size_str = str(window_size)
    elif isinstance(window_size, list):
        window_size_str = ' '.join(str(x) for x in window_size)

    # List of parameter in order for the TXT
    param_file_content = [
        method,
        conn_val,
        str(pix_res),
        window_size_str,
        precision_val,
        stats_val,
		input_map
    ]

    # Build the command line here
    gwb_executable = "GWB_FRAG"
    command = [
        gwb_executable,
        '-i=' + str(input_dir_path),
        '-o=' + str(output_dir_path)
    ]

    _gwb_common(
        command=command,
        param_file_path=param_file_path,
        param_file_content=param_file_content,
        blank_lines=36
    )

def gwb_dist(
    input_dir: Union[str, Path],
    output_dir: Union[str, Path],
    eucl_hysom: bool,
    conn_8: bool = True
) -> None:
    """
    Processes GeoTIFF file using the GWB_DIST module for Euclidean Distance analysis.
    The input files are expected to be 8-bit unsigned with specific values:
            0: missing/NoData (optional)
            1: background
            2: foreground
	The resulted outputs are:
			TIFF map: classified with ACC classes.
			TXT file: with listing the Euclidean distance frequency distribution.
			PNG file: with the histogram of the Euclidean distance frequency distribution.
			CSV file: listing the Hypsometric Curve summary attributes.
			PNG file: with the histogram of the Hypsometric Curve.

    Args:
        input_dir (str | Path): Path of the input TIFF files.
        output_dir (str | Path): Path to the directory where results will be saved.
                                 Note: the module requires this directory to be empty.
        eucl_hysom (bool): Euclidean Distance only or including Hysometric Curve.
                           True = Euclidean Distance only, 
                           False = Euclidean Distance + Hysometric Curve.
        conn_8 (bool, optional): Foreground connectivity.
                                 True = 8-connectivity (default), 
                                 False = 4-connectivity.

    Returns:
        None: The function primarily communicates success/failure and output
              through printed messages and writes files to the specified `output_dir`.
    """

	# Paths
    input_dir_path = Path(input_dir)
    output_dir_path = Path(output_dir)
	
	# Parameter file
    param_file_path = input_dir_path / "dist-parameters.txt"
	
	# Parameter values
    eucl_hysom_val = '1' if eucl_hysom else '2'  
    conn_val = '8' if conn_8 else '4'

    # List of parameter in order for the TXT
    param_file_content = [
        conn_val,
        eucl_hysom_val,
    ]

    # Build the command line here
    gwb_executable = "GWB_DIST"
    command = [
        gwb_executable,
        '-i=' + str(input_dir_path),
        '-o=' + str(output_dir_path)
    ]

    _gwb_common(
        command=command,
        param_file_path=param_file_path,
        param_file_content=param_file_content,
        blank_lines=16
    )

def gwb_lm(
    input_dir: Union[str, Path],
    output_dir: Union[str, Path],
    kdim: Union[str, List[int]]
) -> None:
    """
    Processes GeoTIFF file using the GWB_LM module for Landscape Mosaic analysis.
    The input files are expected to be 8-bit unsigned with specific values:
            0: missing/NoData (optional)
            1: Agriculture
            2: Natural
			3: Developed
	The resulted outputs are:
			TIFF map: classified with LM 19 classes.
			TIFF map: classified with LM 103 classes.
			PNG file: "heatmap" with a visual summary of occurrence frequency in the 103 sub-spaces.
			CSV file: "heatmap"with precise occurrence frequency values of 103 sub-spaces.
			SAV file: "heatmap"binary encoded summary data for potential change analysis.
			PNG file: with the legend of the heatmap.
			
    Args:
        input_dir (str | Path): Path of the input TIFF files.
        output_dir (str | Path): Path to the directory where results will be saved.
                                 Note: the module requires this directory to be empty.
        kdim (int | list[int]): 1 to 10 window sizes (unit: pixels, uneven within [3, 501] ) 
                                in increasing order, e.g. [3, 5, 11, 17].        

    Returns:
        None: The function primarily communicates success/failure and output
              through printed messages and writes files to the specified `output_dir`.
    """

	# Paths
    input_dir_path = Path(input_dir)
    output_dir_path = Path(output_dir)
	
	# Parameter file
    param_file_path = input_dir_path / "lm-parameters.txt"
	
	# Parameter values
    if isinstance(kdim, int):
        kdim_str = str(kdim)
    elif isinstance(kdim, list):
        kdim_str = ' '.join(str(x) for x in kdim)

    # List of parameter in order for the TXT
    param_file_content = [
        kdim_str
    ]

    # Build the command line here
    gwb_executable = "GWB_LM"
    command = [
        gwb_executable,
        '-i=' + str(input_dir_path),
        '-o=' + str(output_dir_path)
    ]

    _gwb_common(
        command=command,
        param_file_path=param_file_path,
        param_file_content=param_file_content,
        blank_lines=13
    )

def gwb_parc(
    input_dir: Union[str, Path],
    output_dir: Union[str, Path],
    conn_8: bool = True
) -> None:
    """
    Processes GeoTIFF file using the GWB_PARC module for Parcellation analysis.
    The input files are expected to be 8-bit unsigned with these values:
            0: missing/NoData (optional)
			[1-255]: at least two different landcover classes
	The resulted outputs are:
			CSV file: with summary statistics.
			TXT file: with summary statistics.
			
    Args:
        input_dir (str | Path): Path of the input TIFF files.
        output_dir (str | Path): Path to the directory where results will be saved.
                                 Note: the module requires this directory to be empty.
        conn_8 (bool, optional): Foreground connectivity.
                                 True = 8-connectivity (default), False = 4-connectivity.   

    Returns:
        None: The function primarily communicates success/failure and output
              through printed messages and writes files to the specified `output_dir`.
    """

	# Paths
    input_dir_path = Path(input_dir)
    output_dir_path = Path(output_dir)
	
	# Parameter file
    param_file_path = input_dir_path / "parc-parameters.txt"
	
	# Parameter values
    conn_val = '8' if conn_8 else '4'

    # List of parameter in order for the TXT
    param_file_content = [
        conn_val
    ]

    # Build the command line here
    gwb_executable = "GWB_PARC"
    command = [
        gwb_executable,
        '-i=' + str(input_dir_path),
        '-o=' + str(output_dir_path)
    ]

    _gwb_common(
        command=command,
        param_file_path=param_file_path,
        param_file_content=param_file_content,
        blank_lines=16
    )


def gwb_rec(
    input_dir: Union[str, Path],
    output_dir: Union[str, Path],
    classes: List[Tuple[int, int]]
) -> None:
    """
    Processes GeoTIFF file using the GWB_REC module for Recoding of categorical class values.
    The input files are expected to be 8-bit unsigned within the range [0, 255].
	The resulted outputs files are TIFF maps re-coded.

    Args:
        input_dir (str | Path): Path of the input TIFF files.
        output_dir (str | Path): Path to the directory where results will be saved.
                                 Note: the module requires this directory to be empty.
        classes (list[tuple[int, int]]): list of tuples (max 255) with pairs of class values (old value, new value).
										 E.g.: [(1,4), (2,5), (3,6)]

    Returns:
        None: The function primarily communicates success/failure and output
              through printed messages and writes files to the specified `output_dir`.
    """

	# Paths
    input_dir_path = Path(input_dir)
    output_dir_path = Path(output_dir)
	
	# Parameter file
    param_file_path = input_dir_path / "rec-parameters.txt"
	
	# Parameter values
    classes_val = [" ".join(map(str, t)) for t in classes]

    # List of parameter in order for the TXT
    param_file_content = classes_val

    # Build the command line here
    gwb_executable = "GWB_REC"
    command = [
        gwb_executable,
        '-i=' + str(input_dir_path),
        '-o=' + str(output_dir_path)
    ]

    _gwb_common(
        command=command,
        param_file_path=param_file_path,
        param_file_content=param_file_content,
        blank_lines=22
    )


def gwb_sc(
    input_dir: Union[str, Path],
    output_dir: Union[str, Path],
    R: int = 1,
	W: int = 27,
	A: int = 0,
	B: int = 0,
	H: bool = True,
	F: bool = False
) -> None:
    """
    Processes GeoTIFF file using the GWB_SC module for the spatial convolution program SpatCon.
    The input files are expected to be 8-bit unsigned with these values:
            0: missing/NoData (optional)
			[1-255]: at least two different landcover classes
	The resulted outputs are processed TIFF maps according the mapping rules.

    Args:
        input_dir (str | Path): Path of the input TIFF files.
        output_dir (str | Path): Path to the directory where results will be saved.
                                 Note: the module requires this directory to be empty.
		R (int, optional): list of mapping rule:
							1 = Majority (most frequent) pixel value (default)
							6 = Landscape mosaic (19-class version)
							7 = Landscape mosaic (103-class version)
							10 = Number of unique pixel values
							20 = Median pixel value
							21 = Mean pixel value
							5x = Pixel diversity:
							   51 = Gini-Simpson pixel diversity
							   52 = Gini-Simpson pixel evenness
							   53 = Shannon pixel evenness
							   54 = Pmax
							7x = Pixel adjacency (with regard to order of pixels in pairs):
							   71 = Angular second moment
							   72 = Gini-Simpson adjacency evenness
							   73 = Shannon adjacency evenness
							   74 = Sum of diagonals
							   75 = Proportion of total adjacencies involving a specific pixel value
							   76 = Proportion of total adjacencies which are between two specific pixel values
							   77 = Proportion of adjacencies involving a specific pixel value which are adjacencies with that same pixel value
							   78 = Proportion of adjacencies involving a specific pixel value which are adjacencies
										between that pixel value and another specific pixel value
							8x = Pixel value density and ratios
							   81 = Area density
							   82 = Ratio of the frequencies of two specified pixel values
							   83 = Combined ratio of two specific pixel values
		W (int, optional): window size, minimum value = 3, maximum value < of the input map pixel sizes (x,y). (Default = 27)
		A (int, optional): first target code, required for mapping rules 75, 76, 77, 78, 81, 82, 83. Default = 0
		B (int, optional): second target code, required for mapping rules 76, 78, 82, 83. Default = 0
		H (bool, optional): ignore missing values or adjacencies (no effect for mapping rules 21, 82, 83): 
						    True = ignore (default), False = include. 
		F (bool, optional): output in 32-bit float raster file
							True = 32-bit float. False = 8-bit byte (default)

    Returns:
        None: The function primarily communicates success/failure and output
              through printed messages and writes files to the specified `output_dir`.
    """

	# Paths
    input_dir_path = Path(input_dir)
    output_dir_path = Path(output_dir)
	
	# Parameter file
    param_file_path = input_dir_path / "sc-parameters.txt"
	
	# Parameter values
    H_val = '1' if H else '2'
    F_val = '1' if F else '0'

    # List of parameter in order for the TXT
    param_file_content = [
        'R ' + str(R),
		'W ' + str(W),
		'A ' + str(A),
		'B ' + str(B),
		'H ' + H_val,
		'F ' + F_val,
		'Z 0',
		'M 0'
    ]

    # Build the command line here
    gwb_executable = "GWB_SC"
    command = [
        gwb_executable,
        '-i=' + str(input_dir_path),
        '-o=' + str(output_dir_path)
    ]

    _gwb_common(
        command=command,
        param_file_path=param_file_path,
        param_file_content=param_file_content,
        blank_lines=52
    )


def gwb_gsc(
    input_dir: Union[str, Path],
    output_dir: Union[str, Path],
    M: int,
	F: bool,
	G: bool,
	P: bool = False,
	W: int = 0,
	A: bool = True,
	B: int = 1,
	X: int = 0,
	Y: int = 0,
	K: int = 0
) -> None:
    """
    Processes GeoTIFF file using the GWB_GSC module for the spatial convolution program GraySpatCon.
    The input files are expected to be 8-bit unsigned with the values:
			[0-100]
			255: missing/NoData (optional)
	The resulted outputs are processed TIFF maps according the mapping rules and can be:
			8-bit unsigned with 255 missing/NoData
			32-bit float with -0.01 as missing/NoData
			32-but float with -9000000.0 as missing/NoData for metrics 44, 45, and 50.
	
    Args:
        input_dir (str | Path): Path of the input TIFF files.
        output_dir (str | Path): Path to the directory where results will be saved.
                                 Note: the module requires this directory to be empty.
		M (int): metric selection:
				 1 = Mean							27 = Range
				 2 = EvennessOrderedAdj				28 = Dissimilarity
				 3 = EvennessUnorderedAdj			29 = Contrast
				 4 = EntropyOrderedAdj				30 = UniformityOrderedAdj
				 5 = EntropyUnorderedAdj			31 = UniformityUnorderedAdj
				 6 = DiagonalContagion				32 = Homogeneity
				 7 = ShannonDiversity				33 = InverseDifference
				 8 = ShannonEvenness				34 = SimilarityRMax
				 9 = Median							35 = SimilarityRGlobal
				10 = GSDiversity					36 = SimilarityRWindow
				11 = GSEvenness						37 = DominanceOrderedAdj
				12 = EquitabilityOrderedAdj			38 = DominanceUnorderedAdj
				13 = EquitabilityUnorderedAdj		39 = DifferenceEntropy
				14 = DiversityOrderedAdj			40 = DifferenceEvenness
				15 = DiversityUnorderedAdj			41 = SumEntropy
				16 = Majority						42 = SumEvenness
				17 = LandscapeMosaic19				43 = AutoCorrelation
				18 = LandscapeMosaic103				44 = Correlation
				19 = NumberGrayLevels				45 = ClusterShade
				20 = MaxAreaDensity					46 = ClusterProminence
				21 = FocalAreaDensity				47 = RootMeanSquare
				22 = FocalAdjT1						48 = AverageAbsDeviation
				23 = FocalAdjT1andT2				49 = kContagion
				24 = FocalAdjT1givenT2				50 = Skewness
				25 = StandardDeviation				51 = Kurtosis
				26 = CoefficientVariation			52 = Clustering
		F (bool): output in 32-bit float raster file. F = True for metrics 44, 45, 50.
				  True = 32-bit float. False = 8-bit.
		G (bool): analysis with moving windows or the entire map.
				  True = moving windows. False = entire map.
		P (bool, optional): consider zero as missing/NoData and exclude 0 pixels from the process.
				            True = exclude zero. False = include zero (default)
		W (int, optional): window size expressed as the number of pixels on the side of a NxN window (e.g., N = 5 for 5x5).
						   N must be positive integer > 1 (eg, 3,5,7,9...), maximum < x or y dimension of input map
                           Required if G = True.
		A (bool, optional)): mask the value 255 missing/NoData of the input map on output map. 
							 This option works only with moving windows analysus: G = True.
							 True = mask on output. False = do not mask on output.
		B (int, optional): byte stretch if converting to bytes. Required if the output is 8-bit: F = False:
						   For metrics bounded in [0.0, 1.0] only: (metrics 2, 3, 6, 8, 10-15, 20-24, 31-38, 40, 42, 49)
							  1 = From metric value in [0.0, 1.0] to byte in [0, 100]
							  2 = From metric value in [0.0, 1.0] to byte in [0, 254]
						   For all metrics except 1, 9, 16, 17, 18, 19, 25, 27:
							  3 = From metric value in [Min, Max] to byte in [0, 254]
							  4 = From metric value in [0.0, Max] to byte in [0, 254]
							  5 = From metric value in [0.0, Max] to byte in [0, 100]
							  Where Min and Max are the observed minimum and observed maximum values
							  over the entire output image
						   For metrics 1, 9, 16, 17, 18, 19, 25, 27 only:
							  6 = No stretch allowed; the metric value is converted to byte		
		X (int, optional): target code 1 (t1), with N in a range [0, 100]. Required for metrics 21, 22, 23, 24.
		Y (int, optional): target code 2 (t2), with N in a range [0, 100]. Required for metrics 23, 24.
		K (int, optional): target difference level (k*), with N in a range [0, 100]. Required for metric 49.

    Returns:
        None: The function primarily communicates success/failure and output
              through printed messages and writes files to the specified `output_dir`.
    """

	# Paths
    input_dir_path = Path(input_dir)
    output_dir_path = Path(output_dir)
	
	# Parameter file
    param_file_path = input_dir_path / "gsc-parameters.txt"
	
	# Parameter values
    F_val = '2' if F else '1'
    G_val = '0' if G else '1'
    P_val = '1' if P else '0'
    A_val = '1' if A else '0'

    # List of parameter in order for the TXT
    param_file_content = [
        'M ' + str(M),
		'F ' + F_val,
		'G ' + G_val,
		'P ' + P_val,
		'W ' + str(W),
		'A ' + A_val,
		'B ' + str(B),
		'X ' + str(X),
		'Y ' + str(Y),
		'K ' + str(K)
    ]

    # Build the command line here
    gwb_executable = "GWB_GSC"
    command = [
        gwb_executable,
        '-i=' + str(input_dir_path),
        '-o=' + str(output_dir_path)
    ]

    _gwb_common(
        command=command,
        param_file_path=param_file_path,
        param_file_content=param_file_content,
        blank_lines=121
    )


def gwb_spa(
    input_dir: Union[str, Path],
    output_dir: Union[str, Path],
	classes: int,
    stats: bool = False
) -> None:
    """
    Processes GeoTIFF file using the GWB_SPA module for Simplified Spatial Pattern Analysis.
    The input files are expected to be 8-bit unsigned with specific values:
            0: missing/NoData (optional)
            1: background
            2: foreground
	The resulted outputs are TIFF maps classified with SPA classes.

    Args:
        input_dir (str | Path): Path of the input TIFF files.
        output_dir (str | Path): Path to the directory where results will be saved.
                                 Note: the module requires this directory to be empty.
		classes (int): number of pattern classes:
						2 = SLF, Contiguous
						3 = Core, Core-Openings, Margin
						5 = Core, Core-Openings, Edge, Perforation, Margin
						6 = Core, Core-Openings, Edge, Perforation, Islet, Margin		
        stats (bool, optional): add summary statistics
                                True = enable, False = disable (default).

    Returns:
        None: The function primarily communicates success/failure and output
              through printed messages and writes files to the specified `output_dir`.
    """
    # Paths
    input_dir_path = Path(input_dir)
    output_dir_path = Path(output_dir)

    # Parameter file
    param_file_path = input_dir_path / "spa-parameters.txt"

    # Parameter values
    stats_val = '1' if stats else '0'

    # List of parameters in order for the TXT
    param_file_content = [
		str(classes),
        stats_val
    ]

    # Build the command line here
    gwb_executable = "GWB_SPA"
    command = [
        gwb_executable,
        '-i=' + str(input_dir_path),
        '-o=' + str(output_dir_path)
    ]

    _gwb_common(
        command=command,
        param_file_path=param_file_path,
        param_file_content=param_file_content,
        blank_lines=20
    )


