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

#ifndef _MIALLIB_MISCEL_H
#define _MIALLIB_MISCEL_H 1

#include "mialtypes.h"

/* dirmean.c */
extern IMAGE *dirmean(IMAGE *imx, IMAGE *imy, IMAGE *imse, int ox, int oy, int oz);
extern IMAGE *coherence(IMAGE *imx, IMAGE *imy, IMAGE *imse, int ox, int oy, int oz);

/* gsl.c */
extern IMAGE *coor_extrema_paraboloid(IMAGE *b);
extern IMAGE *fitlinear(IMAGE *xarray, IMAGE  *yarray);

/* transition.c */
extern IMAGE *transgrad(IMAGE *im, int graph);

/* remsens.c  */
extern double julian_date(short int year, short int month, short int day, double hour);

#endif /* miallib_miscel.h */
