from dataclasses import dataclass
import numpy as np


@dataclass
class _BaseResult:
    stats: dict = None
    array: np.ndarray = None

    def __repr__(self):
        has_array = self.array is not None
        keys = list(self.stats.keys()) if self.stats else []
        return f"{self.__class__.__name__}(stats={keys}, array={'yes' if has_array else 'no'})"


class MSPAResult(_BaseResult): pass
class FragResult(_BaseResult): pass
class LandMosResult(_BaseResult): pass
class AccResult(_BaseResult): pass

@dataclass
class RssResult:
    stats: dict

    def __repr__(self):
        keys = list(self.stats.keys()) if self.stats else []
        return f"RssResult(stats={keys})"