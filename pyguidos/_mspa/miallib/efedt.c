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

/* error free Euclidean distance (squared values)
   following the pseudo-code of Meijster-Roerdink-Hesselink (2000)
   (original idea in Saito and Toriwaki, 1994).
   We compute the distance function on the foreground pixels, i.e.,
   those different from 0.

   first: 2003-11-25
   2nd last:  2003-11-28 (with influence zones)
   last: 2013 added omp speed-up

   20131001: changed INT32 to USHORT for the images imx and imy

 */

/** @file
 *  Error free Euclidean distance transform (square of) following the pseudo-code described in
 *  \cite meijster-roerdink-hesselink2000 following the original idea of \cite saito-toriwaki94
 *  @author Pierre Soille
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "miallib.h"
#ifdef OPENMP
#include <omp.h>
#endif

#define NORM(x,y)  ( ((x)*(x)) + ((y)*(y)) )
#define NORMU(x,y) ( (((UINT64)x)*((UINT64)x)) + (((UINT64)y)*((UINT64)y)) )
#define	SQU(x)	   ( ((UINT64)x) * ((UINT64)x) )


/** \addtogroup group_dist
 *  @{
 */

#define EFEDT_G_IS_UINT32 1


#include "uc_def.h"
#ifdef  EFEDT_G_IS_UINT32
#define GTYPE UINT32
#define t_GTYPE t_UINT32
#define GTYPE_MAX UINT32_MAX
#define BIGVAL (GTYPE_MAX/2)
#else
#define GTYPE USHORT
#define t_GTYPE t_USHORT
#define GTYPE_MAX USHORT_MAX
#define BIGVAL (GTYPE_MAX/2)
#endif
#define DTYPE UINT32
#define t_DTYPE t_UINT32
IMAGE *uc_sqedt(IMAGE *im)
{
  IMAGE *img, *imdt;
  PIX_TYPE *b;
  GTYPE *g, bigval;
  DTYPE *dt;
  long int *t, *s;
  long long int x, y, w, m, n, q, sep;

  unsigned long int offset, yxm;

  m=GetImNx(im);
  n=GetImNy(im);

  /* ursprunglich!!! bigval=m+n and mit fixed Type UINT32; geaendert am 20131002 for USHORT */

  bigval=BIGVAL;

  /* create temporary and output images */
  img = create_image(t_GTYPE, GetImNx(im), GetImNy(im), GetImNz(im));
  if (img == NULL){
    (void)sprintf(buf,"IMAGE  *uc_sqedt(IMAGE *im): not enough memory!\n"); errputstr(buf);
    return(NULL);
  }
  imdt = create_image(t_DTYPE, GetImNx(im), GetImNy(im), GetImNz(im));
  if (imdt == NULL){
    (void)sprintf(buf,"IMAGE  *uc_sqedt(IMAGE *im): not enough memory!\n"); errputstr(buf);
    free_image(img);
    return(NULL);
  }

  /* create t and s arrays */
#ifndef OPENMP
  t = (long int *)calloc(sizeof(long int), m);
  s = (long int *)calloc(sizeof(long int), m);
#endif

  b=(PIX_TYPE *)GetImPtr(im);
  g=(GTYPE *)GetImPtr(img);
  dt=(DTYPE *)GetImPtr(imdt);

  /* scans 1 and 2 */
#ifdef OPENMP
#pragma omp parallel for private(x,offset,y)
#endif
  for (x=0; x<m; x++){
    if ( *(b+x) == 0 ) /* background pixel */
      *(g+x)=0;
    else
      *(g+x)=bigval;
    for (y=1; y<n; y++){ /* forward column scan */
      offset=x+y*m;
      if ( *(b+offset) == 0 )
	*(g+offset)=0;
      else
	*(g+offset)=1+*(g+offset-m);
    }
    for (y=n-2; y>=0; y--){ /* backward column scan */
      offset=x+y*m;
      if ( *(g+offset+m) < *(g+offset) )
	*(g+offset)=1+*(g+offset+m);
    }
  }

  /* scans 3 and 4 */
#ifdef OPENMP
#pragma omp parallel for private(t,s,yxm,q,x,sep,w)
#endif
  for (y=0; y<n; y++){
#ifdef OPENMP
    t = (long int *)calloc(sizeof(long int), m);
    s = (long int *)calloc(sizeof(long int), m);
#endif
    yxm=y*m;
    q=0;
    s[0]=0;
    t[0]=0;
    for (x=1; x<m; x++){
      while ( (q>=0) && (NORMU(t[q]-s[q],*(g+s[q]+yxm)) > NORMU(t[q]-x,*(g+x+yxm))) )
	q--;
      if (q<0){
	q=0;
	s[0]=x;
      }
      else{
	sep=(SQU(x)-SQU(s[q])+SQU(*(g+x+yxm))-SQU(*(g+s[q]+yxm))) / (2*(x-s[q]));
	w=1+ sep;
	if (w<m){
	  q++;
	  s[q]=x;
	  t[q]=w;
	}
      }
    }
    for (x=m-1; x>=0; x--){
      *(dt+x+yxm)=NORMU(x-s[q],*(g+s[q]+yxm));
      if( x==t[q] )
	q--;
    }
#ifdef OPENMP
  free(s); free(t);
#endif
  }

#ifndef OPENMP
  free(s); free(t);
#endif
  free_image(img);
  return(imdt);
}
#undef BIGVAL
#undef GTYPE
#undef t_GTYPE
#undef DTYPE
#undef t_DTYPE
#undef GTYPE_MAX
#include "uc_undef.h"


IMAGE *sqedt(IMAGE *im)
{
  switch (GetImDataType(im)){

  case t_UCHAR:
    return (uc_sqedt(im));
    break;
  default:
    (void)sprintf(buf,"sqedt(im): invalid pixel type: im must be of type UCHAR\n"); errputstr(buf);
    return(NULL);
  }
  return NULL;
}

#include "uc_def.h"
#ifdef  EFEDT_G_IS_UINT32
#define GTYPE UINT32
#define t_GTYPE t_UINT32
#define GTYPE_MAX UINT32_MAX
#define BIGVAL (m+n)
#else
#define GTYPE USHORT
#define t_GTYPE t_USHORT
#define GTYPE_MAX USHORT_MAX
#define BIGVAL (GTYPE_MAX/2)
#endif
IMAGE *uc_iz(IMAGE *im)
{
  IMAGE *img, *imiz;
  PIX_TYPE *b, *iz;
  GTYPE *g, bigval;

  long int *t, *s;
  int x, y, w, m, n, q, sep;
  long int i;

  unsigned long int offset, yxm;

  m=GetImNx(im);
  n=GetImNy(im);
  bigval=BIGVAL;

  /* make sure the MSB of the input image is always equal to zero (this bit is used for the internal computations) */
  b=(PIX_TYPE *)GetImPtr(im);
  for(i=m*n; i>0; i--)
    if (*b++ >= PIX_MSB){
      (void)sprintf(buf,"IMAGE *uc_iz(IMAGE *im): the image values must be <=127 (2^7 - 1)!\n"); errputstr(buf);
      return(NULL);
    }

  /* create temporary and output images */
  img = create_image(t_GTYPE, GetImNx(im), GetImNy(im), GetImNz(im));
  if (img == NULL){
    (void)sprintf(buf,"IMAGE *uc_iz(IMAGE *im): not enough memory!\n"); errputstr(buf);
    return(NULL);
  }
  imiz = create_image(GetImDataType(im), GetImNx(im), GetImNy(im), GetImNz(im));
  if (imiz == NULL){
    (void)sprintf(buf,"IMAGE *uc_iz(IMAGE *im): not enough memory!\n"); errputstr(buf);
    free_image(img);
    return(NULL);
  }

#ifndef OPENMP
  /* create t and s arrays */
  t = (long int *)calloc(sizeof(long int), m);
  s = (long int *)calloc(sizeof(long int), m);
#endif

  b=(PIX_TYPE *)GetImPtr(im);
  g=(GTYPE *)GetImPtr(img);
  iz=(PIX_TYPE *)GetImPtr(imiz);

  /* scans 1 and 2 */
#ifdef OPENMP
#pragma omp parallel for private(x,offset,y)
#endif
  for (x=0; x<m; x++){
    if ( *(b+x) != 0 )
      *(g+x)=0;
    else
      *(g+x)=bigval;
    for (y=1; y<n; y++){ /* forward column scan */
      offset=x+y*m;
      if ( *(b+offset) != 0 )
	*(g+offset)=0;
      else
	*(g+offset)=1+*(g+offset-m);
    }
    for (y=n-2; y>=0; y--){ /* backward column scan */
      offset=x+y*m;
      if ( *(g+offset+m) < *(g+offset) ){
	*(g+offset)=1+*(g+offset+m);
	*(b+offset)|=PIX_MSB; /* point is ABOVE reference */
      }
    }
  }

  /* scans 3 and 4 */
#ifdef OPENMP
#pragma omp parallel for private(t,s,yxm,q,x,sep,w)
#endif
  for (y=0; y<n; y++){
#ifdef OPENMP
    t = (long int *)calloc(sizeof(long int), m);
    s = (long int *)calloc(sizeof(long int), m);
#endif
    yxm=y*m;
    q=0;
    s[0]=0;
    t[0]=0;
    for (x=1; x<m; x++){
      while ( (q>=0) && (NORMU(t[q]-s[q],*(g+s[q]+yxm)) > NORMU(t[q]-x,*(g+x+yxm))) )
	q--;
      if (q<0){
	q=0;
	s[0]=x;
      }
      else{
	sep=(SQU(x)-SQU(s[q])+SQU(*(g+x+yxm))-SQU(*(g+s[q]+yxm))) / (2*(x-s[q]));
	w=1+ sep;
	if (w<m){
	  q++;
	  s[q]=x;
	  t[q]=w;
	}
      }
    }
    for (x=m-1; x>=0; x--){
      if ( *(b+s[q]+yxm) & PIX_MSB )
	*(iz+x+yxm)= *(b+s[q]+ (*(g+s[q]+yxm)+y)*m);
      else
	*(iz+x+yxm)= *(b+s[q]+ (y-*(g+s[q]+yxm))*m);
      if( x==t[q] )
	q--;
    }
#ifdef OPENMP
  free(s); free(t);
#endif
  }

  /* reset input image */
  for(i=m*n; i>0; i--)
    *b++ &= ~PIX_MSB;

#ifndef OPENMP
  free(s); free(t);
#endif
  free_image(img);
  return(imiz);
}
#undef BIGVAL
#undef GTYPE
#undef t_GTYPE
#undef GTYPE_MAX
#include "uc_undef.h"


#include "us_def.h"
#ifdef  EFEDT_G_IS_UINT32
#define GTYPE UINT32
#define t_GTYPE t_UINT32
#define GTYPE_MAX UINT32_MAX
#define BIGVAL (m+n)
#else
#define GTYPE USHORT
#define t_GTYPE t_USHORT
#define GTYPE_MAX USHORT_MAX
#define BIGVAL (GTYPE_MAX/2)
#endif
IMAGE *us_iz(IMAGE *im)
{
  IMAGE *img, *imiz;
  PIX_TYPE *b, *iz;
  GTYPE *g, bigval;

  long int *t, *s;
  int x, y, w, m, n, q, sep;
  long int i;

  unsigned long int offset, yxm;

  m=GetImNx(im);
  n=GetImNy(im);
  bigval=BIGVAL;

  /* make sure the MSB of the input image is always equal to zero (this bit is used for the internal computations) */
  b=(PIX_TYPE *)GetImPtr(im);
  for(i=m*n; i>0; i--)
    if (*b++ &= PIX_MSB){
      (void)sprintf(buf,"IMAGE *us_iz(IMAGE *im): the image values must be <= 32767 (2^15 - 1) !\n"); errputstr(buf);
      return(NULL);
    }

  /* create temporary and output images */
  img = create_image(t_GTYPE, GetImNx(im), GetImNy(im), GetImNz(im));
  if (img == NULL){
    (void)sprintf(buf,"IMAGE  *us_iz(IMAGE *im): not enough memory!\n"); errputstr(buf);
    return(NULL);
  }
  imiz = create_image(GetImDataType(im), GetImNx(im), GetImNy(im), GetImNz(im));
  if (imiz == NULL){
    (void)sprintf(buf,"IMAGE  *us_iz(IMAGE *im): not enough memory!\n"); errputstr(buf);
    free_image(img);
    return(NULL);
  }

#ifndef OPENMP
  /* create t and s arrays */
  t = (long int *)calloc(sizeof(long int), m);
  s = (long int *)calloc(sizeof(long int), m);
#endif

  b=(PIX_TYPE *)GetImPtr(im);
  g=(GTYPE *)GetImPtr(img);
  iz=(PIX_TYPE *)GetImPtr(imiz);

  /* scans 1 and 2 */
#ifdef OPENMP
#pragma omp parallel for private(x,offset,y)
#endif
  for (x=0; x<m; x++){
    if ( *(b+x) != 0 )
      *(g+x)=0;
    else
      *(g+x)=bigval;
    for (y=1; y<n; y++){ /* forward column scan */
      offset=x+y*m;
      if ( *(b+offset) != 0 )
	*(g+offset)=0;
      else
	*(g+offset)=1+*(g+offset-m);
    }
    for (y=n-2; y>=0; y--){ /* backward column scan */
      offset=x+y*m;
      if ( *(g+offset+m) < *(g+offset) ){
	*(g+offset)=1+*(g+offset+m);
	*(b+offset)|=PIX_MSB; /* point is ABOVE reference */
      }
    }
  }

  /* scans 3 and 4 */
#ifdef OPENMP
#pragma omp parallel for private(t,s,yxm,q,x,sep,w)
#endif
  for (y=0; y<n; y++){
#ifdef OPENMP
    t = (long int *)calloc(sizeof(long int), m);
    s = (long int *)calloc(sizeof(long int), m);
#endif
    yxm=y*m;
    q=0;
    s[0]=0;
    t[0]=0;
    for (x=1; x<m; x++){
      while ( (q>=0) && (NORMU(t[q]-s[q],*(g+s[q]+yxm)) > NORMU(t[q]-x,*(g+x+yxm))) )
	q--;
      if (q<0){
	q=0;
	s[0]=x;
      }
      else{
	sep=(SQU(x)-SQU(s[q])+SQU(*(g+x+yxm))-SQU(*(g+s[q]+yxm))) / (2*(x-s[q]));
	w=1+ sep;
	if (w<m){
	  q++;
	  s[q]=x;
	  t[q]=w;
	}
      }
    }
    for (x=m-1; x>=0; x--){
      if ( *(b+s[q]+yxm) & PIX_MSB )
	*(iz+x+yxm)= *(b+s[q]+ (*(g+s[q]+yxm)+y)*m);
      else
	*(iz+x+yxm)= *(b+s[q]+ (y-*(g+s[q]+yxm))*m);
      if( x==t[q] )
	q--;
    }
#ifdef OPENMP
  free(s); free(t);
#endif
  }

  /* reset input image */
  for(i=m*n; i>0; i--)
   *b++ &= ~PIX_MSB;

#ifndef OPENMP
  free(s); free(t);
#endif
  free_image(img);
  return(imiz);
}
#undef BIGVAL
#undef GTYPE
#undef t_GTYPE
#undef GTYPE_MAX
#include "us_undef.h"


#include "u32_def.h"
#ifdef  EFEDT_G_IS_UINT32
#define GTYPE UINT32
#define t_GTYPE t_UINT32
#define GTYPE_MAX UINT32_MAX
#define BIGVAL (m+n)
#else
#define GTYPE USHORT
#define t_GTYPE t_USHORT
#define GTYPE_MAX USHORT_MAX
#define BIGVAL (GTYPE_MAX/2)
#endif
IMAGE *u32_iz(IMAGE *im)
{
  IMAGE *img, *imiz;
  PIX_TYPE *b, *iz;
  GTYPE *g, bigval;

  long int *t, *s;
  int x, y, w, m, n, q, sep;
  long int i;

  unsigned long int offset, yxm;

  m=GetImNx(im);
  n=GetImNy(im);
  bigval=BIGVAL;

  /* make sure the MSB of the input image is always equal to zero (this bit is used for the internal computations) */
  b=(PIX_TYPE *)GetImPtr(im);
  for(i=m*n; i>0; i--)
    if (*b++ >= PIX_MSB){
      (void)sprintf(buf,"IMAGE *us_iz(IMAGE *im): the image values must be <= 2147483647 (2^31 - 1) !\n"); errputstr(buf);
      return(NULL);
    }


  /* create temporary and output images */
  img = create_image(t_GTYPE, GetImNx(im), GetImNy(im), GetImNz(im));
  if (img == NULL){
    (void)sprintf(buf,"IMAGE  *u32_iz(IMAGE *im): not enough memory!\n"); errputstr(buf);
    return(NULL);
  }
  imiz = create_image(GetImDataType(im), GetImNx(im), GetImNy(im), GetImNz(im));
  if (imiz == NULL){
    (void)sprintf(buf,"IMAGE  *u32_iz(IMAGE *im): not enough memory!\n"); errputstr(buf);
    free_image(img);
    return(NULL);
  }

#ifndef OPENMP
  /* create t and s arrays */
  t = (long int *)calloc(sizeof(long int), m);
  s = (long int *)calloc(sizeof(long int), m);
#endif

  b=(PIX_TYPE *)GetImPtr(im);
  g=(GTYPE *)GetImPtr(img);
  iz=(PIX_TYPE *)GetImPtr(imiz);

  /* scans 1 and 2 */
#ifdef OPENMP
#pragma omp parallel for private(x,offset,y)
#endif
  for (x=0; x<m; x++){
    if ( *(b+x) != 0 )
      *(g+x)=0;
    else
      *(g+x)=bigval;
    for (y=1; y<n; y++){ /* forward column scan */
      offset=x+y*m;
      if ( *(b+offset) != 0 )
	*(g+offset)=0;
      else
	*(g+offset)=1+*(g+offset-m);
    }
    for (y=n-2; y>=0; y--){ /* backward column scan */
      offset=x+y*m;
      if ( *(g+offset+m) < *(g+offset) ){
	*(g+offset)=1+*(g+offset+m);
	*(b+offset)|=PIX_MSB; /* point is ABOVE reference */
      }
    }
  }

  /* scans 3 and 4 */
#ifdef OPENMP
#pragma omp parallel for private(t,s,yxm,q,x,sep,w)
#endif
  for (y=0; y<n; y++){
#ifdef OPENMP
    t = (long int *)calloc(sizeof(long int), m);
    s = (long int *)calloc(sizeof(long int), m);
#endif
    yxm=y*m;
    q=0;
    s[0]=0;
    t[0]=0;
    for (x=1; x<m; x++){
      while ( (q>=0) && (NORMU(t[q]-s[q],*(g+s[q]+yxm)) > NORMU(t[q]-x,*(g+x+yxm))) )
	q--;
      if (q<0){
	q=0;
	s[0]=x;
      }
      else{
	sep=(SQU(x)-SQU(s[q])+SQU(*(g+x+yxm))-SQU(*(g+s[q]+yxm))) / (2*(x-s[q]));
	w=1+ sep;
	if (w<m){
	  q++;
	  s[q]=x;
	  t[q]=w;
	}
      }
    }
    for (x=m-1; x>=0; x--){
      if ( *(b+s[q]+yxm) & PIX_MSB )
	*(iz+x+yxm)= *(b+s[q]+ (*(g+s[q]+yxm)+y)*m);
      else
	*(iz+x+yxm)= *(b+s[q]+ (y-*(g+s[q]+yxm))*m);
      if( x==t[q] )
	q--;
    }
#ifdef OPENMP
  free(s); free(t);
#endif
  }

  /* reset input image */
  for(i=m*n; i>0; i--)
   *b++ &= ~PIX_MSB;

#ifndef OPENMP
  free(s); free(t);
#endif
  free_image(img);
  return(imiz);
}
#undef BIGVAL
#undef GTYPE
#undef t_GTYPE
#undef GTYPE_MAX
#include "u32_undef.h"


IMAGE *iz(IMAGE *im)
{
  switch (GetImDataType(im)){

  case t_UCHAR:
    return (uc_iz(im));
    break;
  case t_USHORT:
    return (us_iz(im));
    break;
  case t_UINT32:
    return (u32_iz(im));
    break;
  default:
    (void)sprintf(buf,"iz(im): invalid pixel type\n"); errputstr(buf);
    return(NULL);
  }
  return NULL;
}

/*@}*/
