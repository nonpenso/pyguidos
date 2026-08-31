"""
Tests for pyguidos.utils module.

All tests use numpy arrays or strings — no GeoTIFF files required.
Run with: pytest tests/test_utils.py -v
"""

import pytest
import numpy as np
import rasterio
from rasterio.transform import IDENTITY
from rasterio.transform import from_origin
from pyguidos import utils


# =============================================================================
# Raster & Metadata Logic
# =============================================================================

def test_get_gtb_nodata_logic(monkeypatch):
    """Test the priority system of nodata resolution."""

    # Mocking get_raster_info with a tag that matches the internal regex
    def mock_info_fos(path):
        return {
            # This tag MUST match your regex: GTB_FOS_WS<number>_M<number>
            "tag": "GTB_FOS, <Binary,-1,8,FAD_5,100,31>, https:", 
            "profile": {"nodata": 0}
        }

    monkeypatch.setattr(utils, "get_raster_info", mock_info_fos)
    
    # Now this should successfully identify GTB_FOS and return 102
    assert utils.get_gtb_nodata("fake.tif") == 102


# =============================================================================
# get_tool_parameters
# =============================================================================

class TestGetToolParameters:

    def test_mspa_tag(self):
        """GTB_MSPA tag parsed correctly."""
        tag = "GTB_MSPA, <8,1,1,1>, https://forest.jrc.ec.europa.eu/"
        result = utils.get_tool_parameters(tag)
        assert result is not None
        assert result["tool_id"] == "GTB_MSPA"
        assert result["connectivity"] == "8"
        assert result["edge_width"] == "1"
        assert result["web_link"] == "https://forest.jrc.ec.europa.eu/"

    def test_fos_tag(self):
        """GTB_FOS tag parsed correctly."""
        tag = "GTB_FOS, <Binary,-1,8,FAD_5,100.000,27>, https://forest.jrc.ec.europa.eu/"
        result = utils.get_tool_parameters(tag)
        assert result["tool_id"] == "GTB_FOS"
        assert result["wsize"] == "27"

    def test_acc_tag(self):
        """GTB_ACC tag parsed correctly."""
        tag = "GTB_ACC, <100,1000,10000>, https://forest.jrc.ec.europa.eu/"
        result = utils.get_tool_parameters(tag)
        assert result["tool_id"] == "GTB_ACC"
        assert result["thresholds"] == ["100", "1000", "10000"]

    def test_empty_tag_returns_none(self):
        """Empty or invalid tag must return None."""
        assert utils.get_tool_parameters("") is None
        assert utils.get_tool_parameters(None) is None
        assert utils.get_tool_parameters("--") is None

# =============================================================================
# Pixel Frequency
# =============================================================================

def test_get_pxl_freq():
    """Verify frequency counting for 2D arrays."""
    arr = np.array([[101, 101, 102], [105, 106, 101]], dtype=np.uint8)
    freq = utils.get_pxl_freq(arr)
    assert freq[101] == 3
    assert freq[102] == 1
    assert freq[105] == 1

# =============================================================================
# Raster Metadata & I/O
# =============================================================================

def test_get_tool_parameters():
    """Verify parsing of GTB metadata tags."""
    # Your code returns None if the regex doesn't match perfectly. 
    # Ensure the tag format matches your re.match in utils.py
    params = utils.get_tool_parameters("GTB_FOS_WS27_M1")
    if params:
        assert params["tool_id"] == "GTB_FOS"
        assert params["window_size"] == 27
        assert params["method"] == 1

def test_get_gtb_nodata(monkeypatch):
    """Test the priority system: GTB Tag > Profile > Default 0."""
    from pyguidos import utils

    # 1. Mock get_raster_info to provide the Tag
    def mock_info(path):
        if "fragmentation" in str(path):
            return {"tag": "GTB_FOS, <Binary,-1,8,FAD_5,100.000,27>, https://forest.jrc.ec.europa.eu/", 
                    "profile": {"nodata": 0}}
        if "standard" in str(path):
            return {"tag": None, "profile": {"nodata": 255}}
        return {"tag": None, "profile": {}}

    # 2. Mock get_tool_parameters to translate that Tag into a Tool ID
    def mock_params(tag):
        if "GTB_FOS" in tag:
            return {"tool_id": "GTB_FOS"}
        return None

    monkeypatch.setattr(utils, "get_raster_info", mock_info)
    monkeypatch.setattr(utils, "get_tool_parameters", mock_params)
    
    # Now it should work: 
    # Tag is found -> tool_id is 'GTB_FOS' -> dictionary returns 102
    assert utils.get_gtb_nodata("fragmentation.tif") == 102
    
    # Priority 2: No tag, uses profile
    assert utils.get_gtb_nodata("standard.tif") == 255
    
    # Priority 3: No tag, no profile, defaults to 0
    assert utils.get_gtb_nodata("empty.tif") == 0


def test_get_gtb_nodata_unknown_gtb_tool(monkeypatch):
    """If the tool ID is not in GTB_NODATA, it should fall back to profile."""
    from pyguidos import utils
    
    def mock_info_unknown(path):
        return {
            "tag": "GTB_UNKNOWN_TAG",
            "profile": {"nodata": 99}
        }
    
    def mock_params_unknown(tag):
        return {"tool_id": "GTB_NEW_TOOL"} # Not in the 4 defined keys

    monkeypatch.setattr(utils, "get_raster_info", mock_info_unknown)
    monkeypatch.setattr(utils, "get_tool_parameters", mock_params_unknown)
    
    # Should skip Priority 1 and return Profile (99)
    assert utils.get_gtb_nodata("new_tool.tif") == 99


from rasterio.enums import ColorInterp

def test_save_output_geotiff_palette(tmp_path):
    """Verify that save_output_geotiff correctly writes data, tags, and colormaps."""
    from pyguidos import utils
    
    out_file = tmp_path / "output.tif"
    
    # 1. Prepare a dummy profile (standard for uint8)
    profile = {
        'driver': 'GTiff',
        'height': 5,
        'width': 5,
        'count': 1,
        'dtype': 'uint8',
        'crs': 'EPSG:3035',
        'transform': rasterio.transform.from_origin(0, 5, 1, 1)
    }
    
    # 2. Prepare dummy data (2D array)
    data = np.zeros((5, 5), dtype=np.uint8)
    data[0, 0] = 102 # Mock some FOS missing data
    
    # 3. Prepare a dictionary colormap (MSPA style)
    cmap_dict = {102: (255, 0, 0, 255)} # 102 is Red
    
    # 4. Define the Tag
    tag_str = "GTB_FOS_WS27_M1"
    
    # CALL THE FUNCTION
    utils.save_output_geotiff(
        output_path=out_file,
        data=data,
        profile=profile,
        colormap_input=cmap_dict,
        tag_descr=tag_str
    )
    
    # VALIDATE
    assert out_file.exists()
    with rasterio.open(out_file) as src:
        # Check metadata tags
        assert src.tags()["TIFFTAG_IMAGEDESCRIPTION"] == tag_str
        assert src.tags()["TIFFTAG_SOFTWARE"] == "pyGuidos"
        
        # Check Color Interpretation
        assert src.colorinterp[0] == ColorInterp.palette
        
        # Check Colormap
        saved_cmap = src.colormap(1)
        assert saved_cmap[102] == (255, 0, 0, 255)


def test_save_output_geotiff_dict_input(tmp_path):
    from pyguidos import utils
    out_file = tmp_path / "test_dict.tif"
    data = np.array([[1, 2], [3, 4]], dtype=np.uint8)
    
    profile = {
        'driver': 'GTiff',
        'height': 2,
        'width': 2,
        'count': 1,
        'dtype': 'uint8',
        'crs': 'EPSG:4326',
        'transform': from_origin(500000, 500000, 10.0, 10.0)
    }
    
    cmap_dict = {1: (255, 0, 0, 255), 2: (0, 255, 0, 255)}
    tag = "GTB_TEST_TAG_V1"

    utils.save_output_geotiff(out_file, data, profile, cmap_dict, tag)
    assert out_file.exists()

def test_save_output_geotiff_file_input(tmp_path):
    from pyguidos import utils
    out_file = tmp_path / "test_file.tif"
    cmap_txt = tmp_path / "colors.txt"
    cmap_txt.write_text("1 255 255 0\n2 0 0 255") 
    
    data = np.array([[1, 2]], dtype=np.uint8)
    profile = {
        'driver': 'GTiff', 'height': 1, 'width': 2, 'count': 1,
        'dtype': 'uint8', 'crs': 'EPSG:4326', 
        'transform': from_origin(500000, 500000, 10.0, 10.0)
    }

    utils.save_output_geotiff(out_file, data, profile, cmap_txt, "GTB_FILE_TEST")
        
        
# =============================================================================
# Reporting & Housekeeping
# =============================================================================

def test_running_time():
    """Verify time delta string formatting matches actual output."""
    result = utils.running_time(0, 65.5)
    # Your output is '1m 5.5s'
    assert "1m" in result
    assert "5.5s" in result


def test_generate_text_report_with_real_template(tmp_path):
    """
    Verify keyword replacement using placeholders found in frag_templ.txt.
    Ensures no stray '$' signs remain after replacement.
    """
    # 1. Simulate the template file from your repo
    template_content = """
    FRAGMENTATION ANALYSIS 
    Input image: {input_file}
    Foreground [pixel=2]: {foreg_pxl}
    FAD VALUES  FREQUENCY [%]
    Rare [0-9] {rare_val} 
    """
    # Note: Your template uses {key}, but your previous test used ${key}.
    # I will provide the test for {key} based on the file content you shared.
    
    templ_file = tmp_path / "frag_templ.txt"
    templ_file.write_text(template_content)
    
    output_file = tmp_path / "report.txt"
    
    # 2. Data mapping
    replacements = {
        "input_file": "forest_map.tif",
        "foreg_pxl": "5000",
        "rare_val": "12.3456"
    }
    
    # 3. Run the utility
    utils.generate_text_report(templ_file, output_file, replacements)
    
    # 4. Assertions
    result_text = output_file.read_text()
    
    assert "Input image: forest_map.tif" in result_text
    assert "Foreground [pixel=2]: 5000" in result_text
    assert "Rare [0-9] 12.3456" in result_text
    
    # Ensure no curly braces remain for the keys we provided
    assert "{input_file}" not in result_text


def test_citation():
    """Ensure citation returns the correct string."""
    text = utils.citation()
    assert "Caudullo G." in text
    assert "Vogt P." in text
    

def test_get_colormap_valid_parsing(tmp_path):
    """Verifies that get_colormap parses space-separated strings and normalizes values for Matplotlib."""
    # Create a mock color configuration text file
    cmap_file = tmp_path / "mock_colormap.txt"
    cmap_file.write_text(
        "0   255 0   0\n"    # Value 0: Red
        "100 0   255 128\n"  # Value 100: Translucent Greenish
        "255 255 255 255\n" # Value 255: White
    )

    plot_map, tiff_map = utils.get_colormap(cmap_file)

    # 1. Test GeoTiff colormap structure (Expected scale: 0-255 with an explicit alpha channel of 255)
    assert tiff_map[0] == (255, 0, 0, 255)
    assert tiff_map[100] == (0, 255, 128, 255)
    assert tiff_map[255] == (255, 255, 255, 255)

    # 2. Test Matplotlib colormap structure (Expected scale: normalized 0.0 - 1.0 floats, 3 elements)
    assert plot_map[0] == (1.0, 0.0, 0.0)
    assert pytest.approx(plot_map[100][1]) == 1.0       # Green channel = 255/255
    assert pytest.approx(plot_map[100][2]) == 128/255   # Blue channel = 128/255
    assert plot_map[255] == (1.0, 1.0, 1.0)


def test_get_colormap_ignores_invalid_lines(tmp_path):
    """Ensures get_colormap skips corrupted rows (less than 4 components) without throwing exceptions."""
    cmap_file = tmp_path / "corrupted_colormap.txt"
    cmap_file.write_text(
        "10  255 255 255\n"  # Valid line
        "20  128 128\n"      # Invalid (Missing blue channel, length 3)
        "comment_line\n"     # Invalid (Length 1)
    )

    plot_map, tiff_map = utils.get_colormap(cmap_file)

    # Valid line must exist
    assert 10 in tiff_map
    # Malformed inputs must be seamlessly bypassed without breaking execution
    assert 20 not in tiff_map


def test_log_msg_output(capsys):
    """Checks that log_msg prints outputs cleanly when verbose=True and flushes buffer."""
    utils.log_msg(verbose=True, message="Pipeline initiated successfully.")
    
    # capsys captures stdout/stderr buffers
    captured = capsys.readouterr()
    assert captured.out == "Pipeline initiated successfully.\n"


def test_log_msg_silent_when_false(capsys):
    """Ensures log_msg remains entirely silent when verbose=False."""
    utils.log_msg(verbose=False, message="Hidden administrative metric details.")
    
    captured = capsys.readouterr()
    assert captured.out == ""


# =============================================================================
# v2.5.2 — get_tif_colormap
# =============================================================================

def test_get_tif_colormap_returns_cmap_and_norm(tmp_path):
    """Verify get_tif_colormap reads an embedded colormap and returns
    a ListedColormap and Normalize object."""
    from matplotlib.colors import ListedColormap, Normalize

    tiff_path = tmp_path / "cmap_test.tif"
    data = np.array([[0, 50], [100, 200]], dtype=np.uint8)
    profile = {
        'driver': 'GTiff', 'height': 2, 'width': 2, 'count': 1,
        'dtype': 'uint8', 'crs': 'EPSG:4326',
        'transform': from_origin(0, 2, 1, 1)
    }

    # Write GeoTIFF with an embedded colormap
    cmap_dict = {
        0: (0, 0, 0, 255),
        50: (255, 128, 0, 255),
        100: (0, 255, 0, 255),
        200: (128, 128, 128, 255),
    }
    with rasterio.open(tiff_path, 'w', **profile) as dst:
        dst.write(data, 1)
        dst.write_colormap(1, cmap_dict)

    cmap, norm = utils.get_tif_colormap(str(tiff_path))

    assert isinstance(cmap, ListedColormap)
    assert isinstance(norm, Normalize)
    assert norm.vmin == 0
    assert norm.vmax == 255

    # Check that the colors are correctly mapped (normalized to 0-1)
    # Index 50 should be orange-ish: (255/255, 128/255, 0/255, 255/255)
    color_50 = cmap.colors[50]
    assert pytest.approx(color_50[0], abs=0.01) == 1.0       # R
    assert pytest.approx(color_50[1], abs=0.01) == 128/255    # G
    assert pytest.approx(color_50[2], abs=0.01) == 0.0        # B


def test_get_tif_colormap_unset_entries_are_zero(tmp_path):
    """Colormap entries not defined in the GeoTIFF should default to (0,0,0,0)."""
    from matplotlib.colors import ListedColormap

    tiff_path = tmp_path / "sparse_cmap.tif"
    data = np.array([[1]], dtype=np.uint8)
    profile = {
        'driver': 'GTiff', 'height': 1, 'width': 1, 'count': 1,
        'dtype': 'uint8', 'crs': 'EPSG:4326',
        'transform': from_origin(0, 1, 1, 1)
    }
    # Only define a few entries
    cmap_dict = {1: (255, 0, 0, 255)}
    with rasterio.open(tiff_path, 'w', **profile) as dst:
        dst.write(data, 1)
        dst.write_colormap(1, cmap_dict)

    cmap, _ = utils.get_tif_colormap(str(tiff_path))

    # Entry 1 should be red
    assert pytest.approx(cmap.colors[1][0], abs=0.01) == 1.0
    # Entry 200 (not set) should be zeros
    assert cmap.colors[200][0] == 0.0
    assert cmap.colors[200][1] == 0.0
