"""
Wheel smoke test run by cibuildwheel (not collected by pytest).

Checks that the compiled MSPA extension loads its OpenMP runtime and runs,
and that it coexists in one process with Numba's parallel kernels (two
threading runtimes in the same process).
"""
import numpy as np
from pyguidos._mspa import _mspa
from pyguidos import engine

a = np.full((300, 300), 2, dtype=np.uint8)
a[100:200, 100:200] = 1                  # a hole -> perforation classes
a[:, :5] = 0                             # some missing data
out, _ = _mspa.mspa(a, 1.0, 8, 1, 1)     # MSPA (OpenMP where available)
assert out.shape == a.shape and out.dtype == np.uint8
fad = engine.compute_FAD(a, 27, 1)       # Numba parallel kernel
assert fad.shape == a.shape
out2, _ = _mspa.mspa(a, 1.0, 8, 1, 1)    # MSPA again after Numba started its threads
assert np.array_equal(out, out2)
print("ci_smoke: MSPA + Numba OK")
