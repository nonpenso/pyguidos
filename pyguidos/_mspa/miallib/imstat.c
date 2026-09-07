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

#include <stdio.h>
#include <stdlib.h>
#if !defined(__APPLE__)
#include <malloc.h>
#endif
#include <math.h>
#include "miallib.h"

extern float roundf(float x);

/** @defgroup group_stat Image statistics and pixel sorting
 *  Functions computing image statistics, some other target information as well as pixel sorting
 *  @{
 */


#include "uc_def.h"
ERROR_TYPE uc_getfirstmaxpos(IMAGE *im, unsigned long int *pos)
{
  unsigned long int i, npix;
  PIX_TYPE *p1, maxi=PIX_MIN;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

  *pos=0;
  for (i=0; i<npix; i++, p1++){
    if (*p1>maxi){
      maxi = *p1;
      *pos=i;
    }
  }
  return(NO_ERROR);
}
#include "uc_undef.h"

#include "f_def.h"
ERROR_TYPE f_getfirstmaxpos(IMAGE *im, unsigned long int *pos)
{
  unsigned long int i, npix;
  PIX_TYPE *p1, maxi=PIX_MIN;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

  *pos=0;
  for (i=0; i<npix; i++, p1++){
    if (*p1>maxi){
      maxi = *p1;
      *pos=i;
    }
  }
  return(NO_ERROR);
}
#include "f_undef.h"

#include "d_def.h"
ERROR_TYPE d_getfirstmaxpos(IMAGE *im, unsigned long int *pos)
{
  unsigned long int i, npix;
  PIX_TYPE *p1, maxi=PIX_MIN;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

  *pos=0;
  for (i=0; i<npix; i++, p1++){
    if (*p1>maxi){
      maxi = *p1;
      *pos=i;
    }
  }
  return(NO_ERROR);
}
#include "d_undef.h"


ERROR_TYPE getfirstmaxpos(IMAGE *im, unsigned long int *pos)
{
  switch (GetImDataType(im)){

  case t_UCHAR:
    uc_getfirstmaxpos(im, pos);
    break;
  case t_FLOAT:
    f_getfirstmaxpos(im, pos);
    break;
  case t_DOUBLE:
    d_getfirstmaxpos(im, pos);
    break;
  default:
    (void)sprintf(buf,"getfirstmaxpos(im): invalid pixel type\n"); errputstr(buf);
    return(ERROR);
  }
  return NO_ERROR;
}



/*************************************************************************/
/*               Compute min and max values of an image                  */

#include "g_def.h"
ERROR_TYPE generic_min_max(IMAGE *im, G_TYPE *pg)
{
  mia_size_t i, npix;
  PIX_TYPE *p1, mini, maxi;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);
  mini = *p1;
  maxi = *p1;

  for (i=0; i<npix; i++, p1++){
    if (*p1 < mini)
      mini = *p1;
    else if (*p1 > maxi)
      maxi = *p1;
  }

  pg[0].generic_val = mini;
  pg[1].generic_val = maxi;

#if (t_PIX_TYPE==t_UCHAR)
  pg[0].uc_val = mini;
  pg[1].uc_val = maxi;
#endif

  return(NO_ERROR);
}
#include "g_undef.h"

#include "uc_def.h"
ERROR_TYPE uc_min_max(IMAGE *im, G_TYPE *pg)
{
  mia_size_t i, npix;
  PIX_TYPE *p1, mini, maxi;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);
  mini = *p1;
  maxi = *p1;

  for (i=0; i<npix; i++, p1++){
    if (*p1 < mini)
      mini = *p1;
    else if (*p1 > maxi)
      maxi = *p1;
  }

  pg[0].uc_val = mini;
  pg[1].uc_val = maxi;

#if (t_PIX_TYPE==t_GENERIC)
  pg[0].generic_val = mini;
  pg[1].generic_val = maxi;
#endif

  return(NO_ERROR);
}
#include "uc_undef.h"

#include "s_def.h"
ERROR_TYPE s_min_max(IMAGE *im, G_TYPE *pg)
{
  mia_size_t i, npix;
  PIX_TYPE *p1, mini, maxi;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);
  mini = *p1;
  maxi = *p1;

  for (i=0; i<npix; i++, p1++){
    if (*p1 < mini)
      mini = *p1;
    else if (*p1 > maxi)
      maxi = *p1;
  }

  pg[0].s_val = mini;
  pg[1].s_val = maxi;

  return(NO_ERROR);
}
#include "s_undef.h"

#include "u32_def.h"
ERROR_TYPE u32_min_max(IMAGE *im, G_TYPE *pg)
{
  mia_size_t i, npix;
  PIX_TYPE *p1, mini, maxi;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);
  mini = *p1;
  maxi = *p1;

  for (i=0; i<npix; i++, p1++){
    if (*p1 < mini)
      mini = *p1;
    else if (*p1 > maxi)
      maxi = *p1;
  }

  pg[0].u32_val = mini;
  pg[1].u32_val = maxi;

  return(NO_ERROR);
}
#include "u32_undef.h"

#include "i64_def.h"
ERROR_TYPE i64_min_max(IMAGE *im, G_TYPE *pg)
{
  mia_size_t i, npix;
  PIX_TYPE *p1, mini, maxi;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);
  mini = *p1;
  maxi = *p1;

  for (i=0; i<npix; i++, p1++){
    if (*p1 < mini)
      mini = *p1;
    else if (*p1 > maxi)
      maxi = *p1;
  }

  pg[0].i64_val = mini;
  pg[1].i64_val = maxi;

  return(NO_ERROR);
}
#include "i64_undef.h"

#include "u64_def.h"
ERROR_TYPE u64_min_max(IMAGE *im, G_TYPE *pg)
{
  mia_size_t i, npix;
  PIX_TYPE *p1, mini, maxi;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);
  mini = *p1;
  maxi = *p1;

  for (i=0; i<npix; i++, p1++){
    if (*p1 < mini)
      mini = *p1;
    else if (*p1 > maxi)
      maxi = *p1;
  }

  pg[0].u64_val = mini;
  pg[1].u64_val = maxi;

  return(NO_ERROR);
}
#include "u64_undef.h"

#include "f_def.h"
ERROR_TYPE f_min_max(IMAGE *im, G_TYPE *pg)
{
  mia_size_t i, npix;
  PIX_TYPE *p1, mini, maxi;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);
  mini = *p1;
  maxi = *p1;

  for (i=0; i<npix; i++, p1++){
    if (*p1 < mini)
      mini = *p1;
    else if (*p1 > maxi)
      maxi = *p1;
  }

  pg[0].f_val = mini;
  pg[1].f_val = maxi;

  return(NO_ERROR);
}
#include "f_undef.h"

#include "d_def.h"
ERROR_TYPE d_min_max(IMAGE *im, G_TYPE *pg)
{
  mia_size_t i, npix;
  PIX_TYPE *p1, mini, maxi;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);
  mini = *p1;
  maxi = *p1;

  for (i=0; i<npix; i++, p1++){
    if (*p1 < mini)
      mini = *p1;
    else if (*p1 > maxi)
      maxi = *p1;
  }

  pg[0].d_val = mini;
  pg[1].d_val = maxi;

  return(NO_ERROR);
}
#include "d_undef.h"


#include "us_def.h"
ERROR_TYPE us_min_max(IMAGE *im, G_TYPE *pg)
{
  mia_size_t i, npix;
  PIX_TYPE *p1, mini, maxi;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);
  mini = *p1;
  maxi = *p1;

  for (i=0; i<npix; i++, p1++){
    if (*p1 < mini)
      mini = *p1;
    else if (*p1 > maxi)
      maxi = *p1;
  }

  pg[0].us_val = mini;
  pg[1].us_val = maxi;

  return(NO_ERROR);
}
#include "us_undef.h"

#include "i32_def.h"
ERROR_TYPE i32_min_max(IMAGE *im, G_TYPE *pg)
{
  mia_size_t i, npix;
  PIX_TYPE *p1, mini, maxi;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);
  mini = *p1;
  maxi = *p1;

  for (i=0; i<npix; i++, p1++){
    if (*p1 < mini)
      mini = *p1;
    else if (*p1 > maxi)
      maxi = *p1;
  }

  pg[0].i32_val = mini;
  pg[1].i32_val = maxi;

  return(NO_ERROR);
}
#include "i32_undef.h"

G_TYPE *min_max(IMAGE *im)
{
  G_TYPE *pg;
  ERROR_TYPE rval;

  /* allocate memory for range */
  pg = (G_TYPE *)calloc((size_t)2,sizeof(G_TYPE));
  if (pg == NULL){
    (void)sprintf(buf,"min_max(im): not enough memory\n"); errputstr(buf);
    return(pg);
  }

  switch (GetImDataType(im)){

  case t_UCHAR:
    rval = uc_min_max(im, pg);
    break;

  case t_USHORT:
    rval = us_min_max(im, pg);
    break;

  case t_SHORT:
    rval = s_min_max(im, pg);
    break;

  case t_UINT32:
    rval = u32_min_max(im, pg);
    break;

  case t_INT32:
    rval = i32_min_max(im, pg);
    break;

  case t_INT64:
    rval = i64_min_max(im, pg);
    break;

  case t_UINT64:
    rval = u64_min_max(im, pg);
    break;

  case t_FLOAT:
    rval = f_min_max(im, pg);
    break;

  case t_DOUBLE:
    rval = d_min_max(im, pg);
    break;

  default:
    (void)sprintf(buf,"min_max(im): invalid pixel type\n"); errputstr(buf);
    return(NULL);
  }
  if (rval == ERROR)
    free((char *)pg);
  return(pg);
}


/*************************************************************************/
/*                                                                       */


#ifndef NO_generic_IMAGE
#include "g_def.h"
IMAGE *generic_histo1d(IMAGE *im)
{
  IMAGE *imout;
  PIX_TYPE *pcrt, *pend, maxi;
  HST1D_TYPE *phst;
  G_TYPE *pg;
  long int npix;

#if SIGNED
  (void)sprintf(buf,"generic_histo1d(): signed data type not handled\n"); errputstr(buf);
  return(NULL);
#endif
#if FLOATING
  (void)sprintf(buf,"generic_histo1d(): float data type not handled\n"); errputstr(buf);
  return(NULL);
#endif

  npix = GetImNPix(im);

  /* get min & max values */
  pg = min_max(im);
  if (pg == NULL)
    return(NULL);
  maxi = pg[1].generic_val;
  free((char *)pg);

  /* create histogram */
  imout = (IMAGE *)create_image(t_HST1D, (int)maxi+1, (int)1, (int)1);
  if (imout == NULL){
    (void)sprintf(buf,"generic_histo1d(): not enough memory!\n"); errputstr(buf);
    return(imout);
  }

  phst = (HST1D_TYPE *)GetImPtr(imout);
  pend = (PIX_TYPE *)GetImPtr(im) + npix;
  for (pcrt = (PIX_TYPE *)GetImPtr(im); pcrt < pend; pcrt++)
    phst[*pcrt] += 1;

  return(imout);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */


#include "u32_def.h"
IMAGE *u32_histo1d(IMAGE *im)
{
  IMAGE *imout;
  PIX_TYPE *pcrt, *pend, maxi;
  HST1D_TYPE *phst;
  G_TYPE *pg;
  long int npix;

#if SIGNED
  (void)sprintf(buf,"ul_histo1d(): signed data type not handled\n"); errputstr(buf);
  return(NULL);
#endif
#if FLOATING
  (void)sprintf(buf,"ul_histo1d(): float data type not handled\n"); errputstr(buf);
  return(NULL);
#endif

  npix = GetImNPix(im);

  /* get min & max values */
  pg = min_max(im);
  if (pg == NULL)
    return(NULL);
  maxi = pg[1].u32_val;
  free((char *)pg);

  /* create histogram */
  imout = (IMAGE *)create_image(t_HST1D, (int)maxi+1, (int)1, (int)1);
  if (imout == NULL){
    (void)sprintf(buf,"ul_histo1d(): not enough memory!\n"); errputstr(buf);
    return(imout);
  }

  phst = (HST1D_TYPE *)GetImPtr(imout);
  pend = (PIX_TYPE *)GetImPtr(im) + npix;
  for (pcrt = (PIX_TYPE *)GetImPtr(im); pcrt < pend; pcrt++)
    phst[*pcrt] += 1;

  return(imout);
}
#include "u32_undef.h"



#include "us_def.h"
IMAGE *us_histo1d(IMAGE *im)
{
  IMAGE *imout;
  PIX_TYPE *pcrt, *pend, maxi;
  HST1D_TYPE *phst;
  G_TYPE *pg;
  long int npix;

#if SIGNED
  (void)sprintf(buf,"us_histo1d(): signed data type not handled\n"); errputstr(buf);
  return(NULL);
#endif
#if FLOATING
  (void)sprintf(buf,"us_histo1d(): float data type not handled\n"); errputstr(buf);
  return(NULL);
#endif

  npix = GetImNPix(im);

  /* get min & max values */
  pg = min_max(im);
  if (pg == NULL)
    return(NULL);
  maxi = pg[1].us_val;
  free((char *)pg);

  /* create histogram */
  imout = (IMAGE *)create_image(t_HST1D, (int)maxi+1, (int)1, (int)1);
  if (imout == NULL){
    (void)sprintf(buf,"us_histo1d(): not enough memory!\n"); errputstr(buf);
    return(imout);
  }

  phst = (HST1D_TYPE *)GetImPtr(imout);
  pend = (PIX_TYPE *)GetImPtr(im) + npix;
  for (pcrt = (PIX_TYPE *)GetImPtr(im); pcrt < pend; pcrt++)
    phst[*pcrt] += 1;

  return(imout);
}
#include "us_undef.h"

#include "i32_def.h"
IMAGE *i32_histo1d(IMAGE *im)
{
  IMAGE *imout;
  PIX_TYPE *pcrt, *pend, maxi, mini;
  HST1D_TYPE *phst;
  G_TYPE *pg;
  long int npix;

#if FLOATING
  (void)sprintf(buf,"l_histo1d(): float data type not handled\n"); errputstr(buf);
  return(NULL);
#endif

  npix = GetImNPix(im);

  /* get min & max values */
  pg = min_max(im);
  if (pg == NULL)
    return(NULL);
  maxi = pg[1].i32_val;
  mini = pg[0].i32_val;
  free((char *)pg);

  if (mini < 0){
    (void)sprintf(buf,"l_histo1d(): negative values(mini=%d) not handled\n", (int)mini); errputstr(buf);
    return(NULL);
  }
  /* create histogram */
  imout = (IMAGE *)create_image(t_HST1D, (int)maxi+1, (int)1, (int)1);
  if (imout == NULL){
    (void)sprintf(buf,"l_histo1d(): not enough memory!\n"); errputstr(buf);
    return(imout);
  }

  phst = (HST1D_TYPE *)GetImPtr(imout);
  pend = (PIX_TYPE *)GetImPtr(im) + npix;
  for (pcrt = (PIX_TYPE *)GetImPtr(im); pcrt < pend; pcrt++)
    phst[*pcrt] += 1;

  return(imout);
}
#include "i32_undef.h"

IMAGE *histo1d(IMAGE *im)
{
  switch (GetImDataType(im)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_histo1d(im));
    break;
#endif

#ifndef NO_uc_IMAGE
  case t_UCHAR:
    return(uc_histo1d(im));
    break;
#endif

  case t_USHORT:
    return(us_histo1d(im));
    break;

  case t_INT32:
    return(i32_histo1d(im));
    break;

  case t_UINT32:
    return(u32_histo1d(im));
    break;

  default:
    (void)sprintf(buf,"histo1d(im): invalid pixel type\n"); errputstr(buf);
  }
  return(NULL);
}


/*************************************************************************/
/*                                                                       */


#ifndef NO_generic_IMAGE
#define HST2D_TYPE_LOCAL UINT32
#define t_HST2D_LOCAL t_UINT32
#include "g_def.h"
IMAGE *generic_histo2d(IMAGE *im1, IMAGE *im2)
{
  IMAGE *imout;
  PIX_TYPE *pcrt, *pend, *p2;
  PIX_TYPE maxi1, maxi2;
  HST2D_TYPE_LOCAL *phst;
  G_TYPE *pg;
  long int npix, nx;

#if SIGNED
  (void)sprintf(buf,"generic_histo2d(): signed data type not handled\n"); errputstr(buf);
  return(NULL);
#endif
#if FLOATING
  (void)sprintf(buf,"generic_histo2d(): float data type not handled\n"); errputstr(buf);
  return(NULL);
#endif

  npix = GetImNPix(im1);

  /* get min & max values */
  pg = min_max(im1);
  if (pg == NULL)
    return(NULL);
  maxi1 = pg[1].generic_val;

  pg = min_max(im2);
  if (pg == NULL)
    return(NULL);
  maxi2 = pg[1].generic_val;
  free((char *)pg);

  /* create histogram */
  nx = maxi1+1;
  imout = (IMAGE *)create_image(t_HST2D_LOCAL, (int)nx, (int)maxi2+1, (int)1);
  if (imout == NULL){
    (void)sprintf(buf,"generic_histo2d(): not enough memory!\n"); errputstr(buf);
    return(imout);
  }

  phst = (HST2D_TYPE_LOCAL *)GetImPtr(imout);
  pend = (PIX_TYPE *)GetImPtr(im1) + npix;
  p2   = (PIX_TYPE *)GetImPtr(im2);
  for (pcrt = (PIX_TYPE *)GetImPtr(im1); pcrt < pend; pcrt++, p2++)
    phst[*pcrt + *p2 * nx] += 1;
  return(imout);
}
#include "g_undef.h"
#undef HST2D_TYPE_LOCAL
#undef t_HST2D_LOCAL
#endif /* #ifndef NO_generic_IMAGE */

#include "us_def.h"
#define HST2D_TYPE_LOCAL UINT32
#define t_HST2D_LOCAL t_UINT32
IMAGE *us_histo2d(IMAGE *im1, IMAGE *im2)
{
  IMAGE *imout;
  PIX_TYPE *pcrt, *pend, *p2;
  PIX_TYPE maxi1, maxi2;
  HST2D_TYPE_LOCAL *phst;
  G_TYPE *pg;
  long int npix, nx;

#if SIGNED
  (void)sprintf(buf,"us_histo2d(): signed data type not handled\n"); errputstr(buf);
  return(NULL);
#endif
#if FLOATING
  (void)sprintf(buf,"us_histo2d(): float data type not handled\n"); errputstr(buf);
  return(NULL);
#endif

  npix = GetImNPix(im1);

  /* get min & max values */
  pg = min_max(im1);
  if (pg == NULL)
    return(NULL);
  maxi1 = pg[1].us_val;

  pg = min_max(im2);
  if (pg == NULL)
    return(NULL);
  maxi2 = pg[1].us_val;
  free((char *)pg);

  /* create histogram */
  nx = maxi1+1;
  imout = (IMAGE *)create_image(t_HST2D_LOCAL, (int)nx, (int)maxi2+1, (int)1);
  if (imout == NULL){
    (void)sprintf(buf,"us_histo2d(): not enough memory!\n"); errputstr(buf);
    return(imout);
  }

  phst = (HST2D_TYPE_LOCAL *)GetImPtr(imout);
  pend = (PIX_TYPE *)GetImPtr(im1) + npix;
  p2   = (PIX_TYPE *)GetImPtr(im2);
  for (pcrt = (PIX_TYPE *)GetImPtr(im1); pcrt < pend; pcrt++, p2++)
    phst[*pcrt + *p2 * nx] += 1;
  return(imout);
}
#undef HST2D_TYPE_LOCAL
#undef t_HST2D_LOCAL
#include "us_undef.h"



IMAGE *histo2d(IMAGE *im1, IMAGE *im2)
{
  /* check for possible errors */
  if (szcompat(im1, im2) != NO_ERROR){
    (void)sprintf(buf,"ERROR in *histo2d(im1, im2): \
                images of different size or type\n"); errputstr(buf);
    return(NULL);
  }

  switch (GetImDataType(im1)){
#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_histo2d(im1, im2));
    break;
#endif

#ifndef NO_uc_IMAGE
  case t_UCHAR:
    return(uc_histo2d(im1, im2));
    break;
#endif

  case t_USHORT:
    return(us_histo2d(im1, im2));
    break;

#ifndef NO_u32_IMAGE
  case t_UINT32:
    return(u32_histo2d(im1, im2));
    break;
#endif

  default:
    (void)sprintf(buf,"histo2d(im1, im2): invalid pixel type\n"); errputstr(buf);
  }
  return(NULL);
}

/*************************************************************************/
/*                                                                       */


#ifndef NO_generic_IMAGE
#include "g_def.h"
IMAGE *generic_histo3d(IMAGE *im1, IMAGE *im2, IMAGE *im3)
{
  IMAGE *imout;
  PIX_TYPE *pcrt, *pend, *p2, *p3;
  PIX_TYPE maxi1, maxi2, maxi3;
  HST3D_TYPE *phst;
  G_TYPE *pg;
  long int npix, nx, nxy;

#if SIGNED
  (void)sprintf(buf,"generic_histo3d(): signed data type not handled\n"); errputstr(buf);
  return(NULL);
#endif
#if FLOATING
  (void)sprintf(buf,"generic_histo3d(): float data type not handled\n"); errputstr(buf);
  return(NULL);
#endif

  npix = GetImNPix(im1);

  /* get min & max values */
  pg = min_max(im1);
  if (pg == NULL)
    return(NULL);
  maxi1 = pg[1].generic_val;

  pg = min_max(im2);
  if (pg == NULL)
    return(NULL);
  maxi2 = pg[1].generic_val;
  free((char *)pg);

  pg = min_max(im3);
  if (pg == NULL)
    return(NULL);
  maxi3 = pg[1].generic_val;
  free((char *)pg);

  /* create histogram */
  nx  = maxi1+1;
  nxy = (maxi1+1) * (maxi2+1);
  imout = (IMAGE *)create_image(t_HST3D, (int)nx, (int)maxi2+1,  (int)maxi3+1);
  if (imout == NULL){
    (void)sprintf(buf,"generic_histo3d(): not enough memory!\n"); errputstr(buf);
    return(imout);
  }

  phst = (HST3D_TYPE *)GetImPtr(imout);
  pend = (PIX_TYPE *)GetImPtr(im1) + npix;
  p2   = (PIX_TYPE *)GetImPtr(im2);
  p3   = (PIX_TYPE *)GetImPtr(im3);
  for (pcrt = (PIX_TYPE *)GetImPtr(im1); pcrt < pend; pcrt++, p2++, p3++)
    phst[*pcrt + *p2 * nx + *p3 * nxy] += 1;
  return(imout);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */

#include "us_def.h"
IMAGE *us_histo3d(IMAGE *im1, IMAGE *im2, IMAGE *im3)
{
  IMAGE *imout;
  PIX_TYPE *pcrt, *pend, *p2, *p3;
  PIX_TYPE maxi1, maxi2, maxi3;
  HST3D_TYPE *phst;
  G_TYPE *pg;
  long int npix, nx, nxy;

#if SIGNED
  (void)sprintf(buf,"us_histo3d(): signed data type not handled\n"); errputstr(buf);
  return(NULL);
#endif
#if FLOATING
  (void)sprintf(buf,"us_histo3d(): float data type not handled\n"); errputstr(buf);
  return(NULL);
#endif

  npix = GetImNPix(im1);

  /* get min & max values */
  pg = min_max(im1);
  if (pg == NULL)
    return(NULL);
  maxi1 = pg[1].us_val;

  pg = min_max(im2);
  if (pg == NULL)
    return(NULL);
  maxi2 = pg[1].us_val;
  free((char *)pg);

  pg = min_max(im3);
  if (pg == NULL)
    return(NULL);
  maxi3 = pg[1].us_val;
  free((char *)pg);

  /* create histogram */
  nx  = maxi1+1;
  nxy = (maxi1+1) * (maxi2+1);
  imout = (IMAGE *)create_image(t_HST3D, (int)nx, (int)maxi2+1,  (int)maxi3+1);
  if (imout == NULL){
    (void)sprintf(buf,"us_histo3d(): not enough memory!\n"); errputstr(buf);
    return(imout);
  }

  phst = (HST3D_TYPE *)GetImPtr(imout);
  pend = (PIX_TYPE *)GetImPtr(im1) + npix;
  p2   = (PIX_TYPE *)GetImPtr(im2);
  p3   = (PIX_TYPE *)GetImPtr(im3);
  for (pcrt = (PIX_TYPE *)GetImPtr(im1); pcrt < pend; pcrt++, p2++, p3++)
    phst[*pcrt + *p2 * nx + *p3 * nxy] += 1;
  return(imout);
}
#include "us_undef.h"

IMAGE *histo3d(IMAGE *im1, IMAGE *im2, IMAGE *im3)
{
  /* check for possible errors */
  if ((szcompat(im1, im2) != NO_ERROR) || (szcompat(im1, im3) != NO_ERROR)){
    (void)sprintf(buf,"ERROR in *histo3d(im1, im2, im3): \
                images of different size or type\n"); errputstr(buf);
    return(NULL);
  }

  switch (GetImDataType(im1)){
#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_histo3d(im1, im2, im3));
    break;
#endif

  case t_USHORT:
    return(us_histo3d(im1, im2, im3));
    break;

  default:
    (void)sprintf(buf,"histo3d(im1, im2, im3): invalid pixel type\n"); errputstr(buf);
  }
  return(NULL);
}

/*************************************************************************/
/*                                                                       */


#ifndef NO_generic_IMAGE
#include "g_def.h"
IMAGE *generic_rsum(IMAGE *im)
{
  IMAGE *imout;
  PIX_TYPE *pim;
  HST1D_TYPE *phst;
  long int npix, i;

  npix = GetImNPix(im);

  /* create array for summing up */
  imout = (IMAGE *)create_image(t_HST1D, (int)npix+1, (int)1, (int)1);
  if (imout == NULL){
    (void)sprintf(buf,"generic_rsum(im): not enough memory!\n"); errputstr(buf);
    return(imout);
  }

  phst = (HST1D_TYPE *)GetImPtr(imout);
  pim  = (PIX_TYPE *)GetImPtr(im);
  phst[0] = pim[0];
  for (i=1; i<npix; i++)
    phst[i] = phst[i-1] + pim[i];
  for (i=npix; i>0; i--) /* phst[i] = #{ p | f(p) < i } */
    phst[i] = phst[i-1];
  phst[0] = 0;

  return(imout);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */



#include "u32_def.h"
IMAGE *u32_rsum(IMAGE *im)
{
  IMAGE *imout;
  PIX_TYPE *pim;
  HST1D_TYPE *phst;
  long int npix, i;

  npix = GetImNPix(im);

  /* create array for summing up */
  imout = (IMAGE *)create_image(t_HST1D, (int)npix+1, (int)1, (int)1);
  if (imout == NULL){
    (void)sprintf(buf,"ul_rsum(im): not enough memory!\n"); errputstr(buf);
    return(imout);
  }

  phst = (HST1D_TYPE *)GetImPtr(imout);
  pim  = (PIX_TYPE *)GetImPtr(im);
  phst[0] = pim[0];
  for (i=1; i<npix; i++)
    phst[i] = phst[i-1] + pim[i];
  for (i=npix; i>0; i--) /* phst[i] = #{ p | f(p) < i } */
    phst[i] = phst[i-1];
  phst[0] = 0;

  return(imout);
}
#include "u32_undef.h"



#include "i32_def.h"
IMAGE *i32_rsum(IMAGE *im)
{
  IMAGE *imout;
  PIX_TYPE *pim;
  HST1D_TYPE *phst;
  long int npix, i;

  npix = GetImNPix(im);

  /* create array for summing up */
  imout = (IMAGE *)create_image(t_HST1D, (int)npix+1, (int)1, (int)1);
  if (imout == NULL){
    (void)sprintf(buf,"l_rsum(im): not enough memory!\n"); errputstr(buf);
    return(imout);
  }

  phst = (HST1D_TYPE *)GetImPtr(imout);
  pim  = (PIX_TYPE *)GetImPtr(im);
  phst[0] = pim[0];
  for (i=1; i<npix; i++)
    phst[i] = phst[i-1] + pim[i];
  for (i=npix; i>0; i--)
    phst[i] = phst[i-1];
  phst[0] = 0;

  return(imout);
}
#include "i32_undef.h"



IMAGE *rsum(IMAGE *im)
{
  switch (GetImDataType(im)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_rsum(im));
    break;
#endif

#ifndef NO_uc_IMAGE
  case t_UCHAR:
    return(uc_rsum(im));
    break;
#endif

#ifndef NO_us_IMAGE
  case t_USHORT:
    return(us_rsum(im));
    break;
#endif

#ifndef NO_s_IMAGE
  case t_SHORT:
    return(s_rsum(im));
    break;
#endif

  case t_UINT32:
    return(u32_rsum(im));
    break;

  case t_INT32:
    return(i32_rsum(im));
    break;

  default:
    (void)sprintf(buf,"rsum(): invalid pixel type\n"); errputstr(buf);
  }
  return(NULL);
}

#ifndef NO_generic_IMAGE
#include "g_def.h"
ERROR_TYPE generic_histcompress(IMAGE *im)
{
  IMAGE *imhst;
  HST1D_TYPE *phst;
  PIX_TYPE *pim, val=0;
  unsigned long i, nval, npix;

  imhst = histo1d(im);
  if (imhst==NULL)
    return(ERROR);
  phst = (HST1D_TYPE *)GetImPtr(imhst);
  nval = GetImNPix(imhst);
  pim  = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

  for(i=0;i<nval;i++)
    if (phst[i])
      phst[i]=val++;

  for(i=0;i<npix;i++,pim++)
    *pim=phst[*pim];

  free_image(imhst);
  return(NO_ERROR);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */


#include "us_def.h"
ERROR_TYPE us_histcompress(IMAGE *im)
{
  IMAGE *imhst;
  HST1D_TYPE *phst;
  PIX_TYPE *pim, val=0;
  unsigned long i, nval, npix;

  imhst = histo1d(im);
  if (imhst==NULL)
    return(ERROR);
  phst = (HST1D_TYPE *)GetImPtr(imhst);
  nval = GetImNPix(imhst);
  pim  = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

  for(i=0;i<nval;i++)
    if (phst[i])
      phst[i]=val++;

  for(i=0;i<npix;i++,pim++)
    *pim=phst[*pim];

  free_image(imhst);
  return(NO_ERROR);
}
#include "us_undef.h"

#include "u32_def.h"
ERROR_TYPE u32_histcompress(IMAGE *im)
{
  IMAGE *imhst;
  HST1D_TYPE *phst;
  PIX_TYPE *pim, val=0;
  unsigned long i, nval, npix;

  imhst = histo1d(im);
  if (imhst==NULL)
    return(ERROR);
  phst = (HST1D_TYPE *)GetImPtr(imhst);
  nval = GetImNPix(imhst);
  pim  = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

  for(i=0;i<nval;i++)
    if (phst[i])
      phst[i]=val++;

  for(i=0;i<npix;i++,pim++)
    *pim=phst[*pim];

  free_image(imhst);
  return(NO_ERROR);
}
#include "u32_undef.h"


ERROR_TYPE histcompress(IMAGE *im)
{
  switch (GetImDataType(im)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_histcompress(im));
    break;
#endif

#ifndef NO_uc_IMAGE
  case t_UCHAR:
    return(uc_histcompress(im));
    break;
#endif

  case t_USHORT:
    return(us_histcompress(im));
    break;
  case t_UINT32:
    return(u32_histcompress(im));
    break;
  default:
    (void)sprintf(buf,"histcompress(): invalid pixel type (must be unsigned)\n"); errputstr(buf);
  }
  return(ERROR);
}

#include "uc_def.h"
ERROR_TYPE uc_lookup(IMAGE *im, IMAGE *imlut)
{
  /* generic code for unsigned integer types only */
  long int i;
  long int npix=GetImNPix(im);
  int maxlutidx=GetImNPix(imlut)-1;
  PIX_TYPE *p=(PIX_TYPE *)GetImPtr(im);
  LUT_TYPE *lut=(LUT_TYPE *)GetImPtr(imlut);

 #pragma omp parallel for
  for(i=0;i<npix;i++){
    if(p[i]>maxlutidx)
      (void)sprintf(buf,"warning: lookup(): LUT not matching image values\n");
    else if (lut[p[i]]>PIX_MAX){
      (void)sprintf(buf,"warning: lookup(): LUT value greater than PIX_MAX value (output set to PIX_MAX)\n");
      p[i]=PIX_MAX;
    }
    else if (lut[p[i]]<PIX_MIN){
      (void)sprintf(buf,"warning: lookup(): LUT value lower than PIX_MIN value (output set to PIX_MIN)\n");
      p[i]=PIX_MIN;
    }
    else{
#if FLOATING
      p[i]=(PIX_TYPE)roundf(lut[p[i]]);
#else
      p[i]=(PIX_TYPE)lut[p[i]];
#endif
    }
  }
  return NO_ERROR;
}
#include "uc_undef.h"

#include "us_def.h"
ERROR_TYPE us_lookup(IMAGE *im, IMAGE *imlut)
{
  long int i;
  long int npix=GetImNPix(im);
  int maxlutidx=GetImNPix(imlut)-1;
  PIX_TYPE *p=(PIX_TYPE *)GetImPtr(im);
  LUT_TYPE *lut=(LUT_TYPE *)GetImPtr(imlut);

#pragma omp parallel for
  for(i=0;i<npix;i++){
    if(p[i]>maxlutidx)
      (void)sprintf(buf,"warning: lookup(): LUT not matching image values\n");
    else if (lut[p[i]]>PIX_MAX){
      (void)sprintf(buf,"warning: lookup(): LUT value greater than PIX_MAX value (output set to PIX_MAX)\n");
      p[i]=PIX_MAX;
    }
    else if (lut[p[i]]<PIX_MIN){
      (void)sprintf(buf,"warning: lookup(): LUT value lower than PIX_MIN value (output set to PIX_MIN)\n");
      p[i]=PIX_MIN;
    }
    else{
#if FLOATING
      p[i]=(PIX_TYPE)roundf(lut[p[i]]);
#else
      p[i]=(PIX_TYPE)lut[p[i]];
#endif
    }
  }
  return NO_ERROR;
}
#include "us_undef.h"

#include "u32_def.h"
ERROR_TYPE u32_lookup(IMAGE *im, IMAGE *imlut)
{
  long int i;
  long int npix=GetImNPix(im);
  int maxlutidx=GetImNPix(imlut)-1;
  PIX_TYPE *p=(PIX_TYPE *)GetImPtr(im);
  LUT_TYPE *lut=(LUT_TYPE *)GetImPtr(imlut);

#pragma omp parallel for
  for(i=0;i<npix;i++){
    if(p[i]>maxlutidx)
      (void)sprintf(buf,"warning: lookup(): LUT not matching image values\n");
    else if (lut[p[i]]>PIX_MAX){
      (void)sprintf(buf,"warning: lookup(): LUT value greater than PIX_MAX value (output set to PIX_MAX)\n");
      p[i]=PIX_MAX;
    }
    else if (lut[p[i]]<PIX_MIN){
      (void)sprintf(buf,"warning: lookup(): LUT value lower than PIX_MIN value (output set to PIX_MIN)\n");
      p[i]=PIX_MIN;
    }
    else{
#if FLOATING
      p[i]=(PIX_TYPE)roundf(lut[p[i]]);
#else
      p[i]=(PIX_TYPE)lut[p[i]];
#endif
    }
  }
  return NO_ERROR;
}
#include "u32_undef.h"

#include "f_def.h"
ERROR_TYPE f_lookup(IMAGE *im, IMAGE *imlut)
{
  long int i;
  long int npix=GetImNPix(im);
  int maxlutidx=GetImNPix(imlut)-1;
  PIX_TYPE *p=(PIX_TYPE *)GetImPtr(im);
  LUT_TYPE *lut=(LUT_TYPE *)GetImPtr(imlut), val;

#pragma omp parallel for
  for(i=0;i<npix;i++){
    if((INT32)p[i]>maxlutidx)
      (void)sprintf(buf,"warning: lookup(): LUT not matching image values\n");
    /* the following tests are not necessary since image type is equal to lut type */
/*     else if (val>PIX_MAX){ */
/*       (void)sprintf(buf,"warning: lookup(): LUT value greater than PIX_MAX value (output set to PIX_MAX)\n"); */
/*       p[i]=PIX_MAX; */
/*     } */
/*     else if (val<PIX_MIN){ */
/*       (void)sprintf(buf,"warning: lookup(): LUT value lower than PIX_MIN value (output set to PIX_MIN)\n"); */
/*       p[i]=PIX_MIN; */
/*     } */
    else{
      val=lut[(INT32)p[i]];
      p[i]=(PIX_TYPE)val;
    }
  }
  return NO_ERROR;
}
#include "f_undef.h"


ERROR_TYPE lookup(IMAGE *im, IMAGE *imlut)
{
  if (GetImDataType(imlut)!=t_FLOAT){
    (void)sprintf(buf,"lookup(): imlut must be of type FLOAT\n");
    errputstr(buf);
    return ERROR;
  }
  switch (GetImDataType(im)){
  case t_UCHAR:
    return(uc_lookup(im, imlut));
    break;
  case t_USHORT:
    return(us_lookup(im, imlut));
    break;
  case t_UINT32:
    return(u32_lookup(im, imlut));
    break;
  case t_FLOAT:
    return(f_lookup(im, imlut));
    break;
  default:
    (void)sprintf(buf,"lookup(): invalid pixel type\n");
    errputstr(buf);
  }
  return(ERROR);
}


#include "uc_def.h"
ERROR_TYPE uc_lookuptypematch(IMAGE *im, IMAGE *imlut)
{
  /* generic code for unsigned integer types only */
  long int i;
  long int npix=GetImNPix(im);
  int maxlutidx=GetImNPix(imlut)-1;
  PIX_TYPE *p=(PIX_TYPE *)GetImPtr(im);
  PIX_TYPE *lut=(PIX_TYPE *)GetImPtr(imlut);

#pragma omp parallel for
  for(i=0;i<npix;i++){
    if(p[i]>maxlutidx)
      (void)sprintf(buf,"warning: lookup(): LUT not matching image values\n");
    else
      p[i]=lut[p[i]];
  }
  return NO_ERROR;
}
#include "uc_undef.h"

#include "u32_def.h"
ERROR_TYPE u32_lookuptypematch(IMAGE *im, IMAGE *imlut)
{
  /* generic code for unsigned integer types only */
  long int i;
  long int npix=GetImNPix(im);
  int maxlutidx=GetImNPix(imlut)-1;
  PIX_TYPE *p=(PIX_TYPE *)GetImPtr(im);
  PIX_TYPE *lut=(PIX_TYPE *)GetImPtr(imlut);

#pragma omp parallel for
  for(i=0;i<npix;i++){
    if(p[i]>maxlutidx)
      (void)sprintf(buf,"warning: lookup(): LUT not matching image values\n");
    else
      p[i]=lut[p[i]];
  }
  return NO_ERROR;
}
#include "u32_undef.h"



ERROR_TYPE lookuptypematch(IMAGE *im, IMAGE *imlut)
{
  if (GetImDataType(imlut)!=GetImDataType(im)){
    (void)sprintf(buf,"lookup(): imlut must be with the same type as im\n");
    errputstr(buf);
    return ERROR;
  }

  switch (GetImDataType(im)){
  case t_UCHAR:
    return(uc_lookuptypematch(im, imlut));
    break;
  case t_UINT32:
    return(u32_lookuptypematch(im, imlut));
    break;
  default:
    (void)sprintf(buf,"lookuptypematch(): invalid pixel type\n");
    errputstr(buf);
  }
  return(ERROR);
}


#include "uc_def.h"
#define LUTRGB_TYPE UCHAR
IMAGE *uc_lookuprgb(IMAGE *imr, IMAGE *img, IMAGE *imb, IMAGE *imlut)
{
  /*
    Developed for colour histogram matching (for GMES CORE003 data set)

    First 20130430
  */

  /* generic code for unsigned integer types only */
  IMAGE *imout;
  long int i,idx;
  long int npix=GetImNPix(imr);
  int maxlutidx=GetImNPix(imlut)-1;
  int nxlut=GetImNx(imlut);
  int nylut=GetImNy(imlut);
  PIX_TYPE *p;
  PIX_TYPE *pr=(PIX_TYPE *)GetImPtr(imr);
  PIX_TYPE *pg=(PIX_TYPE *)GetImPtr(img);
  PIX_TYPE *pb=(PIX_TYPE *)GetImPtr(imb);
  LUTRGB_TYPE *lut=(LUTRGB_TYPE *)GetImPtr(imlut);

  imout=create_image(t_PIX_TYPE, GetImNx(imr), GetImNy(imr), GetImNz(imr));
  if (imout==NULL)
    return NULL;
  p=(PIX_TYPE *)GetImPtr(imout);

#pragma omp parallel for private(i,idx)
  for(i=0;i<npix;i++){
    idx=pr[i]+pg[i]*nxlut+pb[i]*nxlut*nylut;
    if(idx>maxlutidx)
      (void)sprintf(buf,"warning: lookuprgb(): LUT not matching image values\n");
    else if (lut[idx]>PIX_MAX){
      (void)sprintf(buf,"warning: lookuprgb(): LUT value greater than PIX_MAX value (output set to PIX_MAX)\n");
      p[i]=PIX_MAX;
    }
    else if (lut[idx]<PIX_MIN){
      (void)sprintf(buf,"warning: lookuprgb(): LUT value lower than PIX_MIN value (output set to PIX_MIN)\n");
      p[i]=PIX_MIN;
    }
    else{
#if FLOATING
      p[i]=(PIX_TYPE)roundf(lut[idx]);
#else
      p[i]=(PIX_TYPE)lut[idx];
#endif
    }
  }
  return imout;
}
#undef LUTRGB_TYPE
#include "uc_undef.h"

IMAGE *lookuprgb(IMAGE *imr, IMAGE *img, IMAGE *imb, IMAGE *imlut)
{
  if (GetImDataType(imlut)!=t_UCHAR){
    (void)sprintf(buf,"lookuprgb(): imlut must be of type UCHAR\n");
    errputstr(buf);
    return NULL;
  }
  if ( (szcompat(imr,img)!= NO_ERROR) ||  (szcompat(imr,imb)!= NO_ERROR) ){
    (void)sprintf(buf,"lookuprgb(IMAGE *imr, IMAGE *img, IMAGE *imb, IMAGE *imlut): imr, img, and imb must be of same type and size\n");
    errputstr(buf);
    return NULL;
  }
  switch (GetImDataType(imr)){
  case t_UCHAR:
    return(uc_lookuprgb(imr, img, imb, imlut));
    break;
  default:
    (void)sprintf(buf,"lookuprgb(): invalid pixel type\n");
    errputstr(buf);
  }
  return(NULL);
}




/*************************************************************************/
/*                                                                       */

#ifndef NO_generic_IMAGE
#include "g_def.h"
ERROR_TYPE generic_volume(IMAGE *im)
{
  VOL_TYPE sum = (VOL_TYPE)0;
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

  for (i=0; i<npix; i++, p1++)
    sum += *p1;

  SetImVol(im,sum);

  return(NO_ERROR);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */

#include "us_def.h"
ERROR_TYPE us_volume(IMAGE *im)
{
  VOL_TYPE sum = (VOL_TYPE)0;
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

  for (i=0; i<npix; i++, p1++)
    sum += *p1;

  SetImVol(im,sum);

  return(NO_ERROR);
}
#include "us_undef.h"

#include "s_def.h"
ERROR_TYPE s_volume(IMAGE *im)
{
  VOL_TYPE sum = (VOL_TYPE)0;
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

  for (i=0; i<npix; i++, p1++)
    sum += *p1;

  SetImVol(im,sum);

  return(NO_ERROR);
}
#include "s_undef.h"

#include "i32_def.h"
ERROR_TYPE i32_volume(IMAGE *im)
{
  VOL_TYPE sum = (VOL_TYPE)0;
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

  for (i=0; i<npix; i++, p1++)
    sum += *p1;

  SetImVol(im,sum);

  return(NO_ERROR);
}
#include "i32_undef.h"

#include "u32_def.h"
ERROR_TYPE u32_volume(IMAGE *im)
{
  VOL_TYPE sum = (VOL_TYPE)0;
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

  for (i=0; i<npix; i++, p1++)
    sum += *p1;

  SetImVol(im,sum);

  return(NO_ERROR);
}
#include "u32_undef.h"

#include "u64_def.h"
ERROR_TYPE u64_volume(IMAGE *im)
{
  VOL_TYPE sum = (VOL_TYPE)0;
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

  for (i=0; i<npix; i++, p1++)
    sum += *p1;

  SetImVol(im,sum);

  return(NO_ERROR);
}
#include "u64_undef.h"

#include "i64_def.h"
ERROR_TYPE i64_volume(IMAGE *im)
{
  VOL_TYPE sum = (VOL_TYPE)0;
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

  for (i=0; i<npix; i++, p1++)
    sum += *p1;

  SetImVol(im,sum);

  return(NO_ERROR);
}
#include "i64_undef.h"

#include "f_def.h"
ERROR_TYPE f_volume(IMAGE *im)
{
  VOL_TYPE sum = (VOL_TYPE)0;
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

  for (i=0; i<npix; i++, p1++)
    sum += *p1;

  SetImVol(im,sum);

  return(NO_ERROR);
}
#include "f_undef.h"


#include "d_def.h"
ERROR_TYPE d_volume(IMAGE *im)
{
  VOL_TYPE sum = (VOL_TYPE)0;
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

  for (i=0; i<npix; i++, p1++)
    sum += *p1;

  SetImVol(im,sum);

  return(NO_ERROR);
}
#include "d_undef.h"


ERROR_TYPE volume(IMAGE *im)
{
  switch (GetImDataType(im)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_volume(im));
    break;
#endif

#ifndef NO_uc_IMAGE
  case t_UCHAR:
    return(uc_volume(im));
    break;
#endif

  case t_USHORT:
    return(us_volume(im));
    break;

  case t_SHORT:
    return(s_volume(im));
    break;

  case t_INT32:
    return(i32_volume(im));
    break;

  case t_UINT32:
    return(u32_volume(im));
    break;

  case t_INT64:
    return(i64_volume(im));
    break;

  case t_UINT64:
    return(u64_volume(im));
    break;

  case t_FLOAT:
    return(f_volume(im));
    break;

  case t_DOUBLE:
    return(d_volume(im));
    break;

  default:
    (void)sprintf(buf,"WARNING in volume(im): invalid pixel type\n"); errputstr(buf);
  }
  return(ERROR);
}


#ifndef NO_generic_IMAGE
#include "g_def.h"
IMAGE *generic_class2d(IMAGE *im1, IMAGE *im2, IMAGE *imlut)
{
  /*
  ** buf_1:  buffer number of first image component  I(CHAR)   &  O(CHAR | SHORT)
  ** buf_2:  buffer number of second image component  I(CHAR)
  ** buf_3:  buffer number of labelled histogram    I(CHAR | SHORT)
  */

  IMAGE *imout;
  PIX_TYPE *p1, *p2, *p_end;
  HST2D_TYPE *plut, *pout;
  int nxlut;

  /* create output image */
  imout = (IMAGE *)create_image(t_HST2D, GetImNx(im1), GetImNy(im1), GetImNz(im1));
  if (imout == NULL){
    (void)sprintf(buf,"generic_class2d(): not enough memory!\n"); errputstr(buf);
    return(imout);
  }

  nxlut = GetImNx(imlut);
  p1    =(PIX_TYPE *)GetImPtr(im1);
  p2    =(PIX_TYPE *)GetImPtr(im2);
  plut  =(HST2D_TYPE *)GetImPtr(imlut);
  pout  =(HST2D_TYPE *)GetImPtr(imout);


  p_end = p1 + GetImNPix(im1);
  for (; p1 < p_end; p1++, p2++, pout++)
    *pout = plut[*p2 * nxlut + *p1];

  return imout;
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */


#include "us_def.h"
IMAGE *us_class2d(IMAGE *im1, IMAGE *im2, IMAGE *imlut)
{
  /*
  ** buf_1:  buffer number of first image component  I(CHAR)   &  O(CHAR | SHORT)
  ** buf_2:  buffer number of second image component  I(CHAR)
  ** buf_3:  buffer number of labelled histogram    I(CHAR | SHORT)
  */

  IMAGE *imout;
  PIX_TYPE *p1, *p2, *p_end;
  HST2D_TYPE *plut, *pout;
  int nxlut;

  /* create output image */
  imout = (IMAGE *)create_image(t_HST2D, GetImNx(im1), GetImNy(im1), GetImNz(im1));
  if (imout == NULL){
    (void)sprintf(buf,"us_class2d(): not enough memory!\n"); errputstr(buf);
    return(imout);
  }

  nxlut = GetImNx(imlut);
  p1    =(PIX_TYPE *)GetImPtr(im1);
  p2    =(PIX_TYPE *)GetImPtr(im2);
  plut  =(HST2D_TYPE *)GetImPtr(imlut);
  pout  =(HST2D_TYPE *)GetImPtr(imout);


  p_end = p1 + GetImNPix(im1);
  for (; p1 < p_end; p1++, p2++, pout++)
    *pout = plut[*p2 * nxlut + *p1];

  return imout;
}
#include "us_undef.h"


#include "us_def.h"
#define HST2D_TYPE_LOCAL USHORT
#define t_HST2D_LOCAL t_USHORT
IMAGE *usus_class2d(IMAGE *im1, IMAGE *im2, IMAGE *imlut)
{
  /*
  ** buf_1:  buffer number of first image component  I(CHAR)   &  O(CHAR | SHORT)
  ** buf_2:  buffer number of second image component  I(CHAR)
  ** buf_3:  buffer number of labelled histogram    I(CHAR | SHORT)
  */

  IMAGE *imout;
  PIX_TYPE *p1, *p2, *p_end;
  HST2D_TYPE_LOCAL *plut, *pout;
  int nxlut;

  /* create output image */
  imout = (IMAGE *)create_image(t_HST2D_LOCAL, GetImNx(im1), GetImNy(im1), GetImNz(im1));
  if (imout == NULL){
    (void)sprintf(buf,"us_class2d(): not enough memory!\n"); errputstr(buf);
    return(imout);
  }

  nxlut = GetImNx(imlut);
  p1    =(PIX_TYPE *)GetImPtr(im1);
  p2    =(PIX_TYPE *)GetImPtr(im2);
  plut  =(HST2D_TYPE_LOCAL *)GetImPtr(imlut);
  pout  =(HST2D_TYPE_LOCAL *)GetImPtr(imout);


  p_end = p1 + GetImNPix(im1);
  for (; p1 < p_end; p1++, p2++, pout++)
    *pout = plut[*p2 * nxlut + *p1];

  return imout;
}
#undef HST2D_TYPE_LOCAL
#undef t_HST2D_LOCAL
#include "us_undef.h"



IMAGE *class2d(IMAGE *im1, IMAGE *im2, IMAGE *imlut)
{

  /* check image compatibilty */

  if (szcompat(im1, im2) != NO_ERROR){
    (void)sprintf(buf,"ERROR in *class2d(im1, im2): \
                images of different size or type\n"); errputstr(buf);
    return(NULL);
  }
  if (GetImDataType(imlut)!= t_HST2D){
    (void)sprintf(buf,"ERROR in *class2d(im1, im2): \
                2D LUT must be of type t_HST2D\n"); errputstr(buf);
    return(NULL);
  }

  switch (GetImDataType(im1)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_class2d(im1, im2, imlut));
    break;
#endif
  case t_USHORT:
    switch (GetImDataType(imlut)){
    case t_HST2D:
      return(us_class2d(im1, im2, imlut));
      break;
    case t_USHORT:
      return(usus_class2d(im1, im2, imlut));
      break;
    default:
      (void)sprintf(buf,"WARNING in *class2d(im1, im2, imlut): invalid imlut  type\n"); errputstr(buf);
      return NULL;
    }
  }
  return NULL;
}





#include "us_def.h"
#define OUT_TYPE long int /* was double   */
#define t_OUT t_INT32     /* was t_DOUBLE */
IMAGE *us_area(IMAGE *im, int r, int type)
{
  PIX_TYPE *pin;
  IMAGE *imout;
  OUT_TYPE *pout;
  int x,y, nx=GetImNx(im), ny=GetImNy(im), r2=r*r, twor2;
  double a,b,c,d,e,s,aire;

  /* create output image */
  imout = create_image(t_OUT, nx-1, ny-1, 1);
  if (imout == NULL){
    (void)sprintf(buf,"us_area(): not enough memory!\n"); errputstr(buf);
    return(NULL);
  }

  pin =(PIX_TYPE *)GetImPtr(im);
  pout=(OUT_TYPE *)GetImPtr(imout);
  twor2 = 2*r2;

  switch(type){
  case 0: /* 45 degrees triangulation */
    for (y=1; y<ny; y++){
      for (x=1; x<nx; x++){
	a=sqrt(SQ(fabs((double) *pin     - *(pin+1)))  + r2);
	b=sqrt(SQ(fabs((double) *pin     - *(pin+nx))) + r2);
	c=sqrt(SQ(fabs((double) *(pin+1) - *(pin+nx))) + twor2);
	s = (a+b+c)/2.0;
	aire=sqrt( s*(s-a)*(s-b)*(s-c) );

	a=sqrt(SQ(fabs((double) *(pin+1)  - *(pin+nx+1)))  + r2);
	b=sqrt(SQ(fabs((double) *(pin+nx) - *(pin+nx+1)))  + r2);
	s = (a+b+c)/2.0;
	aire+=sqrt( s*(s-a)*(s-b)*(s-c) );
	*pout++ =aire;
	pin++;
      }
      pin++;
    }
    return imout;
  case 1: /* -45 degrees triangulation */
    for (y=1; y<ny; y++){
      for (x=1; x<nx; x++){
	a=sqrt(SQ( (double) *(pin+nx) - *(pin+nx+1)) + r2);
	b=sqrt(SQ( (double) *pin      - *(pin+nx))   + r2);
	c=sqrt(SQ( (double) *pin      - *(pin+nx+1)) + twor2);
	s = (a+b+c)/2.0;
	aire=sqrt( s*(s-a)*(s-b)*(s-c) );

	a=sqrt(SQ( (double) *pin     - *(pin+1))    + r2);
	b=sqrt(SQ( (double) *(pin+1) - *(pin+nx+1)) + r2);
	s = (a+b+c)/2.0;
	aire+=sqrt( s*(s-a)*(s-b)*(s-c) );
	*pout++ =aire;
	pin++;
      }
      pin++;
    }
    return imout;
  case 2: /*  triangular prism method */
    for (y=1; y<ny; y++){
      for (x=1; x<nx; x++){
	a = *pin;
	b = *(pin+1);
	c = *(pin+nx);
	d = *(pin+nx+1);
	e = (a+b+c+d )/2.0;  /* interpolated centre value already multiplied by 2 */
	aire =  sqrt( SQ(b-a) + SQ(e-a-b) + r2 );
	aire += sqrt( SQ(c-b) + SQ(e-b-c) + r2 );
	aire += sqrt( SQ(d-c) + SQ(e-c-d) + r2 );
	aire += sqrt( SQ(a-d) + SQ(e-d-a) + r2 );
	aire *= r/4.0;
	*pout++ =aire;
	pin++;
      }
      pin++;
    }
    return imout;
  default:
    (void)sprintf(buf,"us_area(IMAGE *im, int r, int type): invalid type (must be in [0,4])\n"); errputstr(buf);
    return(NULL);
  }
}
#undef OUT_TYPE
#undef t_OUT
#include "us_undef.h"


/* 2003-03-17
   computes the surface area of a grey tone image by setting
   the corner of each square cell to the area of its square influence
   zone.  It follows that the returned image is one pixel less in x
   and y directions.  The variable type is used for specifying the way
   the area is calculated:
   0 for triangulation at  45 degrees,
   1 for triangulation at -45 degrees,
   2 for triangular prism method.
   r defines the length of the side of the square pixel expressed in the
   units of the pixel intensity values

   Idea: take flow directions into account for defining the type of triangulation
   when the flow directions are at +/- 45 degrees.  Triangular prism could be
   considered otherwise.
*/
IMAGE *area(IMAGE *im, int r, int type)
{

  switch (GetImDataType(im)){

  case t_USHORT:
    return(us_area(im, r, type));
    break;

  default:
    (void)sprintf(buf,"area(IMAGE *im, int r, int type): invalid pixel type for im\n"); errputstr(buf);
    return(NULL);
  }
  return(NULL);
}

#include "uc_def.h"
ERROR_TYPE uc_dirmax(IMAGE *im, int dir)
{
  /*
  ** im: an 1d array for a 2d image
  ** nx: number of image columns
  ** ny: number of image lines
  ** dir: direction 0=down-top, 1=left-right
  **                2=top-down, 3=right-left
  */

  int		i,j,nx,ny;
  PIX_TYPE	max=0,*max_a;
  PIX_TYPE	*pim, *aux_max_a,*aux_im;

  pim = (PIX_TYPE *)GetImPtr(im);
  nx  = GetImNx(im);
  ny  = GetImNy(im);

  switch(dir){

  case 0:
    if((max_a=(PIX_TYPE *)malloc(nx*sizeof(PIX_TYPE)))==(PIX_TYPE *)NULL)
      return(ERROR);
    for(i=0,aux_max_a=max_a;i<nx;i++,*aux_max_a++=0) ;
    aux_im=pim+nx*ny-1;
    for(j=0;j<ny;j++)
      for(i=0,aux_max_a=max_a;i<nx;i++)
	if(*aux_im>*aux_max_a)
	  *aux_max_a++=*aux_im--;
	else
	  *aux_im--=*aux_max_a++;
    free((char*)max_a);
    break;

  case 1:
    aux_im=pim;
    for(j=0;j<ny;j++)
      for(i=0,max=0;i<nx;i++)
	if(*aux_im>max)
	  max=*aux_im++;
	else
	  *aux_im++=max;
    break;

  case 2:
    if((max_a=(PIX_TYPE *)malloc(nx*sizeof(PIX_TYPE)))==(PIX_TYPE *)NULL)
      return(ERROR);
    for(i=0,aux_max_a=max_a;i<nx;i++,*aux_max_a++=0) ;
    aux_im=pim;
    for(j=0;j<ny;j++)
      for(i=0,aux_max_a=max_a;i<nx;i++)
	if(*aux_im>*aux_max_a)
	  *aux_max_a++=*aux_im++;
	else
	  *aux_im++=*aux_max_a++;
    free((char*)max_a);
    break;

  case 3:
    aux_im=pim+nx*ny-1;
    for(j=0;j<ny;j++)
      for(i=0,max=0;i<nx;i++)
	if(*aux_im>max)
	  max=*aux_im--;
	else
	  *aux_im--=max;
    break;

  default:
    return(ERROR);
  }
  return(NO_ERROR);
}
#include "uc_undef.h"

#include "us_def.h"
ERROR_TYPE us_dirmax(IMAGE *im, int dir)
{
  /*
  ** im: an 1d array for a 2d image
  ** nx: number of image columns
  ** ny: number of image lines
  ** dir: direction 0=down-top, 1=left-right
  **                2=top-down, 3=right-left
  */

  int		i,j,nx,ny;
  PIX_TYPE	max=0,*max_a;
  PIX_TYPE	*pim, *aux_max_a,*aux_im;

  pim = (PIX_TYPE *)GetImPtr(im);
  nx  = GetImNx(im);
  ny  = GetImNy(im);

  switch(dir){

  case 0:
    if((max_a=(PIX_TYPE *)malloc(nx*sizeof(PIX_TYPE)))==(PIX_TYPE *)NULL)
      return(ERROR);
    for(i=0,aux_max_a=max_a;i<nx;i++,*aux_max_a++=0) ;
    aux_im=pim+nx*ny-1;
    for(j=0;j<ny;j++)
      for(i=0,aux_max_a=max_a;i<nx;i++)
	if(*aux_im>*aux_max_a)
	  *aux_max_a++=*aux_im--;
	else
	  *aux_im--=*aux_max_a++;
    free((char*)max_a);
    break;

  case 1:
    aux_im=pim;
    for(j=0;j<ny;j++)
      for(i=0,max=0;i<nx;i++)
	if(*aux_im>max)
	  max=*aux_im++;
	else
	  *aux_im++=max;
    break;

  case 2:
    if((max_a=(PIX_TYPE *)malloc(nx*sizeof(PIX_TYPE)))==(PIX_TYPE *)NULL)
      return(ERROR);
    for(i=0,aux_max_a=max_a;i<nx;i++,*aux_max_a++=0) ;
    aux_im=pim;
    for(j=0;j<ny;j++)
      for(i=0,aux_max_a=max_a;i<nx;i++)
	if(*aux_im>*aux_max_a)
	  *aux_max_a++=*aux_im++;
	else
	  *aux_im++=*aux_max_a++;
    free((char*)max_a);
    break;

  case 3:
    aux_im=pim+nx*ny-1;
    for(j=0;j<ny;j++)
      for(i=0,max=0;i<nx;i++)
	if(*aux_im>max)
	  max=*aux_im--;
	else
	  *aux_im--=max;
    break;

  case 31:
    for(j=0;j<ny;j++){
      aux_im=pim+(j*nx);
      for(i=0,max=0;i<nx;i++,aux_im++)
	if(*aux_im>max)
	  max=*aux_im;
      aux_im=pim+(j*nx);
      for(i=0;i<nx;i++,aux_im++)
	if(*aux_im!=max)
	  *aux_im=0;
    }
  break;

  case 20:
    for(j=0;j<nx;j++){
      aux_im=pim+j;
      for(i=0,max=0;i<ny;i++,aux_im+=nx)
	if(*aux_im>max)
	  max=*aux_im;
      aux_im=pim+j;
      for(i=0;i<ny;i++,aux_im+=nx)
	if(*aux_im!=max)
	  *aux_im=0;
    }
  break;

  default:
    return(ERROR);
  }
  return(NO_ERROR);
}
#include "us_undef.h"



#include "f_def.h"
ERROR_TYPE f_dirmax(IMAGE *im, int dir)
{
  /*
  ** im: an 1d array for a 2d image
  ** nx: number of image columns
  ** ny: number of image lines
  ** dir: direction 0=down-top, 1=left-right
  **                2=top-down, 3=right-left
  */

  int		i,j,nx,ny;
  PIX_TYPE	max=0,*max_a;
  PIX_TYPE	*pim, *aux_max_a,*aux_im;

  pim = (PIX_TYPE *)GetImPtr(im);
  nx  = GetImNx(im);
  ny  = GetImNy(im);

  switch(dir){

  case 0:
    if((max_a=(PIX_TYPE *)malloc(nx*sizeof(PIX_TYPE)))==(PIX_TYPE *)NULL)
      return(ERROR);
    for(i=0,aux_max_a=max_a;i<nx;i++,*aux_max_a++=0) ;
    aux_im=pim+nx*ny-1;
    for(j=0;j<ny;j++)
      for(i=0,aux_max_a=max_a;i<nx;i++)
	if(*aux_im>*aux_max_a)
	  *aux_max_a++=*aux_im--;
	else
	  *aux_im--=*aux_max_a++;
    free((char*)max_a);
    break;

  case 1:
    aux_im=pim;
    for(j=0;j<ny;j++)
      for(i=0,max=0;i<nx;i++)
	if(*aux_im>max)
	  max=*aux_im++;
	else
	  *aux_im++=max;
    break;

  case 2:
    if((max_a=(PIX_TYPE *)malloc(nx*sizeof(PIX_TYPE)))==(PIX_TYPE *)NULL)
      return(ERROR);
    for(i=0,aux_max_a=max_a;i<nx;i++,*aux_max_a++=0) ;
    aux_im=pim;
    for(j=0;j<ny;j++)
      for(i=0,aux_max_a=max_a;i<nx;i++)
	if(*aux_im>*aux_max_a)
	  *aux_max_a++=*aux_im++;
	else
	  *aux_im++=*aux_max_a++;
    free((char*)max_a);
    break;

  case 3:
    aux_im=pim+nx*ny-1;
    for(j=0;j<ny;j++)
      for(i=0,max=0;i<nx;i++)
	if(*aux_im>max)
	  max=*aux_im--;
	else
	  *aux_im--=max;
    break;

  default:
    return(ERROR);
  }
  return(NO_ERROR);
}
#include "f_undef.h"


ERROR_TYPE dirmax(IMAGE *im, int dir)
{

  switch (GetImDataType(im)){

  case t_UCHAR:
    return(uc_dirmax(im, dir));
    break;

  case t_USHORT:
    return(us_dirmax(im, dir));
    break;

  case t_FLOAT:
    return(f_dirmax(im, dir));
    break;

  default:
    (void)sprintf(buf, "Error in dirmax(): \
                invalid ImDataType\n"); errputstr(buf);
    return(ERROR);
  }
  return(ERROR);
}



#include "f_def.h"
#define PIX_TYPE_OUT float
IMAGE *f_dirsum(IMAGE *im, int dir)
{
  /*
  ** im: an image
  ** dir: direction 0=vertical, 1=horizontal
  **
  */

  int		i,j,nx,ny;
  PIX_TYPE	*pim;
  IMAGE         *imout;
  PIX_TYPE_OUT  *pout;

  pim = (PIX_TYPE *)GetImPtr(im);
  nx  = GetImNx(im);
  ny  = GetImNy(im);

  switch(dir){

  case 0:
    imout=create_image(t_FLOAT, nx, 1, 1);
    pout=(PIX_TYPE_OUT *)GetImPtr(imout);

    for(j=0;j<ny;j++){
      for(i=0; i<nx;i++)
	*pout++ +=*pim++;
      pout-=nx;
    }
    break;

  case 1:
    imout=create_image(t_FLOAT, 1, ny, 1);
    pout=(PIX_TYPE_OUT *)GetImPtr(imout);

    for(j=0;j<ny;j++){
      for(i=0; i<nx;i++)
	*pout +=*pim++;
      pout++;
    }
    break;

  default:
    return(NULL);
  }
  return(imout);
}
#undef PIX_TYPE_OUT
#include "f_undef.h"




IMAGE *dirsum(IMAGE *im, int dir)
{
  switch (GetImDataType(im)){

  case t_FLOAT:
    return(f_dirsum(im, dir));
    break;

  default:
    (void)sprintf(buf, "Error in dirsum(): \
                invalid ImDataType\n"); errputstr(buf);
    return(NULL);
  }
  return(NULL);
}


/*************************************************************************/
/*                 predicate for identity test between 2 images          */


/**
 * @synopsis Image equality predicate
 *
 * @param im1 : a pointer to an IMAGE

 * @param im2 : a pointer to an IMAGE
 *
 * @return 0 if the two input images have identical pixel values (and data type), 1 otherwise
 *
 * @creationdate
 */
ERROR_TYPE imequalp(IMAGE *im1, IMAGE *im2)
{
  int *pim1, *pim2;
  unsigned long int i, nbyte, nword;

  nbyte = GetImNByte(im1);
  nword=nbyte/sizeof(int);

  pim1 = (int *)GetImPtr(im1);
  pim2 = (int *)GetImPtr(im2);

  /* error checking */
  if (GetImNByte(im2) != nbyte){
    (void)sprintf(buf, "ERROR in imequalp(im1, im2): images do not have the same number of bytes\n"); errputstr(buf);
    return(ERROR);
  }

  /* here we go */
  for (i=0; i<nword; i++, pim1++, pim2++)
    if (*pim1 ^ *pim2)
      break;
  if (i==nword)
    return NO_ERROR; /* equal */
  return ERROR; /* differ */
}




/*************************************************************************/
/*                 get maximum value of an image                         */

ERROR_TYPE getmax(IMAGE *im, double *maxval)
{
  G_TYPE *pg, pgs;

  /* get min values */
  pg = min_max(im);
  if (pg == NULL)
    return ERROR;
  pgs = pg[1];
  free((char *)pg);

  switch(GetImDataType(im)){
  case t_UCHAR:
    *maxval=(double) pgs.uc_val;
    break;
  case t_USHORT:
    *maxval=(double) pgs.us_val;
    break;
  case t_SHORT:
    *maxval=(double) pgs.us_val;
    break;
  case t_INT32:
    *maxval=(double) pgs.i32_val;
    break;
  case t_UINT32:
    *maxval=(double) pgs.u32_val;
    break;
  case t_INT64:
    *maxval=(long int) pgs.i64_val;
    break;
  case t_UINT64:
    *maxval=(unsigned long) pgs.u64_val;
    break;
  case t_FLOAT:
    *maxval=(double) pgs.f_val;
    break;
  case t_DOUBLE:
    *maxval=(double) pgs.d_val;
    break;
  default:
    (void)sprintf(buf, "error in getmax: \
                undefined image data type\n"); errputstr(buf);
    return(ERROR);
  }

  return NO_ERROR;
}

/*************************************************************************/
/*                 get maximum value of an image                         */

ERROR_TYPE getminmax(IMAGE *im, double *minval, double *maxval)
{
  G_TYPE *pg, pgmin, pgmax;

  /* get min values */
  pg = min_max(im);
  if (pg == NULL)
    return ERROR;
  pgmin = pg[0];
  pgmax = pg[1];
  free((char *)pg);

  switch(GetImDataType(im)){
  case t_UCHAR:
    *minval=(double) pgmin.uc_val;
    *maxval=(double) pgmax.uc_val;
    break;
  case t_USHORT:
    *minval=(double) pgmin.us_val;
    *maxval=(double) pgmax.us_val;
    break;
  case t_SHORT:
    *minval=(double) pgmin.s_val;
    *maxval=(double) pgmax.s_val;
    break;
  case t_INT32:
    *minval=(double) pgmin.i32_val;
    *maxval=(double) pgmax.i32_val;
    break;
  case t_UINT32:
    *minval=(double) pgmin.u32_val;
    *maxval=(double) pgmax.u32_val;
    break;
  case t_INT64:
    printf("min/max values may not fit a double!\n");
    *minval=(double) pgmin.i64_val;
    *maxval=(double) pgmax.i64_val;
    break;
  case t_UINT64:
    printf("min/max values may not fit a double!\n");
    *minval=(double) pgmin.u64_val;
    *maxval=(double) pgmax.u64_val;
    break;
  case t_FLOAT:
    *minval=(double) pgmin.f_val;
    *maxval=(double) pgmax.f_val;
    break;
  case t_DOUBLE:
    *minval=(double) pgmin.d_val;
    *maxval=(double) pgmax.d_val;
    break;
  default:
    (void)sprintf(buf, "error in getmax: \
                undefined image data type\n"); errputstr(buf);
    return(ERROR);
  }
  return NO_ERROR;
}



/**@}*/
