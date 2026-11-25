/* Windows (MSVC /openmp) build wrapper: compiles the unmodified miallib
 * ced.c with its OpenMP loops enabled.
 * Used by setup.py instead of miallib/ced.c on Windows only; see setup.py. */
#define OPENMP 1
#include "ced.c"
