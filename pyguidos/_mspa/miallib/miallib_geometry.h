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

#ifndef _MIALLIB_GEOMETRY_H
#define _MIALLIB_GEOMETRY_H       1

#include "mialtypes.h"


/* geom.c */
extern ERROR_TYPE framebox(IMAGE *im, int *box, G_TYPE gval);
extern ERROR_TYPE addframebox(IMAGE *im, int *box, G_TYPE gval);
extern ERROR_TYPE subframebox(IMAGE *im, int *box);
extern ERROR_TYPE imputop(IMAGE *im1, IMAGE *im2, int x, int y, int z, int op);
extern ERROR_TYPE imputcompose(IMAGE *im1, IMAGE *imlbl, IMAGE *im2, int x, int y, int z, int val);
extern IMAGE *imcut(IMAGE *im, int x1, int y1, int z1, int x2, int y2, int z2);
extern IMAGE *getboundingbox(IMAGE *im);
extern IMAGE *magnify(IMAGE *im, int n);
extern IMAGE **rotatecoor(IMAGE *im, double theta);

/* setshft.c */
extern ERROR_TYPE szcompat(IMAGE *im1, IMAGE *im2);
extern ERROR_TYPE szgeocompat(IMAGE *im1, IMAGE *im2);

/* bresenham.c */
extern ERROR_TYPE plotline(IMAGE *im, int x1, int y1, int x2, int y2, int val);


/*  ovlmatrix.c */
extern ERROR_TYPE ovlmatrix(IMAGE *matrix, IMAGE *maxg_array, char *odir);

#endif /* miallib_geometry.h */
