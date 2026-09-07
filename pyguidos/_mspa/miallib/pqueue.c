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


#include <stdlib.h>
#include <stdio.h>

#include "pqueue.h"


/*
 *  pqinit: initialize the queue.
 *
 *  Parameters:
 *
 *    q           Pointer to a priority queue, or NULL if the user
 *                wishes to leave it to pqinit to allocate the queue.
 *
 *    n           Numer of queue items for which memory should be
 *                preallocated, that is, the initial size of the
 *                item array the queue uses. If you insert more than
 *                n items to the queue, another n items will
 *                be allocated automatically.
 *
 *  Return values:
 *
 *   non-NULL     Priority queue has been initialized.
 *
 *   NULL         Insufficient memory.
 */
struct
pqueue *pqinit(struct pqueue *q, int n)
{
  struct pqueue *tmp = q;

  if (!q && !(q = malloc(sizeof(struct pqueue)))){
    return NULL;
  }
  if (!(q->d = malloc(sizeof(PQDATUM) * n))){
    if (!tmp) free(q);
    return NULL;
  }
  q->avail = q->step = n;
  q->size = 1;
  return q;
}

/*
 *  pqinsert: insert an item into the queue.
 *
 *  Parameters:
 *
 *    q           Pointer to a priority queue.
 *
 *    d           Datum to be inserted.
 *
 *  Return values:
 *
 *    1           The item has been inserted.
 *
 *    0           The item could not be appended. Either the queue i
 *                pointer provided was NULL, or the function was unable
 *                to allocate the amount of memory needed for
 *                the new item.
 */
int
pqinsert(struct pqueue *q, PQDATUM d)
{
  PQDATUM *tmp;
  unsigned int i, newsize;

  if (!q) return 0;

  /* allocate more memory if necessary */
  if (q->size >= q->avail){
    newsize = q->size + q->step;
    if (!(tmp = realloc(q->d, sizeof(PQDATUM) * newsize))){
      return 0;
    };
    q->d = tmp;
    q->avail = newsize;
  }

  /* insert item */
  i = q->size++;
  while (i > 1 && PQPRIO(q->d[i / 2]) < PQPRIO(d)){
    q->d[i] = q->d[i / 2];
    i /= 2;
  }
  q->d[i] = d;
  return 1;
}

int
pqmaxinsert(struct pqueue *q, PQDATUM d)
{
  PQDATUM *tmp;
  unsigned int i, newsize;

  if (!q) return 0;

  /* allocate more memory if necessary */
  if (q->size >= q->avail){
    newsize = q->size + q->step;
    if (!(tmp = realloc(q->d, sizeof(PQDATUM) * newsize))){
      return 0;
    };
    q->d = tmp;
    q->avail = newsize;
  }

  /* insert item */
  i = q->size++;
  while (i > 1 && PQPRIO(q->d[i / 2]) < PQPRIO(d)){
    q->d[i] = q->d[i / 2];
    i /= 2;
  }
  q->d[i] = d;
  return 1;
}

int
pqmininsert(struct pqueue *q, PQDATUM d)
{
  PQDATUM *tmp;
  unsigned int i, newsize;

  if (!q) return 0;

  /* allocate more memory if necessary */
  if (q->size >= q->avail){
    newsize = q->size + q->step;
    if (!(tmp = realloc(q->d, sizeof(PQDATUM) * newsize))){
      return 0;
    };
    q->d = tmp;
    q->avail = newsize;
  }

  /* insert item */
  i = q->size++;
  while (i > 1 && PQPRIO(q->d[i / 2]) > PQPRIO(d)){
    q->d[i] = q->d[i / 2];
    i /= 2;
  }
  q->d[i] = d;
  return 1;
}

/*
 *  pqremove: remove the highest-ranking item from the queue.
 *
 *  Parameters:
 *
 *    p           Pointer to a priority queue.
 *
 *    d           Pointer to the PQDATUM variable that will hold the
 *                datum corresponding to the queue item removed.
 *
 *  Return values:
 *
 *    non-NULL    An item has been removed. The variable that d points
 *                to now contains the datum associated with the item
 *                in question.
 *
 *    NULL        No item could be removed. Either the queue pointer
 *                provided was NULL, or the queue was empty. The chunk
 *                of memory that d points to has not been modified.
 */
PQDATUM *
pqremove(struct pqueue *q, PQDATUM *d)
{
  PQDATUM tmp;
  unsigned int i = 1, j;

  if (!q || q->size == 1) return NULL;
  *d = q->d[1];
  tmp = q->d[--q->size];
  while (i <= q->size / 2){
    j = 2 * i;
    if (j < q->size &&
	PQPRIO(q->d[j]) < PQPRIO(q->d[j + 1])){
      j++;
    }
    if (PQPRIO(q->d[j]) <= PQPRIO(tmp)){
      break;
    }
    q->d[i] = q->d[j];
    i = j;
  }
  q->d[i] = tmp;
  return d;
}
PQDATUM *
pqmaxremove(struct pqueue *q, PQDATUM *d)
{
  PQDATUM tmp;
  unsigned int i = 1, j;

  if (!q || q->size == 1) return NULL;
  *d = q->d[1];
  tmp = q->d[--q->size];
  while (i <= q->size / 2){
    j = 2 * i;
    if (j < q->size &&
	PQPRIO(q->d[j]) < PQPRIO(q->d[j + 1])){
      j++;
    }
    if (PQPRIO(q->d[j]) <= PQPRIO(tmp)){
      break;
    }
    q->d[i] = q->d[j];
    i = j;
  }
  q->d[i] = tmp;
  return d;
}
PQDATUM *
pqminremove(struct pqueue *q, PQDATUM *d)
{
  PQDATUM tmp;
  unsigned int i = 1, j;

  if (!q || q->size == 1) return NULL;
  *d = q->d[1];
  tmp = q->d[--q->size];
  while (i <= q->size / 2){
    j = 2 * i;
    if (j < q->size &&
	PQPRIO(q->d[j]) > PQPRIO(q->d[j + 1])){
      j++;
    }
    if (PQPRIO(q->d[j]) >= PQPRIO(tmp)){
      break;
    }
    q->d[i] = q->d[j];
    i = j;
  }
  q->d[i] = tmp;
  return d;
}

/*
 *  pqpeek: access highest-ranking item without removing it.
 *
 *  Parameters:
 *
 *    q           Pointer to a priority queue.
 *
 *    d           Pointer to the PQDATUM variable that will hold the
 *                datum corresponding to the highest-ranking item.
 *
 *  Return values:
 *
 *    non-NULL   Success. The variable that d points to now contains
 *               the datum associated with the highest-ranking item.
 *
 *    NULL       Failure. Either the queue pointer provided was NULL,
 *               or the queue was empty. The chunk of memory that d
 *               points to has not been modified.
 */
PQDATUM *
pqpeek(struct pqueue *q, PQDATUM *d)
{
  if (!q || q->size == 1) return NULL;
  *d = q->d[1];
  return d;
}



/*
 * free memory allocated for a pqueue
 */
void free_pq(struct pqueue *q)
{
  /*  int i, count=0; */
#ifdef DEBUG
  printf("q->avail=%d\n", q->avail);
  printf("q->size=%d\n", q->size);
#endif
/*   for (i=1; i<q->avail; i++){ */
/*     if (q->d[i] != NULL){ */
/*       printf("unexpected non null datum in pq during freeing \n"); */
/*       printf("q-d[%d]->prio=%d \n", i, q->d[i]->prio); */
/*       //free(q->d[i]); */
/*       count++; */
/*     } */
/*   } */
/*   printf("%d datum were set to NULL in free_pq\n", count); */

  free((void*) q->d);
  free((void*) q);
}


/*
 * free memory allocated for a pqueue
 */
void emergencyfree_pq(struct pqueue *q)
{
  unsigned int i, count=0;

  printf("q->avail=%d\n", q->avail);
  printf("q->size=%d\n", q->size);

  for (i=1; i<q->size; i++){
    if (q->d[i] != NULL){
      count++;
      free((char*) (q->d[i]) );
    }
  }

  printf("%d datum were freed in free_pq\n", count);

  free((void *)q->d);
  free((void*)q);
}


/*
 * reset pqueue
 */
void reset_pq(struct pqueue *q)
{
  unsigned int i;

  for (i=1; i<q->size; i++){
    if (q->d[i] != NULL){
      free((char*) (q->d[i]) );
    }
  }
  q->size=1;
}
