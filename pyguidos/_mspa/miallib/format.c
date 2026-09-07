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
#include <math.h>
#include "miallib.h"

#ifdef OPENMP
#include <omp.h>
#endif

/** \defgroup group_format Type conversions
 *  Functions related to the image type conversions.
 *  @{
 */


/*
   One has to be careful with data type conversions where the values
   are not just cast but also rescaled.  Check consistency!
   Think about sign when going from signed to unsigned.

   When planning parallel for with openmp beware that it does NOT
   work with destructive conversion (20120329)

*/


/*************************************************************************/
/*                                                                       */
IMAGE *to_tiff1bitpp(IMAGE *im)
{
  IMAGE *imout;
  UCHAR *ptmp, *ptr_compac;
  int nx=GetImNx(im), nlin=GetImNlin(im);
  int i, j, k, der, reste;

  if (GetImDataType(im) != t_UCHAR){
    (void)sprintf(buf,"IMAGE *to_tiff1bitpp(im): not t_UCHAR image!\n"); errputstr(buf); errputstr(buf);
    return(NULL);
  }

  /* create output image */
  imout = create_image(t_TIFFONEBITPERPIXEL, GetImNx(im), GetImNy(im), GetImNz(im));
  if (imout == NULL){
    (void)sprintf(buf,"IMAGE *to_tiff1bitpp(im): not enough memory!\n"); errputstr(buf);
    return(NULL);
  }

  ptmp   = (UCHAR *)GetImPtr(imout);
  ptr_compac = (UCHAR *)GetImPtr(im);
  der    = nx / 8;
  reste  = nx % 8;
  reste  = 8 - reste;
  for (k = 0; k < nlin; k++){
    for (i = 0; i < der; i++){
      *ptmp = (*ptr_compac++)<<7;
      for (j = 6; j >= 0; j--)
	*ptmp |= (*ptr_compac++)<<j;
      ptmp++;
    }
    if (reste != 8){
      *ptmp = (*ptr_compac++)<<7;
      for (i = 6; i >= reste; i--)
	*ptmp |= (*ptr_compac++)<<i;
      ptmp++;
    }
  }
  SetImNByte(imout, nlin*(nx/BITPERCHAR+(nx%8 ? 1 : 0)));
  return(imout);
}
/*************************************************************************/
/*                                                                       */
IMAGE *to_tiff4bitpp(IMAGE *im)
{
  IMAGE *imout;
  UCHAR *ptmp, *ptr_compac;
  int nx=GetImNx(im), nlin=GetImNlin(im);
  int i, k, der, reste;

  if (GetImDataType(im) != t_UCHAR){
    (void)sprintf(buf,"IMAGE *to_tiff4bitpp(im): not t_UCHAR image!\n"); errputstr(buf);
    return(NULL);
  }

  /* create output image */
  imout = create_image(t_FOURBITPERPIXEL, GetImNx(im), GetImNy(im), GetImNz(im));
  if (imout == NULL){
    (void)sprintf(buf,"IMAGE *to_tiff4bitpp(im): not enough memory!\n"); errputstr(buf);
    return(NULL);
  }

  ptmp   = (UCHAR *)GetImPtr(imout);
  ptr_compac = (UCHAR *)GetImPtr(im);
  der        = nx / 2;
  reste      = nx % 2;

  for (k = 0; k < nlin; k++){
    for (i = 0; i < der; i++){
      *ptmp = (*ptr_compac++)<<4;
      *ptmp++ |= (*ptr_compac++);
      }
    if (reste != 0)
      *ptmp++ = (*ptr_compac++)<<4;
  }
  SetImNByte(imout, nlin*(nx/2+(nx%2)));
  if (GetImLut(im) != NULL)
    copy_lut(imout, im);
  return(imout);
}

#include "b_def.h"
ERROR_TYPE b_to_uchar(IMAGE *im, IMAGE *imout)
     /* not used (beware that there are two input arguments
      contrary to all other to_uchar functions */
{
  unsigned long int i, npix, der, reste;
  long int nx, ny, nz;
  int k, j;
  UCHAR *p2;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  p2   = (UCHAR *)GetImPtr(imout);
  nx   = GetImNx(im);
  ny   = GetImNy(im);
  nz   = GetImNz(im);
  npix = nx*ny*nz;

  der   = nx/BITPERWORD;
  reste = nx%BITPERWORD;

  for (k=0; k < npix; k+=nx){
    for (i=0; i<der; i++, p1++){
      for (j=0; j < BITPERWORD; j++){
	p2[BITPERWORD*i+j+k] = *p1 & (W_MSB>>j);
      }
    }
    if (reste){
      for (j=0; j < reste; j++){
	p2[BITPERWORD*i+j+k] = *p1 & (W_MSB>>j);
      }
      p1++;
    }
  }
  return(NO_ERROR);
}
#include "b_undef.h"

#include "us_def.h"
ERROR_TYPE us_to_uchar(IMAGE *im)
{
  mia_size_t i, npix;
  unsigned long int nbyte;
  UCHAR *p2;
  PIX_TYPE *p1, mini, maxi;
  double range;
  G_TYPE *pg;

  p1    = (PIX_TYPE *)GetImPtr(im);
  p2    = (UCHAR *)GetImPtr(im);
  npix  = GetImNPix(im);
  pg    = min_max(im);
  if (pg == NULL)
    return(ERROR);
  mini  = pg[0].us_val;
  maxi  = pg[1].us_val;
  free((char *)pg);
  range = (double)maxi - mini;

#if SIGNED
  if ( (maxi <= UCHAR_MAX) && (mini >= UCHAR_MIN) )
#else
  if (maxi <= UCHAR_MAX)
#endif
    for (i=0; i<npix; i++)
      p2[i] = (UCHAR)(p1[i]);
  else if ( ((double)maxi-mini) <= (double)UCHAR_MAX){
    for (i=0; i<npix; i++)
      p2[i] = (UCHAR)(p1[i] - mini);
  }
  else{
    for (i=0; i<npix; i++)
      p2[i] = (UCHAR)(((double)p1[i] - mini)/range * UCHAR_MAX);
  }

  nbyte=GetImNPix(im);
  if (nbyte%sizeof(long int)) /* pad for word size */
    nbyte+=sizeof(long int);
  p1=(PIX_TYPE *)realloc((void *)GetImPtr(im), nbyte);
  if(p1==NULL)
    return ERROR;
  SetImDataType(im, t_UCHAR);
  SetImPtr(im, p1);
  SetImNByte(im, nbyte);

  return(NO_ERROR);
}
#include "us_undef.h"

#include "s_def.h"
ERROR_TYPE s_to_uchar(IMAGE *im)
{
  mia_size_t i, npix;
  unsigned long int nbyte;
  UCHAR *p2;
  PIX_TYPE *p1, mini, maxi;
  double range;
  G_TYPE *pg;

  p1    = (PIX_TYPE *)GetImPtr(im);
  p2    = (UCHAR *)GetImPtr(im);
  npix  = GetImNPix(im);
  pg    = min_max(im);
  if (pg == NULL)
    return(ERROR);
  mini  = pg[0].s_val;
  maxi  = pg[1].s_val;
  free((char *)pg);
  range = (double)maxi - mini;

  if ( (maxi <= UCHAR_MAX) && (mini >= UCHAR_MIN) )
    for (i=0; i<npix; i++, p1++, p2++)
      *p2 = (UCHAR)(*p1);
  else if ( ((double)maxi-mini) <= (double)UCHAR_MAX){
    for (i=0; i<npix; i++, p1++, p2++)
      *p2 = (UCHAR)(*p1 - mini);
  }
  else{
    for (i=0; i<npix; i++, p1++, p2++)
      *p2 = (UCHAR)(((double)*p1 - mini)/range * UCHAR_MAX);
  }

  nbyte=GetImNPix(im);
  if (nbyte%sizeof(long int)) /* pad for word size */
    nbyte+=sizeof(long int);
  p1=(PIX_TYPE *)realloc((void *)GetImPtr(im), nbyte);
  if(p1==NULL)
    return ERROR;
  SetImDataType(im, t_UCHAR);
  SetImPtr(im, p1);
  SetImNByte(im, nbyte);

  return(NO_ERROR);
}
#include "s_undef.h"

#include "i32_def.h"
ERROR_TYPE i32_to_uchar(IMAGE *im)
{
  mia_size_t i, npix;
  unsigned long int nbyte;
  UCHAR *p2;
  PIX_TYPE *p1, mini, maxi;
  double range;
  G_TYPE *pg;

  p1    = (PIX_TYPE *)GetImPtr(im);
  p2    = (UCHAR *)GetImPtr(im);
  npix  = GetImNPix(im);
  pg    = min_max(im);
  if (pg == NULL)
    return(ERROR);
  mini  = pg[0].i32_val;
  maxi  = pg[1].i32_val;
  free((char *)pg);
  range = (double)maxi - mini;

  if ( (maxi <= UCHAR_MAX) && (mini >= UCHAR_MIN) )
    for (i=0; i<npix; i++)
      p2[i] = (UCHAR)(p1[i]);
  else if ( ((double)maxi-mini) <= (double)UCHAR_MAX){
    for (i=0; i<npix; i++)
      p2[i] = (UCHAR)(p1[i] - mini);
  }
  else{
    for (i=0; i<npix; i++)
      p2[i] = (UCHAR)(((double)p1[i] - mini)/range * UCHAR_MAX);
  }

  nbyte=GetImNPix(im);
  if (nbyte%sizeof(long int)) /* pad for word size */
    nbyte+=sizeof(long int);
  p1=(PIX_TYPE *)realloc((void *)GetImPtr(im), nbyte);
  if(p1==NULL)
    return ERROR;
  SetImDataType(im, t_UCHAR);
  SetImPtr(im, p1);
  SetImNByte(im, nbyte);

  return(NO_ERROR);
}
#include "i32_undef.h"

#include "u32_def.h"
ERROR_TYPE u32_to_uchar(IMAGE *im)
{
  mia_size_t i, npix;
  unsigned long int nbyte;
  UCHAR *p2;
  PIX_TYPE *p1, mini, maxi;
  double range;
  G_TYPE *pg;

  p1    = (PIX_TYPE *)GetImPtr(im);
  p2    = (UCHAR *)GetImPtr(im);
  npix  = GetImNPix(im);
  pg    = min_max(im);
  if (pg == NULL)
    return(ERROR);
  mini  = pg[0].u32_val;
  maxi  = pg[1].u32_val;
  free((char *)pg);
  range = (double)maxi - mini;

  if ( (maxi <= UCHAR_MAX) && (mini >= UCHAR_MIN) )
    for (i=0; i<npix; i++, p1++, p2++)
      *p2 = (UCHAR)(*p1);
  else if ( ((double)maxi-mini) <= (double)UCHAR_MAX){
    for (i=0; i<npix; i++, p1++, p2++)
      *p2 = (UCHAR)(*p1 - mini);
  }
  else{
    for (i=0; i<npix; i++, p1++, p2++)
      *p2 = (UCHAR)(((double)*p1 - mini)/range * UCHAR_MAX);
  }

  nbyte=GetImNPix(im);
  if (nbyte%sizeof(long int)) /* pad for word size */
    nbyte+=sizeof(long int);
  p1=(PIX_TYPE *)realloc((void *)GetImPtr(im), nbyte);
  if(p1==NULL)
    return ERROR;
  SetImDataType(im, t_UCHAR);
  SetImPtr(im, p1);
  SetImNByte(im, nbyte);

  return(NO_ERROR);
}
#include "u32_undef.h"

#include "i64_def.h"
ERROR_TYPE i64_to_uchar(IMAGE *im)
{
  mia_size_t i, npix;
  unsigned long int nbyte;
  UCHAR *p2;
  PIX_TYPE *p1, mini, maxi;
  double range;
  G_TYPE *pg;

  p1    = (PIX_TYPE *)GetImPtr(im);
  p2    = (UCHAR *)GetImPtr(im);
  npix  = GetImNPix(im);
  pg    = min_max(im);
  if (pg == NULL)
    return(ERROR);
  mini  = pg[0].i64_val;
  maxi  = pg[1].i64_val;
  free((char *)pg);
  range = (double)maxi - mini;

  if ( (maxi <= UCHAR_MAX) && (mini >= UCHAR_MIN) )
    for (i=0; i<npix; i++)
      p2[i] = (UCHAR)(p1[i]);
  else if ( ((double)maxi-mini) <= (double)UCHAR_MAX){
    for (i=0; i<npix; i++)
      p2[i] = (UCHAR)(p1[i] - mini);
  }
  else{
    for (i=0; i<npix; i++)
      p2[i] = (UCHAR)(((double)p1[i] - mini)/range * UCHAR_MAX);
  }

  nbyte=GetImNPix(im);
  if (nbyte%sizeof(long int)) /* pad for word size */
    nbyte+=sizeof(long int);
  p1=(PIX_TYPE *)realloc((void *)GetImPtr(im), nbyte);
  if(p1==NULL)
    return ERROR;
  SetImDataType(im, t_UCHAR);
  SetImPtr(im, p1);
  SetImNByte(im, nbyte);

  return(NO_ERROR);
}
#include "i64_undef.h"

#include "f_def.h"
ERROR_TYPE f_to_uchar(IMAGE *im)
{
  mia_size_t i, npix;
  unsigned long int nbyte;
  UCHAR *p2;
  PIX_TYPE *p1, mini, maxi;
  double range;
  G_TYPE *pg;

  p1    = (PIX_TYPE *)GetImPtr(im);
  p2    = (UCHAR *)GetImPtr(im);
  npix  = GetImNPix(im);
  pg    = min_max(im);
  if (pg == NULL)
    return(ERROR);
  mini  = pg[0].f_val;
  maxi  = pg[1].f_val;
  free((char *)pg);
  range = (double)maxi - mini;

  if ( (maxi <= UCHAR_MAX) && (mini >= UCHAR_MIN) )
    for (i=0; i<npix; i++, p1++, p2++)
      *p2 = (UCHAR)(*p1);
  else if ( ((double)maxi-mini) <= (double)UCHAR_MAX){
    for (i=0; i<npix; i++, p1++, p2++)
      *p2 = (UCHAR)(*p1 - mini);
  }
  else{
    for (i=0; i<npix; i++, p1++, p2++)
      *p2 = (UCHAR)(((double)*p1 - mini)/range * UCHAR_MAX);
  }

  nbyte=GetImNPix(im);
  if (nbyte%sizeof(long int)) /* pad for word size */
    nbyte+=sizeof(long int);
  p1=(PIX_TYPE *)realloc((void *)GetImPtr(im), nbyte);
  if(p1==NULL)
    return ERROR;
  SetImDataType(im, t_UCHAR);
  SetImPtr(im, p1);
  SetImNByte(im, nbyte);

  return(NO_ERROR);
}
#include "f_undef.h"


#include "d_def.h"
ERROR_TYPE d_to_uchar(IMAGE *im)
{
  mia_size_t i, npix;
  unsigned long int nbyte;
  UCHAR *p2;
  PIX_TYPE *p1, mini, maxi;
  double range;
  G_TYPE *pg;

  p1    = (PIX_TYPE *)GetImPtr(im);
  p2    = (UCHAR *)GetImPtr(im);
  npix  = GetImNPix(im);
  pg    = min_max(im);
  if (pg == NULL)
    return(ERROR);
  mini  = pg[0].d_val;
  maxi  = pg[1].d_val;
  free((char *)pg);
  range = (double)maxi - mini;

  if ( (maxi <= UCHAR_MAX) && (mini >= UCHAR_MIN) )
    for (i=0; i<npix; i++, p1++, p2++)
      *p2 = (UCHAR)(*p1);
  else if ( ((double)maxi-mini) <= (double)UCHAR_MAX){
    for (i=0; i<npix; i++, p1++, p2++)
      *p2 = (UCHAR)(*p1 - mini);
  }
  else{
    for (i=0; i<npix; i++, p1++, p2++)
      *p2 = (UCHAR)(((double)*p1 - mini)/range * UCHAR_MAX);
  }

  nbyte=GetImNPix(im);
  if (nbyte%sizeof(long int)) /* pad for word size */
    nbyte+=sizeof(long int);
  p1=(PIX_TYPE *)realloc((void *)GetImPtr(im), nbyte);
  if(p1==NULL)
    return ERROR;
  SetImDataType(im, t_UCHAR);
  SetImPtr(im, p1);
  SetImNByte(im, nbyte);

  return(NO_ERROR);
}
#include "d_undef.h"

ERROR_TYPE to_uchar(IMAGE *im)
{
  switch (GetImDataType(im)){

  case t_UCHAR:
    (void)sprintf(buf, "message in ERROR_TYPE to_uchar(im): \
                  im is already of type UCHAR\n"); errputstr(buf);
    return(NO_ERROR);
    break;

  case t_USHORT:
    return(us_to_uchar(im));
    break;

   case t_INT32:
    return(i32_to_uchar(im));
    break;

   case t_UINT32:
    return(u32_to_uchar(im));
    break;

   case t_INT64:
    return(i64_to_uchar(im));
    break;

   case t_FLOAT:
    return(f_to_uchar(im));
    break;

   case t_DOUBLE:
    return(d_to_uchar(im));
    break;

  default:
    (void)sprintf(buf,"to_uchar(im): invalid pixel type (must be unsigned)\n"); errputstr(buf);
    return(ERROR);
  }
}


/*************************************************************************/
/*                                                                       */

#include "uc_def.h"
ERROR_TYPE uc_to_ushort(IMAGE *im, IMAGE *imout)
{
  mia_size_t i, npix;
  USHORT *p2;
  PIX_TYPE *p1;

  p1    = (PIX_TYPE *)GetImPtr(im);
  p2    = (USHORT *)GetImPtr(imout);
  npix  = GetImNPix(im);

  for (i=0; i<npix; i++)
    *p2++ = (USHORT)(*p1++);
  return(NO_ERROR);
}
#include "uc_undef.h"

#include "i32_def.h"
ERROR_TYPE i32_to_ushort(IMAGE *im, IMAGE *imout)
{
  mia_size_t i, npix;
  USHORT *p2;
  PIX_TYPE *p1, mini, maxi;
  double range;
  G_TYPE *pg;

  p1    = (PIX_TYPE *)GetImPtr(im);
  p2    = (USHORT *)GetImPtr(imout);
  npix  = GetImNPix(im);
  pg    = min_max(im);
  if (pg == NULL)
    return(ERROR);
  mini  = pg[0].i32_val;
  maxi  = pg[1].i32_val;
  range = (double)maxi - mini;

  if ( (maxi <= USHORT_MAX) && (mini >= USHORT_MIN) )
    for (i=0; i<npix; i++, p1++, p2++)
      *p2 = (USHORT)(*p1);
  else if (((double)maxi-mini) < (double)USHORT_MAX + 1){
    for (i=0; i<npix; i++, p1++, p2++)
      *p2 = (USHORT)(*p1 - mini);
  }
  else{
    for (i=0; i<npix; i++, p1++, p2++)
      *p2 = (USHORT)(((double)*p1 - mini)/range * USHORT_MAX);
  }
  free((char *)pg);
  return(NO_ERROR);
}
#include "i32_undef.h"

#include "f_def.h"
ERROR_TYPE f_to_ushort(IMAGE *im, IMAGE *imout)
{
  mia_size_t i, npix;
  USHORT *p2;
  PIX_TYPE *p1, mini, maxi;
  double range;
  G_TYPE *pg;

  p1    = (PIX_TYPE *)GetImPtr(im);
  p2    = (USHORT *)GetImPtr(imout);
  npix  = GetImNPix(im);
  pg    = min_max(im);
  if (pg == NULL)
    return(ERROR);
  mini  = pg[0].f_val;
  maxi  = pg[1].f_val;
  range = (double)maxi - mini;

  if (((double)maxi-mini) < (double)USHORT_MAX + 1){
    if (maxi <= USHORT_MAX)
      for (i=0; i<npix; i++, p1++, p2++)
	*p2 = (USHORT)(*p1);
    else
      for (i=0; i<npix; i++, p1++, p2++)
	*p2 = (USHORT)(*p1 - mini);
  }
  else{
    for (i=0; i<npix; i++, p1++, p2++)
      *p2 = (USHORT)(((double)*p1 - mini)/range * USHORT_MAX);
  }
  free((char *)pg);
  return(NO_ERROR);
}
#include "f_undef.h"

#include "d_def.h"
ERROR_TYPE d_to_ushort(IMAGE *im, IMAGE *imout)
{
  mia_size_t i, npix;
  USHORT *p2;
  PIX_TYPE *p1, mini, maxi;
  double range;
  G_TYPE *pg;

  p1    = (PIX_TYPE *)GetImPtr(im);
  p2    = (USHORT *)GetImPtr(imout);
  npix  = GetImNPix(im);
  pg    = min_max(im);
  if (pg == NULL)
    return(ERROR);
  mini  = pg[0].f_val;
  maxi  = pg[1].f_val;
  range = (double)maxi - mini;

  if (((double)maxi-mini) < (double)USHORT_MAX + 1){
    if (maxi <= USHORT_MAX)
      for (i=0; i<npix; i++, p1++, p2++)
	*p2 = (USHORT)(*p1);
    else
      for (i=0; i<npix; i++, p1++, p2++)
	*p2 = (USHORT)(*p1 - mini);
  }
  else{
    for (i=0; i<npix; i++, p1++, p2++)
      *p2 = (USHORT)(((double)*p1 - mini)/range * USHORT_MAX);
  }
  free((char *)pg);
  return(NO_ERROR);
}
#include "d_undef.h"

IMAGE *to_ushort(IMAGE *im)
{
  ERROR_TYPE rval;
  IMAGE *imout;

  /* create output image */
  imout = create_image(t_USHORT, GetImNx(im), GetImNy(im), GetImNz(im));
  if (imout == NULL){
    (void)sprintf(buf,"to_ushort(im): not enough memory!\n"); errputstr(buf);
    return(NULL);
  }

  switch (GetImDataType(im)){

  case t_UCHAR:
    rval = uc_to_ushort(im, imout);
    break;

#ifndef NO_us_IMAGE
  case t_USHORT:
    (void)sprintf(buf, "ERROR in to_ushort(im): \
                  im is already of type USHORT\n"); errputstr(buf);
    return(NULL);
    break;
#endif

#ifndef NO_s_IMAGE
  case t_SHORT:
    rval = s_to_ushort(im, imout);
    break;
#endif

#ifndef NO_u32_IMAGE
  case t_UINT32:
    rval = u32_to_ushort(im, imout);
    break;
#endif

  case t_INT32:
    rval = i32_to_ushort(im, imout);
    break;

  case t_FLOAT:
    rval = f_to_ushort(im, imout);
    break;

  case t_DOUBLE:
    rval = d_to_ushort(im, imout);
    break;

  default:
    (void)sprintf(buf,"to_ushort(im): invalid pixel type\n"); errputstr(buf);
    return(NULL);
  }
  if (rval == ERROR){
    free((char *)imout);
    return(NULL);
  }
  return(imout);
}



/*************************************************************************/
/*                                                                       */

#ifndef NO_generic_IMAGE
#include "g_def.h"
ERROR_TYPE generic_to_int32(IMAGE *im, IMAGE *imout)
{
  mia_size_t i, npix;
  INT32 *p2;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  p2   = (INT32 *)GetImPtr(imout);
  npix = GetImNPix(im);

  for (i=0; i<npix; i++, p1++, p2++)
    *p2 = (INT32)*p1;
  return(NO_ERROR);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */


#include "s_def.h"
ERROR_TYPE s_to_int32(IMAGE *im, IMAGE *imout)
{
  mia_size_t i, npix;
  INT32 *p2;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  p2   = (INT32 *)GetImPtr(imout);
  npix = GetImNPix(im);

  for (i=0; i<npix; i++, p1++, p2++)
    *p2 = (INT32)*p1;
  return(NO_ERROR);
}
#include "s_undef.h"

#include "us_def.h"
ERROR_TYPE us_to_int32(IMAGE *im, IMAGE *imout)
{
  mia_size_t i, npix;
  INT32 *p2;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  p2   = (INT32 *)GetImPtr(imout);
  npix = GetImNPix(im);

  for (i=0; i<npix; i++, p1++, p2++)
    *p2 = (INT32)*p1;
  return(NO_ERROR);
}
#include "us_undef.h"

#include "f_def.h"
ERROR_TYPE f_to_int32(IMAGE *im, IMAGE *imout)
{
  mia_size_t i, npix;
  INT32 *p2;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  p2   = (INT32 *)GetImPtr(imout);
  npix = GetImNPix(im);

  /* bug: no range check */
  for (i=0; i<npix; i++, p1++, p2++)
    *p2 = (INT32)*p1;
  return(NO_ERROR);
}
#include "f_undef.h"

#include "d_def.h"
ERROR_TYPE d_to_int32(IMAGE *im, IMAGE *imout)
{
  mia_size_t i, npix;
  INT32 *p2;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  p2   = (INT32 *)GetImPtr(imout);
  npix = GetImNPix(im);

  /* bug: no range check */
  for (i=0; i<npix; i++, p1++, p2++)
    *p2 = (INT32)*p1;
  return(NO_ERROR);
}
#include "d_undef.h"


IMAGE *to_int32(IMAGE *im)
{
  IMAGE *imout;
  ERROR_TYPE rval;

  /* create output image */
  imout = create_image(t_INT32, GetImNx(im), GetImNy(im), GetImNz(im));
  if (imout == NULL){
    (void)sprintf(buf,"to_int32(im): not enough memory!\n"); errputstr(buf);
    return(NULL);
  }

  switch (GetImDataType(im)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    rval = generic_to_int32(im, imout);
    break;
#endif

#ifndef NO_uc_IMAGE
  case t_UCHAR:
    rval = uc_to_int32(im, imout);
    break;
#endif

  case t_SHORT:
    rval = s_to_int32(im, imout);
    break;
  case t_USHORT:
    rval = us_to_int32(im, imout);
    break;

#ifndef NO_u32_IMAGE
  case t_UINT32:
    rval = u32_to_int32(im, imout);
    break;
#endif

#ifndef NO_i32_IMAGE
  case t_INT32:
    (void)sprintf(buf, "ERROR in to_int32(im): \
                  im is already of type INT32\n"); errputstr(buf);
    return(NULL);
    break;
#endif

  case t_FLOAT:
    rval = f_to_int32(im, imout);
    break;

  case t_DOUBLE:
    rval = d_to_int32(im, imout);
    break;


  default:
    (void)sprintf(buf,"to_int32(im): invalid pixel type\n"); errputstr(buf);
    return(NULL);
  }
  if (rval == ERROR){
    free((char *)imout);
    return(NULL);
  }
  return(imout);
}





/*************************************************************************/
/*                                                                       */

#ifndef NO_generic_IMAGE
#include "g_def.h"
ERROR_TYPE generic_to_float(IMAGE *im, IMAGE *imout)
{
  mia_size_t i, npix;
  MIALFLOAT *p2;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  p2   = (MIALFLOAT *)GetImPtr(imout);
  npix = GetImNPix(im);

  /* potential bug (depending on data type): no range check */
  for (i=0; i<npix; i++, p1++, p2++)
    *p2 = (MIALFLOAT)*p1;
  return(NO_ERROR);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */



#include "s_def.h"
ERROR_TYPE s_to_float(IMAGE *im, IMAGE *imout)
{
  mia_size_t i, npix;
  MIALFLOAT *p2;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  p2   = (MIALFLOAT *)GetImPtr(imout);
  npix = GetImNPix(im);

  /* bug: no range check */
  for (i=0; i<npix; i++, p1++, p2++)
    *p2 = (MIALFLOAT)*p1;
  return(NO_ERROR);
}
#include "s_undef.h"

#include "us_def.h"
ERROR_TYPE us_to_float(IMAGE *im, IMAGE *imout)
{
  mia_size_t i, npix;
  MIALFLOAT *p2;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  p2   = (MIALFLOAT *)GetImPtr(imout);
  npix = GetImNPix(im);

  /* bug: no range check */
  for (i=0; i<npix; i++, p1++, p2++)
    *p2 = (MIALFLOAT)*p1;
  return(NO_ERROR);
}
#include "us_undef.h"


#include "i32_def.h"
ERROR_TYPE i32_to_float(IMAGE *im, IMAGE *imout)
{
  mia_size_t i, npix;
  MIALFLOAT *p2;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  p2   = (MIALFLOAT *)GetImPtr(imout);
  npix = GetImNPix(im);

  /* bug: no range check */
  for (i=0; i<npix; i++, p1++, p2++)
    *p2 = (MIALFLOAT)*p1;
  return(NO_ERROR);
}
#include "i32_undef.h"


#include "u32_def.h"
ERROR_TYPE u32_to_float(IMAGE *im, IMAGE *imout)
{
  mia_size_t i, npix;
  MIALFLOAT *p2;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  p2   = (MIALFLOAT *)GetImPtr(imout);
  npix = GetImNPix(im);

  /* bug: no range check */
  for (i=0; i<npix; i++, p1++, p2++)
    *p2 = (MIALFLOAT)*p1;
  return(NO_ERROR);
}
#include "u32_undef.h"


#include "d_def.h"
ERROR_TYPE d_to_float(IMAGE *im, IMAGE *imout)
{
  mia_size_t i, npix;
  MIALFLOAT *p2;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  p2   = (MIALFLOAT *)GetImPtr(imout);
  npix = GetImNPix(im);

  /* bug: no range check */
  for (i=0; i<npix; i++, p1++, p2++)
    *p2 = (MIALFLOAT)*p1;
  return(NO_ERROR);
}
#include "d_undef.h"


IMAGE *to_float(IMAGE *im)
{
  IMAGE *imout;
  ERROR_TYPE rval;

  /* create output image */
  imout = create_image(t_FLOAT, GetImNx(im), GetImNy(im), GetImNz(im));
  if (imout == NULL){
    (void)sprintf(buf,"to_float(im): not enough memory!\n"); errputstr(buf);
    return(NULL);
  }

  switch (GetImDataType(im)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    rval = generic_to_float(im, imout);
    break;
#endif

  case t_SHORT:
    rval = s_to_float(im, imout);
    break;

  case t_USHORT:
    rval = us_to_float(im, imout);
    break;

  case t_INT32:
    rval = i32_to_float(im, imout);
    break;

  case t_UINT32:
    rval = u32_to_float(im, imout);
    break;

  case t_DOUBLE:
    rval = d_to_float(im, imout);
    break;

  default:
    (void)sprintf(buf,"to_float(im): invalid pixel type\n"); errputstr(buf);
    return(NULL);
  }
  if (rval == ERROR){
    free((char *)imout);
    return(NULL);
  }
  return(imout);
}

ERROR_TYPE dbltofloat(IMAGE *im)
{
  double *pim =(double *)GetImPtr(im);
  float  *pout=(float *)GetImPtr(im);
  unsigned long int i, npix=GetImNPix(im), nbyte;

  if (GetImDataType(im)!=t_DOUBLE){
    (void)sprintf(buf,"dbltofloat(im): im must be of type double\n"); errputstr(buf);
  }

  for (i=0; i<npix; i++, pim++, pout++)
      *pout = (float)(*pim);

  nbyte=GetImNPix(im)*sizeof(float);
  if (nbyte%sizeof(long int)) /* pad for word size */
    nbyte+=sizeof(long int);
  pout=(float *)realloc((void *)GetImPtr(im), nbyte);
  if(pout==NULL)
    return ERROR;
  SetImDataType(im, t_FLOAT);
  SetImPtr(im, pout);
  SetImNByte(im, nbyte);

  return NO_ERROR;
}

#include "uc_def.h"
ERROR_TYPE uc_to_double(IMAGE *im, IMAGE *imout)
{
  mia_size_t i, npix;
  double *p2;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  p2   = (double *)GetImPtr(imout);
  npix = GetImNPix(im);

  /* potential bug (depending on data type): no range check */
  for (i=0; i<npix; i++, p1++, p2++)
    *p2 = (double)*p1;
  return(NO_ERROR);
}
#include "uc_undef.h"


#include "us_def.h"
ERROR_TYPE us_to_double(IMAGE *im, IMAGE *imout)
{
  mia_size_t i, npix;
  double *p2;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  p2   = (double *)GetImPtr(imout);
  npix = GetImNPix(im);

  /* potential bug (depending on data type): no range check */
  for (i=0; i<npix; i++, p1++, p2++)
    *p2 = (double)*p1;
  return(NO_ERROR);
}
#include "us_undef.h"

#include "u32_def.h"
ERROR_TYPE u32_to_double(IMAGE *im, IMAGE *imout)
{
  mia_size_t i, npix;
  double *p2;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  p2   = (double *)GetImPtr(imout);
  npix = GetImNPix(im);

  /* potential bug (depending on data type): no range check */
  for (i=0; i<npix; i++, p1++, p2++)
    *p2 = (double)*p1;
  return(NO_ERROR);
}
#include "u32_undef.h"

#include "i32_def.h"
ERROR_TYPE i32_to_double(IMAGE *im, IMAGE *imout)
{
  mia_size_t i, npix;
  double *p2;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  p2   = (double *)GetImPtr(imout);
  npix = GetImNPix(im);

  /* potential bug (depending on data type): no range check */
  for (i=0; i<npix; i++, p1++, p2++)
    *p2 = (double)*p1;
  return(NO_ERROR);
}
#include "i32_undef.h"

#include "f_def.h"
ERROR_TYPE f_to_double(IMAGE *im, IMAGE *imout)
{
  mia_size_t i, npix;
  double *p2;
  PIX_TYPE *p1;

  p1   = (PIX_TYPE *)GetImPtr(im);
  p2   = (double *)GetImPtr(imout);
  npix = GetImNPix(im);

  /* potential bug (depending on data type): no range check */
  for (i=0; i<npix; i++, p1++, p2++)
    *p2 = (double)*p1;
  return(NO_ERROR);
}
#include "f_undef.h"

IMAGE *to_double(IMAGE *im)
{
  IMAGE *imout;
  ERROR_TYPE rval;

  /* create output image */
  imout = create_image(t_DOUBLE, GetImNx(im), GetImNy(im), GetImNz(im));
  if (imout == NULL){
    (void)sprintf(buf,"to_double(im): not enough memory!\n"); errputstr(buf);
    return(NULL);
  }

  switch (GetImDataType(im)){

  case t_UCHAR:
    rval = uc_to_double(im, imout);
    break;
  case t_USHORT:
    rval = us_to_double(im, imout);
    break;
  case t_UINT32:
    rval = u32_to_double(im, imout);
    break;
  case t_INT32:
    rval = i32_to_double(im, imout);
    break;
  case t_FLOAT:
    rval = f_to_double(im, imout);
    break;
  default:
    (void)sprintf(buf,"to_double(im): invalid pixel type\n"); errputstr(buf);
    return(NULL);
  }
  if (rval == ERROR){
    free((char *)imout);
    return(NULL);
  }
  return(imout);
}


ERROR_TYPE uint32_to_float(IMAGE *im)
{
  UINT32 *puint32;
  float *pfloat;
  long int i, npix=GetImNPix(im);

  if (GetImDataType(im)!= t_UINT32)
    return ERROR;

  puint32=(UINT32 *)GetImPtr(im);
  pfloat=(float *)GetImPtr(im);

#pragma omp parallel for
  for (i=0; i<npix; i++)
    pfloat[i]=(float)puint32[i];

  im->DataType=t_FLOAT;

  return NO_ERROR;
}

/*************************************************************************/
/*            swap bytes (for LITTLE<->BIG ENDIAN conversions)           */
/*************************************************************************/

/*
**  Function to swap long types.
*/
void swap_long(INT32 *pl)
{
  union mylong
  {
    INT32 lo;
    struct
    {
      char c1, c2, c3, c4;
    } ch;
  } ll;

  char ch_tmp;

  ll.lo = *pl;

  ch_tmp    =  ll.ch.c1;
  ll.ch.c1  =  ll.ch.c4;
  ll.ch.c4  =   ch_tmp;
  ch_tmp    =   ll.ch.c2;
  ll.ch.c2  =   ll.ch.c3;
  ll.ch.c3  =   ch_tmp;

  *pl = ll.lo;
}

/*
**  Function to swap short types.
*/
void swap_short(SHORT *pl)
{
  union myshort
  {
    short int lo;
    struct
    {
      char c1, c2;
    } ch;
  } ll;

  char ch_tmp;

  ll.lo = *pl;

  ch_tmp    =  ll.ch.c1;
  ll.ch.c1  =  ll.ch.c2;
  ll.ch.c2  =   ch_tmp;

  *pl = ll.lo;
}

ERROR_TYPE swap(IMAGE *im)
{
  unsigned long int i, nbyte;

  nbyte = GetImNPix(im);

  /* here we go */
  switch (GetImDataType(im)){

  case t_USHORT:
  case t_SHORT:
    {
    SHORT *pim;
    pim = (SHORT *)GetImPtr(im);
    for (i=0; i<nbyte; i++, pim++)
      swap_short(pim);
    }
    break;
  case t_UINT32:
  case t_INT32:
    {
    INT32 *pim;
    pim = (INT32 *)GetImPtr(im);
    for (i=0; i<nbyte; i++, pim++)
      swap_long(pim);
    }
    break;

  default:
    (void)sprintf(buf, "ERROR in swap(im): \
                invalid ImDataType\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}

/*@}*/
