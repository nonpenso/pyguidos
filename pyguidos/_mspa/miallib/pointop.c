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
#include "op.h"

#ifdef OPENMP
#include <omp.h>
#endif

/** @defgroup group_pointop Point operations
 *  Functions that are operating at the level of pixels without any neighbourhood information.
 *  @{
 */



/*************************************************************************/
/*                 im1[i] = im1[i] bit_op im2[i]                         */


ERROR_TYPE bitwise_op(IMAGE *im1, IMAGE *im2, int op)
{
  long int *pim1, *pim2;
  mia_size_t i, nbyte, nword;

  nbyte = GetImNByte(im1);
  nword=nbyte/sizeof(long int);

  pim1 = (long int *)GetImPtr(im1);
  pim2 = (long int *)GetImPtr(im2);

  /* error checking */
  if (GetImNByte(im2) != nbyte){
    (void)sprintf(buf, "ERROR in bitwise_op(im1, im2, op): images do not have the same number of bytes\n"); errputstr(buf);
    return(ERROR);
  }

  /* here we go */
  switch(op){
  case AND_op: /* bitwise AND */
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<nword; i++)
      pim1[i] &= pim2[i];
    break;
  case OR_op: /* bitwise OR */
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<nword; i++)
      pim1[i] |= pim2[i];
    break;
  case XOR_op: /* bitwise XOR */
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<nword; i++)
      pim1[i] ^= pim2[i];
    break;
  case NAND_op: /* bitwise AND */
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<nword; i++){
      pim1[i] &= pim2[i];
      pim1[i] = ~pim1[i];
    }
    break;
  default:
    (void)sprintf(buf, "ERROR in bitwise_op(im1, im2, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}


/*************************************************************************/
/*                      im = negation(im)                                     */
ERROR_TYPE negation(IMAGE *im)
{
  unsigned long int i, nbyte, npix;

  nbyte = GetImNByte(im);
  npix = GetImNPix(im);

  /* here we go */
  switch (GetImDataType(im)){

#ifndef NO_bi_IMAGE
  case t_ONEBITPERPIXEL:{
    UCHAR *pim;
    pim = (UCHAR *)GetImPtr(im);
    for (i=0; i<nbyte; i++, pim++)
      *pim = ~*pim;
  }
    break;
#endif

  case t_UCHAR:{
    UCHAR *pim;
    pim = (UCHAR *)GetImPtr(im);
    for (i=0; i<nbyte; i++, pim++){
      if (*pim)
	*pim = 0;
      else
	*pim = 1;
    }
  }
    break;

  case t_USHORT:{
    USHORT *pim;
    pim = (USHORT *)GetImPtr(im);
    for (i=0; i<npix; i++, pim++){
      if (*pim)
	*pim = 0;
      else
	*pim = 1;
    }
  }
    break;

  case t_UINT32:{
    UINT32 *pim;
    pim = (UINT32 *)GetImPtr(im);
    for (i=0; i<npix; i++, pim++){
      if (*pim)
	*pim = 0;
      else
	*pim = 1;
    }
  }
    break;

  default:
    (void)sprintf(buf, "ERROR in negation(im): \
                invalid ImDataType\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}


/*************************************************************************/
/*                     im1[i] = im1[i] arith_op im2[i]                   */

#ifndef NO_generic_IMAGE
#include "g_def.h"
ERROR_TYPE generic_arith(IMAGE *im1, IMAGE *im2, int op)
{
  mia_size_t i, ovfl = 0, npix;
#if OVFL_TEST
  long int test;
#endif
  PIX_TYPE *p1, *p2;
  //pk 24/04/2023 Dangerous: should be defined as local variable within parallel loop
  /* double dval; */

  p1 = (PIX_TYPE *)GetImPtr(im1);
  p2 = (PIX_TYPE *)GetImPtr(im2);

  npix = GetImNPix(im1);

  switch(op){
  case AND_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++)
      p1[i] &= p2[i];
    break;
  case OR_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++)
      p1[i] |= p2[i];
    break;
  case XOR_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++)
      p1[i] ^= p2[i];
    break;
  case ADD_op_ovfl: /* allow overflow */
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++)
      p1[i] += p2[i];
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
#ifdef OPENMP
#pragma omp parallel for private(test) reduction(+:ovfl)
#endif
    for (i=0; i<npix; i++){
      test = p1[i] + p2[i];
      if (test > PIX_MAX){
	ovfl++;
	test = PIX_MAX;
      }
      p1[i] = (PIX_TYPE)test;
    }
    break;
  case SUB_op:
#ifdef OPENMP
#pragma omp parallel for private(test) reduction(+:ovfl)
#endif
    for (i=0; i<npix; i++){
      test = p1[i] - p2[i];
      if (test < PIX_MIN){
	ovfl++;
	test = PIX_MIN;
      }
      p1[i] = (PIX_TYPE)test;
    }
    break;
  case SUBSWAP_op:
#ifdef OPENMP
#pragma omp parallel for private(test) reduction(+:ovfl)
#endif
    for (i=0; i<npix; i++){
      test = p2[i] - p1[i];
      if (test < PIX_MIN){
	ovfl++;
	test = PIX_MIN;
      }
      p1[i] = (PIX_TYPE)test;
    }
    break;
  case SUB_op_ovfl:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++){
      p1[i] -= p2[i];
    }
    break;
  case SUBSWAP_op_ovfl:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++){
      p1[i] = p2[i] - p1[i];
    }
    break;
  case ABSSUB_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++){
#if FLOATING
      p1[i] = (PIX_TYPE)fabs((double)p1[i]-(double)p2[i]);
#else
      p1[i] = (PIX_TYPE)abs((int)p1[i]-(int)p2[i]);
#endif
    }
    break;
  case MULT_op:
#ifdef OPENMP
#pragma omp parallel for private(test) reduction(+:ovfl)
#endif
    for (i=0; i<npix; i++)  {
      test = p1[i] * p2[i];
      if (test > PIX_MAX){
	ovfl++;
	test = PIX_MAX;
      }
      p1[i] = (PIX_TYPE)test;
    }
    break;
  case MULT_op_ovfl:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++)
      p1[i] *=  p2[i];
    break;
  case DIV_op:
#ifdef OPENMP
#pragma omp parallel for reduction(+:ovfl)
#endif
    for (i=0; i<npix; i++){
      if (p2[i] == (PIX_TYPE)0){
	if (p1[i] != 0){
#if SIGNED
	p1[i] = SGN(p1[i])*PIX_MAX;
#else
	p1[i] = (p1[i] & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	p1[i] /=  p2[i];
    }
    break;
  case INF_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++)
      if (p2[i] < p1[i])
	p1[i] = p2[i];
    break;
  case SUP_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++)
      if (p2[i] > p1[i])
	p1[i] = p2[i];
    break;
  case MASK_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++)
      if (p2[i] != (PIX_TYPE)0)
	p1[i] = p2[i];
    break;
  case MASK_op2:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++)
      if (p1[i] ==(PIX_TYPE) 0)
	p1[i] = p2[i];
    break;
  case CMP_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++){
      if (p1[i] < p2[i])
	p1[i] = 1;
      else if (p1[i] > p2[i])
	p1[i] = 2;
      else
	p1[i] = 0;
    }
    break;
  case EQUAL_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++){
      if (p1[i] == p2[i])
	p1[i] = 1;
      else
	p1[i] = 0;
    }
    break;
  case NDI_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++){
      double dval=(double)p1[i]+(double)p2[i];
      if (dval==0.0)
	p1[i]=PIX_MAX;
      else
	p1[i]=((double)p1[i]-(double)p2[i])/dval;
    }
    break;
  default:
    (void)sprintf(buf, "ERROR in generic_arith(im1, im2, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in generic_arith(im1, im2, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */

#include "s_def.h"
ERROR_TYPE s_arith(IMAGE *im1, IMAGE *im2, int op)
{
  mia_size_t i, ovfl = 0, npix;
#if OVFL_TEST
  long int test;
#endif
  PIX_TYPE *p1, *p2;
  //pk 24/04/2023 prepare for parallel loop in future...
  /* double dval; */

  p1 = (PIX_TYPE *)GetImPtr(im1);
  p2 = (PIX_TYPE *)GetImPtr(im2);

  npix = GetImNPix(im1);

  switch(op){
  case AND_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 &= *p2;
    }
    break;
  case OR_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 |= *p2;
    }
    break;
  case XOR_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 ^= *p2;
    }
    break;
  case ADD_op_ovfl: /* allow overflow */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 += *p2;
    }
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
    for (i=0; i<npix; i++, p1++, p2++){
      test = *p1 + *p2;
      if (test > PIX_MAX){
	ovfl++;
	test = PIX_MAX;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case SUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
      test = *p1 - *p2;
      if (test < PIX_MIN){
	ovfl++;
	test = PIX_MIN;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case SUBSWAP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      test = *p2 - *p1;
      if (test < PIX_MIN){
	ovfl++;
	test = PIX_MIN;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case SUB_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 -= *p2;
    }
    break;
  case SUBSWAP_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p2 - *p1;
    }
    break;
  case ABSSUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
#if FLOATING
      *p1 = (PIX_TYPE)fabs((double)*p1-(double)*p2);
#else
      *p1 = (PIX_TYPE)abs((int)*p1-(int)*p2);
#endif
    }
    break;
  case MULT_op:
    for (i=0; i<npix; i++, p1++, p2++)  {
      test = *p1 * *p2;
      if (test > PIX_MAX){
	ovfl++;
	test = PIX_MAX;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case MULT_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++)
      *p1 *=  *p2;
    break;
  case DIV_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p2 == (PIX_TYPE)0){
	if (*p1 != 0){
#if SIGNED
	*p1 = SGN(*p1)*PIX_MAX;
#else
	*p1 = (*p1 & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	*p1 /=  *p2;
    }
    break;
  case INF_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 < *p1)
	*p1 = *p2;
    break;
  case SUP_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 > *p1)
	*p1 = *p2;
    break;
  case MASK_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 != (PIX_TYPE)0)
	*p1 = *p2;
    break;
  case MASK_op2:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p1 == (PIX_TYPE)0)
	*p1 = *p2;
    break;
  case CMP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 < *p2)
	*p1 = 1;
      else if (*p1 > *p2)
	*p1 = 2;
      else
	*p1 = 0;
    }
    break;
  case EQUAL_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 == *p2)
	*p1 = 1;
      else
	*p1 = 0;
    }
    break;
  case NDI_op:
    for (i=0; i<npix; i++, p1++, p2++){
      double dval=(double)*p1+(double)*p2;
      if (dval==0.)
	*p1=PIX_MAX;
      else
	*p1=((double)*p1-(double)*p2)/dval;
    }
    break;
  default:
    (void)sprintf(buf, "ERROR in s_arith(im1, im2, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in s_arith(im1, im2, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "s_undef.h"

#include "us_def.h"
ERROR_TYPE us_arith(IMAGE *im1, IMAGE *im2, int op)
{
  mia_size_t i, ovfl = 0, npix;
#if OVFL_TEST
  long int test;
#endif
  PIX_TYPE *p1, *p2;
  //pk 24/04/2023 prepare for parallel loop in future...
  /* double dval; */

  p1 = (PIX_TYPE *)GetImPtr(im1);
  p2 = (PIX_TYPE *)GetImPtr(im2);

  npix = GetImNPix(im1);

  switch(op){
  case AND_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 &= *p2;
    }
    break;
  case OR_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 |= *p2;
    }
    break;
  case XOR_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 ^= *p2;
    }
    break;
  case ADD_op_ovfl: /* allow overflow */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 += *p2;
    }
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
    for (i=0; i<npix; i++, p1++, p2++){
      test = *p1 + *p2;
      if (test > PIX_MAX){
	ovfl++;
	test = PIX_MAX;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case SUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
      test = *p1 - *p2;
      if (test < PIX_MIN){
	ovfl++;
	test = PIX_MIN;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case SUBSWAP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      test = *p2 - *p1;
      if (test < PIX_MIN){
	ovfl++;
	test = PIX_MIN;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case SUB_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 -= *p2;
    }
    break;
  case SUBSWAP_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p2 - *p1;
    }
    break;
  case ABSSUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
#if FLOATING
      *p1 = (PIX_TYPE)fabs((double)*p1-(double)*p2);
#else
      *p1 = (PIX_TYPE)abs((int)*p1-(int)*p2);
#endif
    }
    break;
  case MULT_op:
    for (i=0; i<npix; i++, p1++, p2++)  {
      test = *p1 * *p2;
      if (test > PIX_MAX){
	ovfl++;
	test = PIX_MAX;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case MULT_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++)
      *p1 *=  *p2;
    break;
  case DIV_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p2 == (PIX_TYPE)0){
	if (*p1 != 0){
#if SIGNED
	*p1 = SGN(*p1)*PIX_MAX;
#else
	*p1 = (*p1 & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	*p1 /=  *p2;
    }
    break;
  case INF_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 < *p1)
	*p1 = *p2;
    break;
  case SUP_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 > *p1)
	*p1 = *p2;
    break;
  case MASK_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 != 0)
	*p1 = *p2;
    break;
  case MASK_op2:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p1 == 0)
	*p1 = *p2;
    break;
  case CMP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 < *p2)
	*p1 = 1;
      else if (*p1 > *p2)
	*p1 = 2;
      else
	*p1 = 0;
    }
    break;
  case EQUAL_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 == *p2)
	*p1 = 1;
      else
	*p1 = 0;
    }
    break;
  case NDI_op:
    for (i=0; i<npix; i++, p1++, p2++){
      double dval=(double)*p1+(double)*p2;
      if (dval==0.)
	*p1=PIX_MAX;
      else
	*p1=((double)*p1-(double)*p2)/dval;
    }
    break;
  default:
    (void)sprintf(buf, "ERROR in generic_arith(im1, im2, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in generic_arith(im1, im2, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "us_undef.h"

#include "us_def.h"
#define PIX_TYPE2 UCHAR
ERROR_TYPE usuc_arith(IMAGE *im1, IMAGE *im2, int op)
{
  mia_size_t i, ovfl = 0, npix;
#if OVFL_TEST
  long int test;
#endif
  PIX_TYPE *p1;
  PIX_TYPE2 *p2;
  //pk 24/04/2023 prepare for parallel loop in future...
  /* double dval; */

  p1 = (PIX_TYPE *)GetImPtr(im1);
  p2 = (PIX_TYPE2 *)GetImPtr(im2);

  npix = GetImNPix(im1);

  switch(op){
  case AND_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 &= *p2;
    }
    break;
  case OR_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 |= *p2;
    }
    break;
  case XOR_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 ^= *p2;
    }
    break;
  case ADD_op_ovfl: /* allow overflow */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 += *p2;
    }
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
    for (i=0; i<npix; i++, p1++, p2++){
      test = *p1 + *p2;
      if (test > PIX_MAX){
	ovfl++;
	test = PIX_MAX;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case SUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
      test = *p1 - *p2;
      if (test < PIX_MIN){
	ovfl++;
	test = PIX_MIN;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case SUBSWAP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      test = *p2 - *p1;
      if (test < PIX_MIN){
	ovfl++;
	test = PIX_MIN;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case SUB_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 -= *p2;
    }
    break;
  case SUBSWAP_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p2 - *p1;
    }
    break;
  case ABSSUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
#if FLOATING
      *p1 = (PIX_TYPE)fabs((double)*p1-(double)*p2);
#else
      *p1 = (PIX_TYPE)abs((int)*p1-(int)*p2);
#endif
    }
    break;
  case MULT_op:
    for (i=0; i<npix; i++, p1++, p2++)  {
      test = *p1 * *p2;
      if (test > PIX_MAX){
	ovfl++;
	test = PIX_MAX;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case MULT_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++)
      *p1 *=  *p2;
    break;
  case DIV_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p2 == (PIX_TYPE2)0){
	if (*p1 != 0){
#if SIGNED
	*p1 = SGN(*p1)*PIX_MAX;
#else
	*p1 = (*p1 & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	*p1 /=  *p2;
    }
    break;
  case INF_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 < *p1)
	*p1 = *p2;
    break;
  case SUP_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 > *p1)
	*p1 = *p2;
    break;
  case MASK_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 !=(PIX_TYPE2) 0)
	*p1 = *p2;
    break;
  case MASK_op2:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p1 == (PIX_TYPE)0)
	*p1 = *p2;
    break;
  case CMP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 < *p2)
	*p1 = 1;
      else if (*p1 > *p2)
	*p1 = 2;
      else
	*p1 = 0;
    }
    break;
  case EQUAL_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 == *p2)
	*p1 = 1;
      else
	*p1 = 0;
    }
    break;
  case NDI_op:
    for (i=0; i<npix; i++, p1++, p2++){
      double dval=(double)*p1+(double)*p2;
      if (dval==0.)
	*p1=PIX_MAX;
      else
	*p1=((double)*p1-(double)*p2)/dval;
    }
    break;
  default:
    (void)sprintf(buf, "ERROR in generic_arith(im1, im2, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in usuc_arith(im1, im2, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#undef PIX_TYPE2
#include "us_undef.h"


#include "s_def.h"
#define PIX_TYPE2 UCHAR
ERROR_TYPE suc_arith(IMAGE *im1, IMAGE *im2, int op)
{
  mia_size_t i, ovfl = 0, npix;
#if OVFL_TEST
  long int test;
#endif
  PIX_TYPE *p1;
  PIX_TYPE2 *p2;
  //pk 24/04/2023 prepare for parallel loop in future...
  /* double dval; */

  p1 = (PIX_TYPE *)GetImPtr(im1);
  p2 = (PIX_TYPE2 *)GetImPtr(im2);

  npix = GetImNPix(im1);

  switch(op){
  case AND_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 &= *p2;
    }
    break;
  case OR_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 |= *p2;
    }
    break;
  case XOR_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 ^= *p2;
    }
    break;
  case ADD_op_ovfl: /* allow overflow */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 += *p2;
    }
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
    for (i=0; i<npix; i++, p1++, p2++){
      test = *p1 + *p2;
      if (test > PIX_MAX){
	ovfl++;
	test = PIX_MAX;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case SUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
      test = *p1 - *p2;
      if (test < PIX_MIN){
	ovfl++;
	test = PIX_MIN;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case SUBSWAP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      test = *p2 - *p1;
      if (test < PIX_MIN){
	ovfl++;
	test = PIX_MIN;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case SUB_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 -= *p2;
    }
    break;
  case SUBSWAP_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p2 - *p1;
    }
    break;
  case ABSSUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
#if FLOATING
      *p1 = (PIX_TYPE)fabs((double)*p1-(double)*p2);
#else
      *p1 = (PIX_TYPE)abs((int)*p1-(int)*p2);
#endif
    }
    break;
  case MULT_op:
    for (i=0; i<npix; i++, p1++, p2++)  {
      test = *p1 * *p2;
      if (test > PIX_MAX){
	ovfl++;
	test = PIX_MAX;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case MULT_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++)
      *p1 *=  *p2;
    break;
  case DIV_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p2 == 0){
	if (*p1 != 0){
#if SIGNED
	*p1 = SGN(*p1)*PIX_MAX;
#else
	*p1 = (*p1 & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	*p1 /=  *p2;
    }
    break;
  case INF_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 < *p1)
	*p1 = *p2;
    break;
  case SUP_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 > *p1)
	*p1 = *p2;
    break;
  case MASK_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 != 0)
	*p1 = *p2;
    break;
  case MASK_op2:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p1 == 0)
	*p1 = *p2;
    break;
  case CMP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 < *p2)
	*p1 = 1;
      else if (*p1 > *p2)
	*p1 = 2;
      else
	*p1 = 0;
    }
    break;
  case EQUAL_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 == *p2)
	*p1 = 1;
      else
	*p1 = 0;
    }
    break;
  case NDI_op:
    for (i=0; i<npix; i++, p1++, p2++){
      double dval=(double)*p1+(double)*p2;
      if (dval==0.)
	*p1=PIX_MAX;
      else
	*p1=((double)*p1-(double)*p2)/dval;
    }
    break;
  default:
    (void)sprintf(buf, "ERROR in generic_arith(im1, im2, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in usuc_arith(im1, im2, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#undef PIX_TYPE2
#include "s_undef.h"


#include "i32_def.h"
#define PIX_TYPE2 UCHAR
ERROR_TYPE luc_arith(IMAGE *im1, IMAGE *im2, int op)
{
  mia_size_t i, ovfl = 0, npix;
#if OVFL_TEST
  long int test;
#endif
  PIX_TYPE *p1;
  PIX_TYPE2 *p2;
  //pk 24/04/2023 prepare for parallel loop in future...
  /* double dval; */

  p1 = (PIX_TYPE *)GetImPtr(im1);
  p2 = (PIX_TYPE2 *)GetImPtr(im2);

  npix = GetImNPix(im1);

  switch(op){
  case AND_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 &= *p2;
    }
    break;
  case OR_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 |= *p2;
    }
    break;
  case XOR_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 ^= *p2;
    }
    break;
  case ADD_op_ovfl: /* allow overflow */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 += *p2;
    }
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p1 + *p2;
    }
    break;
  case SUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p1 - *p2;
    }
    break;
  case SUBSWAP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 =  *p2 - *p1;
    }
    break;
  case SUB_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 -= *p2;
    }
    break;
  case SUBSWAP_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p2 - *p1;
    }
    break;
  case ABSSUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
#if FLOATING
      *p1 = (PIX_TYPE)fabs((double)*p1-(double)*p2);
#else
      *p1 = (PIX_TYPE)abs((int)*p1-(int)*p2);
#endif
    }
    break;
  case MULT_op:
    for (i=0; i<npix; i++, p1++, p2++)  {
      *p1 =  *p1 * *p2;
    }
    break;
  case MULT_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++)
      *p1 *=  *p2;
    break;
  case DIV_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p2 == 0){
	if (*p1 != 0){
#if SIGNED
	*p1 = SGN(*p1)*PIX_MAX;
#else
	*p1 = (*p1 & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	*p1 /=  *p2;
    }
    break;
  case INF_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 < *p1)
	*p1 = *p2;
    break;
  case SUP_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 > *p1)
	*p1 = *p2;
    break;
  case MASK_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 != 0)
	*p1 = *p2;
    break;
  case MASK_op2:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p1 == 0)
	*p1 = *p2;
    break;
  case CMP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 < *p2)
	*p1 = 1;
      else if (*p1 > *p2)
	*p1 = 2;
      else
	*p1 = 0;
    }
    break;
  case EQUAL_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 == *p2)
	*p1 = 1;
      else
	*p1 = 0;
    }
    break;
  case NDI_op:
    for (i=0; i<npix; i++, p1++, p2++){
      double dval=(double)*p1+(double)*p2;
      if (dval==0.)
	*p1=PIX_MAX;
      else
	*p1=((double)*p1-(double)*p2)/dval;
    }
    break;
  default:
    (void)sprintf(buf, "ERROR in generic_arith(im1, im2, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in usuc_arith(im1, im2, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#undef PIX_TYPE2
#include "i32_undef.h"


#include "i32_def.h"
#define PIX_TYPE2 USHORT
ERROR_TYPE lus_arith(IMAGE *im1, IMAGE *im2, int op)
{
  mia_size_t i, ovfl = 0, npix;
#if OVFL_TEST
  long int test;
#endif
  PIX_TYPE *p1;
  PIX_TYPE2 *p2;
  //pk 24/04/2023 prepare for parallel loop in future...
  /* double dval; */

  p1 = (PIX_TYPE *)GetImPtr(im1);
  p2 = (PIX_TYPE2 *)GetImPtr(im2);

  npix = GetImNPix(im1);

  switch(op){
  case AND_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 &= *p2;
    }
    break;
  case OR_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 |= *p2;
    }
    break;
  case XOR_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 ^= *p2;
    }
    break;
  case ADD_op_ovfl: /* allow overflow */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 += *p2;
    }
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p1 + *p2;
    }
    break;
  case SUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p1 - *p2;
    }
    break;
  case SUBSWAP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 =  *p2 - *p1;
    }
    break;
  case SUB_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 -= *p2;
    }
    break;
  case SUBSWAP_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p2 - *p1;
    }
    break;
  case ABSSUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
#if FLOATING
      *p1 = (PIX_TYPE)fabs((double)*p1-(double)*p2);
#else
      *p1 = (PIX_TYPE)abs((int)*p1-(int)*p2);
#endif
    }
    break;
  case MULT_op:
    for (i=0; i<npix; i++, p1++, p2++)  {
      *p1 =  *p1 * *p2;
    }
    break;
  case MULT_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++)
      *p1 *=  *p2;
    break;
  case DIV_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p2 == 0){
	if (*p1 != 0){
#if SIGNED
	*p1 = SGN(*p1)*PIX_MAX;
#else
	*p1 = (*p1 & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	*p1 /=  *p2;
    }
    break;
  case INF_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 < *p1)
	*p1 = *p2;
    break;
  case SUP_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 > *p1)
	*p1 = *p2;
    break;
  case MASK_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 != 0)
	*p1 = *p2;
    break;
  case MASK_op2:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p1 == 0)
	*p1 = *p2;
    break;
  case CMP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 < *p2)
	*p1 = 1;
      else if (*p1 > *p2)
	*p1 = 2;
      else
	*p1 = 0;
    }
    break;
  case EQUAL_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 == *p2)
	*p1 = 1;
      else
	*p1 = 0;
    }
    break;
  case NDI_op:
    for (i=0; i<npix; i++, p1++, p2++){
      double dval=(double)*p1+(double)*p2;
      if (dval==0.)
	*p1=PIX_MAX;
      else
	*p1=((double)*p1-(double)*p2)/dval;
    }
    break;
  default:
    (void)sprintf(buf, "ERROR in generic_arith(im1, im2, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in usuc_arith(im1, im2, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#undef PIX_TYPE2
#include "i32_undef.h"




#include "i32_def.h"
ERROR_TYPE i32_arith(IMAGE *im1, IMAGE *im2, int op)
{
  mia_size_t i, ovfl = 0, npix;
#if OVFL_TEST
  long int test;
#endif
  PIX_TYPE *p1, *p2;
  //pk 24/04/2023 prepare for parallel loop in future...
  /* double dval; */

  p1 = (PIX_TYPE *)GetImPtr(im1);
  p2 = (PIX_TYPE *)GetImPtr(im2);

  npix = GetImNPix(im1);

  switch(op){
  case AND_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 &= *p2;
    }
    break;
  case OR_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 |= *p2;
    }
    break;
  case XOR_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 ^= *p2;
    }
    break;
  case ADD_op_ovfl: /* allow overflow */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 += *p2;
    }
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1+=*p2;
    }
    break;
  case SUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1-=*p2;
    }
    break;
  case SUBSWAP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p2-*p1;
    }
    break;
  case SUB_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 -= *p2;
    }
    break;
  case SUBSWAP_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p2 - *p1;
    }
    break;
  case ABSSUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
#if FLOATING
      *p1 = (PIX_TYPE)fabs((double)*p1-(double)*p2);
#else
      *p1 = (PIX_TYPE)abs((int)*p1-(int)*p2);
#endif
    }
    break;
  case MULT_op:
    for (i=0; i<npix; i++, p1++, p2++)  {
      *p1 *= *p2;
    }
    break;
  case MULT_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++)
      *p1 *=  *p2;
    break;
  case DIV_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p2 == 0){
	if (*p1 != 0){
#if SIGNED
	*p1 = SGN(*p1)*PIX_MAX;
#else
	*p1 = (*p1 & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	*p1 /=  *p2;
    }
    break;
  case INF_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 < *p1)
	*p1 = *p2;
    break;
  case SUP_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 > *p1)
	*p1 = *p2;
    break;
  case MASK_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 != 0)
	*p1 = *p2;
    break;
  case MASK_op2:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p1 == 0)
	*p1 = *p2;
    break;
  case CMP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 < *p2)
	*p1 = 1;
      else if (*p1 > *p2)
	*p1 = 2;
      else
	*p1 = 0;
    }
    break;
  case EQUAL_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 == *p2)
	*p1 = 1;
      else
	*p1 = 0;
    }
    break;
  case NDI_op:
    for (i=0; i<npix; i++, p1++, p2++){
      double dval=(double)*p1+(double)*p2;
      if (dval==0.)
	*p1=PIX_MAX;
      else
	*p1=((double)*p1-(double)*p2)/dval;
    }
    break;
  default:
    (void)sprintf(buf, "ERROR in generic_arith(im1, im2, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in generic_arith(im1, im2, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "i32_undef.h"


#include "u32_def.h"
ERROR_TYPE u32_arith(IMAGE *im1, IMAGE *im2, int op)
{
  mia_size_t i, ovfl = 0, npix;
#if OVFL_TEST
  long int test;
#endif
  PIX_TYPE *p1, *p2;
  //pk 24/04/2023 prepare for parallel loop in future...
  /* double dval; */

  p1 = (PIX_TYPE *)GetImPtr(im1);
  p2 = (PIX_TYPE *)GetImPtr(im2);

  npix = GetImNPix(im1);

  switch(op){
  case AND_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 &= *p2;
    }
    break;
  case OR_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 |= *p2;
    }
    break;
  case XOR_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 ^= *p2;
    }
    break;
  case ADD_op_ovfl: /* allow overflow */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 += *p2;
    }
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1+=*p2;
    }
    break;
  case SUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1-=*p2;
    }
    break;
  case SUBSWAP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p2-*p1;
    }
    break;
  case SUB_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 -= *p2;
    }
    break;
  case SUBSWAP_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p2 - *p1;
    }
    break;
  case ABSSUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
#if FLOATING
      *p1 = (PIX_TYPE)fabs((double)*p1-(double)*p2);
#else
      *p1 = (PIX_TYPE)abs((int)*p1-(int)*p2);
#endif
    }
    break;
  case MULT_op:
    for (i=0; i<npix; i++, p1++, p2++)  {
      *p1 *= *p2;
    }
    break;
  case MULT_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++)
      *p1 *=  *p2;
    break;
  case DIV_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p2 == 0){
	if (*p1 != 0){
#if SIGNED
	*p1 = SGN(*p1)*PIX_MAX;
#else
	*p1 = (*p1 & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	*p1 /=  *p2;
    }
    break;
  case INF_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 < *p1)
	*p1 = *p2;
    break;
  case SUP_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 > *p1)
	*p1 = *p2;
    break;
  case MASK_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 != 0)
	*p1 = *p2;
    break;
  case MASK_op2:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p1 == 0)
	*p1 = *p2;
    break;
  case CMP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 < *p2)
	*p1 = 1;
      else if (*p1 > *p2)
	*p1 = 2;
      else
	*p1 = 0;
    }
    break;
  case EQUAL_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 == *p2)
	*p1 = 1;
      else
	*p1 = 0;
    }
    break;
  case NDI_op:
    for (i=0; i<npix; i++, p1++, p2++){
      double dval=(double)*p1+(double)*p2;
      if (dval==0.)
        *p1=PIX_MAX;
      else
        *p1=((double)*p1-(double)*p2)/dval;
    }
    break;
  default:
    (void)sprintf(buf, "ERROR in generic_arith(im1, im2, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in generic_arith(im1, im2, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "u32_undef.h"



#include "u32_def.h"
#define PIX_TYPE2 UCHAR
ERROR_TYPE uluc_arith(IMAGE *im1, IMAGE *im2, int op)
{
  mia_size_t i, ovfl = 0, npix;
#if OVFL_TEST
  long int test;
#endif
  PIX_TYPE *p1;
  PIX_TYPE2 *p2;
  //pk 24/04/2023 prepare for parallel loop in future...
  /* double dval; */

  p1 = (PIX_TYPE *)GetImPtr(im1);
  p2 = (PIX_TYPE2 *)GetImPtr(im2);

  npix = GetImNPix(im1);

  switch(op){
  case AND_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 &= *p2;
    }
    break;
  case OR_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 |= *p2;
    }
    break;
  case XOR_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 ^= *p2;
    }
    break;
  case ADD_op_ovfl: /* allow overflow */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 += *p2;
    }
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p1 + *p2;
    }
    break;
  case SUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p1 - *p2;
    }
    break;
  case SUBSWAP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 =  *p2 - *p1;
    }
    break;
  case SUB_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 -= *p2;
    }
    break;
  case SUBSWAP_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p2 - *p1;
    }
    break;
  case ABSSUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
#if FLOATING
      *p1 = (PIX_TYPE)fabs((double)*p1-(double)*p2);
#else
      *p1 = (PIX_TYPE)abs((int)*p1-(int)*p2);
#endif
    }
    break;
  case MULT_op:
    for (i=0; i<npix; i++, p1++, p2++)  {
      *p1 =  *p1 * *p2;
    }
    break;
  case MULT_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++)
      *p1 *=  *p2;
    break;
  case DIV_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p2 == 0){
	if (*p1 != 0){
#if SIGNED
	*p1 = SGN(*p1)*PIX_MAX;
#else
	*p1 = (*p1 & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	*p1 /=  *p2;
    }
    break;
  case INF_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 < *p1)
	*p1 = *p2;
    break;
  case SUP_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 > *p1)
	*p1 = *p2;
    break;
  case MASK_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 != 0)
	*p1 = *p2;
    break;
  case MASK_op2:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p1 == 0)
	*p1 = *p2;
    break;
  case CMP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 < *p2)
	*p1 = 1;
      else if (*p1 > *p2)
	*p1 = 2;
      else
	*p1 = 0;
    }
    break;
  case EQUAL_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 == *p2)
	*p1 = 1;
      else
	*p1 = 0;
    }
    break;
  case NDI_op:
    for (i=0; i<npix; i++, p1++, p2++){
      double dval=(double)*p1+(double)*p2;
      if (dval==0.)
        *p1=PIX_MAX;
      else
        *p1=((double)*p1-(double)*p2)/dval;
    }
    break;
  default:
    (void)sprintf(buf, "ERROR in generic_arith(im1, im2, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in usuc_arith(im1, im2, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#undef PIX_TYPE2
#include "u32_undef.h"




#include "f_def.h"
ERROR_TYPE f_arith(IMAGE *im1, IMAGE *im2, int op)
{
  mia_size_t i, ovfl = 0, npix;
#if OVFL_TEST
  long int test;
#endif
  PIX_TYPE *p1, *p2;
  //pk 24/04/2023 prepare for parallel loop in future...
  /* double dval; */

  p1 = (PIX_TYPE *)GetImPtr(im1);
  p2 = (PIX_TYPE *)GetImPtr(im2);

  npix = GetImNPix(im1);

  switch(op){
  case ADD_op_ovfl: /* allow overflow */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 += *p2;
    }
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p1 + *p2;
    }
    break;
  case SUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p1 - *p2;
    }
    break;
  case SUBSWAP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p2-*p1;
    }
    break;
  case SUB_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 -= *p2;
    }
    break;
  case SUBSWAP_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p2 - *p1;
    }
    break;
  case ABSSUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
#if FLOATING
      *p1 = (PIX_TYPE)fabs((double)*p1-(double)*p2);
#else
      *p1 = (PIX_TYPE)abs((int)*p1-(int)*p2);
#endif
    }
    break;
  case MULT_op:
    for (i=0; i<npix; i++, p1++, p2++)  {
      *p1 = *p1 * *p2;
    }
    break;
  case MULT_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++)
      *p1 *=  *p2;
    break;
  case DIV_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p2 == (PIX_TYPE)0){
	if (*p1 != 0){
#if SIGNED
	*p1 = SGN(*p1)*PIX_MAX;
#else
	*p1 = (*p1 & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	*p1 /=  *p2;
    }
    break;
  case INF_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 < *p1)
	*p1 = *p2;
    break;
  case SUP_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 > *p1)
	*p1 = *p2;
    break;
  case MASK_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 != (PIX_TYPE)0)
	*p1 = *p2;
    break;
  case MASK_op2:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p1 == (PIX_TYPE)0)
	*p1 = *p2;
    break;
  case CMP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 < *p2)
	*p1 = 1;
      else if (*p1 > *p2)
	*p1 = 2;
      else
	*p1 = 0;
    }
    break;
  case EQUAL_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 == *p2)
	*p1 = 1;
      else
	*p1 = 0;
    }
    break;
  case NDI_op:
    for (i=0; i<npix; i++, p1++, p2++){
      double dval=(double)*p1+(double)*p2;
      if (dval==0.)
        *p1=PIX_MAX;
      else
        *p1=((double)*p1-(double)*p2)/dval;
    }
    break;
#if FLOATING
  case ATAN_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1=atan2f(*p2,*p1);
      if (*p1<0.0)
	*p1+=2*PI;
    }
/*       dval=(double)*p1; */
/*       dval2=(double)*p2; */
/*       if (dval==0.) */
/* 	*p1=0.0; */
/*       else{ */
/* 	*p1=(PIX_TYPE)atan(dval2 / dval); */
/* 	if ( (dval<0.0) && (dval2<0.0) ) */
/* 	  *p1+=PI; */
/* 	else if (dval<0) */
/* 	  *p1+=PI; */
/* 	else if (dval2<0) */
/* 	  *p1+=(2*PI); */
/*       } */
    break;
#endif
  default:
    (void)sprintf(buf, "ERROR in generic_arith(im1, im2, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in generic_arith(im1, im2, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "f_undef.h"

#include "f_def.h"
#define PIX_TYPE2 UCHAR
ERROR_TYPE fuc_arith(IMAGE *im1, IMAGE *im2, int op)
{
  mia_size_t i, ovfl = 0, npix;
#if OVFL_TEST
  long int test;
#endif
  PIX_TYPE *p1;
  PIX_TYPE2 *p2;
  //pk 24/04/2023 prepare for parallel loop in future...
  /* double dval; */

  p1 = (PIX_TYPE *)GetImPtr(im1);
  p2 = (PIX_TYPE2 *)GetImPtr(im2);

  npix = GetImNPix(im1);

  switch(op){ /* no bitwise operators */
  case ADD_op_ovfl: /* allow overflow */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 += *p2;
    }
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 += *p2;
    }
    break;
  case SUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1-=*p2;
    }
    break;
  case SUBSWAP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1= *p2 - *p1;
    }
    break;
  case SUB_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 -= *p2;
    }
    break;
  case SUBSWAP_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p2 - *p1;
    }
    break;
  case ABSSUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
#if FLOATING
      *p1 = (PIX_TYPE)fabs((double)*p1-(double)*p2);
#else
      *p1 = (PIX_TYPE)abs((int)*p1-(int)*p2);
#endif
    }
    break;
  case MULT_op:
    for (i=0; i<npix; i++, p1++, p2++)  {
      *p1 *= *p2;
    }
    break;
  case MULT_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++)
      *p1 *=  *p2;
    break;
  case DIV_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p2 == 0){
	if (*p1 != 0){
#if SIGNED
	*p1 = SGN(*p1)*PIX_MAX;
#else
	*p1 = (*p1 & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	*p1 /=  *p2;
    }
    break;
  case INF_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 < *p1)
	*p1 = *p2;
    break;
  case SUP_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 > *p1)
	*p1 = *p2;
    break;
  case MASK_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 != 0)
	*p1 = *p2;
    break;
  case MASK_op2:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p1 == 0)
	*p1 = *p2;
    break;
  case CMP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 < *p2)
	*p1 = 1;
      else if (*p1 > *p2)
	*p1 = 2;
      else
	*p1 = 0;
    }
    break;
  case EQUAL_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 == *p2)
	*p1 = 1;
      else
	*p1 = 0;
    }
    break;
  case NDI_op:
    for (i=0; i<npix; i++, p1++, p2++){
      double dval=(double)*p1+(double)*p2;
      if (dval==0.)
        *p1=PIX_MAX;
      else
        *p1=((double)*p1-(double)*p2)/dval;
    }
    break;
  default:
    (void)sprintf(buf, "ERROR in generic_arith(im1, im2, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in fuc_arith(im1, im2, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#undef PIX_TYPE2
#include "f_undef.h"

#include "f_def.h"
#define PIX_TYPE2 USHORT
ERROR_TYPE fus_arith(IMAGE *im1, IMAGE *im2, int op)
{
  mia_size_t i, ovfl = 0, npix;
#if OVFL_TEST
  long int test;
#endif
  PIX_TYPE *p1;
  PIX_TYPE2 *p2;
  //pk 24/04/2023 prepare for parallel loop in future...
  /* double dval; */

  p1 = (PIX_TYPE *)GetImPtr(im1);
  p2 = (PIX_TYPE2 *)GetImPtr(im2);

  npix = GetImNPix(im1);

  switch(op){ /* no bitwise operators */
  case ADD_op_ovfl: /* allow overflow */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 += *p2;
    }
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 += *p2;
    }
    break;
  case SUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1-=*p2;
    }
    break;
  case SUBSWAP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1= *p2 - *p1;
    }
    break;
  case SUB_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 -= *p2;
    }
    break;
  case SUBSWAP_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p2 - *p1;
    }
    break;
  case ABSSUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
#if FLOATING
      *p1 = (PIX_TYPE)fabs((double)*p1-(double)*p2);
#else
      *p1 = (PIX_TYPE)abs((int)*p1-(int)*p2);
#endif
    }
    break;
  case MULT_op:
    for (i=0; i<npix; i++, p1++, p2++)  {
      *p1 *= *p2;
    }
    break;
  case MULT_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++)
      *p1 *=  *p2;
    break;
  case DIV_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p2 == 0){
	if (*p1 != 0){
#if SIGNED
	*p1 = SGN(*p1)*PIX_MAX;
#else
	*p1 = (*p1 & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	*p1 /=  *p2;
    }
    break;
  case INF_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 < *p1)
	*p1 = *p2;
    break;
  case SUP_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 > *p1)
	*p1 = *p2;
    break;
  case MASK_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 != 0)
	*p1 = *p2;
    break;
  case MASK_op2:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p1 == 0)
	*p1 = *p2;
    break;
  case CMP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 < *p2)
	*p1 = 1;
      else if (*p1 > *p2)
	*p1 = 2;
      else
	*p1 = 0;
    }
    break;
  case EQUAL_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 == *p2)
	*p1 = 1;
      else
	*p1 = 0;
    }
    break;
  case NDI_op:
    for (i=0; i<npix; i++, p1++, p2++){
      double dval=(double)*p1+(double)*p2;
      if (dval==0.)
        *p1=PIX_MAX;
      else
        *p1=((double)*p1-(double)*p2)/dval;
    }
    break;
  default:
    (void)sprintf(buf, "ERROR in generic_arith(im1, im2, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in fuc_arith(im1, im2, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#undef PIX_TYPE2
#include "f_undef.h"

#include "d_def.h"
ERROR_TYPE d_arith(IMAGE *im1, IMAGE *im2, int op)
{
  mia_size_t i, ovfl = 0, npix;
#if OVFL_TEST
  long int test;
#endif
  PIX_TYPE *p1, *p2;
  //pk 24/04/2023 prepare for parallel loop in future...
  /* double dval; */

  p1 = (PIX_TYPE *)GetImPtr(im1);
  p2 = (PIX_TYPE *)GetImPtr(im2);

  npix = GetImNPix(im1);

  switch(op){
  case ADD_op_ovfl: /* allow overflow */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 += *p2;
    }
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p1 + *p2;
    }
    break;
  case SUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p1 - *p2;
    }
    break;
  case SUBSWAP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1=*p2-*p1;
    }
    break;
  case SUB_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 -= *p2;
    }
    break;
  case SUBSWAP_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++){
      *p1 = *p2 - *p1;
    }
    break;
  case ABSSUB_op:
    for (i=0; i<npix; i++, p1++, p2++){
#if FLOATING
      *p1 = (PIX_TYPE)fabs((double)*p1-(double)*p2);
#else
      *p1 = (PIX_TYPE)abs((int)*p1-(int)*p2);
#endif
    }
    break;
  case MULT_op:
    for (i=0; i<npix; i++, p1++, p2++)  {
      *p1 = *p1 * *p2;
    }
    break;
  case MULT_op_ovfl:
    for (i=0; i<npix; i++, p1++, p2++)
      *p1 *=  *p2;
    break;
  case DIV_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p2 == 0){
	if (*p1 != 0){
#if SIGNED
	*p1 = SGN(*p1)*PIX_MAX;
#else
	*p1 = (*p1 & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	*p1 /=  *p2;
    }
    break;
  case INF_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 < *p1)
	*p1 = *p2;
    break;
  case SUP_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 > *p1)
	*p1 = *p2;
    break;
  case MASK_op:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p2 != 0)
	*p1 = *p2;
    break;
  case MASK_op2:
    for (i=0; i<npix; i++, p1++, p2++)
      if (*p1 == 0)
	*p1 = *p2;
    break;
  case CMP_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 < *p2)
	*p1 = 1;
      else if (*p1 > *p2)
	*p1 = 2;
      else
	*p1 = 0;
    }
    break;
  case EQUAL_op:
    for (i=0; i<npix; i++, p1++, p2++){
      if (*p1 == *p2)
	*p1 = 1;
      else
	*p1 = 0;
    }
    break;
  case NDI_op:
    for (i=0; i<npix; i++, p1++, p2++){
      double dval=(double)*p1+(double)*p2;
      if (dval==0.)
        *p1=PIX_MAX;
      else
        *p1=((double)*p1-(double)*p2)/dval;
    }
    break;
  default:
    (void)sprintf(buf, "ERROR in generic_arith(im1, im2, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in generic_arith(im1, im2, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "d_undef.h"


ERROR_TYPE arith(IMAGE *im1, IMAGE *im2, int op)
{

  /* check for possible errors */
  if (szgeocompat(im1, im2) != NO_ERROR){
    (void)sprintf(buf,"ERROR in arith(im1, im2, op): \
                images of different size\n"); errputstr(buf);
    return(ERROR);
  }

  switch (GetImDataType(im1)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    if (GetImDataType(im1) != GetImDataType(im2)){
      (void)sprintf(buf,"ERROR in arith(im1, im2, op): \
                invalid image data type combination\n"); errputstr(buf);
      return(ERROR);
    }
    return(generic_arith(im1, im2, op));
    break;
#endif

  case t_USHORT:
    if (GetImDataType(im2) == t_UCHAR){
      return(usuc_arith(im1, im2, op));
    }
    else if (GetImDataType(im2) == t_USHORT){
      return(us_arith(im1, im2, op));
    }
    (void)sprintf(buf,"ERROR in arith(im1, im2, op): \
                invalid image data type combination\n"); errputstr(buf);
    return(ERROR);

    break;

  case t_SHORT: /* overflow check is turned off ! */
    if (GetImDataType(im2) == t_UCHAR){
      return(suc_arith(im1, im2, op));
    }
    else if (GetImDataType(im2) == t_SHORT){
      return(s_arith(im1, im2, op));
    }
    (void)sprintf(buf,"ERROR in arith(im1, im2, op): \
                invalid image data type combination\n"); errputstr(buf);
    return(ERROR);
    break;


  case t_INT32: /* overflow check is turned off ! */
    if (GetImDataType(im2) == t_UCHAR){
      return(luc_arith(im1, im2, op));
    }
    if (GetImDataType(im2) == t_USHORT){
      return(lus_arith(im1, im2, op));
    }
    else if (GetImDataType(im1) != GetImDataType(im2)){
      (void)sprintf(buf,"ERROR in arith(im1, im2, op): \
                invalid image data type combination\n"); errputstr(buf);
      return(ERROR);
    }
    return(i32_arith(im1, im2, op));
    break;

  case t_UINT32: /* overflow check is turned off ! */
    if (GetImDataType(im2) == t_UCHAR){
      return(uluc_arith(im1, im2, op));
    }
    else if (GetImDataType(im1) != GetImDataType(im2)){
      (void)sprintf(buf,"ERROR in arith(im1, im2, op): \
                invalid image data type combination\n"); errputstr(buf);
      return(ERROR);
    }
    return(u32_arith(im1, im2, op));
    break;

  case t_FLOAT: /* overflow check is turned off ! */
    if (GetImDataType(im2) == t_FLOAT){
      return(f_arith(im1, im2, op));
    }
    else if (GetImDataType(im2) == t_UCHAR){
      return(fuc_arith(im1, im2, op));
    }
    (void)sprintf(buf,"ERROR in arith(im1, im2, op): \
                invalid image data type combination\n"); errputstr(buf);
      return(ERROR);
    break;
  case t_DOUBLE: /* overflow check is turned off ! */
    if (GetImDataType(im1) != GetImDataType(im2)){
      (void)sprintf(buf,"ERROR in arith(im1, im2, op): \
                invalid image data type combination\n"); errputstr(buf);
      return(ERROR);
    }
    return(d_arith(im1, im2, op));
    break;

  default:
    (void)sprintf(buf,"arith(im1, im2, op): invalid pixel type\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}
/*************************************************************************/
/*                     im1[i] = im1[i] arith_op cst                      */

#ifndef NO_generic_IMAGE
#include "g_def.h"
ERROR_TYPE generic_arithcst(IMAGE *im1, PIX_TYPE cst, int op)
{
  mia_size_t i, ovfl = 0, npix;
  int k;
#if OVFL_TEST
  long int test;
#endif
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im1);

  npix = GetImNPix(im1);

  switch(op){
  case AND_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++){
      p1[i] &= cst;
    }
    break;
#if (SIGNED==0)
  case NAND_op:
    cst=~cst;
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++){
      p1[i] &= cst;
    }
    break;
#endif
  case OR_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++){
      p1[i] |= cst;
    }
    break;
  case XOR_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++){
      p1[i] ^= cst;
    }
    break;
  case ADD_op_ovfl: /* allow overflow */
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++){
      p1[i] += cst;
    }
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
#ifdef OPENMP
#pragma omp parallel for private(test) reduction(+:ovfl)
#endif
    for (i=0; i<npix; i++){
#if OVFL_TEST
      test = p1[i] + cst;
      if (test > PIX_MAX){
	ovfl++;
	test = PIX_MAX;
      }
      p1[i] = (PIX_TYPE)test;
#else
      p1[i] += cst
#endif
    }
    break;
  case SUB_op:
#ifdef OPENMP
#pragma omp parallel for private(test) reduction(+:ovfl)
#endif
    for (i=0; i<npix; i++){
#if OVFL_TEST
      test = p1[i] - cst;
      if (test < PIX_MIN){
	ovfl++;
	test = PIX_MIN;
      }
      p1[i] = (PIX_TYPE)test;
#else
      p1[i] -= cst
#endif
    }
    break;
  case SUB_op_ovfl:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++){
      p1[i] -= cst;
    }
    break;
  case ABSSUB_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++){
#if FLOATING
      p1[i] = (PIX_TYPE)fabs((double)p1[i]-cst);
#else
      p1[i] = (PIX_TYPE)abs((int)p1[i]-(int)cst);
#endif
    }
    break;
  case MULT_op:
#ifdef OPENMP
#pragma omp parallel for private(test) reduction(+:ovfl)
#endif
    for (i=0; i<npix; i++)  {
#if OVFL_TEST
      test = p1[i] * cst;
      if (test > PIX_MAX){
	ovfl++;
	test = PIX_MAX;
      }
      p1[i] = (PIX_TYPE)test;
#else
      p1[i] *= cst
#endif
    }
    break;
  case MULT_op_ovfl:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++)
      p1[i] *= cst;
    break;
  case DIV_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++){
      if (cst == 0){
	if (p1[i] != 0){
#if SIGNED
	p1[i] = SGN(p1[i])*PIX_MAX;
#else
	p1[i] = (p1[i] & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	p1[i] /= cst;
    }
    break;
  case INF_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++)
      if (cst < p1[i])
	p1[i] = cst;
    break;
  case SUP_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++)
      if (cst > p1[i])
	p1[i] = cst;
    break;
  case MASK_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++)
      if (cst != 0)
	p1[i] = cst;
    break;
  case MASK_op2:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++)
      if (p1[i] == 0)
	p1[i] = cst;
    break;
  case CMP_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++)
      if (p1[i] < cst)
	p1[i] = 1;
      else if (p1[i] > cst)
	p1[i] = 2;
      else
	p1[i] = 0;
    break;
#if SIGNED
  case SUBSWAPCST_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++)
      p1[i]=cst-p1[i];
    break;
#endif
#if (SIGNED==0)
  case FirstBitOn_op:
#ifdef OPENMP
#pragma omp parallel for
#endif
    for (i=0; i<npix; i++){
      for (k=0;k<BitPerPixel;k++){
	if (p1[i] & 1<<k){
	  p1[i]= 1<<k;
	  break;
	}
      }
    }
    break;
#endif
  default:
    (void)sprintf(buf, "ERROR in generic_arithcst(im1, cst, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in generic_arithcst(im1, cst, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */

#include "us_def.h"
ERROR_TYPE us_arithcst(IMAGE *im1, PIX_TYPE cst, int op)
{
  mia_size_t i, ovfl = 0, npix;
  int k;
#if OVFL_TEST
  long int test;
#endif
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im1);

  npix = GetImNPix(im1);

  switch(op){
  case AND_op:
    for (i=0; i<npix; i++, p1++){
      *p1 &= cst;
    }
    break;
#if (SIGNED==0)
  case NAND_op:
    cst=~cst;
    for (i=0; i<npix; i++, p1++){
      *p1 &= cst;
    }
    break;
#endif
  case OR_op:
    for (i=0; i<npix; i++, p1++){
      *p1 |= cst;
    }
    break;
  case XOR_op:
    for (i=0; i<npix; i++, p1++){
      *p1 ^= cst;
    }
    break;
  case ADD_op_ovfl: /* allow overflow */
    for (i=0; i<npix; i++, p1++){
      *p1 += cst;
    }
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
    for (i=0; i<npix; i++, p1++){
      test = *p1 + cst;
      if (test > PIX_MAX){
	ovfl++;
	test = PIX_MAX;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case SUB_op:
    for (i=0; i<npix; i++, p1++){
      test = *p1 - cst;
      if (test < PIX_MIN){
	ovfl++;
	test = PIX_MIN;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case SUB_op_ovfl:
    for (i=0; i<npix; i++, p1++){
      *p1 -= cst;
    }
    break;
  case ABSSUB_op:
    for (i=0; i<npix; i++, p1++){
#if FLOATING
      *p1 = (PIX_TYPE)fabs((double)*p1-cst);
#else
      *p1 = (PIX_TYPE)abs((int)*p1-(int)cst);
#endif
    }
    break;
  case MULT_op:
    for (i=0; i<npix; i++, p1++)  {
      test = *p1 * cst;
      if (test > PIX_MAX){
	ovfl++;
	test = PIX_MAX;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case MULT_op_ovfl:
    for (i=0; i<npix; i++, p1++)
      *p1 *= cst;
    break;
  case DIV_op:
    for (i=0; i<npix; i++, p1++){
      if (cst == 0){
	if (*p1 != 0){
#if SIGNED
	*p1 = SGN(*p1)*PIX_MAX;
#else
	*p1 = (*p1 & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	*p1 /= cst;
    }
    break;
  case INF_op:
    for (i=0; i<npix; i++, p1++)
      if (cst < *p1)
	*p1 = cst;
    break;
  case SUP_op:
    for (i=0; i<npix; i++, p1++)
      if (cst > *p1)
	*p1 = cst;
    break;
  case MASK_op:
    for (i=0; i<npix; i++, p1++)
      if (cst != 0)
	*p1 = cst;
    break;
  case MASK_op2:
    for (i=0; i<npix; i++, p1++)
      if (*p1 == 0)
	*p1 = cst;
    break;
  case CMP_op:
    for (i=0; i<npix; i++, p1++)
      if (*p1 < cst)
	*p1 = 1;
      else if (*p1 > cst)
	*p1 = 2;
      else
	*p1 = 0;
    break;
#if SIGNED
  case SUBSWAPCST_op:
    for (i=0; i<npix; i++, p1++)
      *p1=cst-*p1;
    break;
#endif
#if (SIGNED==0)
  case FirstBitOn_op:
    for (i=0; i<npix; i++, p1++){
      for (k=0;k<BitPerPixel;k++){
	if (*p1 & 1<<k){
	  *p1= 1<<k;
	  break;
	}
      }
    }
    break;
#endif
  default:
    (void)sprintf(buf, "ERROR in us_arithcst(im1, cst, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in us_arithcst(im1, cst, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "us_undef.h"



#include "s_def.h"
ERROR_TYPE s_arithcst(IMAGE *im1, PIX_TYPE cst, int op)
{
  mia_size_t i, ovfl = 0, npix;
#if OVFL_TEST
  long int test;
#endif
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im1);

  npix = GetImNPix(im1);

  switch(op){
  case AND_op:
    for (i=0; i<npix; i++, p1++){
      *p1 &= cst;
    }
    break;
  case OR_op:
    for (i=0; i<npix; i++, p1++){
      *p1 |= cst;
    }
    break;
  case XOR_op:
    for (i=0; i<npix; i++, p1++){
      *p1 ^= cst;
    }
    break;
  case ADD_op_ovfl: /* allow overflow */
    for (i=0; i<npix; i++, p1++){
      *p1 += cst;
    }
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
    for (i=0; i<npix; i++, p1++){
      test = *p1 + cst;
      if (test > PIX_MAX){
	ovfl++;
	test = PIX_MAX;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case SUB_op:
    for (i=0; i<npix; i++, p1++){
      test = *p1 - cst;
      if (test < PIX_MIN){
	ovfl++;
	test = PIX_MIN;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case SUB_op_ovfl:
    for (i=0; i<npix; i++, p1++){
      *p1 -= cst;
    }
    break;
  case ABSSUB_op:
    for (i=0; i<npix; i++, p1++){
#if FLOATING
      *p1 = (PIX_TYPE)fabs((double)*p1-cst);
#else
      *p1 = (PIX_TYPE)abs((int)*p1-(int)cst);
#endif
    }
    break;
  case MULT_op:
    for (i=0; i<npix; i++, p1++)  {
      test = *p1 * cst;
      if (test > PIX_MAX){
	ovfl++;
	test = PIX_MAX;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case MULT_op_ovfl:
    for (i=0; i<npix; i++, p1++)
      *p1 *= cst;
    break;
  case DIV_op:
    for (i=0; i<npix; i++, p1++){
      if (cst == 0){
	if (*p1 != 0){
#if SIGNED
	*p1 = SGN(*p1)*PIX_MAX;
#else
	*p1 = (*p1 & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	*p1 /= cst;
    }
    break;
  case INF_op:
    for (i=0; i<npix; i++, p1++)
      if (cst < *p1)
	*p1 = cst;
    break;
  case SUP_op:
    for (i=0; i<npix; i++, p1++)
      if (cst > *p1)
	*p1 = cst;
    break;
  case MASK_op:
    for (i=0; i<npix; i++, p1++)
      if (cst != 0)
	*p1 = cst;
    break;
  case MASK_op2:
    for (i=0; i<npix; i++, p1++)
      if (*p1 == 0)
	*p1 = cst;
    break;
  case CMP_op:
    for (i=0; i<npix; i++, p1++)
      if (*p1 < cst)
	*p1 = 1;
      else if (*p1 > cst)
	*p1 = 2;
      else
	*p1 = 0;
    break;
#if SIGNED
  case SUBSWAPCST_op:
    for (i=0; i<npix; i++, p1++)
      *p1=cst-*p1;
    break;
#endif
  default:
    (void)sprintf(buf, "ERROR in s_arithcst(im1, cst, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in s_arithcst(im1, cst, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "s_undef.h"


#include "i32_def.h"
ERROR_TYPE i32_arithcst(IMAGE *im1, PIX_TYPE cst, int op)
{
  mia_size_t i, ovfl = 0, npix;
#if OVFL_TEST
  long int test;
#endif
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im1);

  npix = GetImNPix(im1);

  switch(op){
  case AND_op:
    for (i=0; i<npix; i++, p1++){
      *p1 &= cst;
    }
    break;
  case OR_op:
    for (i=0; i<npix; i++, p1++){
      *p1 |= cst;
    }
    break;
  case XOR_op:
    for (i=0; i<npix; i++, p1++){
      *p1 ^= cst;
    }
    break;
  case ADD_op_ovfl: /* allow overflow */
    for (i=0; i<npix; i++, p1++){
      *p1 += cst;
    }
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
    for (i=0; i<npix; i++, p1++){
      *p1 += cst;
    }
    break;
  case SUB_op:
    for (i=0; i<npix; i++, p1++){
      *p1 -= cst;
    }
    break;
  case SUB_op_ovfl:
    for (i=0; i<npix; i++, p1++){
      *p1 -= cst;
    }
    break;
  case ABSSUB_op:
    for (i=0; i<npix; i++, p1++){
#if FLOATING
      *p1 = (PIX_TYPE)fabs((double)*p1-cst);
#else
      *p1 = (PIX_TYPE)abs((int)*p1-(int)cst);
#endif
    }
    break;
  case MULT_op:
    for (i=0; i<npix; i++, p1++)  {
      *p1 *= cst;
    }
    break;
  case MULT_op_ovfl:
    for (i=0; i<npix; i++, p1++)
      *p1 *= cst;
    break;
  case DIV_op:
    for (i=0; i<npix; i++, p1++){
      if (cst == 0){
	if (*p1 != 0){
#if SIGNED
	*p1 = SGN(*p1)*PIX_MAX;
#else
	*p1 = (*p1 & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	*p1 /= cst;
    }
    break;
  case INF_op:
    for (i=0; i<npix; i++, p1++)
      if (cst < *p1)
	*p1 = cst;
    break;
  case SUP_op:
    for (i=0; i<npix; i++, p1++)
      if (cst > *p1)
	*p1 = cst;
    break;
  case MASK_op:
    for (i=0; i<npix; i++, p1++)
      if (cst != 0)
	*p1 = cst;
    break;
  case MASK_op2:
    for (i=0; i<npix; i++, p1++)
      if (*p1 == 0)
	*p1 = cst;
    break;
  case CMP_op:
    for (i=0; i<npix; i++, p1++)
      if (*p1 < cst)
	*p1 = 1;
      else if (*p1 > cst)
	*p1 = 2;
      else
	*p1 = 0;
    break;
#if SIGNED
  case SUBSWAPCST_op:
    for (i=0; i<npix; i++, p1++)
      *p1=cst-*p1;
    break;
#endif
  default:
    (void)sprintf(buf, "ERROR in i32_arithcst(im1, cst, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in i32_arithcst(im1, cst, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "i32_undef.h"

#include "u32_def.h"
ERROR_TYPE u32_arithcst(IMAGE *im1, PIX_TYPE cst, int op)
{
  mia_size_t i, ovfl = 0, npix;
  int k;
  //#if OVFL_TEST
  long int test;
  //#endif
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im1);

  npix = GetImNPix(im1);

  switch(op){
  case AND_op:
    for (i=0; i<npix; i++, p1++){
      *p1 &= cst;
    }
    break;
#if (SIGNED==0)
  case NAND_op:
    cst=~cst;
    for (i=0; i<npix; i++, p1++){
      *p1 &= cst;
    }
    break;
#endif
  case OR_op:
    for (i=0; i<npix; i++, p1++){
      *p1 |= cst;
    }
    break;
  case XOR_op:
    for (i=0; i<npix; i++, p1++){
      *p1 ^= cst;
    }
    break;
  case ADD_op_ovfl: /* allow overflow */
    for (i=0; i<npix; i++, p1++){
      *p1 += cst;
    }
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
    for (i=0; i<npix; i++, p1++){
      test = *p1 + cst;
      if (test > PIX_MAX){
	ovfl++;
	test = PIX_MAX;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case SUB_op:
    for (i=0; i<npix; i++, p1++){
      test = *p1 - cst;
      if (test < PIX_MIN){
	ovfl++;
	test = PIX_MIN;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case SUB_op_ovfl:
    for (i=0; i<npix; i++, p1++){
      *p1 -= cst;
    }
    break;
  case ABSSUB_op:
    for (i=0; i<npix; i++, p1++){
#if FLOATING
      *p1 = (PIX_TYPE)fabs((double)*p1-cst);
#else
      *p1 = (PIX_TYPE)abs((int)*p1-(int)cst);
#endif
    }
    break;
  case MULT_op:
    for (i=0; i<npix; i++, p1++)  {
      test = *p1 * cst;
      if (test > PIX_MAX){
	ovfl++;
	test = PIX_MAX;
      }
      *p1 = (PIX_TYPE)test;
    }
    break;
  case MULT_op_ovfl:
    for (i=0; i<npix; i++, p1++)
      *p1 *= cst;
    break;
  case DIV_op:
    for (i=0; i<npix; i++, p1++){
      if (cst == 0){
	if (*p1 != 0){
#if SIGNED
	*p1 = SGN(*p1)*PIX_MAX;
#else
	*p1 = (*p1 & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	*p1 /= cst;
    }
    break;
  case INF_op:
    for (i=0; i<npix; i++, p1++)
      if (cst < *p1)
	*p1 = cst;
    break;
  case SUP_op:
    for (i=0; i<npix; i++, p1++)
      if (cst > *p1)
	*p1 = cst;
    break;
  case MASK_op:
    for (i=0; i<npix; i++, p1++)
      if (cst != 0)
	*p1 = cst;
    break;
  case MASK_op2:
    for (i=0; i<npix; i++, p1++)
      if (*p1 == 0)
	*p1 = cst;
    break;
  case CMP_op:
    for (i=0; i<npix; i++, p1++)
      if (*p1 < cst)
	*p1 = 1;
      else if (*p1 > cst)
	*p1 = 2;
      else
	*p1 = 0;
    break;
#if SIGNED
  case SUBSWAPCST_op:
    for (i=0; i<npix; i++, p1++)
      *p1=cst-*p1;
    break;
#endif
#if (SIGNED==0)
  case FirstBitOn_op:
    for (i=0; i<npix; i++, p1++){
      for (k=0;k<BitPerPixel;k++){
	if (*p1 & 1<<k){
	  *p1= 1<<k;
	  break;
	}
      }
    }
    break;
#endif
  default:
    (void)sprintf(buf, "ERROR in generic_arithcst(im1, cst, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in generic_arithcst(im1, cst, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "u32_undef.h"

#include "f_def.h"
ERROR_TYPE f_arithcst(IMAGE *im1, PIX_TYPE cst, int op)
{
  mia_size_t i, ovfl = 0, npix;
#if OVFL_TEST
  long int test;
#endif
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im1);

  npix = GetImNPix(im1);

  switch(op){
  case ADD_op_ovfl: /* allow overflow */
    for (i=0; i<npix; i++, p1++){
      *p1 += cst;
    }
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
    for (i=0; i<npix; i++, p1++){
      *p1 += cst;
    }
    break;
  case SUB_op:
    for (i=0; i<npix; i++, p1++){
      *p1 -= cst;
    }
    break;
  case SUB_op_ovfl:
    for (i=0; i<npix; i++, p1++){
      *p1 -= cst;
    }
    break;
  case ABSSUB_op:
    for (i=0; i<npix; i++, p1++){
#if FLOATING
      *p1 = (PIX_TYPE)fabs((double)*p1-cst);
#else
      *p1 = (PIX_TYPE)abs((int)*p1-(int)cst);
#endif
    }
    break;
  case MULT_op:
    for (i=0; i<npix; i++, p1++)  {
      *p1 *= cst;
    }
    break;
  case MULT_op_ovfl:
    for (i=0; i<npix; i++, p1++)
      *p1 *= cst;
    break;
  case DIV_op:
    for (i=0; i<npix; i++, p1++){
      if (cst == (PIX_TYPE)0){
	if (*p1 != 0){
#if SIGNED
	*p1 = SGN(*p1)*PIX_MAX;
#else
	*p1 = (*p1 & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	*p1 /= cst;
    }
    break;
  case INF_op:
    for (i=0; i<npix; i++, p1++)
      if (cst < *p1)
	*p1 = cst;
    break;
  case SUP_op:
    for (i=0; i<npix; i++, p1++)
      if (cst > *p1)
	*p1 = cst;
    break;
  case MASK_op:
    for (i=0; i<npix; i++, p1++)
      if (cst != (PIX_TYPE)0)
	*p1 = cst;
    break;
  case MASK_op2:
    for (i=0; i<npix; i++, p1++)
      if (*p1 == (PIX_TYPE)0)
	*p1 = cst;
    break;
  case CMP_op:
    for (i=0; i<npix; i++, p1++)
      if (*p1 < cst)
	*p1 = 1;
      else if (*p1 > cst)
	*p1 = 2;
      else
	*p1 = 0;
    break;
  case POW_op:
    for (i=0; i<npix; i++, p1++)
	*p1 = powf(*p1,cst);
    break;
#if SIGNED
  case SUBSWAPCST_op:
    for (i=0; i<npix; i++, p1++)
      *p1=cst-*p1;
    break;
#endif
  default:
    (void)sprintf(buf, "ERROR in f_arithcst(im1, cst, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in f_arithcst(im1, cst, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "f_undef.h"

#include "d_def.h"
ERROR_TYPE d_arithcst(IMAGE *im1, PIX_TYPE cst, int op)
{
  mia_size_t i, ovfl = 0, npix;
#if OVFL_TEST
  long int test;
#endif
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im1);

  npix = GetImNPix(im1);

  switch(op){
#if (FLOATING==0)
  case AND_op:
    for (i=0; i<npix; i++, p1++){
      *p1 &= cst;
    }
    break;
#if (SIGNED==0)
  case NAND_op:
    cst=~cst;
    for (i=0; i<npix; i++, p1++){
      *p1 &= cst;
    }
    break;
#endif
  case OR_op:
    for (i=0; i<npix; i++, p1++){
      *p1 |= cst;
    }
    break;
  case XOR_op:
    for (i=0; i<npix; i++, p1++){
      *p1 ^= cst;
    }
    break;
#endif
  case ADD_op_ovfl: /* allow overflow */
    for (i=0; i<npix; i++, p1++){
      *p1 += cst;
    }
    break;
  case ADD_op: /* set to PIX_MAX if overflow occurs */
    for (i=0; i<npix; i++, p1++){
#if OVFL_TEST
      test = *p1 + cst;
      if (test > PIX_MAX){
	ovfl++;
	test = PIX_MAX;
      }
      *p1 = (PIX_TYPE)test;
#else
      *p1 += cst;
#endif
    }
    break;
  case SUB_op:
    for (i=0; i<npix; i++, p1++){
#if OVFL_TEST
      test = *p1 - cst;
      if (test < PIX_MIN){
	ovfl++;
	test = PIX_MIN;
      }
      *p1 = (PIX_TYPE)test;
#else
      *p1 -= cst;
#endif
    }
    break;
  case SUB_op_ovfl:
    for (i=0; i<npix; i++, p1++){
      *p1 -= cst;
    }
    break;
  case ABSSUB_op:
    for (i=0; i<npix; i++, p1++){
#if FLOATING
      *p1 = (PIX_TYPE)fabs((double)*p1-cst);
#else
      *p1 = (PIX_TYPE)abs((int)*p1-(int)cst);
#endif
    }
    break;
  case MULT_op:
    for (i=0; i<npix; i++, p1++)  {
#if OVFL_TEST
      test = *p1 * cst;
      if (test > PIX_MAX){
	ovfl++;
	test = PIX_MAX;
      }
      *p1 = (PIX_TYPE)test;
#else
      *p1 *= cst ;
#endif
    }
    break;
  case MULT_op_ovfl:
    for (i=0; i<npix; i++, p1++)
      *p1 *= cst;
    break;
  case DIV_op:
    for (i=0; i<npix; i++, p1++){
      if (cst == 0){
	if (*p1 != 0){
#if SIGNED
	*p1 = SGN(*p1)*PIX_MAX;
#else
	*p1 = (*p1 & 0x1)*PIX_MAX;
#endif
	ovfl++;
	}
      }
      else
	*p1 /= cst;
    }
    break;
  case INF_op:
    for (i=0; i<npix; i++, p1++)
      if (cst < *p1)
	*p1 = cst;
    break;
  case SUP_op:
    for (i=0; i<npix; i++, p1++)
      if (cst > *p1)
	*p1 = cst;
    break;
  case MASK_op:
    for (i=0; i<npix; i++, p1++)
      if (cst != 0)
	*p1 = cst;
    break;
  case MASK_op2:
    for (i=0; i<npix; i++, p1++)
      if (*p1 == 0)
	*p1 = cst;
    break;
  case CMP_op:
    for (i=0; i<npix; i++, p1++)
      if (*p1 < cst)
	*p1 = 1;
      else if (*p1 > cst)
	*p1 = 2;
      else
	*p1 = 0;
    break;
#if SIGNED
  case SUBSWAPCST_op:
    for (i=0; i<npix; i++, p1++)
      *p1=cst-*p1;
    break;
#endif
#if (SIGNED==0)
  case FirstBitOn_op:
    for (i=0; i<npix; i++, p1++){
      for (k=0;k<BitPerPixel;k++){
	if (*p1 & 1<<k){
	  *p1= 1<<k;
	  break;
	}
      }
    }
    break;
#endif
  default:
    (void)sprintf(buf, "ERROR in generic_arithcst(im1, cst, op): \
                invalid op value\n"); errputstr(buf);
    return(ERROR);
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in generic_arithcst(im1, cst, op=%d): \
             %ld over- and underflow(s) or division(s) by 0\n", op, ovfl); stdputstr(buf);
  }
  return(NO_ERROR);
}
#include "d_undef.h"

ERROR_TYPE arithcst(IMAGE *im, G_TYPE gt, int op)
{

  switch (GetImDataType(im)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_arithcst(im, gt.generic_val, op));
    break;
#endif

#ifndef NO_uc_IMAGE
  case t_UCHAR:
    return(uc_arithcst(im, gt.uc_val, op));
    break;
#endif

  case t_USHORT:
    return(us_arithcst(im, gt.us_val, op));
    break;

  case t_SHORT:
    return(s_arithcst(im, gt.s_val, op));
    break;

  case t_UINT32:
    return(u32_arithcst(im, gt.u32_val, op));
    break;

  case t_INT32:
    return(i32_arithcst(im, gt.i32_val, op));
    break;

  case t_FLOAT:
    return(f_arithcst(im, gt.f_val, op));
    break;

  case t_DOUBLE:
    return(d_arithcst(im, gt.d_val, op));
    break;

  default:
    (void)sprintf(buf,"arithcst(im, gt, op): invalid pixel type\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}


/************************************************************************/
/*                         im1[i] = abs(im1[i])                         */

#ifndef NO_generic_IMAGE
#include "g_def.h"
#if FLOATING
#include <math.h>
#endif
ERROR_TYPE generic_imabs(IMAGE *im)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

  for (i=0; i<npix; i++, p1++){
#if FLOATING
    *p1 = (PIX_TYPE)fabs((double)*p1);
#else
    *p1 = (PIX_TYPE)abs((int)*p1);
#endif
  }
  return(NO_ERROR);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */


#include "s_def.h"
ERROR_TYPE s_imabs(IMAGE *im)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

  for (i=0; i<npix; i++, p1++){
#if FLOATING
    *p1 = (PIX_TYPE)fabs((double)*p1);
#else
    *p1 = (PIX_TYPE)abs((int)*p1);
#endif
  }
  return(NO_ERROR);
}
#include "s_undef.h"

#include "i32_def.h"
#if FLOATING
#include <math.h>
#endif
ERROR_TYPE i32_imabs(IMAGE *im)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

  for (i=0; i<npix; i++, p1++){
#if FLOATING
    *p1 = (PIX_TYPE)fabs((double)*p1);
#else
    *p1 = (PIX_TYPE)abs((int)*p1);
#endif
  }
  return(NO_ERROR);
}
#include "i32_undef.h"


#include "f_def.h"
#if FLOATING
#include <math.h>
#endif
ERROR_TYPE f_imabs(IMAGE *im)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

  for (i=0; i<npix; i++, p1++){
#if FLOATING
    *p1 = (PIX_TYPE)fabs((double)*p1);
#else
    *p1 = (PIX_TYPE)abs((int)*p1);
#endif
  }
  return(NO_ERROR);
}
#include "f_undef.h"


#include "d_def.h"
#if FLOATING
#include <math.h>
#endif
ERROR_TYPE d_imabs(IMAGE *im)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

  for (i=0; i<npix; i++, p1++){
#if FLOATING
    *p1 = (PIX_TYPE)fabs((double)*p1);
#else
    *p1 = (PIX_TYPE)abs((int)*p1);
#endif
  }
  return(NO_ERROR);
}
#include "d_undef.h"


ERROR_TYPE imabs(IMAGE *im)
{
  switch (GetImDataType(im)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_imabs(im));
    break;
#endif

#ifndef NO_uc_IMAGE
  case t_UCHAR:
    return(NO_ERROR);
    break;
#endif

#ifndef NO_us_IMAGE
  case t_USHORT:
    return(NO_ERROR);
    break;
#endif

  case t_SHORT:
    return(s_imabs(im));
    break;

#ifndef NO_u32_IMAGE
  case t_UINT32:
    return(NO_ERROR);
    break;
#endif

  case t_INT32:
    return(i32_imabs(im));
    break;

  case t_FLOAT:
    return(f_imabs(im));
    break;

  case t_DOUBLE:
    return(d_imabs(im));
    break;

  default:
    (void)sprintf(buf,"ERROR imabs(im): invalid pixel type\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}


/*************************************************************************/

#include "s_def.h"
ERROR_TYPE s_imsqrt(IMAGE *im)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;
  int flag=0;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix;i++){
#if SIGNED
    if (p1[i]<0){
      flag=1;
      p1[i]=-1;
    }
    else
      p1[i] = (PIX_TYPE)sqrt((double)p1[i]);
#else
    p1[i] = (PIX_TYPE)sqrt((double)p1[i]);
#endif
  }

  if(flag){
    (void)sprintf(buf,"sqrt(im): WARNING negative input value (output set to -1)\n");
    errputstr(buf);
  }
  return(NO_ERROR);
}
#include "s_undef.h"

#include "us_def.h"
ERROR_TYPE us_imsqrt(IMAGE *im)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;
  int flag=0;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++){
#if SIGNED
    if (p1[i]<0){
      flag=1;
      p1[i]=-1;
    }
    else
      p1[i] = (PIX_TYPE)sqrt((double)p1[i]);
#else
    p1[i] = (PIX_TYPE)sqrt((double)p1[i]);
#endif
  }

  if(flag){
    (void)sprintf(buf,"sqrt(im): WARNING negative input value (output set to -1)\n");
    errputstr(buf);
  }
  return(NO_ERROR);
}
#include "us_undef.h"

#include "i32_def.h"
ERROR_TYPE i32_imsqrt(IMAGE *im)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;
  int flag=0;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix;i++){
#if SIGNED
    if (p1[i]<0){
      flag=1;
      p1[i]=-1;
    }
    else
      p1[i] = (PIX_TYPE)sqrt((double)p1[i]);
#else
    p1[i] = (PIX_TYPE)sqrt((double)p1[i]);
#endif
  }

  if(flag){
    (void)sprintf(buf,"sqrt(im): WARNING negative input value (output set to -1)\n");
    errputstr(buf);
  }
  return(NO_ERROR);
}
#include "i32_undef.h"


#include "u32_def.h"
ERROR_TYPE u32_imsqrt(IMAGE *im)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;
  int flag=0;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix;i++){
#if SIGNED
    if (p1[i]<0){
      flag=1;
      p1[i]=-1;
    }
    else
      p1[i] = (PIX_TYPE)sqrt((double)p1[i]);
#else
    p1[i] = (PIX_TYPE)sqrt((double)p1[i]);
#endif
  }

  if(flag){
    (void)sprintf(buf,"sqrt(im): WARNING negative input value (output set to -1)\n");
    errputstr(buf);
  }
  return(NO_ERROR);
}
#include "u32_undef.h"

#include "f_def.h"
ERROR_TYPE f_imsqrt(IMAGE *im)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;
  int flag=0;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix;i++){
#if SIGNED
    if (p1[i]<0){
      flag=1;
      p1[i]=-1;
    }
    else
      p1[i] = (PIX_TYPE)sqrt((double)p1[i]);
#else
    p1[i] = (PIX_TYPE)sqrt((double)p1[i]);
#endif
  }

  if(flag){
    (void)sprintf(buf,"sqrt(im): WARNING negative input value (output set to -1)\n");
    errputstr(buf);
  }
  return(NO_ERROR);
}
#include "f_undef.h"


ERROR_TYPE imsqrt(IMAGE *im)
{
  switch (GetImDataType(im)){

  case t_SHORT:
    return(s_imsqrt(im));
    break;

  case t_USHORT:
    return(us_imsqrt(im));
    break;

  case t_INT32:
    return(i32_imsqrt(im));
    break;

  case t_UINT32:
    return(u32_imsqrt(im));
    break;

  case t_FLOAT:
    return(f_imsqrt(im));
    break;

  default:
    (void)sprintf(buf,"ERROR imsqrt(im): invalid pixel type\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}

#include "f_def.h"
ERROR_TYPE f_imlog(IMAGE *im)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix;i++){
      p1[i] = (PIX_TYPE)logf(p1[i]);
  }
  return(NO_ERROR);
}
#include "f_undef.h"

ERROR_TYPE imlog(IMAGE *im)
{
  switch (GetImDataType(im)){

  case t_FLOAT:
    return(f_imlog(im));
    break;

  default:
    (void)sprintf(buf,"ERROR imlog(im): invalid pixel type (must be FLOAT)\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}


#include "f_def.h"
ERROR_TYPE f_imatan(IMAGE *im)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix;i++){
      p1[i] = (PIX_TYPE)atanf(p1[i]);
  }
  return(NO_ERROR);
}
#include "f_undef.h"


ERROR_TYPE imatan(IMAGE *im)
{
  switch (GetImDataType(im)){

  case t_FLOAT:
    return(f_imatan(im));
    break;

  default:
    (void)sprintf(buf,"ERROR imatan(im): invalid pixel type (must be FLOAT)\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}



#include "f_def.h"
ERROR_TYPE f_imcos(IMAGE *im)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix;i++){
      p1[i] = (PIX_TYPE)cosf(p1[i]);
  }
  return(NO_ERROR);
}
#include "f_undef.h"


ERROR_TYPE imcos(IMAGE *im)
{
  switch (GetImDataType(im)){

  case t_FLOAT:
    return(f_imcos(im));
    break;

  default:
    (void)sprintf(buf,"ERROR imcos(im): invalid pixel type (must be FLOAT)\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}


#include "f_def.h"
ERROR_TYPE f_imacos(IMAGE *im)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix;i++){
      p1[i] = (PIX_TYPE)acosf(p1[i]);
  }
  return(NO_ERROR);
}
#include "f_undef.h"


ERROR_TYPE imacos(IMAGE *im)
{
  switch (GetImDataType(im)){

  case t_FLOAT:
    return(f_imacos(im));
    break;

  default:
    (void)sprintf(buf,"ERROR imacos(im): invalid pixel type (must be FLOAT)\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}

#include "f_def.h"
ERROR_TYPE f_imsin(IMAGE *im)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix;i++){
      p1[i] = (PIX_TYPE)sinf(p1[i]);
  }
  return(NO_ERROR);
}
#include "f_undef.h"


ERROR_TYPE imsin(IMAGE *im)
{
  switch (GetImDataType(im)){

  case t_FLOAT:
    return(f_imsin(im));
    break;

  default:
    (void)sprintf(buf,"ERROR imsin(im): invalid pixel type (must be FLOAT)\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}


#include "f_def.h"
ERROR_TYPE f_imasin(IMAGE *im)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix;i++){
      p1[i] = (PIX_TYPE)asinf(p1[i]);
  }
  return(NO_ERROR);
}
#include "f_undef.h"


ERROR_TYPE imasin(IMAGE *im)
{
  switch (GetImDataType(im)){

  case t_FLOAT:
    return(f_imasin(im));
    break;

  default:
    (void)sprintf(buf,"ERROR imasin(im): invalid pixel type (must be FLOAT)\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}



/*************************************************************************/
/*                        im1[i] =  bg if im1[i] in [t1,t2]              */
/*                        im[i] =  fg otherwise                         */

#ifndef NO_generic_IMAGE
#include "g_def.h"
ERROR_TYPE generic_thresh(IMAGE *im, PIX_TYPE t1, PIX_TYPE t2, PIX_TYPE bg, PIX_TYPE fg)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++){
    if (p1[i] >= t1 && p1[i] <= t2)
      p1[i] = fg;
    else
      p1[i] = bg;
  }
  return(NO_ERROR);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */


#include "s_def.h"
ERROR_TYPE s_thresh(IMAGE *im, PIX_TYPE t1, PIX_TYPE t2, PIX_TYPE bg, PIX_TYPE fg)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++){
    if (p1[i] >= t1 && p1[i] <= t2)
      p1[i] = fg;
    else
      p1[i] = bg;
  }
  return(NO_ERROR);
}
#include "s_undef.h"


#include "i32_def.h"
ERROR_TYPE i32_thresh(IMAGE *im, PIX_TYPE t1, PIX_TYPE t2, PIX_TYPE bg, PIX_TYPE fg)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++){
    if (p1[i] >= t1 && p1[i] <= t2)
      p1[i] = fg;
    else
      p1[i] = bg;
  }
  return(NO_ERROR);
}
#include "i32_undef.h"

#include "us_def.h"
ERROR_TYPE us_thresh(IMAGE *im, PIX_TYPE t1, PIX_TYPE t2, PIX_TYPE bg, PIX_TYPE fg)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++){
    if (p1[i] >= t1 && p1[i] <= t2)
      p1[i] = fg;
    else
      p1[i] = bg;
  }
  return(NO_ERROR);
}
#include "us_undef.h"

#include "u32_def.h"
ERROR_TYPE u32_thresh(IMAGE *im, PIX_TYPE t1, PIX_TYPE t2, PIX_TYPE bg, PIX_TYPE fg)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++){
    if (p1[i] >= t1 && p1[i] <= t2)
      p1[i] = fg;
    else
      p1[i] = bg;
  }
  return(NO_ERROR);
}
#include "u32_undef.h"

#include "f_def.h"
ERROR_TYPE f_thresh(IMAGE *im, PIX_TYPE t1, PIX_TYPE t2, PIX_TYPE bg, PIX_TYPE fg)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++){
    if (p1[i] >= t1 && p1[i] <= t2)
      p1[i] = fg;
    else
      p1[i] = bg;
  }
  return(NO_ERROR);
}
#include "f_undef.h"

#include "f_def.h"
ERROR_TYPE f_threshstrict(IMAGE *im, PIX_TYPE t1, PIX_TYPE t2, PIX_TYPE bg, PIX_TYPE fg)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++){
    if (p1[i] > t1 && p1[i] < t2)
      p1[i] = fg;
    else
      p1[i] = bg;
  }
  return(NO_ERROR);
}
#include "f_undef.h"

ERROR_TYPE thresh(IMAGE *im, G_TYPE gt1, G_TYPE gt2, G_TYPE gbg, G_TYPE gfg)
{

  switch (GetImDataType(im)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_thresh(im, gt1.generic_val, gt2.generic_val, gbg.generic_val, gfg.generic_val));
    break;
#endif

#ifndef NO_uc_IMAGE
  case t_UCHAR:
    return(uc_thresh(im, gt1.uc_val, gt2.uc_val, gbg.uc_val, gfg.uc_val));
    break;
#endif

  case t_USHORT:
    return(us_thresh(im, gt1.us_val, gt2.us_val, gbg.us_val, gfg.us_val));
    break;

  case t_SHORT:
    return(s_thresh(im, gt1.s_val, gt2.s_val, gbg.s_val, gfg.s_val));
    break;

  case t_UINT32:
    return(u32_thresh(im, gt1.u32_val, gt2.u32_val, gbg.u32_val, gfg.u32_val));
    break;

  case t_INT32:
    return(i32_thresh(im, gt1.i32_val, gt2.i32_val, gbg.i32_val, gfg.i32_val));
    break;

  case t_FLOAT:
    return(f_thresh(im, gt1.f_val, gt2.f_val, gbg.f_val, gfg.f_val));
    break;

#ifndef NO_d_IMAGE
  case t_DOUBLE:
    return(d_thresh(im, gt1.d_val, gt2.d_val, gbg.d_val, gfg.d_val));
    break;
#endif

  default:
    (void)sprintf(buf,"thresh(im, gt1, gt2, gbg, gfg): invalid pixel type\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}

/*************************************************************************/
/*                        im[i] =  val if im[i] in [t1,t2]               */

#ifndef NO_generic_IMAGE
#include "g_def.h"
ERROR_TYPE generic_setlevel(IMAGE *im, PIX_TYPE t1, PIX_TYPE t2, PIX_TYPE val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1  = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);
#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++){
    if (p1[i] >= t1 && p1[i] <= t2)
      p1[i] = val;
  }
  return(NO_ERROR);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */

#include "s_def.h"
ERROR_TYPE s_setlevel(IMAGE *im, PIX_TYPE t1, PIX_TYPE t2, PIX_TYPE val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1  = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++){
    if (p1[i] >= t1 && p1[i] <= t2)
      p1[i] = val;
  }
  return(NO_ERROR);
}
#include "s_undef.h"


#include "us_def.h"
ERROR_TYPE us_setlevel(IMAGE *im, PIX_TYPE t1, PIX_TYPE t2, PIX_TYPE val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1  = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++){
    if (p1[i] >= t1 && p1[i] <= t2)
      p1[i] = val;
  }
  return(NO_ERROR);
}
#include "us_undef.h"


#include "i32_def.h"
ERROR_TYPE i32_setlevel(IMAGE *im, PIX_TYPE t1, PIX_TYPE t2, PIX_TYPE val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1  = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++){
    if (p1[i] >= t1 && p1[i] <= t2)
      p1[i] = val;
  }
  return(NO_ERROR);
}
#include "i32_undef.h"

#include "u32_def.h"
ERROR_TYPE u32_setlevel(IMAGE *im, PIX_TYPE t1, PIX_TYPE t2, PIX_TYPE val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1  = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++){
    if (p1[i] >= t1 && p1[i] <= t2)
      p1[i] = val;
  }
  return(NO_ERROR);
}
#include "u32_undef.h"

#include "f_def.h"
ERROR_TYPE f_setlevel(IMAGE *im, PIX_TYPE t1, PIX_TYPE t2, PIX_TYPE val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1  = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++){
    if (p1[i] >= t1 && p1[i] <= t2)
      p1[i] = val;
  }
  return(NO_ERROR);
}
#include "f_undef.h"

#include "d_def.h"
ERROR_TYPE d_setlevel(IMAGE *im, PIX_TYPE t1, PIX_TYPE t2, PIX_TYPE val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1  = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++){
    if (p1[i] >= t1 && p1[i] <= t2)
      p1[i] = val;
  }
  return(NO_ERROR);
}
#include "d_undef.h"

ERROR_TYPE setlevel(IMAGE *im, G_TYPE gt1, G_TYPE gt2, G_TYPE gval)
{
  switch (GetImDataType(im)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_setlevel(im, gt1.generic_val, gt2.generic_val, gval.generic_val));
    break;
#endif

#ifndef NO_uc_IMAGE
  case t_UCHAR:
    return(uc_setlevel(im, gt1.uc_val, gt2.uc_val, gval.uc_val));
    break;
#endif

  case t_USHORT:
    return(us_setlevel(im, gt1.us_val, gt2.us_val, gval.us_val));
    break;

  case t_SHORT:
    return(s_setlevel(im, gt1.s_val, gt2.s_val, gval.s_val));
    break;

  case t_UINT32:
    return(u32_setlevel(im, gt1.u32_val, gt2.u32_val, gval.u32_val));
    break;

  case t_INT32:
    return(i32_setlevel(im, gt1.i32_val, gt2.i32_val, gval.i32_val));
    break;

  case t_FLOAT:
    return(f_setlevel(im, gt1.f_val, gt2.f_val, gval.f_val));
    break;

  case t_DOUBLE:
    return(d_setlevel(im, gt1.d_val, gt2.d_val, gval.d_val));
    break;

  default:
    (void)sprintf(buf,"setlevel(im, gt1, gt2, gval): invalid pixel type\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}


/*************************************************************************/
/*                        im[i] =  im[i] mod val                         */
/* beware that modff is used for float types (no true generic function ! */

#ifndef NO_generic_IMAGE
#include "g_def.h"
ERROR_TYPE generic_modulo(IMAGE *im, int val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++)
    p1[i] %= val;
  return(NO_ERROR);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */


#include "us_def.h"
ERROR_TYPE us_modulo(IMAGE *im, int val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++)
    p1[i] %= val;
  return(NO_ERROR);
}
#include "us_undef.h"


#include "i32_def.h"
ERROR_TYPE i32_modulo(IMAGE *im, int val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++)
    p1[i] %= val;
  return(NO_ERROR);
}
#include "i32_undef.h"


#include "u32_def.h"
ERROR_TYPE u32_modulo(IMAGE *im, int val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++)
    p1[i] %= val;
  return(NO_ERROR);
}
#include "u32_undef.h"


#include "f_def.h"
ERROR_TYPE f_modulo(IMAGE *im, int val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);
  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++)
    p1[i]= fmodf(p1[i], (float)val);
  return(NO_ERROR);
}
#include "f_undef.h"


ERROR_TYPE modulo(IMAGE *im, int val)
{
  if (val==0){
    (void)sprintf(buf,"modulo(im, val): val must be different from 0\n"); errputstr(buf);
    return(ERROR);
  }
  switch (GetImDataType(im)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_modulo(im, val));
    break;
#endif

#ifndef NO_uc_IMAGE
  case t_UCHAR:
    return(uc_modulo(im, val));
    break;
#endif

  case t_USHORT:
    return(us_modulo(im, val));
    break;

#ifndef NO_s_IMAGE
  case t_SHORT:
    return(s_modulo(im, val));
    break;
#endif

  case t_UINT32:
    return(u32_modulo(im, val));
    break;

  case t_INT32:
    return(i32_modulo(im, val));
    break;

  case t_FLOAT:
    return(f_modulo(im, val));
    break;

  default:
    (void)sprintf(buf,"modulo(im, val): invalid pixel type\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}


/*************************************************************************/
/*                        im[i] =  complement[ im[i] ]                 */


#ifndef NO_generic_IMAGE
#include "g_def.h"
ERROR_TYPE generic_complement(IMAGE *im)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

  for (i=0; i<npix; i++, p1++){
#if SIGNED
  *p1 = ((PIX_TYPE)-1) * *p1;
#else
  *p1 = PIX_MAX - *p1;
#endif
  }
  return(NO_ERROR);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */


#include "us_def.h"
ERROR_TYPE us_complement(IMAGE *im)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

  for (i=0; i<npix; i++, p1++){
#if SIGNED
  *p1 = ((PIX_TYPE)-1) * *p1;
#else
  *p1 = PIX_MAX - *p1;
#endif
  }
  return(NO_ERROR);
}
#include "us_undef.h"


#include "i32_def.h"
ERROR_TYPE i32_complement(IMAGE *im)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

  for (i=0; i<npix; i++, p1++){
#if SIGNED
  *p1 = ((PIX_TYPE)-1) * *p1;
#else
  *p1 = PIX_MAX - *p1;
#endif
  }
  return(NO_ERROR);
}
#include "i32_undef.h"


ERROR_TYPE complement(IMAGE *im)
{

  switch (GetImDataType(im)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_complement(im));
    break;
#endif

#ifndef NO_uc_IMAGE
  case t_UCHAR:
    return(uc_complement(im));
    break;
#endif

  case t_USHORT:
    return(us_complement(im));
    break;


#ifndef NO_s_IMAGE
  case t_SHORT:
    return(s_complement(im));
    break;
#endif

#ifndef NO_u32_IMAGE
  case t_UINT32:
    return(u32_complement(im));
    break;
#endif


  case t_INT32:
    return(i32_complement(im));
    break;


  default:
    (void)sprintf(buf,"complement(im): invalid pixel type\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}

#include "uc_def.h"
ERROR_TYPE uc_power2p(IMAGE *im)
{
  long int i;
  long int npix=GetImNPix(im);
  PIX_TYPE *p=(PIX_TYPE *)GetImPtr(im);

  for(i=0;i<npix;i++,p++){
    *p = ~*p;
    if ( ((*p+1)|*p) == PIX_MAX)
      *p=1;
    else
      *p=0;
  }
  return NO_ERROR;
}
#include "uc_undef.h"

#include "us_def.h"
ERROR_TYPE us_power2p(IMAGE *im)
{
  long int i;
  long int npix=GetImNPix(im);
  PIX_TYPE *p=(PIX_TYPE *)GetImPtr(im);

  for(i=0;i<npix;i++,p++){
    *p = ~*p;
    if ( ((*p+1)|*p) == PIX_MAX)
      *p=1;
    else
      *p=0;
  }
  return NO_ERROR;
}
#include "us_undef.h"

#include "u32_def.h"
ERROR_TYPE u32_power2p(IMAGE *im)
{
  long int i;
  long int npix=GetImNPix(im);
  PIX_TYPE *p=(PIX_TYPE *)GetImPtr(im);

  for(i=0;i<npix;i++,p++){
    *p = ~*p;
    if ( ((*p+1)|*p) == PIX_MAX)
      *p=1;
    else
      *p=0;
  }
  return NO_ERROR;
}
#include "u32_undef.h"

ERROR_TYPE power2p(IMAGE *im)
{
  /* power of 2 predicate 2004-12-7   */
  /* only for unsigned data types !!! */
  switch (GetImDataType(im)){

  case t_UCHAR:
    return(uc_power2p(im));
    break;

  case t_USHORT:
    return(us_power2p(im));
    break;

  case t_UINT32:
    return(u32_power2p(im));
    break;

  default:
    (void)sprintf(buf,"power2p(im): invalid pixel type (must be unsigned!)\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}

/*************************************************************************/
/*                            im[i] =  val                              */

#ifndef NO_generic_IMAGE
#include "g_def.h"
ERROR_TYPE generic_blank(IMAGE *im, PIX_TYPE val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1  = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++)
    p1[i] = val;
  return(NO_ERROR);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */

#include "i32_def.h"
ERROR_TYPE i32_blank(IMAGE *im, PIX_TYPE val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1  = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++)
    p1[i] = val;
  return(NO_ERROR);
}
#include "i32_undef.h"


#include "u32_def.h"
ERROR_TYPE u32_blank(IMAGE *im, PIX_TYPE val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1  = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++)
    p1[i] = val;
  return(NO_ERROR);
}
#include "u32_undef.h"


#include "us_def.h"
ERROR_TYPE us_blank(IMAGE *im, PIX_TYPE val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1  = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++)
    p1[i] = val;
  return(NO_ERROR);
}
#include "us_undef.h"

#include "f_def.h"
ERROR_TYPE f_blank(IMAGE *im, PIX_TYPE val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1  = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++)
    p1[i] = val;
  return(NO_ERROR);
}
#include "f_undef.h"

#include "d_def.h"
ERROR_TYPE d_blank(IMAGE *im, PIX_TYPE val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1  = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++)
    p1[i] = val;
  return(NO_ERROR);
}
#include "d_undef.h"

ERROR_TYPE blank(IMAGE *im, G_TYPE gval)
{

  switch (GetImDataType(im)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_blank(im, gval.generic_val));
    break;
#endif

#ifndef NO_uc_IMAGE
  case t_UCHAR:
    return(uc_blank(im, gval.uc_val));
    break;
#endif

  case t_USHORT:
    return(us_blank(im, gval.us_val));
    break;

#ifndef NO_s_IMAGE
  case t_SHORT:
    return(s_blank(im, gval.s_val));
    break;
#endif

  case t_UINT32:
    return(u32_blank(im, gval.u32_val));
    break;

  case t_INT32:
    return(i32_blank(im, gval.i32_val));
    break;

  case t_FLOAT:
    return(f_blank(im, gval.f_val));
    break;

  case t_DOUBLE:
    return(d_blank(im, gval.d_val));
    break;

  default:
    (void)sprintf(buf,"blank(im, gval): invalid pixel type\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}

/*************************************************************************/
/*                    im[i] =  im[i] <shift> val                       */

#ifndef NO_generic_IMAGE
#include "g_def.h"
ERROR_TYPE generic_shift(IMAGE *im, int val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);


  if (val > 0){
    for (i=0; i<npix; i++, p1++)
     *p1 >>=  val;
  }
  else{
    val = abs(val);
    for (i=0; i<npix; i++, p1++)
     *p1 <<=  val;
  }
  return(NO_ERROR);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */

#include "us_def.h"
ERROR_TYPE us_shift(IMAGE *im, int val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);


  if (val > 0){
    for (i=0; i<npix; i++, p1++)
     *p1 >>=  val;
  }
  else{
    val = abs(val);
    for (i=0; i<npix; i++, p1++)
     *p1 <<=  val;
  }
  return(NO_ERROR);
}
#include "us_undef.h"

#include "i32_def.h"
ERROR_TYPE i32_shift(IMAGE *im, int val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);

  if (val > 0){
    for (i=0; i<npix; i++, p1++)
     *p1 >>=  val;
  }
  else{
    val = abs(val);
    for (i=0; i<npix; i++, p1++)
     *p1 <<=  val;
  }
  return(NO_ERROR);
}
#include "i32_undef.h"

#include "u32_def.h"
ERROR_TYPE u32_shift(IMAGE *im, int val)
{
  mia_size_t i, npix;
  PIX_TYPE *p1;

  p1 = (PIX_TYPE *)GetImPtr(im);

  npix = GetImNPix(im);


  if (val > 0){
    for (i=0; i<npix; i++, p1++)
     *p1 >>=  val;
  }
  else{
    val = abs(val);
    for (i=0; i<npix; i++, p1++)
     *p1 <<=  val;
  }
  return(NO_ERROR);
}
#include "u32_undef.h"

ERROR_TYPE shift(IMAGE *im, int val)
{
  /*
  ** author: P. Soille
  ** im:
  ** val:

  ** comment: image must not be float or double
  */

  switch (GetImDataType(im)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_shift(im, val));
    break;
#endif

#ifndef NO_uc_IMAGE
  case t_UCHAR:
    return(uc_shift(im, val));
    break;
#endif

  case t_USHORT:
    return(us_shift(im, val));
    break;

#ifndef NO_s_IMAGE
  case t_SHORT:
    return(s_shift(im, val));
    break;
#endif

  case t_INT32:
    return(i32_shift(im, val));
    break;

  case t_UINT32:
    return(u32_shift(im, val));
    break;


  default:
    (void)sprintf(buf,"shift(im, val): invalid pixel type\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}



/*************************************************************************/

#ifndef NO_generic_IMAGE
#include "g_def.h"
ERROR_TYPE generic_setrange(IMAGE *im, PIX_TYPE lval, PIX_TYPE uval)
{
  mia_size_t i, npix;
  PIX_TYPE deltai, deltao, minii, maxii;
  double a, b;
  PIX_TYPE *p1;
  G_TYPE *pg;

  if (uval < lval){
    (void)sprintf(buf,"setrange(): uval<lval\n"); errputstr(buf);
    return(ERROR);
  }

  /* get min & max values */
  pg = min_max(im);
  if (pg == NULL)
    return(ERROR);
  minii  = pg[0].generic_val;
  maxii  = pg[1].generic_val;
  free((char *)pg);

  p1     = (PIX_TYPE *)GetImPtr(im);
  npix   = GetImNPix(im);
  deltai = maxii - minii;
  deltao = uval - lval;
  if (deltai == 0){
    (void)sprintf(buf,"setrange(): deltai=0\n"); errputstr(buf);
    return(ERROR);
  }

  a = (double)deltao/(double)deltai;
  b = (double)lval - a * (double)minii;
#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++)
    p1[i] = (PIX_TYPE) (a * (double)p1[i] + b);
  return(NO_ERROR);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */


#include "us_def.h"
ERROR_TYPE us_setrange(IMAGE *im, PIX_TYPE lval, PIX_TYPE uval)
{
  mia_size_t i, npix;
  PIX_TYPE deltai, deltao, minii, maxii;
  double a, b;
  PIX_TYPE *p1;
  G_TYPE *pg;

  if (uval < lval){
    (void)sprintf(buf,"setrange(): uval<lval\n"); errputstr(buf);
    return(ERROR);
  }

  /* get min & max values */
  pg = min_max(im);
  if (pg == NULL)
    return(ERROR);
  minii  = pg[0].us_val;
  maxii  = pg[1].us_val;
  free((char *)pg);

  p1     = (PIX_TYPE *)GetImPtr(im);
  npix   = GetImNPix(im);
  deltai = maxii - minii;
  deltao = uval - lval;
  if (deltai == 0){
    (void)sprintf(buf,"setrange(): deltai=0\n"); errputstr(buf);
    return(ERROR);
  }

  a = (double)deltao/(double)deltai;
  b = (double)lval - a * (double)minii;
#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++)
    p1[i] = (PIX_TYPE) (a * (double)p1[i] + b);
  return(NO_ERROR);
}
#include "us_undef.h"


#include "i32_def.h"
ERROR_TYPE i32_setrange(IMAGE *im, PIX_TYPE lval, PIX_TYPE uval)
{
  mia_size_t i, npix;
  PIX_TYPE deltai, deltao, minii, maxii;
  double a, b;
  PIX_TYPE *p1;
  G_TYPE *pg;

  if (uval < lval){
    (void)sprintf(buf,"setrange(): uval<lval\n"); errputstr(buf);
    return(ERROR);
  }

  /* get min & max values */
  pg = min_max(im);
  if (pg == NULL)
    return(ERROR);
  minii  = pg[0].i32_val;
  maxii  = pg[1].i32_val;
  free((char *)pg);

  p1     = (PIX_TYPE *)GetImPtr(im);
  npix   = GetImNPix(im);
  deltai = maxii - minii;
  deltao = uval - lval;
  if (deltai == 0){
    (void)sprintf(buf,"setrange(): deltai=0\n"); errputstr(buf);
    return(ERROR);
  }

  a = (double)deltao/(double)deltai;
  b = (double)lval - a * (double)minii;
#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++)
    p1[i] = (PIX_TYPE) (a * (double)p1[i] + b);
  return(NO_ERROR);
}
#include "i32_undef.h"


#include "f_def.h"
ERROR_TYPE f_setrange(IMAGE *im, PIX_TYPE lval, PIX_TYPE uval)
{
  mia_size_t i, npix;
  PIX_TYPE deltai, deltao, minii, maxii;
  double a, b;
  PIX_TYPE *p1;
  G_TYPE *pg;

  if (uval < lval){
    (void)sprintf(buf,"setrange(): uval<lval\n"); errputstr(buf);
    return(ERROR);
  }

  /* get min & max values */
  pg = min_max(im);
  if (pg == NULL)
    return(ERROR);
  minii  = pg[0].f_val;
  maxii  = pg[1].f_val;
  free((char *)pg);

  p1     = (PIX_TYPE *)GetImPtr(im);
  npix   = GetImNPix(im);
  deltai = maxii - minii;
  deltao = uval - lval;
  if (deltai == 0){
    (void)sprintf(buf,"setrange(): deltai=0\n"); errputstr(buf);
    return(ERROR);
  }

  a = (double)deltao/deltai;
  b = lval - a * minii;
#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++)
    p1[i] = (PIX_TYPE) (a * (double)p1[i] + b);
  return(NO_ERROR);
}
#include "f_undef.h"


#include "d_def.h"
ERROR_TYPE d_setrange(IMAGE *im, PIX_TYPE lval, PIX_TYPE uval)
{
  mia_size_t i, npix;
  PIX_TYPE deltai, deltao, minii, maxii;
  double a, b;
  PIX_TYPE *p1;
  G_TYPE *pg;

  if (uval < lval){
    (void)sprintf(buf,"setrange(): uval<lval\n"); errputstr(buf);
    return(ERROR);
  }

  /* get min & max values */
  pg = min_max(im);
  if (pg == NULL)
    return(ERROR);
  minii  = pg[0].d_val;
  maxii  = pg[1].d_val;
  free((char *)pg);

  p1     = (PIX_TYPE *)GetImPtr(im);
  npix   = GetImNPix(im);
  deltai = maxii - minii;
  deltao = uval - lval;
  if (deltai == 0){
    (void)sprintf(buf,"setrange(): deltai=0\n"); errputstr(buf);
    return(ERROR);
  }

  a = (double)deltao/deltai;
  b = lval - a * minii;
#ifdef OPENMP
#pragma omp parallel for
#endif
  for (i=0; i<npix; i++)
    p1[i] = (PIX_TYPE) (a * p1[i] + b);
  return(NO_ERROR);
}
#include "d_undef.h"



ERROR_TYPE setrange(IMAGE *im, G_TYPE gt1, G_TYPE gt2)
{
  switch (GetImDataType(im)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_setrange(im, gt1.generic_val, gt2.generic_val));
    break;
#endif

#ifndef NO_uc_IMAGE
  case t_UCHAR:
    return(uc_setrange(im, gt1.uc_val, gt2.uc_val));
    break;
#endif


  case t_USHORT:
    return(us_setrange(im, gt1.us_val, gt2.us_val));
    break;


#ifndef NO_s_IMAGE
  case t_SHORT:
    return(s_setrange(im, gt1.s_val, gt2.s_val));
    break;
#endif

#ifndef NO_u32_IMAGE
  case t_UINT32:
    return(u32_setrange(im, gt1.u32_val, gt2.u32_val));
    break;
#endif

  case t_INT32:
    return(i32_setrange(im, gt1.i32_val, gt2.i32_val));
    break;

  case t_FLOAT:
    return(f_setrange(im, gt1.f_val, gt2.f_val));
    break;

  case t_DOUBLE:
    return(d_setrange(im, gt1.d_val, gt2.d_val));
    break;

  default:
    (void)sprintf(buf,"setrange(im, gt1, gt2): invalid pixel type\n"); errputstr(buf);
    return(ERROR);
  }
  return(NO_ERROR);
}

/*************************************************************************/
/*                     imout[i] = ndi(im1[i], im2[i])                    */

#ifndef NO_generic_IMAGE
#include "g_def.h"
IMAGE *generic_ndi(IMAGE *im1, IMAGE *im2)
{
  mia_size_t i, ovfl = 0, npix;
  IMAGE *imout;
  PIX_TYPE *p1, *p2;
  MIALFLOAT *pout;
  //pk 24/04/2023 Dangerous: should be defined as local variable within parallel loop
  /* MIALFLOAT dval; */

  imout = create_image(t_MIALFLOAT, GetImNx(im1), GetImNy(im1), GetImNz(im1));
  if (imout == NULL){
    (void)sprintf(buf,"generic_ndi(im): not enough memory!\n"); errputstr(buf);
    return(NULL);
  }

  p1 = (PIX_TYPE *)GetImPtr(im1);
  p2 = (PIX_TYPE *)GetImPtr(im2);
  pout = (MIALFLOAT *)GetImPtr(imout);

  npix = GetImNPix(im1);
#ifdef OPENMP
#pragma omp parallel for reduction(+:ovfl)
#endif
  for (i=0; i<npix; i++){
    MIALFLOAT dval=(MIALFLOAT)p1[i]+(MIALFLOAT)p2[i];
    if (dval==0.0){
      pout[i]=-2;
      /* dval=(MIALFLOAT)p1[i]-(MIALFLOAT)p2[i]; */
      /* if (dval==0.0) */
      /* 	pout[i]=FP_NAN; */
      /* else */
      /* 	pout[i]=FP_INFINITE; */
      ovfl+=1;
    }
    else
      pout[i]=((MIALFLOAT)p1[i]-(MIALFLOAT)p2[i])/dval;
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in generic_ndi(im1, im2): \
             %ld division(s) by 0 set to -2)\n", ovfl); stdputstr(buf);
  }
  return(imout);
}
#include "g_undef.h"
#endif /* #ifndef NO_generic_IMAGE */

#include "us_def.h"
IMAGE *us_ndi(IMAGE *im1, IMAGE *im2)
{
  mia_size_t i, ovfl = 0, npix;
  IMAGE *imout;
  PIX_TYPE *p1, *p2;
  MIALFLOAT *pout;
  //pk 24/04/2023 Dangerous: should be defined as local variable within parallel loop
  /* MIALFLOAT dval; */

  imout = create_image(t_MIALFLOAT, GetImNx(im1), GetImNy(im1), GetImNz(im1));
  if (imout == NULL){
    (void)sprintf(buf,"us_ndi(im): not enough memory!\n"); errputstr(buf);
    return(NULL);
  }

  p1 = (PIX_TYPE *)GetImPtr(im1);
  p2 = (PIX_TYPE *)GetImPtr(im2);
  pout = (MIALFLOAT *)GetImPtr(imout);

  npix = GetImNPix(im1);
#ifdef OPENMP
#pragma omp parallel for reduction(+:ovfl)
#endif
  for (i=0; i<npix; i++){
    MIALFLOAT dval=(MIALFLOAT)p1[i]+(MIALFLOAT)p2[i];
    if (dval==0.0){
      pout[i]=-2;
      /* dval=(MIALFLOAT)p1[i]-(MIALFLOAT)p2[i]; */
      /* if (dval==0.0) */
      /* 	pout[i]=FP_NAN; */
      /* else */
      /* 	pout[i]=FP_INFINITE; */
      ovfl+=1;
    }
    else
      pout[i]=((MIALFLOAT)p1[i]-(MIALFLOAT)p2[i])/dval;
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in us_ndi(im1, im2): \
             %ld division(s) by 0 set to -2)\n", ovfl); stdputstr(buf);
  }
  return(imout);
}
#include "us_undef.h"

#include "i32_def.h"
IMAGE *i32_ndi(IMAGE *im1, IMAGE *im2)
{
  mia_size_t i, ovfl = 0, npix;
  IMAGE *imout;
  PIX_TYPE *p1, *p2;
  MIALFLOAT *pout;
  //pk 24/04/2023 Dangerous: should be defined as local variable within parallel loop
  /* MIALFLOAT dval; */

  imout = create_image(t_MIALFLOAT, GetImNx(im1), GetImNy(im1), GetImNz(im1));
  if (imout == NULL){
    (void)sprintf(buf,"i32_ndi(im): not enough memory!\n"); errputstr(buf);
    return(NULL);
  }

  p1 = (PIX_TYPE *)GetImPtr(im1);
  p2 = (PIX_TYPE *)GetImPtr(im2);
  pout = (MIALFLOAT *)GetImPtr(imout);

  npix = GetImNPix(im1);

#ifdef OPENMP
#pragma omp parallel for reduction(+:ovfl)
#endif
  for (i=0; i<npix; i++){
    MIALFLOAT dval=(MIALFLOAT)p1[i]+(MIALFLOAT)p2[i];
    if (dval==0.0){
      pout[i]=-2;
      /* dval=(MIALFLOAT)p1[i]-(MIALFLOAT)p2[i]; */
      /* if (dval==0.0) */
      /* 	pout[i]=FP_NAN; */
      /* else */
      /* 	pout[i]=FP_INFINITE; */
      ovfl+=1;
    }
    else
      pout[i]=((MIALFLOAT)p1[i]-(MIALFLOAT)p2[i])/dval;
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in i32_ndi(im1, im2): \
             %ld division(s) by 0 set to -2)\n", ovfl); stdputstr(buf);
  }
  return(imout);
}
#include "i32_undef.h"

#include "u32_def.h"
IMAGE *u32_ndi(IMAGE *im1, IMAGE *im2)
{
  mia_size_t i, ovfl = 0, npix;
  IMAGE *imout;
  PIX_TYPE *p1, *p2;
  MIALFLOAT *pout;
  //pk 24/04/2023 Dangerous: should be defined as local variable within parallel loop
  /* MIALFLOAT dval; */

  imout = create_image(t_MIALFLOAT, GetImNx(im1), GetImNy(im1), GetImNz(im1));
  if (imout == NULL){
    (void)sprintf(buf,"u32_ndi(im): not enough memory!\n"); errputstr(buf);
    return(NULL);
  }

  p1 = (PIX_TYPE *)GetImPtr(im1);
  p2 = (PIX_TYPE *)GetImPtr(im2);
  pout = (MIALFLOAT *)GetImPtr(imout);

  npix = GetImNPix(im1);

#ifdef OPENMP
#pragma omp parallel for reduction(+:ovfl)
#endif
  for (i=0; i<npix; i++){
    MIALFLOAT dval=(MIALFLOAT)p1[i]+(MIALFLOAT)p2[i];
    if (dval==0.0){
      pout[i]=-2;
      /* dval=(MIALFLOAT)p1[i]-(MIALFLOAT)p2[i]; */
      /* if (dval==0.0) */
      /* 	pout[i]=FP_NAN; */
      /* else */
      /* 	pout[i]=FP_INFINITE; */
      ovfl+=1;
    }
    else
      pout[i]=((MIALFLOAT)p1[i]-(MIALFLOAT)p2[i])/dval;
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in u32_ndi(im1, im2): \
             %ld division(s) by 0 set to -2)\n", ovfl); stdputstr(buf);
  }
  return(imout);
}
#include "u32_undef.h"

#include "f_def.h"
IMAGE *f_ndi(IMAGE *im1, IMAGE *im2)
{
  mia_size_t i, ovfl = 0, npix;
  IMAGE *imout;
  PIX_TYPE *p1, *p2;
  MIALFLOAT *pout;
  //pk 24/04/2023 Dangerous: should be defined as local variable within parallel loop
  /* MIALFLOAT dval; */

  imout = create_image(t_MIALFLOAT, GetImNx(im1), GetImNy(im1), GetImNz(im1));
  if (imout == NULL){
    (void)sprintf(buf,"f_ndi(im): not enough memory!\n"); errputstr(buf);
    return(NULL);
  }

  p1 = (PIX_TYPE *)GetImPtr(im1);
  p2 = (PIX_TYPE *)GetImPtr(im2);
  pout = (MIALFLOAT *)GetImPtr(imout);

  npix = GetImNPix(im1);

#ifdef OPENMP
#pragma omp parallel for reduction(+:ovfl)
#endif
  for (i=0; i<npix; i++){
    MIALFLOAT dval=(MIALFLOAT)p1[i]+(MIALFLOAT)p2[i];
    if (dval==0.0){
      pout[i]=-2;
      /* dval=(MIALFLOAT)p1[i]-(MIALFLOAT)p2[i]; */
      /* if (dval==0.0) */
      /* 	pout[i]=FP_NAN; */
      /* else */
      /* 	pout[i]=FP_INFINITE; */
      ovfl+=1;
    }
    else
      pout[i]=((MIALFLOAT)p1[i]-(MIALFLOAT)p2[i])/dval;
  }
  if (ovfl){
    (void)sprintf(buf, "WARNING in f_ndi(im1, im2): \
             %ld division(s) by 0 set to -2)\n", ovfl); stdputstr(buf);
  }
  return(imout);
}
#include "f_undef.h"

IMAGE *ndi(IMAGE *im1, IMAGE *im2)
{
  /* check for possible errors */
  if (szcompat(im1, im2) != NO_ERROR){
    (void)sprintf(buf,"ERROR in arith(im1, im2, op): \
                images of different size or type\n"); errputstr(buf);
    return(NULL);
  }

  switch (GetImDataType(im1)){

#ifndef NO_generic_IMAGE
  case t_GENERIC:
    return(generic_ndi(im1, im2));
    break;
#endif
  case t_USHORT:
    return(us_ndi(im1, im2));
    break;
  case t_INT32:
    return(i32_ndi(im1, im2));
    break;
  case t_UINT32:
    return(u32_ndi(im1, im2));
    break;
  case t_FLOAT:
    return(f_ndi(im1, im2));
    break;
  default:
    (void)sprintf(buf,"ndi(im, im2): invalid pixel type\n"); errputstr(buf);
    return(NULL);
  }
}

/**@}*/
