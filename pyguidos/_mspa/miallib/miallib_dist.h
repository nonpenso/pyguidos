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

#ifndef _MIALLIB_DIST_H
#define _MIALLIB_DIST_H       1

#include "mialtypes.h"


/* dist.c */
extern ERROR_TYPE dst2d4(IMAGE *im);
extern ERROR_TYPE dst2dchamfer(IMAGE *im);
extern ERROR_TYPE chamfer2d(IMAGE *im, int type);
extern IMAGE *edistfifo2d(IMAGE *im, int graph);

/* efedt.c */
extern IMAGE *sqedt(IMAGE *im);
extern IMAGE *iz(IMAGE *im);

/* oiiz.c */
extern ERROR_TYPE oiiz(IMAGE *im);

/* geodist.c */
extern ERROR_TYPE geodist(IMAGE *im_m, IMAGE *im_r, int graph);

/* ced.c */
extern IMAGE *ced(IMAGE *ref, IMAGE *mask);


#endif /* miallib_dist.h */
