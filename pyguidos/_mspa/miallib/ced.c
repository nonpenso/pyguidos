/***********************************************************************
Author(s): Pierre Soille
Copyright (C) 2005-2020 European Union (Joint Research Centre)

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

#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>
#include  "miallib.h"
#include  "fifo.h"
#ifdef OPENMP
#include <omp.h>
#endif


/** \addtogroup group_dist
 *  @{
 */

/*
**  'Optimised' Bresenham's algorithm used in ced function.
*/
int inmaskp(IMAGE *buf_n, long int offset_r, long int x2, long int y2)
{
  /*
  **  buf_n  :  I(UCHAR)
  **  x1 y1  :  coordinates of first point
  **  x2 y2  :  coordinates of second point
  */

  long int d, dx, dy, s1, s2, incr1, incr2, x, y, temp, der;
  long int *pp;
  UCHAR *p  = (UCHAR *)GetImPtr(buf_n) + offset_r;
  int interchange  = TRUE;
  long int ncol = GetImNx(buf_n);

  dx = abs(x2);  dy = abs(y2);
  s1 = SGN(x2);  s2 = SGN(y2);
  if (dy > dx)
    MYSWAP(dx, dy, temp);
  else
    interchange = FALSE;
  d = 2 * dy - dx;
  incr1 = 2 * dy;
  incr2 = 2 * (dy - dx);
  x = y = 0;
  der = dx;
  pp = &x;
  if (interchange)
    pp = &y;
  while (abs(*pp) < der)
    {
      if (interchange)
	y += s2;
      else
	x += s1;
      if (d < 0)
	d += incr1;
      else
	{
	  d += incr2;
	  if (interchange)
	    x += s1;
	  else
	    y += s2;
	}
      if (*(p + x + y * ncol) == 0){
	return FALSE;
      }
    }
  return TRUE;
}


#define INQUEUE 2
#define FICTITIOUS -1
#define DXY_TYPE SHORT
#define t_DXY_TYPE t_SHORT
IMAGE *ced(IMAGE *ref, IMAGE *mask)
{
  /*
  ** authors: Pierre Soille (c) 1991
  ** IMAGE *ref:  binary UCHAR image with reference pixels set to 1
  ** IMAGE *mask: binary UCHAR image with geodesic mask pixels set to 1

  ** comment: rewritten on Oct. 11 2005 [1.5 days]
  **          added DXY_TYPE and set it to SHORT on 20130902 (was fixed to INT32 before)
  **          so that the equivalent of 1 4-byte image is saved
  **          ! to_ushort needs to be changed internally if DXY_TYPE changes !
  */

  IMAGE *imx, *imy, *imb;
  UCHAR *pr, *pm;
  float dp, dcrt, *pb, dmin;
  int nx, graph=4, k, dx[8], dy[8], base_flag;
  long int i, npix, shft[27], ofs, ofsk;
  DXY_TYPE *px, *py;
  int box[6];
  FIFO4 *q=NULL;

  BOX_2D;

  if ( (GetImDataType(ref)!=t_UCHAR) || (GetImDataType(mask)!=t_UCHAR) ){
    (void)sprintf(buf, "ERROR in ced(): \
               both the ref and mask images must be of type t_UCHAR\n"); errputstr(buf);
    return NULL;
  }
  if (graph == 4){
    dx[0] = 0;  dy[0] = 1;
    dx[1] = 1;  dy[1] = 0;
    dx[2] = 0;  dy[2] = -1;
    dx[3] = -1;  dy[3] = 0;
  }
  else if (graph == 8){
    dx[0] = 0;  dy[0] = 1;
    dx[1] = 1;  dy[1] = 0;
    dx[2] = 0;  dy[2] = -1;
    dx[3] = -1;  dy[3] = 0;
    printf("incomplete set for dx and dy when graph equals 8\n");
  }

  generic_addframebox(ref,box,0);
  generic_addframebox(mask,box,0);
  set_seq_shift(GetImNx(ref), GetImNy(ref), GetImNz(ref), graph, shft);

  nx = GetImNx(ref);
  npix=GetImNPix(ref);

  q = create_fifo4((npix)/100L+1024);
  if (q == NULL){
    return NULL;
  }

  /* init queue */
  pr = (UCHAR *)GetImPtr(ref);
  pm = (UCHAR *)GetImPtr(mask);
  for (i=0;i<npix;i++){
    if (pr[i] && pm[i]){
      pm[i]=3;
      for (k=0;k<graph;k++){
	ofsk=i+shft[k];
	if ( (pm[ofsk]==1) && (pr[ofsk]==0) ){
	  fifo4_add(q,ofsk);
	  pm[ofsk]=INQUEUE;
	}
      }
    }
  }
  imx=(IMAGE *)to_ushort(mask);
  if (imx==NULL){
    free_fifo4(q);
    return NULL;
  }
  SetImDataType(imx,t_SHORT);
  imy=(IMAGE *)to_ushort(mask);
  if (imy==NULL){
    free_fifo4(q);
    free_image(imx);
    return NULL;
  }
  SetImDataType(imy,t_SHORT);
  imb=(IMAGE *)create_image(t_FLOAT,GetImNx(ref),GetImNy(ref),1);
  if (imb==NULL){
    free_fifo4(q);
    free_image(imx);
    free_image(imy);
    return NULL;
  }

  s_setlevel(imx,0,2,SHORT_MAX);
  s_setlevel(imy,0,2,SHORT_MAX);
  s_setlevel(imx,3,3,0);
  s_setlevel(imy,3,3,0);

  /* ordered propagation of distances */
  px=(DXY_TYPE *)GetImPtr(imx);
  py=(DXY_TYPE *)GetImPtr(imy);
  pb=(float *)GetImPtr(imb);
  while (fifo4_empty(q)==FALSE){
    fifo4_add(q,FICTITIOUS);
    dmin=MIALFLOAT_MAX;
    fifo4_lookreset(q);
    while( (ofs=fifo4_look(q)) != FICTITIOUS ){
      if( px[ofs] != SHORT_MAX)
	continue;
      dp=MIALFLOAT_MAX;
      for(k=0;k<graph;k++){
	ofsk=ofs+shft[k];
	if( px[ofsk] != SHORT_MAX ){
	  if (inmaskp(mask,ofsk-px[ofsk]-py[ofsk]*nx,px[ofsk]+dx[k],py[ofsk]+dy[k])){
	    base_flag=FALSE;
	    dcrt= sqrt((double)(SQ((INT64)px[ofsk] + dx[k]) + SQ((INT64)py[ofsk] + dy[k]))) + pb[ofsk];
	  }
	  else{
	    base_flag=TRUE;
	    dcrt= sqrt((double)(SQ((INT64)px[ofsk]) + SQ((INT64)py[ofsk]))) + pb[ofsk] + 1;
	  }
	  if (dcrt < dp){
	    if (base_flag){
	      px[ofs] = dx[k];
	      py[ofs] = dy[k];
	      pb[ofs] = dcrt - 1;
	    }
	    else{
	      px[ofs] = px[ofsk] + dx[k];
	      py[ofs] = py[ofsk] + dy[k];
	      pb[ofs] = pb[ofsk];
	    }
	    dp = dcrt;
	  }
	}
      }
      if (dmin > dp)
	dmin = dp;
    }
    while( (ofs=fifo4_remove(q)) != FICTITIOUS ){
      if (sqrt((double)(SQ((INT64)px[ofs]) + SQ((INT64)py[ofs]))) + pb[ofs] > dmin){
	fifo4_add(q,ofs);
	/* px[ofs] = INT32_MAX; */
      }
      else{
	for (k = 0; k < graph; ++k){
	  ofsk=ofs+shft[k];
	  if (pm[ofsk] == 1){
	    fifo4_add(q,ofsk);
	    pm[ofsk]=INQUEUE;
	  }
	}
      }
    }
  }

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0;i<npix;i++){
    if (px[i] != SHORT_MAX)
      pb[i] += (MIALFLOAT)sqrt((double)((INT64)px[i] * px[i] + (INT64)py[i] * py[i]));
  }

  free_image(imx);
  free_image(imy);
  free_fifo4(q);

  subframebox(ref,box);
  subframebox(mask,box);
  subframebox(imb,box);
  generic_setlevel(mask,2,3,1);
  return imb;
}
#undef INQUEUE
#undef FICTITIOUS
#undef DXY_TYPE
#undef t_DXY_TYPE


/*@}*/
