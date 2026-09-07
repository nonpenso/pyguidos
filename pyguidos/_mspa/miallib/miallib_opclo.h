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

#ifndef _MIALLIB_OPCLO_H
#define _MIALLIB_OPCLO_H       1

#include "mialtypes.h"

/* uswilk.c */
extern IMAGE *attribute(IMAGE *imliiar, int type, int oporclo, double lambdaVal, int graph);
extern IMAGE *GreyAreaOpening(IMAGE *imliiar, int lambdaVal, int graph);
extern IMAGE *GreyAreaClosing(IMAGE *imliiar, int lambdaVal, int graph);
extern IMAGE *GreyAreaOpeningROI(IMAGE *imliiar, int lambdaVal, int graph);
extern IMAGE *GreyAreaClosingROI(IMAGE *imliiar, int lambdaVal, int graph);

/* myhull.c  */
// extern IMAGE *chull(IMAGE *ilbl, int graph);

/* hull.c */
extern IMAGE *hpclose(IMAGE *im, int dx, int dy);

/* hullti.c */
extern IMAGE *hpcloseti(IMAGE *im, int dx, int dy);



#endif /* miallib_opclo.h */
