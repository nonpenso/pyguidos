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

/** @file
 *  Watershed computation by immersion simulation \cite soille-vincent90  see also \cite vincent-soille91
 *  @author Pierre Soille
 */
#include <stdio.h>
#include <stdlib.h>
#include "miallib.h"
#include "fifo.h"


/** @defgroup group_seg Segmentation
 *  Functions dealing with image segmentation
 *  @{
 */
/*
**  Function to sort pointers to an input array of n pixels in an
**  increasing order of their value.  Processing time is in O(n).
**  Distributive sort.
*/

#include "uc_def.h"
IMAGE *uc_pixsort(IMAGE *im, IMAGE *imrsum)
{
  IMAGE *imsort;
  PIX_TYPE *pim, **psort;
  HST1D_TYPE *prsum;
  unsigned i, npix = GetImNPix(im);

  imsort = create_image(t_PTR, GetImNx(im), GetImNy(im), GetImNz(im));
  if (imsort==NULL)
    return(NULL);

  psort = (PIX_TYPE **)GetImPtr(imsort);
  prsum = (HST1D_TYPE *)GetImPtr(imrsum);
  pim   = (PIX_TYPE *)GetImPtr(im);

  for (i=0; i<npix; i++){
    *(psort + prsum[pim[i]]) = (PIX_TYPE *)(pim + i);
    prsum[pim[i]] += 1;
  }
  for (i=GetImNPix(imrsum)-1; i>0; i--)
    prsum[i] = prsum[i-1];
  prsum[0] = 0;

  return(imsort);
}
#include "uc_undef.h"

#include "u32_def.h"
IMAGE *u32_pixsort(IMAGE *im, IMAGE *imrsum)
{
  IMAGE *imsort;
  PIX_TYPE *pim, **psort;
  HST1D_TYPE *prsum;
  unsigned i, npix = GetImNPix(im);

  imsort = create_image(t_PTR, GetImNx(im), GetImNy(im), GetImNz(im));
  if (imsort==NULL)
    return(NULL);

  psort = (PIX_TYPE **)GetImPtr(imsort);
  prsum = (HST1D_TYPE *)GetImPtr(imrsum);
  pim   = (PIX_TYPE *)GetImPtr(im);

  for (i=0; i<npix; i++){
    *(psort + prsum[pim[i]]) = (PIX_TYPE *)(pim + i);
    prsum[pim[i]] += 1;
  }
  for (i=GetImNPix(imrsum)-1; i>0; i--)
    prsum[i] = prsum[i-1];
  prsum[0] = 0;

  return(imsort);
}
#include "u32_undef.h"

#include "us_def.h"
IMAGE *us_pixsort(IMAGE *im, IMAGE *imrsum)
{
  IMAGE *imsort;
  PIX_TYPE *pim, **psort;
  HST1D_TYPE *prsum;
  unsigned i, npix = GetImNPix(im);

  imsort = create_image(t_PTR, GetImNx(im), GetImNy(im), GetImNz(im));
  if (imsort==NULL)
    return(NULL);

  psort = (PIX_TYPE **)GetImPtr(imsort);
  prsum = (HST1D_TYPE *)GetImPtr(imrsum);
  pim   = (PIX_TYPE *)GetImPtr(im);

  for (i=0; i<npix; i++){
    *(psort + prsum[pim[i]]) = (PIX_TYPE *)(pim + i);
    prsum[pim[i]] += 1;
  }
  for (i=GetImNPix(imrsum)-1; i>0; i--)
    prsum[i] = prsum[i-1];
  prsum[0] = 0;

  return(imsort);
}
#include "us_undef.h"


#include "i32_def.h"
IMAGE *i32_pixsort(IMAGE *im, IMAGE *imrsum)
{
  IMAGE *imsort;
  PIX_TYPE *pim, **psort;
  HST1D_TYPE *prsum;
  unsigned i, npix = GetImNPix(im);

  imsort = create_image(t_PTR, GetImNx(im), GetImNy(im), GetImNz(im));
  if (imsort==NULL)
    return(NULL);

  psort = (PIX_TYPE **)GetImPtr(imsort);
  prsum = (HST1D_TYPE *)GetImPtr(imrsum);
  pim   = (PIX_TYPE *)GetImPtr(im);

  for (i=0; i<npix; i++){
    *(psort + prsum[pim[i]]) = (PIX_TYPE *)(pim + i);
    prsum[pim[i]] += 1;
  }
  for (i=GetImNPix(imrsum)-1; i>0; i--)
    prsum[i] = prsum[i-1];
  prsum[0] = 0;

  return(imsort);
}
#include "i32_undef.h"

/*!
 * @param im an IMAGE pointer argument.
 * @param imrsum an IMAGE pointer argument holding the running sum of the histogram of the values in im.

 Sort the image pixels in increasing order of their pixel value and output an array of pointers to the sorted pixel values.
 */
IMAGE *pixsort(IMAGE *im, IMAGE *imrsum)
{
  switch (GetImDataType(im)){

  case t_UCHAR:
    return(uc_pixsort(im, imrsum));
    break;

  case t_USHORT:
    return(us_pixsort(im, imrsum));
    break;

  case t_INT32:
    return(i32_pixsort(im, imrsum));
    break;

  case t_UINT32:
    return(u32_pixsort(im, imrsum));
    break;

  default:
    (void)sprintf(buf,"pixsort(): invalid pixel type\n"); errputstr(buf);
  }
  return(NULL);
}

#ifndef NO_generic_IMAGE
#include "g_def.h"
IMAGE *generic_sort_offset(IMAGE *im, IMAGE *imrsum)
{
  IMAGE *imsort;
  PIX_TYPE *pim;
  UINT32 *psort;
  HST1D_TYPE *prsum;
  unsigned i, npix = GetImNPix(im);

  imsort = create_image(t_UINT32, GetImNx(im), GetImNy(im), GetImNz(im));
  if (imsort==NULL)
    return(NULL);

  psort = (UINT32 *)GetImPtr(imsort);
  prsum = (HST1D_TYPE *)GetImPtr(imrsum);
  pim   = (PIX_TYPE *)GetImPtr(im);

  for (i=0; i<npix; i++){
    *(psort + prsum[pim[i]]) = i;
    prsum[pim[i]] += 1;
  }
  for (i=GetImNPix(imrsum)-1; i>0; i--)
    prsum[i] = prsum[i-1];
  prsum[0] = 0;

  return(imsort);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */

#include "us_def.h"
IMAGE *us_sort_offset(IMAGE *im, IMAGE *imrsum)
{
  IMAGE *imsort;
  PIX_TYPE *pim;
  UINT32 *psort;
  HST1D_TYPE *prsum;
  unsigned i, npix = GetImNPix(im);

  imsort = create_image(t_UINT32, GetImNx(im), GetImNy(im), GetImNz(im));
  if (imsort==NULL)
    return(NULL);

  psort = (UINT32 *)GetImPtr(imsort);
  prsum = (HST1D_TYPE *)GetImPtr(imrsum);
  pim   = (PIX_TYPE *)GetImPtr(im);

  for (i=0; i<npix; i++){
    *(psort + prsum[pim[i]]) = i;
    prsum[pim[i]] += 1;
  }
  for (i=GetImNPix(imrsum)-1; i>0; i--)
    prsum[i] = prsum[i-1];
  prsum[0] = 0;

  return(imsort);
}
#include "us_undef.h"

#include "i32_def.h"
IMAGE *i32_sort_offset(IMAGE *im, IMAGE *imrsum)
{
  IMAGE *imsort;
  PIX_TYPE *pim;
  UINT32 *psort;
  HST1D_TYPE *prsum;
  unsigned i, npix = GetImNPix(im);

  imsort = create_image(t_UINT32, GetImNx(im), GetImNy(im), GetImNz(im));
  if (imsort==NULL)
    return(NULL);

  psort = (UINT32 *)GetImPtr(imsort);
  prsum = (HST1D_TYPE *)GetImPtr(imrsum);
  pim   = (PIX_TYPE *)GetImPtr(im);

  for (i=0; i<npix; i++){
    *(psort + prsum[pim[i]]) = i;
    prsum[pim[i]] += 1;
  }
  for (i=GetImNPix(imrsum)-1; i>0; i--)
    prsum[i] = prsum[i-1];
  prsum[0] = 0;

  return(imsort);
}
#include "i32_undef.h"


IMAGE *sort_offset(IMAGE *im, IMAGE *imrsum)
{
  switch (GetImDataType(im)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_sort_offset(im, imrsum));
    break;
#endif

#ifndef NO_uc_IMAGE
  case t_UCHAR:
    return(uc_sort_offset(im, imrsum));
    break;
#endif

  case t_USHORT:
    return(us_sort_offset(im, imrsum));
    break;

  case t_INT32:
    return(i32_sort_offset(im, imrsum));
    break;

  default:
    (void)sprintf(buf,"sort_offset(): invalid pixel type\n"); errputstr(buf);
  }
  return(NULL);
}


#include "us_def.h"
#define  INQUEUE  PIX_MAX-1
#define  WINIT    PIX_MAX-2
#define  MASK     PIX_MAX-3
#define  WSHED    PIX_MAX-4
ERROR_TYPE us_flood1(IMAGE *im, IMAGE *hst, IMAGE *srtim, int graph)
{
  PIX_TYPE *pim, *pcrt, *pend, *ptr, *p, **psrt, lbl = 0;
  HST1D_TYPE *phst;
  FIFO4 *q;
  long int i, j, k, nlev, shift[27], npix;
  int flag = FALSE;

  pim  = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

  /* set to WINIT input image */
  pend = pim + npix;
  for (pcrt = pim; pcrt < pend;)
    *pcrt++ = WINIT;

  /* create a queue */
  q = create_fifo4(npix/100L);
  if (q == NULL){
    (void) sprintf(buf, "us_flood1(): not enough memory"); errputstr(buf);
    return ERROR;
  }

  /* take graph into account */
  if (set_seq_shift(GetImNx(im), GetImNy(im), GetImNz(im), graph, shift) == ERROR)
    return ERROR;

  /* here we go */
  nlev = GetImNx(hst)-1;
  phst = (HST1D_TYPE *)GetImPtr(hst);
  psrt = (PIX_TYPE **)GetImPtr(srtim);
  for (i = 1; i < nlev; ++i){
    /* geodesic SKIZ of level Li-1 inside level Li */
    for (j = phst[i-1]; j < phst[i]; ++j){
      *(ptr = psrt[j]) = MASK;
      for (k = 0; k < graph; ++k){
	if (*(ptr + shift[k]) < MASK){
	  *ptr = INQUEUE;
	  fifo4_add(q, (fifo4_word_t)ptr);
	  break;
	}
      }
    }
    ptr = (PIX_TYPE *)fifo4_remove(q);
    while (ptr){
      for (k = 0; k < graph; ++k){
	if (*(p = ptr + shift[k]) < WSHED){
	  if ( (*ptr == INQUEUE) || (*ptr == WSHED && flag == TRUE) )
	    *ptr = *p;
	  else if ( (*ptr < WSHED) && (*ptr != *p) ){
	    *ptr = WSHED;
	    flag = FALSE;
	  }
	}
	else if (*p == WSHED){
	  if (*ptr == INQUEUE){
	    *ptr = WSHED;
	    flag = TRUE;
	  }
	}
	else if (*p == MASK){
	  *p = INQUEUE;
	  fifo4_add(q, (fifo4_word_t)p);
	}
      }
      ptr = (PIX_TYPE *)fifo4_remove(q);
    }
    /* check if new minima have been discovered, and label them */
    for (j = phst[i-1]; j < phst[i]; ++j){
      ptr = psrt[j];
      if (*ptr == MASK){
	*ptr = lbl;
	fifo4_add(q, (fifo4_word_t)ptr);
	while ((ptr = (PIX_TYPE *)fifo4_remove(q))){
	  for (k = 0; k < graph; ++k){
	    if (*(p = ptr + shift[k]) == MASK){
	      *p = lbl;
	      fifo4_add(q, (fifo4_word_t)p);
	    }
	  }
	}
	lbl++;
	if (lbl==WSHED){ /* start again with 0 labels and prey */
	  (void)sprintf(buf,"WARNING in us_flood1: label overflow starting from 0 again\n");
	  errputstr(buf);
	  lbl=0;
	}
      }
    }
  }
  free_fifo4(q);

  for (i=0; i<npix; i++){
    if (pim[i]==WSHED)
      pim[i] = 1;
    else
      pim[i] = 0;
  }
  return NO_ERROR;
}
#undef  WSHED
#undef  INQUEUE
#undef  WINIT
#undef  MASK
#include "us_undef.h"


#include "us_def.h"
ERROR_TYPE us_ws(IMAGE *im, int graph)
{
  IMAGE *imhst, *imrsum, *imsort;
  G_TYPE *pg;
  PIX_TYPE maxi;
  int box[6];

  if (GetImNy(im) == 1)
    {BOX_1D;}
  else if (GetImNz(im) == 1)
    {BOX_2D;}
  else
    {BOX_3D;}

  /* get min & max values */
  pg = min_max(im);
  if (pg == NULL)
    return(ERROR);
  maxi = pg[1].us_val;
  free((char *)pg);

  if (maxi==PIX_MAX){
    if (us_setlevel(im, PIX_MAX, PIX_MAX, PIX_MAX-1) == ERROR)
      return(ERROR);
    maxi -= 1;
  }
  if (us_framebox(im, box, maxi+1) == ERROR)
    return(ERROR);

  /* Compute cumulated histogram */
  imhst = histo1d(im);
  if (imhst==NULL)
    return(ERROR);
  imrsum = rsum(imhst);
  free_image(imhst);
  if (imrsum==NULL)
    return(ERROR);


  /* Sort pointers to buf_n in an increasing order of its gray levels */
  imsort = us_pixsort(im, imrsum);
  if (imsort==NULL)
    return(ERROR);

  /* simulate flooding */
  if (us_flood1(im, imrsum, imsort, graph) == ERROR){
    free_image(imrsum);
    free_image(imsort);
    return(ERROR);
  }
  free_image(imrsum);
  free_image(imsort);

  if (us_framebox(im, box, 1) == ERROR)
    return(ERROR);
  return(NO_ERROR);
}
#include "us_undef.h"



IMAGE *ws(IMAGE *im, int graph)
{
  IMAGE *imtmp;

  switch (GetImDataType(im)){

  case t_UCHAR:
    imtmp = to_ushort(im);
    if (imtmp == NULL)
      return(NULL);
    if (us_ws(imtmp, graph) == ERROR){
      free_image(imtmp);
      return(NULL);
    }
    /* imout = to_uchar(imtmp); */
    /* free_image(imtmp); */
    to_uchar(imtmp);
    return(imtmp);
    break;
  case t_USHORT:
    imtmp = copy_image(im);
    if (imtmp == NULL)
      return(NULL);
    if (us_ws(imtmp, graph) == ERROR){
      free_image(imtmp);
      return(NULL);
    }
    /* imout = to_uchar(imtmp); */
    /* free_image(imtmp); */
    to_uchar(imtmp);
    return(imtmp);
    break;
  default:
    (void)sprintf(buf,"ws(): invalid pixel type\n"); errputstr(buf);
  }
  return(NULL);
}


/*@}*/
