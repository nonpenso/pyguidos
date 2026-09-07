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

/* this file is not cleaned ...  old stuff ... */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "miallib.h"
#include "fifo.h"




/** \addtogroup group_label
 *  @{
 */

extern int cluster();
extern ERROR_TYPE agglo_cluster(int *x, int *y, int *pn, double maxdst);
extern ERROR_TYPE nearest_cluster(IMAGE *im, int *x, int *y, int *pn, double maxdst);
extern ERROR_TYPE knearest_cluster(IMAGE *im, int *x, int *y, int *pn, int k, double maxdst);

#define set_mean_type(TYPE1, TYPE2, i1, i2) \
{\
  TYPE1 *p1 = (TYPE1 *) GetImPtr(i1);  \
  TYPE2 *p2 = (TYPE2 *) GetImPtr(i2);  \
  LOOPDN(i, GetImNPix(i1)){  \
    psum[*p2] += *p1;  \
    p1++;  \
    p2++;  \
  }\
  for (i = 0; i < maxlbl; ++i){\
    if (pnbr[i])  \
      psum[i] /= pnbr[i];  \
  }   \
  psum[0] = 0;  \
  p2 = (TYPE2 *)GetImPtr(i2);  \
  LOOPDN(i, GetImNPix(i1)){  \
    *p2 = (TYPE2) psum[*p2];  \
    p2++;\
  }  \
}

#define set_max_type(TYPE1, TYPE2, i1, i2) \
{\
  TYPE1 *p1 = (TYPE1 *) GetImPtr(i1);  \
  TYPE2 *p2 = (TYPE2 *) GetImPtr(i2);  \
  LOOPDN(i, GetImNPix(i1)){  \
    if (psum[*p2]<*p1) \
      psum[*p2]=*p1; \
    p1++;  \
    p2++;  \
  }\
  psum[0] = 0;  \
  p2 = (TYPE2 *)GetImPtr(i2);  \
  LOOPDN(i, GetImNPix(i1)){  \
    *p2 = (TYPE2) psum[*p2];  \
    p2++;\
  }  \
}

#define set_min_type(TYPE1, TYPE2, i1, i2) \
{\
  TYPE1 *p1 = (TYPE1 *) GetImPtr(i1);  \
  TYPE2 *p2 = (TYPE2 *) GetImPtr(i2);  \
  LOOPDN(i, GetImNPix(i1)){  \
    if (psum[*p2]>*p1) \
      psum[*p2]=*p1; \
    p1++;  \
    p2++;  \
  }\
  psum[0] = 0;  \
  p2 = (TYPE2 *)GetImPtr(i2);  \
  LOOPDN(i, GetImNPix(i1)){  \
    *p2 = (TYPE2) psum[*p2];  \
    p2++;\
  }  \
}

#define set_range_type(TYPE1, TYPE2, i1, i2) \
{\
  TYPE1 *p1 = (TYPE1 *) GetImPtr(i1);  \
  TYPE2 *p2 = (TYPE2 *) GetImPtr(i2);  \
  LOOPDN(i, GetImNPix(i1)){  \
    if (pmin[*p2]>*p1) \
      pmin[*p2]=*p1; \
    if (pmax[*p2]<*p1) \
      pmax[*p2]=*p1; \
    p1++;  \
    p2++;  \
  }\
  p2 = (TYPE2 *)GetImPtr(i2);  \
  LOOPDN(i, GetImNPix(i1)){  \
    *p2 =(TYPE2) (pmax[*p2]-pmin[*p2]);		\
    p2++;\
  }  \
}


#define set_sum_type(TYPE1, TYPE2, i1, i2) \
{\
  TYPE1 *p1 = (TYPE1 *) GetImPtr(i1);  \
  TYPE2 *p2 = (TYPE2 *) GetImPtr(i2);  \
  LOOPDN(i, GetImNPix(i1)){  \
    psum[*p2]+=*p1; \
    p1++;  \
    p2++;  \
  }\
  p2 = (TYPE2 *)GetImPtr(i2);  \
  LOOPDN(i, GetImNPix(i1)){  \
    *p2 = (TYPE2) psum[*p2];  \
    p2++;\
  }  \
}

#define set_sigma_type(TYPE1, TYPE2, i1, i2) \
{\
  TYPE1 *p1 = (TYPE1 *) GetImPtr(i1);  \
  TYPE2 *p2 = (TYPE2 *) GetImPtr(i2);  \
  LOOPDN(i, GetImNPix(i1)){  \
    psum[*p2] += *p1;  \
    p1++;  \
    p2++;  \
  }\
  for (i = 0; i < maxlbl; ++i){\
    if (pnbr[i])  \
      psum[i] /= pnbr[i];  \
  }   \
  p1 = (TYPE1 *) GetImPtr(i1);  \
  p2 = (TYPE2 *)GetImPtr(i2);  \
  LOOPDN(i, GetImNPix(i1)){  \
    psigma[*p2] += fabs((double)*p1-psum[*p2]);  \
    p1++;  \
    p2++;  \
  }\
  for (i = 0; i < maxlbl; ++i){\
    if (pnbr[i])  \
      psigma[i] /= pnbr[i];  \
  }   \
  p2 = (TYPE2 *)GetImPtr(i2);  \
  LOOPDN(i, GetImNPix(i1)){  \
    *p2 = (TYPE2) psigma[*p2];  \
    p2++;\
  }  \
}

extern IMBLOB *create_blob(LBL_TYPE n);



ERROR_TYPE tessel_sum(IMAGE *ilbl, IMAGE *ival)
{
  IMAGE *imhst;
  int i, maxlbl;
  HST1D_TYPE *psum;

  imhst = histo1d(ilbl);
  if (imhst==NULL)
    return(ERROR);
  maxlbl = GetImNx(imhst);

  psum = (HST1D_TYPE *)calloc(maxlbl, sizeof(HST1D_TYPE));
  if (psum==NULL){
    (void)sprintf(buf,"tessel_sum(): insufficient memory\n"); errputstr(buf);
    free_image(imhst);
    return(ERROR);
  }

  switch (GetImDataType(ilbl))
    {
    case t_USHORT:
      switch (GetImDataType(ival))
	{
	case t_UCHAR:
	  set_sum_type(UCHAR,  USHORT, ival, ilbl);
	  break;
	case t_USHORT:
	  set_sum_type(USHORT, USHORT, ival, ilbl);
	  break;
	case t_INT32:
	  set_sum_type(INT32,  USHORT , ival, ilbl);
	  break;
	case t_FLOAT:
	  set_sum_type(MIALFLOAT,  USHORT, ival, ilbl);
	  break;
	case t_DOUBLE:
	  set_sum_type(DOUBLE, USHORT, ival, ilbl);
	  break;
	default:
	  free_image(imhst);
	  free((char *)psum);
	  return(ERROR);
	}
      break;
    case t_INT32:
      switch (GetImDataType(ival))
	{
	case t_UCHAR:
	  set_sum_type(UCHAR,  INT32, ival, ilbl);
	  break;
	case t_USHORT:
	  set_sum_type(USHORT, INT32, ival, ilbl);
	  break;
	case t_INT32:
	  set_sum_type(INT32,  INT32, ival, ilbl);
	  break;
	case t_FLOAT:
	  set_sum_type(MIALFLOAT,  INT32, ival, ilbl);
	  break;
	case t_DOUBLE:
	  set_sum_type(DOUBLE, INT32, ival, ilbl);
	  break;
	default:
	  free_image(imhst);
	  free((char *)psum);
	  return(ERROR);
	}
      break;
    case t_UINT32:
      switch (GetImDataType(ival))
	{
	case t_UCHAR:
	  set_sum_type(UCHAR,  UINT32, ival, ilbl);
	  break;
	case t_USHORT:
	  set_sum_type(USHORT, UINT32, ival, ilbl);
	  break;
	case t_INT32:
	  set_sum_type(INT32,  UINT32, ival, ilbl);
	  break;
	case t_FLOAT:
	  set_sum_type(MIALFLOAT,  UINT32, ival, ilbl);
	  break;
	case t_DOUBLE:
	  set_sum_type(DOUBLE, UINT32, ival, ilbl);
	  break;
	default:
	  free_image(imhst);
	  free((char *)psum);
	  return(ERROR);
	}
      break;
    default:
      free_image(imhst);
      free((char *)psum);
      return(ERROR);
    }

  free_image(imhst);
  free((char *)psum);
  return(NO_ERROR);
}


ERROR_TYPE tessel_range(IMAGE *ilbl, IMAGE *ival)
{
  IMAGE *imhst;
  mia_size_t  i, maxlbl;
  DOUBLE *pmin, *pmax;

  imhst = histo1d(ilbl);
  if (imhst==NULL)
    return(ERROR);
  maxlbl = GetImNx(imhst);

  pmin = (DOUBLE *)calloc(maxlbl+1, sizeof(DOUBLE));
  if (pmin==NULL){
    (void)sprintf(buf,"tessel_range(): insufficient memory\n"); errputstr(buf);
    free_image(imhst);
    return(ERROR);
  }
  pmax = (DOUBLE *)calloc(maxlbl+1, sizeof(DOUBLE));
  if (pmax==NULL){
    (void)sprintf(buf,"tessel_range(): insufficient memory\n"); errputstr(buf);
    free((char *)pmin);
    free_image(imhst);
    return(ERROR);
  }
  for(i=0;i<=maxlbl;i++){
    pmin[i]=DOUBLE_MAX;
    pmax[i]=DOUBLE_MIN;
  }

  switch (GetImDataType(ilbl))
    {
    case t_USHORT:
      switch (GetImDataType(ival))
	{
	case t_UCHAR:
	  set_range_type(UCHAR,  USHORT, ival, ilbl);
	  break;
	case t_USHORT:
	  set_range_type(USHORT, USHORT, ival, ilbl);
	  break;
	case t_INT32:
	  set_range_type(INT32,  USHORT , ival, ilbl);
	  break;
	case t_FLOAT:
	  set_range_type(MIALFLOAT,  USHORT, ival, ilbl);
	  break;
	case t_DOUBLE:
	  set_range_type(DOUBLE, USHORT, ival, ilbl);
	  break;
	default:
	  free_image(imhst);
	  free((char *)pmin);
	  free((char *)pmax);
	  (void)sprintf(buf,"tessel_range(): invalid ival data type\n");
	  errputstr(buf);
	  return(ERROR);
	}
      break;
    case t_INT32:
      switch (GetImDataType(ival))
	{
	case t_UCHAR:
	  set_range_type(UCHAR,  INT32, ival, ilbl);
	  break;
	case t_USHORT:
	  set_range_type(USHORT, INT32, ival, ilbl);
	  break;
	case t_INT32:
	  set_range_type(INT32,  INT32, ival, ilbl);
	  break;
	case t_FLOAT:
	  set_range_type(MIALFLOAT,  INT32, ival, ilbl);
	  break;
	case t_DOUBLE:
	  set_range_type(DOUBLE, INT32, ival, ilbl);
	  break;
	default:
	  free_image(imhst);
	  free((char *)pmin);
	  free((char *)pmax);
	  (void)sprintf(buf,"tessel_range(): invalid ival data type\n");
	  errputstr(buf);
	  return(ERROR);
	}
      break;
    default:
      free_image(imhst);
      free((char *)pmin);
      free((char *)pmax);
      (void)sprintf(buf,"tessel_range(): invalid ilbll data type\n");
      errputstr(buf);
      return(ERROR);
    }

  free_image(imhst);
  free((char *)pmin);
  free((char *)pmax);
  return(NO_ERROR);
}

ERROR_TYPE tessel_mean(IMAGE *ilbl, IMAGE *ival)
{
  IMAGE *imhst;
  int i, maxlbl;
  HST1D_TYPE *pnbr, *psum;

  imhst = histo1d(ilbl);
  if (imhst==NULL)
    return(ERROR);
  maxlbl = GetImNx(imhst);


  pnbr = (HST1D_TYPE *)GetImPtr(imhst);
  psum = (HST1D_TYPE *)calloc(maxlbl, sizeof(HST1D_TYPE));
  if (psum==NULL){
    (void)sprintf(buf,"tessel_mean(): insufficient memory\n"); errputstr(buf);
    free_image(imhst);
    return(ERROR);
  }

  switch (GetImDataType(ival)){
  case t_UCHAR:
    switch (GetImDataType(ilbl)){
    case t_UCHAR:
      set_mean_type(UCHAR, UCHAR, ival, ilbl);
      break;
    case t_USHORT:
      set_mean_type(UCHAR, USHORT, ival, ilbl);
      break;
    case t_INT32:
      set_mean_type(UCHAR, INT32, ival, ilbl);
      break;
    case t_UINT32:
      set_mean_type(UCHAR, UINT32, ival, ilbl);
      break;
    default:
      free_image(imhst);
      free((char *)psum);
      (void)sprintf(buf,"tessel_mean(): data type of ilbl not allowed\n"); errputstr(buf);
      return(ERROR);
    }
    break;
  case t_USHORT:
    switch (GetImDataType(ilbl)){
    case t_UCHAR:
      set_mean_type(USHORT, UCHAR, ival, ilbl);
      break;
    case t_USHORT:
      set_mean_type(USHORT, USHORT, ival, ilbl);
      break;
    case t_INT32:
      set_mean_type(USHORT, INT32, ival, ilbl);
      break;
    case t_UINT32:
      set_mean_type(USHORT, UINT32, ival, ilbl);
      break;
    default:
      free_image(imhst);
      free((char *)psum);
      (void)sprintf(buf,"tessel_mean(): data type of ilbl not allowed\n"); errputstr(buf);
      return(ERROR);
    }
    break;
  case t_INT32:
    switch (GetImDataType(ilbl)){
    case t_UCHAR:
      set_mean_type(INT32, UCHAR, ival, ilbl);
      break;
    case t_USHORT:
      set_mean_type(INT32, USHORT, ival, ilbl);
      break;
    case t_INT32:
      set_mean_type(INT32, INT32, ival, ilbl);
      break;
    case t_UINT32:
      set_mean_type(INT32, UINT32, ival, ilbl);
      break;
    default:
      free_image(imhst);
      free((char *)psum);
      (void)sprintf(buf,"tessel_mean(): data type of ilbl not allowed\n"); errputstr(buf);
      return(ERROR);
    }
    break;
  case t_UINT32:
    switch (GetImDataType(ilbl)){
    case t_UCHAR:
      set_mean_type(UINT32, UCHAR, ival, ilbl);
      break;
    case t_USHORT:
      set_mean_type(UINT32, USHORT, ival, ilbl);
      break;
    case t_INT32:
      set_mean_type(UINT32, INT32, ival, ilbl);
      break;
    case t_UINT32:
      set_mean_type(UINT32, UINT32, ival, ilbl);
      break;
    default:
      free_image(imhst);
      free((char *)psum);
      (void)sprintf(buf,"tessel_mean(): data type of ilbl not allowed\n"); errputstr(buf);
      return(ERROR);
    }
    break;
  default:
    free_image(imhst);
    free((char *)psum);
    (void)sprintf(buf,"tessel_mean(): data type of ival not allowed\n"); errputstr(buf);
    return(ERROR);
  }

  free_image(imhst);
  free((char *)psum);
  return(NO_ERROR);
}


ERROR_TYPE tessel_min(IMAGE *ilbl, IMAGE *ival)
{
  IMAGE *imhst;
  int i, maxlbl;
  HST1D_TYPE *psum;

  imhst = histo1d(ilbl);
  if (imhst==NULL)
    return(ERROR);
  maxlbl = GetImNx(imhst);

  psum = (HST1D_TYPE *)calloc(maxlbl, sizeof(HST1D_TYPE));
  for (i=0; i<maxlbl; i++)
    psum[i]=INT32_MAX;
  if (psum==NULL){
    (void)sprintf(buf,"tessel_min(): insufficient memory\n"); errputstr(buf);
    free_image(imhst);
    return(ERROR);
  }

  switch (GetImDataType(ival)){
  case t_UCHAR:
    switch (GetImDataType(ilbl)){
    case t_UCHAR:
      set_min_type(UCHAR, UCHAR, ival, ilbl);
      break;
    case t_USHORT:
      set_min_type(UCHAR, USHORT, ival, ilbl);
      break;
    case t_INT32:
      set_min_type(UCHAR, INT32, ival, ilbl);
      break;
    case t_UINT32:
      set_min_type(UCHAR, UINT32, ival, ilbl);
      break;
    default:
      free_image(imhst);
      free((char *)psum);
      (void)sprintf(buf,"tessel_min(): data type of ilbl not allowed\n"); errputstr(buf);
      return(ERROR);
    }
    break;
  case t_USHORT:
    switch (GetImDataType(ilbl)){
    case t_UCHAR:
      set_min_type(USHORT, UCHAR, ival, ilbl);
      break;
    case t_USHORT:
      set_min_type(USHORT, USHORT, ival, ilbl);
      break;
    case t_INT32:
      set_min_type(USHORT, INT32, ival, ilbl);
      break;
    case t_UINT32:
      set_min_type(USHORT, UINT32, ival, ilbl);
      break;
    default:
      free_image(imhst);
      free((char *)psum);
      (void)sprintf(buf,"tessel_min(): data type of ilbl not allowed\n"); errputstr(buf);
      return(ERROR);
    }
    break;
  case t_INT32:
    switch (GetImDataType(ilbl)){
    case t_UCHAR:
      set_min_type(INT32, UCHAR, ival, ilbl);
      break;
    case t_USHORT:
      set_min_type(INT32, USHORT, ival, ilbl);
      break;
    case t_INT32:
      set_min_type(INT32, INT32, ival, ilbl);
      break;
    case t_UINT32:
      set_min_type(INT32, UINT32, ival, ilbl);
      break;
    default:
      free_image(imhst);
      free((char *)psum);
      (void)sprintf(buf,"tessel_min(): data type of ilbl not allowed\n"); errputstr(buf);
      return(ERROR);
    }
    break;
  case t_UINT32:
    switch (GetImDataType(ilbl)){
    case t_UCHAR:
      set_min_type(UINT32, UCHAR, ival, ilbl);
      break;
    case t_USHORT:
      set_min_type(UINT32, USHORT, ival, ilbl);
      break;
    case t_INT32:
      set_min_type(UINT32, INT32, ival, ilbl);
      break;
    case t_UINT32:
      set_min_type(UINT32, UINT32, ival, ilbl);
      break;
    default:
      free_image(imhst);
      free((char *)psum);
      (void)sprintf(buf,"tessel_min(): data type of ilbl not allowed\n"); errputstr(buf);
      return(ERROR);
    }
    break;
  default:
    free_image(imhst);
    free((char *)psum);
    (void)sprintf(buf,"tessel_min(): data type of ival not allowed\n"); errputstr(buf);
    return(ERROR);
  }
  free_image(imhst);
  free((char *)psum);
  return(NO_ERROR);
}

ERROR_TYPE tessel_max(IMAGE *ilbl, IMAGE *ival)
{
  IMAGE *imhst;
  int i, maxlbl;
  HST1D_TYPE *psum;

  imhst = histo1d(ilbl);
  if (imhst==NULL){
    (void)sprintf(buf,"tessel_min(): insufficient memory\n"); errputstr(buf);
    return(ERROR);
  }
  maxlbl = GetImNx(imhst);

  psum = (HST1D_TYPE *)calloc(maxlbl, sizeof(HST1D_TYPE));
  for (i=0; i<maxlbl; i++)
    psum[i]=INT32_MIN;
  if (psum==NULL){
    (void)sprintf(buf,"tessel_max(): insufficient memory\n"); errputstr(buf);
    free_image(imhst);
    return(ERROR);
  }

  switch (GetImDataType(ival)){
  case t_UCHAR:
    switch (GetImDataType(ilbl)){
    case t_UCHAR:
      set_max_type(UCHAR, UCHAR, ival, ilbl);
      break;
    case t_USHORT:
      set_max_type(UCHAR, USHORT, ival, ilbl);
      break;
    case t_INT32:
      set_max_type(UCHAR, INT32, ival, ilbl);
      break;
    case t_UINT32:
      set_max_type(UCHAR, UINT32, ival, ilbl);
      break;
    default:
      free_image(imhst);
      free((char *)psum);
      (void)sprintf(buf,"tessel_max(): data type of ilbl not allowed\n"); errputstr(buf);
      return(ERROR);
    }
    break;
  case t_USHORT:
    switch (GetImDataType(ilbl)){
    case t_UCHAR:
      set_max_type(USHORT, UCHAR, ival, ilbl);
      break;
    case t_USHORT:
      set_max_type(USHORT, USHORT, ival, ilbl);
      break;
    case t_INT32:
      set_max_type(USHORT, INT32, ival, ilbl);
      break;
    case t_UINT32:
      set_max_type(USHORT, UINT32, ival, ilbl);
      break;
    default:
      free_image(imhst);
      free((char *)psum);
      (void)sprintf(buf,"tessel_max(): data type of ilbl not allowed\n"); errputstr(buf);
      return(ERROR);
    }
    break;
  case t_INT32:
    switch (GetImDataType(ilbl)){
    case t_UCHAR:
      set_max_type(INT32, UCHAR, ival, ilbl);
      break;
    case t_USHORT:
      set_max_type(INT32, USHORT, ival, ilbl);
      break;
    case t_INT32:
      set_max_type(INT32, INT32, ival, ilbl);
      break;
    case t_UINT32:
      set_max_type(INT32, UINT32, ival, ilbl);
      break;
    default:
      free_image(imhst);
      free((char *)psum);
      (void)sprintf(buf,"tessel_max(): data type of ilbl not allowed\n"); errputstr(buf);
      return(ERROR);
    }
    break;
  case t_UINT32:
    switch (GetImDataType(ilbl)){
    case t_UCHAR:
      set_max_type(UINT32, UCHAR, ival, ilbl);
      break;
    case t_USHORT:
      set_max_type(UINT32, USHORT, ival, ilbl);
      break;
    case t_INT32:
      set_max_type(UINT32, INT32, ival, ilbl);
      break;
    case t_UINT32:
      set_max_type(UINT32, UINT32, ival, ilbl);
      break;
    default:
      free_image(imhst);
      free((char *)psum);
      (void)sprintf(buf,"tessel_max(): data type of ilbl not allowed\n"); errputstr(buf);
      return(ERROR);
    }
    break;
  default:
    free_image(imhst);
    free((char *)psum);
    (void)sprintf(buf,"tessel_max(): data type of ival not allowed\n"); errputstr(buf);
    return(ERROR);
  }
  free_image(imhst);
  free((char *)psum);
  return(NO_ERROR);
}

ERROR_TYPE tessel_sigma(IMAGE *ilbl, IMAGE *ival)
{
  IMAGE *imhst;
  int i, maxlbl;
  HST1D_TYPE *pnbr, *psum;
  double *psigma;

  imhst = histo1d(ilbl);
  if (imhst==NULL)
    return(ERROR);
  maxlbl = GetImNx(imhst);


  pnbr = (HST1D_TYPE *)GetImPtr(imhst);
  psum = (HST1D_TYPE *)calloc(maxlbl, sizeof(HST1D_TYPE));
  if (psum==NULL){
    (void)sprintf(buf,"tessel_sigma(): insufficient memory\n"); errputstr(buf);
    free_image(imhst);
    return(ERROR);
  }

  psigma = (double *)calloc(maxlbl, sizeof(double));
  if (psigma==NULL){
    (void)sprintf(buf,"tessel_sigma(): insufficient memory\n"); errputstr(buf);
    free((char *)psum);
    free_image(imhst);
    return(ERROR);
  }

  switch (GetImDataType(ival))
    {
    case t_UCHAR:
      set_sigma_type(UCHAR, LBL_TYPE, ival, ilbl);
      break;
    case t_USHORT:
      set_sigma_type(USHORT, LBL_TYPE, ival, ilbl);
      break;
    case t_INT32:
      set_sigma_type(INT32, LBL_TYPE, ival, ilbl);
      break;
    default:
      free_image(imhst);
      free((char *)psum);
      free((char *)psigma);
      return(ERROR);
    }
  free_image(imhst);
  free((char *)psum);
  free((char *)psigma);
  return(NO_ERROR);
}



#ifndef NO_generic_IMAGE
#include "g_def.h"
ERROR_TYPE uc_tessel_dir(IMAGE *ilbl, IMAGE *ival, int type)
{
  PIX_TYPE *pival, val;
  LBL_TYPE *pilbl, lbl, maxlbl=0;
  IMBLOB *part, *blob;
  long int i, npix;
  int x,y,nx,ny,area,m00;
#ifndef MSPA
  int *px, *py, n;
#endif
  double xcg=0.0,ycg=0.0,phi,mu11,mu20,mu02,c,ratio;
  double r=0.0,slope,cst; /* for the linear regression */

  nx=GetImNx(ilbl);
  ny=GetImNy(ilbl);
  npix=GetImNPix(ilbl);

  pilbl=(LBL_TYPE *)GetImPtr(ilbl);
  pival=(PIX_TYPE *)GetImPtr(ival);

  for(i=0; i<npix; i++) /* get max label value */
    if(maxlbl<pilbl[i])
      maxlbl=pilbl[i];

  maxlbl+=1; /* allocate memory for blob array */
  part=create_blob(maxlbl);
  if (part==NULL){
    (void)sprintf(buf,"uc_tessel_dir(): not enough memory!\n"); errputstr(buf);
    return(ERROR);
  }

  for(y=0; y<ny; y++){ /* zero and 1st order moments */
    for(x=0; x<nx; x++){
      val=pival[x+nx*y];
      lbl=pilbl[x+nx*y];
      blob=&(part[lbl]);
      blob->area +=1;
      blob->m00 += val;
      blob->m10 += (x*val);
      blob->m01 += (y*val);
    }
  }

  for(i=1; i<maxlbl; i++){ /* coordinates of gravity center */
    if (part[i].m00 != 0){
      part[i].xcg = part[i].m10/(double)part[i].m00;
      part[i].ycg = part[i].m01/(double)part[i].m00;
    }
  }

  for(y=0; y<ny; y++){ /* centred moments */
    for(x=0; x<nx; x++){
      val=pival[x+nx*y];
      lbl=pilbl[x+nx*y];
      blob=&(part[lbl]);
      xcg=blob->xcg;
      ycg=blob->ycg;
      blob->mu11 += (x-xcg)*(y-ycg)*val;
      blob->mu20 += (x-xcg)*(x-xcg)*val;
      blob->mu02 += (y-ycg)*(y-ycg)*val;
    }
  }

  for(i=1; i<maxlbl; i++){
    area=part[i].area;
    m00=part[i].m00;
    mu11=part[i].mu11;
    mu20=part[i].mu20;
    mu02=part[i].mu02;
    if (type==9){ /* calculate blob direction using moments ~\cite{teague80}*/
      if (mu20 != mu02){
	phi=atan(2*mu11/(mu20-mu02))/2;
	if (mu20 < mu02){
	  if (mu11 > 0)
	    phi+=(PI/2);
	  else if (mu11 < 0)
	    phi-=(PI/2);
	  else
	    phi=PI/2;
	}
	if (phi<0.0)
	  phi+=PI;
      }
      else if (mu11 > 0)
	phi=(PI/4);
      else if (mu11 < 0)
	phi=(3*PI/4);
      else
	phi=0;
      part[i].dir = (UCHAR)(255.0*phi/PI); /* 255 for label 0 */
    }
    else if (type==10){ /* calculate the correlation coefficient */
      mu20/=area;
      mu02/=area;
      mu11/=area;
      if (mu20>2.0 && mu02>2.0){
	r=mu11/sqrt(mu20*mu02);
	slope=r*sqrt(mu02)/sqrt(mu20);
	cst=ycg-r*xcg*sqrt(mu02*mu20)*area;
	(void)sprintf(buf,"LR: lbl=%d\t slope=%f\t offset=%f\t r=%f\n", (int)i, (float)slope, (float)cst, (float)r); stdputstr(buf);
      }
      else if (mu20<=2.0){
	(void)sprintf(buf,"linear regression for particle %ld outputs a vertical line\n", i); stdputstr(buf);
	r=1;
      }
      else if (mu02<=2.0){
	(void)sprintf(buf,"linear regression for particle %ld outputs a horizontal line\n", i); stdputstr(buf);
	r=1;
      }
      part[i].r=r;
    }
    else if (type==11 || type==13 || type==14 || type==15){ /* compute the ratio of length of the semi minor and major axis */
      c=sqrt((mu20-mu02)*(mu20-mu02)+4*mu11*mu11);
      ratio=sqrt((mu20+mu02+c)/(mu20+mu02-c));
      //(void)sprintf(buf,"ellipsis ratio of particle %d = %f\n", i, (float)ratio); stdputstr(buf);
      part[i].ratio=ratio;
      part[i].minor=(int)(2*sqrt(2*(mu20+mu02-c)/m00));
      //(void)sprintf(buf,"minor axis particle %d = %d\n", i, part[i].minor); stdputstr(buf);
      part[i].major=(int)(2*sqrt(2*(mu20+mu02+c)/m00));
      //(void)sprintf(buf,"major axis particle %d = %d\n", i, part[i].major); stdputstr(buf);
      part[i].irradiance=255 * 4*m00/(PI*part[i].minor*part[i].major);
      //(void)sprintf(buf,"irradiance particle %d = %d\n", i, part[i].irradiance); stdputstr(buf);
    }
  }
  part[0].dir = 255;
  part[0].ratio = 0;
  part[0].r = 0;
  part[0].minor=0;
  part[0].major=0;

  /* set each blob to its its computed value */
  if (type==11){ /* gravity centre */
    for (i=1; i<maxlbl; i++)
       pilbl[(int)(part[i].xcg)+(int)(part[i].ycg)*nx]=1;
  }
#ifndef MSPA
  else if (type==16){ /* agglomerative clustering */
    n=maxlbl-2;
    px=(int *)calloc(n, sizeof(int));
    py=(int *)calloc(n, sizeof(int));
    for (i=2; i<maxlbl; i++){
      px[i-2]=part[i].xcg;
      py[i-2]=part[i].ycg;
    }
    agglo_cluster(px, py, &n, 1.0);

    for(i=0; i<n; i++)
       pilbl[px[i]+py[i]*nx]=1;

    free((char *)px);
    free((char *)py);

  }
  else if (type==17){
    n=maxlbl-2;
    px=(int *)calloc(n, sizeof(int));
    py=(int *)calloc(n, sizeof(int));
    for (i=2; i<maxlbl; i++){
      px[i-2]=part[i].xcg;
      py[i-2]=part[i].ycg;
    }
    nearest_cluster(ilbl, px, py, &n, 75.0);

    free((char *)px);
    free((char *)py);

  }
  else if (type==18){ /* k nearest neighbours */
    n=maxlbl-2;
    px=(int *)calloc(n, sizeof(int));
    py=(int *)calloc(n, sizeof(int));
    for (i=2; i<maxlbl; i++){
      px[i-2]=part[i].xcg;
      py[i-2]=part[i].ycg;
    }
    knearest_cluster(ilbl, px, py, &n, 1, 60.0);

    free((char *)px);
    free((char *)py);

  }
#endif /* ifndef MSPA */
#if defined(CURE_CLUSTER)
  else if (type==19){ /* clustering using CURE clustering algorithm */
    DOUBLE **pattern;
    int *lblt;

    n=maxlbl-1;

    pattern=(DOUBLE **)calloc(n,sizeof(DOUBLE *));
    for (i=0; i<n; i++)
      pattern[i]=(DOUBLE *)calloc((size_t)2,sizeof(DOUBLE));

    lblt=(int *)calloc(n,sizeof(int));

    for (i=0; i<n; i++){
      pattern[i][0]=part[i+2].xcg;
      pattern[i][1]=part[i+2].ycg;
    }
    cluster(pattern, NULL, 2, n, lblt, (double)6400.0);  /* squared distance */

    for (i=0; i<npix; i++)
      if (pilbl[i]>1)
	pilbl[i]=lblt[pilbl[i]-2]+1;

    for (i=0; i<maxlbl-2; i++)
      free((char *)pattern[i]);
    free((char *)pattern);
    free((char *)lblt);

  }
#endif /* if defined(CURE_CLUSTER) */
  else{
    for (i=0; i<npix; i++){
      if (type==9)
	pilbl[i] = (LBL_TYPE)part[pilbl[i]].dir;
      else if (type==10)
	pilbl[i] = (LBL_TYPE)(fabs(part[pilbl[i]].r)*255);
      else if (type==13)
	pilbl[i] = (LBL_TYPE)part[pilbl[i]].minor;
      else if (type==14)
	pilbl[i] = (LBL_TYPE)part[pilbl[i]].major;
      else if (type==15)
	pilbl[i] = (LBL_TYPE)part[pilbl[i]].irradiance;
      else
	pilbl[(int)(part[i].xcg)+(int)(part[i].ycg)*nx]=1;
    }
  }
  return(NO_ERROR);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */

#include "uc_def.h" /* ival image */
ERROR_TYPE uc_tessel_majorityngb(IMAGE *ilbl, IMAGE *ival)
{
  int i, j, lblmaj, maxfreq;
  G_TYPE *pg;
  USHORT *plbl, maxlbl;
  int **ptr;
  int nx=GetImNx(ilbl);
  int ny=GetImNy(ilbl);
  PIX_TYPE *pval, maxval;

  if (GetImDataType(ilbl)!=t_USHORT || GetImDataType(ival)!=t_UCHAR){
    (void)sprintf(buf,"tessel_majorityngb(): image type error\n"); errputstr(buf);
    return(ERROR);
  }

  /* get min & max values */
  pg = min_max(ilbl);
  if (pg == NULL)
    return(ERROR);
  maxlbl = pg[1].us_val;
  free((char *)pg);
  pg = min_max(ival);
  if (pg == NULL)
    return(ERROR);
  maxval = pg[1].uc_val;
  free((char *)pg);

  /* allocate memory for histograms of neighbouring pixels of each labelled region */
  ptr = (int **)calloc(maxlbl+1,sizeof(int));
  for (i=0; i<maxlbl+1; i++)
    ptr[i]=(int *)calloc(maxval+1,sizeof(int));

  plbl=((USHORT *)GetImPtr(ilbl))+nx+1;
  pval=((UCHAR *)GetImPtr(ival))+nx+1;
  for (j=2; j<ny; j++){
    for (i=2; i<nx; i++){
      if (*plbl==0){ /* 4-connected ngb to start with */
	*(ptr[*(plbl-nx)]+*pval) +=1;
        *(ptr[*(plbl-1)]+*pval)  +=1;
        *(ptr[*(plbl+1)]+*pval)  +=1;
        *(ptr[*(plbl+nx)]+*pval) +=1;
      }
      pval++; plbl++;
    }
    pval+=2; plbl+=2;
  }

  /* search for most frequent neighbouring value */
  for (i=1; i<maxlbl+1; i++){
    maxfreq=0;
    lblmaj=0;
    for (j=0; j<maxval+1; j++){
      if (*(ptr[i]+j)>maxfreq){
	lblmaj=j;
	maxfreq=*(ptr[i]+j);
      }
    }
    *(ptr[i])=lblmaj;
  }
  **ptr=0; /* that's the background */

  plbl=(USHORT *)GetImPtr(ilbl);
  LOOPDN(i, GetImNPix(ilbl)){
    if (*plbl)
      *plbl=*(ptr[*plbl]);
    plbl++;
  }

  for (i=0; i<maxlbl+1; i++)
    free((char *)ptr[i]);
  free((char *)ptr);
  return(NO_ERROR);
}
#include "uc_undef.h"

ERROR_TYPE us_tessel_majorityngb(IMAGE *ilbl, IMAGE *ival)
{
  int i, j, lblmaj, maxfreq;
  G_TYPE *pg;
  USHORT *plbl, maxlbl;
  USHORT *pval, maxval;
  int **ptr;
  int nx=GetImNx(ilbl);
  int ny=GetImNy(ilbl);

  if (GetImDataType(ilbl)!=t_USHORT || GetImDataType(ival)!=t_USHORT){
    (void)sprintf(buf,"tessel_majorityngb(): image type error\n"); errputstr(buf);
    return(ERROR);
  }

  /* get min & max values */
  pg = min_max(ilbl);
  if (pg == NULL)
    return(ERROR);
  maxlbl = pg[1].us_val;
  free((char *)pg);
  pg = min_max(ival);
  if (pg == NULL)
    return(ERROR);
  maxval = pg[1].us_val;
  free((char *)pg);

  /* allocate memory for histograms of neighbouring pixels of each labelled region */
  ptr = (int **)calloc(maxlbl+1,sizeof(int));
  for (i=0; i<maxlbl+1; i++)
    ptr[i]=(int *)calloc(maxval+1,sizeof(int));

  plbl=((USHORT *)GetImPtr(ilbl))+nx+1;
  pval=((USHORT *)GetImPtr(ival))+nx+1;
  for (j=2; j<ny; j++){
    for (i=2; i<nx; i++){
      if (*plbl==0){ /* 4-connected ngb to start with */
	*(ptr[*(plbl-nx)]+*pval) +=1;
        *(ptr[*(plbl-1)]+*pval)  +=1;
        *(ptr[*(plbl+1)]+*pval)  +=1;
        *(ptr[*(plbl+nx)]+*pval) +=1;
      }
      pval++; plbl++;
    }
    pval+=2; plbl+=2;
  }

  /* search for most frequent neighbouring value */
  for (i=1; i<maxlbl+1; i++){
    maxfreq=0;
    lblmaj=0;
    for (j=0; j<maxval+1; j++){
      if (*(ptr[i]+j)>maxfreq){
	lblmaj=j;
	maxfreq=*(ptr[i]+j);
      }
    }
    *(ptr[i])=lblmaj;
  }
  **ptr=0; /* that's the background */

  plbl=(USHORT *)GetImPtr(ilbl);
  LOOPDN(i, GetImNPix(ilbl)){
    if (*plbl)
      *plbl=*(ptr[*plbl]);
    plbl++;
  }

  for (i=0; i<maxlbl+1; i++)
    free((char *)ptr[i]);
  free((char *)ptr);
  return(NO_ERROR);
}

ERROR_TYPE i32_tessel_majorityngb(IMAGE *ilbl, IMAGE *ival)
{
  int i, j, lblmaj, maxfreq;
  G_TYPE *pg;
  INT32 *plbl, maxlbl;
  UCHAR *pval, maxval;
  int **ptr;
  int nx=GetImNx(ilbl);
  int ny=GetImNy(ilbl);

  if (GetImDataType(ilbl)!=t_INT32 || GetImDataType(ival)!=t_UCHAR){
    (void)sprintf(buf,"l_tessel_majorityngb() image type error\n"); errputstr(buf);
    return(ERROR);
  }

  /* get min & max values */
  pg = min_max(ilbl);
  if (pg == NULL)
    return(ERROR);
  maxlbl = pg[1].i32_val;
  free((char *)pg);
  pg = min_max(ival);
  if (pg == NULL)
    return(ERROR);
  maxval = pg[1].i32_val;
  free((char *)pg);

  /* allocate memory for histograms of neighbouring pixels of each labelled region */
  ptr = (int **)calloc(maxlbl+1,sizeof(int));
  for (i=0; i<maxlbl+1; i++)
    ptr[i]=(int *)calloc(maxval+1,sizeof(int));

  plbl=((INT32 *)GetImPtr(ilbl))+nx+1;
  pval=((UCHAR *)GetImPtr(ival))+nx+1;
  for (j=2; j<ny; j++){
    for (i=2; i<nx; i++){
      if (*plbl==0){ /* 4-connected ngb to start with */
	*(ptr[*(plbl-nx)]+*pval) +=1;
        *(ptr[*(plbl-1)]+*pval)  +=1;
        *(ptr[*(plbl+1)]+*pval)  +=1;
        *(ptr[*(plbl+nx)]+*pval) +=1;
      }
      pval++; plbl++;
    }
    pval+=2; plbl+=2;
  }

  /* search for most frequent neighbouring value */
  for (i=1; i<maxlbl+1; i++){
    maxfreq=0;
    lblmaj=0;
    for (j=0; j<maxval+1; j++){
      if (*(ptr[i]+j)>maxfreq){
	lblmaj=j;
	maxfreq=*(ptr[i]+j);
      }
    }
    *(ptr[i])=lblmaj;
  }
  **ptr=0; /* that's the background */

  plbl=(INT32 *)GetImPtr(ilbl);
  LOOPDN(i, GetImNPix(ilbl)){
    if (*plbl)
      *plbl=*(ptr[*plbl]);
    plbl++;
  }

  for (i=0; i<maxlbl+1; i++)
    free((char *)ptr[i]);
  free((char *)ptr);
  return(NO_ERROR);
}

ERROR_TYPE tessel_majorityngb(IMAGE *ilbl, IMAGE *ival)
{
  if (GetImDataType(ilbl)!= t_USHORT){
      (void)sprintf(buf, "ERROR in tessel_majorityngb()\
                  image of labels must of type t_USHORT\n"); errputstr(buf);
      return(ERROR);
  }
  switch (GetImDataType(ival)){
  case t_UCHAR:
    return(uc_tessel_majorityngb(ilbl,ival));
  case t_USHORT:
    return(us_tessel_majorityngb(ilbl,ival));
  case t_INT32:
    return(i32_tessel_majorityngb(ilbl,ival));
  }
  return ERROR;
}

ERROR_TYPE set_regions(IMAGE *ilbl, IMAGE *ival, int indic)
{
  /*
   *	Entree: imlbl = image "label" obtenu par 'pavage', 'label' ou
   *			'plages'. Elle a un certain nombre de plages
   *			ayant toutes un label different, et eventuellement
   *			des pixels a 0.
   *		imval = image de gris dans laquelle on cherchera les valeurs
   *			a affecter aux differentes regions de imlbl
   *			en fonction de l'indicateur indic.
   * On suppose que les 2 images precedentes ont memes caracteristiques.
   *		indic = indicateur pouvant prendre les valeurs suivantes:
   *			- 1 ---> moyenne de la zone
   *			- 2 ---> ecart-type
   *			- 3 ---> maximum
   *			- 4 ---> minimum
   *			- 5 ---> max - min
   *			- 6 ---> (max + min) / 2
   *			- 7 ---> label du premier pixel non nul de la zone
   *			- 8 ---> sum of the grey levels of the zone
   *                    - 9 ---> valeur correspondant a l'orientation codee enter 0 et 255
   *                    -10 ---> correlation coefficient of each blob
   *                    -11 ---> compute gravity centre
   *                    -12 ---> use majority rule for neighbours to the region
   *                    -13 ---> set blob to length of minor axis
   *                    -14 ---> set blob to length of major axis
   *                    -15 ---> irradiance of equivalent ellipse fit *255
   *
   * Sortie: imlbl = image ou chacune des zones precedentes a une
   *			nouvelle valeur, determinee a l'aide de indic
   *			et de imval. Seuls les pixels
   *			initialement a 0 dans imlbl ne sont pas touches.
   */


  /* error checking */
  if ( (indic == 1) || (indic == 3) || (indic == 4) || (indic == 8) || (indic == 12) || (indic == 20)  ){
    if (GetImNPix(ilbl) != GetImNPix(ival) ){
      (void)sprintf(buf, "ERROR in set_regions(): \
                  images of different size \n"); errputstr(buf);
      return(ERROR);
    }
  }
  else if ( (GetImNPix(ilbl) != GetImNPix(ival)) ||\
	    (GetImDataType(ilbl) != t_LBL_TYPE) ){
    (void)sprintf(buf, "ERROR in set_regions(): \
                  images of different size or \
                  label image not of type t_LBL_TYPE\n"); errputstr(buf);
    return(ERROR);
  }

  switch (indic){
    case 1:
      return(tessel_mean(ilbl, ival));
      break;
    case 2:
      return(tessel_sigma(ilbl, ival));
      break;
    case 3:
      return(tessel_max(ilbl, ival));
      break;
    case 4:
      return(tessel_min(ilbl, ival));
      break;
    case 8:
      return(tessel_sum(ilbl, ival));
      break;     
    case 9:  /* orientation of matching ellipsis */
    case 10: /* correlation coefficient */
    case 11: /* gravity centre */ 
    case 13: /* minor axis of matching ellipsis */
    case 14: /* major axis of matching ellipsis */
    case 15: /* irradiance */
    case 16: /* agglomerative clustering */
    case 17: /* nearest neighbours */
    case 18: /* k nearest neighbours */
    case 19: /* CURE hierarchical clustering */
      return(uc_tessel_dir(ilbl,ival,indic));
      break;
    case 12:
      return(tessel_majorityngb(ilbl,ival));
      break;
    case 20:
      return(tessel_range(ilbl,ival));
      break;
    default:
	 (void)sprintf(buf, "ERROR in set_regions(): \
    			     invalid indicator number\n"); errputstr(buf);
  }
  return ERROR;
}



#include "uc_def.h" /* ival image */
ERROR_TYPE uc_setregionsgraph(IMAGE *ilbl, IMAGE *ival, int indic, int graph)
{
  int i, j, k;
  G_TYPE *pg;
  UINT32 *plbl, maxlbl;
  PIX_TYPE *ptrmin, *ptrmax;
  PIX_TYPE *ptrmind, *ptrmaxd;
  int nx=GetImNx(ilbl);
  int ny=GetImNy(ilbl);
  PIX_TYPE *pval;
  long int shft[27];

  /* take graph into account */
  if (set_seq_shift(GetImNx(ilbl), GetImNy(ilbl), GetImNz(ilbl), graph, shft) == ERROR)
    return ERROR;


  /* get min & max values */
  pg = min_max(ilbl);
  if (pg == NULL)
    return(ERROR);
  maxlbl = pg[1].u32_val;
  free((char *)pg);

  /* allocate memory for histograms of neighbouring pixels of each labelled region */
  ptrmin = (PIX_TYPE *)calloc(maxlbl+1,sizeof(PIX_TYPE));
  ptrmax = (PIX_TYPE *)calloc(maxlbl+1,sizeof(PIX_TYPE));
  for (i=0; i<maxlbl; i++){
    ptrmin[i]=PIX_MAX;
    ptrmax[i]=PIX_MIN;
  }
  ptrmind = (PIX_TYPE *)calloc(maxlbl+1,sizeof(PIX_TYPE));
  ptrmaxd = (PIX_TYPE *)calloc(maxlbl+1,sizeof(PIX_TYPE));
  for (i=0; i<maxlbl; i++){
    ptrmind[i]=PIX_MAX;
    ptrmaxd[i]=PIX_MIN;
  }

  plbl=((UINT32 *)GetImPtr(ilbl))+nx+1;
  pval=((PIX_TYPE *)GetImPtr(ival))+nx+1;
  for (j=2; j<ny; j++){
    for (i=2; i<nx; i++){
      for (k=0; k<graph; k++){
	if ( *plbl != *(plbl+shft[k]) ){

	  if ( ptrmin[*(plbl+shft[k])] > *pval)
	    ptrmin[*(plbl+shft[k])]=*pval;
	  else if ( ptrmax[*(plbl+shft[k])] < *pval)
	    ptrmax[*(plbl+shft[k])]=*pval;

	  if ( ptrmind[*(plbl+shft[k])] > abs( (int) *pval-*(pval+shft[k])) )
	    ptrmind[*(plbl+shft[k])]=abs( (int) *pval-*(pval+shft[k]));
	  else if ( ptrmaxd[*(plbl+shft[k])] < abs( (int) *pval-*(pval+shft[k])))
	    ptrmaxd[*(plbl+shft[k])]=abs( (int) *pval-*(pval+shft[k]));

	}
      }
      pval++; plbl++;
    }
    pval+=2; plbl+=2;
  }

  plbl=(UINT32 *)GetImPtr(ilbl);
  LOOPDN(i, GetImNPix(ilbl)){
    if (indic==0)
      *plbl=ptrmin[*plbl];
    else if (indic==1)
      *plbl=ptrmax[*plbl];
    else if (indic==2)
      *plbl=ptrmax[*plbl]-ptrmin[*plbl];
    else if (indic==3)
      *plbl=ptrmind[*plbl];
    else if (indic==4)
      *plbl=ptrmaxd[*plbl];
    else if (indic==5)
      *plbl=ptrmaxd[*plbl]-ptrmind[*plbl];
    plbl++;
  }

  free((char *)ptrmin);
  free((char *)ptrmax);
  return(NO_ERROR);
}
#include "uc_undef.h"




ERROR_TYPE setregionsgraph(IMAGE *ilbl, IMAGE *ival, int indic, int graph)
{

  if ( (GetImNPix(ilbl) != GetImNPix(ival)) || \
	    (GetImDataType(ilbl) != t_LBL_TYPE) ){
    (void)sprintf(buf, "ERROR in setregionsgraph(): \
                  images of different size or \
                  label image not of type t_LBL_TYPE\n"); errputstr(buf);
    return(ERROR);
  }

  return uc_setregionsgraph(ilbl, ival, indic, graph);
}




#ifndef NO_generic_IMAGE
#include "g_def.h"
ERROR_TYPE generic_tessel_surface(IMAGE *im)
{
  PIX_TYPE *p;
  int i, count=0;
  HST1D_TYPE *pnbr;
  IMAGE *imhst;

  imhst=histo1d(im);

  if (imhst==NULL)
    return ERROR;

  pnbr = (HST1D_TYPE *) GetImPtr(imhst);
  p = (PIX_TYPE *)GetImPtr(im);

  pnbr[0]=0; /* force background to zero */

  LOOPDN(i, GetImNPix(im)){
    if (pnbr[*p]>PIX_MAX){
     count+=1;
     pnbr[*p]=PIX_MAX;
    }
    *p = pnbr[*p];
    p++;
  }
  if (count){
    (void)sprintf(buf,"%d regions have an area greater than PIX_MAX, output value saturated at PIX_MAX\n", count); errputstr(buf);
  }

  free_image(imhst);

  return NO_ERROR;
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */


#include "i32_def.h"
ERROR_TYPE i32_tessel_surface(IMAGE *im)
{
  PIX_TYPE *p;
  int i, count=0;
  HST1D_TYPE *pnbr;
  IMAGE *imhst;

  imhst=histo1d(im);

  if (imhst==NULL)
    return ERROR;

  pnbr = (HST1D_TYPE *) GetImPtr(imhst);
  p = (PIX_TYPE *)GetImPtr(im);

  pnbr[0]=0; /* force background to zero */

  LOOPDN(i, GetImNPix(im)){
    if (pnbr[*p]>PIX_MAX){
     count+=1;
     pnbr[*p]=PIX_MAX;
    }
    *p = pnbr[*p];
    p++;
  }
  if (count){
    (void)sprintf(buf,"%d regions have an area greater than PIX_MAX, output value saturated at PIX_MAX\n", count); errputstr(buf);
  }

  free_image(imhst);

  return NO_ERROR;
}
#include "i32_undef.h"


#include "u32_def.h"
ERROR_TYPE u32_tessel_surface(IMAGE *im)
{
  PIX_TYPE *p;
  int i, count=0;
  HST1D_TYPE *pnbr;
  IMAGE *imhst;

  imhst=histo1d(im);

  if (imhst==NULL)
    return ERROR;

  pnbr = (HST1D_TYPE *) GetImPtr(imhst);
  p = (PIX_TYPE *)GetImPtr(im);

  pnbr[0]=0; /* force background to zero */

  LOOPDN(i, GetImNPix(im)){
    if (pnbr[*p]>PIX_MAX){
     count+=1;
     pnbr[*p]=PIX_MAX;
    }
    *p = pnbr[*p];
    p++;
  }
  if (count){
    (void)sprintf(buf,"%d regions have an area greater than PIX_MAX, output value saturated at PIX_MAX\n", count); errputstr(buf);
  }

  free_image(imhst);

  return NO_ERROR;
}
#include "u32_undef.h"


#include "us_def.h"
ERROR_TYPE us_tessel_surface(IMAGE *im)
{
  PIX_TYPE *p;
  int i, count=0;
  HST1D_TYPE *pnbr;
  IMAGE *imhst;

  imhst=histo1d(im);

  if (imhst==NULL)
    return ERROR;

  pnbr = (HST1D_TYPE *) GetImPtr(imhst);
  p = (PIX_TYPE *)GetImPtr(im);

  pnbr[0]=0; /* force background to zero */

  LOOPDN(i, GetImNPix(im)){
    if (pnbr[*p]>PIX_MAX){
     count+=1;
     pnbr[*p]=PIX_MAX;
    }
    *p = pnbr[*p];
    p++;
  }
  if (count){
    (void)sprintf(buf,"%d regions have an area greater than PIX_MAX, output value saturated at PIX_MAX\n", count); errputstr(buf);
  }

  free_image(imhst);

  return NO_ERROR;
}
#include "us_undef.h"


/*
 **	Set each region of partioned image im to
 **	the its corresponding surface.
 */
ERROR_TYPE tessel_surface(IMAGE *im)
{
  switch (GetImDataType(im))
    {
    case t_UCHAR:
      return generic_tessel_surface(im);
      break;
    case t_USHORT:
      return us_tessel_surface(im);
      break;
    case t_INT32:
      return i32_tessel_surface(im);
      break;
    case t_UINT32:
      return u32_tessel_surface(im);
      break;
    default:
      return(ERROR);
    }
}

#define set_lbl2lbl_type(TYPE1, TYPE2, i1, i2, i3) \
{ \
  TYPE1 *plbl1 = (TYPE1 *) GetImPtr(i1);  \
  TYPE2 *plbl2 = (TYPE2 *) GetImPtr(i2);  \
  TYPE1 *pval2 = (TYPE1 *) GetImPtr(i3);  \
  LOOPDN(i, GetImNPix(i1)){  \
    if (parea[*plbl1]<*pval2){ \
      parea[*plbl1]=*pval2; \
      plbl[*plbl1]=*plbl2; \
    } \
    plbl1++;  \
    plbl2++;  \
    pval2++;  \
  }\
  plbl1 = (TYPE1 *)GetImPtr(i1);  \
  LOOPDN(i, GetImNPix(i1)){  \
    *plbl1 = plbl[*plbl1];  \
    plbl1++;\
  }  \
}

#include "u32_def.h"
ERROR_TYPE u32_relabel(IMAGE *ilbl1, IMAGE *ilbl2, IMAGE *iarea2)
{
  G_TYPE *pg;
  PIX_TYPE maxlbl1;
  PIX_TYPE *parea, *plbl;
  unsigned long int i;

  /* get max lbl in ilbl1 */
  pg = min_max(ilbl1);
  if (pg == NULL)
    return(ERROR);
  maxlbl1 = pg[1].u32_val +1;
  free((char *)pg);

  parea = (PIX_TYPE *)calloc(maxlbl1, sizeof(PIX_TYPE));
  if (parea==NULL){
    (void)sprintf(buf,"(): insufficient memory\n"); errputstr(buf);
  }
  plbl = (PIX_TYPE *)calloc(maxlbl1, sizeof(PIX_TYPE));
  if (plbl==NULL){
    free(parea);
    (void)sprintf(buf,"(): insufficient memory\n"); errputstr(buf);
  }

  set_lbl2lbl_type(UINT32, UINT32, ilbl1, ilbl2, iarea2);

  free(parea);
  free(plbl);
  return  NO_ERROR;

}
#include "u32_undef.h"


ERROR_TYPE relabel(IMAGE *ilbl1, IMAGE *ilbl2, IMAGE *iarea2)
{
  switch (GetImDataType(ilbl1)){
    case t_UINT32:
    case t_INT32:
      return u32_relabel(ilbl1, ilbl2, iarea2);
      break;
    default:
      return(ERROR);
    }
}


/*@}*/
