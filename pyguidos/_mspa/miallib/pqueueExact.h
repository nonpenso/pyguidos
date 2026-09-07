/***********************************************************************
Author(s): Dominik Brunner and Pierre Soille
Copyright (C) 2004-2020 European Union (Joint Research Centre)

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

/***************************************************************************
                          pqueueExact.h  -  description
                             -------------------
    begin                : Thu Apr 22 2004
 ***************************************************************************/

#ifndef init_pqueueExact
#define init_pqueueExact

#define PQDATUM   struct node *
#define PQPRIO(p) (p->prio)


/*
 *  Priority queue structure
 */
struct pqueue
{
	int size, avail, step;
	PQDATUM *d;
};

struct node {
  double prio;   /* priority level */
  long int val;    /* an associated long integer value */
  int offset; /* offset to image origin */
  //int sof;    /* offset to reference seed pixel (used in mssrgcore) */
};

extern struct pqueue * pqExactInit(struct pqueue *, int);
extern int pqExactInsert(struct pqueue *, PQDATUM );
extern int pqExactMinInsert(struct pqueue *, PQDATUM );
extern int pqExactMaxInsert(struct pqueue *, PQDATUM );
extern PQDATUM * pqExactRemove(struct pqueue *, PQDATUM *);
extern PQDATUM * pqExactMinRemove(struct pqueue *, PQDATUM *);
extern PQDATUM * pqExactMaxRemove(struct pqueue *, PQDATUM *);
extern PQDATUM * pqExactPeek(struct pqueue *, PQDATUM *);
extern void freeExact_pq(struct pqueue *);

#endif
