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
#include "miallib.h"

#ifdef XLISP
extern void xlabort(char *);    /* serious XLISP error handler */
#endif


void nrerror(char *error_text)
{
  sprintf(buf,"Image library run-time error...\n"); errputstr(buf);
  sprintf(buf,"%s\n",error_text); errputstr(buf);
  sprintf(buf,"...now exiting to top-level...\n"); errputstr(buf);
#ifdef XLISP
  xlabort("Emergency Emergency");
#else
  exit(1);
#endif
}

/** \addtogroup group_format
 *  @{
 */



#include "uc_def.h"
IMAGE *uc_deinterleave(IMAGE *im)
{
  IMAGE *imout;
  PIX_TYPE *pin, *pout;
  unsigned long int nelem, x, j, nz=GetImNz(im);

  imout = create_image(GetImDataType(im), GetImNx(im), GetImNy(im), GetImNz(im));
  if (imout == NULL){
    (void)sprintf(buf,"deinterleave(im): not enough memory!\n"); errputstr(buf);
    return(NULL);
  }

  nelem=GetImNPixPerPlane(im);
  pin=(PIX_TYPE *)GetImPtr(im);
  pout=(PIX_TYPE *)GetImPtr(imout);
  for (x=0;x<nelem ; x++){
    for (j=0; j<nz; j++){
      *(pout+j*nelem)=*pin++;
    }
    pout++;
  }
  return imout;
}
#include "uc_undef.h"

#include "us_def.h"
IMAGE *us_deinterleave(IMAGE *im)
{
  IMAGE *imout;
  PIX_TYPE *pin, *pout;
  unsigned long int nelem, x, j, nz=GetImNz(im);

  imout = create_image(GetImDataType(im), GetImNx(im), GetImNy(im), GetImNz(im));
  if (imout == NULL){
    (void)sprintf(buf,"deinterleave(im): not enough memory!\n"); errputstr(buf);
    return(NULL);
  }

  nelem=GetImNPixPerPlane(im);
  pin=(PIX_TYPE *)GetImPtr(im);
  pout=(PIX_TYPE *)GetImPtr(imout);
  for (x=0;x<nelem ; x++){
    for (j=0; j<nz; j++){
      *(pout+j*nelem)=*pin++;
    }
    pout++;
  }
  return imout;
}
#include "us_undef.h"

#include "u32_def.h"
IMAGE *u32_deinterleave(IMAGE *im)
{
  IMAGE *imout;
  PIX_TYPE *pin, *pout;
  unsigned long int nelem, x, j, nz=GetImNz(im);

  imout = create_image(GetImDataType(im), GetImNx(im), GetImNy(im), GetImNz(im));
  if (imout == NULL){
    (void)sprintf(buf,"deinterleave(im): not enough memory!\n"); errputstr(buf);
    return(NULL);
  }

  nelem=GetImNPixPerPlane(im);
  pin=(PIX_TYPE *)GetImPtr(im);
  pout=(PIX_TYPE *)GetImPtr(imout);
  for (x=0;x<nelem ; x++){
    for (j=0; j<nz; j++){
      *(pout+j*nelem)=*pin++;
    }
    pout++;
  }
  return imout;
}
#include "u32_undef.h"


/* deinterleave (bip2bsq): useful when reading TIFF image with chunky format */


/**
 * @synopsis  converts a multiband image, represented as a 3D image, from band interleaved format to band sequential format
 *
 * @param an image with band interleaved representation
 *
 * @return an image with band sequential representation
 *
 */
IMAGE *deinterleave(IMAGE *im)
{
  switch (GetImDataType(im)){
  case t_UCHAR:
    return(uc_deinterleave(im));
    break;
  case t_USHORT:
    return(us_deinterleave(im));
    break;
  case t_UINT32:
  case t_INT32:
  case t_FLOAT:
    return(u32_deinterleave(im));
    break;
  default:
    (void)sprintf(buf,"deinterleave(im): invalid pixel type\n"); errputstr(buf);
    return(NULL);
  }
}

/*@}*/
