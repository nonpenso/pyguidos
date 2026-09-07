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

#ifndef _MIALLIB_FORMAT_H
#define _MIALLIB_FORMAT_H       1

#include "mialtypes.h"


/* format.c */
extern IMAGE *to_tiff1bitpp(IMAGE *im);
extern IMAGE *to_tiff4bitpp(IMAGE *im);
extern IMAGE *to_ushort(IMAGE *im);
extern IMAGE *to_int32(IMAGE *im);
extern IMAGE *to_float(IMAGE *im);
extern IMAGE *to_double(IMAGE *im);
extern ERROR_TYPE to_uchar(IMAGE *im);
extern ERROR_TYPE dbltofloat(IMAGE *im);
extern ERROR_TYPE uint32_to_float(IMAGE *im);
extern ERROR_TYPE swap(IMAGE *im);


/* miscel.c */
extern IMAGE *deinterleave(IMAGE *im);


/* colconv.c */
extern IMAGE *imhsi2rgb(IMAGE *imh, IMAGE *ims, IMAGE *imi);
extern IMAGE *imhls2rgb(IMAGE *imh, IMAGE *ims, IMAGE *imi);
extern IMAGE **imrgb2hsx(IMAGE *imr, IMAGE *img, IMAGE *imb, int type);
extern IMAGE *crgb2rgb(IMAGE *imh, IMAGE *ims, IMAGE *imi);



#endif /* miallib_format.h */
