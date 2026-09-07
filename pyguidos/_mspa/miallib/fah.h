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



typedef struct {
  long int *qp;     /* Pointer to circular queue  */
  long int *qps;    /* Pointer to storage position  */
  long int *qpr;    /* Pointer to retrieve position */
  long int *qplast; /* Pointer to last+1 element  */
  long int *qpl;    /* Pointer to look position  */
  long int qcount;  /* Used to resize the queue  */
  long int qlength;  /* length of the queue  */
} FIFO;


extern FIFO *alloc_fifo(long int);
extern void fifo_add(FIFO *, long int);
extern long int fifo_remove(FIFO *);
extern long int fifo_look(FIFO *);
extern long int fifo_empty(FIFO *);
extern void fifo_increase(FIFO *);
extern void fifo_reset(FIFO *);
extern void clear_fifo(FIFO *);


