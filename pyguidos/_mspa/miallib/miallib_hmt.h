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

#ifndef _MIALLIB_HMT_H
#define _MIALLIB_HMT_H       1

#include "mialtypes.h"



/* skelodthin.c */
extern ERROR_TYPE skeleton(IMAGE *im);
extern ERROR_TYPE bprune(IMAGE *im, int occa, int graph);

/* epc.c */
extern IMAGE *epc(IMAGE *im, IMAGE *lut);
extern IMAGE *epcgrey(IMAGE *im, IMAGE *lut);

/* switch.c */
extern IMAGE *switchop(IMAGE *im, IMAGE *imse, int ox, int oy, int oz);

/* oiht.c */
extern ERROR_TYPE oiskeleton(IMAGE *im, IMAGE *imanchor);
extern ERROR_TYPE oiask(IMAGE *im, IMAGE *imanchor);


#ifdef ODOITHIN
/* skel.c */
extern ERROR_TYPE binODthin_noqueue(IMAGE *imin, int stype, int atype, IMAGE *imanchor);
extern ERROR_TYPE binODthin_FIFO(IMAGE *imin, int stype, int atype, IMAGE *imanchor);
extern ERROR_TYPE binOIthin_noqueue(IMAGE *imin, int stype, int atype, IMAGE *imanchor);
extern ERROR_TYPE binOIthin_FIFO(IMAGE *imin, int stype, int atype, IMAGE *imanchor);
#endif


#endif /* miallib_hmt.h */
