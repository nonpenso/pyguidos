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


#define PQDATUM   struct node *
#define PQPRIO(p) (p->prio)


/*
 *  Priority queue structure
 */
struct pqueue
{
  unsigned int size, avail, step;
  PQDATUM *d;
};

struct node {
  int prio;   /* priority level */
  int val;    /* an associated integer value */
  unsigned long int offset; /* offset to image origin */
  int sof;    /* offset to reference seed pixel (used in mssrgcore) */
};

extern struct pqueue * pqinit(struct pqueue *, int);
extern int pqinsert(struct pqueue *, PQDATUM );
extern int pqmininsert(struct pqueue *, PQDATUM );
extern int pqmaxinsert(struct pqueue *, PQDATUM );
extern PQDATUM * pqremove(struct pqueue *, PQDATUM *);
extern PQDATUM * pqminremove(struct pqueue *, PQDATUM *);
extern PQDATUM * pqmaxremove(struct pqueue *, PQDATUM *);
extern PQDATUM * pqpeek(struct pqueue *, PQDATUM *);
extern void free_pq(struct pqueue *);
extern void reset_pq(struct pqueue *);
