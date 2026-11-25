/* Windows (MSVC /openmp) build wrapper: compiles the unmodified miallib
 * efedt.c (Euclidean distance transform) with its OpenMP loops enabled.
 * Used by setup.py instead of miallib/efedt.c on Windows only; see setup.py. */
#define OPENMP 1
#include "efedt.c"
