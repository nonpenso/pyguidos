"""Tests for the MSPA tool (pg.mspa / pg.mspa_stats).

Includes a bit-identical regression test against the GuidosToolbox (GTB)
reference outputs when the reference data is available under
``codes/mspa`` (skipped otherwise so the suite stays portable).
"""
import re
from pathlib import Path

import numpy as np
import pytest
import rasterio
from rasterio.transform import from_origin

from pyguidos.mspa import mspa, mspa_stats


# --------------------------------------------------------------------------- #
# Synthetic-input tests (self-contained, always run)
# --------------------------------------------------------------------------- #
@pytest.fixture(scope="module")
def mspa_result(tmp_path_factory):
    """Run MSPA once on a small synthetic binary raster (1=BG, 2=FG)."""
    tmp_dir = tmp_path_factory.mktemp("data")
    input_tif = tmp_dir / "input_mspa.tif"

    data = np.ones((20, 20), dtype=np.uint8)
    data[5:15, 5:15] = 2  # a solid foreground block -> core + edge

    with rasterio.open(
        input_tif, "w",
        driver="GTiff",
        height=20, width=20, count=1,
        dtype="uint8",
        crs="EPSG:3035",
        transform=from_origin(0, 20, 1, 1),
    ) as dst:
        dst.write(data, 1)

    return mspa(str(input_tif), connectivity=8, edge_width=1,
                transition=True, intext=True, stat_files=True)


def test_mspa_returns_stats_structure(mspa_result):
    """The result dict exposes the three standard sections."""
    assert "input stats" in mspa_result
    assert "output stats" in mspa_result
    assert "output paths" in mspa_result


def test_mspa_input_foreground_count(mspa_result):
    """Foreground pixel count reported for the 10x10 FG block."""
    assert mspa_result["input stats"]["foreground pxl"] == 100


def test_mspa_output_tif_has_palette(mspa_result):
    """Output GeoTIFF is written with the shared MSPA colormap."""
    out_tif = mspa_result["output paths"]["path tif"]
    assert Path(out_tif).exists()
    with rasterio.open(out_tif) as ds:
        cmap = ds.colormap(1)
    # Standard GTB palette anchors (see templates/mspa_colormap.txt)
    assert cmap[0] == (220, 220, 220, 255)   # background grey
    assert cmap[17] == (0, 200, 0, 255)      # core green


def test_mspa_txt_report_rendered(mspa_result):
    """The .txt report is written and has no unfilled placeholders."""
    out_tif = mspa_result["output paths"]["path tif"]
    txt = Path(out_tif).with_suffix(".txt")
    assert txt.exists()
    body = txt.read_text(encoding="utf-8")
    assert re.findall(r"\{[a-zA-Z_]+\}", body) == []
    assert "MORPHOLOGICAL SPATIAL PATTERN ANALYSIS" in body
    assert "Porosity" in body


def test_mspa_stats_standalone(mspa_result):
    """mspa_stats() can re-derive stats from an existing result GeoTIFF."""
    out_tif = mspa_result["output paths"]["path tif"]
    stats = mspa_stats(out_tif, stat_files=True)
    assert stats["input stats"]["foreground pxl"] == 100
    assert "aggregated foregr" in stats["output stats"]
    assert "porosity" in stats["output stats"]


def _write_tagged_tiff(path, tag=None):
    """Write a tiny uint8 GeoTIFF, optionally with a GTB metadata tag."""
    with rasterio.open(
        path, "w", driver="GTiff", height=4, width=4, count=1,
        dtype="uint8", crs="EPSG:3035", transform=from_origin(0, 4, 1, 1),
    ) as dst:
        dst.write(np.ones((4, 4), dtype=np.uint8), 1)
        if tag is not None:
            dst.update_tags(TIFFTAG_IMAGEDESCRIPTION=tag)


def test_mspa_stats_rejects_non_gtb_tiff(tmp_path):
    """A plain (non-GTB) GeoTIFF is rejected as 'not a GuidosToolbox output'."""
    plain = tmp_path / "plain.tif"
    _write_tagged_tiff(plain, tag=None)
    with pytest.raises(SystemExit) as exc:
        mspa_stats(str(plain))
    assert "not a GuidosToolbox output" in str(exc.value)


def test_mspa_stats_rejects_wrong_gtb_tool(tmp_path):
    """A GTB output from another tool (e.g. GTB_SPA) is rejected by name."""
    spa_like = tmp_path / "spa_like.tif"
    _write_tagged_tiff(
        spa_like,
        tag="GTB_SPA, <5,6>, https://forest.jrc.ec.europa.eu/en/activities/lpa",
    )
    with pytest.raises(SystemExit) as exc:
        mspa_stats(str(spa_like))
    msg = str(exc.value)
    assert "GTB_SPA" in msg
    assert "GTB_MSPA" in msg


# --------------------------------------------------------------------------- #
# Bit-identical regression test vs. GTB reference (skipped if data absent)
# --------------------------------------------------------------------------- #
_REF_DIR = Path(__file__).resolve().parents[2] / "codes" / "mspa"

_REF_CASES = [
    # (edge_width, reference output filename)
    (1, "input_8_1_1_1.tif"),
    (5, "input_8_5_1_1.tif"),
]

_ref_available = (_REF_DIR / "input.tif").exists() and all(
    (_REF_DIR / name).exists() for _, name in _REF_CASES
)


@pytest.mark.skipif(not _ref_available,
                    reason="GTB reference data not available under codes/mspa")
@pytest.mark.parametrize("edge_width,ref_name", _REF_CASES)
def test_mspa_bit_identical_to_gtb(tmp_path, edge_width, ref_name):
    """pg.mspa reproduces the GTB reference output exactly (bit-identical)."""
    work_in = tmp_path / "input.tif"
    with rasterio.open(_REF_DIR / "input.tif") as src:
        profile = src.profile
        data = src.read(1)
    with rasterio.open(work_in, "w", **profile) as dst:
        dst.write(data, 1)

    res = mspa(str(work_in), connectivity=8, edge_width=edge_width,
               transition=True, intext=True, stat_files=False)

    out_tif = res["output paths"]  # None when stat_files=False
    # Output tif path is deterministic from the naming scheme:
    produced = tmp_path / f"input_8_{edge_width}_1_1.tif"
    assert produced.exists()

    with rasterio.open(produced) as ds:
        got = ds.read(1)
    with rasterio.open(_REF_DIR / ref_name) as ds:
        ref = ds.read(1)

    assert np.array_equal(got, ref), (
        f"MSPA output differs from GTB reference for edge_width={edge_width}"
    )
