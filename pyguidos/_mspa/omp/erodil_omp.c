/* Windows (MSVC /openmp) build wrapper: compiles the unmodified miallib
 * erodil.c (erosions / dilations) with its OpenMP loops enabled.
 * Used by setup.py instead of miallib/erodil.c on Windows only; see setup.py. */
#define OPENMP 1
#include "erodil.c"
