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

#include <stdio.h>
#include <stdlib.h>
#include "fah.h"
#include "miallib.h"


#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif


/*
 **  Create a new circular FIFO queue of size length.
 */
FIFO *alloc_fifo(long int length)
{
  /*
  ** author: Pierre Soille
  ** length: number of elements of the queue
  */

  FIFO *q = (FIFO *)malloc(sizeof(FIFO));
  if (q == NULL)
    return NULL;
  else{
    if ((q->qp = (long int *)calloc(length, sizeof(long int))) == NULL){
      (void) sprintf(buf, "alloc_fifo(): NOT ENOUGH MEMORY FOR THE QUEUE\n"); errputstr(buf);
      return NULL;
    }
    else{
      q->qps    = q->qp;
      q->qpr    = q->qp;
      q->qplast  = q->qp + length;  /* BUG ? -1 ... ? */
      q->qpl    = NULL;
      q->qcount   = 1;
      q->qlength = length;
      return q;
    }
  }
}


/*
 **  Add data in back of the circular FIFO.
 */
void fifo_add(FIFO *q, long int val)
{
  /*
  ** author: Pierre Soille
  ** q: pointer to a queue
  ** val: value to add in the queue
  */

  *(q->qps++) = val;
  if (q->qps == q->qpr)
    fifo_increase(q);
  if (q->qps == q->qplast){
    if (q->qpr == q->qp)
      fifo_increase(q);
    else
      q->qps = q->qp;   /*  Loop back  */
  }
}


/*
 **  Return data in front of the circular FIFO.
 */
long int fifo_remove(FIFO *q)
{
  /*
  ** author: Pierre Soille
  ** q: pointer to a queue
  */

  if (q->qpr == q->qplast)  q->qpr = q->qp; /*  Loop back    */
  if (q->qpr == q->qps)  return 0;       /*  FIFO is empty  */

  /* Return first element */
  return *(q->qpr++);
}


/*
 **  Look data in front of the circular FIFO.
 */
long int fifo_look(FIFO *q)
{
  if (q->qpl == q->qplast)  q->qpl = q->qp;  /*  Loop back    */
  if (q->qpl == q->qps)    return 0;  /*  End of look   */

  /*  Return first element  */
  return *(q->qpl++);
}


/*
 **  Return TRUE if FIFO is empty.
 */
long int fifo_empty(FIFO *q)
{
  if (q->qpr == q->qplast)  q->qpr = q->qp; /* Loop back      */
  if (q->qpr == q->qps)  return TRUE;       /* FIFO is empty    */
  else  return FALSE;                       /* FIFO is not empty  */

}


/*
 **  Increase the size of the FIFO.
 */
void fifo_increase(FIFO *q)
{
  /*
  ** author: Pierre Soille
  ** q: pointer to a queue
  */

  long int qoffset_s, qoffset_r, qoffset_l, qlength, *qptr1, *qptr2;
  register long int i;

  qoffset_s = (long int)(q->qps - q->qp);
  qoffset_r = (long int)(q->qpr - q->qp);
  qoffset_l = (long int)(q->qplast - q->qp);
  qlength   = (long int)(q->qplast - q->qp + 1 + (q->qplast - q->qp + 1) / q->qcount);
  if ((q->qp = (long int *)realloc(q->qp, qlength * sizeof(long int))) == NULL){
    (void) sprintf(buf,"fifo_increase(): NOT ENOUGH MEMORY !!!\n"); errputstr(buf);
    exit(0);
  }
  q->qlength = qlength;
  q->qplast = q->qp + qlength;
  q->qcount++;
  q->qps = q->qp + qoffset_s;
  qptr1  = q->qplast;
  qptr2  = q->qp + qoffset_l;
  if (qoffset_r != 0){
    for (i = qoffset_r; i < qoffset_l; ++i)
      *(--qptr1) = *(--qptr2);
  }
  q->qpr = qptr1;
}


/*
 **  Free the memory related to the FIFO queue q.
 */
void clear_fifo(FIFO *q)
{
  /*
  ** author: Pierre Soille
  ** q: pointer to a queue
  */

  if (q->qp != NULL)
    free((char *)q->qp);
  if (q != NULL)
    free((char *)q);
}


/*
 **  Free the memory related to the FIFO queue q.
 */
void fifo_reset(FIFO *q)
{
  /*
   ** author: Pierre Soille
   ** q: pointer to a queue
   */
  q->qps    = q->qp;
  q->qpr    = q->qp;
  q->qplast  = q->qp + q->qlength;
  q->qpl    = NULL;
}

