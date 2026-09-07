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



#ifndef _MIALLIB_ERODIL_H
#define _MIALLIB_ERODIL_H       1

#include "mialtypes.h"


/* lerodil.c */
extern ERROR_TYPE linero(IMAGE *im, int dx, int dy, int n, int line_type);
extern ERROR_TYPE lindil(IMAGE *im, int dx, int dy, int n,int line_type);

/* herk.c */
extern ERROR_TYPE herkpldil(IMAGE *im, int dx, int dy, int k, int o, int t);
extern ERROR_TYPE herkplero(IMAGE *im, int dx, int dy, int k, int o, int t);

/* erodil.c */
extern ERROR_TYPE erode4(IMAGE *im, int ox, int oy);
extern ERROR_TYPE dilate4(IMAGE *im, int ox, int oy);
extern IMAGE *erode(IMAGE *im, IMAGE *imse, int ox, int oy, int oz, int trflag);
extern IMAGE *dilate(IMAGE *im, IMAGE *imse, int ox, int oy, int oz, int trflag);
extern IMAGE *volerode(IMAGE *im, IMAGE *imse, IMAGE *imweight, int ox, int oy, int oz);
extern IMAGE *rank(IMAGE *im, IMAGE *imse, int rank, int ox, int oy, int oz, int trflag);

/* rank.c */
extern IMAGE *squarerank(IMAGE *im, int k, int rank, int ox, int oy);

/* linerank.c */
extern ERROR_TYPE linerank(IMAGE *im, int dx, int dy, int k, int rank, int o);

/* herkbl.c */
extern IMAGE *lrankti(IMAGE *im, int dx, int dy, int k, int rank, int o, int t, int tr);

/* msmm.c */
extern IMAGE *msgradlinf(IMAGE **imap, int nc, int graph);
extern IMAGE *msgradlinfngb(IMAGE **imap, int nc, IMAGE *imngb, int ox, int oy, int oz);


#endif /* miallib_erodil.h */
