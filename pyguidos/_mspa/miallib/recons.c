/***********************************************************************
Author(s): Pierre Soille
Copyright (C) 2003-2020 European Union (Joint Research Centre)

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
#include "miallib.h"
#include "fifo.h"
#include "pqueue.h"

// add_framebox/sub_framebox too much memory demanding in reconstruction
// added flagged version on 2006-10-20



/** @defgroup group_geod Geodesic transformations
 *  Functions dealing with image I/0.
 *  @{
 */





#include "uc_def.h"
ERROR_TYPE uc_rdil(IMAGE *mark, IMAGE *mask, int n, int flag)
{
  /*
  ** author: P. Soille 2003-07-02
  ** mark: Marker Image
  ** mask: Mask Image (mask <= mark)
  ** shft: array of offsets to neighbours
  ** n: number of neighbours
  ** flag: if 0 then overwrite border else add border

  ** comment: The transformed image is written in the marker image.
  */

  long int shft[27];

  int nav = n>>1;
  register PIX_TYPE *p_mk, *p_im, *p_end, *pmask, *pmark;
  register int k;
  long int offset;
  int box[6];
  long int nx = GetImNx(mark);
  long int ny = GetImNy(mark);
  long int nz = GetImNz(mark);

  FIFO4 *q;

  /* create a queue */
  q = create_fifo4(100L);
  if (q == NULL){
    (void) sprintf(buf, "uc_rdil(): not enough memory"); errputstr(buf);
    return ERROR;
  }

  if (GetImNy(mark) == 1)
    {BOX_1D;}
  else if (GetImNz(mark) == 1)
    {BOX_2D;}
  else
    {BOX_3D;}

  if (flag){
    generic_addframebox(mask, box, PIX_MIN);
    generic_addframebox(mark, box, PIX_MIN);
  }
  else{
    generic_framebox(mask, box, PIX_MIN);
    generic_framebox(mark, box, PIX_MIN);
  }

  nx = GetImNx(mark);
  ny = GetImNy(mark);
  nz = GetImNz(mark);

  /* take graph into account */
  if (set_seq_shift(GetImNx(mark), GetImNy(mark), GetImNz(mark), n, shft) == ERROR){
      free_fifo4(q);
      return ERROR;
    }

  /*  Here we go!   (Until stability)  */
  pmask=(PIX_TYPE *)GetImPtr(mask);
  pmark=(PIX_TYPE *)GetImPtr(mark);

  /*  Forward scan  */
  p_im  = (PIX_TYPE *)GetImPtr(mask) + get_offset_first_pixel(nx, ny, nz, n);
  p_mk  = (PIX_TYPE *)GetImPtr(mark) + get_offset_first_pixel(nx, ny, nz, n);
  p_end = (PIX_TYPE *)GetImPtr(mark) + get_offset_last_pixel(nx, ny, nz, n) + 1;
  for (; p_mk < p_end; p_mk++, p_im++){
      for (k = 0; k < nav; k++){
	if (*p_mk < *(p_mk + shft[k]))
	  *p_mk = *(p_mk + shft[k]);
      }
      if (*p_im < *p_mk)
	*p_mk = *p_im;
  }

  /*  Backward scan  */
  p_im  = (PIX_TYPE *)GetImPtr(mask) + get_offset_last_pixel(nx, ny, nz, n);
  p_mk  = (PIX_TYPE *)GetImPtr(mark) + get_offset_last_pixel(nx, ny, nz, n);
  p_end = (PIX_TYPE *)GetImPtr(mark) + get_offset_first_pixel(nx, ny, nz, n) - 1;
  for (; p_mk > p_end; --p_mk, --p_im){
    for (k = nav; k < n; k++){
      if (*p_mk < *(p_mk + shft[k]))
	*p_mk = *(p_mk + shft[k]);
    }
    if (*p_im < *p_mk)
      *p_mk = *p_im;
    for (k = nav; k < n; k++){
      if ( (*(p_mk + shft[k]) < *p_mk) && (*(p_mk + shft[k]) < *(p_im + shft[k])) ){
	fifo4_add(q,(long int)(p_im-pmask)); /* offset to origin in queue */
	break;
      }
    }
  }

  while ((offset=fifo4_remove(q)) != (long int) NULL){
    p_im=pmask+offset;
    p_mk=pmark+offset;
    for (k = 0; k < n; k++){
      if ( (*(p_mk+shft[k]) < *p_mk) && (*(p_im+shft[k]) != *(p_mk+shft[k])) ){
	*(p_mk+shft[k])=MIN(*p_mk,*(p_im+shft[k]));
	fifo4_add(q,offset+shft[k]);
      }
    }
  }
  free_fifo4(q);
  if (flag){
    subframebox(mask, box);
    subframebox(mark, box);
  }
  else{
    generic_framebox(mask, box, PIX_MIN);
    generic_framebox(mark, box, PIX_MIN);
  }
  return(NO_ERROR);
}
#include "uc_undef.h"


#include "us_def.h"
ERROR_TYPE us_rdil(IMAGE *mark, IMAGE *mask, int n, int flag)
{
  /*
  ** author: P. Soille 2003-07-02
  ** mark: Marker Image
  ** mask: Mask Image (mask <= mark)
  ** shft: array of offsets to neighbours
  ** n: number of neighbours

  ** comment: The transformed image is written in the marker image.
  */

  long int shft[27];

  int nav = n>>1;
  register PIX_TYPE *p_mk, *p_im, *p_end, *pmask, *pmark;
  register int k;
  long int offset;
  int box[6];
  long int nx = GetImNx(mark);
  long int ny = GetImNy(mark);
  long int nz = GetImNz(mark);

  FIFO4 *q;

  /* create a queue */
  q = create_fifo4(100L);
  if (q == NULL){
    (void) sprintf(buf, "uc_rdil(): not enough memory"); errputstr(buf);
    return ERROR;
  }

  if (GetImNy(mark) == 1)
    {BOX_1D;}
  else if (GetImNz(mark) == 1)
    {BOX_2D;}
  else
    {BOX_3D;}

  if (flag){
    us_addframebox(mask, box, PIX_MIN);
    us_addframebox(mark, box, PIX_MIN);
  }
  else{
    us_framebox(mask, box, PIX_MIN);
    us_framebox(mark, box, PIX_MIN);
  }

  nx = GetImNx(mark);
  ny = GetImNy(mark);
  nz = GetImNz(mark);

  /* take graph into account */
  if (set_seq_shift(GetImNx(mark), GetImNy(mark), GetImNz(mark), n, shft) == ERROR){
      free_fifo4(q);
      return ERROR;
    }

  /*  Here we go!   (Until stability)  */
  pmask=(PIX_TYPE *)GetImPtr(mask);
  pmark=(PIX_TYPE *)GetImPtr(mark);

  /*  Forward scan  */
  p_im  = (PIX_TYPE *)GetImPtr(mask) + get_offset_first_pixel(nx, ny, nz, n);
  p_mk  = (PIX_TYPE *)GetImPtr(mark) + get_offset_first_pixel(nx, ny, nz, n);
  p_end = (PIX_TYPE *)GetImPtr(mark) + get_offset_last_pixel(nx, ny, nz, n) + 1;
  for (; p_mk < p_end; p_mk++, p_im++){
    for (k = 0; k < nav; k++){
      if (*p_mk < *(p_mk + shft[k]))
	*p_mk = *(p_mk + shft[k]);
    }
    if (*p_im < *p_mk)
      *p_mk = *p_im;
  }

  /*  Backward scan  */
  p_im  = (PIX_TYPE *)GetImPtr(mask) + get_offset_last_pixel(nx, ny, nz, n);
  p_mk  = (PIX_TYPE *)GetImPtr(mark) + get_offset_last_pixel(nx, ny, nz, n);
  p_end = (PIX_TYPE *)GetImPtr(mark) + get_offset_first_pixel(nx, ny, nz, n) - 1;
  for (; p_mk > p_end; --p_mk, --p_im){
    for (k = nav; k < n; k++){
      if (*p_mk < *(p_mk + shft[k]))
	*p_mk = *(p_mk + shft[k]);
    }
    if (*p_im < *p_mk)
      *p_mk = *p_im;
    for (k = nav; k < n; k++){
      if ( (*(p_mk + shft[k]) < *p_mk) && (*(p_mk + shft[k]) < *(p_im + shft[k])) ){
	fifo4_add(q,(long int)(p_im-pmask)); /* offset to origin in queue */
	break;
      }
    }
  }

  while ((offset=fifo4_remove(q)) != (long int) NULL){
    p_im=pmask+offset;
    p_mk=pmark+offset;
    for (k = 0; k < n; k++){
      if ( (*(p_mk+shft[k]) < *p_mk) && (*(p_im+shft[k]) != *(p_mk+shft[k])) ){
	*(p_mk+shft[k])=MIN(*p_mk,*(p_im+shft[k]));
	fifo4_add(q,offset+shft[k]);
      }
    }
  }
  free_fifo4(q);
  if (flag){
    subframebox(mask, box);
    subframebox(mark, box);
  }
  else{
    us_framebox(mask, box, PIX_MIN);
    us_framebox(mark, box, PIX_MIN);
  }
  return(NO_ERROR);
}
#include "us_undef.h"


#include "i32_def.h"
ERROR_TYPE i32_rdil(IMAGE *mark, IMAGE *mask, int n, int flag)
{
  /*
  ** author: P. Soille 2003-07-02
  ** mark: Marker Image
  ** mask: Mask Image (mask <= mark)
  ** shft: array of offsets to neighbours
  ** n: number of neighbours

  ** comment: The transformed image is written in the marker image.
  */

  long int shft[27];

  int nav = n>>1;
  register PIX_TYPE *p_mk, *p_im, *p_end, *pmask, *pmark;
  register int k;
  long int offset;
  int box[6];
  long int nx = GetImNx(mark);
  long int ny = GetImNy(mark);
  long int nz = GetImNz(mark);

  FIFO4 *q;

  /* create a queue */
  q = create_fifo4(100L);
  if (q == NULL){
    (void) sprintf(buf, "uc_rdil(): not enough memory"); errputstr(buf);
    return ERROR;
  }

  if (GetImNy(mark) == 1)
    {BOX_1D;}
  else if (GetImNz(mark) == 1)
    {BOX_2D;}
  else
    {BOX_3D;}

  if (flag){
    i32_addframebox(mask, box, PIX_MIN);
    i32_addframebox(mark, box, PIX_MIN);
  }
  else{
    i32_framebox(mask, box, PIX_MIN);
    i32_framebox(mark, box, PIX_MIN);
  }

  nx = GetImNx(mark);
  ny = GetImNy(mark);
  nz = GetImNz(mark);

  /* take graph into account */
  if (set_seq_shift(GetImNx(mark), GetImNy(mark), GetImNz(mark), n, shft) == ERROR){
      free_fifo4(q);
      return ERROR;
    }

  /*  Here we go!   (Until stability)  */
  pmask=(PIX_TYPE *)GetImPtr(mask);
  pmark=(PIX_TYPE *)GetImPtr(mark);

  /*  Forward scan  */
  p_im  = (PIX_TYPE *)GetImPtr(mask) + get_offset_first_pixel(nx, ny, nz, n);
  p_mk  = (PIX_TYPE *)GetImPtr(mark) + get_offset_first_pixel(nx, ny, nz, n);
  p_end = (PIX_TYPE *)GetImPtr(mark) + get_offset_last_pixel(nx, ny, nz, n) + 1;
  for (; p_mk < p_end; p_mk++, p_im++){
    for (k = 0; k < nav; k++){
      if (*p_mk < *(p_mk + shft[k]))
	*p_mk = *(p_mk + shft[k]);
    }
    if (*p_im < *p_mk)
      *p_mk = *p_im;
  }

  /*  Backward scan  */
  p_im  = (PIX_TYPE *)GetImPtr(mask) + get_offset_last_pixel(nx, ny, nz, n);
  p_mk  = (PIX_TYPE *)GetImPtr(mark) + get_offset_last_pixel(nx, ny, nz, n);
  p_end = (PIX_TYPE *)GetImPtr(mark) + get_offset_first_pixel(nx, ny, nz, n) - 1;
  for (; p_mk > p_end; --p_mk, --p_im){
    for (k = nav; k < n; k++){
      if (*p_mk < *(p_mk + shft[k]))
	*p_mk = *(p_mk + shft[k]);
    }
    if (*p_im < *p_mk)
      *p_mk = *p_im;
    for (k = nav; k < n; k++){
      if ( (*(p_mk + shft[k]) < *p_mk) && (*(p_mk + shft[k]) < *(p_im + shft[k])) ){
	fifo4_add(q,(long int)(p_im-pmask)); /* offset to origin in queue */
	break;
      }
    }
  }

  while ( (offset=fifo4_remove(q)) != (long int) NULL){
    p_im=pmask+offset;
    p_mk=pmark+offset;
    for (k = 0; k < n; k++){
      if ( (*(p_mk+shft[k]) < *p_mk) && (*(p_im+shft[k]) != *(p_mk+shft[k])) ){
	*(p_mk+shft[k])=MIN(*p_mk,*(p_im+shft[k]));
	fifo4_add(q,offset+shft[k]);
      }
    }
  }
  free_fifo4(q);
  if (flag){
    subframebox(mask, box);
    subframebox(mark, box);
  }
  else{
    i32_framebox(mask, box, PIX_MIN);
    i32_framebox(mark, box, PIX_MIN);
  }
  return(NO_ERROR);
}
#include "i32_undef.h"


#include "uc_def.h"
ERROR_TYPE uc_rdilpdownhill(IMAGE *im_mark, IMAGE *im_mask, int n, int flag)
{
  /* \cite{robinson-whelan2004} */
  long int ix,iy,ox,oy,offset;
  int currentQ,currentP;
  int width,height,pixPerImg;
  int val1,val2,maxVal=0;
  int *istart,*irev,*ifwd;
  PIX_TYPE *mask, *marker;

  width=GetImNx(im_mask);
  height=GetImNy(im_mask);
  pixPerImg=width*height;
  mask=(PIX_TYPE *)GetImPtr(im_mask);
  marker=(PIX_TYPE *)GetImPtr(im_mark);

  for (offset = pixPerImg-1; offset >= 0; offset--)
    if (marker[offset] > maxVal)
      maxVal = marker[offset];
  istart = (int*)malloc((maxVal+pixPerImg*2)*sizeof(int));
  irev = istart+maxVal;
  ifwd = irev+pixPerImg;
  for (offset = -maxVal; offset < 0; offset++)
    irev[offset] = offset;
  for (offset = pixPerImg-1; offset >= 0; offset--){
    if (marker[offset] > 0){
      val1 = -marker[offset];
      irev[offset] = val1;
      ifwd[offset] = irev[val1];
      irev[val1] = offset;
      if (ifwd[offset] >= 0)
        irev[ifwd[offset]] = offset;
    }
  }
  for (currentQ = -maxVal; currentQ < 0; currentQ++){
    currentP = irev[currentQ];
    while (currentP >= 0){
      irev[currentQ] = ifwd[currentP];
      irev[currentP] = currentQ;
      ix = currentP%width;
      iy = currentP/width;
      for (oy = iy-1; oy <= iy+1; oy++){
        for (ox = ix-1; ox <= ix+1; ox++){
          if (ox >= 0 && oy >= 0 && ox < width && oy < height ){
            offset = ox+oy*width;
            val1 = marker[offset];
            val2 = marker[currentP]<mask[offset]?marker[currentP]:mask[offset];
            if (val1 < val2){
              if (val1 != 0){
		if (irev[offset]<0)  // Bug solved on 2007-10-25
		  irev[irev[offset]]=ifwd[offset];
		else
		  ifwd[irev[offset]] = ifwd[offset];
                if (ifwd[offset] >= 0)
                  irev[ifwd[offset]] = irev[offset];
              }
              marker[offset] = val2;
              irev[offset] = -val2;
              ifwd[offset] = irev[-val2];
              irev[-val2] = offset;
              if (ifwd[offset] >= 0)
                irev[ifwd[offset]] = offset;
            }
          }
        }
      }
      currentP = irev[currentQ];
    }
  }
  free(istart);
  return NO_ERROR;
}
#include "uc_undef.h"






/*
 **  Geodesic dilation until idempotence.  Overwriting algorithm.
 **
 */
ERROR_TYPE rdil(IMAGE *mark, IMAGE *mask, int graph, int flag)
{
  /* check for possible errors */
  if (szcompat(mark, mask) != NO_ERROR){
    (void)sprintf(buf,"ERROR in rdil(): incompatible input images\n"); errputstr(buf);
    return(ERROR);
  }

  switch (GetImDataType(mark)){

  case t_UCHAR:
    return(uc_rdil(mark, mask, graph, flag));
    break;

  case t_USHORT:
    return(us_rdil(mark, mask, graph, flag));
    break;

  case t_INT32:
    return(i32_rdil(mark, mask, graph, flag));
    break;

#ifndef NO_u32_IMAGE
  case t_UINT32:
    return(u32_rdil(mark, mask, graph, flag));
    break;
#endif

  default:
    (void)sprintf(buf,"rdil(): invalid pixel type\n"); errputstr(buf);
  }
  return(ERROR);
}


#include "uc_def.h"
ERROR_TYPE uc_rero(IMAGE *mark, IMAGE *mask, int n, int flag)
{
  /*
  ** author: P. Soille 2003-07-02
  ** mark: Marker Image
  ** mask: Mask Image (mask <= mark)
  ** shft: array of offsets to neighbours
  ** n: number of neighbours
  ** flag: if 0 then overwrite border else add border

  ** comment: The transformed image is written in the marker image.
  */

  long int shft[27];

  int nav = n>>1;
  register PIX_TYPE *p_mk, *p_im, *p_end, *pmask, *pmark;
  register int k;
  long int offset;
  int box[6];
  long int nx = GetImNx(mark);
  long int ny = GetImNy(mark);
  long int nz = GetImNz(mark);

  FIFO4 *q;

  /* create a queue */
  q = create_fifo4(100L);
  if (q == NULL){
    (void) sprintf(buf, "uc_rero(): not enough memory"); errputstr(buf);
    return ERROR;
  }

  if (GetImNy(mark) == 1)
    {BOX_1D;}
  else if (GetImNz(mark) == 1)
    {BOX_2D;}
  else
    {BOX_3D;}

  if (flag){
    generic_addframebox(mask, box, PIX_MAX);
    generic_addframebox(mark, box, PIX_MAX);
  }
  else{
    generic_framebox(mask, box, PIX_MAX);
    generic_framebox(mark, box, PIX_MAX);
  }

  nx = GetImNx(mark);
  ny = GetImNy(mark);
  nz = GetImNz(mark);
  /* take graph into account */
  if (set_seq_shift(GetImNx(mark), GetImNy(mark), GetImNz(mark), n, shft) == ERROR){
      free_fifo4(q);
      return ERROR;
    }

  /*  Here we go!   (Until stability)  */
  pmask=(PIX_TYPE *)GetImPtr(mask);
  pmark=(PIX_TYPE *)GetImPtr(mark);

  /*  Forward scan  */
  p_im  = (PIX_TYPE *)GetImPtr(mask) + get_offset_first_pixel(nx, ny, nz, n);
  p_mk  = (PIX_TYPE *)GetImPtr(mark) + get_offset_first_pixel(nx, ny, nz, n);
  p_end = (PIX_TYPE *)GetImPtr(mark) + get_offset_last_pixel(nx, ny, nz, n) + 1;
  for (; p_mk < p_end; p_mk++, p_im++){
    for (k = 0; k < nav; k++){
      if (*p_mk > *(p_mk + shft[k]))
	*p_mk = *(p_mk + shft[k]);
    }
    if (*p_im > *p_mk)
      *p_mk = *p_im;
  }

  /*  Backward scan  */
  p_im  = (PIX_TYPE *)GetImPtr(mask) + get_offset_last_pixel(nx, ny, nz, n);
  p_mk  = (PIX_TYPE *)GetImPtr(mark) + get_offset_last_pixel(nx, ny, nz, n);
  p_end = (PIX_TYPE *)GetImPtr(mark) + get_offset_first_pixel(nx, ny, nz, n) - 1;
  for (; p_mk > p_end; --p_mk, --p_im){
    for (k = nav; k < n; k++){
      if (*p_mk > *(p_mk + shft[k]))
	*p_mk = *(p_mk + shft[k]);
    }
    if (*p_im > *p_mk)
      *p_mk = *p_im;
    for (k = nav; k < n; k++){
      if ( (*(p_mk + shft[k]) > *p_mk) && (*(p_mk + shft[k]) > *(p_im + shft[k])) ){
	fifo4_add(q,(long int)(p_im-pmask)); /* offset to origin in queue */
	break;
      }
    }
  }


  while ((offset=fifo4_remove(q)) != (long int) NULL){
    p_im=pmask+offset;
    p_mk=pmark+offset;
    for (k = 0; k < n; k++){
      if ( (*(p_mk+shft[k]) > *p_mk) && (*(p_im+shft[k]) != *(p_mk+shft[k])) ){
	*(p_mk+shft[k])=MAX(*p_mk,*(p_im+shft[k]));
	fifo4_add(q,offset+shft[k]);
      }
    }
  }
  free_fifo4(q);
  if (flag){
    subframebox(mask, box);
    subframebox(mark, box);
  }
  else{
    generic_framebox(mask, box, PIX_MIN);
    generic_framebox(mark, box, PIX_MIN);
  }
  return(NO_ERROR);
}
#include "uc_undef.h"


#include "us_def.h"
ERROR_TYPE us_rero(IMAGE *mark, IMAGE *mask, int n, int flag)
{
  /*
  ** author: P. Soille 2003-07-02
  ** mark: Marker Image
  ** mask: Mask Image (mask <= mark)
  ** shft: array of offsets to neighbours
  ** n: number of neighbours

  ** comment: The transformed image is written in the marker image.
  */

  long int shft[27];

  int nav = n>>1;
  register PIX_TYPE *p_mk, *p_im, *p_end, *pmask, *pmark;
  register int k;
  long int offset;
  int box[6];
  long int nx = GetImNx(mark);
  long int ny = GetImNy(mark);
  long int nz = GetImNz(mark);

  FIFO4 *q;

  /* create a queue */
  q = create_fifo4(100L);
  if (q == NULL){
    (void) sprintf(buf, "uc_rero(): not enough memory"); errputstr(buf);
    return ERROR;
  }

  if (GetImNy(mark) == 1)
    {BOX_1D;}
  else if (GetImNz(mark) == 1)
    {BOX_2D;}
  else
    {BOX_3D;}

  if (flag){
    us_addframebox(mask, box, PIX_MAX);
    us_addframebox(mark, box, PIX_MAX);
  }
  else{
    us_framebox(mask, box, PIX_MAX);
    us_framebox(mark, box, PIX_MAX);
  }

  nx = GetImNx(mark);
  ny = GetImNy(mark);
  nz = GetImNz(mark);
  /* take graph into account */
  if (set_seq_shift(GetImNx(mark), GetImNy(mark), GetImNz(mark), n, shft) == ERROR){
      free_fifo4(q);
      return ERROR;
    }

  /*  Here we go!   (Until stability)  */
  pmask=(PIX_TYPE *)GetImPtr(mask);
  pmark=(PIX_TYPE *)GetImPtr(mark);

  /*  Forward scan  */
  p_im  = (PIX_TYPE *)GetImPtr(mask) + get_offset_first_pixel(nx, ny, nz, n);
  p_mk  = (PIX_TYPE *)GetImPtr(mark) + get_offset_first_pixel(nx, ny, nz, n);
  p_end = (PIX_TYPE *)GetImPtr(mark) + get_offset_last_pixel(nx, ny, nz, n) + 1;
  for (; p_mk < p_end; p_mk++, p_im++){
    for (k = 0; k < nav; k++){
      if (*p_mk > *(p_mk + shft[k]))
	*p_mk = *(p_mk + shft[k]);
    }
    if (*p_im > *p_mk)
      *p_mk = *p_im;
  }

  /*  Backward scan  */
  p_im  = (PIX_TYPE *)GetImPtr(mask) + get_offset_last_pixel(nx, ny, nz, n);
  p_mk  = (PIX_TYPE *)GetImPtr(mark) + get_offset_last_pixel(nx, ny, nz, n);
  p_end = (PIX_TYPE *)GetImPtr(mark) + get_offset_first_pixel(nx, ny, nz, n) - 1;
  for (; p_mk > p_end; --p_mk, --p_im){
    for (k = nav; k < n; k++){
      if (*p_mk > *(p_mk + shft[k]))
	*p_mk = *(p_mk + shft[k]);
    }
    if (*p_im > *p_mk)
      *p_mk = *p_im;
    for (k = nav; k < n; k++){
      if ( (*(p_mk + shft[k]) > *p_mk) && (*(p_mk + shft[k]) > *(p_im + shft[k])) ){
	fifo4_add(q,(long int)(p_im-pmask)); /* offset to origin in queue */
	break;
      }
    }
  }

  while ((offset=fifo4_remove(q)) != (long int) NULL){
    p_im=pmask+offset;
    p_mk=pmark+offset;
    for (k = 0; k < n; k++){
      if ( (*(p_mk+shft[k]) > *p_mk) && (*(p_im+shft[k]) != *(p_mk+shft[k])) ){
	*(p_mk+shft[k])=MAX(*p_mk,*(p_im+shft[k]));
	fifo4_add(q,offset+shft[k]);
      }
    }
  }
  free_fifo4(q);
  if (flag){
    subframebox(mask, box);
    subframebox(mark, box);
  }
  else{
    us_framebox(mask, box, PIX_MIN);
    us_framebox(mark, box, PIX_MIN);
  }
  return(NO_ERROR);
}
#include "us_undef.h"


#include "i32_def.h"
ERROR_TYPE i32_rero(IMAGE *mark, IMAGE *mask, int n, int flag)
{
  /*
  ** author: P. Soille 2003-07-02
  ** mark: Marker Image
  ** mask: Mask Image (mask <= mark)
  ** shft: array of offsets to neighbours
  ** n: number of neighbours

  ** comment: The transformed image is written in the marker image.
  */

  long int shft[27];

  int nav = n>>1;
  register PIX_TYPE *p_mk, *p_im, *p_end, *pmask, *pmark;
  register int k;
  long int offset;
  int box[6];
  long int nx = GetImNx(mark);
  long int ny = GetImNy(mark);
  long int nz = GetImNz(mark);

  FIFO4 *q;

  /* create a queue */
  q = create_fifo4(100L);
  if (q == NULL){
    (void) sprintf(buf, "uc_rero(): not enough memory"); errputstr(buf);
    return ERROR;
  }

  if (GetImNy(mark) == 1)
    {BOX_1D;}
  else if (GetImNz(mark) == 1)
    {BOX_2D;}
  else
    {BOX_3D;}

  if (flag){
    i32_addframebox(mask, box, PIX_MAX);
    i32_addframebox(mark, box, PIX_MAX);
  }
  else{
    i32_framebox(mask, box, PIX_MAX);
    i32_framebox(mark, box, PIX_MAX);
  }

  nx = GetImNx(mark);
  ny = GetImNy(mark);
  nz = GetImNz(mark);
  /* take graph into account */
  if (set_seq_shift(GetImNx(mark), GetImNy(mark), GetImNz(mark), n, shft) == ERROR){
      free_fifo4(q);
      return ERROR;
    }

  /*  Here we go!   (Until stability)  */
  pmask=(PIX_TYPE *)GetImPtr(mask);
  pmark=(PIX_TYPE *)GetImPtr(mark);

  /*  Forward scan  */
  p_im  = (PIX_TYPE *)GetImPtr(mask) + get_offset_first_pixel(nx, ny, nz, n);
  p_mk  = (PIX_TYPE *)GetImPtr(mark) + get_offset_first_pixel(nx, ny, nz, n);
  p_end = (PIX_TYPE *)GetImPtr(mark) + get_offset_last_pixel(nx, ny, nz, n) + 1;
  for (; p_mk < p_end; p_mk++, p_im++){
    for (k = 0; k < nav; k++){
      if (*p_mk > *(p_mk + shft[k]))
	*p_mk = *(p_mk + shft[k]);
    }
    if (*p_im > *p_mk)
      *p_mk = *p_im;
  }

  /*  Backward scan  */
  p_im  = (PIX_TYPE *)GetImPtr(mask) + get_offset_last_pixel(nx, ny, nz, n);
  p_mk  = (PIX_TYPE *)GetImPtr(mark) + get_offset_last_pixel(nx, ny, nz, n);
  p_end = (PIX_TYPE *)GetImPtr(mark) + get_offset_first_pixel(nx, ny, nz, n) - 1;
  for (; p_mk > p_end; --p_mk, --p_im){
    for (k = nav; k < n; k++){
      if (*p_mk > *(p_mk + shft[k]))
	*p_mk = *(p_mk + shft[k]);
    }
    if (*p_im > *p_mk)
      *p_mk = *p_im;
    for (k = nav; k < n; k++){
      if ( (*(p_mk + shft[k]) > *p_mk) && (*(p_mk + shft[k]) > *(p_im + shft[k])) ){
	fifo4_add(q,(long int)(p_im-pmask)); /* offset to origin in queue */
	break;
      }
    }
  }

  while ( (offset=fifo4_remove(q)) != (long int) NULL){
    p_im=pmask+offset;
    p_mk=pmark+offset;
    for (k = 0; k < n; k++){
      if ( (*(p_mk+shft[k]) > *p_mk) && (*(p_im+shft[k]) != *(p_mk+shft[k])) ){
	*(p_mk+shft[k])=MAX(*p_mk,*(p_im+shft[k]));
	fifo4_add(q,offset+shft[k]);
      }
    }
  }
  free_fifo4(q);
  if (flag){
    subframebox(mask, box);
    subframebox(mark, box);
  }
  else{
    i32_framebox(mask, box, PIX_MIN);
    i32_framebox(mark, box, PIX_MIN);
  }
  return(NO_ERROR);
}
#include "i32_undef.h"


/*
 **  Reconstruciton by erosion using Vincent's algorithm [2003-07-02]
 **
 */
ERROR_TYPE rero(IMAGE *mark, IMAGE *mask, int graph, int flag)
{
  /* check for possible errors */
  if (szcompat(mark, mask) != NO_ERROR){
    (void)sprintf(buf,"ERROR in rero(): incompatible input images\n"); errputstr(buf);
    return(ERROR);
  }

  switch (GetImDataType(mark)){

  case t_UCHAR:
    return(uc_rero(mark, mask, graph, flag));
    break;

  case t_USHORT:
    return(us_rero(mark, mask, graph, flag));
    break;

  case t_INT32:
    return(i32_rero(mark, mask, graph, flag));
    break;


  default:
    (void)sprintf(buf,"rero(): invalid pixel type\n"); errputstr(buf);
  }
  return(ERROR);
}


#include "uc_def.h"
ERROR_TYPE uc_rerodilp(IMAGE *mark, IMAGE *mask, int n, int flag)
{
  /*
  ** author: P. Soille 2007-10-22
  ** mark: Marker Image
  ** mask: Mask Image (mask <= mark)
  ** shft: array of offsets to neighbours
  ** n: number of neighbours
  ** flag: if 0 then overwrite border else add border

  ** comment: The transformed image is written in the marker image.
  Version with a unique queue.
  */

  long int shft[27];

  int nav = n>>1;
  register PIX_TYPE *p_mk, *p_im, *p_end, *pmask, *pmark;
  register int k;
  long int offset;
  int box[6];
  long int nx = GetImNx(mark);
  long int ny = GetImNy(mark);
  long int nz = GetImNz(mark);

  FIFO4 *q;

  /* create a queue */
  q = create_fifo4(100L);
  if (q == NULL){
    (void) sprintf(buf, "uc_rdil(): not enough memory"); errputstr(buf);
    return ERROR;
  }

  if (GetImNy(mark) == 1)
    {BOX_1D;}
  else if (GetImNz(mark) == 1)
    {BOX_2D;}
  else
    {BOX_3D;}

  if (flag){
    generic_addframebox(mask, box, PIX_MIN);
    generic_addframebox(mark, box, PIX_MIN);
  }
  else{
    generic_framebox(mask, box, PIX_MIN);
    generic_framebox(mark, box, PIX_MIN);
  }

  nx = GetImNx(mark);
  ny = GetImNy(mark);
  nz = GetImNz(mark);
  /* take graph into account */
  if (set_seq_shift(GetImNx(mark), GetImNy(mark), GetImNz(mark), n, shft) == ERROR){
      free_fifo4(q);
      return ERROR;
    }

  /*  Here we go!   (Until stability)  */
  pmask=(PIX_TYPE *)GetImPtr(mask);
  pmark=(PIX_TYPE *)GetImPtr(mark);

  /*  Forward scan  */
  p_im  = (PIX_TYPE *)GetImPtr(mask) + get_offset_first_pixel(nx, ny, nz, n);
  p_mk  = (PIX_TYPE *)GetImPtr(mark) + get_offset_first_pixel(nx, ny, nz, n);
  p_end = (PIX_TYPE *)GetImPtr(mark) + get_offset_last_pixel(nx, ny, nz, n) + 1;
  for (; p_mk < p_end; p_mk++, p_im++){
    if (*p_mk<*p_im){
      for (k = 0; k < nav; k++){
	if (*p_mk <  MIN(*(p_mk + shft[k]),*(p_im + shft[k])) ) // do not jump over
	  *p_mk = MIN(*(p_mk + shft[k]),*(p_im + shft[k]));
      }
      if (*p_im < *p_mk)
	*p_mk = *p_im;
    }
    else if (*p_mk>*p_im){
      for (k = 0; k < nav; k++){
	if (*p_mk > MAX(*(p_mk + shft[k]),*(p_im + shft[k])) ) // do not jump over
	  *p_mk =  MAX(*(p_mk + shft[k]),*(p_im + shft[k]));
      }
      if (*p_im > *p_mk)
	*p_mk = *p_im;
    }
  }

  /*  Backward scan  */
  p_im  = (PIX_TYPE *)GetImPtr(mask) + get_offset_last_pixel(nx, ny, nz, n);
  p_mk  = (PIX_TYPE *)GetImPtr(mark) + get_offset_last_pixel(nx, ny, nz, n);
  p_end = (PIX_TYPE *)GetImPtr(mark) + get_offset_first_pixel(nx, ny, nz, n) - 1;
  for (; p_mk > p_end; --p_mk, --p_im){
    if (*p_mk<*p_im){
      for (k = nav; k < n; k++){
	if (*p_mk < MIN(*(p_mk + shft[k]),*(p_im + shft[k])) )  // do not jump over
	  *p_mk = MIN(*(p_mk + shft[k]),*(p_im + shft[k]));
      }
      if (*p_im < *p_mk)
	*p_mk = *p_im;
    }
    else if (*p_mk>*p_im){
      for (k = nav; k < n; k++){
	if (*p_mk > MAX(*(p_mk + shft[k]),*(p_im + shft[k])) ) // do not jump over
	  *p_mk = MAX(*(p_mk + shft[k]),*(p_im + shft[k]));
      }
      if (*p_im > *p_mk)
	*p_mk = *p_im;
    }

    for (k = nav; k < n; k++){
      if ( (*(p_mk + shft[k]) < MIN(*p_mk,*p_im)) && (*(p_mk + shft[k]) < *(p_im + shft[k])) ){
	fifo4_add(q,(long int)(p_im-pmask)); /* offset to origin in queue */
	break;
      }
      else if  ( (*(p_mk + shft[k]) > MAX(*p_mk,*p_im)) && (*(p_mk + shft[k]) > *(p_im + shft[k])) ){
	fifo4_add(q,(long int)(p_im-pmask)); /* offset to origin in queue */
	break;
      }
    }
  }

  while ((offset=fifo4_remove(q)) != (long int) NULL){
    p_im=pmask+offset;
    p_mk=pmark+offset;
    for (k=0; k<n; k++){
      if ( (*(p_mk) <= *(p_im)) && (*(p_mk+shft[k]) < *p_mk) && (*(p_mk+shft[k]) < *(p_im+shft[k])) ){
	  *(p_mk+shft[k])=MIN(*p_mk,*(p_im+shft[k]));
	  fifo4_add(q,offset+shft[k]);
	}
      else if ( (*(p_mk) >= *(p_im)) && (*(p_mk+shft[k]) > *p_mk) && (*(p_mk+shft[k]) > *(p_im+shft[k])) ){
	  *(p_mk+shft[k])=MAX(*p_mk,*(p_im+shft[k]));
	  fifo4_add(q,offset+shft[k]);
      }
    }
  }


  free_fifo4(q);
  if (flag){
    subframebox(mask, box);
    subframebox(mark, box);
  }
  else{
    generic_framebox(mask, box, PIX_MIN);
    generic_framebox(mark, box, PIX_MIN);
  }
  return(NO_ERROR);
}
#include "uc_undef.h"


#include "uc_def.h"
ERROR_TYPE uc_rerodilp2q(IMAGE *mark, IMAGE *mask, int n, int flag)
{
  /*
  ** author: P. Soille 2007-10-22
  ** mark: Marker Image
  ** mask: Mask Image (mask <= mark)
  ** shft: array of offsets to neighbours
  ** n: number of neighbours
  ** flag: if 0 then overwrite border else add border

  ** comment: The transformed image is written in the marker image.  Version
  with two queues, one for each reconstruction.
  */

  long int shft[27];

  int nav = n>>1;
  register PIX_TYPE *p_mk, *p_im, *p_end, *pmask, *pmark;
  register int k;
  long int offset;
  int box[6];
  long int nx = GetImNx(mark);
  long int ny = GetImNy(mark);
  long int nz = GetImNz(mark);

  FIFO4 *qero, *qdil;

  /* create a queue */
  qero = create_fifo4(100L);
  if (qero == NULL){
    (void) sprintf(buf, "uc_rdil(): not enough memory"); errputstr(buf);
    return ERROR;
  }
  qdil = create_fifo4(100L);
  if (qdil == NULL){
    free_fifo4(qero);
    (void) sprintf(buf, "uc_rdil(): not enough memory"); errputstr(buf);
    return ERROR;
  }

  if (GetImNy(mark) == 1)
    {BOX_1D;}
  else if (GetImNz(mark) == 1)
    {BOX_2D;}
  else
    {BOX_3D;}

  if (flag){
    generic_addframebox(mask, box, PIX_MIN);
    generic_addframebox(mark, box, PIX_MIN);
  }
  else{
    generic_framebox(mask, box, PIX_MIN);
    generic_framebox(mark, box, PIX_MIN);
  }

  nx = GetImNx(mark);
  ny = GetImNy(mark);
  nz = GetImNz(mark);
  /* take graph into account */
  if (set_seq_shift(GetImNx(mark), GetImNy(mark), GetImNz(mark), n, shft) == ERROR){
      free_fifo4(qero);
      free_fifo4(qdil);
      return ERROR;
    }

  /*  Here we go!   (Until stability)  */
  pmask=(PIX_TYPE *)GetImPtr(mask);
  pmark=(PIX_TYPE *)GetImPtr(mark);

  /*  Forward scan  */
  p_im  = (PIX_TYPE *)GetImPtr(mask) + get_offset_first_pixel(nx, ny, nz, n);
  p_mk  = (PIX_TYPE *)GetImPtr(mark) + get_offset_first_pixel(nx, ny, nz, n);
  p_end = (PIX_TYPE *)GetImPtr(mark) + get_offset_last_pixel(nx, ny, nz, n) + 1;
  for (; p_mk < p_end; p_mk++, p_im++){
    if (*p_mk<*p_im){
      for (k = 0; k < nav; k++){
	if (*p_mk <  MIN(*(p_mk + shft[k]),*(p_im + shft[k])) ) // do not jump over
	  *p_mk = MIN(*(p_mk + shft[k]),*(p_im + shft[k]));
      }
      if (*p_im < *p_mk)
	*p_mk = *p_im;
    }
    else if (*p_mk>*p_im){
      for (k = 0; k < nav; k++){
	if (*p_mk > MAX(*(p_mk + shft[k]),*(p_im + shft[k])) ) // do not jump over
	  *p_mk =  MAX(*(p_mk + shft[k]),*(p_im + shft[k]));
      }
      if (*p_im > *p_mk)
	*p_mk = *p_im;
    }
  }

  /*  Backward scan  */
  p_im  = (PIX_TYPE *)GetImPtr(mask) + get_offset_last_pixel(nx, ny, nz, n);
  p_mk  = (PIX_TYPE *)GetImPtr(mark) + get_offset_last_pixel(nx, ny, nz, n);
  p_end = (PIX_TYPE *)GetImPtr(mark) + get_offset_first_pixel(nx, ny, nz, n) - 1;
  for (; p_mk > p_end; --p_mk, --p_im){
    if (*p_mk<*p_im){
      for (k = nav; k < n; k++){
	if (*p_mk < MIN(*(p_mk + shft[k]),*(p_im + shft[k])) )  // do not jump over
	  *p_mk = MIN(*(p_mk + shft[k]),*(p_im + shft[k]));
      }
      if (*p_im < *p_mk)
	*p_mk = *p_im;
    }
    else if (*p_mk>*p_im){
      for (k = nav; k < n; k++){
	if (*p_mk > MAX(*(p_mk + shft[k]),*(p_im + shft[k])) ) // do not jump over
	  *p_mk = MAX(*(p_mk + shft[k]),*(p_im + shft[k]));
      }
      if (*p_im > *p_mk)
	*p_mk = *p_im;
    }

    for (k = nav; k < n; k++){
      if ( (*(p_mk + shft[k]) < MIN(*p_mk,*p_im)) && (*(p_mk + shft[k]) < *(p_im + shft[k])) ){
	fifo4_add(qdil,(long int)(p_im-pmask)); /* offset to origin in queue */
	break;
      }
      if ( (*(p_mk + shft[k]) > MAX(*p_mk,*p_im)) && (*(p_mk + shft[k]) > *(p_im + shft[k])) ){
	fifo4_add(qero,(long int)(p_im-pmask)); /* offset to origin in queue */
	break;
      }
    }
  }

  while ((offset=fifo4_remove(qero)) != (long int) NULL){
    p_im=pmask+offset;
    p_mk=pmark+offset;
    for (k=0; k<n; k++){
      if ( (*(p_mk+shft[k]) > *p_mk) && (*(p_mk+shft[k]) > *(p_im+shft[k])) ){
	  *(p_mk+shft[k])=MAX(*p_mk,*(p_im+shft[k]));
	  fifo4_add(qero,offset+shft[k]);
      }
    }
  }

  while ((offset=fifo4_remove(qdil)) != (long int) NULL){
    p_im=pmask+offset;
    p_mk=pmark+offset;
    for (k=0; k<n; k++){
      if ( (*(p_mk+shft[k]) < *p_mk) && (*(p_mk+shft[k]) < *(p_im+shft[k])) ){
	  *(p_mk+shft[k])=MIN(*p_mk,*(p_im+shft[k]));
	  fifo4_add(qdil,offset+shft[k]);
      }
    }
  }

  free_fifo4(qero);
  free_fifo4(qdil);
  if (flag){
    subframebox(mask, box);
    subframebox(mark, box);
  }
  else{
    generic_framebox(mask, box, PIX_MIN);
    generic_framebox(mark, box, PIX_MIN);
  }
  return(NO_ERROR);
}
#include "uc_undef.h"


#include "uc_def.h"
ERROR_TYPE uc_rerodilp2pq(IMAGE *mark, IMAGE *mask, int n, int flag)
{
  /*
  ** author: P. Soille 2007-10-22
  ** mark: Marker Image
  ** mask: Mask Image (mask <= mark)
  ** shft: array of offsets to neighbours
  ** n: number of neighbours
  ** flag: if 0 then overwrite border else add border

  ** comment: The transformed image is written in the marker image.  Version
  with two priority queues, one for each reconstruction.
  */

  long int shft[27];

  int nav = n>>1;
  register PIX_TYPE *p_mk, *p_im, *p_end, *pmask, *pmark;
  register int k;
  long int offset;
  int box[6];
  long int nx = GetImNx(mark);
  long int ny = GetImNy(mark);
  long int nz = GetImNz(mark);

  struct pqueue *pqero, *pqdil;
  PQDATUM apqd[1];
  struct node *pqd;

  /* create 2 priority queues */
  pqero = (struct pqueue *)pqinit(NULL, 100);  /* priority queue */
  if (pqero == NULL)
    return ERROR;
  pqdil = (struct pqueue *)pqinit(NULL, 100);  /* priority queue */
  if (pqdil == NULL){
    free_pq(pqero);
    return ERROR;
  }

  if (GetImNy(mark) == 1)
    {BOX_1D;}
  else if (GetImNz(mark) == 1)
    {BOX_2D;}
  else
    {BOX_3D;}

  if (flag){
    generic_addframebox(mask, box, PIX_MIN);
    generic_addframebox(mark, box, PIX_MIN);
  }
  else{
    generic_framebox(mask, box, PIX_MIN);
    generic_framebox(mark, box, PIX_MIN);
  }

  nx = GetImNx(mark);
  ny = GetImNy(mark);
  nz = GetImNz(mark);
  /* take graph into account */
  if (set_seq_shift(GetImNx(mark), GetImNy(mark), GetImNz(mark), n, shft) == ERROR){
    free_pq(pqero);
    free_pq(pqdil);
    return ERROR;
  }

  /*  Here we go!   (Until stability)  */
  pmask=(PIX_TYPE *)GetImPtr(mask);
  pmark=(PIX_TYPE *)GetImPtr(mark);

  /*  Forward scan  */
  p_im  = (PIX_TYPE *)GetImPtr(mask) + get_offset_first_pixel(nx, ny, nz, n);
  p_mk  = (PIX_TYPE *)GetImPtr(mark) + get_offset_first_pixel(nx, ny, nz, n);
  p_end = (PIX_TYPE *)GetImPtr(mark) + get_offset_last_pixel(nx, ny, nz, n) + 1;
  for (; p_mk < p_end; p_mk++, p_im++){
    if (*p_mk<*p_im){
      for (k = 0; k < nav; k++){
	if (*p_mk <  MIN(*(p_mk + shft[k]),*(p_im + shft[k])) ) // do not jump over
	  *p_mk = MIN(*(p_mk + shft[k]),*(p_im + shft[k]));
      }
      if (*p_im < *p_mk)
	*p_mk = *p_im;
    }
    else if (*p_mk>*p_im){
      for (k = 0; k < nav; k++){
	if (*p_mk > MAX(*(p_mk + shft[k]),*(p_im + shft[k])) ) // do not jump over
	  *p_mk =  MAX(*(p_mk + shft[k]),*(p_im + shft[k]));
      }
      if (*p_im > *p_mk)
	*p_mk = *p_im;
    }
  }

  /*  Backward scan  */
  p_im  = (PIX_TYPE *)GetImPtr(mask) + get_offset_last_pixel(nx, ny, nz, n);
  p_mk  = (PIX_TYPE *)GetImPtr(mark) + get_offset_last_pixel(nx, ny, nz, n);
  p_end = (PIX_TYPE *)GetImPtr(mark) + get_offset_first_pixel(nx, ny, nz, n) - 1;
  for (; p_mk > p_end; --p_mk, --p_im){
    if (*p_mk<*p_im){
      for (k = nav; k < n; k++){
	if (*p_mk < MIN(*(p_mk + shft[k]),*(p_im + shft[k])) )  // do not jump over
	  *p_mk = MIN(*(p_mk + shft[k]),*(p_im + shft[k]));
      }
      if (*p_im < *p_mk)
	*p_mk = *p_im;
    }
    else if (*p_mk>*p_im){
      for (k = nav; k < n; k++){
	if (*p_mk > MAX(*(p_mk + shft[k]),*(p_im + shft[k])) ) // do not jump over
	  *p_mk = MAX(*(p_mk + shft[k]),*(p_im + shft[k]));
      }
      if (*p_im > *p_mk)
	*p_mk = *p_im;
    }

    for (k = nav; k < n; k++){
      if ( (*(p_mk + shft[k]) < MIN(*p_mk,*p_im)) && (*(p_mk + shft[k]) < *(p_im + shft[k])) ){
	pqd = (PQDATUM )malloc(sizeof(struct node));
	pqd->prio = *(p_mk + shft[k]);
	//pqd->val   = val;
	pqd->offset= (long int)(p_im-pmask);
	pqmininsert(pqdil, pqd);
	break;
      }
      if ( (*(p_mk + shft[k]) > MAX(*p_mk,*p_im)) && (*(p_mk + shft[k]) > *(p_im + shft[k])) ){
	pqd = (PQDATUM )malloc(sizeof(struct node));
	pqd->prio = *(p_mk + shft[k]);
	//pqd->val   = val;
	pqd->offset= (long int)(p_im-pmask);
	pqmaxinsert(pqero, pqd);
	break;
      }
    }
  }
  while ( pqpeek(pqero, apqd) != NULL ){
    pqminremove(pqero, apqd);
    offset=(*apqd)->offset;
    p_im=pmask+offset;
    p_mk=pmark+offset;
    for (k=0; k<n; k++){
      if ( (*(p_mk+shft[k]) > *p_mk) && (*(p_mk+shft[k]) > *(p_im+shft[k])) ){
	*(p_mk+shft[k])=MAX(*p_mk,*(p_im+shft[k]));
	pqd = (PQDATUM )malloc(sizeof(struct node));
	pqd->prio = *(p_mk + shft[k]);
	//pqd->val   = val;
	pqd->offset= offset+shft[k];
	pqmininsert(pqero,pqd);
      }
    }
    free((char*) *apqd);
  }

  while ( pqpeek(pqdil, apqd) != NULL ){
    pqmaxremove(pqdil, apqd);
    offset=(*apqd)->offset;
    p_im=pmask+offset;
    p_mk=pmark+offset;
    for (k=0; k<n; k++){
      if ( (*(p_mk+shft[k]) < *p_mk) && (*(p_mk+shft[k]) < *(p_im+shft[k])) ){
	*(p_mk+shft[k])=MIN(*p_mk,*(p_im+shft[k]));
	pqd = (PQDATUM )malloc(sizeof(struct node));
	pqd->prio = *(p_mk + shft[k]);
	//pqd->val   = val;
	pqd->offset= offset+shft[k];
	pqmaxinsert(pqdil,pqd);
      }
    }
    free((char*) *apqd);
  }

  free_pq(pqero);
  free_pq(pqdil);

  if (flag){
    subframebox(mask, box);
    subframebox(mark, box);
  }
  else{
    generic_framebox(mask, box, PIX_MIN);
    generic_framebox(mark, box, PIX_MIN);
  }
  return(NO_ERROR);
}
#include "uc_undef.h"



#include "uc_def.h"
ERROR_TYPE uc_rerodilpdownhill(IMAGE *im_mark, IMAGE *im_mask, int n, int flag)
{
  /* based on rdildownhill, see also \cite{robinson-whelan2004} */
  long int ix,iy,ox,oy,offset;
  long int currentQ,currentP;
  long int width,height,pixPerImg;
  int val1,val2,maxVal=0;
  int *istart,*irev,*ifwd;
  PIX_TYPE *mask, *marker;

  width=GetImNx(im_mask);
  height=GetImNy(im_mask);
  pixPerImg=width*height;
  mask=(PIX_TYPE *)GetImPtr(im_mask);
  marker=(PIX_TYPE *)GetImPtr(im_mark);

  for (offset = pixPerImg-1; offset >= 0; offset--)
    if (marker[offset] > maxVal)
      maxVal = marker[offset];
  istart = (int*)malloc((maxVal+pixPerImg*2)*sizeof(int));
  irev = istart+maxVal;
  ifwd = irev+pixPerImg;
  for (offset = -maxVal; offset < 0; offset++)
    irev[offset] = offset;
  for (offset = pixPerImg-1; offset >= 0; offset--){
    if (marker[offset] > 0){
      val1 = -marker[offset];
      irev[offset] = val1;
      ifwd[offset] = irev[val1];
      irev[val1] = offset;
      if (ifwd[offset] >= 0)
        irev[ifwd[offset]] = offset;
    }
  }
  for (currentQ = -maxVal; currentQ < 0; currentQ++){
    currentP = irev[currentQ];
    while (currentP >= 0){
      irev[currentQ] = ifwd[currentP];
      irev[currentP] = currentQ;
      ix = currentP%width;
      iy = currentP/width;
      for (oy = iy-1; oy <= iy+1; oy++){
        for (ox = ix-1; ox <= ix+1; ox++){
          if (ox >= 0 && oy >= 0 && ox < width && oy < height ){
            offset = ox+oy*width;
            val1 = marker[offset];
            val2 = marker[currentP]<mask[offset]?marker[currentP]:mask[offset];
            if (val1 < val2){
              if (val1 != 0){
		if (irev[offset]<0)  /* Bug solved on 2007-10-25 */
		  irev[irev[offset]]=ifwd[offset];
		else
		  ifwd[irev[offset]] = ifwd[offset];
                if (ifwd[offset] >= 0)
                  irev[ifwd[offset]] = irev[offset];
              }
              marker[offset] = val2;
              irev[offset] = -val2;
              ifwd[offset] = irev[-val2];
              irev[-val2] = offset;
              if (ifwd[offset] >= 0)
                irev[ifwd[offset]] = offset;
            }
          }
        }
      }
      currentP = irev[currentQ];
    }
  }
  free(istart);
  return NO_ERROR;
}
#include "uc_undef.h"




/*
 **  Self-dual reconstruction following alternative definition of Soille
 **  (no jump over).
 **
 */
ERROR_TYPE rerodilp(IMAGE *mark, IMAGE *mask, int graph, int flag, int version)
{
  /* check for possible errors */
  if (szcompat(mark, mask) != NO_ERROR){
    (void)sprintf(buf,"ERROR in rerodilp(): incompatible input images\n");
    errputstr(buf);
    return(ERROR);
  }

  switch (GetImDataType(mark)){

  case t_UCHAR:
    switch (version){
    case 1:
       return(uc_rerodilp(mark, mask, graph, flag));
       break;
    case 2:
       return(uc_rerodilp2q(mark, mask, graph, flag));
       break;
    case 3:
       return(uc_rerodilp2pq(mark, mask, graph, flag));
       break;
    case 4:
       return(uc_rerodilpdownhill(mark, mask, graph, flag));
       break;
    }
    break;
  default:
    (void)sprintf(buf,"rerodilp(): invalid pixel type\n"); errputstr(buf);
  }
  return(ERROR);
}

/*@}*/
