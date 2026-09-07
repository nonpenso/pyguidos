/***********************************************************************
Author(s): Pierre Soille
Copyright (C) 2000-2020 European Union (Joint Research Centre)

This file is part of miallib.

miallib is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

miallib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with miallib.  If not, see <https://www.gnu.org/licenses/>.
***********************************************************************/


/*
 ** FIFO type definition
 */

#include <stdint.h>

/* pyGuidos portability fix: the FIFO4 queue stores pixel POINTERS in several
   miallib functions (e.g. label.c does `fifo4_add(q, (long int)p)`). On LP64
   platforms (Linux, macOS) `long int` is 64-bit and pointer-sized, so this
   worked. On Windows LLP64 `long int` is only 32-bit, which TRUNCATES the
   stored pointer and causes an access violation when it is later retrieved
   and dereferenced. Using a pointer-sized integer (intptr_t) fixes this on
   Windows while remaining identical to the original on LP64 platforms. */
typedef intptr_t fifo4_word_t;

typedef struct {
  fifo4_word_t *qp;     /* Pointer to circular queue    */
  fifo4_word_t *qps;    /* Pointer to storage position  */
  fifo4_word_t *qpr;    /* Pointer to retrieve position */
  fifo4_word_t *qplast; /* Pointer to last+1 element    */
  fifo4_word_t *qpl;    /* Pointer to look position     */
  long int mod;    /* Modulation factor            */
} FIFO4;

#define  FICT_PIX  1

extern FIFO4 *create_fifo4(long int);
extern void  fifo4_add(FIFO4 *, fifo4_word_t);
extern fifo4_word_t  fifo4_remove(FIFO4 *);
extern fifo4_word_t fifo4_look(FIFO4 *);
extern void fifo4_lookreset(FIFO4 *);
extern long  int fifo4_empty(FIFO4 *);
extern void  fifo4_increase(FIFO4 *);
extern void  free_fifo4(FIFO4 *);
extern void fifo4_flush(FIFO4 *);
