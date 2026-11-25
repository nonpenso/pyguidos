/* Fixed configuration header for the pyGuidos vendored MSPA subset.
 *
 * The upstream miallib generates this file from cmake-config.h.in. For the
 * pyGuidos embedded MSPA build we only need the MSPA morphological engine,
 * so we provide a minimal fixed configuration:
 *   - OPENMP is NOT defined here. The miallib sources include <omp.h> and
 *     their OpenMP pragmas only inside `#ifdef OPENMP` guards (all 122
 *     pragmas of the compiled files are guarded). setup.py defines OPENMP
 *     per platform: for all files on Linux (-fopenmp), for efedt.c,
 *     erodil.c and ced.c on Windows (/openmp, via pyguidos/_mspa/omp/
 *     wrappers), not on macOS. Without it the build is serial.
 *     (Do NOT `#define OPENMP 0` - that still satisfies `#ifdef OPENMP`.)
 *   - Version constants kept for any code that references them.
 */
#ifndef _CONFIG_MIALLIB_H
#define _CONFIG_MIALLIB_H 1

/* Matches upstream miallib CMakeLists.txt (vendored sources: v2.0.0) */
#define MIALLIB_VERSION_MAJOR 2
#define MIALLIB_VERSION_MINOR 0
#define MIALLIB_VERSION_PATCH 0
#define MIALLIB_VERSION 2

#endif /* _CONFIG_MIALLIB_H */
