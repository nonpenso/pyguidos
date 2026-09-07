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



#ifndef _MIALLIB_POINTOP_H
#define _MIALLIB_POINTOP_H       1

#include "mialtypes.h"

/* pointop.c */
extern ERROR_TYPE bitwise_op(IMAGE *im1, IMAGE *im2, int op);
extern ERROR_TYPE negation(IMAGE *im);
extern ERROR_TYPE arith(IMAGE *im1, IMAGE *im2, int op);
extern ERROR_TYPE arithcst(IMAGE *im, G_TYPE gt, int op);
extern ERROR_TYPE imabs(IMAGE *im);
extern ERROR_TYPE imsqrt(IMAGE *im);
extern ERROR_TYPE imlog(IMAGE *im);
extern ERROR_TYPE imatan(IMAGE *im);
extern ERROR_TYPE imcos(IMAGE *im);
extern ERROR_TYPE imacos(IMAGE *im);
extern ERROR_TYPE imsin(IMAGE *im);
extern ERROR_TYPE imasin(IMAGE *im);
extern ERROR_TYPE thresh(IMAGE *im, G_TYPE gt1, G_TYPE gt2, G_TYPE gbg, G_TYPE gfg);
extern ERROR_TYPE setlevel(IMAGE *im, G_TYPE gt1, G_TYPE gt2, G_TYPE gval);
extern ERROR_TYPE modulo(IMAGE *im, int val);
extern ERROR_TYPE complement(IMAGE *im);
extern ERROR_TYPE power2p(IMAGE *im);
extern ERROR_TYPE blank(IMAGE *im, G_TYPE gval);
extern ERROR_TYPE shift(IMAGE *im, int val);
extern ERROR_TYPE setrange(IMAGE *im, G_TYPE gt1, G_TYPE gt2);
extern IMAGE *ndi(IMAGE *im1, IMAGE *im2);


#endif /* miallib_pointop.h */
