"""
Tests for pyguidos.results module.

All tests are pure Python — no GeoTIFF files or binaries required.
Run with: pytest tests/test_results.py -v
"""

import pytest
import numpy as np
from pyguidos.results import (
    MSPAResult,
    FragResult,
    LandMosResult,
    AccResult,
    RssResult,
    _BaseResult
)


# =============================================================================
# _BaseResult
# =============================================================================

class TestBaseResult:

    def test_cannot_instantiate_directly(self):
        """_BaseResult is a base class — instantiation is allowed but
        repr uses the subclass name so we just check it works."""
        result = _BaseResult(stats={'a': 1})
        assert result.stats == {'a': 1}

    def test_array_default_none(self):
        """array defaults to None."""
        result = _BaseResult(stats={})
        assert result.array is None

    def test_array_can_be_set(self):
        """array can be set to a numpy array."""
        arr = np.zeros((10, 10), dtype=np.uint8)
        result = _BaseResult(stats={}, array=arr)
        assert result.array is not None


# =============================================================================
# MSPAResult
# =============================================================================

class TestMSPAResult:

    def test_instantiation_with_stats(self):
        """MSPAResult can be instantiated with stats only."""
        stats = {'output paths': None, 'input stats': {}, 'output stats': {}}
        result = MSPAResult(stats=stats)
        assert result.stats == stats
        assert result.array is None

    def test_instantiation_with_array(self):
        """MSPAResult can be instantiated with stats and array."""
        arr = np.zeros((5, 5), dtype=np.uint8)
        result = MSPAResult(stats={}, array=arr)
        assert result.array is not None
        assert result.array.shape == (5, 5)

    def test_repr_contains_class_name(self):
        """__repr__ must contain MSPAResult."""
        result = MSPAResult(stats={'output paths': None,
                                   'input stats': {},
                                   'output stats': {}})
        assert 'MSPAResult' in repr(result)

    def test_repr_no_array(self):
        """__repr__ shows 'no' when array is None."""
        result = MSPAResult(stats={})
        assert 'no' in repr(result)

    def test_repr_with_array(self):
        """__repr__ shows 'yes' when array is set."""
        arr = np.zeros((5, 5), dtype=np.uint8)
        result = MSPAResult(stats={}, array=arr)
        assert 'yes' in repr(result)

    def test_repr_shows_stats_keys(self):
        """__repr__ shows the stats dictionary keys."""
        result = MSPAResult(stats={'output paths': None,
                                   'input stats': {},
                                   'output stats': {}})
        r = repr(result)
        assert 'output paths' in r or 'input stats' in r or 'output stats' in r


# =============================================================================
# FragResult
# =============================================================================

class TestFragResult:

    def test_instantiation(self):
        """FragResult can be instantiated."""
        result = FragResult(stats={})
        assert result.stats == {}
        assert result.array is None

    def test_repr_contains_class_name(self):
        """__repr__ must contain FragResult."""
        result = FragResult(stats={})
        assert 'FragResult' in repr(result)

    def test_with_array(self):
        """FragResult stores array correctly."""
        arr = np.ones((3, 3), dtype=np.uint8)
        result = FragResult(stats={}, array=arr)
        assert result.array.shape == (3, 3)

    def test_repr_no_array(self):
        """__repr__ shows 'no' when array is None."""
        result = FragResult(stats={})
        assert 'no' in repr(result)

    def test_repr_with_array(self):
        """__repr__ shows 'yes' when array is set."""
        result = FragResult(stats={}, array=np.zeros((2, 2), dtype=np.uint8))
        assert 'yes' in repr(result)


# =============================================================================
# LandMosResult
# =============================================================================

class TestLandMosResult:

    def test_instantiation(self):
        """LandMosResult can be instantiated."""
        result = LandMosResult(stats={})
        assert result.stats == {}
        assert result.array is None

    def test_repr_contains_class_name(self):
        """__repr__ must contain LandMosResult."""
        result = LandMosResult(stats={})
        assert 'LandMosResult' in repr(result)

    def test_with_array(self):
        """LandMosResult stores array correctly."""
        arr = np.zeros((4, 4), dtype=np.uint8)
        result = LandMosResult(stats={}, array=arr)
        assert result.array is not None

    def test_repr_no_array(self):
        """__repr__ shows 'no' when array is None."""
        result = LandMosResult(stats={})
        assert 'no' in repr(result)

    def test_repr_with_array(self):
        """__repr__ shows 'yes' when array is set."""
        result = LandMosResult(stats={}, array=np.zeros((2, 2), dtype=np.uint8))
        assert 'yes' in repr(result)


# =============================================================================
# AccResult
# =============================================================================

class TestAccResult:

    def test_instantiation(self):
        """AccResult can be instantiated."""
        result = AccResult(stats={})
        assert result.stats == {}
        assert result.array is None

    def test_repr_contains_class_name(self):
        """__repr__ must contain AccResult."""
        result = AccResult(stats={})
        assert 'AccResult' in repr(result)

    def test_with_array(self):
        """AccResult stores array correctly."""
        arr = np.zeros((4, 4), dtype=np.uint8)
        result = AccResult(stats={}, array=arr)
        assert result.array is not None

    def test_repr_no_array(self):
        """__repr__ shows 'no' when array is None."""
        result = AccResult(stats={})
        assert 'no' in repr(result)

    def test_repr_with_array(self):
        """__repr__ shows 'yes' when array is set."""
        result = AccResult(stats={}, array=np.zeros((2, 2), dtype=np.uint8))
        assert 'yes' in repr(result)


# =============================================================================
# RssResult
# =============================================================================

class TestRssResult:

    def test_instantiation(self):
        """RssResult can be instantiated with stats only."""
        result = RssResult(stats={})
        assert result.stats == {}

    def test_no_array_field(self):
        """RssResult has no array field."""
        result = RssResult(stats={})
        assert not hasattr(result, 'array')

    def test_repr_contains_class_name(self):
        """__repr__ must contain RssResult."""
        result = RssResult(stats={})
        assert 'RssResult' in repr(result)

    def test_repr_shows_stats_keys(self):
        """__repr__ shows the stats dictionary keys."""
        result = RssResult(stats={'output paths': None, 'input stats': {}})
        r = repr(result)
        assert 'output paths' in r or 'input stats' in r

    def test_stats_accessible(self):
        """Stats dictionary is accessible."""
        stats = {'output paths': {'path txt': '/tmp/test.txt'},
                 'input stats': {'foreground pxl': 1000},
                 'output stats': {'ECA': 500.0}}
        result = RssResult(stats=stats)
        assert result.stats['output stats']['ECA'] == 500.0


# =============================================================================
# Cross-class checks
# =============================================================================

class TestCrossClass:

    def test_all_result_classes_have_stats(self):
        """All result classes must have a stats field."""
        classes = [MSPAResult, FragResult, LandMosResult, AccResult]
        for cls in classes:
            result = cls(stats={'test': 1})
            assert hasattr(result, 'stats')
            assert result.stats == {'test': 1}

    def test_all_base_results_have_array(self):
        """All _BaseResult subclasses must have an array field."""
        classes = [MSPAResult, FragResult, LandMosResult, AccResult]
        for cls in classes:
            result = cls(stats={})
            assert hasattr(result, 'array')
            assert result.array is None

    def test_all_reprs_are_distinct(self):
        """Each result class must have a distinct __repr__."""
        classes = [MSPAResult, FragResult, LandMosResult, AccResult, RssResult]
        reprs = [repr(cls(stats={})) for cls in classes]
        assert len(set(reprs)) == len(reprs)

    def test_rss_result_no_array(self):
        """RssResult must not have array field unlike other results."""
        rss = RssResult(stats={})
        mspa = MSPAResult(stats={})
        assert not hasattr(rss, 'array')
        assert hasattr(mspa, 'array')
