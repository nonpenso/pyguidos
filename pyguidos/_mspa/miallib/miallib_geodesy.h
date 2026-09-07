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

#ifndef _MIALLIB_GEODESY_H
#define _MIALLIB_GEODESY_H       1

#include "mialtypes.h"


/* recons.c */
extern ERROR_TYPE rdil(IMAGE *mark, IMAGE *mask, int graph, int flag);
extern ERROR_TYPE rero(IMAGE *mark, IMAGE *mask, int graph, int flag);
extern ERROR_TYPE rerodilp(IMAGE *mark, IMAGE *mask, int graph, int flag, int version);

/* complete.c */
extern ERROR_TYPE complete(IMAGE *im_i, IMAGE *im_rmin, int graph);

/* rminmax.c */
extern IMAGE *minima(IMAGE *imin, int graph);

/* ggeo.c */
extern ERROR_TYPE sqtgpla(IMAGE *im_m, IMAGE *im_r, int graph);
extern ERROR_TYPE sqtg(IMAGE *im_m, IMAGE *im_r, int graph);
extern IMAGE *sqtgsym(IMAGE *im_m, IMAGE *im_r, int graph);

#endif /* miallib_geodesy.h */
