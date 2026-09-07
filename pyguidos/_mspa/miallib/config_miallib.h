/* Fixed configuration header for the pyGuidos vendored MSPA subset.
 *
 * The upstream miallib generates this file from cmake-config.h.in. For the
 * pyGuidos embedded MSPA build we only need the MSPA morphological engine,
 * so we provide a minimal fixed configuration:
 *   - OPENMP is intentionally left UNDEFINED. The miallib sources include
 *     <omp.h> and OpenMP pragmas only inside `#ifdef OPENMP` guards, so
 *     leaving it undefined avoids the OpenMP dependency entirely. A few
 *     unguarded `#pragma omp` lines remain in some files; unknown pragmas
 *     are harmlessly ignored by the compiler when OpenMP is not enabled.
 *     (Do NOT `#define OPENMP 0` — that still satisfies `#ifdef OPENMP`.)
 *   - Version constants kept for any code that references them.
 */
#ifndef _CONFIG_MIALLIB_H
#define _CONFIG_MIALLIB_H 1

#define MIALLIB_VERSION_MAJOR 1
#define MIALLIB_VERSION_MINOR 2
#define MIALLIB_VERSION_PATCH 1
#define MIALLIB_VERSION 1

#endif /* _CONFIG_MIALLIB_H */
