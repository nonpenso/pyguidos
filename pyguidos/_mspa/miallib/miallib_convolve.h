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

#ifndef _MIALLIB_CONVOLVE_H
#define _MIALLIB_CONVOLVE_H       1

#include "mialtypes.h"


/* convolve.c */
extern IMAGE *convolve(IMAGE *im, IMAGE *imse, IMAGE *imweight, int ox, int oy, int oz);
extern IMAGE *convolvedownsample(IMAGE *im, IMAGE *imse, IMAGE *imweight, int w, int ox, int oy, int oz);
extern IMAGE *rsum2d(IMAGE *im);
extern IMAGE *rsum3d(IMAGE *im);
extern IMAGE *rsumsq2d(IMAGE *im);
extern IMAGE *mean2d(IMAGE *im, int width);
extern IMAGE *mean2dse(IMAGE *im, IMAGE *imse, int ox, int oy);
extern IMAGE *variance2dse(IMAGE *im, IMAGE *imse, int ox, int oy);
extern IMAGE *squarevol(IMAGE *im, int k, int ox, int oy);
extern ERROR_TYPE azimuth(IMAGE *ix, IMAGE *iy);
extern ERROR_TYPE mapori(IMAGE *i0, int ox, int oy);

/* phase_correlation.c */
extern IMAGE *phase_correlation(IMAGE *im, IMAGE *im_template);

#endif /* miallib_CONVOLVE.h */
