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
#include <string.h>
#include "miallib.h"

#ifdef OPENMP
#include <omp.h>
#endif


extern void uc_set_shift_and_box_and_weight(unsigned char *im1, UCHAR *im2, int *box, long int x, long int y, long int *shift, UCHAR *weight);

typedef struct tms Ttime;




/** @defgroup group_erodil Erosions and dilations
 *  Functions dealing with mathematical morphology erosions and dilations.
 *  @{
 */



#include "uc_def.h"
IMAGE *uc_erode2(IMAGE *im, IMAGE *imse, int ox, int oy, int oz, int trflag)
     /* slower (and using realloc) version whereby a frame is added and then subtracted */
{
  IMAGE *imout;
  int box[BOXELEM];
  long int *shft, n;
  PIX_TYPE *p1, *p2, *im1;
  long int k, x, y, z;
  long int lstx, lsty, lstz;
  int nx,ny,nz;

  /* create shift array */
  n = objectpix(imse);
  if (n==0) /* no point in SE */
    return NULL;
  shft = (long int *)calloc(n, sizeof(long int));
  if (shft == NULL)
    return NULL;

  /* create output image */
  imout = (IMAGE *)create_image(GetImDataType(im), GetImNx(im), GetImNy(im), GetImNz(im));
  if (imout == NULL){
    (void)sprintf(buf,"erode(): not enough memory!\n"); errputstr(buf);
    return(imout);
  }
  nx=GetImNx(imse);
  ny=GetImNy(imse);
  nz=GetImNz(imse);

  if (trflag){
    ox=nx-ox-1;
    oy=ny-oy-1;
    oz=nz-oz-1;
  }

  /* set box values */
  if (ox < 0) box[0] = 0;
  else box[0] = ox;
  if (ox >= nx) box[1] = 0;
  else box[1] = nx - 1 - ox;

  if (oy < 0) box[2] = 0;
  else box[2] = oy;
  if (oy >= ny) box[3] = 0;
  else box[3] = ny - 1 - oy;

  if (oz < 0) box[4] = 0;
  else box[4] = oz;
  if (oz >= nz) box[5] = 0;
  else box[5] = nz - 1 - oz;

  generic_addframebox(im, box, PIX_MAX);

  nx=GetImNx(im);
  ny=GetImNy(im);
  nz=GetImNz(im);

  /*  set shifts  */
  box[0] = GetImNx(imse);
  box[1] = GetImNy(imse);
  box[2] = GetImNz(imse);
  box[3] = ox;
  box[4] = oy;
  box[5] = oz;
  set_shift_and_box((unsigned char *)GetImPtr(imse), box, GetImNx(im), GetImNy(im), shft);

  /* here we go */
  lstx = nx - box[1];
  lsty = ny - box[3];
  lstz = nz - box[5];
  p2=(PIX_TYPE *)GetImPtr(imout);
  im1=(PIX_TYPE *)GetImPtr(im);

  for (z = box[4]; z < lstz; z++){
    p1 = im1 + nx * ny * z;
    p1 += nx * box[2];
    p1 += box[0];
    for (y = box[2]; y < lsty; y++){
      for (x = box[0]; x < lstx; ++x){
	*p2 = *(p1 + shft[0]);
	for (k = 1; k < n; k++){
	  if (*p2 > *(p1 + shft[k]))
	    *p2 = *(p1 + shft[k]);
	}
	p1++;
	p2++;
      }
      p1 += box[0] + box[1];
    }
  }
  subframebox(im, box);

  free((char *) shft);
  return(imout);
}
#include "uc_undef.h"

#include "uc_def.h"
void uc_erode(IMAGE *im, IMAGE *imout, int nx, int ny, int nz, int *box, long int *shft, int n)
{
  PIX_TYPE *im1, *im2, *p1, *p2;
  IMAGE *im_frame;
  long int k, x, y, z;
  long int lstx, lsty, lstz;

  im1 = (PIX_TYPE *)GetImPtr(im);
  im2 = (PIX_TYPE *)GetImPtr(imout);

  lstx = nx - box[1];
  lsty = ny - box[3];
  lstz = nz - box[5];

  im_frame = getframebox(im, box);
  generic_framebox(im, box, PIX_MAX);

  for (z = box[4]; z < lstz; z++){
    p1 = im1 + nx * ny * z;
    p1 += nx * box[2];
    p1 += box[0];
    p2 = im2 + nx * ny * z;
    p2 += nx * box[2];
    p2 += box[0];
    for (y = box[2]; y < lsty; y++){
      for (x = box[0]; x < lstx; ++x){
	*p2 = *(p1 + shft[0]);
	for (k = 1; k < n; k++){
	  if (*p2 > *(p1 + shft[k]))
	    *p2 = *(p1 + shft[k]);
	}
	p1++;
	p2++;
      }
      p1 += box[0] + box[1];
      p2 += box[0] + box[1];
    }
  }
  setframebox(im, im_frame, box);
  free_image(im_frame);
}
#include "uc_undef.h"


#include "us_def.h"
void us_erode(IMAGE *im, IMAGE *imout, int nx, int ny, int nz, int *box, long int *shft, int n)
{
  PIX_TYPE *im1, *im2, *p1, *p2;
  IMAGE *im_frame;
  long int k, x, y, z;
  long int lstx, lsty, lstz;

  im1 = (PIX_TYPE *)GetImPtr(im);
  im2 = (PIX_TYPE *)GetImPtr(imout);

  lstx = nx - box[1];
  lsty = ny - box[3];
  lstz = nz - box[5];

  im_frame = getframebox(im, box);
  us_framebox(im, box, PIX_MAX);

  for (z = box[4]; z < lstz; z++){
    p1 = im1 + nx * ny * z;
    p1 += nx * box[2];
    p1 += box[0];
    p2 = im2 + nx * ny * z;
    p2 += nx * box[2];
    p2 += box[0];
    for (y = box[2]; y < lsty; y++){
      for (x = box[0]; x < lstx; ++x){
	*p2 = *(p1 + shft[0]);
	for (k = 1; k < n; k++){
	  if (*p2 > *(p1 + shft[k]))
	    *p2 = *(p1 + shft[k]);
	}
	p1++;
	p2++;
      }
      p1 += box[0] + box[1];
      p2 += box[0] + box[1];
    }
  }
  setframebox(im, im_frame, box);
  free_image(im_frame);
}
#include "us_undef.h"

#include "i32_def.h"
void i32_erode(IMAGE *im, IMAGE *imout, int nx, int ny, int nz, int *box, long int *shft, int n)
{
  PIX_TYPE *im1, *im2, *p1, *p2;
  IMAGE *im_frame;
  long int k, x, y, z;
  long int lstx, lsty, lstz;

  im1 = (PIX_TYPE *)GetImPtr(im);
  im2 = (PIX_TYPE *)GetImPtr(imout);

  lstx = nx - box[1];
  lsty = ny - box[3];
  lstz = nz - box[5];

  im_frame = getframebox(im, box);
  i32_framebox(im, box, PIX_MAX);

  for (z = box[4]; z < lstz; z++){
    p1 = im1 + nx * ny * z;
    p1 += nx * box[2];
    p1 += box[0];
    p2 = im2 + nx * ny * z;
    p2 += nx * box[2];
    p2 += box[0];
    for (y = box[2]; y < lsty; y++){
      for (x = box[0]; x < lstx; ++x){
	*p2 = *(p1 + shft[0]);
	for (k = 1; k < n; k++){
	  if (*p2 > *(p1 + shft[k]))
	    *p2 = *(p1 + shft[k]);
	}
	p1++;
	p2++;
      }
      p1 += box[0] + box[1];
      p2 += box[0] + box[1];
    }
  }
  setframebox(im, im_frame, box);
  free_image(im_frame);
}
#include "i32_undef.h"



IMAGE *erode(IMAGE *im, IMAGE *imse, int ox, int oy, int oz, int trflag)
{
  IMAGE *imout;
  int box[BOXELEM];
  int n, i, abval;
  long int *shft;

  /* check */
  if (GetImDataType(imse)!=t_UCHAR){
    (void)sprintf(buf,"erode(): imse must be of type UCHAR!\n"); errputstr(buf);
    return NULL;
  }
  /* create shift array */
  n = objectpix(imse);
  if (n==0) /* no point in SE */
    return NULL;
  shft = (long int *)calloc(n, sizeof(long int));
  if (shft == NULL)
    return NULL;

  /* create output image */
  imout = (IMAGE *)create_image(GetImDataType(im), GetImNx(im), GetImNy(im), GetImNz(im));
  if (imout == NULL){
    (void)sprintf(buf,"erode(): not enough memory!\n"); errputstr(buf);
    return(imout);
  }

  /*  Take SE  into account  */
  box[0] = GetImNx(imse);
  box[1] = GetImNy(imse);
  box[2] = GetImNz(imse);
  box[3] = ox;
  box[4] = oy;
  box[5] = oz;
  set_shift_and_box((unsigned char *)GetImPtr(imse), box, GetImNx(im), GetImNy(im), shft);

  if (trflag){ /* reflect SE and box values */
    for (i=0;i<n;i++)
      shft[i]*=-1;

    for (i=0; i<BOXELEM/2; i++){
      abval=box[2*i];
      box[2*i]=box[2*i+1];
      box[2*i+1]=abval;
    }
  }
  switch (GetImDataType(im)){

  case t_UCHAR:
/*     free((char *) shft);  THIS IS FOR THE SLOWER VERSION WHEREBY A FRAME IS ADDED AND THEN REMOVED */
/*     return (uc_erode2(im, imse, ox, oy, oz, trflag));  */
    uc_erode(im, imout, GetImNx(im), GetImNy(im), GetImNz(im), box, shft, n);
    break;
  case t_USHORT:
    us_erode(im, imout, GetImNx(im), GetImNy(im), GetImNz(im), box, shft, n);
    break;
  case t_INT32:
    i32_erode(im, imout, GetImNx(im), GetImNy(im), GetImNz(im), box, shft, n);
    break;
  default:
    (void)sprintf(buf,"erode(): invalid pixel type\n"); errputstr(buf);
    free_image(imout); imout=NULL;
  }
  free((char *) shft);
  return(imout);
}


#include "uc_def.h"
void uc_dilate(IMAGE *im, IMAGE *imout, int nx, int ny, int nz, int *box, long int *shft, int n)
{
  PIX_TYPE *im1, *im2, *p1, *p2;
  IMAGE *im_frame;
  long int x, y, z;
  long int lstx, lsty, lstz;
  long int *pshft, *pshftfin;

  int cnt=0, cnt2=0;


  im1 = (PIX_TYPE *)GetImPtr(im);
  im2 = (PIX_TYPE *)GetImPtr(imout);

  lstx = nx - box[1];
  lsty = ny - box[3];
  lstz = nz - box[5];

  im_frame = getframebox(im, box);
  uc_framebox(im, box, PIX_MIN);

  pshftfin=shft+n;

  for (z = box[4]; z < lstz; z++){
    p1 = im1 + nx * ny * z;
    p1 += nx * box[2];
    p1 += box[0];
    p2 = im2 + nx * ny * z;
    p2 += nx * box[2];
    p2 += box[0];
    for (y = box[2]; y < lsty; y++){
      for (x = box[0]; x < lstx; ++x){
	pshft=shft;
	*p2 = *(p1 + *pshft++);
	for (; pshft < pshftfin; pshft++){
	  if (*p2 < *(p1 + *pshft)){
	    cnt++;
	    *p2 = *(p1 + *pshft);
	  }
	  cnt2++;
	  }
	p1++;
	p2++;
      }
      p1 += box[0] + box[1];
      p2 += box[0] + box[1];
    }
  }
  setframebox(im, im_frame, box);
  free_image(im_frame);
}
#include "uc_undef.h"


#include "us_def.h"
void us_dilate(IMAGE *im, IMAGE *imout, int nx, int ny, int nz, int *box, long int *shft, int n)
{
  PIX_TYPE *im1, *im2, *p1, *p2;
  IMAGE *im_frame;
  long int k, x, y, z;
  long int lstx, lsty, lstz;


  im1 = (PIX_TYPE *)GetImPtr(im);
  im2 = (PIX_TYPE *)GetImPtr(imout);

  lstx = nx - box[1];
  lsty = ny - box[3];
  lstz = nz - box[5];

  im_frame = getframebox(im, box);
  us_framebox(im, box, PIX_MIN);

  for (z = box[4]; z < lstz; z++){
    p1 = im1 + nx * ny * z;
    p1 += nx * box[2];
    p1 += box[0];
    p2 = im2 + nx * ny * z;
    p2 += nx * box[2];
    p2 += box[0];
    for (y = box[2]; y < lsty; y++){
      for (x = box[0]; x < lstx; ++x){
	*p2 = *(p1 + shft[0]);
	for (k = 1; k < n; k++){
	  if (*p2 < *(p1 + shft[k]))
	    *p2 = *(p1 + shft[k]);
	}
	p1++;
	p2++;
      }
      p1 += box[0] + box[1];
      p2 += box[0] + box[1];
    }
  }
  setframebox(im, im_frame, box);
  free_image(im_frame);
}
#include "us_undef.h"


#include "i32_def.h"
void i32_dilate(IMAGE *im, IMAGE *imout, int nx, int ny, int nz, int *box, long int *shft, int n)
{
  PIX_TYPE *im1, *im2, *p1, *p2;
  IMAGE *im_frame;
  long int k, x, y, z;
  long int lstx, lsty, lstz;


  im1 = (PIX_TYPE *)GetImPtr(im);
  im2 = (PIX_TYPE *)GetImPtr(imout);

  lstx = nx - box[1];
  lsty = ny - box[3];
  lstz = nz - box[5];

  im_frame = getframebox(im, box);
  i32_framebox(im, box, PIX_MIN);

  for (z = box[4]; z < lstz; z++){
    p1 = im1 + nx * ny * z;
    p1 += nx * box[2];
    p1 += box[0];
    p2 = im2 + nx * ny * z;
    p2 += nx * box[2];
    p2 += box[0];
    for (y = box[2]; y < lsty; y++){
      for (x = box[0]; x < lstx; ++x){
	*p2 = *(p1 + shft[0]);
	for (k = 1; k < n; k++){
	  if (*p2 < *(p1 + shft[k]))
	    *p2 = *(p1 + shft[k]);
	}
	p1++;
	p2++;
      }
      p1 += box[0] + box[1];
      p2 += box[0] + box[1];
    }
  }
  setframebox(im, im_frame, box);
  free_image(im_frame);
}
#include "i32_undef.h"



IMAGE *dilate(IMAGE *im, IMAGE *imse, int ox, int oy, int oz, int trflag)
{
  IMAGE *imout;
  int box[BOXELEM];
  int n, i, abval;
  long int *shft;

  /* check */
  if (GetImDataType(imse)!=t_UCHAR){
    (void)sprintf(buf,"dilate(): imse must be of type UCHAR!\n"); errputstr(buf);
    return NULL;
  }

  /* create shift array */
  n = objectpix(imse);
  if (n==0) /* no point in SE */
    return NULL;
  shft = (long int *)calloc(n, sizeof(long int));
  if (shft == NULL)
    return NULL;

  /* create output image */
  imout = (IMAGE *)create_image(GetImDataType(im), GetImNx(im), GetImNy(im), GetImNz(im));
  if (imout == NULL){
    (void)sprintf(buf,"dilate(): not enough memory!\n"); errputstr(buf);
    return(imout);
  }

  /*  Take SE  into account  */
  box[0] = GetImNx(imse);
  box[1] = GetImNy(imse);
  box[2] = GetImNz(imse);
  box[3] = ox;
  box[4] = oy;
  box[5] = oz;
  set_shift_and_box((unsigned char *)GetImPtr(imse), box, GetImNx(im), GetImNy(im), shft);

  if (trflag){ /* reflect SE and box values */
    for (i=0;i<n;i++)
      shft[i]*=-1;

    for (i=0; i<BOXELEM/2; i++){
      abval=box[2*i];
      box[2*i]=box[2*i+1];
      box[2*i+1]=abval;
    }
  }

  switch (GetImDataType(im)){
  case t_UCHAR:
    uc_dilate(im, imout, GetImNx(im), GetImNy(im), GetImNz(im), box, shft, n);
    break;
  case t_USHORT:
    us_dilate(im, imout, GetImNx(im), GetImNy(im), GetImNz(im), box, shft, n);
    break;
  case t_INT32:
    i32_dilate(im, imout, GetImNx(im), GetImNy(im), GetImNz(im), box, shft, n);
    break;
  default:
    (void)sprintf(buf,"dilate(): invalid pixel type\n"); errputstr(buf);
    free_image(imout); imout=NULL;
  }
  free((char *) shft);
  return(imout);
}


#include "uc_def.h"
void uc_volerode(im1, im2, nx, ny, nz, box, shft, weight, n)
     PIX_TYPE *im1, *im2;
     long int nx, ny, nz;
     int *box;
     long int *shft;
     UCHAR *weight;
     long int n;
{
  PIX_TYPE *p1, *p2;
  long int k, x, y, z;
  long int lstx, lsty, lstz;

  lstx = nx - box[1];
  lsty = ny - box[3];
  lstz = nz - box[5];

  for (z = box[4]; z < lstz; z++){
    p1 = im1 + nx * ny * z;
    p1 += nx * box[2];
    p1 += box[0];
    p2 = im2 + nx * ny * z;
    p2 += nx * box[2];
    p2 += box[0];
    for (y = box[2]; y < lsty; y++){
      for (x = box[0]; x < lstx; ++x){
	*p2 = *(p1 + shft[0]) + weight[0];
	for (k = 1; k < n; k++){
	  if (*p2 > *(p1 + shft[k]) + weight[k])
	    *p2 = *(p1 + shft[k]) + weight[k];
	}
	p1++;
	p2++;
      }
      p1 += box[0] + box[1];
      p2 += box[0] + box[1];
    }
  }
}
#include "uc_undef.h"


IMAGE *volerode(IMAGE *im, IMAGE *imse, IMAGE *imweight, int ox, int oy, int oz)
{
  IMAGE *imout=NULL, *imtmp=NULL;
  int box[BOXELEM];
  int n;
  long int *shft;
  UCHAR *ptrf, *weight;


  /* make sure image of weights is of UCHAR type */
  if ((GetImDataType(imweight)!=t_UCHAR) || (GetImDataType(imse)!=t_UCHAR) ){
    return NULL;
  }
  else

  /* create shift array */
  n = objectpix(imse);
  if (n==ERROR) /* no point in SE */
    return NULL;
  shft = (long int *)calloc(n, sizeof(long int));
  if (shft == NULL)
    return NULL;
  weight = (UCHAR *)calloc(n, sizeof(UCHAR));
  if (weight == NULL){
    free(shft);
    return NULL;
  }

  /* create output image */
  imout = (IMAGE *)create_image(GetImDataType(im), GetImNx(im), GetImNy(im), GetImNz(im));
  if (imout == NULL){
    free(shft); free(weight);
    (void)sprintf(buf,"volerode(): not enough memory!\n"); errputstr(buf);
    return(imout);
  }

  /*  Take SE  into account  */
  box[0] = GetImNx(imse);
  box[1] = GetImNy(imse);
  box[2] = GetImNz(imse);
  box[3] = ox;
  box[4] = oy;
  box[5] = oz;
  ptrf=(UCHAR *)GetImPtr(imtmp);
  uc_set_shift_and_box_and_weight((unsigned char *)GetImPtr(imse), ptrf, box, \
			       GetImNx(im), GetImNy(im), shft, weight);

  if (imtmp!=NULL)
      free_image(imtmp);

  switch (GetImDataType(im)){

  case t_UCHAR:
    uc_volerode((UCHAR *)GetImPtr(im), (UCHAR *)GetImPtr(imout), GetImNx(im), GetImNy(im), GetImNz(im), box, shft, weight, n);
    break;


  default:
    (void)sprintf(buf,"volerode(): invalid pixel type\n"); errputstr(buf);
    free_image(imout); imout=NULL;
  }
  free((char *) shft);
  free((char *) weight);
  return(imout);
}

#include "i32_def.h"
void i32_voldilate(im1, im2, nx, ny, nz, box, shft, weight, n)
     PIX_TYPE *im1, *im2;
     long int nx, ny, nz;
     int *box;
     long int *shft, *weight;
     long int n;
{
  PIX_TYPE *p1, *p2;
  long int k, x, y, z;
  long int lstx, lsty, lstz;

  lstx = nx - box[1];
  lsty = ny - box[3];
  lstz = nz - box[5];

  for (z = box[4]; z < lstz; z++){
    p1 = im1 + nx * ny * z;
    p1 += nx * box[2];
    p1 += box[0];
    p2 = im2 + nx * ny * z;
    p2 += nx * box[2];
    p2 += box[0];
    for (y = box[2]; y < lsty; y++){
      for (x = box[0]; x < lstx; ++x){
	*p2 = *(p1 + shft[0]) + weight[0];
	for (k = 1; k < n; k++){
	  if (*p2 < *(p1 + shft[k]) + weight[k])
	    *p2 = *(p1 + shft[k]) + weight[k];
	}
	p1++;
	p2++;
      }
      p1 += box[0] + box[1];
      p2 += box[0] + box[1];
    }
  }
}
#include "i32_undef.h"


#include "uc_def.h"
ERROR_TYPE uc_erode4(IMAGE *im, int ox, int oy)
{
  PIX_TYPE *pim, *pci, out, *p1, *p2, *p3, *p[3];
  PIX_TYPE *pc1, *pc2, *pc3, *pc4, *pc5;
  int nx=GetImNx(im), ny=GetImNy(im);
  int i, x, bnx=nx+2;

  pim=(PIX_TYPE *)GetImPtr(im);
  pci=pim;
  p1=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  p2=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  p3=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  for(i=0; i<bnx;i++){
    p1[i]=PIX_MAX;
    p2[i]=PIX_MAX;
    p3[i]=PIX_MAX;
  }
  p[0]=p1;
  p[1]=p2;
  p[2]=p3;
  for(i=0;i<3-oy;i++)
    memcpy(p[i+oy]+ox,(pim+i*nx),nx*sizeof(PIX_TYPE));

  for(i=0; i<ny; i++){
    pc1=p1+1;
    pc2=p2;
    pc3=p2+1;
    pc4=p2+2;
    pc5=p3+1;

#pragma omp parallel for private(out)
    for(x=0;x<nx;x++){
      out=pc1[x];
      if(pc2[x]<out)
	out=pc2[x];
      if(pc3[x]<out)
	out=pc3[x];
      if(pc4[x]<out)
	out=pc4[x];
      if(pc5[x]<out)
	out=pc5[x];
      pci[x]=out;
    }
    pci+=nx;
    pc1=p1;
    p1=p2;
    p2=p3;
    p3=pc1;
    if(i<ny+oy-3)
      memcpy(p3+ox,(pim+(i-oy+3)*nx),nx*sizeof(PIX_TYPE));
    else
      for (x=0; x<bnx; x++)
	p3[x]=PIX_MAX;
  }
  free(p1); free(p2); free(p3);
  return NO_ERROR;
}
#include "uc_undef.h"

#include "us_def.h"
ERROR_TYPE us_erode4(IMAGE *im, int ox, int oy)
{
  PIX_TYPE *pim, *pci, out, *p1, *p2, *p3, *p[3];
  PIX_TYPE *pc1, *pc2, *pc3, *pc4, *pc5;
  int nx=GetImNx(im), ny=GetImNy(im);
  int i, x, bnx=nx+2;

  pim=(PIX_TYPE *)GetImPtr(im);
  pci=pim;
  p1=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  p2=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  p3=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  for(i=0; i<bnx;i++){
    p1[i]=PIX_MAX;
    p2[i]=PIX_MAX;
    p3[i]=PIX_MAX;
  }
  p[0]=p1;
  p[1]=p2;
  p[2]=p3;
  for(i=0;i<3-oy;i++)
    memcpy(p[i+oy]+ox,(pim+i*nx),nx*sizeof(PIX_TYPE));

  for(i=0; i<ny; i++){
    pc1=p1+1;
    pc2=p2;
    pc3=p2+1;
    pc4=p2+2;
    pc5=p3+1;

#pragma omp parallel for private(out)
    for(x=0;x<nx;x++){
      out=pc1[x];
      if(pc2[x]<out)
	out=pc2[x];
      if(pc3[x]<out)
	out=pc3[x];
      if(pc4[x]<out)
	out=pc4[x];
      if(pc5[x]<out)
	out=pc5[x];
      pci[x]=out;
    }
    pci+=nx;
    pc1=p1;
    p1=p2;
    p2=p3;
    p3=pc1;
    if(i<ny+oy-3)
      memcpy(p3+ox,(pim+(i-oy+3)*nx),nx*sizeof(PIX_TYPE));
    else
      for (x=0; x<bnx; x++)
	p3[x]=PIX_MAX;
  }
  free(p1); free(p2); free(p3);
  return NO_ERROR;
}
#include "us_undef.h"

#include "i32_def.h"
ERROR_TYPE i32_erode4(IMAGE *im, int ox, int oy)
{
  PIX_TYPE *pim, *pci, out, *p1, *p2, *p3, *p[3];
  PIX_TYPE *pc1, *pc2, *pc3, *pc4, *pc5;
  int nx=GetImNx(im), ny=GetImNy(im);
  int i, x, bnx=nx+2;

  pim=(PIX_TYPE *)GetImPtr(im);
  pci=pim;
  p1=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  p2=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  p3=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  for(i=0; i<bnx;i++){
    p1[i]=PIX_MAX;
    p2[i]=PIX_MAX;
    p3[i]=PIX_MAX;
  }
  p[0]=p1;
  p[1]=p2;
  p[2]=p3;
  for(i=0;i<3-oy;i++)
    memcpy(p[i+oy]+ox,(pim+i*nx),nx*sizeof(PIX_TYPE));

  for(i=0; i<ny; i++){
    pc1=p1+1;
    pc2=p2;
    pc3=p2+1;
    pc4=p2+2;
    pc5=p3+1;

#pragma omp parallel for private(out)
    for(x=0;x<nx;x++){
      out=pc1[x];
      if(pc2[x]<out)
	out=pc2[x];
      if(pc3[x]<out)
	out=pc3[x];
      if(pc4[x]<out)
	out=pc4[x];
      if(pc5[x]<out)
	out=pc5[x];
      pci[x]=out;
    }
    pci+=nx;
    pc1=p1;
    p1=p2;
    p2=p3;
    p3=pc1;
    if(i<ny+oy-3)
      memcpy(p3+ox,(pim+(i-oy+3)*nx),nx*sizeof(PIX_TYPE));
    else
      for (x=0; x<bnx; x++)
	p3[x]=PIX_MAX;
  }
  free(p1); free(p2); free(p3);
  return NO_ERROR;
}
#include "i32_undef.h"

#include "u32_def.h"
ERROR_TYPE u32_erode4(IMAGE *im, int ox, int oy)
{
  PIX_TYPE *pim, *pci, out, *p1, *p2, *p3, *p[3];
  PIX_TYPE *pc1, *pc2, *pc3, *pc4, *pc5;
  int nx=GetImNx(im), ny=GetImNy(im);
  int i, x, bnx=nx+2;

  pim=(PIX_TYPE *)GetImPtr(im);
  pci=pim;
  p1=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  p2=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  p3=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  for(i=0; i<bnx;i++){
    p1[i]=PIX_MAX;
    p2[i]=PIX_MAX;
    p3[i]=PIX_MAX;
  }
  p[0]=p1;
  p[1]=p2;
  p[2]=p3;
  for(i=0;i<3-oy;i++)
    memcpy(p[i+oy]+ox,(pim+i*nx),nx*sizeof(PIX_TYPE));

  for(i=0; i<ny; i++){
    pc1=p1+1;
    pc2=p2;
    pc3=p2+1;
    pc4=p2+2;
    pc5=p3+1;

#pragma omp parallel for private(out)
    for(x=0;x<nx;x++){
      out=pc1[x];
      if(pc2[x]<out)
	out=pc2[x];
      if(pc3[x]<out)
	out=pc3[x];
      if(pc4[x]<out)
	out=pc4[x];
      if(pc5[x]<out)
	out=pc5[x];
      pci[x]=out;
    }
    pci+=nx;
    pc1=p1;
    p1=p2;
    p2=p3;
    p3=pc1;
    if(i<ny+oy-3)
      memcpy(p3+ox,(pim+(i-oy+3)*nx),nx*sizeof(PIX_TYPE));
    else
      for (x=0; x<bnx; x++)
	p3[x]=PIX_MAX;
  }
  free(p1); free(p2); free(p3);
  return NO_ERROR;
}
#include "u32_undef.h"


#include "f_def.h"
ERROR_TYPE f_erode4(IMAGE *im, int ox, int oy)
{
  PIX_TYPE *pim, *pci, out, *p1, *p2, *p3, *p[3];
  PIX_TYPE *pc1, *pc2, *pc3, *pc4, *pc5;
  int nx=GetImNx(im), ny=GetImNy(im);
  int i, x, bnx=nx+2;

  pim=(PIX_TYPE *)GetImPtr(im);
  pci=pim;
  p1=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  p2=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  p3=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  for(i=0; i<bnx;i++){
    p1[i]=PIX_MAX;
    p2[i]=PIX_MAX;
    p3[i]=PIX_MAX;
  }
  p[0]=p1;
  p[1]=p2;
  p[2]=p3;
  for(i=0;i<3-oy;i++)
    memcpy(p[i+oy]+ox,(pim+i*nx),nx*sizeof(PIX_TYPE));

  for(i=0; i<ny; i++){
    pc1=p1+1;
    pc2=p2;
    pc3=p2+1;
    pc4=p2+2;
    pc5=p3+1;

#pragma omp parallel for private(out)
    for(x=0;x<nx;x++){
      out=pc1[x];
      if(pc2[x]<out)
	out=pc2[x];
      if(pc3[x]<out)
	out=pc3[x];
      if(pc4[x]<out)
	out=pc4[x];
      if(pc5[x]<out)
	out=pc5[x];
      pci[x]=out;
    }
    pci+=nx;
    pc1=p1;
    p1=p2;
    p2=p3;
    p3=pc1;
    if(i<ny+oy-3)
      memcpy(p3+ox,(pim+(i-oy+3)*nx),nx*sizeof(PIX_TYPE));
    else
      for (x=0; x<bnx; x++)
	p3[x]=PIX_MAX;
  }
  free(p1); free(p2); free(p3);
  return NO_ERROR;
}
#include "f_undef.h"


ERROR_TYPE erode4(IMAGE *im, int ox, int oy)
{
  if ( (ox > 2) || (oy > 2) ){
    (void)sprintf(buf,"erode4(im): ox and oy must be <= 2\n"); errputstr(buf);
    return(ERROR);
  }

  switch (GetImDataType(im)){
  case t_UCHAR:
    return(uc_erode4(im, ox, oy));
    break;
  case t_USHORT:
    return(us_erode4(im, ox, oy));
    break;
  case t_INT32:
    return(i32_erode4(im, ox, oy));
    break;
  case t_UINT32:
    return(u32_erode4(im, ox, oy));
    break;
  case t_FLOAT:
    return(f_erode4(im, ox, oy));
    break;
  default:
    (void)sprintf(buf,"erode4(im): invalid pixel type\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}


#include "uc_def.h"
ERROR_TYPE uc_dilate4(IMAGE *im, int ox, int oy)
{
  PIX_TYPE *pim, *pci, out, *p1, *p2, *p3, *p[3];
  PIX_TYPE *pc1, *pc2, *pc3, *pc4, *pc5;
  int nx=GetImNx(im), ny=GetImNy(im);
  int i, x, bnx=nx+2;

  pim=(PIX_TYPE *)GetImPtr(im);
  pci=pim;
  p1=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  p2=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  p3=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  if (PIX_MIN != 0){
    for(i=0; i<bnx;i++){
      p1[i]=PIX_MIN;
      p2[i]=PIX_MIN;
      p3[i]=PIX_MIN;
    }
  }
  p[0]=p1;
  p[1]=p2;
  p[2]=p3;
  for(i=0;i<3-oy;i++)
    memcpy(p[i+oy]+ox,(pim+i*nx),nx*sizeof(PIX_TYPE));

  for(i=0; i<ny; i++){
    pc1=p1+1;
    pc2=p2;
    pc3=p2+1;
    pc4=p2+2;
    pc5=p3+1;

#pragma omp parallel for private(out)
    for(x=0;x<nx;x++){
      out=pc1[x];
      if(pc2[x]>out)
	out=pc2[x];
      if(pc3[x]>out)
	out=pc3[x];
      if(pc4[x]>out)
	out=pc4[x];
      if(pc5[x]>out)
	out=pc5[x];
      pci[x]=out;
    }
    pci+=nx;
    pc1=p1;
    p1=p2;
    p2=p3;
    p3=pc1;
    if(i<ny+oy-3)
      memcpy(p3+ox,(pim+(i-oy+3)*nx),nx*sizeof(PIX_TYPE));
    else
      for (x=0; x<bnx; x++)
	p3[x]=PIX_MIN;
  }
  free(p1); free(p2); free(p3);
  return NO_ERROR;
}
#include "uc_undef.h"

#include "us_def.h"
ERROR_TYPE us_dilate4(IMAGE *im, int ox, int oy)
{
  PIX_TYPE *pim, *pci, out, *p1, *p2, *p3, *p[3];
  PIX_TYPE *pc1, *pc2, *pc3, *pc4, *pc5;
  int nx=GetImNx(im), ny=GetImNy(im);
  int i, x, bnx=nx+2;

  pim=(PIX_TYPE *)GetImPtr(im);
  pci=pim;
  p1=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  p2=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  p3=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  if (PIX_MIN != 0){
    for(i=0; i<bnx;i++){
      p1[i]=PIX_MIN;
      p2[i]=PIX_MIN;
      p3[i]=PIX_MIN;
    }
  }
  p[0]=p1;
  p[1]=p2;
  p[2]=p3;
  for(i=0;i<3-oy;i++)
    memcpy(p[i+oy]+ox,(pim+i*nx),nx*sizeof(PIX_TYPE));

  for(i=0; i<ny; i++){
    pc1=p1+1;
    pc2=p2;
    pc3=p2+1;
    pc4=p2+2;
    pc5=p3+1;

#pragma omp parallel for private(out)
    for(x=0;x<nx;x++){
      out=pc1[x];
      if(pc2[x]>out)
	out=pc2[x];
      if(pc3[x]>out)
	out=pc3[x];
      if(pc4[x]>out)
	out=pc4[x];
      if(pc5[x]>out)
	out=pc5[x];
      pci[x]=out;
    }
    pci+=nx;
    pc1=p1;
    p1=p2;
    p2=p3;
    p3=pc1;
    if(i<ny+oy-3)
      memcpy(p3+ox,(pim+(i-oy+3)*nx),nx*sizeof(PIX_TYPE));
    else
      for (x=0; x<bnx; x++)
	p3[x]=PIX_MIN;
  }
  free(p1); free(p2); free(p3);
  return NO_ERROR;
}
#include "us_undef.h"

#include "i32_def.h"
ERROR_TYPE i32_dilate4(IMAGE *im, int ox, int oy)
{
  PIX_TYPE *pim, *pci, out, *p1, *p2, *p3, *p[3];
  PIX_TYPE *pc1, *pc2, *pc3, *pc4, *pc5;
  int nx=GetImNx(im), ny=GetImNy(im);
  int i, x, bnx=nx+2;

  pim=(PIX_TYPE *)GetImPtr(im);
  pci=pim;
  p1=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  p2=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  p3=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  if (PIX_MIN != 0){
    for(i=0; i<bnx;i++){
      p1[i]=PIX_MIN;
      p2[i]=PIX_MIN;
      p3[i]=PIX_MIN;
    }
  }
  p[0]=p1;
  p[1]=p2;
  p[2]=p3;
  for(i=0;i<3-oy;i++)
    memcpy(p[i+oy]+ox,(pim+i*nx),nx*sizeof(PIX_TYPE));

  for(i=0; i<ny; i++){
    pc1=p1+1;
    pc2=p2;
    pc3=p2+1;
    pc4=p2+2;
    pc5=p3+1;

#pragma omp parallel for private(out)
    for(x=0;x<nx;x++){
      out=pc1[x];
      if(pc2[x]>out)
	out=pc2[x];
      if(pc3[x]>out)
	out=pc3[x];
      if(pc4[x]>out)
	out=pc4[x];
      if(pc5[x]>out)
	out=pc5[x];
      pci[x]=out;
    }
    pci+=nx;
    pc1=p1;
    p1=p2;
    p2=p3;
    p3=pc1;
    if(i<ny+oy-3)
      memcpy(p3+ox,(pim+(i-oy+3)*nx),nx*sizeof(PIX_TYPE));
    else
      for (x=0; x<bnx; x++)
	p3[x]=PIX_MIN;
  }
  free(p1); free(p2); free(p3);
  return NO_ERROR;
}
#include "i32_undef.h"

#include "u32_def.h"
ERROR_TYPE u32_dilate4(IMAGE *im, int ox, int oy)
{
  PIX_TYPE *pim, *pci, out, *p1, *p2, *p3, *p[3];
  PIX_TYPE *pc1, *pc2, *pc3, *pc4, *pc5;
  int nx=GetImNx(im), ny=GetImNy(im);
  int i, x, bnx=nx+2;

  pim=(PIX_TYPE *)GetImPtr(im);
  pci=pim;
  p1=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  p2=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  p3=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  if (PIX_MIN != 0){
    for(i=0; i<bnx;i++){
      p1[i]=PIX_MIN;
      p2[i]=PIX_MIN;
      p3[i]=PIX_MIN;
    }
  }
  p[0]=p1;
  p[1]=p2;
  p[2]=p3;
  for(i=0;i<3-oy;i++)
    memcpy(p[i+oy]+ox,(pim+i*nx),nx*sizeof(PIX_TYPE));

  for(i=0; i<ny; i++){
    pc1=p1+1;
    pc2=p2;
    pc3=p2+1;
    pc4=p2+2;
    pc5=p3+1;

#pragma omp parallel for private(out)
    for(x=0;x<nx;x++){
      out=pc1[x];
      if(pc2[x]>out)
	out=pc2[x];
      if(pc3[x]>out)
	out=pc3[x];
      if(pc4[x]>out)
	out=pc4[x];
      if(pc5[x]>out)
	out=pc5[x];
      pci[x]=out;
    }
    pci+=nx;
    pc1=p1;
    p1=p2;
    p2=p3;
    p3=pc1;
    if(i<ny+oy-3)
      memcpy(p3+ox,(pim+(i-oy+3)*nx),nx*sizeof(PIX_TYPE));
    else
      for (x=0; x<bnx; x++)
	p3[x]=PIX_MIN;
  }
  free(p1); free(p2); free(p3);
  return NO_ERROR;
}
#include "u32_undef.h"

#include "f_def.h"
ERROR_TYPE f_dilate4(IMAGE *im, int ox, int oy)
{
  PIX_TYPE *pim, *pci, out, *p1, *p2, *p3, *p[3];
  PIX_TYPE *pc1, *pc2, *pc3, *pc4, *pc5;
  int nx=GetImNx(im), ny=GetImNy(im);
  int i, x, bnx=nx+2;

  pim=(PIX_TYPE *)GetImPtr(im);
  pci=pim;
  p1=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  p2=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  p3=(PIX_TYPE *)calloc(bnx, sizeof(PIX_TYPE));
  if (PIX_MIN != 0){
    for(i=0; i<bnx;i++){
      p1[i]=PIX_MIN;
      p2[i]=PIX_MIN;
      p3[i]=PIX_MIN;
    }
  }
  p[0]=p1;
  p[1]=p2;
  p[2]=p3;
  for(i=0;i<3-oy;i++)
    memcpy(p[i+oy]+ox,(pim+i*nx),nx*sizeof(PIX_TYPE));

  for(i=0; i<ny; i++){
    pc1=p1+1;
    pc2=p2;
    pc3=p2+1;
    pc4=p2+2;
    pc5=p3+1;

#pragma omp parallel for private(out)
    for(x=0;x<nx;x++){
      out=pc1[x];
      if(pc2[x]>out)
	out=pc2[x];
      if(pc3[x]>out)
	out=pc3[x];
      if(pc4[x]>out)
	out=pc4[x];
      if(pc5[x]>out)
	out=pc5[x];
      pci[x]=out;
    }
    pci+=nx;
    pc1=p1;
    p1=p2;
    p2=p3;
    p3=pc1;
    if(i<ny+oy-3)
      memcpy(p3+ox,(pim+(i-oy+3)*nx),nx*sizeof(PIX_TYPE));
    else
      for (x=0; x<bnx; x++)
	p3[x]=PIX_MIN;
  }
  free(p1); free(p2); free(p3);
  return NO_ERROR;
}
#include "f_undef.h"

ERROR_TYPE dilate4(IMAGE *im, int ox, int oy)
{
  if ( (ox > 2) || (oy > 2) ){
    (void)sprintf(buf,"dilate4(im): ox and oy must be <= 2\n"); errputstr(buf);
    return(ERROR);
  }

  switch (GetImDataType(im)){
  case t_UCHAR:
    return(uc_dilate4(im, ox, oy));
    break;
  case t_USHORT:
    return(us_dilate4(im, ox, oy));
    break;
  case t_INT32:
    return(i32_dilate4(im, ox, oy));
    break;
  case t_UINT32:
    return(u32_dilate4(im, ox, oy));
    break;
  case t_FLOAT:
    return(f_dilate4(im, ox, oy));
    break;
  default:
    (void)sprintf(buf,"dilate4(im): invalid pixel type\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}


/**@}*/
