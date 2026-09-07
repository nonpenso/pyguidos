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

/*
**
**  Begin: August 30, 2002.
*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "miallib.h"
#include "op.h"


#if (defined(XLISP))
extern void gc();
#endif

/** \defgroup group_geom Image geometry and frames
 *  Functions related to the image geometry.
 *  @{
 */

#ifndef NO_generic_IMAGE
#include "g_def.h"
void generic_imcut(IMAGE *imin, IMAGE *imout, int x1, int y1, int z1, int x2, int y2, int z2)
{
  long int nx, ny, nnxb, nnx, nny, nnz, i, j, offset;
  PIX_TYPE *p, *pi, *po;

  nx = GetImNx(imin);
  ny = GetImNy(imin);

  nnxb = (x2-x1+1)*sizeof(PIX_TYPE);
  nnx  = (x2-x1+1);
  nny = y2-y1+1;
  nnz = z2-z1+1;

  pi = (PIX_TYPE *)GetImPtr(imin);
  po = (PIX_TYPE *)GetImPtr(imout);

  for (i=0; i < nnz; i++){
    offset = (z1+i) * nx * ny + x1 + nx * y1;
    for (j=0; j < nny; j++){
      p = pi + offset + nx * j;
      memcpy((void *)po, (void *)p, nnxb);
      po += nnx;
    }
  }
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */

#include "us_def.h"
void us_imcut(IMAGE *imin, IMAGE *imout, int x1, int y1, int z1, int x2, int y2, int z2)
{
  long int nx, ny, nnxb, nnx, nny, nnz, i, j, offset;
  PIX_TYPE *p, *pi, *po;

  nx = GetImNx(imin);
  ny = GetImNy(imin);

  nnxb = (x2-x1+1)*sizeof(PIX_TYPE);
  nnx  = (x2-x1+1);
  nny  = y2-y1+1;
  nnz  = z2-z1+1;

  pi = (PIX_TYPE *)GetImPtr(imin);
  po = (PIX_TYPE *)GetImPtr(imout);

  for (i=0; i < nnz; i++){
    offset = (z1+i) * nx * ny + x1 + nx * y1;
    for (j=0; j < nny; j++){
      p  = pi + offset + nx * j;
      memcpy((void *)po, (void *)p, nnxb);
      po += nnx;
    }
  }
}
#include "us_undef.h"


#include "i32_def.h"
void i32_imcut(IMAGE *imin, IMAGE *imout, int x1, int y1, int z1, int x2, int y2, int z2)
{
  long int nx, ny, nnxb, nnx, nny, nnz, i, j, offset;
  PIX_TYPE *p, *pi, *po;

  nx = GetImNx(imin);
  ny = GetImNy(imin);

  nnxb = (x2-x1+1)*sizeof(PIX_TYPE);
  nnx  = (x2-x1+1);
  nny  = y2-y1+1;
  nnz  = z2-z1+1;

  pi = (PIX_TYPE *)GetImPtr(imin);
  po = (PIX_TYPE *)GetImPtr(imout);

  for (i=0; i < nnz; i++){
    offset = (z1+i) * nx * ny + x1 + nx * y1;
    for (j=0; j < nny; j++){
      p  = pi + offset + nx * j;
      memcpy((void *)po, (void *)p, nnxb);
      po += nnx;
    }
  }
}
#include "i32_undef.h"

/* cut an image  */
/* im: pointer to the I/O image  */
/* x1 y1 z1: coordinates of the bottom upper left corner of the box */
/* x2 y2 z2: coordinates of the top lower right corner of the box */


IMAGE *imcut(IMAGE *im, int x1, int y1, int z1, int x2, int y2, int z2)
{
  IMAGE *imout;

  /* check the validity of input parmaters */
  if (x1 < 0 || y1 < 0 || z1 < 0 ||
      x2 >= GetImNx(im) || y2 >= GetImNy(im) || z2 >= GetImNz(im) ||
      x2 < x1 || y2 < y1 || z2 < z1){
    (void)sprintf(buf, "cut(): invalid parameters\n"); errputstr(buf);
    return(NULL);
  }

  /* create output image */
  imout = create_image(GetImDataType(im), x2-x1+1, y2-y1+1, z2-z1+1);
  if (imout == NULL){
    (void)sprintf(buf,"cut(): not enough memory!\n"); errputstr(buf);
    return(NULL);
  }


  switch (GetImDataType(im)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    generic_imcut(im, imout, x1, y1, z1, x2, y2, z2);
    break;
#endif

#ifndef NO_uc_IMAGE
  case t_UCHAR:
    uc_imcut(im, imout, x1, y1, z1, x2, y2, z2);
    break;
#endif

  case t_SHORT:
  case t_USHORT:
    us_imcut(im, imout, x1, y1, z1, x2, y2, z2);
    break;

  case t_UINT32:
  case t_INT32:
  case t_FLOAT:
    i32_imcut(im, imout, x1, y1, z1, x2, y2, z2);
    break;

#ifndef NO_d_IMAGE
  case t_DOUBLE:
    d_imcut(im, imout, x1, y1, z1, x2, y2, z2);
    break;
#endif

  default:
    (void)sprintf(buf,"cut(): invalid pixel type\n"); errputstr(buf);
    free_image(imout);
    return(NULL);
  }
  return(imout);
}


#include "uc_def.h"
IMAGE *uc_getframebox(IMAGE *im, int *box)
{
  long int nx, ny, nz;
  long int x, y, z, l1, l2;
  IMAGE *imout;
  PIX_TYPE *p, *pim, *pout;

  /* check the validity of input parmaters */
  if (box[0] > GetImNx(im) || box[1] > GetImNx(im) ||
      box[2] > GetImNy(im) || box[3] > GetImNy(im) ||
      box[4] > GetImNz(im) || box[5] > GetImNz(im)){
    (void)sprintf(buf, "getframebox(): invalid parameters\n"); errputstr(buf);
    return(NULL);
  }

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);
  /* create output image */
  imout = (IMAGE *)create_image(GetImDataType(im), (box[0]+box[1])*ny*nz + (box[2]+box[3])*nx*nz + (box[4]+box[5])*nx*ny, 1, 1);
  if (imout == NULL){
    (void)sprintf(buf,"getframebox(): not enough memory!\n"); errputstr(buf);
    return(imout);
  }

  pout = (PIX_TYPE *)GetImPtr(imout);
  pim  = (PIX_TYPE *)GetImPtr(im);

  l1 = box[0]; l2 = box[1];	/* left and right borders */
  for (z = 0; z < nz; z++){
    for (y = 0; y < ny; y++){
      p = pim + z * nx * ny + y * nx;
      for (x = 0; x < l1; x++)
	*pout++ = *p++;
      p = pim + z * nx * ny + y * nx + nx - l2;
      for (x = 0; x < l2; x++)
        *pout++ = *p++;
    }
  }
  l1 = box[2] * nx; l2 = box[3] * nx;	/* top and bottom borders */
  for (z = 0; z < nz; z++){
    p = pim + z * nx * ny;
    for (x = 0; x < l1; x++)
      *pout++ = *p++;
    p = pim + z * nx * ny + nx * (ny - box[3]);
    for (x = 0; x < l2; x++)
      *pout++ = *p++;
  }

  l1 = box[4] * nx * ny; l2 = box[5] * nx * ny;	/* up and down borders */
  p = pim;
  for (x = 0; x < l1; x++)
    *pout++ = *p++;
  p = pim + nx * ny * (nz - box[5]);
  for (x = 0; x < l2; x++)
    *pout++ = *p++;
  return imout;
}
#include "uc_undef.h"

#include "us_def.h"
IMAGE *us_getframebox(IMAGE *im, int *box)
{
  long int nx, ny, nz;
  long int x, y, z, l1, l2;
  IMAGE *imout;
  PIX_TYPE *p, *pim, *pout;

  /* check the validity of input parmaters */
  if (box[0] > GetImNx(im) || box[1] > GetImNx(im) ||
      box[2] > GetImNy(im) || box[3] > GetImNy(im) ||
      box[4] > GetImNz(im) || box[5] > GetImNz(im)){
    (void)sprintf(buf, "getframebox(): invalid parameters\n"); errputstr(buf);
    return(NULL);
  }

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);
  /* create output image */
  imout = (IMAGE *)create_image(GetImDataType(im), (box[0]+box[1])*ny*nz + (box[2]+box[3])*nx*nz + (box[4]+box[5])*nx*ny, 1, 1);
  if (imout == NULL){
    (void)sprintf(buf,"getframebox(): not enough memory!\n"); errputstr(buf);
    return(imout);
  }

  pout = (PIX_TYPE *)GetImPtr(imout);
  pim  = (PIX_TYPE *)GetImPtr(im);

  l1 = box[0]; l2 = box[1];	/* left and right borders */
  for (z = 0; z < nz; z++){
    for (y = 0; y < ny; y++){
      p = pim + z * nx * ny + y * nx;
      for (x = 0; x < l1; x++)
	*pout++ = *p++;
      p = pim + z * nx * ny + y * nx + nx - l2;
      for (x = 0; x < l2; x++)
        *pout++ = *p++;
    }
  }
  l1 = box[2] * nx; l2 = box[3] * nx;	/* top and bottom borders */
  for (z = 0; z < nz; z++){
    p = pim + z * nx * ny;
    for (x = 0; x < l1; x++)
      *pout++ = *p++;
    p = pim + z * nx * ny + nx * (ny - box[3]);
    for (x = 0; x < l2; x++)
      *pout++ = *p++;
  }

  l1 = box[4] * nx * ny; l2 = box[5] * nx * ny;	/* up and down borders */
  p = pim;
  for (x = 0; x < l1; x++)
    *pout++ = *p++;
  p = pim + nx * ny * (nz - box[5]);
  for (x = 0; x < l2; x++)
    *pout++ = *p++;
  return imout;
}
#include "us_undef.h"

#include "i32_def.h"
IMAGE *i32_getframebox(IMAGE *im, int *box)
{
  long int nx, ny, nz;
  long int x, y, z, l1, l2;
  IMAGE *imout;
  PIX_TYPE *p, *pim, *pout;

  /* check the validity of input parmaters */
  if (box[0] > GetImNx(im) || box[1] > GetImNx(im) ||
      box[2] > GetImNy(im) || box[3] > GetImNy(im) ||
      box[4] > GetImNz(im) || box[5] > GetImNz(im)){
    (void)sprintf(buf, "getframebox(): invalid parameters\n"); errputstr(buf);
    return(NULL);
  }

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);
  /* create output image */
  imout = (IMAGE *)create_image(GetImDataType(im), (box[0]+box[1])*ny*nz + (box[2]+box[3])*nx*nz + (box[4]+box[5])*nx*ny, 1, 1);
  if (imout == NULL){
    (void)sprintf(buf,"getframebox(): not enough memory!\n"); errputstr(buf);
    return(imout);
  }

  pout = (PIX_TYPE *)GetImPtr(imout);
  pim  = (PIX_TYPE *)GetImPtr(im);

  l1 = box[0]; l2 = box[1];	/* left and right borders */
  for (z = 0; z < nz; z++){
    for (y = 0; y < ny; y++){
      p = pim + z * nx * ny + y * nx;
      for (x = 0; x < l1; x++)
	*pout++ = *p++;
      p = pim + z * nx * ny + y * nx + nx - l2;
      for (x = 0; x < l2; x++)
        *pout++ = *p++;
    }
  }
  l1 = box[2] * nx; l2 = box[3] * nx;	/* top and bottom borders */
  for (z = 0; z < nz; z++){
    p = pim + z * nx * ny;
    for (x = 0; x < l1; x++)
      *pout++ = *p++;
    p = pim + z * nx * ny + nx * (ny - box[3]);
    for (x = 0; x < l2; x++)
      *pout++ = *p++;
  }

  l1 = box[4] * nx * ny; l2 = box[5] * nx * ny;	/* up and down borders */
  p = pim;
  for (x = 0; x < l1; x++)
    *pout++ = *p++;
  p = pim + nx * ny * (nz - box[5]);
  for (x = 0; x < l2; x++)
    *pout++ = *p++;
  return imout;
}
#include "i32_undef.h"



IMAGE *getframebox(IMAGE *im, int *box)
{
  if ( (box[0]+box[1]+box[2]+box[3]+box[4]+box[5]) == 0)
    return NULL;

  switch (GetImDataType(im)){

  case t_UCHAR:
    return(uc_getframebox(im, box));
    break;

  case t_USHORT:
    return(us_getframebox(im, box));
    break;

  case t_INT32:
    return(i32_getframebox(im, box));
    break;

  default:
    (void) sprintf(buf, "getframebox(): invalid pixel type\n"); errputstr(buf);
    return NULL;
  }
  return NULL;
}

#include "uc_def.h"
ERROR_TYPE uc_setframebox(IMAGE *im, IMAGE *imframe, int *box)
{
  long int nx, ny, nz;
  long int x, y, z, l1, l2;
  PIX_TYPE *p, *pim, *pframe;

  /* check the validity of input parmaters */
  if (box[0] > GetImNx(im) || box[1] > GetImNx(im) ||
      box[2] > GetImNy(im) || box[3] > GetImNy(im) ||
      box[4] > GetImNz(im) || box[5] > GetImNz(im)){
    (void)sprintf(buf, "setframebox(): invalid parameters\n"); errputstr(buf);
    return(ERROR);
  }

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);
  /* check size of imframe  */
  if ( GetImNx(imframe) != (box[0]+box[1])*ny*nz + (box[2]+box[3])*nx*nz + (box[4]+box[5])*nx*ny ){
    (void)sprintf(buf,"setframebox(): frame image not matching border geometry (box)!\n"); errputstr(buf);
    return(ERROR);
  }

  pframe  =  (PIX_TYPE *)GetImPtr(imframe);
  pim  = (PIX_TYPE *)GetImPtr(im);

  l1 = box[0]; l2 = box[1];	/* left and right borders */
  for (z = 0; z < nz; z++){
    for (y = 0; y < ny; y++){
      p = pim + z * nx * ny + y * nx;
      for (x = 0; x < l1; x++)
	*p++ = *pframe++;
      p = pim + z * nx * ny + y * nx + nx - l2;
      for (x = 0; x < l2; x++)
        *p++ = *pframe++;
    }
  }
  l1 = box[2] * nx; l2 = box[3] * nx;	/* top and bottom borders */
  for (z = 0; z < nz; z++){
    p = pim + z * nx * ny;
    for (x = 0; x < l1; x++)
      *p++ = *pframe++;
    p = pim + z * nx * ny + nx * (ny - box[3]);
    for (x = 0; x < l2; x++)
      *p++ = *pframe++;
  }

  l1 = box[4] * nx * ny; l2 = box[5] * nx * ny;	/* up and down borders */
  p = pim;
  for (x = 0; x < l1; x++)
    *p++ = *pframe++;;
  p = pim + nx * ny * (nz - box[5]);
  for (x = 0; x < l2; x++)
    *p++ = *pframe++;
  return NO_ERROR;
}
#include "uc_undef.h"

#include "us_def.h"
ERROR_TYPE us_setframebox(IMAGE *im, IMAGE *imframe, int *box)
{
  long int nx, ny, nz;
  long int x, y, z, l1, l2;
  PIX_TYPE *p, *pim, *pframe;

  /* check the validity of input parmaters */
  if (box[0] > GetImNx(im) || box[1] > GetImNx(im) ||
      box[2] > GetImNy(im) || box[3] > GetImNy(im) ||
      box[4] > GetImNz(im) || box[5] > GetImNz(im)){
    (void)sprintf(buf, "setframebox(): invalid parameters\n"); errputstr(buf);
    return(ERROR);
  }

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);
  /* check size of imframe  */
  if ( GetImNx(imframe) != (box[0]+box[1])*ny*nz + (box[2]+box[3])*nx*nz + (box[4]+box[5])*nx*ny ){
    (void)sprintf(buf,"setframebox(): frame image not matching border geometry (box)!\n"); errputstr(buf);
    return(ERROR);
  }

  pframe  =  (PIX_TYPE *)GetImPtr(imframe);
  pim  = (PIX_TYPE *)GetImPtr(im);

  l1 = box[0]; l2 = box[1];	/* left and right borders */
  for (z = 0; z < nz; z++){
    for (y = 0; y < ny; y++){
      p = pim + z * nx * ny + y * nx;
      for (x = 0; x < l1; x++)
	*p++ = *pframe++;
      p = pim + z * nx * ny + y * nx + nx - l2;
      for (x = 0; x < l2; x++)
        *p++ = *pframe++;
    }
  }
  l1 = box[2] * nx; l2 = box[3] * nx;	/* top and bottom borders */
  for (z = 0; z < nz; z++){
    p = pim + z * nx * ny;
    for (x = 0; x < l1; x++)
      *p++ = *pframe++;
    p = pim + z * nx * ny + nx * (ny - box[3]);
    for (x = 0; x < l2; x++)
      *p++ = *pframe++;
  }

  l1 = box[4] * nx * ny; l2 = box[5] * nx * ny;	/* up and down borders */
  p = pim;
  for (x = 0; x < l1; x++)
    *p++ = *pframe++;;
  p = pim + nx * ny * (nz - box[5]);
  for (x = 0; x < l2; x++)
    *p++ = *pframe++;
  return NO_ERROR;
}
#include "us_undef.h"

#include "i32_def.h"
ERROR_TYPE i32_setframebox(IMAGE *im, IMAGE *imframe, int *box)
{
  long int nx, ny, nz;
  long int x, y, z, l1, l2;
  PIX_TYPE *p, *pim, *pframe;

  /* check the validity of input parmaters */
  if (box[0] > GetImNx(im) || box[1] > GetImNx(im) ||
      box[2] > GetImNy(im) || box[3] > GetImNy(im) ||
      box[4] > GetImNz(im) || box[5] > GetImNz(im)){
    (void)sprintf(buf, "setframebox(): invalid parameters\n"); errputstr(buf);
    return(ERROR);
  }

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);
  /* check size of imframe  */
  if ( GetImNx(imframe) != (box[0]+box[1])*ny*nz + (box[2]+box[3])*nx*nz + (box[4]+box[5])*nx*ny ){
    (void)sprintf(buf,"setframebox(): frame image not matching border geometry (box)!\n"); errputstr(buf);
    return(ERROR);
  }

  pframe  =  (PIX_TYPE *)GetImPtr(imframe);
  pim  = (PIX_TYPE *)GetImPtr(im);

  l1 = box[0]; l2 = box[1];	/* left and right borders */
  for (z = 0; z < nz; z++){
    for (y = 0; y < ny; y++){
      p = pim + z * nx * ny + y * nx;
      for (x = 0; x < l1; x++)
	*p++ = *pframe++;
      p = pim + z * nx * ny + y * nx + nx - l2;
      for (x = 0; x < l2; x++)
        *p++ = *pframe++;
    }
  }
  l1 = box[2] * nx; l2 = box[3] * nx;	/* top and bottom borders */
  for (z = 0; z < nz; z++){
    p = pim + z * nx * ny;
    for (x = 0; x < l1; x++)
      *p++ = *pframe++;
    p = pim + z * nx * ny + nx * (ny - box[3]);
    for (x = 0; x < l2; x++)
      *p++ = *pframe++;
  }

  l1 = box[4] * nx * ny; l2 = box[5] * nx * ny;	/* up and down borders */
  p = pim;
  for (x = 0; x < l1; x++)
    *p++ = *pframe++;;
  p = pim + nx * ny * (nz - box[5]);
  for (x = 0; x < l2; x++)
    *p++ = *pframe++;
  return NO_ERROR;
}
#include "i32_undef.h"

ERROR_TYPE setframebox(IMAGE *im, IMAGE *im_frame, int *box)
{
  switch (GetImDataType(im)){

  case t_UCHAR:
    return(uc_setframebox(im, im_frame, box));
    break;

  case t_USHORT:
    return(us_setframebox(im, im_frame, box));
    break;

  case t_INT32:
    return(i32_setframebox(im, im_frame, box));
    break;

  default:
    (void) sprintf(buf, "setframebox(): invalid pixel type\n"); errputstr(buf);
    return ERROR;
  }
  return ERROR;
}



#ifndef NO_generic_IMAGE
#include "g_def.h"
ERROR_TYPE generic_framebox(IMAGE *im, int *box, PIX_TYPE val)
{
  long int nx, ny, nz;
  long int x, y, z, l1, l2;
  PIX_TYPE *p, *pim;

  /* check the validity of input parmaters */
  if (box[0] > GetImNx(im) || box[1] > GetImNx(im) ||
      box[2] > GetImNy(im) || box[3] > GetImNy(im) ||
      box[4] > GetImNz(im) || box[5] > GetImNz(im)){
    (void)sprintf(buf, "framebox(): invalid parameters\n"); errputstr(buf);
    return(ERROR);
  }

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);
  pim = (PIX_TYPE *)GetImPtr(im);

  l1 = box[0]; l2 = box[1];	/* left and right borders */
  for (z = 0; z < nz; z++){
    for (y = 0; y < ny; y++){
      p = pim + z * nx * ny + y * nx;
      for (x = 0; x < l1; x++)
	*p++ = val;
      p = pim + z * nx * ny + y * nx + nx - l2;
      for (x = 0; x < l2; x++)
        *p++ = val;
    }
  }
  l1 = box[2] * nx; l2 = box[3] * nx;	/* top and bottom borders */
  for (z = 0; z < nz; z++){
    p = pim + z * nx * ny;
    for (x = 0; x < l1; x++)
      *p++ = val;
    p = pim + z * nx * ny + nx * (ny - box[3]);
    for (x = 0; x < l2; x++)
      *p++ = val;
  }

  l1 = box[4] * nx * ny; l2 = box[5] * nx * ny;	/* up and down borders */
  p = pim;
  for (x = 0; x < l1; x++)
    *p++ = val;
  p = pim + nx * ny * (nz - box[5]);
  for (x = 0; x < l2; x++)
    *p++ = val;
  return NO_ERROR;
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */


#include "u32_def.h"
ERROR_TYPE u32_framebox(IMAGE *im, int *box, PIX_TYPE val)
{
  long int nx, ny, nz;
  long int x, y, z, l1, l2;
  PIX_TYPE *p, *pim;

  /* check the validity of input parmaters */
  if (box[0] > GetImNx(im) || box[1] > GetImNx(im) ||
      box[2] > GetImNy(im) || box[3] > GetImNy(im) ||
      box[4] > GetImNz(im) || box[5] > GetImNz(im)){
    (void)sprintf(buf, "framebox(): invalid parameters\n"); errputstr(buf);
    return(ERROR);
  }

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);
  pim = (PIX_TYPE *)GetImPtr(im);

  l1 = box[0]; l2 = box[1];	/* left and right borders */
  for (z = 0; z < nz; z++){
    for (y = 0; y < ny; y++){
      p = pim + z * nx * ny + y * nx;
      for (x = 0; x < l1; x++)
	*p++ = val;
      p = pim + z * nx * ny + y * nx + nx - l2;
      for (x = 0; x < l2; x++)
        *p++ = val;
    }
  }
  l1 = box[2] * nx; l2 = box[3] * nx;	/* top and bottom borders */
  for (z = 0; z < nz; z++){
    p = pim + z * nx * ny;
    for (x = 0; x < l1; x++)
      *p++ = val;
    p = pim + z * nx * ny + nx * (ny - box[3]);
    for (x = 0; x < l2; x++)
      *p++ = val;
  }

  l1 = box[4] * nx * ny; l2 = box[5] * nx * ny;	/* up and down borders */
  p = pim;
  for (x = 0; x < l1; x++)
    *p++ = val;
  p = pim + nx * ny * (nz - box[5]);
  for (x = 0; x < l2; x++)
    *p++ = val;
  return NO_ERROR;
}
#include "u32_undef.h"


#include "uc_def.h"
ERROR_TYPE uc_framebox(IMAGE *im, int *box, PIX_TYPE val)
{
  long int nx, ny, nz;
  long int x, y, z, l1, l2;
  PIX_TYPE *p, *pim;

  /* check the validity of input parmaters */
  if (box[0] > GetImNx(im) || box[1] > GetImNx(im) ||
      box[2] > GetImNy(im) || box[3] > GetImNy(im) ||
      box[4] > GetImNz(im) || box[5] > GetImNz(im)){
    (void)sprintf(buf, "framebox(): invalid parameters\n"); errputstr(buf);
    return(ERROR);
  }

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);
  pim = (PIX_TYPE *)GetImPtr(im);

  l1 = box[0]; l2 = box[1];	/* left and right borders */
  for (z = 0; z < nz; z++){
    for (y = 0; y < ny; y++){
      p = pim + z * nx * ny + y * nx;
      for (x = 0; x < l1; x++)
	*p++ = val;
      p = pim + z * nx * ny + y * nx + nx - l2;
      for (x = 0; x < l2; x++)
        *p++ = val;
    }
  }
  l1 = box[2] * nx; l2 = box[3] * nx;	/* top and bottom borders */
  for (z = 0; z < nz; z++){
    p = pim + z * nx * ny;
    for (x = 0; x < l1; x++)
      *p++ = val;
    p = pim + z * nx * ny + nx * (ny - box[3]);
    for (x = 0; x < l2; x++)
      *p++ = val;
  }

  l1 = box[4] * nx * ny; l2 = box[5] * nx * ny;	/* up and down borders */
  p = pim;
  for (x = 0; x < l1; x++)
    *p++ = val;
  p = pim + nx * ny * (nz - box[5]);
  for (x = 0; x < l2; x++)
    *p++ = val;
  return NO_ERROR;
}
#include "uc_undef.h"


#include "i32_def.h"
ERROR_TYPE i32_framebox(IMAGE *im, int *box, PIX_TYPE val)
{
  long int nx, ny, nz;
  long int x, y, z, l1, l2;
  PIX_TYPE *p, *pim;

  /* check the validity of input parmaters */
  if (box[0] > GetImNx(im) || box[1] > GetImNx(im) ||
      box[2] > GetImNy(im) || box[3] > GetImNy(im) ||
      box[4] > GetImNz(im) || box[5] > GetImNz(im)){
    (void)sprintf(buf, "framebox(): invalid parameters\n"); errputstr(buf);
    return(ERROR);
  }

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);
  pim = (PIX_TYPE *)GetImPtr(im);

  l1 = box[0]; l2 = box[1];	/* left and right borders */
  for (z = 0; z < nz; z++){
    for (y = 0; y < ny; y++){
      p = pim + z * nx * ny + y * nx;
      for (x = 0; x < l1; x++)
	*p++ = val;
      p = pim + z * nx * ny + y * nx + nx - l2;
      for (x = 0; x < l2; x++)
        *p++ = val;
    }
  }
  l1 = box[2] * nx; l2 = box[3] * nx;	/* top and bottom borders */
  for (z = 0; z < nz; z++){
    p = pim + z * nx * ny;
    for (x = 0; x < l1; x++)
      *p++ = val;
    p = pim + z * nx * ny + nx * (ny - box[3]);
    for (x = 0; x < l2; x++)
      *p++ = val;
  }

  l1 = box[4] * nx * ny; l2 = box[5] * nx * ny;	/* up and down borders */
  p = pim;
  for (x = 0; x < l1; x++)
    *p++ = val;
  p = pim + nx * ny * (nz - box[5]);
  for (x = 0; x < l2; x++)
    *p++ = val;
  return NO_ERROR;
}
#include "i32_undef.h"


#include "us_def.h"
ERROR_TYPE us_framebox(IMAGE *im, int *box, PIX_TYPE val)
{
  long int nx, ny, nz;
  long int x, y, z, l1, l2;
  PIX_TYPE *p, *pim;

  /* check the validity of input parmaters */
  if (box[0] > GetImNx(im) || box[1] > GetImNx(im) ||
      box[2] > GetImNy(im) || box[3] > GetImNy(im) ||
      box[4] > GetImNz(im) || box[5] > GetImNz(im)){
    (void)sprintf(buf, "framebox(): invalid parameters\n"); errputstr(buf);
    return(ERROR);
  }

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);
  pim = (PIX_TYPE *)GetImPtr(im);

  l1 = box[0]; l2 = box[1];	/* left and right borders */
  for (z = 0; z < nz; z++){
    for (y = 0; y < ny; y++){
      p = pim + z * nx * ny + y * nx;
      for (x = 0; x < l1; x++)
	*p++ = val;
      p = pim + z * nx * ny + y * nx + nx - l2;
      for (x = 0; x < l2; x++)
        *p++ = val;
    }
  }
  l1 = box[2] * nx; l2 = box[3] * nx;	/* top and bottom borders */
  for (z = 0; z < nz; z++){
    p = pim + z * nx * ny;
    for (x = 0; x < l1; x++)
      *p++ = val;
    p = pim + z * nx * ny + nx * (ny - box[3]);
    for (x = 0; x < l2; x++)
      *p++ = val;
  }

  l1 = box[4] * nx * ny; l2 = box[5] * nx * ny;	/* up and down borders */
  p = pim;
  for (x = 0; x < l1; x++)
    *p++ = val;
  p = pim + nx * ny * (nz - box[5]);
  for (x = 0; x < l2; x++)
    *p++ = val;
  return NO_ERROR;
}
#include "us_undef.h"

#include "f_def.h"
ERROR_TYPE f_framebox(IMAGE *im, int *box, PIX_TYPE val)
{
  long int nx, ny, nz;
  long int x, y, z, l1, l2;
  PIX_TYPE *p, *pim;

  /* check the validity of input parmaters */
  if (box[0] > GetImNx(im) || box[1] > GetImNx(im) ||
      box[2] > GetImNy(im) || box[3] > GetImNy(im) ||
      box[4] > GetImNz(im) || box[5] > GetImNz(im)){
    (void)sprintf(buf, "framebox(): invalid parameters\n"); errputstr(buf);
    return(ERROR);
  }

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);
  pim = (PIX_TYPE *)GetImPtr(im);

  l1 = box[0]; l2 = box[1];	/* left and right borders */
  for (z = 0; z < nz; z++){
    for (y = 0; y < ny; y++){
      p = pim + z * nx * ny + y * nx;
      for (x = 0; x < l1; x++)
	*p++ = val;
      p = pim + z * nx * ny + y * nx + nx - l2;
      for (x = 0; x < l2; x++)
        *p++ = val;
    }
  }
  l1 = box[2] * nx; l2 = box[3] * nx;	/* top and bottom borders */
  for (z = 0; z < nz; z++){
    p = pim + z * nx * ny;
    for (x = 0; x < l1; x++)
      *p++ = val;
    p = pim + z * nx * ny + nx * (ny - box[3]);
    for (x = 0; x < l2; x++)
      *p++ = val;
  }

  l1 = box[4] * nx * ny; l2 = box[5] * nx * ny;	/* up and down borders */
  p = pim;
  for (x = 0; x < l1; x++)
    *p++ = val;
  p = pim + nx * ny * (nz - box[5]);
  for (x = 0; x < l2; x++)
    *p++ = val;
  return NO_ERROR;
}
#include "f_undef.h"

/* overwrite a frame of arbitrary shape around an image */
/* im: pointer to the I/O image */
/* box: array of 6 elements for the shape of the frame */
/* val: value of the frame to be overwritten */


ERROR_TYPE framebox(IMAGE *im, int *box, G_TYPE gval)
{
  switch (GetImDataType(im)){

#ifdef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_framebox(im, box, gval.generic_val));
    break;
#endif
  case t_UCHAR:
    return(uc_framebox(im, box, gval.uc_val));
    break;

  case t_USHORT:
    return(us_framebox(im, box, gval.us_val));
    break;

  case t_INT32:
    return(i32_framebox(im, box, gval.i32_val));
    break;

  case t_UINT32:
    return(u32_framebox(im, box, gval.u32_val));
    break;

  case t_FLOAT:
    return(f_framebox(im, box, gval.f_val));
    break;

  default:
    (void) sprintf(buf, "framebox(): invalid pixel type\n"); errputstr(buf);
    return ERROR;
  }
  return NO_ERROR;
}


#include "us_def.h"
ERROR_TYPE us_addframebox(IMAGE *im, int *box, PIX_TYPE val)
{
  unsigned long int nbyte;
  long int nxi, nyi, nzi;
  long int nxo, nyo, nzo;
  long int x, y, z, l1, l2;
  PIX_TYPE *pi, *po, *pim;

  nxi = GetImNx(im);
  nyi = GetImNy(im);
  nzi = GetImNz(im);
  nxo = nxi + box[0] + box[1];
  nyo = nyi + box[2] + box[3];
  nzo = nzi + box[4] + box[5];

  nbyte=nxo*nyo*nzo*sizeof(PIX_TYPE);
  if (nbyte%sizeof(long int)) /* pad for word size */
    nbyte+=sizeof(long int);

  pim = (PIX_TYPE *)realloc((char *)GetImPtr(im), nbyte);
  if (pim == NULL){
#ifdef XLISP
    gc();
    pim = (PIX_TYPE *)realloc((char *)GetImPtr(im), nbyte);
    if (pim == NULL){
      (void)sprintf(buf,"ERROR: addframebox: realloc returns a void pointer\n");
      errputstr(buf);
      return ERROR;
    }
#else
    (void) sprintf(buf,"addframebox(): not enough memory.\n"); errputstr(buf);
    return ERROR;
#endif
  }

  SetImNx(im, nxo);
  SetImNy(im, nyo);
  SetImNz(im, nzo);
  SetImPtr(im, pim);
  SetImNByte(im, nbyte);

  l2 = box[5] * nxo * nyo;	/* up border */
  po = pim + nxo * nyo * (nzo - box[5]);
  for (x = 0; x < l2; x++)
    *po++ = val;

  pi = pim + nxi * nyi * nzi - 1;
  po = pim + (nzo - box[5]) * nxo * nyo - 1;
  for (z = 0; z < nzi; z++){
    l2 = box[3] * nxo;
    for (x = 0; x < l2; x++)	/* bottom border */
      *po-- = val;
    l1 = box[0];
    l2 = box[1];
    for (y = 0; y < nyi; y++){
      for (x = 0; x < l2; x++)	/* right border */
        *po-- = val;
      for (x = 0; x < nxi; x++)
	*po-- = *pi--;
      for (x = 0; x < l1; x++)	/* left border */
        *po-- = val;
    }
    l1 = box[2] * nxo;		/* top border */
    for (x = 0; x < l1; x++)
	*po-- = val;
  }

  l1 = box[4] * nxo * nyo;	/* down border */
  for (x = 0; x < l1; x++)
    *po-- = val;

  return NO_ERROR;
}
#include "us_undef.h"


#include "s_def.h"
ERROR_TYPE s_addframebox(IMAGE *im, int *box, PIX_TYPE val)
{
  unsigned long int nbyte;
  long int nxi, nyi, nzi;
  long int nxo, nyo, nzo;
  long int x, y, z, l1, l2;
  PIX_TYPE *pi, *po, *pim;

  nxi = GetImNx(im);
  nyi = GetImNy(im);
  nzi = GetImNz(im);
  nxo = nxi + box[0] + box[1];
  nyo = nyi + box[2] + box[3];
  nzo = nzi + box[4] + box[5];

  nbyte=nxo*nyo*nzo*sizeof(PIX_TYPE);
  if (nbyte%sizeof(long int)) /* pad for word size */
    nbyte+=sizeof(long int);

  pim = (PIX_TYPE *)realloc((char *)GetImPtr(im), nbyte);
  if (pim == NULL){
#ifdef XLISP
    gc();
    pim = (PIX_TYPE *)realloc((char *)GetImPtr(im), nbyte);
    if (pim == NULL){
      (void)sprintf(buf,"ERROR: addframebox: realloc returns a void pointer\n");
      errputstr(buf);
      return ERROR;
    }
#else
    (void) sprintf(buf,"addframebox(): not enough memory.\n"); errputstr(buf);
    return ERROR;
#endif
  }

  SetImNx(im, nxo);
  SetImNy(im, nyo);
  SetImNz(im, nzo);
  SetImPtr(im, pim);
  SetImNByte(im, nbyte);

  l2 = box[5] * nxo * nyo;	/* up border */
  po = pim + nxo * nyo * (nzo - box[5]);
  for (x = 0; x < l2; x++)
    *po++ = val;

  pi = pim + nxi * nyi * nzi - 1;
  po = pim + (nzo - box[5]) * nxo * nyo - 1;
  for (z = 0; z < nzi; z++){
    l2 = box[3] * nxo;
    for (x = 0; x < l2; x++)	/* bottom border */
      *po-- = val;
    l1 = box[0];
    l2 = box[1];
    for (y = 0; y < nyi; y++){
      for (x = 0; x < l2; x++)	/* right border */
        *po-- = val;
      for (x = 0; x < nxi; x++)
	*po-- = *pi--;
      for (x = 0; x < l1; x++)	/* left border */
        *po-- = val;
    }
    l1 = box[2] * nxo;		/* top border */
    for (x = 0; x < l1; x++)
	*po-- = val;
  }

  l1 = box[4] * nxo * nyo;	/* down border */
  for (x = 0; x < l1; x++)
    *po-- = val;

  return NO_ERROR;
}
#include "s_undef.h"


#ifndef NO_generic_IMAGE
#include "g_def.h"
ERROR_TYPE generic_addframebox(IMAGE *im, int *box, PIX_TYPE val)
{
  unsigned long int nbyte;
  long int nxi, nyi, nzi;
  long int nxo, nyo, nzo;
  long int x, y, z, l1, l2;
  PIX_TYPE *pi, *po, *pim;

  nxi = GetImNx(im);
  nyi = GetImNy(im);
  nzi = GetImNz(im);
  nxo = nxi + box[0] + box[1];
  nyo = nyi + box[2] + box[3];
  nzo = nzi + box[4] + box[5];

  nbyte=nxo*nyo*nzo*sizeof(PIX_TYPE);
  if (nbyte%sizeof(long int)) /* pad for word size */
    nbyte+=sizeof(long int);
  pim = (PIX_TYPE *)realloc((char *)GetImPtr(im), nbyte);
  if (pim == NULL){
#if (defined(XLISP))
    gc();
    pim = (PIX_TYPE *)realloc((char *)GetImPtr(im), nbyte);
    if (pim == NULL){
      (void)sprintf(buf,"ERROR: addframebox: realloc returns a void pointer\n");
      errputstr(buf);
      return ERROR;
    }
#else
    (void) sprintf(buf,"addframebox(): not enough memory.\n"); errputstr(buf);
    return ERROR;
#endif
  }

  SetImNx(im, nxo);
  SetImNy(im, nyo);
  SetImNz(im, nzo);
  SetImPtr(im, pim);
  SetImNByte(im, nbyte);

  l2 = box[5] * nxo * nyo;	/* up border */
  po = pim + nxo * nyo * (nzo - box[5]);
  for (x = 0; x < l2; x++)
    *po++ = val;

  pi = pim + nxi * nyi * nzi - 1;
  po = pim + (nzo - box[5]) * nxo * nyo - 1;
  for (z = 0; z < nzi; z++){
    l2 = box[3] * nxo;
    for (x = 0; x < l2; x++)	/* bottom border */
      *po-- = val;
    l1 = box[0];
    l2 = box[1];
    for (y = 0; y < nyi; y++){
      for (x = 0; x < l2; x++)	/* right border */
        *po-- = val;
      for (x = 0; x < nxi; x++)
	*po-- = *pi--;
      for (x = 0; x < l1; x++)	/* left border */
        *po-- = val;
    }
    l1 = box[2] * nxo;		/* top border */
    for (x = 0; x < l1; x++)
	*po-- = val;
  }

  l1 = box[4] * nxo * nyo;	/* down border */
  for (x = 0; x < l1; x++)
    *po-- = val;

  return NO_ERROR;
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */



#include "i32_def.h"
ERROR_TYPE i32_addframebox(IMAGE *im, int *box, PIX_TYPE val)
{
  unsigned long int nbyte;
  long int nxi, nyi, nzi;
  long int nxo, nyo, nzo;
  long int x, y, z, l1, l2;
  PIX_TYPE *pi, *po, *pim;

  nxi = GetImNx(im);
  nyi = GetImNy(im);
  nzi = GetImNz(im);
  nxo = nxi + box[0] + box[1];
  nyo = nyi + box[2] + box[3];
  nzo = nzi + box[4] + box[5];

  nbyte=nxo*nyo*nzo*sizeof(PIX_TYPE);
  if (nbyte%sizeof(long int)) /* pad for word size */
    nbyte+=sizeof(long int);
  pim = (PIX_TYPE *)realloc((char *)GetImPtr(im), nbyte);
  if (pim == NULL){
#if (defined(XLISP))
    gc();
    pim = (PIX_TYPE *)realloc((char *)GetImPtr(im), nbyte);
    if (pim == NULL){
      (void)sprintf(buf,"ERROR: addframebox: realloc returns a void pointer\n");
      errputstr(buf);
      return ERROR;
    }
#else
    (void) sprintf(buf,"addframebox(): not enough memory.\n"); errputstr(buf);
    return ERROR;
#endif
  }

  SetImNx(im, nxo);
  SetImNy(im, nyo);
  SetImNz(im, nzo);
  SetImPtr(im, pim);
  SetImNByte(im, nbyte);

  l2 = box[5] * nxo * nyo;	/* up border */
  po = pim + nxo * nyo * (nzo - box[5]);
  for (x = 0; x < l2; x++)
    *po++ = val;

  pi = pim + nxi * nyi * nzi - 1;
  po = pim + (nzo - box[5]) * nxo * nyo - 1;
  for (z = 0; z < nzi; z++){
    l2 = box[3] * nxo;
    for (x = 0; x < l2; x++)	/* bottom border */
      *po-- = val;
    l1 = box[0];
    l2 = box[1];
    for (y = 0; y < nyi; y++){
      for (x = 0; x < l2; x++)	/* right border */
        *po-- = val;
      for (x = 0; x < nxi; x++)
	*po-- = *pi--;
      for (x = 0; x < l1; x++)	/* left border */
        *po-- = val;
    }
    l1 = box[2] * nxo;		/* top border */
    for (x = 0; x < l1; x++)
	*po-- = val;
  }

  l1 = box[4] * nxo * nyo;	/* down border */
  for (x = 0; x < l1; x++)
    *po-- = val;

  return NO_ERROR;
}
#include "i32_undef.h"

#include "u32_def.h"
ERROR_TYPE u32_addframebox(IMAGE *im, int *box, PIX_TYPE val)
{
  unsigned long int nbyte;
  long int nxi, nyi, nzi;
  long int nxo, nyo, nzo;
  long int x, y, z, l1, l2;
  PIX_TYPE *pi, *po, *pim;

  nxi = GetImNx(im);
  nyi = GetImNy(im);
  nzi = GetImNz(im);
  nxo = nxi + box[0] + box[1];
  nyo = nyi + box[2] + box[3];
  nzo = nzi + box[4] + box[5];

  nbyte=nxo*nyo*nzo*sizeof(PIX_TYPE);
  if (nbyte%sizeof(long int)) /* pad for word size */
    nbyte+=sizeof(long int);
  pim = (PIX_TYPE *)realloc((char *)GetImPtr(im), nbyte);
  if (pim == NULL){
#if (defined(XLISP))
    gc();
    pim = (PIX_TYPE *)realloc((char *)GetImPtr(im), nbyte);
    if (pim == NULL){
      (void)sprintf(buf,"ERROR: addframebox: realloc returns a void pointer\n");
      errputstr(buf);
      return ERROR;
    }
#else
    (void) sprintf(buf,"addframebox(): not enough memory.\n"); errputstr(buf);
    return ERROR;
#endif
  }

  SetImNx(im, nxo);
  SetImNy(im, nyo);
  SetImNz(im, nzo);
  SetImPtr(im, pim);
  SetImNByte(im, nbyte);

  l2 = box[5] * nxo * nyo;	/* up border */
  po = pim + nxo * nyo * (nzo - box[5]);
  for (x = 0; x < l2; x++)
    *po++ = val;

  pi = pim + nxi * nyi * nzi - 1;
  po = pim + (nzo - box[5]) * nxo * nyo - 1;
  for (z = 0; z < nzi; z++){
    l2 = box[3] * nxo;
    for (x = 0; x < l2; x++)	/* bottom border */
      *po-- = val;
    l1 = box[0];
    l2 = box[1];
    for (y = 0; y < nyi; y++){
      for (x = 0; x < l2; x++)	/* right border */
        *po-- = val;
      for (x = 0; x < nxi; x++)
	*po-- = *pi--;
      for (x = 0; x < l1; x++)	/* left border */
        *po-- = val;
    }
    l1 = box[2] * nxo;		/* top border */
    for (x = 0; x < l1; x++)
	*po-- = val;
  }

  l1 = box[4] * nxo * nyo;	/* down border */
  for (x = 0; x < l1; x++)
    *po-- = val;

  return NO_ERROR;
}
#include "u32_undef.h"


#include "f_def.h"
ERROR_TYPE f_addframebox(IMAGE *im, int *box, PIX_TYPE val)
{
  unsigned long int nbyte;
  long int nxi, nyi, nzi;
  long int nxo, nyo, nzo;
  long int x, y, z, l1, l2;
  PIX_TYPE *pi, *po, *pim;

  nxi = GetImNx(im);
  nyi = GetImNy(im);
  nzi = GetImNz(im);
  nxo = nxi + box[0] + box[1];
  nyo = nyi + box[2] + box[3];
  nzo = nzi + box[4] + box[5];

  nbyte=nxo*nyo*nzo*sizeof(PIX_TYPE);
  if (nbyte%sizeof(long int)) /* pad for word size */
    nbyte+=sizeof(long int);
  pim = (PIX_TYPE *)realloc((char *)GetImPtr(im), nbyte);
  if (pim == NULL){
#if (defined(XLISP))
    gc();
    pim = (PIX_TYPE *)realloc((char *)GetImPtr(im), nbyte);
    if (pim == NULL){
      (void)sprintf(buf,"ERROR: addframebox: realloc returns a void pointer\n");
      errputstr(buf);
      return ERROR;
    }
#else
    (void) sprintf(buf,"addframebox(): not enough memory.\n"); errputstr(buf);
    return ERROR;
#endif
  }

  SetImNx(im, nxo);
  SetImNy(im, nyo);
  SetImNz(im, nzo);
  SetImPtr(im, pim);
  SetImNByte(im, nbyte);

  l2 = box[5] * nxo * nyo;	/* up border */
  po = pim + nxo * nyo * (nzo - box[5]);
  for (x = 0; x < l2; x++)
    *po++ = val;

  pi = pim + nxi * nyi * nzi - 1;
  po = pim + (nzo - box[5]) * nxo * nyo - 1;
  for (z = 0; z < nzi; z++){
    l2 = box[3] * nxo;
    for (x = 0; x < l2; x++)	/* bottom border */
      *po-- = val;
    l1 = box[0];
    l2 = box[1];
    for (y = 0; y < nyi; y++){
      for (x = 0; x < l2; x++)	/* right border */
        *po-- = val;
      for (x = 0; x < nxi; x++)
	*po-- = *pi--;
      for (x = 0; x < l1; x++)	/* left border */
        *po-- = val;
    }
    l1 = box[2] * nxo;		/* top border */
    for (x = 0; x < l1; x++)
	*po-- = val;
  }

  l1 = box[4] * nxo * nyo;	/* down border */
  for (x = 0; x < l1; x++)
    *po-- = val;

  return NO_ERROR;
}
#include "f_undef.h"


/* add a border around an image */
/* im: pointer to the I/O image */
/* box: array of 6 elements for the shape of the border */
/* val: value of the added border */

ERROR_TYPE addframebox(IMAGE *im, int *box, G_TYPE gval)
{
  if ( (box[0]+box[1]+box[2]+box[3]+box[4]+box[5]) == 0)
    return NO_ERROR;

  switch (GetImDataType(im)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_addframebox(im, box, gval.generic_val));
    break;
#endif

  case t_USHORT:
    return(us_addframebox(im, box, gval.us_val));
    break;

  case t_SHORT:
    return(s_addframebox(im, box, gval.s_val));
    break;

  case t_UINT32:
    return(u32_addframebox(im, box, gval.u32_val));
    break;

  case t_INT32:
    return(i32_addframebox(im, box, gval.i32_val));
    break;

  case t_FLOAT:
    return(f_addframebox(im, box, gval.f_val));
    break;

  default:
    (void) sprintf(buf, "addframebox(): invalid pixel type\n"); errputstr(buf);
    return ERROR;
  }
  return NO_ERROR;
}



#ifndef NO_generic_IMAGE
#include "g_def.h"
ERROR_TYPE generic_subframebox(IMAGE *im, int *box)
{
  unsigned long int nbyte;
  long int nxi, nyi, nzi;
  long int nxo, nyo, nzo;
  long int x, y, z;
  PIX_TYPE *pi, *po;


  nxi = GetImNx(im);
  nyi = GetImNy(im);
  nzi = GetImNz(im);

  nxo = nxi - box[0] - box[1];
  nyo = nyi - box[2] - box[3];
  nzo = nzi - box[4] - box[5];

  pi = (PIX_TYPE *)GetImPtr(im) + nxi * nyi * box[4];
  po = (PIX_TYPE *)GetImPtr(im);
  for (z = 0; z < nzo; z++){
    pi += box[2] * nxi;
    for (y = 0; y < nyo; y++){
      pi += box[0];
      for (x = 0; x < nxo; x++)
	*po++ = *pi++;
      pi += box[1];
    }
    pi += box[3] * nxi;
  }

  nbyte=nxo*nyo*nzo*sizeof(PIX_TYPE);
  if (nbyte%sizeof(long int)) /* pad for word size */
    nbyte+=sizeof(long int);
  po = (PIX_TYPE *)realloc((char *)GetImPtr(im), nbyte);
  if (po == NULL){
    (void) sprintf(buf, "subframebox(): Realloc unsuccessful!!!  Image modified\n"); errputstr(buf);
    return ERROR;
  }

  SetImNx(im, nxo);
  SetImNy(im, nyo);
  SetImNz(im, nzo);
  SetImPtr(im, po);
  SetImNByte(im, nbyte);

  return NO_ERROR;
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */


#include "i32_def.h"
ERROR_TYPE i32_subframebox(IMAGE *im, int *box)
{
  unsigned long int nbyte;
  long int nxi, nyi, nzi;
  long int nxo, nyo, nzo;
  long int x, y, z;
  PIX_TYPE *pi, *po;


  nxi = GetImNx(im);
  nyi = GetImNy(im);
  nzi = GetImNz(im);

  nxo = nxi - box[0] - box[1];
  nyo = nyi - box[2] - box[3];
  nzo = nzi - box[4] - box[5];

  pi = (PIX_TYPE *)GetImPtr(im) + nxi * nyi * box[4];
  po = (PIX_TYPE *)GetImPtr(im);
  for (z = 0; z < nzo; z++){
    pi += box[2] * nxi;
    for (y = 0; y < nyo; y++){
      pi += box[0];
      for (x = 0; x < nxo; x++)
	*po++ = *pi++;
      pi += box[1];
    }
    pi += box[3] * nxi;
  }

  nbyte=nxo*nyo*nzo*sizeof(PIX_TYPE);
  if (nbyte%sizeof(long int)) /* pad for word size */
    nbyte+=sizeof(long int);
  po = (PIX_TYPE *)realloc((char *)GetImPtr(im), nbyte);
  if (po == NULL){
    (void) sprintf(buf, "subframebox(): Realloc unsuccessful!!!  Image modified\n"); errputstr(buf);
    return ERROR;
  }

  SetImNx(im, nxo);
  SetImNy(im, nyo);
  SetImNz(im, nzo);
  SetImPtr(im, po);
  SetImNByte(im, nbyte);

  return NO_ERROR;
}
#include "i32_undef.h"

#include "us_def.h"
ERROR_TYPE us_subframebox(IMAGE *im, int *box)
{
  unsigned long int nbyte;
  long int nxi, nyi, nzi;
  long int nxo, nyo, nzo;
  long int x, y, z;
  PIX_TYPE *pi, *po;


  nxi = GetImNx(im);
  nyi = GetImNy(im);
  nzi = GetImNz(im);

  nxo = nxi - box[0] - box[1];
  nyo = nyi - box[2] - box[3];
  nzo = nzi - box[4] - box[5];

  pi = (PIX_TYPE *)GetImPtr(im) + nxi * nyi * box[4];
  po = (PIX_TYPE *)GetImPtr(im);
  for (z = 0; z < nzo; z++){
    pi += box[2] * nxi;
    for (y = 0; y < nyo; y++){
      pi += box[0];
      for (x = 0; x < nxo; x++)
	*po++ = *pi++;
      pi += box[1];
    }
    pi += box[3] * nxi;
  }


  nbyte=nxo*nyo*nzo*sizeof(PIX_TYPE);
  if (nbyte%sizeof(long int)) /* pad for word size */
    nbyte+=sizeof(long int);
  po = (PIX_TYPE *)realloc((char *)GetImPtr(im), nbyte);
  if (po == NULL){
    (void) sprintf(buf, "subframebox(): Realloc unsuccessful!!!  Image modified\n"); errputstr(buf);
    return ERROR;
  }

  SetImNx(im, nxo);
  SetImNy(im, nyo);
  SetImNz(im, nzo);
  SetImPtr(im, po);
  SetImNByte(im, nbyte);

  return NO_ERROR;
}
#include "us_undef.h"


ERROR_TYPE subframebox(IMAGE *im, int *box)
{

  if ( (box[0]+box[1]+box[2]+box[3]+box[4]+box[5]) == 0)
    return NO_ERROR;

  switch (GetImDataType(im)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_subframebox(im, box));
    break;
#endif
#ifndef NO_uc_IMAGE
  case t_UCHAR:
    return(uc_subframebox(im, box));
    break;
#endif

  case t_SHORT:
  case t_USHORT:
    return(us_subframebox(im, box));
    break;

  case t_FLOAT: /* POTENTIAL BUG: assumes that sizeof(float) is equal to sizeof(long int) */
  case t_UINT32:
  case t_INT32:
    return(i32_subframebox(im, box));
    break;

  default:
    (void) sprintf(buf, "subframebox(): invalid pixel type\n"); errputstr(buf);
    return ERROR;
  }
  return NO_ERROR;
}



#ifndef NO_generic_IMAGE
#include "g_def.h"
ERROR_TYPE generic_imputop(IMAGE *imout, IMAGE *imin, int x1, int y1, int z1, int op)
{
  /* we assume that imin is smaller than imout in all directions */
  int nx, ny, nz, i, j, k;
  PIX_TYPE *p, *pi, *po;
  long int test;

  nx = GetImNx(imin);
  ny = GetImNy(imin);
  nz = GetImNz(imin);

  pi = (PIX_TYPE *)GetImPtr(imin);
  po = (PIX_TYPE *)GetImPtr(imout);

  switch(op){
  case AND_op:
#if (FLOATING==0)
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p&=*pi;
	}
      }
    }
    break;
  case OR_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p|=*pi;
	}
      }
    }
    break;
  case XOR_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p^=*pi;
	}
      }
    }
    break;
#endif
  case ADD_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  test = *p+*pi;
	  if (test > PIX_MAX){
	    *p = PIX_MAX;
	  }
	  else
	    *p=(PIX_TYPE) test;
	}
      }
    }
    break;
  case SUB_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  test = *p-*pi;
	  if (test < PIX_MIN){
	    *p = PIX_MIN;
	  }
	  else
	    *p=(PIX_TYPE) test;
	}
      }
    }
    break;
  case ADD_op_ovfl: /* do not check for overflows */
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p+=*pi;
	}
      }
    }
    break;
  case INF_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p>*pi)
	    *p = *pi;
	}
      }
    }
    break;
  case SUP_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p<*pi)
	    *p = *pi;
	}
      }
    }
    break;
  case OVW_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	    *p = *pi;
	}
      }
    }
    break;
  case MASK_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*pi!=0)
	    *p = *pi;
	}
      }
    }
    break;
  case MASK_op2:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p==0)
	    *p = *pi;
	}
      }
    }
    break;
  case MULT_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	    *p *= *pi;
	}
      }
    }
    break;
  case CMP_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p < *pi)
	    *p = 1;
	  else if (*p > *pi)
	    *p = 2;
	  else
	    *p = 0;
	}
      }
    }
    break;
  default:
    (void)sprintf(buf, "ERROR in inmputop(): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */

#include "us_def.h"
ERROR_TYPE us_imputop(IMAGE *imout, IMAGE *imin, int x1, int y1, int z1, int op)
{
  /* we assume that imin is smaller than imout in all directions */
  int nx, ny, nz, i, j, k;
  PIX_TYPE *p, *pi, *po;
  long int test;

  nx = GetImNx(imin);
  ny = GetImNy(imin);
  nz = GetImNz(imin);

  pi = (PIX_TYPE *)GetImPtr(imin);
  po = (PIX_TYPE *)GetImPtr(imout);

  switch(op){
  case AND_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p&=*pi;
	}
      }
    }
    break;
  case OR_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p|=*pi;
	}
      }
    }
    break;
  case XOR_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p^=*pi;
	}
      }
    }
    break;
  case ADD_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  test = *p+*pi;
	  if (test > PIX_MAX){
	    *p = PIX_MAX;
	  }
	  else
	    *p=(PIX_TYPE) test;
	}
      }
    }
    break;
  case ADD_op_ovfl: /* do not check for overflows */
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p+=*pi;
	}
      }
    }
    break;
  case SUB_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  test = *p-*pi;
	  if (test < PIX_MIN){
	    *p = PIX_MIN;
	  }
	  else
	    *p=(PIX_TYPE) test;
	}
      }
    }
    break;
  case INF_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p>*pi)
	    *p = *pi;
	}
      }
    }
    break;
  case SUP_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p<*pi)
	    *p = *pi;
	}
      }
    }
    break;
  case OVW_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	    *p = *pi;
	}
      }
    }
    break;
  case MASK_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*pi!=0)
	    *p = *pi;
	}
      }
    }
    break;
  case MASK_op2:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p==0)
	    *p = *pi;
	}
      }
    }
    break;
  case MULT_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	    *p *= *pi;
	}
      }
    }
    break;
  case CMP_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p < *pi)
	    *p = 1;
	  else if (*p > *pi)
	    *p = 2;
	  else
	    *p = 0;
	}
      }
    }
    break;
  default:
    (void)sprintf(buf, "ERROR in inmputop(): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}
#include "us_undef.h"

#include "s_def.h"
ERROR_TYPE s_imputop(IMAGE *imout, IMAGE *imin, int x1, int y1, int z1, int op)
{
  /* we assume that imin is smaller than imout in all directions */
  int nx, ny, nz, i, j, k;
  PIX_TYPE *p, *pi, *po;
  long int test;

  nx = GetImNx(imin);
  ny = GetImNy(imin);
  nz = GetImNz(imin);

  pi = (PIX_TYPE *)GetImPtr(imin);
  po = (PIX_TYPE *)GetImPtr(imout);

  switch(op){
  case AND_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p&=*pi;
	}
      }
    }
    break;
  case OR_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p|=*pi;
	}
      }
    }
    break;
  case XOR_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p^=*pi;
	}
      }
    }
    break;
  case ADD_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  test = *p+*pi;
	  if (test > PIX_MAX){
	    *p = PIX_MAX;
	  }
	  else
	    *p=(PIX_TYPE) test;
	}
      }
    }
    break;
  case ADD_op_ovfl: /* do not check for overflows */
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p+=*pi;
	}
      }
    }
    break;
  case SUB_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  test = *p-*pi;
	  if (test < PIX_MIN){
	    *p = PIX_MIN;
	  }
	  else
	    *p=(PIX_TYPE) test;
	}
      }
    }
    break;
  case INF_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p>*pi)
	    *p = *pi;
	}
      }
    }
    break;
  case SUP_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p<*pi)
	    *p = *pi;
	}
      }
    }
    break;
  case OVW_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	    *p = *pi;
	}
      }
    }
    break;
  case MASK_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*pi!=0)
	    *p = *pi;
	}
      }
    }
    break;
  case MASK_op2:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p==0)
	    *p = *pi;
	}
      }
    }
    break;
  case MULT_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	    *p *= *pi;
	}
      }
    }
    break;
  case CMP_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p < *pi)
	    *p = 1;
	  else if (*p > *pi)
	    *p = 2;
	  else
	    *p = 0;
	}
      }
    }
    break;
  default:
    (void)sprintf(buf, "ERROR in inmputop(): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}
#include "s_undef.h"

#include "f_def.h"
ERROR_TYPE f_imputop(IMAGE *imout, IMAGE *imin, int x1, int y1, int z1, int op)
{
  /* we assume that imin is smaller than imout in all directions */
  int nx, ny, nz, i, j, k;
  PIX_TYPE *p, *pi, *po;
  float test;

  nx = GetImNx(imin);
  ny = GetImNy(imin);
  nz = GetImNz(imin);

  pi = (PIX_TYPE *)GetImPtr(imin);
  po = (PIX_TYPE *)GetImPtr(imout);

  switch(op){
#if (FLOATING==0)
  case AND_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p&=*pi;
	}
      }
    }
    break;
  case OR_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p|=*pi;
	}
      }
    }
    break;
  case XOR_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p^=*pi;
	}
      }
    }
    break;
#endif
  case ADD_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  test = *p+*pi;
	  if (test > PIX_MAX){
	    *p = PIX_MAX;
	  }
	  else
	    *p=(PIX_TYPE) test;
	}
      }
    }
    break;
  case SUB_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  test = *p-*pi;
	  if (test < PIX_MIN){
	    *p = PIX_MIN;
	  }
	  else
	    *p=(PIX_TYPE) test;
	}
      }
    }
    break;
  case ADD_op_ovfl: /* do not check for overflows */
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p+=*pi;
	}
      }
    }
    break;
  case INF_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p>*pi)
	    *p = *pi;
	}
      }
    }
    break;
  case SUP_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p<*pi)
	    *p = *pi;
	}
      }
    }
    break;
  case OVW_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	    *p = *pi;
	}
      }
    }
    break;
  case MASK_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*pi!=0)
	    *p = *pi;
	}
      }
    }
    break;
  case MASK_op2:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p==0)
	    *p = *pi;
	}
      }
    }
    break;
  case MULT_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	    *p *= *pi;
	}
      }
    }
    break;
  case CMP_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p < *pi)
	    *p = 1;
	  else if (*p > *pi)
	    *p = 2;
	  else
	    *p = 0;
	}
      }
    }
    break;
  default:
    (void)sprintf(buf, "ERROR in inmputop(): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}
#include "f_undef.h"

#include "i32_def.h"
ERROR_TYPE i32_imputop(IMAGE *imout, IMAGE *imin, int x1, int y1, int z1, int op)
{
  /* we assume that imin is smaller than imout in all directions */
  int nx, ny, nz, i, j, k;
  PIX_TYPE *p, *pi, *po;
  long int test;

  nx = GetImNx(imin);
  ny = GetImNy(imin);
  nz = GetImNz(imin);

  pi = (PIX_TYPE *)GetImPtr(imin);
  po = (PIX_TYPE *)GetImPtr(imout);

  switch(op){
  case AND_op:
#if (FLOATING==0)
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p&=*pi;
	}
      }
    }
    break;
  case OR_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p|=*pi;
	}
      }
    }
    break;
  case XOR_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p^=*pi;
	}
      }
    }
    break;
#endif
  case ADD_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  test = *p+*pi;
	  if (test > PIX_MAX){
	    *p = PIX_MAX;
	  }
	  else
	    *p=(PIX_TYPE) test;
	}
      }
    }
    break;
  case SUB_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  test = *p-*pi;
	  if (test < PIX_MIN){
	    *p = PIX_MIN;
	  }
	  else
	    *p=(PIX_TYPE) test;
	}
      }
    }
    break;
  case ADD_op_ovfl: /* do not check for overflows */
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p+=*pi;
	}
      }
    }
    break;
  case INF_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p>*pi)
	    *p = *pi;
	}
      }
    }
    break;
  case SUP_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p<*pi)
	    *p = *pi;
	}
      }
    }
    break;
  case OVW_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	    *p = *pi;
	}
      }
    }
    break;
  case MASK_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*pi!=0)
	    *p = *pi;
	}
      }
    }
    break;
  case MASK_op2:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p==0)
	    *p = *pi;
	}
      }
    }
    break;
  case MULT_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	    *p *= *pi;
	}
      }
    }
    break;
  case CMP_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p < *pi)
	    *p = 1;
	  else if (*p > *pi)
	    *p = 2;
	  else
	    *p = 0;
	}
      }
    }
    break;
  default:
    (void)sprintf(buf, "ERROR in inmputop(): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}
#include "i32_undef.h"



#include "u32_def.h"
ERROR_TYPE u32_imputop(IMAGE *imout, IMAGE *imin, int x1, int y1, int z1, int op)
{
  /* we assume that imin is smaller than imout in all directions */
  int nx, ny, nz, i, j, k;
  PIX_TYPE *p, *pi, *po;
  long int test;

  nx = GetImNx(imin);
  ny = GetImNy(imin);
  nz = GetImNz(imin);

  pi = (PIX_TYPE *)GetImPtr(imin);
  po = (PIX_TYPE *)GetImPtr(imout);

  switch(op){
  case AND_op:
#if (FLOATING==0)
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p&=*pi;
	}
      }
    }
    break;
  case OR_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p|=*pi;
	}
      }
    }
    break;
  case XOR_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p^=*pi;
	}
      }
    }
    break;
#endif
  case ADD_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  test = *p+*pi;
	  if (test > PIX_MAX){
	    *p = PIX_MAX;
	  }
	  else
	    *p=(PIX_TYPE) test;
	}
      }
    }
    break;
  case SUB_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  test = *p-*pi;
	  if (test < PIX_MIN){
	    *p = PIX_MIN;
	  }
	  else
	    *p=(PIX_TYPE) test;
	}
      }
    }
    break;
  case ADD_op_ovfl: /* do not check for overflows */
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  *p+=*pi;
	}
      }
    }
    break;
  case INF_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p>*pi)
	    *p = *pi;
	}
      }
    }
    break;
  case SUP_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p<*pi)
	    *p = *pi;
	}
      }
    }
    break;
  case OVW_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	    *p = *pi;
	}
      }
    }
    break;
  case MASK_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*pi!=0)
	    *p = *pi;
	}
      }
    }
    break;
  case MASK_op2:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p==0)
	    *p = *pi;
	}
      }
    }
    break;
  case MULT_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	    *p *= *pi;
	}
      }
    }
    break;
  case CMP_op:
    for (i=0; i < nz; i++){
      for (j=0; j < ny; j++){
	p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
	for (k=0; k < nx; k++,pi++,p++){
	  if (*p < *pi)
	    *p = 1;
	  else if (*p > *pi)
	    *p = 2;
	  else
	    *p = 0;
	}
      }
    }
    break;
  default:
    (void)sprintf(buf, "ERROR in inmputop(): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}
#include "u32_undef.h"




ERROR_TYPE imputop(IMAGE *im1, IMAGE *im2, int x, int y, int z, int op)
{
  /* check that im2 is a subset of im1 given x, y, and z */
  if (GetImNx(im2)>GetImNx(im1)-x || GetImNy(im2)>GetImNy(im1)-y || \
      GetImNz(im2)>GetImNz(im1)-z || (GetImDataType(im2)!=GetImDataType(im1) )){
    iminfo(im1);
    iminfo(im2);
    (void)sprintf(buf, "imputop(im1.type=%d, im2.type=%d, %d, %d, %d, %d): invalid parameters or image size or types (input images must have the same type)\n", GetImDataType(im1), GetImDataType(im2), x, y, z, op); errputstr(buf);
    return(ERROR);
  }
  switch (GetImDataType(im1)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_imputop(im1, im2, x, y, z, op));
    break;
#endif

  case t_SHORT:
    return(s_imputop(im1, im2, x, y, z, op));
    break;

  case t_USHORT:
    return(us_imputop(im1, im2, x, y, z, op));
    break;

  case t_FLOAT:
    return(f_imputop(im1, im2, x, y, z, op));
    break;

  case t_INT32:
    return(i32_imputop(im1, im2, x, y, z, op));
    break;

  case t_UINT32:
    return(u32_imputop(im1, im2, x, y, z, op));
    break;

  default:
    (void)sprintf(buf,"imputop()op: invalid pixel type\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}


#include "uc_def.h"
#define PIX_IMLBL_TYPE unsigned char
ERROR_TYPE uc_uc_imputcompose(IMAGE *imin, IMAGE *imlbl, IMAGE *imout, int x1, int y1, int z1, int val)
{
  /* we assume that imin is smaller than imout in all directions */
  int nx, ny, nz, i, j, k;
  PIX_TYPE *p, *pi, *po;
  PIX_IMLBL_TYPE *plbl, *plbl0;

  nx = GetImNx(imin);
  ny = GetImNy(imin);
  nz = GetImNz(imin);

  pi = (PIX_TYPE *)GetImPtr(imin);
  po = (PIX_TYPE *)GetImPtr(imout);
  plbl0 = (PIX_IMLBL_TYPE *)GetImPtr(imlbl);

  for (i=0; i < nz; i++){
    for (j=0; j < ny; j++){
      p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
      plbl = plbl0 + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
      for (k=0; k < nx; k++,pi++,p++,plbl++){
	if(*plbl==val)
	  *p=*pi;
      }
    }
  }
  return(NO_ERROR);
}
#undef PIX_IMLBL_TYPE
#include "uc_undef.h"

#include "uc_def.h"
#define PIX_IMLBL_TYPE unsigned short
ERROR_TYPE us_uc_imputcompose(IMAGE *imin, IMAGE *imlbl, IMAGE *imout, int x1, int y1, int z1, int val)
{
  /* we assume that imin is smaller than imout in all directions */
  int nx, ny, nz, i, j, k;
  PIX_TYPE *p, *pi, *po;
  PIX_IMLBL_TYPE *plbl, *plbl0;

  nx = GetImNx(imin);
  ny = GetImNy(imin);
  nz = GetImNz(imin);

  pi = (PIX_TYPE *)GetImPtr(imin);
  po = (PIX_TYPE *)GetImPtr(imout);
  plbl0 = (PIX_IMLBL_TYPE *)GetImPtr(imlbl);

  for (i=0; i < nz; i++){
    for (j=0; j < ny; j++){
      p = po + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
      plbl = plbl0 + (z1+i) * GetImNx(imout)*GetImNy(imout) + GetImNx(imout)*(y1+j) + x1;
      for (k=0; k < nx; k++,pi++,p++,plbl++){
	if(*plbl==val)
	  *p=*pi;
      }
    }
  }
  return(NO_ERROR);
}
#undef PIX_IMLBL_TYPE
#include "uc_undef.h"

ERROR_TYPE imputcompose(IMAGE *im1, IMAGE *imlbl, IMAGE *im2, int x, int y, int z, int val)
{
  /* check that im1 is a subset of im2 given x, y, and z */
  if (GetImNx(im1)>GetImNx(im2)-x || GetImNy(im1)>GetImNy(im2)-y || GetImNz(im1)>GetImNz(im2)-z || (GetImDataType(im1)!=GetImDataType(im2) )){
    (void)sprintf(buf, "imput(): invalid parameters or image size or types (input images must have the same type)\n"); errputstr(buf);
    return(ERROR);
  }
  switch (GetImDataType(im1)){
  case t_UCHAR:
    switch (GetImDataType(imlbl)){
    case t_UCHAR:
      return(uc_uc_imputcompose(im1, imlbl, im2, x, y, z, val));
      break;
      break;
    case t_USHORT:
      return(us_uc_imputcompose(im1, imlbl, im2, x, y, z, val));
      break;
    default:
      (void)sprintf(buf,"imputcompose(): invalid pixel type for label image\n"); errputstr(buf);
      return(ERROR);
    }
  default:
    (void)sprintf(buf,"imputcompose(): invalid pixel type for 1st image\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}

#include "uc_def.h"
IMAGE *uc_getboundingbox(IMAGE *im)
{
  IMAGE *bb_im;
  PIX_TYPE *pim;
  INT32 *pbb, ulcx=GetImNx(im), ulcy=GetImNy(im), lrcx=0, lrcy=0;
  int x,y,nx=GetImNx(im),ny=GetImNy(im);

  bb_im=create_image(t_INT32, 4, 1, 1);
  pbb=(INT32 *)GetImPtr(bb_im);
  pim=(PIX_TYPE *)GetImPtr(im);

  for (y=0;y<ny;y++){
    for (x=0;x<nx;x++){
      if (*pim++){
	if (x<ulcx)
	  ulcx=x;
	if (x>lrcx)
	  lrcx=x;
	if (y<ulcy)
	  ulcy=y;
	if (y>lrcy)
	  lrcy=y;
      }
    }
  }
  pbb[0]=ulcx;
  pbb[1]=ulcy;
  pbb[2]=lrcx;
  pbb[3]=lrcy;
  return bb_im;
}
#include "uc_undef.h"

#include "us_def.h"
IMAGE *us_getboundingbox(IMAGE *im)
{
  IMAGE *bb_im;
  PIX_TYPE *pim;
  INT32 *pbb, ulcx=GetImNx(im), ulcy=GetImNy(im), lrcx=0, lrcy=0;
  int x,y,nx=GetImNx(im),ny=GetImNy(im);

  bb_im=create_image(t_INT32, 4, 1, 1);
  pbb=(INT32 *)GetImPtr(bb_im);
  pim=(PIX_TYPE *)GetImPtr(im);

  for (y=0;y<ny;y++){
    for (x=0;x<nx;x++){
      if (*pim++){
	if (x<ulcx)
	  ulcx=x;
	if (x>lrcx)
	  lrcx=x;
	if (y<ulcy)
	  ulcy=y;
	if (y>lrcy)
	  lrcy=y;
      }
    }
  }
  pbb[0]=ulcx;
  pbb[1]=ulcy;
  pbb[2]=lrcx;
  pbb[3]=lrcy;
  return bb_im;
}
#include "us_undef.h"

#include "f_def.h"
IMAGE *f_getboundingbox(IMAGE *im)
{
  IMAGE *bb_im;
  PIX_TYPE *pim;
  INT32 *pbb, ulcx=GetImNx(im), ulcy=GetImNy(im), lrcx=0, lrcy=0;
  int x,y,nx=GetImNx(im),ny=GetImNy(im);

  bb_im=create_image(t_INT32, 4, 1, 1);
  pbb=(INT32 *)GetImPtr(bb_im);
  pim=(PIX_TYPE *)GetImPtr(im);

  for (y=0;y<ny;y++){
    for (x=0;x<nx;x++){
      if (*pim++){
	if (x<ulcx)
	  ulcx=x;
	if (x>lrcx)
	  lrcx=x;
	if (y<ulcy)
	  ulcy=y;
	if (y>lrcy)
	  lrcy=y;
      }
    }
  }
  pbb[0]=ulcx;
  pbb[1]=ulcy;
  pbb[2]=lrcx;
  pbb[3]=lrcy;
  return bb_im;
}
#include "f_undef.h"


IMAGE *getboundingbox(IMAGE *im)
{
  switch (GetImDataType(im)){
  case t_UCHAR:
    return(uc_getboundingbox(im));
    break;
  case t_USHORT:
    return(us_getboundingbox(im));
    break;
  case t_FLOAT:
    return(f_getboundingbox(im));
    break;
  default:
    (void)sprintf(buf,"getboundingbox(im): invalid pixel type\n"); errputstr(buf);
    return(NULL);
  }
  return(NULL);
}


#include "uc_def.h"
IMAGE *uc_magnify(IMAGE *im, int n)
{
  IMAGE *imout;
  PIX_TYPE *p, *pout;
  int nx=GetImNx(im);
  int ny=GetImNy(im);
  int x, y, i, j;

  imout=(IMAGE *)create_image(t_PIX_TYPE, n*nx, n*ny, 1);
  if (imout==NULL){
    return NULL;
  }

  p=(PIX_TYPE *)GetImPtr(im);
  pout=(PIX_TYPE *)GetImPtr(imout);

  for (y=0;y<ny;y++){
    for (x=0;x<nx;x++){
      for (i=0; i<n;i++){
	for (j=0; j<n; j++){
	  pout[x*n+i+nx*n*n*y+j*nx*n]=*p;
	}
      }
      p++;
    }
  }
  return imout;
}
#include "uc_undef.h"

#include "us_def.h"
IMAGE *us_magnify(IMAGE *im, int n)
{
  IMAGE *imout;
  PIX_TYPE *p, *pout;
  int nx=GetImNx(im);
  int ny=GetImNy(im);
  int x, y, i, j;

  imout=(IMAGE *)create_image(t_PIX_TYPE, n*nx, n*ny, 1);
  if (imout==NULL){
    return NULL;
  }

  p=(PIX_TYPE *)GetImPtr(im);
  pout=(PIX_TYPE *)GetImPtr(imout);

  for (y=0;y<ny;y++){
    for (x=0;x<nx;x++){
      for (i=0; i<n;i++){
	for (j=0; j<n; j++){
	  pout[x*n+i+nx*n*n*y+j*nx*n]=*p;
	}
      }
      p++;
    }
  }
  return imout;
}
#include "us_undef.h"

#include "u32_def.h"
IMAGE *u32_magnify(IMAGE *im, int n)
{
  IMAGE *imout;
  PIX_TYPE *p, *pout;
  int nx=GetImNx(im);
  int ny=GetImNy(im);
  int x, y, i, j;

  imout=(IMAGE *)create_image(t_PIX_TYPE, n*nx, n*ny, 1);
  if (imout==NULL){
    return NULL;
  }

  p=(PIX_TYPE *)GetImPtr(im);
  pout=(PIX_TYPE *)GetImPtr(imout);

  for (y=0;y<ny;y++){
    for (x=0;x<nx;x++){
      for (i=0; i<n;i++){
	for (j=0; j<n; j++){
	  pout[x*n+i+nx*n*n*y+j*nx*n]=*p;
	}
      }
      p++;
    }
  }
  return imout;
}
#include "u32_undef.h"

IMAGE *magnify(IMAGE *im, int n)
{
  if (GetImNz(im) > 1){
    (void)sprintf(buf,"magnify(im, n): input image im must be 2-D\n"); errputstr(buf);
    return(NULL);
  }
  switch (GetImDataType(im)){
  case t_UCHAR:
    return(uc_magnify(im, n));
    break;
  case t_USHORT:
    return(us_magnify(im, n));
    break;
  case t_UINT32:
  case t_MIALFLOAT:
    return(u32_magnify(im, n));
    break;
  default:
    (void)sprintf(buf,"magnify(im, n): invalid pixel type\n"); errputstr(buf);
    return(NULL);
  }
  return(NULL);
}




#define rotate_x(x, y, sint, cost) ((cost * x) + (sint * y))
#define rotate_y(x, y, sint, cost) ((cost * y) - (sint * x))

#include "uc_def.h"
IMAGE **uc_rotatecoor(IMAGE *im, double theta)
{
  IMAGE *imx, *imy, **imout;
  MIALFLOAT *pimx, *pimy;
  int nx=GetImNx(im);
  int ny=GetImNy(im);
  int x, y, xc, yc, nxp, nyp;
  double cost=cos(theta), sint=sin(theta);
  double urc_x, llc_x, lrc_x, urc_y, llc_y, lrc_y;
  double xmin, xmax, ymin, ymax;

  urc_x=rotate_x(nx, 0, sint, cost);
  urc_y=rotate_y(nx, 0, sint, cost);
  llc_x=rotate_x(0, ny, sint, cost);
  llc_y=rotate_y(0, ny, sint, cost);
  lrc_x=rotate_x(nx, ny,sint, cost);
  lrc_y=rotate_y(nx, ny,sint, cost);

  xmin=MIN(0.0,MIN(urc_x,MIN(llc_x,lrc_x)));
  xmax=MAX(0.0,MAX(urc_x,MAX(llc_x,lrc_x)));
  ymin=MIN(0.0,MIN(urc_y,MIN(llc_y,lrc_y)));
  ymax=MAX(0.0,MAX(urc_y,MAX(llc_x,lrc_y)));

  nxp=(int)round(abs(xmax-xmin))+1;
  nyp=(int)round(abs(ymax-ymin))+1;

  imout=(IMAGE **)calloc(sizeof(IMAGE *), 2);
  imx=create_image(t_FLOAT, nxp, nyp, 1);
  if (imx==NULL)
    return NULL;
  imy=create_image(t_FLOAT, nxp, nyp, 1);
  if (imy==NULL){
    free_image(imx);
    return NULL;
  }
  f_blank(imx, -100.0);
  f_blank(imy, -100.0);

  pimx=(MIALFLOAT *)GetImPtr(imx);
  pimy=(MIALFLOAT *)GetImPtr(imy);
  // printf("xmin=%d xmax=%d ymin=%d ymax=%d", (int)xmin, (int)xmax, (int)ymin, (int)ymax);
  for (x=floor(xmin),xc=0; x<=xmax; x++, xc++){
    for (y=floor(ymin),yc=0; y<=ymax; y++, yc++){
      // printf("rotate_x(%d,%d,-sint,cost)=%f\n", x, y, (float)rotate_x(x,y,-sint,cost));
      *(pimx+xc+yc*nxp)=(float)rotate_x(x,y,-sint,cost);
      *(pimy+xc+yc*nxp)=(float)rotate_y(x,y,-sint,cost);
    }
  }

  imout[0]=imx;
  imout[1]=imy;

  return imout;
}
#include "uc_undef.h"


#include "us_def.h"
IMAGE **us_rotatecoor(IMAGE *im, double theta)
{
  IMAGE *imx, *imy, **imout;
  MIALFLOAT *pimx, *pimy;
  int nx=GetImNx(im);
  int ny=GetImNy(im);
  int x, y, xc, yc, nxp, nyp;
  double cost=cos(theta), sint=sin(theta);
  double urc_x, llc_x, lrc_x, urc_y, llc_y, lrc_y;
  double xmin, xmax, ymin, ymax;

  urc_x=rotate_x(nx, 0, sint, cost);
  urc_y=rotate_y(nx, 0, sint, cost);
  llc_x=rotate_x(0, ny, sint, cost);
  llc_y=rotate_y(0, ny, sint, cost);
  lrc_x=rotate_x(nx, ny,sint, cost);
  lrc_y=rotate_y(nx, ny,sint, cost);

  xmin=MIN(0.0,MIN(urc_x,MIN(llc_x,lrc_x)));
  xmax=MAX(0.0,MAX(urc_x,MAX(llc_x,lrc_x)));
  ymin=MIN(0.0,MIN(urc_y,MIN(llc_y,lrc_y)));
  ymax=MAX(0.0,MAX(urc_y,MAX(llc_x,lrc_y)));

  nxp=(int)round(abs(xmax-xmin))+1;
  nyp=(int)round(abs(ymax-ymin))+1;

  imout=(IMAGE **)calloc(sizeof(IMAGE *), 2);
  imx=create_image(t_FLOAT, nxp, nyp, 1);
  if (imx==NULL)
    return NULL;
  imy=create_image(t_FLOAT, nxp, nyp, 1);
  if (imy==NULL){
    free_image(imx);
    return NULL;
  }
  f_blank(imx, -100.0);
  f_blank(imy, -100.0);

  pimx=(MIALFLOAT *)GetImPtr(imx);
  pimy=(MIALFLOAT *)GetImPtr(imy);
  // printf("xmin=%d xmax=%d ymin=%d ymax=%d", (int)xmin, (int)xmax, (int)ymin, (int)ymax);
  for (x=floor(xmin),xc=0; x<=xmax; x++, xc++){
    for (y=floor(ymin),yc=0; y<=ymax; y++, yc++){
      // printf("rotate_x(%d,%d,-sint,cost)=%f\n", x, y, (float)rotate_x(x,y,-sint,cost));
      *(pimx+xc+yc*nxp)=(float)rotate_x(x,y,-sint,cost);
      *(pimy+xc+yc*nxp)=(float)rotate_y(x,y,-sint,cost);
    }
  }

  imout[0]=imx;
  imout[1]=imy;

  return imout;
}
#include "us_undef.h"


IMAGE **rotatecoor(IMAGE *im, double theta)
{
  switch (GetImDataType(im)){
  case t_UCHAR:
    return(uc_rotatecoor(im, theta));
    break;
  case t_USHORT:
    return(uc_rotatecoor(im, theta));
    break;
  default:
    (void)sprintf(buf,"rotatecoor(im, theta): invalid pixel type\n"); errputstr(buf);
    return(NULL);
  }
  return(NULL);
}

/** @}*/
