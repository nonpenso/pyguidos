/***********************************************************************
Author(s): Pieter Kempeneers
           Francesco Lovergine <autobot@lovergine.com>
Copyright (C) 2000-2024 European Union (Joint Research Centre)

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
** Header file for myhull.c
*/

#ifndef _MYHULL_H
#define _MYHULL_H

/* for point with integer coordinates */
typedef struct {
    int a;
    int b;
} intpair_t;

/* Function to find the convex hull using Andrew's monotone chain algorithm 
 * points: array of input points
 * n: number of input points
 * hull: pre-allocated array to store the resulting hull points (should be at least n+1 in size)
 * returns: number of points in the hull
 */
int chainHull_2D(intpair_t *points, int n, intpair_t *hull);

/* Compute convex hull of an image with labels
 * ilbl: input labeled image
 * graph: connectivity (4 or 8)
 * returns: image with hull points marked
 */
IMAGE *chull(IMAGE *ilbl, int graph);

#endif /* _MYHULL_H */
