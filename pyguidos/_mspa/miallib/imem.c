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

/**

    imem.c
    Purpose: IMAGE memory management functions

    @author Pierre Soille
    @version latest
*/


/** @defgroup group_mem Memory Functions
 *  Functions dealing with image memory including associated Look-Up-Tables.
 *  @{
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef UNIX
#include <strings.h>  /* DOES NOT WORK ON WINDOWS */
#endif
#ifdef RLIMIT
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

#include "miallib.h"


#ifdef DMALLOC /* INCOMPATIBLE WITH EFENCE */
#include <dmalloc.h>
#endif


#if (defined(XLISP))
extern void gc();
/*PSRM char buf[256]; PSRM*/
#endif


/*************************************************************************/
#ifdef PYTHON
void stdputstr(char *buf)
{
  printf("%s", buf);
}
void errputstr(char *buf)
{
  printf("%s", buf);
}
// see https://docs.python.org/2/c-api/memory.html
// void* PyMem_Malloc(size_t n)
// void PyMem_Free(void *p)
// TYPE* PyMem_New(TYPE, size_t n)
#define my_malloc(x) PyMem_Malloc(x)
#define my_free_malloc(x) PyMem_Free(x)
#define my_calloc(x,y) PyMem_New(x,y)   /* x for TYPE and y for number of elements */
#define my_free_calloc(x) PyMem_Del(x)
#elif !defined(XLISP)
void stdputstr(char *buf)
{
  fprintf(stdout,"%s\n", buf);
}
void errputstr(char *buf)
{
  fprintf(stderr,"%s\n", buf);
}
#define my_malloc(x) malloc(x)
#define my_free_malloc(x) free(x)
#define my_calloc(x,y) (x *)calloc(y,sizeof(x))
#define my_free_calloc(x) free(x)
#endif


/**********************************/
int fprintfgenericnum(G_TYPE gval, int type)
{
  if (type == t_GENERIC){
    (void)sprintf(buf,"%d\n", (int)gval.generic_val); stdputstr(buf);
  }
  else if (type == t_UCHAR){
    (void)sprintf(buf,"%d\n", (int)gval.uc_val); stdputstr(buf);
  }
  else if  (type == t_USHORT){
    (void)sprintf(buf,"%d\n", (int)gval.us_val); stdputstr(buf);
  }
  else if (type == t_SHORT){
    (void)sprintf(buf,"%d\n", (int)gval.s_val); stdputstr(buf);
  }
  else if (type == t_INT32){
    (void)sprintf(buf,"%d\n", (int)gval.i32_val); stdputstr(buf);
  }
  else if (type == t_UINT32){
    (void)sprintf(buf,"%u\n", (unsigned) gval.u32_val); stdputstr(buf);
  }
  else if (type == t_INT64){
    (void)sprintf(buf,"%ld\n", (long int)gval.i64_val); stdputstr(buf);
  }
  else if (type == t_UINT64){
    (void)sprintf(buf,"%lu\n", (unsigned long) gval.u64_val); stdputstr(buf);
  }
  else if (type == t_FLOAT){
    (void)sprintf(buf,"%f\n", gval.f_val); stdputstr(buf);
  }
  else if (type == t_DOUBLE){
    (void)sprintf(buf,"%lf\n", (float)gval.d_val); stdputstr(buf);
  }
  else
    return(ERROR);
  return(NO_ERROR);
}

/*************************************************************************/

/**
 * create an IMAGE in memory and allocate the memory for the specified number of pixels
 * @param data_type integer for data type
 * @param nx long integer for number of columns
 * @param ny integer for number of lines
 * @param nz integer for number of planes
 * @return IMAGE pointer on success, NULL otherwise
*/
IMAGE *create_image(int data_type, long int nx, int ny, int nz)
{
  IMAGE *im;
  void *p;
  mia_size_t nbyte;

  if (((mia_size_t)nx*ny*nz) == 0){
    (void)sprintf(buf,"ERROR in create_image(data_type=%d, nx=%ld, ny=%d, nz=%d)\t \
                       invalid size parameters (must be positive) \n", \
		       data_type, nx, ny, nz); errputstr(buf);
    return(NULL);
  }

#ifdef RLIMIT
  struct rusage usage[1];
  struct rlimit rlim[1];
#endif

#ifdef RLIMIT
    getrusage(RUSAGE_SELF, usage);
    getrlimit(RLIMIT_FSIZE, rlim);
    fprintf(stderr, "RLIMIT_FSIZE cur=%d\n", rlim->rlim_cur);
    fprintf(stderr, "RLIMIT_FSIZE max=%d\n", rlim->rlim_max);
    getrlimit(RLIMIT_DATA, rlim);
    fprintf(stderr, "RLIMIT_DATA cur=%d\n", rlim->rlim_cur);
    fprintf(stderr, "RLIMIT_DATA max=%d\n", rlim->rlim_max);
    getrlimit(RLIMIT_CORE, rlim);
    fprintf(stderr, "RLIMIT_CORE cur=%d\n", rlim->rlim_cur);
    fprintf(stderr, "RLIMIT_CORE max=%d\n", rlim->rlim_max);
    rlim->rlim_cur=RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, rlim);
    getrlimit(RLIMIT_MEMLOCK, rlim);
    fprintf(stderr, "RLIMIT_MEMLOCK cur=%d\n", rlim->rlim_cur);
    fprintf(stderr, "RLIMIT_MEMLOCK max=%d\n", rlim->rlim_max);
    getrlimit(RLIMIT_AS, rlim);
    fprintf(stderr, "RLIMIT_AS cur=%d\n", rlim->rlim_cur);
    fprintf(stderr, "RLIMIT_AS max=%d\n", rlim->rlim_max);
#endif

    im = (IMAGE *)calloc((size_t)1, sizeof(IMAGE));
  if (im == NULL){
#if (defined(XLISP))
    gc();
    im = (IMAGE *)calloc((size_t)1, sizeof(IMAGE));
    if (im == NULL){
      (void)sprintf(buf,"ERROR in create_image(data_type=%d, nx=%ld, ny=%d, nz=%d) \
                         not enough memory\n", data_type, nx, ny, nz); errputstr(buf);
      xmem();
    }
    return(im);
#else
   (void)sprintf(buf,"ERROR in create_image(data_type=%d, nx=%ld, ny=%d, nz=%d) \
                      not enough memory\n", data_type, nx, ny, nz); errputstr(buf);
    return(im);
#endif
  }

  switch(data_type){
  case t_ONEBITPERPIXEL:
    nbyte = sizeof(UCHAR)*(nx/BITPERCHAR+(nx%BITPERWORD ? 4 : 0))*ny*nz;   /* GLOUP 2006 CH */
    break;
  case t_RGB:
    /* `bug': we do not handle this type, except for I */
    /*  nz is set to 3 (one color per plane) */
    nz=3;
  case t_FOURBITPERPIXEL:
    /* `bug': we do not handle this type, except for I/O */
    /* nbyte = ny*(nx/2+(nx%2))*sizeof(UCHAR); */
    /* -> dummy type */
  case t_TIFFONEBITPERPIXEL:
    /* `bug': we do not handle this type, except for I/O */
    /* -> dummy type */
  case t_UCHAR:
    nbyte = sizeof(UCHAR)*(mia_size_t)nx*ny*nz;
    break;
  case t_USHORT:
    nbyte = sizeof(USHORT)*(mia_size_t)nx*ny*nz;
    break;
  case t_SHORT:
    nbyte = sizeof(SHORT)*(mia_size_t)nx*ny*nz;
    break;
  case t_UINT32:
    nbyte = sizeof(UINT32)*(mia_size_t)nx*ny*nz;
    break;
  case t_INT32:
    nbyte = sizeof(INT32)*(mia_size_t)nx*ny*nz;
    break;
  case t_UINT64:
    nbyte = sizeof(UINT64)*(mia_size_t)nx*ny*nz;
    break;
  case t_INT64:
    nbyte = sizeof(INT64)*(mia_size_t)nx*ny*nz;
    break;
  case t_FLOAT:
    nbyte = sizeof(MIALFLOAT)*(mia_size_t)nx*ny*nz;
    break;
  case t_DOUBLE:
    nbyte = sizeof(DOUBLE)*(mia_size_t)nx*ny*nz;
    break;
  case t_PTR:
    nbyte = sizeof(void *)*(mia_size_t)nx*ny*nz;
    break;
  default:
    (void)sprintf(buf,"create_image(data_type=%d, nx=%ld, ny=%d, nz=%d): \
                  invalid data type\n", data_type, nx, ny, nz); errputstr(buf);
    free((char *)im);
    return(NULL);
  }

  if (nbyte%sizeof(long int)) /* pad for word size */
    nbyte+=sizeof(long int);

#if (defined(XLISP)) /* always call xlisp garbage collector (2004-12-6) */
    gc();
#endif
    p = (char *)calloc(nbyte, sizeof(char));

  if (p == NULL){
#if (defined(XLISP))
    xmem();
    gc();
    xmem();
    printf("nbyte=%lu\n", nbyte);
    p = (char *)calloc(nbyte, sizeof(char));
    if (p == NULL){
      (void)sprintf(buf,"ERROR in create_image(data_type=%d, nx=%ld, ny=%d, nz=%d) \
                         not enough memory (after call to garbage collector) \n", \
		    data_type, nx, ny, nz); errputstr(buf);
      xmem();
      free((char *)im);
      return(NULL);
    }
    memset((void *) p, 0, nbyte * sizeof(char));
#else
    (void)sprintf(buf,"ERROR in create_image(data_type=%d, nx=%ld, ny=%d, nz=%d) \
                       not enough memory\n", data_type, nx, ny, nz); errputstr(buf);
    free((char *)im);
    return(NULL);
#endif
  }

  SetImDataType(im, data_type);
  SetImNByte(im,nbyte);
  SetImPtr(im,p);
  SetImNx(im,nx);
  SetImNy(im,ny);
  SetImNz(im,nz);
  return(im);
}

/**
    free an image and its associated memory used to stored the pixel values
    @param im IMAGE pointer
    @return void
*/
void free_image(IMAGE *im)
{
  void *p;

  // printf("message: entering free_image()\n");
#ifdef DMALLOC
  malloc_debug(2);
  (void)sprintf(buf,"malloc_debug returns %d in free_image\n",malloc_debug()); errputstr(buf);
#endif
  if (im != NULL){
    SetImNByte(im, 0);
    p = (void *)GetImPtr(im);
    if (p != NULL)
      free((void *)p);
    p = (void *)GetImLut(im);
    if  (p != NULL)
      free((void *)p);
    free((void *)im);
  }
  else{
    (void)sprintf(buf,"free_image(): Trying to free a freed image ...");  errputstr(buf);
  }
}

/**
    free the LUT associated with an IMAGE
    @param im IMAGE pointer
*/
void free_lut(IMAGE *im)
{
  char *p;

  if (im != NULL){
    p = (char *)GetImLut(im);
    if  (p != NULL){
      free((char *)p);
      SetImLut(im, NULL);
    }
    else{
	(void)sprintf(buf,"free_lut(): Trying to free a LUT already freed ...");  errputstr(buf);
    }
  }
  else{
    (void)sprintf(buf,"free_lut(): Trying to free the LUT of a freed image ...");  errputstr(buf);
  }
}


/**
 * returns the number of bits used to store an individual pixel, -1 on failure.
 * @param im IMAGE pointer
 * @return integer holding number of bits used to store a pixel, -1 on failure
*/
int GetImBitPerPixel(IMAGE *im)
{
  switch (GetImDataType(im)){
  case t_TIFFONEBITPERPIXEL:
  case t_ONEBITPERPIXEL:
    return(1);
    break;
  case t_FOURBITPERPIXEL:
    return(4);
    break;
  case t_UCHAR:
    return(BITPERCHAR);
    break;
  case t_USHORT:
  case t_SHORT:
    return(BITPERSHORT);
    break;
  case t_UINT32:
  case t_INT32:
    return(BITPERINT32);
    break;
  case t_UINT64:
  case t_INT64:
    return(BITPERINT64);
    break;
  case t_FLOAT:
    return(BITPERFLOAT);
    break;
  case t_DOUBLE:
    return(BITPERDOUBLE);
    break;
  case t_RGB:
    return(BITPERBAND);
    break;
  default:
    (void)sprintf(buf,"GetImBitPerPixel(im): invalid pixel type\n"); errputstr(buf);
    return(-1);
  }
}

/**
    print in stdout the values of the fields of the given IMAGE
    @param im IMAGE pointer
    @return void
*/
ERROR_TYPE iminfo(IMAGE *im)
{
  G_TYPE *pg;
  unsigned short int *plut;
  int maxi;
  IMAGE *imhst=NULL;
  HST1D_TYPE *phst;
  int i;
#ifdef XLDEBUG
  (void)sprintf(buf,"Image structure pointer\t=\t%p\n",im); stdputstr(buf);
  (void)sprintf(buf,"Pointer to image pixels\t=\t%p\n",GetImPtr(im)); stdputstr(buf);
#endif /* XLDEBUG */
  (void)sprintf(buf,"Image data type\t\t=\t%d\n",GetImDataType(im)); stdputstr(buf);
  pg = min_max(im);
  if (pg == NULL){
    (void)sprintf(buf,"Unable to compute min/max image values\n"); errputstr(buf);
    maxi = -1; /* make sure it won't be used later */
  }
  else{
    (void)sprintf(buf,"Minimum image value\t=\t"); stdputstr(buf);
    (void)fprintfgenericnum(pg[0],GetImDataType(im));
    (void)sprintf(buf,"Maximum image value\t=\t"); stdputstr(buf);
    (void)fprintfgenericnum(pg[1],GetImDataType(im));
    maxi = (int)pg[1].uc_val;
    free((char *)pg);
  }
  (void)sprintf(buf,"Image size in x\t\t=\t%ld\n",GetImNx(im)); stdputstr(buf);
  (void)sprintf(buf,"Image size in y\t\t=\t%ld\n",GetImNy(im)); stdputstr(buf);
  (void)sprintf(buf,"Image size in z\t\t=\t%ld\n",GetImNz(im)); stdputstr(buf);
  (void)sprintf(buf,"Number of bytes\t\t=\t%ld\n",GetImNByte(im)); stdputstr(buf);
  plut = (unsigned short int *)GetImLut(im);
  if (plut != NULL){
    imhst = histo1d(im);
    if (imhst != NULL){
      (void)sprintf(buf,"ColorMap:\n"); stdputstr(buf);
      phst = (HST1D_TYPE *)GetImPtr(imhst);
      if (GetImDataType(im)==t_FOURBITPERPIXEL){
	for (i=0; i<=maxi; i++){
	  if (phst[i]!=0){
	    (void)sprintf(buf,"R[%3d]= %3d\t G[%3d]= %3d\t B[%3d]= %3d\n",i, (int)plut[i], i, (int)plut[i+16],i , (int)plut[i+32]);
	    stdputstr(buf);
	  }
	}
      }
      else if (GetImDataType(im)==t_UCHAR){
	for (i=0; i<=maxi; i++){
	  if (phst[i] != 0){
	    (void)sprintf(buf,"R[%3d]= %3d\t G[%3d]= %3d\t B[%3d]= %3d\n",i, (int)plut[i]>>8, i, (int)plut[i+256]>>8,i , (int)plut[i+512]>>8);
	    stdputstr(buf);
	  }
	}
      }
      free_image(imhst);
    }
  }
  return(NO_ERROR);
}

/*!
    generate a physical copy of an IMAGE
    @param im IMAGE pointer
    @return IMAGE pointer on success, NULL otherwise
*/
IMAGE *copy_image(IMAGE *im)
{
  IMAGE *imout;
  int i;
  /* UCHAR *pim, *pimout; */
  unsigned short int *plut;

#ifdef DMALLOC
  malloc_debug(2);
#endif
  imout = create_image(GetImDataType(im),GetImNx(im),GetImNy(im),(size_t) GetImNz(im));
  if (imout != NULL){
    /* pimout = (UCHAR *)GetImPtr(imout);
       pim    = (UCHAR *)GetImPtr(im); */
    memcpy((void *)GetImPtr(imout), (void *)GetImPtr(im), GetImNByte(im));
    /* for (i=GetImNByte(im); i > 0; i--)
     *pimout++= *pim++; */
    if (GetImLut(im) != NULL){ /* possible bug ... should be rewritten */
      plut = (unsigned short int *)malloc(3*256*sizeof(short));
      if (plut!=NULL){
	SetImLut(imout,plut);
	for (i=0; i<768; i++)
	  plut[i]=im->lut[i];
      }
    }
  }

#ifdef DMALLOC
  (void)sprintf(buf,"malloc_debug returns %d in copy_image\n",malloc_debug()); errputstr(buf);
#endif
  return(imout);
}


/*************************************************************************/

IMBLOB *create_blob(LBL_TYPE n)
{
  IMBLOB *blob;

  blob= (IMBLOB *)calloc((size_t)n, sizeof(IMBLOB));
  if (blob == NULL){
#if (defined(XLISP))
    gc();
    blob= (IMBLOB *)calloc((size_t)n, sizeof(IMBLOB));
    if (blob == NULL){
      (void)sprintf(buf,"ERROR in create_blob(n): not enough memory\n"); errputstr(buf);
    }
    return(blob);
#else
    (void)sprintf(buf,"ERROR in create_blob(n): not enough memory\n"); errputstr(buf);
#endif
  }
  return(blob);
}

/**
    create a LUT and associate it with the input IMAGE.  A LUT consists of 768 values indicating the RGB value of any byte pixel value (the codes are interleaved by band in the RGB order).
    @param im IMAGE pointer
    @return NO_ERROR on success, ERROR otherwise
*/
ERROR_TYPE create_lut(IMAGE *im)
{
  unsigned short int *  out_lut;
  int  i;

  out_lut = GetImLut(im);
  if(out_lut == NULL){
    out_lut = (unsigned short int *)malloc(3*256*sizeof(short));
  }
  else{
    (void)sprintf(buf,"WARNING in create_lut: image has already a LUT\n"); errputstr(buf);
    return(NO_ERROR);
  }

  if(out_lut == NULL){
    (void)sprintf(buf,"ERROR in create_lut not enough memory to create new lut\n"); errputstr(buf);
    return(ERROR);
  }

  SetImLut(im, out_lut);
  for (i=0; i<768; i++)
    out_lut[i]=(i%256)<<8;

  return(NO_ERROR);
}


/**
    copy the LUT associated with the first IMAGE to the LUT associated with the second IMAGE
    @param im1 IMAGE pointer
    @param im2 IMAGE pointer
    @return NO_ERROR on success, ERROR otherwise
*/
ERROR_TYPE copy_lut(IMAGE *im1, IMAGE *im2)
{
  unsigned short int *out_lut;
  int n=768; /* RGB LUTs have 256 entries per band */
  int  i;

  if(GetImLut(im2) == NULL){
    (void)sprintf(buf,"ERROR in copy_lut no lut in input image\n"); errputstr(buf);
    return(ERROR);
  }

  out_lut = GetImLut(im1);
  if(out_lut == NULL){
    out_lut = (unsigned short int *)malloc(n*sizeof(short));
  }
  else{
    free(out_lut);
    out_lut = (unsigned short int *)malloc(n*sizeof(short));
  }

  if(out_lut == NULL){
    (void)sprintf(buf,"ERROR in copy_lut not enough memory to create new lut\n"); errputstr(buf);
    return(ERROR);
  }

  SetImLut(im1, out_lut);
  for (i=0; i<n; i++)
    out_lut[i]=im2->lut[i];

  return(NO_ERROR);
}

/**
    set the pixel value of im at position offset to the value encapsulated in g
    @param im IMAGE pointer
    @param offset int for offset from origin (0 for 1st pixel)
    @param g G_TYPE for pixel value
    @return NO_ERROR on success, ERROR otherwise
*/
ERROR_TYPE setpixval(IMAGE *im, unsigned long offset, G_TYPE g)
{
  switch(GetImDataType(im)){
  case t_UCHAR:
    *((UCHAR *) GetImPtr(im) + offset) = g.uc_val;
    return NO_ERROR;
  case t_USHORT:
    *((USHORT *) GetImPtr(im) + offset) = g.us_val;
    return NO_ERROR;
  case t_SHORT:
    *((SHORT *) GetImPtr(im) + offset) = g.s_val;
    return NO_ERROR;
  case t_INT32:
    *((INT32 *) GetImPtr(im) + offset) = g.i32_val;
    return NO_ERROR;
  case t_UINT32:
    *((UINT32 *) GetImPtr(im) + offset) = g.u32_val;
    return NO_ERROR;
  case t_INT64:
    *((INT64 *) GetImPtr(im) + offset) = g.i64_val;
    return NO_ERROR;
  case t_UINT64:
    *((UINT64 *) GetImPtr(im) + offset) = g.u64_val;
    return NO_ERROR;
  case t_FLOAT:
    *((MIALFLOAT *) GetImPtr(im) + offset) = g.f_val;
    return NO_ERROR;
  case t_DOUBLE:
    *((DOUBLE *) GetImPtr(im) + offset) = g.d_val;
    return NO_ERROR;
  default:
    (void)sprintf(buf,"setpixval(): invalid pixel type=%d\n", GetImDataType(im)); errputstr(buf);
  }
  return ERROR;
}


/**
    get the pixel value of im at position offset
    @param im IMAGE pointer
    @param offset int for offset from origin (0 for 1st pixel)
    @return G_TYPE for pixel value
*/
G_TYPE getpixval(IMAGE *im, unsigned long offset)
{
  G_TYPE g;

  switch(GetImDataType(im)){
  case t_UCHAR:
    g.uc_val= *((UCHAR *) GetImPtr(im) + offset);
    break;
  case t_USHORT:
    g.us_val= *((USHORT *) GetImPtr(im) + offset);
    break;
  case t_SHORT:
    g.s_val= *((SHORT *) GetImPtr(im) + offset);
    break;
  case t_INT32:
    g.i32_val= *((INT32 *) GetImPtr(im) + offset);
    break;
  case t_UINT32:
    g.u32_val= *((UINT32 *) GetImPtr(im) + offset);
    break;
  case t_INT64:
    g.i64_val= *((INT64 *) GetImPtr(im) + offset);
    break;
  case t_UINT64:
    g.u64_val= *((UINT64 *) GetImPtr(im) + offset);
    break;
  case t_FLOAT:
    g.f_val= *((MIALFLOAT *) GetImPtr(im) + offset);
    break;
  case t_DOUBLE:
    g.d_val= *((DOUBLE *) GetImPtr(im)  + offset);
    break;
  default:
    (void)sprintf(buf,"getpixval(): invalid pixel type=%d\n", GetImDataType(im)); errputstr(buf);
  }
  return g;
}


#include "uc_def.h"
ERROR_TYPE uc_FindPixWithVal(IMAGE *im, PIX_TYPE val, unsigned long int *ofs)
{
  PIX_TYPE *p, *pend;

  p=(PIX_TYPE *)GetImPtr(im);
  pend=p+GetImNPix(im);

  for(;p<pend;p++){
    if(*p==val)
      break;
  }
  if(p==pend)
    return ERROR;
  *ofs=p-(PIX_TYPE *)GetImPtr(im);

  return NO_ERROR;

}
#include "uc_undef.h"


#include "us_def.h"
ERROR_TYPE us_FindPixWithVal(IMAGE *im, PIX_TYPE val, unsigned long int *ofs)
{
  PIX_TYPE *p, *pend;

  p=(PIX_TYPE *)GetImPtr(im);
  pend=p+GetImNPix(im);

  for(;p<pend;p++){
    if(*p==val)
      break;
  }
  if(p==pend)
    return ERROR;
  *ofs=p-(PIX_TYPE *)GetImPtr(im);

  return NO_ERROR;

}
#include "us_undef.h"

#include "i32_def.h"
ERROR_TYPE i32_FindPixWithVal(IMAGE *im, PIX_TYPE val, unsigned long int *ofs)
{
  PIX_TYPE *p, *pend;

  p=(PIX_TYPE *)GetImPtr(im);
  pend=p+GetImNPix(im);

  for(;p<pend;p++){
    if(*p==val)
      break;
  }
  if(p==pend)
    return ERROR;
  *ofs=p-(PIX_TYPE *)GetImPtr(im);

  return NO_ERROR;

}
#include "i32_undef.h"


#include "f_def.h"
ERROR_TYPE f_FindPixWithVal(IMAGE *im, PIX_TYPE val, unsigned long int *ofs)
{
  PIX_TYPE *p, *pend;

  p=(PIX_TYPE *)GetImPtr(im);
  pend=p+GetImNPix(im);

  for(;p<pend;p++){
    if(*p==val)
      break;
  }
  if(p==pend)
    return ERROR;
  *ofs=p-(PIX_TYPE *)GetImPtr(im);

  return NO_ERROR;

}
#include "f_undef.h"


ERROR_TYPE FindPixWithVal(IMAGE *im, G_TYPE gval, unsigned long int *ofs)
{
  switch (GetImDataType(im)){

  case t_UCHAR:
    return(uc_FindPixWithVal(im, gval.uc_val, ofs));
    break;

  case t_SHORT:
  case t_USHORT:
    return(us_FindPixWithVal(im, gval.us_val, ofs));
    break;

  case t_INT32:
  case t_UINT32:
    return(i32_FindPixWithVal(im, gval.i32_val, ofs));
    break;

  case t_FLOAT:
    return(f_FindPixWithVal(im, gval.f_val, ofs));
    break;

  default:
    (void)sprintf(buf,"FindPixelWithVal(im, val, ofs): invalid pixel type\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}


/**
    create a pointer to an IMAGE array with n elements
    @param n int for number of elements
    @return IMAGE * pointer
*/
IMAGE **create_imarray(int n)
{
  return((IMAGE **)calloc(n, sizeof(IMAGE *)));
}




#include "uc_def.h"
IMAGE *uc_imtoarray(IMAGE *im, IMAGE *imroi)
{
  IMAGE *imout;
  PIX_TYPE *pout;
  PIX_TYPE *pim;
  UCHAR *proi;
  mia_size_t i, o=0, n=0, npix=GetImNPix(im);

  proi=(UCHAR *)GetImPtr(imroi);

  for(i=0; i<npix; i++)
    if(proi[i])
      n++;

  imout=create_image(t_PIX_TYPE, (long int)n, 1, 1);
  if (imout==NULL){
    (void)sprintf(buf,"uc_imtoarray(IMAGE *im, IMAGE *imroi): not enough memory\n"); errputstr(buf);
    return NULL;
  }

  pim=(PIX_TYPE *)GetImPtr(im);
  pout=(PIX_TYPE *)GetImPtr(imout);

  for(i=0; i<npix; i++)
    if (proi[i])
      pout[o++]=pim[i];
  return imout;
}
#include "uc_undef.h"



#include "us_def.h"
IMAGE *us_imtoarray(IMAGE *im, IMAGE *imroi)
{
  IMAGE *imout;
  PIX_TYPE *pout;
  PIX_TYPE *pim;
  UCHAR *proi;
  mia_size_t i, o=0, n=0, npix=GetImNPix(im);

  proi=(UCHAR *)GetImPtr(imroi);

  for(i=0; i<npix; i++)
    if(proi[i])
      n++;

  imout=create_image(t_PIX_TYPE, (long int)n, 1, 1);
  if (imout==NULL){
    (void)sprintf(buf,"uc_imtoarray(IMAGE *im, IMAGE *imroi): not enough memory\n"); errputstr(buf);
    return NULL;
  }

  pim=(PIX_TYPE *)GetImPtr(im);
  pout=(PIX_TYPE *)GetImPtr(imout);

  for(i=0; i<npix; i++)
    if (proi[i])
      pout[o++]=pim[i];
  return imout;
}
#include "us_undef.h"

#include "g32_def.h"
IMAGE *g32_imtoarray(IMAGE *im, IMAGE *imroi)
{
  IMAGE *imout;
  PIX_TYPE *pout;
  PIX_TYPE *pim;
  UCHAR *proi;
  mia_size_t i, o=0, n=0, npix=GetImNPix(im);

  proi=(UCHAR *)GetImPtr(imroi);

  for(i=0; i<npix; i++)
    if(proi[i])
      n++;

  imout=create_image(GetImDataType(im), (long int)n, 1, 1);
  if (imout==NULL){
    (void)sprintf(buf,"uc_imtoarray(IMAGE *im, IMAGE *imroi): not enough memory\n"); errputstr(buf);
    return NULL;
  }

  pim=(PIX_TYPE *)GetImPtr(im);
  pout=(PIX_TYPE *)GetImPtr(imout);

  for(i=0; i<npix; i++)
    if (proi[i])
      pout[o++]=pim[i];
  return imout;
}
#include "g32_undef.h"


IMAGE *imtoarray(IMAGE *im, IMAGE *imroi)
{
  /* first 20130620 MODIS - SPOT regression for cloud detection */

  if ( (szgeocompat(im, imroi) != NO_ERROR) || (GetImDataType(imroi) != t_UCHAR)){
    (void)sprintf(buf,"imtoarray(IMAGE *im, IMAGE *imroi): images must be the same size and imroi of type UCHAR\n"); errputstr(buf);
    return NULL;
  }

  switch(GetImDataType(im)){
  case t_UCHAR:
    return(uc_imtoarray(im, imroi));
    break;
  case t_USHORT:
    return(us_imtoarray(im, imroi));
  case t_INT32:
  case t_UINT32:
  case t_FLOAT:
    return(g32_imtoarray(im, imroi));
    break;
  default:
    (void)sprintf(buf, "error in imtoarray(): \
                undefined image data type\n"); errputstr(buf);
    return(NULL);
  }
}




#include "uc_def.h"
IMAGE *uc_arraytoim(IMAGE *im, IMAGE *imroi)
{
  IMAGE *imout;
  PIX_TYPE *pout;
  PIX_TYPE *pim;
  UCHAR *proi;
  mia_size_t i, o=0, npix=GetImNPix(imroi);

  proi=(UCHAR *)GetImPtr(imroi);

  imout=create_image(t_PIX_TYPE, GetImNx(imroi), GetImNy(imroi), GetImNz(imroi));
  if (imout==NULL){
    (void)sprintf(buf,"uc_arraytoim(IMAGE *im, IMAGE *imroi): not enough memory\n"); errputstr(buf);
    return NULL;
  }

  pim=(PIX_TYPE *)GetImPtr(im);
  pout=(PIX_TYPE *)GetImPtr(imout);

  for(i=0; i<npix; i++)
    if (proi[i])
      pout[i]=pim[o++];

  return imout;
}
#include "uc_undef.h"


#include "us_def.h"
IMAGE *us_arraytoim(IMAGE *im, IMAGE *imroi)
{
  IMAGE *imout;
  PIX_TYPE *pout;
  PIX_TYPE *pim;
  UCHAR *proi;
  mia_size_t i, o=0, npix=GetImNPix(imroi);

  proi=(UCHAR *)GetImPtr(imroi);

  imout=create_image(t_PIX_TYPE, GetImNx(imroi), GetImNy(imroi), GetImNz(imroi));
  if (imout==NULL){
    (void)sprintf(buf,"uc_arraytoim(IMAGE *im, IMAGE *imroi): not enough memory\n"); errputstr(buf);
    return NULL;
  }

  pim=(PIX_TYPE *)GetImPtr(im);
  pout=(PIX_TYPE *)GetImPtr(imout);

  for(i=0; i<npix; i++)
    if (proi[i])
      pout[i]=pim[o++];

  return imout;
}
#include "us_undef.h"

#include "g32_def.h"
IMAGE *g32_arraytoim(IMAGE *im, IMAGE *imroi)
{
  IMAGE *imout;
  PIX_TYPE *pout;
  PIX_TYPE *pim;
  UCHAR *proi;
  mia_size_t i, o=0, npix=GetImNPix(imroi);

  proi=(UCHAR *)GetImPtr(imroi);

  imout=create_image(GetImDataType(im), GetImNx(imroi), GetImNy(imroi), GetImNz(imroi));
  if (imout==NULL){
    (void)sprintf(buf,"uc_arraytoim(IMAGE *im, IMAGE *imroi): not enough memory\n"); errputstr(buf);
    return NULL;
  }

  pim=(PIX_TYPE *)GetImPtr(im);
  pout=(PIX_TYPE *)GetImPtr(imout);

  for(i=0; i<npix; i++)
    if (proi[i])
      pout[i]=pim[o++];

  return imout;
}
#include "g32_undef.h"

IMAGE *arraytoim(IMAGE *im, IMAGE *imroi)
{
  /* first 20130621 MODIS - SPOT regression for cloud detection */

  if ( (GetImNPix(im)>GetImNPix(imroi)) || (GetImDataType(imroi) != t_UCHAR)){
    (void)sprintf(buf,"arraytoim(IMAGE *im, IMAGE *imroi): im must not have more pixels than imroi and imroi must be of type UCHAR\n"); errputstr(buf);
    return NULL;
  }

  switch(GetImDataType(im)){
  case t_UCHAR:
    return(uc_arraytoim(im, imroi));
    break;
  case t_USHORT:
    return(us_arraytoim(im, imroi));
    break;
  case t_INT32:
  case t_UINT32:
  case t_FLOAT:
    return(g32_arraytoim(im, imroi));
    break;
  default:
    (void)sprintf(buf, "error in arraytoim(): \
                undefined image data type\n"); errputstr(buf);
    return(NULL);
  }
  return(NULL);
}


#ifndef NO_generic_IMAGE
#include "g_def.h"
ERROR_TYPE generic_dumpxyz(IMAGE *im, int x, int y, int z, int dx, int dy)
{
  int y1, y2, x1, x2;
  int i, j;
  int nx, ny, nz;
  PIX_TYPE *p;

  p=(PIX_TYPE *)GetImPtr(im);

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);

  x = MIN(abs(x),nx-1);
  y = MIN(abs(y),ny-1);
  z = MIN(abs(z),nz-1);


  x1 = x-dx/2;
  if (x1<0)
    x1=0;

  y1 = y-dy/2;
  if (y1<0)
    y1=0;

  x2 = x+dx/2;
  if (x2>=nx)
    x2=nx-1;

  y2 = y+dy/2;
  if (y2>=ny)
    y2=ny-1;

  (void)sprintf(buf,"z=%4d  ", z); stdputstr(buf);
  for (i=x1; i<=x2; i++){
    (void)sprintf(buf," %4d|",i); stdputstr(buf);
  }
  (void)sprintf(buf,"\n"); stdputstr(buf);
  for (i=y1; i<=y2; i++){
    (void)sprintf(buf,"|%4d|",i); stdputstr(buf);
    for (j=x1; j<=x2; j++){
#if FLOATING
      (void)sprintf(buf,"%6.3f",(float)p[z*nx*ny+i*nx+j]); stdputstr(buf);
#else
      (void)sprintf(buf,"%6d",(int)p[z*nx*ny+i*nx+j]); stdputstr(buf);
#endif
    }
    (void)sprintf(buf,"\n"); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */


#include "s_def.h"
ERROR_TYPE s_dumpxyz(IMAGE *im, int x, int y, int z, int dx, int dy)
{
  int y1, y2, x1, x2;
  int i, j;
  int nx, ny, nz;
  PIX_TYPE *p;

  p=(PIX_TYPE *)GetImPtr(im);

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);

  x = MIN(abs(x),nx-1);
  y = MIN(abs(y),ny-1);
  z = MIN(abs(z),nz-1);


  x1 = x-dx/2;
  if (x1<0)
    x1=0;

  y1 = y-dy/2;
  if (y1<0)
    y1=0;

  x2 = x+dx/2;
  if (x2>=nx)
    x2=nx-1;

  y2 = y+dy/2;
  if (y2>=ny)
    y2=ny-1;

  (void)sprintf(buf,"z=%4d  ", z); stdputstr(buf);
  for (i=x1; i<=x2; i++){
    (void)sprintf(buf," %4d|",i); stdputstr(buf);
  }
  (void)sprintf(buf,"\n"); stdputstr(buf);
  for (i=y1; i<=y2; i++){
    (void)sprintf(buf,"|%4d|",i); stdputstr(buf);
    for (j=x1; j<=x2; j++){
#if FLOATING
      (void)sprintf(buf,"%6.3f",(float)p[z*nx*ny+i*nx+j]); stdputstr(buf);
#else
      (void)sprintf(buf,"%6d",(int)p[z*nx*ny+i*nx+j]); stdputstr(buf);
#endif
    }
    (void)sprintf(buf,"\n"); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "s_undef.h"



#include "us_def.h"
ERROR_TYPE us_dumpxyz(IMAGE *im, int x, int y, int z, int dx, int dy)
{
  int y1, y2, x1, x2;
  int i, j;
  int nx, ny, nz;
  PIX_TYPE *p;

  p=(PIX_TYPE *)GetImPtr(im);

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);

  x = MIN(abs(x),nx-1);
  y = MIN(abs(y),ny-1);
  z = MIN(abs(z),nz-1);


  x1 = x-dx/2;
  if (x1<0)
    x1=0;

  y1 = y-dy/2;
  if (y1<0)
    y1=0;

  x2 = x+dx/2;
  if (x2>=nx)
    x2=nx-1;

  y2 = y+dy/2;
  if (y2>=ny)
    y2=ny-1;

  (void)sprintf(buf,"z=%4d  ", z); stdputstr(buf);
  for (i=x1; i<=x2; i++){
    (void)sprintf(buf," %4d|",i); stdputstr(buf);
  }
  (void)sprintf(buf,"\n"); stdputstr(buf);
  for (i=y1; i<=y2; i++){
    (void)sprintf(buf,"|%4d|",i); stdputstr(buf);
    for (j=x1; j<=x2; j++){
#if FLOATING
      (void)sprintf(buf,"%6.3f",(float)p[z*nx*ny+i*nx+j]); stdputstr(buf);
#else
      (void)sprintf(buf,"%6d",(int)p[z*nx*ny+i*nx+j]); stdputstr(buf);
#endif
    }
    (void)sprintf(buf,"\n"); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "us_undef.h"




#include "i32_def.h"
ERROR_TYPE i32_dumpxyz(IMAGE *im, int x, int y, int z, int dx, int dy)
{
  int y1, y2, x1, x2;
  int i, j;
  int nx, ny, nz;
  PIX_TYPE *p;

  p=(PIX_TYPE *)GetImPtr(im);

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);

  x = MIN(abs(x),nx-1);
  y = MIN(abs(y),ny-1);
  z = MIN(abs(z),nz-1);


  x1 = x-dx/2;
  if (x1<0)
    x1=0;

  y1 = y-dy/2;
  if (y1<0)
    y1=0;

  x2 = x+dx/2;
  if (x2>=nx)
    x2=nx-1;

  y2 = y+dy/2;
  if (y2>=ny)
    y2=ny-1;

  (void)sprintf(buf,"z=%4d  ", z); stdputstr(buf);
  for (i=x1; i<=x2; i++){
    (void)sprintf(buf," %4d|",i); stdputstr(buf);
  }
  (void)sprintf(buf,"\n"); stdputstr(buf);
  for (i=y1; i<=y2; i++){
    (void)sprintf(buf,"|%4d|",i); stdputstr(buf);
    for (j=x1; j<=x2; j++){
#if FLOATING
      (void)sprintf(buf,"%6.3f",(float)p[z*nx*ny+i*nx+j]); stdputstr(buf);
#else
      (void)sprintf(buf,"%6d",(int)p[z*nx*ny+i*nx+j]); stdputstr(buf);
#endif
    }
    (void)sprintf(buf,"\n"); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "i32_undef.h"


#include "u32_def.h"
ERROR_TYPE u32_dumpxyz(IMAGE *im, int x, int y, int z, int dx, int dy)
{
  int y1, y2, x1, x2;
  int i, j;
  int nx, ny, nz;
  PIX_TYPE *p;

  p=(PIX_TYPE *)GetImPtr(im);

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);

  x = MIN(abs(x),nx-1);
  y = MIN(abs(y),ny-1);
  z = MIN(abs(z),nz-1);


  x1 = x-dx/2;
  if (x1<0)
    x1=0;

  y1 = y-dy/2;
  if (y1<0)
    y1=0;

  x2 = x+dx/2;
  if (x2>=nx)
    x2=nx-1;

  y2 = y+dy/2;
  if (y2>=ny)
    y2=ny-1;

  (void)sprintf(buf,"z=%4d  ", z); stdputstr(buf);
  for (i=x1; i<=x2; i++){
    (void)sprintf(buf," %4d|",i); stdputstr(buf);
  }
  (void)sprintf(buf,"\n"); stdputstr(buf);
  for (i=y1; i<=y2; i++){
    (void)sprintf(buf,"|%4d|",i); stdputstr(buf);
    for (j=x1; j<=x2; j++){
#if FLOATING
      (void)sprintf(buf,"%6.3f",(float)p[z*nx*ny+i*nx+j]); stdputstr(buf);
#else
      (void)sprintf(buf,"%6u",p[z*nx*ny+i*nx+j]); stdputstr(buf);
#endif
    }
    (void)sprintf(buf,"\n"); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "u32_undef.h"


#include "i64_def.h"
ERROR_TYPE i64_dumpxyz(IMAGE *im, int x, int y, int z, int dx, int dy)
{
  int y1, y2, x1, x2;
  int i, j;
  int nx, ny, nz;
  PIX_TYPE *p;

  p=(PIX_TYPE *)GetImPtr(im);

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);

  x = MIN(abs(x),nx-1);
  y = MIN(abs(y),ny-1);
  z = MIN(abs(z),nz-1);


  x1 = x-dx/2;
  if (x1<0)
    x1=0;

  y1 = y-dy/2;
  if (y1<0)
    y1=0;

  x2 = x+dx/2;
  if (x2>=nx)
    x2=nx-1;

  y2 = y+dy/2;
  if (y2>=ny)
    y2=ny-1;

  (void)sprintf(buf,"z=%4d  ", z); stdputstr(buf);
  for (i=x1; i<=x2; i++){
    (void)sprintf(buf," %4d|",i); stdputstr(buf);
  }
  (void)sprintf(buf,"\n"); stdputstr(buf);
  for (i=y1; i<=y2; i++){
    (void)sprintf(buf,"|%4d|",i); stdputstr(buf);
    for (j=x1; j<=x2; j++){
#if FLOATING
      (void)sprintf(buf,"%6.3f",(float)p[z*nx*ny+i*nx+j]); stdputstr(buf);
#else
      (void)sprintf(buf,"%6ld",(long int)p[z*nx*ny+i*nx+j]); stdputstr(buf);
#endif
    }
    (void)sprintf(buf,"\n"); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "i64_undef.h"

#include "u64_def.h"
ERROR_TYPE u64_dumpxyz(IMAGE *im, int x, int y, int z, int dx, int dy)
{
  int y1, y2, x1, x2;
  int i, j;
  int nx, ny, nz;
  PIX_TYPE *p;

  p=(PIX_TYPE *)GetImPtr(im);

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);

  x = MIN(abs(x),nx-1);
  y = MIN(abs(y),ny-1);
  z = MIN(abs(z),nz-1);


  x1 = x-dx/2;
  if (x1<0)
    x1=0;

  y1 = y-dy/2;
  if (y1<0)
    y1=0;

  x2 = x+dx/2;
  if (x2>=nx)
    x2=nx-1;

  y2 = y+dy/2;
  if (y2>=ny)
    y2=ny-1;

  (void)sprintf(buf,"z=%4d  ", z); stdputstr(buf);
  for (i=x1; i<=x2; i++){
    (void)sprintf(buf," %4d|",i); stdputstr(buf);
  }
  (void)sprintf(buf,"\n"); stdputstr(buf);
  for (i=y1; i<=y2; i++){
    (void)sprintf(buf,"|%4d|",i); stdputstr(buf);
    for (j=x1; j<=x2; j++){
#if FLOATING
      (void)sprintf(buf,"%6.3f",(float)p[z*nx*ny+i*nx+j]); stdputstr(buf);
#else
      (void)sprintf(buf,"%6lu",(long unsigned int)p[z*nx*ny+i*nx+j]); stdputstr(buf);
#endif
    }
    (void)sprintf(buf,"\n"); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "u64_undef.h"

#include "f_def.h"
ERROR_TYPE f_dumpxyz(IMAGE *im, int x, int y, int z, int dx, int dy)
{
  int y1, y2, x1, x2;
  int i, j;
  int nx, ny, nz;
  PIX_TYPE *p;

  p=(PIX_TYPE *)GetImPtr(im);

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);

  x = MIN(abs(x),nx-1);
  y = MIN(abs(y),ny-1);
  z = MIN(abs(z),nz-1);


  x1 = x-dx/2;
  if (x1<0)
    x1=0;

  y1 = y-dy/2;
  if (y1<0)
    y1=0;

  x2 = x+dx/2;
  if (x2>=nx)
    x2=nx-1;

  y2 = y+dy/2;
  if (y2>=ny)
    y2=ny-1;

  (void)sprintf(buf,"z=%4d  ", z); stdputstr(buf);
  for (i=x1; i<=x2; i++){
    (void)sprintf(buf,"%9d|",i); stdputstr(buf);
  }
  (void)sprintf(buf,"\n"); stdputstr(buf);
  for (i=y1; i<=y2; i++){
    (void)sprintf(buf,"|%4d|",i); stdputstr(buf);
    for (j=x1; j<=x2; j++){
#if FLOATING
      (void)sprintf(buf,"%10.3f",(float)p[z*nx*ny+i*nx+j]); stdputstr(buf);
#else
      (void)sprintf(buf,"%6d",(int)p[z*nx*ny+i*nx+j]); stdputstr(buf);
#endif
    }
    (void)sprintf(buf,"\n"); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "f_undef.h"


#include "d_def.h"
ERROR_TYPE d_dumpxyz(IMAGE *im, int x, int y, int z, int dx, int dy)
{
  int y1, y2, x1, x2;
  int i, j;
  int nx, ny, nz;
  PIX_TYPE *p;

  p=(PIX_TYPE *)GetImPtr(im);

  nx = GetImNx(im);
  ny = GetImNy(im);
  nz = GetImNz(im);

  x = MIN(abs(x),nx-1);
  y = MIN(abs(y),ny-1);
  z = MIN(abs(z),nz-1);

  x1 = x-dx/2;
  if (x1<0)
    x1=0;

  y1 = y-dy/2;
  if (y1<0)
    y1=0;

  x2 = x+dx/2;
  if (x2>=nx)
    x2=nx-1;

  y2 = y+dy/2;
  if (y2>=ny)
    y2=ny-1;

  (void)sprintf(buf,"z=%4d  ", z); stdputstr(buf);
  for (i=x1; i<=x2; i++){
    (void)sprintf(buf," %9d|",i); stdputstr(buf);
  }
  (void)sprintf(buf,"\n"); stdputstr(buf);
  for (i=y1; i<=y2; i++){
    (void)sprintf(buf,"|%4d|",i); stdputstr(buf);
    for (j=x1; j<=x2; j++){
#if FLOATING
      (void)sprintf(buf,"%10.3f",(float)p[z*nx*ny+i*nx+j]); stdputstr(buf);
#else
      (void)sprintf(buf,"%6d",(int)p[z*nx*ny+i*nx+j]); stdputstr(buf);
#endif
    }
    (void)sprintf(buf,"\n"); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "d_undef.h"



ERROR_TYPE dumpxyz(IMAGE *im, int x, int y, int z, int dx, int dy)
{
  switch (GetImDataType(im)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_dumpxyz(im, x, y, z, dx, dy));
    break;
#endif
#ifndef NO_uc_IMAGE
  case t_UCHAR:
    return(uc_dumpxyz(im, x, y, z, dx, dy));
    break;
#endif

  case t_SHORT:
    return(s_dumpxyz(im, x, y, z, dx, dy));
    break;
  case t_USHORT:
    return(us_dumpxyz(im, x, y, z, dx, dy));
    break;


  case t_INT32:
    return(i32_dumpxyz(im, x, y, z, dx, dy));
    break;
  case t_UINT32:
    return(u32_dumpxyz(im, x, y, z, dx, dy));
    break;

  case t_INT64:
    return(i64_dumpxyz(im, x, y, z, dx, dy));
    break;
  case t_UINT64:
    return(u64_dumpxyz(im, x, y, z, dx, dy));
    break;

  case t_FLOAT:
    return(f_dumpxyz(im, x, y, z, dx, dy));
    break;

  case t_DOUBLE:
    return(d_dumpxyz(im, x, y, z, dx, dy));
    break;

  default:
    (void) sprintf(buf, "dumpxyz(): invalid pixel type\n"); errputstr(buf);
    return ERROR;
  }
  return NO_ERROR;
}

/**@}*/
