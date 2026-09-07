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

				/* fifo data structure */
#include <stdio.h>
#include <stdlib.h>
#include "fifo.h"



#if (defined(XLISP))
extern char buf[];       /* used by sprintf() and stdputstr() */
extern void stdputstr(); /* print a message to the standard output */
extern void errputstr(); /* print a message to the standard error  */
#else
extern char buf[];       /* used by sprintf() and stdputstr() */
extern void stdputstr(); /* print a message to the standard output */
extern void errputstr(); /* print a message to the standard error  */
#endif

extern void nrerror(char *);


/*
GLOUP: the look functions seems a bit dodgy
*/





/*
 ** Create a new circular FIFO queue of size length.
 */
FIFO4 *create_fifo4(long int mod)
{
  FIFO4 *q;

  q = (FIFO4 *)malloc(sizeof(FIFO4));

  if (q != NULL){
    if ((q->qp = (fifo4_word_t *)calloc(mod+2, sizeof(fifo4_word_t))) != NULL){
      q->qps	= q->qp;
      q->qpr	= q->qp;
      q->qplast	= q->qp + mod + 1;
      q->qpl    = q->qp;
      q->mod 	= mod+2;
    }
    else return NULL;
  }

  return q; /* return the pointer to the queue (NULL if not created) */
}


/*
 ** Add data in back of the circular FIFO.
 */
void fifo4_add(FIFO4 *q, fifo4_word_t data)
{
  *(q->qps++) = data;
  if (q->qps == q->qpr)
    fifo4_increase(q);
  if (q->qps > q->qplast){
    if (q->qpr == q->qp)
      fifo4_increase(q);
    else
      q->qps = q->qp; /* Loop back */
  }
}


/*
 ** Return data in front of the circular FIFO.
 */
fifo4_word_t fifo4_remove(FIFO4 *q)
{
  if (q->qpr > q->qplast)  q->qpr = q->qp; /* Loop back  */
  if (q->qpr == q->qps)  return 0;       /* FIFO is empty */

  return *(q->qpr++);  /* Return first element */
}


/*
 ** Look data in front of the circular FIFO. 2004/03/4:
 ** BEWARE that fifo4_reset must be called beforehand
 ** and that no fifo4_remove operations must be performed
 ** during a cycle of fifo4_look calls.
 ** Indeed, qpl is not reset when calling fifo4_remove!
 */
fifo4_word_t fifo4_look(FIFO4 *q)
{
  if (q->qpl > q->qplast) q->qpl = q->qp; /* Loop back  */
  if (q->qpl == q->qps)  return 0; /* End of look  */

  return *(q->qpl++);  /* Return first element */
}

/*
 ** Reset the look position to the first element to retrieve from the queue.
 ** 2001-11-08
 */
void fifo4_lookreset(FIFO4 *q)
{
  q->qpl = q->qpr;
}

/*
 ** Flush the queue
 */
void fifo4_flush(FIFO4 *q)
{
  q->qps = q->qp;
  q->qpr = q->qp;
  q->qpl = q->qp;
}


/*
 ** Return 1 if FIFO is empty.
 */
long int fifo4_empty(FIFO4 *q)
{
  if (q->qpr > q->qplast)  q->qpr = q->qp; /* Loop back */
  if (q->qpr == q->qps)  return 1; /* FIFO is empty */
  else return 0; /* FIFO is not empty */
}


/*
 ** Increase the size of the FIFO.
 */
void fifo4_increase(FIFO4 *q)
{
  long int qoffset_s, qoffset_r, qoffset_last;  /* , qoffset_l */
  long int nelem;
  fifo4_word_t *qptr1, *qptr2;
  long int i;

  qoffset_s    = (long int)(q->qps - q->qp);
  qoffset_r    = (long int)(q->qpr - q->qp);
  /* qoffset_l    = (long int)(q->qpl - q->qp); */

  qoffset_last = (long int)(q->qplast - q->qp);
  nelem        = (long int)(q->qplast - q->qp) + q->mod + 1;

  if ((q->qp = (fifo4_word_t *)realloc(q->qp, nelem*sizeof(fifo4_word_t))) == NULL)
    nrerror("fifo4_increase(): not enough memory");

  q->qplast = q->qp + nelem - 1;
  q->qps = q->qp + qoffset_s;
  qptr1  = q->qplast;
  qptr2  = q->qp + qoffset_last;
  if (qoffset_r != 0){
    for (i = qoffset_r; i <= qoffset_last; i++, qptr1--, qptr2--)
      *qptr1 = *qptr2;
    q->qpr = (qptr1+1);
  }
  else
    q->qpr = q->qp;
  /* on 2001-11-13  if (qoffset_l <= qoffset_s)
    q->qpl = q->qp + qoffset_l;
  else
    q->qpl = q->qp + qoffset_l + q->mod;
  */
  q->qpl = q->qpr;
}


/*
 ** Free the memory related to the FIFO queue q.
 */
void free_fifo4(FIFO4 *q)
{
  if (q->qp != NULL)
    free((char *)q->qp);
  if (q != NULL)
    free((char *)q);
}



