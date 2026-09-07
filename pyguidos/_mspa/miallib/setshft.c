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
#include "miallib.h"


/** @addtogroup group_geom
 *  @{
 */


ERROR_TYPE szcompat(IMAGE *im1, IMAGE *im2)
{
  if (GetImNx(im1) != GetImNx(im2))
    return(ERROR);
  else if (GetImNy(im1) != GetImNy(im2))
    return(ERROR);
  else if (GetImNz(im1) != GetImNz(im2))
    return(ERROR);
  else if (GetImDataType(im1) != GetImDataType(im2))
    return(ERROR);
  else if ((GetImPtr(im1) == NULL) || (GetImPtr(im2) == NULL))
    return(ERROR);

  return(NO_ERROR);
}

ERROR_TYPE szgeocompat(IMAGE *im1, IMAGE *im2)
{
  if (GetImNx(im1) != GetImNx(im2))
    return(ERROR);
  else if (GetImNy(im1) != GetImNy(im2))
    return(ERROR);
  else if (GetImNz(im1) != GetImNz(im2))
    return(ERROR);
  else if ((GetImPtr(im1) == NULL) || (GetImPtr(im2) == NULL))
    return(ERROR);

  return(NO_ERROR);
}


/* 2007 03 08 */
ERROR_TYPE set_shift(long int nx, long int ny, long int nz, long int graph, long int *shift)
{
  if (graph == 4){      /* && nz == 1: suppressed on 2004-11-18 (side effects?) */
    shift[0] = -nx;
    shift[1] = -1;
    shift[2] = nx;
    shift[3] = 1;
  }
  else if (graph == 8){ /* && nz == 1: suppressed on 2004-11-18 (side effects?) */
    shift[4] = -nx-1;		shift[0] = -nx;  shift[5] = -nx + 1;
    shift[1] = -1;                               shift[3] = 1;	  /* swapped 3 and 2 on 2007-9-19 */
    shift[6] = nx-1;		shift[2] = nx;   shift[7] = nx + 1;

  }
  else if (graph == 6 && nz > 1){
    shift[0] = -nx;  shift[1] = -1;
    shift[2] = -(nx * ny);
    shift[3] = nx;   shift[4] = 1;
    shift[5] = -shift[2];
   }
  else{
    (void) sprintf(buf, "set_seq_shift(): invalid parameters\n"); errputstr(buf);
    return ERROR;
  }
  return NO_ERROR;
}



ERROR_TYPE set_seq_shift(long int nx, long int ny, long int nz, long int graph, long int *shift)
{
  if (graph == 2){      /* added 20110310 for shadow ray processing */
    shift[0] = -1;
    shift[1] = 1;
  }
  else if (graph == 4){      /* && nz == 1: suppressed on 2004-11-18 (side effects?) */
    shift[0] = -nx;
    shift[1] = -1;
    shift[2] = nx;
    shift[3] = 1;
  }
  else if (graph == 8){ /* && nz == 1: suppressed on 2004-11-18 (side effects?) */
    shift[0] = -nx;		shift[1] = -1;
    shift[2] = -nx + 1;	shift[3] = -nx - 1;
    shift[4] = 1;		shift[5] = nx;
    shift[6] = nx - 1;		shift[7] = nx + 1;
  }
  else if (graph == 6 && nz > 1){
    shift[0] = -nx;  shift[1] = -1;
    shift[2] = -(nx * ny);
    shift[3] = nx;   shift[4] = 1;
    shift[5] = -shift[2];
   }
  else if (graph == 18 && nz > 1){
    shift[0] = -nx;   shift[1] = -1;
    shift[2] = shift[0] + 1;         shift[3] = shift[0] - 1;
    shift[4] = -(nx * ny);
    shift[5] = shift[4] + shift[0];   shift[6] = shift[4] - 1;
    shift[7] = shift[4] + 1;         shift[8] = shift[4] - shift[0];
    shift[9] = -shift[0];	           shift[10] = 1;
    shift[11] = -shift[2];           shift[12] = -shift[3];
    shift[13] = -shift[4];           shift[14] = -shift[5];
    shift[15] = -shift[6];           shift[16] = -shift[7];
    shift[17] = -shift[8];
  }
  else if (graph == 26 && nz > 1){
    shift[0] = -nx;  shift[1] = -1;
    shift[2] = shift[0] + 1;        shift[3] = shift[0] - 1;
    shift[4] = -(nx * ny);
    shift[5] = shift[4] + shift[0];  shift[6] = shift[4] - 1;
    shift[7] = shift[4] + 1;        shift[8] = shift[4] - shift[0];
    shift[9] = shift[5] - 1;        shift[10] = shift[5] + 1;
    shift[11] = shift[8] - 1;       shift[12] = shift[8] + 1;
    shift[13] = -shift[0];          shift[14] = -shift[1];
    shift[15] = -shift[2];          shift[16] = -shift[3];
    shift[17] = -shift[4];          shift[18] = -shift[5];
    shift[19] = -shift[6];          shift[20] = -shift[7];
    shift[21] = -shift[8];          shift[22] = -shift[9];
    shift[23] = -shift[10];         shift[24] = -shift[11];
    shift[25] = -shift[12];
  }
  else{
    (void) sprintf(buf, "set_seq_shift(): invalid parameters\n"); errputstr(buf);
    return ERROR;
  }
  return NO_ERROR;
}


ERROR_TYPE setinvseqshift(long int nx, long int ny, long int nz, long int graph, long int *shft)
{
  if (graph == 4 && nz == 1){
    shft[2] = -nx;    shft[3] = -1;
    shft[0] = nx;      shft[1] = 1;
  }
  else if (graph == 8 && nz == 1){
    shft[5] = -nx;      shft[4] = -1;
    shft[6] = -nx + 1;  shft[7] = -nx - 1;
    shft[1] = 1;        shft[0] = nx;
    shft[2] = nx - 1;   shft[3] = nx + 1;
  }
  else if (graph == 6 && nz > 1){
    shft[3] = -nx;  shft[4] = -1;
    shft[2] = nx * ny;
    shft[0] = nx;   shft[1] = 1;
    shft[5] = -shft[2];
  }
  else if (graph == 18 && nz > 1){
    shft[9] = -nx;    shft[10] = -1;
    shft[11] = shft[9] + 1;         shft[12] = shft[9] - 1;
    shft[13] = -(nx * ny);
    shft[14] = shft[13] + shft[9];  shft[15] = shft[13] - 1;
    shft[16] = shft[13] + 1;        shft[17] = shft[13] - shft[9];
    shft[0] = -shft[9];              shft[1] = 1;
    shft[2] = -shft[11];            shft[3] = -shft[12];
    shft[4] = -shft[13];            shft[5] = -shft[14];
    shft[6] = -shft[15];            shft[7] = -shft[16];
    shft[8] = -shft[17];
  }
  else if (graph == 26 && nz > 1){
    shft[13] = -nx;  shft[14] = -1;
    shft[15] = shft[13] + 1;        shft[16] = shft[13] - 1;
    shft[17] = -(nx * ny);
    shft[18] = shft[17] + shft[13];  shft[19] = shft[17] - 1;
    shft[20] = shft[17] + 1;        shft[21] = shft[17] - shft[13];
    shft[22] = shft[18] - 1;        shft[23] = shft[18] + 1;
    shft[24] = shft[21] - 1;       shft[25] = shft[21] + 1;
    shft[0]  = -shft[13];          shft[1] = -shft[14];
    shft[2]  = -shft[15];          shft[3] = -shft[16];
    shft[4]  = -shft[17];          shft[5] = -shft[18];
    shft[6]  = -shft[19];          shft[7] = -shft[20];
    shft[8]  = -shft[21];          shft[9] = -shft[22];
    shft[10] = -shft[23];         shft[11] = -shft[24];
    shft[12] = -shft[25];
  }
  else
    return ERROR;

  return NO_ERROR;
}

void set_shift_and_box(unsigned char *im1, int *box, long int x, long int y, long int *shift)
{
  long int i, j, k;
  long int cnt = 0;
  long int nx, ny, nz, ox, oy, oz;

  nx = box[0];
  ny = box[1];
  nz = box[2];
  ox = box[3];
  oy = box[4];
  oz = box[5];

  for (i = 0; i < nz; i++){
    for (j = 0; j < ny; j++){
      for (k = 0; k < nx; k++){
	if (*im1){
	  shift[cnt] = k - ox + x * (j - oy) + x * y * (i - oz);
          cnt++;
	}
        im1++;
      }
    }
  }

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
}


void set_shift_and_box_and_weight(unsigned char *im1, MIALFLOAT *im2, int *box, long int x, long int y, long int *shift, MIALFLOAT *weight)
{
  long int i, j, k;
  long int cnt = 0;
  long int nx, ny, nz, ox, oy, oz;

  nx = box[0];
  ny = box[1];
  nz = box[2];
  ox = box[3];
  oy = box[4];
  oz = box[5];

  for (i = 0; i < nz; i++){
    for (j = 0; j < ny; j++){
      for (k = 0; k < nx; k++){
	if (*im1){
	  shift[cnt] = k - ox + x * (j - oy) + x * y * (i - oz);
	  weight[cnt] = *im2;
          cnt++;
        }
        im1++; im2++;
      }
    }
  }

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
}


void uc_set_shift_and_box_and_weight(unsigned char *im1, UCHAR *im2, int *box, long int x, long int y, long int *shift, UCHAR *weight)
{
  long int i, j, k;
  long int cnt = 0;
  long int nx, ny, nz, ox, oy, oz;

  nx = box[0];
  ny = box[1];
  nz = box[2];
  ox = box[3];
  oy = box[4];
  oz = box[5];

  for (i = 0; i < nz; i++){
    for (j = 0; j < ny; j++){
      for (k = 0; k < nx; k++){
	if (*im1){
	  shift[cnt] = k - ox + x * (j - oy) + x * y * (i - oz);
	  weight[cnt] = *im2;
          cnt++;
        }
        im1++; im2++;
      }
    }
  }

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
}



void transpose(shift, n)
long int *shift;
long int n;
{
  long int i;

  for (i = 0; i < n; i++)
    shift[i] *= -1;
}


long int objectpix(IMAGE *im)
{
  long int i, n=0;
  long int npix = GetImNPix(im);
  UCHAR *p = (UCHAR *)GetImPtr(im);

  if (GetImDataType(im)!=t_UCHAR){
    (void)sprintf(buf,"objectpix(im): im must be of type uint8 !\n"); errputstr(buf);
    return 0;
  }

  for (i=0; i<npix; i++)
    if (p[i])
      n++;
  return(n);
}

long int get_offset_first_pixel(long int nx, long int ny, long int nz, int graph)
{
  switch (graph){
  case 2:
    return 1;
    break;
  case 4:
  case 8:
    return nx+1;
    break;
  case 6:
  case 18:
  case 27:
    return nx*ny+nx+1;
    break;
  default:
    (void)sprintf(buf,"get_offset_first_pixel(long int nx, long int ny, long int nz, int graph=%d): invalid graph value, must be either 2, 4, 8, 6, 18, or 27\n", graph); errputstr(buf);
    return nx*ny*nz;  /* no pixel can be processed */
  }
}

long int get_offset_last_pixel(long int nx, long int ny, long int nz, int graph)
{
  switch (graph){
  case 2:
    return ny*nx-1;
    break;
  case 4:
  case 8:
    return nx*ny-nx-2;
    break;
  case 6:
  case 18:
  case 27:
    return nx*ny*nz-(nx*ny)-nx-2;
    break;
  default:
    (void)sprintf(buf,"get_offset_first_pixel(long int nx, long int ny, long int nz, int graph=%d): invalid graph value, must be either 2, 4, 8, 6, 18, or 27\n", graph); errputstr(buf);
    return -1;  /* no pixel can be processed */
  }
}

/*@}*/
