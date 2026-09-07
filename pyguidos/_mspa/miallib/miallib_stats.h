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



#ifndef _MIALLIB_STATS_H
#define _MIALLIB_STATS_H       1

#include "mialtypes.h"


/* imstat.c */
extern IMAGE *histo1d(IMAGE *im);
extern IMAGE *histo2d(IMAGE *im1, IMAGE *im2);
extern IMAGE *histo3d(IMAGE *im1, IMAGE *im2, IMAGE *im3);
extern IMAGE *rsum(IMAGE *im);
extern IMAGE *lookuprgb(IMAGE *imr, IMAGE *img, IMAGE *imb, IMAGE *imlut);
extern IMAGE *class2d(IMAGE *im1, IMAGE *im2, IMAGE *imlut);
extern IMAGE *area(IMAGE *im, int r, int type);
extern IMAGE *dirsum(IMAGE *im, int dir);
extern G_TYPE *min_max(IMAGE *im);

extern ERROR_TYPE getfirstmaxpos(IMAGE *im, unsigned long int *pos);
extern ERROR_TYPE histcompress(IMAGE *im);
extern ERROR_TYPE lookup(IMAGE *im, IMAGE *imlut);
extern ERROR_TYPE lookuptypematch(IMAGE *im, IMAGE *imlut);
extern ERROR_TYPE volume(IMAGE *im);
extern ERROR_TYPE dirmax(IMAGE *im, int dir);
extern ERROR_TYPE imequalp(IMAGE *im1, IMAGE *im2);
extern ERROR_TYPE getmax(IMAGE *im, double *maxval);
extern ERROR_TYPE getminmax(IMAGE *im, double *minval, double *maxval);


/* histo.c */
extern IMAGE **histrgbmatch(IMAGE *cdf_rgb_src, IMAGE *cdf_rg_tgt, IMAGE *cdf_rb_tgt, IMAGE *cdf_gb_tgt);
extern IMAGE **histrgb3dmatch(IMAGE *cdf_rgb_src, IMAGE *cdf_rg_tgt, IMAGE *cdf_rb_tgt, IMAGE *cdf_gb_tgt);

/* mblincomb.c */
extern ERROR_TYPE mblincomb(IMAGE **imap, int nc, IMAGE *matrix);
extern ERROR_TYPE condmean(IMAGE **imap, int nc);

/* indexx.c */
extern IMAGE *sortindex(IMAGE *i0);


#ifdef CLASSIF
/* classification.c */
extern ERROR_TYPE classstatsinfo(IMAGE *immos, IMAGE *imin);
extern ERROR_TYPE clmindist(IMAGE *immos, IMAGE *imin, int bklabel, int mode, double thr);
extern ERROR_TYPE clparpip(IMAGE *immos, IMAGE *imin, int bklabel, int mode, double mult);
extern ERROR_TYPE clmaha(IMAGE *immos, IMAGE *imin, int bklabel, int mode, double thr);
extern ERROR_TYPE clmaxlike(IMAGE *immos, IMAGE *imin, int bklabel, int type, double thr);
#endif


/* registration.c */
extern IMAGE *ssda(IMAGE *imin, IMAGE *imt, int xi, int yi, int w);
extern IMAGE *ncclewis(IMAGE *imin, IMAGE *imt, IMAGE *sim, IMAGE *ssqim, int xi, int yi, int w);
extern IMAGE *ncc(IMAGE *imin, IMAGE *imt, int xi, int yi, int w);

#endif /* miallib_stats.h */
