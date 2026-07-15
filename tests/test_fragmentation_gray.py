import pytest
import numpy as np
import rasterio
from pathlib import Path
from rasterio.transform import from_origin

from pyguidos import frag_gray, frag_gray_stats


# =============================================================================
# Fixtures
# =============================================================================

@pytest.fixture(scope="module")
def gray_input_tif(tmp_path_factory):
    """Creates a grayscale input GeoTIFF with values 0-100 and 255 (NoData)."""
    tmp_dir = tmp_path_factory.mktemp("frag_gray_data")
    input_tif = tmp_dir / "input_gray.tif"

    # 15x15 array with gradient foreground
    data = np.zeros((15, 15), dtype=np.uint8)
    # Central 9x9 block with varying intensity
    data[3:12, 3:12] = 70
    data[5:10, 5:10] = 100
    data[7, 7] = 50
    # Add NoData pixels
    data[0, 0] = 255
    data[0, 1] = 255
    # Leave edges as 0 (non-foreground)

    with rasterio.open(
        input_tif, 'w', driver='GTiff',
        height=15, width=15, count=1, dtype='uint8',
        crs='EPSG:3035',
        transform=from_origin(0, 15, 1, 1)
    ) as dst:
        dst.write(data, 1)

    return input_tif


@pytest.fixture(scope="module")
def frag_gray_fad_result(gray_input_tif, tmp_path_factory):
    """Runs frag_gray with FAD method."""
    tmp_dir = tmp_path_factory.mktemp("frag_gray_fad_out")
    return frag_gray(
        str(gray_input_tif),
        method='FAD',
        window_size=3,
        for_threshold=1,
        stat_files=True,
        outdir=str(tmp_dir)
    )


@pytest.fixture(scope="module")
def frag_gray_fac_result(gray_input_tif, tmp_path_factory):
    """Runs frag_gray with FAC method."""
    tmp_dir = tmp_path_factory.mktemp("frag_gray_fac_out")
    return frag_gray(
        str(gray_input_tif),
        method='FAC',
        window_size=3,
        for_threshold=1,
        connectivity=4,
        stat_files=True,
        outdir=str(tmp_dir)
    )


@pytest.fixture(scope="module")
def frag_gray_fed_result(gray_input_tif, tmp_path_factory):
    """Runs frag_gray with FED method."""
    tmp_dir = tmp_path_factory.mktemp("frag_gray_fed_out")
    return frag_gray(
        str(gray_input_tif),
        method='FED',
        window_size=3,
        for_threshold=1,
        connectivity=4,
        stat_files=True,
        outdir=str(tmp_dir)
    )


# =============================================================================
# frag_gray() Integration Tests
# =============================================================================

class TestFragGrayFAD:

    def test_returns_dict(self, frag_gray_fad_result):
        assert isinstance(frag_gray_fad_result, dict)
        assert "output paths" in frag_gray_fad_result
        assert "input stats" in frag_gray_fad_result
        assert "output stats" in frag_gray_fad_result

    def test_output_files_exist(self, frag_gray_fad_result):
        paths = frag_gray_fad_result["output paths"]
        assert Path(paths["path tif"]).exists()
        assert Path(paths["path txt"]).exists()
        assert Path(paths["path csv"]).exists()
        assert Path(paths["path png"]).exists()

    def test_input_stats(self, frag_gray_fad_result):
        stats = frag_gray_fad_result["input stats"]
        assert stats["in foreground pxl"] > 0
        assert stats["in background pxl"] >= 0
        assert stats["missing pxl"] >= 0

    def test_output_stats(self, frag_gray_fad_result):
        stats = frag_gray_fad_result["output stats"]
        assert "fad_av" in stats
        assert "avcon" in stats
        assert stats["fad_av"] > 0

    def test_class_freq_sums(self, frag_gray_fad_result):
        freq = frag_gray_fad_result["output stats"]["class freq"]
        total = sum(freq.values())
        assert total > 0


class TestFragGrayFAC:

    def test_returns_dict(self, frag_gray_fac_result):
        assert isinstance(frag_gray_fac_result, dict)
        assert "output stats" in frag_gray_fac_result

    def test_fac_produces_valid_output(self, frag_gray_fac_result):
        stats = frag_gray_fac_result["output stats"]
        assert stats["fad_av"] > 0


class TestFragGrayFED:

    def test_returns_dict(self, frag_gray_fed_result):
        assert isinstance(frag_gray_fed_result, dict)
        assert "output stats" in frag_gray_fed_result

    def test_fed_greater_or_equal_fac(self, frag_gray_fac_result, frag_gray_fed_result):
        """FED should be >= FAC because FG-BG edges get partial credit."""
        fed_av = frag_gray_fed_result["output stats"]["fad_av"]
        fac_av = frag_gray_fac_result["output stats"]["fad_av"]
        assert fed_av >= fac_av


# =============================================================================
# for_threshold Tests
# =============================================================================

class TestFragGrayThreshold:

    def test_threshold_reduces_foreground(self, gray_input_tif, tmp_path_factory):
        """Higher threshold should reduce foreground pixel count."""
        tmp_dir = tmp_path_factory.mktemp("frag_gray_thresh")
        low_dir = tmp_dir / "low"
        high_dir = tmp_dir / "high"
        low_dir.mkdir()
        high_dir.mkdir()

        result_low = frag_gray(
            str(gray_input_tif), method='FAD', window_size=3,
            for_threshold=1, outdir=str(low_dir))

        result_high = frag_gray(
            str(gray_input_tif), method='FAD', window_size=3,
            for_threshold=80, outdir=str(high_dir))

        fg_low = result_low["input stats"]["out foreground pxl"]
        fg_high = result_high["input stats"]["out foreground pxl"]
        assert fg_low > fg_high

    def test_threshold_100_only_max(self, gray_input_tif, tmp_path_factory):
        """Threshold=100 should only keep pixels with value exactly 100."""
        tmp_dir = tmp_path_factory.mktemp("frag_gray_t100")
        result = frag_gray(
            str(gray_input_tif), method='FAD', window_size=3,
            for_threshold=100, outdir=str(tmp_dir))
        # Only pixels with value 100 should be foreground
        assert result["input stats"]["out foreground pxl"] > 0

    def test_invalid_threshold_exits(self):
        with pytest.raises(SystemExit):
            frag_gray("dummy.tif", method='FAD', window_size=3, for_threshold=0)
        with pytest.raises(SystemExit):
            frag_gray("dummy.tif", method='FAD', window_size=3, for_threshold=101)


# =============================================================================
# frag_gray_stats() Tests
# =============================================================================

class TestFragGrayStats:

    def test_standalone_stats(self, frag_gray_fad_result, tmp_path_factory):
        """frag_gray_stats should work on an existing output GeoTIFF."""
        tmp_dir = tmp_path_factory.mktemp("frag_gray_stats_out")
        tiff_path = frag_gray_fad_result["output paths"]["path tif"]

        stats = frag_gray_stats(tiff_path, stat_files=True, outdir=str(tmp_dir))

        assert "output paths" in stats
        assert "output stats" in stats
        assert stats["output stats"]["fad_av"] > 0

    def test_standalone_stats_with_source(self, frag_gray_fad_result,
                                          gray_input_tif, tmp_path_factory):
        """When source_tiff is given, input FG/BG should be computed."""
        tmp_dir = tmp_path_factory.mktemp("frag_gray_stats_src")
        tiff_path = frag_gray_fad_result["output paths"]["path tif"]

        stats = frag_gray_stats(tiff_path, stat_files=True,
                                outdir=str(tmp_dir),
                                source_tiff=str(gray_input_tif))

        assert stats["input stats"]["in foreground pxl"] != "n/a"
        assert stats["input stats"]["in foreground pxl"] > 0

    def test_standalone_stats_without_source(self, frag_gray_fad_result, tmp_path_factory):
        """Without source_tiff, input FG/BG should be 'n/a'."""
        tmp_dir = tmp_path_factory.mktemp("frag_gray_stats_nosrc")
        tiff_path = frag_gray_fad_result["output paths"]["path tif"]

        stats = frag_gray_stats(tiff_path, stat_files=False)

        assert stats["input stats"]["in foreground pxl"] == "n/a"

    def test_rejects_binary_input(self, tmp_path_factory):
        """frag_gray_stats should reject binary fragmentation outputs."""
        tmp_dir = tmp_path_factory.mktemp("frag_gray_stats_bin")
        tiff_path = tmp_dir / "binary_frag.tif"

        # Create a fake binary frag output with GTB_FOS Binary tag
        data = np.full((5, 5), 50, dtype=np.uint8)
        with rasterio.open(
            tiff_path, 'w', driver='GTiff',
            height=5, width=5, count=1, dtype='uint8',
            crs='EPSG:3035',
            transform=from_origin(0, 5, 1, 1)
        ) as dst:
            dst.write(data, 1)
            dst.update_tags(
                TIFFTAG_IMAGEDESCRIPTION="GTB_FOS, <Binary,-1,4,FAD_5,100.0,3>, https://x"
            )

        with pytest.raises(SystemExit) as e:
            frag_gray_stats(str(tiff_path))
        assert "binary" in str(e.value).lower() or "Binary" in str(e.value)


# =============================================================================
# Validation Tests
# =============================================================================

class TestValidation:

    def test_invalid_method(self, gray_input_tif):
        with pytest.raises(SystemExit):
            frag_gray(str(gray_input_tif), method='XYZ', window_size=3, for_threshold=1)

    def test_invalid_window_size(self, gray_input_tif):
        with pytest.raises(SystemExit):
            frag_gray(str(gray_input_tif), method='FAD', window_size=4, for_threshold=1)

    def test_invalid_connectivity(self, gray_input_tif):
        with pytest.raises(SystemExit):
            frag_gray(str(gray_input_tif), method='FED', window_size=3,
                      for_threshold=1, connectivity=6)
