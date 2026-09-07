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
** Header file for image analysis routines
** by Pierre Soille@ec.europa.eu  1988--2016
*/

#ifndef _MIAL_GLUE_H_
#define _MIAL_GLUE_H_

#ifdef PYTHON
#include <Python.h>
#define printf PySys_WriteStdout
#endif


#ifndef  INT_LIIAR
#define  INT_LIIAR


#include "mialtypes.h"

/* */

#define ERROR      1
#define NO_ERROR   0
#define	TRUE	   1
#define	FALSE	   0
#ifndef PI
#define PI         3.14159265358979323846 /* same as in XLISP-PLUS */
#endif


/* adjust to local system */
#define BITPERCHAR   8
#define BITPERSHORT  16
#define BITPERINT32  32
#define BITPERINT64  64
#define BITPERFLOAT  32
#define BITPERDOUBLE 64
#define BITPERWORD   (sizeof(long int)*8)
#define BYTEPERWORD  sizeof(long int)
#define BITPERBAND   8
#if (__WORDSIZE==32)
#define W_MSB        0x80000000
#elif (__WORDSIZE==64)
#define W_MSB        0x8000000000000000
#endif

#ifndef UCHAR_MIN
#define UCHAR_MIN    0
#endif
#ifndef UCHAR_MAX
#define UCHAR_MAX    0xFF
#endif
#ifndef SHORT_MIN
#define SHORT_MIN    -0x8000
#endif
#ifndef SHORT_MAX
#define SHORT_MAX    0x7FFF
#endif
#ifndef INT_MIN
#define INT_MIN      -0x80000000
#endif
#ifndef INT_MAX
#define INT_MAX      0x7FFFFFFF
#endif
#ifndef USHORT_MIN
#define USHORT_MIN   0
#endif
#ifndef USHORT_MAX
#define USHORT_MAX   0xFFFF
#endif
#ifndef INT32_MIN
#define INT32_MIN    -0x80000000
#endif
#ifndef INT32_MAX
#define	INT32_MAX    0x7FFFFFFF
#endif
#ifndef UINT32_MIN
#define UINT32_MIN   0
#endif
#ifndef UINT32_MAX
#define UINT32_MAX   0xFFFFFFFF
#endif
#ifndef INT64_MIN
#define INT64_MIN    -0x8000000000000000
#endif
#ifndef INT64_MAX
#define	INT64_MAX    0x7FFFFFFFFFFFFFFF
#endif
#ifndef UINT64_MIN
#define UINT64_MIN   0
#endif
#ifndef UINT64_MAX
#define UINT64_MAX   0xFFFFFFFFFFFFFFFF
#endif
#define MIALFLOAT_MAX    ((float)3.40282346638528860e+38)
#define MIALFLOAT_MIN    ((float)1.40129846432481707e-45)
#ifndef DOUBLE_MAX
#define DOUBLE_MAX   1.797693134862315708e+308
#endif
#ifndef DOUBLE_MIN
#define DOUBLE_MIN   4.94065645841246544e-324
#endif


/* pixel type table */
#define t_TIFFONEBITPERPIXEL 12
#define t_ONEBITPERPIXEL      0
#define t_FOURBITPERPIXEL     1
#define t_CHAR                2  /* all should be in the form e.g. uc_TYPE */
#define t_UCHAR               3
#define t_SHORT               4
#define t_USHORT              5
#define t_INT32               6
#define t_UINT32              7
#define t_INT64               8
#define t_UINT64              9
#define t_FLOAT              10
#define t_MIALFLOAT           10
#define t_DOUBLE             11
#define t_RGB                13 /* fake type, requires pixel data type=t_UCHAR */
#define t_PTR                14
#define t_UNSUPPORTED       255

#define t_LBL_TYPE            6 /* consider unsigned long int in the future*/
#define LBL_MAX       INT32_MAX

#define t_GENERIC             3
#define t_HST1D               6
#define t_HST2D               3 /* different types should be allowed, 3 for ACCA */
#define t_HST3D               6
#define t_LUT                10
#define t_LABEL               6


/* macro definitions for grilisp (getset.h) */
#define GetImDataType(x)      ((x)->DataType)  /* data type of a pixel */
#define GetImPtr(x)           ((x)->p_im)      /* pointer to the first pixel */
#define GetImNx(x)            ((unsigned long int)(x)->nx)
#define GetImNy(x)            ((unsigned long int)(x)->ny)
#define GetImNz(x)            ((unsigned long int)(x)->nz)
#define GetImNlin(x)          ((unsigned long int)(x)->nz * (x)->ny)
#define GetImNPix(x)          ((unsigned long int)(x)->nx * (x)->ny * (x)->nz)
#define GetImNPixPerPlane(x)  (((unsigned long int)(x)->nx) * (x)->ny)
#define GetImNByte(x)         ((unsigned long int)(x)->NByte)
#define GetImCenter(x)        ((x)->center)
#define GetImVol(x)           ((x)->vol) /* must call volume() before */
#define GetImLut(x)           ((x)->lut)


#define GetImMin(x, val) \
        switch(GetImDataType(x)){ \
	 case t_UCHAR: \
	   (val) = (x)->umin.uc_val; \
	 break; \
	 case t_USHORT: \
	   (val) = (x)->umin.us_val; \
	 break; \
	 case t_SHORT: \
	   (val) = (x)->umin.s_val; \
	 break; \
	 case t_INT32: \
	   (val) = (x)->umin.i32_val; \
	 break; \
	 case t_UINT32: \
	   (val) = (x)->umin.u32_val; \
	 break; \
	 case t_INT64: \
	   (val) = (x)->umin.i64_val; \
	 break; \
	 case t_UINT64: \
	   (val) = (x)->umin.u64_val; \
	 break; \
	 case t_FLOAT: \
	   (val) = (x)->umin.f_val; \
	 break; \
	 case t_DOUBLE: \
	   (val)= (x)->umin.d_val; \
	 break; \
	 default: \
	   (void)fprintf(stderr,"WARNING: \
           invalid data type in GetImMin \
           macro\n"); \
	  }
#define GetImMax(x, val) \
        switch(GetImDataType(x)){ \
	 case t_UCHAR: \
	   (val) = (x)->umax.uc_val; \
	 break; \
	 case t_USHORT: \
	   (val) = (x)->umax.us_val; \
	 break; \
	 case t_SHORT: \
	   (val) = (x)->umax.s_val; \
	 break; \
	 case t_INT32: \
	   (val) = (x)->umax.i32_val; \
	 break; \
	 case t_UINT32: \
	   (val) = (x)->umax.u32_val; \
	 break; \
	 case t_INT64: \
	   (val) = (x)->umax.i64_val; \
	 break; \
	 case t_UINT64: \
	   (val) = (x)->umax.u64_val; \
	 break; \
	 case t_FLOAT: \
	   (val) = (x)->umax.f_val; \
	 break; \
	 case t_DOUBLE: \
	   (val)= (x)->umax.d_val; \
	 break; \
	 default: \
	   (void)fprintf(stderr,"WARNING: \
           invalid data type in GetImMax \
           macro\n"); \
	 }

#define SetImDataType(x, val) ((x)->DataType = (val))
#define SetImPtr(x, val)      ((x)->p_im = (void *)(val))
#define SetImNByte(x, val)    ((x)->NByte = (val))
#define SetImNx(x, val)       ((x)->nx = (val))
#define SetImNy(x, val)       ((x)->ny = (val))
#define SetImNz(x, val)       ((x)->nz = (val))
#define SetImCenter(x, val)   ((x)->center = (val))
#define SetImVol(x,val)       ((x)->vol = (VOL_TYPE)(val))
#define SetImLut(x, val) ((x)->lut = (val))

#define SetImMin(x, val) \
        switch(GetImDataType(x)){ \
	 case t_UCHAR: \
	   (x)->umin.uc_val = (UCHAR)(val); \
	 break; \
	 case t_USHORT: \
	   (x)->umin.us_val = (USHORT)(val); \
	 break; \
	 case t_SHORT: \
	   (x)->umin.s_val = (SHORT)(val); \
	 break; \
	 case t_INT32: \
	   (x)->umin.i32_val = (INT32)(val); \
	 break; \
	 case t_UINT32: \
	   (x)->umin.u32_val = (UINT32)(val); \
	 break; \
	 case t_INT64: \
	   (x)->umin.i64_val = (INT64)(val); \
	 break; \
	 case t_UINT32: \
	   (x)->umin.u64_val = (UINT64)(val); \
	 break; \
	 case t_FLOAT: \
	   (x)->umin.f_val = (float)(val); \
	 break; \
	 case t_DOUBLE: \
	   (x)->umin.d_val = (double)(val); \
	 break; \
	 default: \
	   (void)fprintf(stderr,"WARNING: \
           invalid data type in SetImMin \
           macro\n"); \
			       }
#define SetImMax(x, val) \
        switch(GetImDataType(x)){ \
	 case t_UCHAR: \
	   (x)->umax.uc_val = (UCHAR)(val); \
	 break; \
	 case t_USHORT: \
	   (x)->umax.us_val = (USHORT)(val); \
	 break; \
	 case t_SHORT: \
	   (x)->umax.s_val = (SHORT)(val); \
	 break; \
	 case t_INT32: \
	   (x)->umax.i32_val = (INT32)(val); \
	 break; \
	 case t_UINT32: \
	   (x)->umax.u32_val = (UINT32)(val); \
	 break; \
	 case t_INT64: \
	   (x)->umax.i64_val = (INT64)(val); \
	 break; \
	 case t_UINT64: \
	   (x)->umax.u64_val = (UINT64)(val); \
	 break; \
	 case t_FLOAT: \
	   (x)->umax.f_val = (float)(val); \
	 break; \
	 case t_DOUBLE: \
	   (x)->umax.d_val = (double)(val); \
	 break; \
	 default: \
	   (void)fprintf(stderr,"WARNING: \
           invalid data type in SetImMax \
           macro\n"); \
			       }



/*
  external function declarations
*/


#endif /* #ifndef INT_LIIAR */



#endif
