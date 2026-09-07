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

#ifndef _MIATYPES_H
#define _MIATYPES_H       1


/* type definitions */
/* consider using or referring to stdint.h in the future*/
typedef unsigned long int mia_size_t;
typedef unsigned char      DST_TYPE;
typedef unsigned char      GENERICPIX;
typedef unsigned char      RGB_TYPE;
typedef int                HST1D_TYPE;
typedef unsigned char      HST2D_TYPE;
typedef int                HST3D_TYPE;
typedef float              LUT_TYPE;
typedef char               CHAR;
typedef unsigned char      UCHAR;
typedef int                INT;
typedef int                INT32;
typedef unsigned int       UINT32;
typedef long long int      INT64;
typedef unsigned long long int UINT64;
typedef short int          SHORT;
typedef unsigned short int USHORT;
typedef float              MIALFLOAT;  /* FLOAT defined as double in windows ... */
typedef double             DOUBLE;
typedef void *             NULL_TYPE;
typedef int                ERROR_TYPE;
typedef double             VOL_TYPE;
typedef void *             PTR_TYPE;
typedef int                LBL_TYPE; /* should be unsigned, see also t_LBL_TYPE */

/* consider defining long long type (8 bytes) on 486: int64 */
typedef struct {
  GENERICPIX generic_val;
  UCHAR  uc_val;
  USHORT us_val;
  SHORT  s_val;
  UINT32 u32_val;
  INT32  i32_val;
  UINT64 u64_val;
  INT64  i64_val;
  MIALFLOAT  f_val;
  DOUBLE d_val;
} G_TYPE;

/* consider projectioncode ulx uly ulz resx resy resz */
typedef struct {
  void *p_im;    /* Pointer to image data */
  int DataType;  /* Image data type */
  int nx;        /* Number of columns */
  int ny;        /* Number of lines */
  int nz;        /* Number of x-y planes */
  mia_size_t NByte;  /* Number of bytes for image data */
  VOL_TYPE vol;  /* Sum of pixel values (volume) */
  USHORT *lut;   /* Pointer to colour map */
  G_TYPE g;      /* used to store a value matching DataType (see e.g. alphacc.c) */
} IMAGE;

typedef struct { /* structure for blob analysis */
  int area;     /* number of pixels */
  int m00;      /* moments of order p-q */
  int m10;
  int m01;
  int minor; /* length of minor axis */
  int major; /* length of major axis */
  int irradiance; /* mean value * 255 */
  double xcg;   /* x coordinate of gravity centre */
  double ycg;   /* y coordinate of gravity centre */
  double mu11;  /* centred moments of order p-q */
  double mu20;
  double mu02;
  double ratio; /* ratio between minor and major axis */
  UCHAR  dir;   /* dir=360/(4pi) arctan(2mu11/(mu20-mu02)) */
  double r;      /* correlation coefficient */
} IMBLOB;

#endif /* mialtypes.h */
