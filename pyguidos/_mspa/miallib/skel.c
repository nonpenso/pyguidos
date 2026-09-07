/***********************************************************************
Author(s): Marcin Iwanowski and Pierre Soille
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

/**
 * @file   skel.c
 *
 * @date
 *
 * @details see also \cite iwanowski-soille2005caip1
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "miallib.h"
#include "fifo.h"

#include "uc_def.h"




/** \addtogroup group_hmtsk
 *  @{
 */



/* binary skeletonizaton procedures from august 2005
   last modification 17.08.05 - Marcin Iwanowski */

/* replace all addframebox by framebox and commented
   all subframebox calls on 3/5/2006 Pierre */

void getngbshift8 (int shift[9],int nx)
{
  /* neighbor shift */
  shift[0] = -1;  /* 4 connected ngbs */
  shift[1] = 1;
  shift[2] = -nx;
  shift[3] = nx;
  shift[4] = -nx-1; /* 8 connected ngbs */
  shift[5] = nx-1;
  shift[6] = -nx+1;
  shift[7] = nx+1;
  shift[8] = 0;  /*  central point */
}

int simple_pixel (PIX_TYPE *pimg, int stype, long int pos, int nx)
{
  int code;
  int htab[6][256] ={ /* simple g=8, g'=4 */
                          { 0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,\
                            1,1,0,0,1,1,1,1,0,1,0,1,0,1,1,0,\
                            1,1,0,0,0,1,0,1,1,1,1,1,0,1,1,0,\
                            0,1,0,0,0,1,0,1,0,1,0,1,0,1,1,0,\
                            1,0,1,0,1,1,1,1,0,0,1,1,0,1,1,0,\
                            0,0,0,0,1,1,1,1,0,0,0,1,0,1,1,0,\
                            0,0,0,0,0,1,0,1,0,0,1,1,0,1,1,0,\
                            0,0,0,0,0,1,0,1,0,0,0,1,0,1,1,0,\
                            1,0,1,0,0,0,1,1,1,1,1,1,0,1,1,0,\
                            0,0,0,0,0,0,1,1,0,1,0,1,0,1,1,0,\
                            0,0,0,0,0,0,0,1,1,1,1,1,0,1,1,0,\
                            0,0,0,0,0,0,0,1,0,1,0,1,0,1,1,0,\
                            0,0,1,0,0,0,1,1,0,0,1,1,0,1,1,0,\
                            0,0,0,0,0,0,1,1,0,0,0,1,0,1,1,0,\
                            0,0,0,0,0,0,0,1,0,0,1,1,0,1,1,0,\
         	            0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,0 },
                           /* simple g=4, g'=8 */
                          { 0,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,\
                            0,1,1,0,1,1,0,0,1,0,0,0,0,0,0,0,\
                            0,1,1,0,1,0,0,0,1,1,0,0,0,0,0,0,\
                            0,1,1,0,1,1,0,0,1,1,0,0,0,1,0,0,\
                            0,1,1,0,1,0,1,0,1,0,0,0,0,0,0,0,\
                            0,1,1,0,1,1,1,1,1,0,0,0,0,0,0,0,\
                            0,1,1,0,1,0,1,0,1,1,0,0,0,0,0,0,\
                            0,1,1,0,1,1,1,1,1,1,0,0,0,1,0,1,\
                            0,1,1,0,1,0,0,0,1,0,1,0,0,0,0,0,\
                            0,1,1,0,1,1,0,0,1,0,1,0,0,0,0,0,\
                            0,1,1,0,1,0,0,0,1,1,1,1,0,0,0,0,\
                            0,1,1,0,1,1,0,0,1,1,1,1,0,1,0,1,\
                            0,1,1,0,1,0,1,0,1,0,1,0,0,0,1,0,\
                            0,1,1,0,1,1,1,1,1,0,1,0,0,0,1,1,\
                            0,1,1,0,1,0,1,0,1,1,1,1,0,0,1,1,\
                            0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0 },
                          /* simple g=4, g'=4 Ronse's {4,8}*/
                          { 0,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,\
                            0,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,\
                            0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,\
                            0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,\
                            0,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,\
                            0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,\
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                            0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,0,\
                            0,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,\
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                            0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,\
                            0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,\
                            0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,\
                            0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,0,\
                            0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,0,\
                            0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,0 },
                           /* w-simple g=8, g'=4;*/
                          { 1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,\
                            1,1,0,0,1,1,1,1,0,1,0,1,0,1,1,0,\
                            1,1,0,0,0,1,0,1,1,1,1,1,0,1,1,0,\
                            0,1,0,0,0,1,0,1,0,1,0,1,0,1,1,0,\
                            1,0,1,0,1,1,1,1,0,0,1,1,0,1,1,0,\
                            0,0,0,0,1,1,1,1,0,0,0,1,0,1,1,0,\
                            0,0,0,0,0,1,0,1,0,0,1,1,0,1,1,0,\
                            0,0,0,0,0,1,0,1,0,0,0,1,0,1,1,0,\
                            1,0,1,0,0,0,1,1,1,1,1,1,0,1,1,0,\
                            0,0,0,0,0,0,1,1,0,1,0,1,0,1,1,0,\
                            0,0,0,0,0,0,0,1,1,1,1,1,0,1,1,0,\
                            0,0,0,0,0,0,0,1,0,1,0,1,0,1,1,0,\
                            0,0,1,0,0,0,1,1,0,0,1,1,0,1,1,0,\
                            0,0,0,0,0,0,1,1,0,0,0,1,0,1,1,0,\
                            0,0,0,0,0,0,0,1,0,0,1,1,0,1,1,0,\
         	            0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,0 },
                           /* w-simple g=4, g'=8 */
                          { 1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,\
                            1,1,1,0,1,1,0,0,1,0,0,0,0,0,0,0,\
                            1,1,1,0,1,0,0,0,1,1,0,0,0,0,0,0,\
                            1,1,1,0,1,1,0,0,1,1,0,0,0,1,0,0,\
                            1,1,1,0,1,0,1,0,1,0,0,0,0,0,0,0,\
                            1,1,1,0,1,1,1,1,1,0,0,0,0,0,0,0,\
                            1,1,1,0,1,0,1,0,1,1,0,0,0,0,0,0,\
                            1,1,1,0,1,1,1,1,1,1,0,0,0,1,0,1,\
                            1,1,1,0,1,0,0,0,1,0,1,0,0,0,0,0,\
                            1,1,1,0,1,1,0,0,1,0,1,0,0,0,0,0,\
                            1,1,1,0,1,0,0,0,1,1,1,1,0,0,0,0,\
                            1,1,1,0,1,1,0,0,1,1,1,1,0,1,0,1,\
                            1,1,1,0,1,0,1,0,1,0,1,0,0,0,1,0,\
                            1,1,1,0,1,1,1,1,1,0,1,0,0,0,1,1,\
                            1,1,1,0,1,0,1,0,1,1,1,1,0,0,1,1,\
                            1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0 },
                           /* w-simple g=4, g'=4 Ronse's {4,8}*/
                          { 1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,\
                            1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,\
                            1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,\
                            0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,\
                            1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,\
                            0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,\
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                            0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,0,\
                            1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,\
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                            0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,\
                            0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,\
                            0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,\
                            0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,0,\
                            0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,0,\
                            0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,0 }};
  code = 0;
  if (*(pimg+pos)>0)
   {
    code  = (*(pimg+pos-1) == 1);
    code |= (*(pimg+pos+1)    == 1) <<1;
    code |= (*(pimg+pos-nx)   == 1) <<2;
    code |= (*(pimg+pos+nx)   == 1) <<3;
    code |= (*(pimg+pos-nx-1) == 1) <<4;
    code |= (*(pimg+pos+nx-1) == 1) <<5;
    code |= (*(pimg+pos-nx+1) == 1) <<6;
    code |= (*(pimg+pos+nx+1) == 1) <<7;
    return (htab[stype][code]); /* returns 1 if point on position 'pos' at level refval of image pointed by '*pimg' is simple, 0 - otherwise */
   }
  else return 0;
}

int test_anchor (PIX_TYPE *panchor, int atype, long int pos)
{
    return ((atype == 0) || (*(panchor + pos)==0));
  /* atype == 0 - we are not using anchor pixels (return 1)
           == 1   we use them and return 1 if anchor pixel at pos does not exist */
}

ERROR_TYPE binODthin_noqueue(IMAGE *imin, int stype, int atype, IMAGE *imanchor)
     /* binary order-dependent thinning non-queue version */
     /*
           stype == 0 simple g=8, g'=4
                 == 1 simple g=4, g'=8
                 == 2 simple g=4, g'=4  Ronse's {4,8}
                 == 3 simple g=8, g'=4
                 == 4 simple g=4, g'=8
                 == 5 simple g=4, g'=4  Ronse's {4,8}
        atype == 0 anchor pixels are not considered (*imanchor not used)
              == 1 anchor pixels considered (*imanchor used)
     */
{
  long int i, npix, startindoff, endindoff;
  int nx;
  //  int kk=0;
  IMAGE *isimple;
  PIX_TYPE *pimin, *pimanchor=NULL, *psimple;
  int deleted = 1;
  int sh[9];
  // int counter = 0;
  int box[6]={2,2,2,2,0,0};

  if (GetImDataType(imin) != t_UCHAR){ (void)sprintf(buf,"wrong input data type !! \n"); errputstr(buf); return ERROR; }

  if (atype == 1){
   if ( (GetImNx(imanchor) != GetImNx(imin)) || (GetImNy(imanchor) != GetImNy(imin)) )
     { (void)sprintf(buf,"input and anchor images have different sizes !! \n"); errputstr(buf); return ERROR; }
   if (GetImDataType(imanchor) != t_UCHAR){ (void)sprintf(buf,"wrong input data type !! \n"); errputstr(buf); return ERROR; }
   generic_framebox(imanchor,box,0);
   pimanchor = (PIX_TYPE *)GetImPtr(imanchor);

  }
  generic_framebox(imin,box,0);
  nx = GetImNx(imin);
  npix = GetImNPix(imin);
  startindoff = 2*nx + 2 ; /* start offset (skip the first two lines (of enlarged image) and the first point of the second) */
  endindoff = npix - 2*nx - 2 ; /* end offset (finish one pixel before the end of the second line before last */
  pimin = (PIX_TYPE *)GetImPtr(imin);

  isimple = create_image(GetImDataType(imin), nx , GetImNy(imin) , GetImNz(imin));
  if (isimple == NULL){
      (void)sprintf(buf,"not enough memory for temporary image\n"); errputstr(buf);
      return(ERROR);
  }
  psimple = (PIX_TYPE *)GetImPtr(isimple);
  getngbshift8(sh,nx);
  deleted=1;
  while (deleted){
       deleted = 0; // counter = 0;
       for (i=startindoff; i<endindoff; i++)
        if ((simple_pixel(pimin,stype,i,nx))
	    && (test_anchor(pimanchor,atype,i))){*(psimple+i)=1;/*counter++;*/} else *(psimple+i)=0;
       for (i=startindoff; i<endindoff; i++)
	if (*(psimple + i) == 1)
	 if (simple_pixel(pimin,stype,i,nx))
          {
	    deleted = 1;
            *(pimin+i) = 0;
          }
       // printf("\n k = %d, counter = %d", kk, counter);
  }
  free_image(isimple);
  // PS if (atype == 1) subframebox(imanchor,box);
  // PS subframebox(imin,box);
  return NO_ERROR;
}


ERROR_TYPE binODthin_FIFO(IMAGE *imin, int stype, int atype, IMAGE *imanchor)
     /* binary order-dependent thinning */
     /* queue version
           stype == 0 simple g=8, g'=4
                 == 1 simple g=4, g'=8
                 == 2 simple g=4, g'=4  Ronse's {4,8}
                 == 3 simple g=8, g'=4
                 == 4 simple g=4, g'=8
                 == 5 simple g=4, g'=4  Ronse's {4,8}
        atype == 0 anchor pixels are not considered (*imanchor not used)
              == 1 anchor pixels considered (*imanchor used)
     */
{
  long int i,j,npix,startindoff,endindoff;
  // int kk=0;
  IMAGE *isimple;
  PIX_TYPE *pimin, *pimanchor=NULL, *psimple;
  int nx;
  int deleted = 1;
  int sh[9];
  // int counter = 0;
  int qswitch = 0;
  FIFO4 *q_a = create_fifo4(50);
  FIFO4 *q_b = create_fifo4(50);
  FIFO4 *qcurrent, *qnext;
  FIFO4 *q2  = create_fifo4(50);
  int box[6]={2,2,2,2,0,0};

  if (GetImDataType(imin) != t_UCHAR){ (void)sprintf(buf,"wrong input data type !! \n"); errputstr(buf); return ERROR; }

  if (qswitch){qcurrent=q_a; qnext=q_b; qswitch=0;} else{qcurrent=q_b; qnext=q_a; qswitch=1;}

  if (atype == 1) /* check only if anchor image is used */
  {
   if ( (GetImNx(imanchor) != GetImNx(imin)) || (GetImNy(imanchor) != GetImNy(imin)) )
     { (void)sprintf(buf,"input and anchor images have different sizes !! \n"); errputstr(buf); return ERROR; }
   if (GetImDataType(imanchor) != t_UCHAR){ (void)sprintf(buf,"wrong input data type !! \n"); errputstr(buf); return ERROR; }
   generic_framebox(imanchor,box,0);
   pimanchor = (PIX_TYPE *)GetImPtr(imanchor);
  }
  generic_framebox(imin,box,0);

  nx = GetImNx(imin);
  npix = GetImNPix(imin);
  startindoff = 2*nx + 2 ; /* start offset (skip the first two lines (of enlarged image) and the first point of the second) */
  endindoff = npix - 2*nx - 2 ; /* end offset (finish one pixel before the end of the second line before last */
  pimin = (PIX_TYPE *)GetImPtr(imin);
  isimple = create_image ( GetImDataType(imin), nx , GetImNy(imin) , GetImNz(imin));
  if (isimple == NULL)
    {
      (void)sprintf(buf,"not enough memory for temporary image\n"); errputstr(buf);
      return(ERROR);
    }
  psimple = (PIX_TYPE *)GetImPtr(isimple);
  getngbshift8(sh,nx);
  /* all simple points are put into the queue */
  for (i=startindoff; i<endindoff; i++)
      if ((simple_pixel(pimin,stype,i,nx)) && (test_anchor(pimanchor,atype,i)))
      {*(psimple+i)=1; fifo4_add(qcurrent,(long int) i);/*counter++;*/} else *(psimple+i)=0;
  // printf (" grskel - initial simple pixels = %d ", counter);
  deleted=1;
  while (deleted){
      // counter = 0;
      deleted = 0;
      while (fifo4_empty(qcurrent) == FALSE){
	i = fifo4_remove(qcurrent);
	if (simple_pixel(pimin,stype,i,nx)){
	     deleted = 1; // counter ++;
             *(psimple+i) = 0;
             *(pimin+i) = 0;
             fifo4_add(q2, (long int) i);
	 } else fifo4_add(qnext, (long int) i); /* back to the queue */
       }
      if (deleted){
	 while (fifo4_empty(q2) == FALSE){
	     i = fifo4_remove(q2);
             for (j=0; j<8; j++)
	      if ((test_anchor(pimanchor,atype,i + sh[j])) && (simple_pixel(pimin,stype,i + sh[j],nx))
                                                           && (*(psimple + i + sh[j]) == 0)){
		 *(psimple+i+sh[j]) = 1;
		 fifo4_add(qnext,(long int) (i+sh[j]));
	       }
	    }
          // printf("\n k = %d, counter = %d", kk++, counter);
         if (qswitch){
	   qcurrent=q_a;
	   qnext=q_b;
	   qswitch=0;
	 }
	 else{
	   qcurrent=q_b;
	   qnext=q_a;
	   qswitch=1;
	 }
      }
  }
  // PS if (atype == 1) subframebox(imanchor,box);
  // PS subframebox(imin,box);

  free_image(isimple);
  free_fifo4(q_a);  free_fifo4(q_b);  free_fifo4(q2);
  return NO_ERROR;
}

int simple_pair(PIX_TYPE *pimg, int stype, long int pos, int ngbpos, int *sh, int *conftype, int *a, int *b, int *c, int *d)
{
/* checks if a pixel is independent from its neighbor */
/*  pimg - image
    stype - type of simpleness
    pos - position of pixel
    ngbpos - position of neighbor (refering to pixel pos)
    conftype - returns type of configuration
*/
 int code;
 int simple_pair2[6][4] = {
                       {0,1,1,1},   /* simple g=8, g'=4 */
                       {1,1,1,0},   /* simple g=4, g'=8 */
                       {0,1,1,0},   /* simple g=4, g'=4 Ronse's {4,8}*/
                       {1,1,1,1},   /* w-simple g=8, g'=4;*/
                       {1,1,1,0},   /* w-simple g=4, g'=8 */
                       {1,1,1,0} }; /* w-simple g=4, g'=4 Ronse's {4,8}*/

 int simple_pair4[6][16] = {
                       {0,1,1,1,1,0,0,0,1,0,0,0,1,0,0,0},   /* simple g=8, g'=4 */
                       {0,0,0,1,0,0,0,1,0,0,0,1,1,1,1,0},   /* simple g=4, g'=8 */
                       {0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0},   /* simple g=4, g'=4 Ronse's {4,8}*/
                       {1,1,1,1,1,0,0,0,1,0,0,0,1,0,0,0},   /* w-simple g=8, g'=4;*/
                       {1,0,0,1,0,0,0,1,0,0,0,1,1,1,1,0},   /* w-simple g=4, g'=8 */
                       {1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0} }; /* w-simple g=4, g'=4 Ronse's {4,8}*/
 if (ngbpos < 4)
 { /*non-diagonal neighbor */
     switch (ngbpos)
     {
      case 0: *a = sh[2] ; *b = sh[4] ; *c = sh[3] ; *d = sh[5] ; break;
      case 1: *a = sh[2] ; *b = sh[6] ; *c = sh[3] ; *d = sh[7] ; break;
      case 2: *a = sh[0] ; *b = sh[4] ; *c = sh[1] ; *d = sh[6] ; break;
      case 3: *a = sh[0] ; *b = sh[5] ; *c = sh[1] ; *d = sh[7] ; break;
     }
     code  = (*(pimg + pos + *d) >= *(pimg + pos));
     code |= (*(pimg + pos + *c) >= *(pimg + pos))<<1;
     code |= (*(pimg + pos + *b) >= *(pimg + pos))<<2;
     code |= (*(pimg + pos + *a) >= *(pimg + pos))<<3;
     *conftype = code + 5;
     return (simple_pair4[stype][code]);
 }
 else
 { /* diagonal neighbor */
     switch (ngbpos)
     {
      case 4: *a = sh[0] ; *b = sh[2] ; break;
      case 5: *a = sh[3] ; *b = sh[0] ; break;
      case 6: *a = sh[2] ; *b = sh[1] ; break;
      case 7: *a = sh[1] ; *b = sh[3] ; break;
     }
     code  = (*(pimg + pos + *b) >= *(pimg + pos));
     code |= (*(pimg + pos + *a) >= *(pimg + pos))<<1;
     *conftype = code + 1;
     return (simple_pair2[stype][code]);
 }
}

int num_sngb (PIX_TYPE *pimg, long int pos, int *sh, int ngbnum)
{
    /* returns the number of ngbs higher-equal than a given pixel */
    int i, sum=0;
    for (i=0; i<ngbnum ; i++)
	if (*(pimg + pos + sh[i]) >= *(pimg + pos)) sum ++;
    return sum;
}


int testsimple (PIX_TYPE *pimg, PIX_TYPE *pimanchor, int atype, int stype, long int pos, int ngbp, int *sh, int nx)
{
 if ((test_anchor(pimanchor,atype,pos + ngbp)) && (simple_pixel(pimg,stype,pos + ngbp,nx))
     &&  (*(pimg + pos) == *(pimg + pos + ngbp) )) return 1;
 else return 0;
}


int indep_simple (PIX_TYPE *pimg, PIX_TYPE *pimanchor, int stype, int atype, long int pos, int *sh, int nx)
{
    int i;
    int allindep = 1;
    int ngbnum = 0;
    int a,b,c,d,aa=0,bb=0;
    int ct=0;
    int conftype, founddiff, zerocount;
    for (i=0; i<8 && allindep ; i++)
      if (testsimple (pimg, pimanchor, atype, stype, pos, sh[i], sh, nx))
            {
	     ngbnum ++;
	     if (simple_pair(pimg, stype, pos, i, sh, &conftype,&a,&b,&c,&d) == 0)
               {allindep = 0; aa=a; bb=b; ct = conftype; }
            }
    if (allindep){ct = conftype;}
    if ((allindep == 0) && ((stype == 1) || (stype == 2) || (stype == 4) || (stype == 5) ))
    {
	if (ct == 4) /* check edge configuration*/
	  if ((testsimple (pimg, pimanchor, atype, stype, pos, aa, sh, nx)) &&
              (testsimple (pimg, pimanchor, atype, stype, pos, bb, sh, nx)) )
	  {
	      zerocount = 0;
              for (i=0; i<4; i++)
		  if (*(pimg + pos + sh[i]) < *(pimg + pos)) zerocount ++;
	      if (zerocount == 2)
	      {
		 allindep = 1;  /* for wshed OK for skel, check isolated quad */
		 if (stype < 3)
		 {
          	   founddiff = 0;
                   for (i=0; i<4 && (founddiff == 0); i++)
                   if (testsimple (pimg, pimanchor, atype, stype, pos, sh[i], sh, nx))
	           {
	  	     if (num_sngb (pimg, pos + sh[i],sh,4) != 2) founddiff = 1; // one simple ngb don't have 3 4-ngb's
	           }
                   if (founddiff == 0) allindep=0;
                 }
	      }
          }
      return allindep;
    }
    if ((allindep == 1) && (stype<3) )  // is independent from all simple neighbors
    {
	// check whether p belongs to a triple or quadruple of isolated simple pixels
	if  ( ( (ngbnum == 2) &&
                ((ct == 2) || (ct == 3) || (ct == 6) || (ct == 7) || (ct == 9) || (ct == 13))
	      ) ||
              ( (ngbnum == 3) &&
                ((ct == 4) || (ct == 8) || (ct == 17))
	      )
	    )
	{
	founddiff = 0;
        for (i=0; i<8 && (founddiff == 0); i++)
          if (testsimple (pimg, pimanchor, atype, stype, pos, sh[i], sh, nx))
	    {
		if (num_sngb (pimg, pos + sh[i],sh,8) != ngbnum) founddiff = 1; // one simple ngb has different number of ngs's than p - bad
	    }
         if (founddiff == 0) allindep=0;
        }
    }
    return allindep;
}



ERROR_TYPE binOIthin_noqueue (IMAGE *imin, int stype, int atype, IMAGE *imanchor)
     /* anchored graytone order-dependent skeleton */
     /*
           stype == 0 simple g=8, g'=4
                 == 1 simple g=4, g'=8
                 == 2 simple g=4, g'=4  Ronse's {4,8}
                 == 3 simple g=8, g'=4
                 == 4 simple g=4, g'=8
                 == 5 simple g=4, g'=4  Ronse's {4,8}
        atype == 0 anchor pixels are not considered (*imanchor not used)
              == 1 anchor pixels considered (*imanchor used)
     */
{
  long int i,npix,startindoff,endindoff;
  // int kk=0;
  IMAGE *isimple;
  PIX_TYPE *pimin, *pimanchor=NULL, *psimple;
  int nx;
  int deleted = 1;
  int sh[9];
  //int counter = 0;

  int box[6]={2,2,2,2,0,0};

 if (GetImDataType(imin) != t_UCHAR){ (void)sprintf(buf,"wrong input data type !! \n"); errputstr(buf); return ERROR; }

 if (atype == 1) /* check only if anchor image is used */
  {
   if ( (GetImNx(imanchor) != GetImNx(imin)) || (GetImNy(imanchor) != GetImNy(imin)) )
    { (void)sprintf(buf,"input and anchor images have different sizes !! \n"); errputstr(buf); return ERROR; }

   generic_framebox(imanchor,box,0);
   pimanchor = (PIX_TYPE *)GetImPtr(imanchor);
  }
  generic_framebox(imin,box,0);
  nx = GetImNx(imin);
  npix = GetImNPix(imin);
  startindoff = 2*nx + 2 ; /* start offset (skip the first two lines (of enlarged image) and the first point of the second) */
  endindoff = npix - 2*nx - 2 ; /* end offset (finish one pixel before the end of the second line before last */
  pimin = (PIX_TYPE *)GetImPtr(imin);
  isimple = create_image ( GetImDataType(imin), nx , GetImNy(imin) , GetImNz(imin));
  if (isimple == NULL)
    {
      (void)sprintf(buf,"not enough memory for temporary image\n"); errputstr(buf);
      return(ERROR);
    }
  psimple = (PIX_TYPE *)GetImPtr(isimple);

  getngbshift8(sh,nx);
  deleted=1;//kk=0;

  while (deleted)
  {
      deleted = 0; //counter = 0;
       for (i=startindoff; i<endindoff; i++)
        if ((simple_pixel(pimin,stype,i,nx))
           && (test_anchor(pimanchor,atype,i)))
	{if (indep_simple(pimin,pimanchor,stype,atype,i,sh,nx)){*(psimple+i)=1;/*counter++;*/} else *(psimple+i)=0; }
       else *(psimple+i)=0;
       for (i=startindoff; i<endindoff; i++)
	if (*(psimple + i) == 1)
          {
	    if (!simple_pixel(pimin,stype,i,nx)) printf("*");
	    deleted = 1;
            *(pimin+i) = 0;
          }
        // printf("\n k = %d, counter = %d, ", kk++, counter);
   }
  printf ("\n");
  // PS if (atype == 1) subframebox(imanchor,box);
  // PS subframebox(imin,box);
  free_image(isimple);
  return NO_ERROR;
}

ERROR_TYPE binOIthin_FIFO (IMAGE *imin, int stype, int atype, IMAGE *imanchor)
     /* anchored graytone order-dependent skeleton */
     /* queue version
           stype == 0 simple g=8, g'=4
                 == 1 simple g=4, g'=8
                 == 2 simple g=4, g'=4  Ronse's {4,8}
                 == 3 simple g=8, g'=4
                 == 4 simple g=4, g'=8
                 == 5 simple g=4, g'=4  Ronse's {4,8}
        atype == 0 anchor pixels are not considered (*imanchor not used)
              == 1 anchor pixels considered (*imanchor used)
     */
{
  long int i,j,npix,startindoff, endindoff;
  int nx;
  //int kk=0;
  int deleted = 1;
  int sh[9];
  // int counter = 0;
  int qswitch = 0;
  int box[6]={2,2,2,2,0,0};
  IMAGE *isimple;
  PIX_TYPE *pimin, *pimanchor=NULL, *psimple;
  FIFO4 *q_a = create_fifo4(50);
  FIFO4 *q_b = create_fifo4(50);
  FIFO4 *qcurrent, *qnext;
  FIFO4 *q2  = create_fifo4(50);

  if (qswitch){qcurrent=q_a;qnext=q_b;qswitch=0;} else{qcurrent=q_b;qnext=q_a;qswitch=1;}

  if (atype == 1) /* check only if anchor image is used */
  {
   if ( (GetImNx(imanchor) != GetImNx(imin)) || (GetImNy(imanchor) != GetImNy(imin)) )
     { (void)sprintf(buf,"input and anchor images have different sizes !! \n"); errputstr(buf); return ERROR; }
   generic_framebox(imanchor,box,0);
   pimanchor = (PIX_TYPE *)GetImPtr(imanchor);
  }
  generic_framebox(imin,box,0);

  nx = GetImNx(imin);
  npix = GetImNPix(imin);
  startindoff = 2*nx + 2 ; /* start offset (skip the first two lines (of enlarged image) and the first point of the second) */
  endindoff = npix - 2*nx - 2 ; /* end offset (finish one pixel before the end of the second line before last */
  pimin = (PIX_TYPE *)GetImPtr(imin);

  isimple = create_image ( GetImDataType(imin), nx , GetImNy(imin) , GetImNz(imin));
  if (isimple == NULL){(void)sprintf(buf,"not enough memory for temporary image\n"); errputstr(buf); return(ERROR);}
  psimple = (PIX_TYPE *)GetImPtr(isimple);

  getngbshift8(sh,nx);
  for (i=startindoff; i<endindoff; i++)
    if ((simple_pixel(pimin,stype,i,nx)) && (test_anchor(pimanchor,atype,i)&& (indep_simple(pimin,pimanchor,stype,atype,i,sh,nx))  ))
        {*(psimple+i)=1; fifo4_add(qcurrent,(long int) i);/*counter++;*/} else *(psimple+i)=0;
  deleted=1;//kk=0;
  while (deleted)
  {
      //counter = 0;
      deleted = 0;
      while (fifo4_empty(qcurrent) == FALSE)
       {
	 i = fifo4_remove(qcurrent);
	 // if (simple_pixel(pimin,stype,i,nx) == 0) printf("*");   /* Just for test, '*' should NEVER appear !!*/
	 deleted = 1; // counter ++;
         *(psimple+i) = 0;
         *(pimin+i) = 0;
         fifo4_add(q2, (long int) i);
       }
      while (fifo4_empty(q2) == FALSE)
      {
       i = fifo4_remove(q2);
       for (j=0; j<8; j++)
	 if ((test_anchor(pimanchor,atype,i + sh[j])) && (simple_pixel(pimin,stype,i + sh[j],nx))
	     && (*(psimple + i + sh[j]) == 0) && (indep_simple(pimin,pimanchor,stype,atype,i + sh[j],sh,nx)) )
                  {*(psimple+i+sh[j]) = 1; fifo4_add(qnext,(long int) (i+sh[j]));}
      }
      // printf("\n k = %d, counter = %d", kk++, counter);
      if (qswitch){qcurrent=q_a;qnext=q_b;qswitch=0;} else{qcurrent=q_b;qnext=q_a;qswitch=1;}
  }
  // PS if (atype == 1) subframebox(imanchor,box);
  // PS subframebox(imin,box);
  free_image(isimple);    free_fifo4(q_a);   free_fifo4(q_b);   free_fifo4(q2);
  return NO_ERROR;
}
#include "uc_undef.h"


/*@}*/
