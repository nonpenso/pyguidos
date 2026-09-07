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

#ifndef _MIALLIB_PROJ_H
#define _MIALLIB_PROJ_H       1

#include "mialtypes.h"

/* grid.c  */
extern IMAGE *grid(IMAGE *im, IMAGE *roi, IMAGE *imx, IMAGE *imy, float alpha);

/* projection.c */
extern IMAGE **cs2cs(double ulc_e, double ulc_n, int nx, int ny, double res, char *parmsi[], int ni, char *parmso[], int no);



#endif /* miallib_proj.h */
