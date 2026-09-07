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
** by Pierre Soille@ec.europa.eu  1988--2018
*/

#ifndef _MIALLIB_H
#define _MIALLIB_H       1


#ifdef PYTHON
#include <Python.h>
#define printf PySys_WriteStdout
#endif

#include <limits.h>
/* pyGuidos: <unistd.h> is POSIX-only and absent on MSVC/Windows.
   Guard it so the vendored MSPA subset compiles cross-platform. */
#if !defined(_MSC_VER)
#include <unistd.h>
#endif

#include "mialtypes.h"
#include "config_miallib.h"


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

#if _WIN32 || _WIN64
#if _WIN64
#define __WORDSIZE 64
#else
#define __WORDSIZE 32
#endif
#endif

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

/* */

#define ERROR      1
#define NO_ERROR   0
#define	TRUE	   1
#define	FALSE	   0
#ifndef PI
#define PI         3.14159265358979323846 /* same as in XLISP-PLUS */
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
#define PLANARCONFIG_BIL      3 /* bil model (band interleaved by line) not supported by TIFF */

/* constants */
#define BOXELEM 6  /* length of a box array */

/* useful macro definitions */
#define SGN(x)       (((x) < 0) ? -1 : (((x) > 0) ? 1 : 0))
#define	MYSWAP(a, b, t)	( (t) = (a), (a) = (b), (b) = (t) )
#ifndef MAX
#define MAX(x, y)	(((x) < (y)) ? (y) : (x))
#endif
#ifndef MIN
#define MIN(x, y)	(((x) < (y)) ? (x) : (y))
#endif
#define	LOOPDN(r, n)		for ( (r) = (n)+1; --(r) > 0; )
#define ERR_MSG(b)     (void)fprintf(stderr,(b))
#define	SQ(x)		((x) * (x))

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

/* other image macro definitions */
#define IsImInteger(x)  (GetImDataType(x)!=10 || GetImDataType(x)!=11)


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

/* macro definitions for framebox */
#define BOX_1D box[0]=box[1]=1; box[2]=box[3]=box[4]=box[5]=0
#define BOX_2D box[0]=box[1]=box[2]=box[3]=1; box[4]=box[5]=0
#define BOX_3D box[0]=box[1]=box[2]=box[3]=box[4]=box[5]=1


/* FOR XLISP ONLY */
#if !(defined(NO_BUF))
#if (defined(XLISP))
extern char buf[];       /* used by sprintf() and stdputstr() */
extern void stdputstr(char *); /* print a message to the standard output */
extern void errputstr(char *); /* print a message to the standard error  */
#else
static char buf[1024];       /* used by sprintf() and stdputstr() */
extern void stdputstr(char *); /* print a message to the standard output */
extern void errputstr(char *); /* print a message to the standard error  */
#endif /* #if (defined(XLISP)) */
#endif


#define NO_uc_IMAGE
#define NO_s_IMAGE
#define NO_us_IMAGE
#define NO_i32_IMAGE
#define NO_u32_IMAGE
#define NO_i64_IMAGE
#define NO_u64_IMAGE
#define NO_f_IMAGE
#define NO_d_IMAGE


/*
  external function declarations
*/

/* imem.c */
extern int GetImBitPerPixel(IMAGE *im);
extern void free_image(IMAGE *);
extern void free_lut(IMAGE *im);
extern ERROR_TYPE iminfo(IMAGE *);
extern IMAGE *create_image(int data_type, long int nx, int ny, int nz);
extern IMAGE *copy_image(IMAGE *im);
extern ERROR_TYPE copy_lut(IMAGE *im1, IMAGE *im2);
extern ERROR_TYPE create_lut(IMAGE *im);
extern IMAGE **create_imarray(int);
extern ERROR_TYPE dumpxyz(IMAGE *im, int x, int y, int z, int dx, int dy);


/* imio_gdal.c */
extern int GDAL2MIALDataType(int aGDALDataType);
extern IMAGE *GDALInfoJIP(char *imfn);
extern IMAGE *GDALRead(char *imfn, int band, int nXOff, int nYOff, int nXSize, int nYSize, int nBufXSize, int nBufYSize);

/* imio.c */
extern IMAGE *read_all(char *fn, int nx, int ny, int nz, int data_type, int header_size, int pc);
extern IMAGE *read_image(char *fn);
extern IMAGE *read_image_to_type(char *fn, int data_type);
extern ERROR_TYPE read_image_data(FILE *fp, IMAGE *im, int pc);
extern ERROR_TYPE write_image_data(FILE *fp, IMAGE *im, int pc);
extern ERROR_TYPE write_ColorMap_tiff(IMAGE *im, char *fn);
extern ERROR_TYPE write_tiff(IMAGE *im, char *fn);
extern ERROR_TYPE writeTiffOneStripPerLine(IMAGE *im, char *fn, char *desc);

/* imio2.c */
extern IMAGE *GetGeoKey(char *fname, char *keyname);
extern IMAGE *GetTIFFTagGeo(char *fn, char *tagname);
extern IMAGE *readTiffSubset(char *fn, int x, int y, unsigned szx, unsigned szy);
extern ERROR_TYPE tiffinfo(char *fn, char *field, float *val);
extern IMAGE *tiffinfoJIP(char *fn);
extern ERROR_TYPE read_image_data2(FILE *fp, IMAGE *im, int x, int y, int inx, int scale);
extern ERROR_TYPE writeGeoTiffOneStripPerLine(IMAGE *im, char *fn, int PCSCode, double xoff, double yoff, double scale, unsigned short RasterType, int nodata_flag, int nodata_val, int metadata_flag, char *metadata_str);
extern ERROR_TYPE writeMBGeoTiffOneStripPerLine(IMAGE **imap, int nc, char *fn, int PCSCode, double xoff, double yoff, double scale, unsigned short RasterType, int nodata_flag, int nodata_val, int metadata_flag, char *metadata_str);
extern void print_mia_banner();


/* imstat.c */
extern IMAGE *histo1d(IMAGE *im);
extern IMAGE *histo2d(IMAGE *im1, IMAGE *im2);
extern IMAGE *histo3d(IMAGE *im1, IMAGE *im2, IMAGE *im3);
extern IMAGE *rsum(IMAGE *im);
extern IMAGE *class2d(IMAGE *im1, IMAGE *im2, IMAGE *imlut);
extern IMAGE *area(IMAGE*, int, int);
extern ERROR_TYPE lookup(IMAGE *, IMAGE *);
extern ERROR_TYPE lookuptypematch(IMAGE *, IMAGE *);
extern IMAGE *lookuprgb(IMAGE *, IMAGE *, IMAGE *, IMAGE *);
extern ERROR_TYPE imequalp(IMAGE *, IMAGE *);
extern ERROR_TYPE volume(IMAGE *);
extern G_TYPE *min_max(IMAGE *im);
extern ERROR_TYPE getfirstmaxpos(IMAGE *, unsigned long int *);
extern ERROR_TYPE getmax(IMAGE *im, double *maxval);
extern ERROR_TYPE getminmax(IMAGE *im, double *minval, double *maxval);


extern ERROR_TYPE generic_min_max(IMAGE *im, G_TYPE *pg);
extern ERROR_TYPE s_min_max(IMAGE *im, G_TYPE *pg);
extern ERROR_TYPE u32_min_max(IMAGE *im, G_TYPE *pg);
extern ERROR_TYPE i64_min_max(IMAGE *im, G_TYPE *pg);
extern ERROR_TYPE u64_min_max(IMAGE *im, G_TYPE *pg);
extern ERROR_TYPE f_min_max(IMAGE *im, G_TYPE *pg);
extern ERROR_TYPE d_min_max(IMAGE *im, G_TYPE *pg);
extern ERROR_TYPE us_min_max(IMAGE *im, G_TYPE *pg);
extern ERROR_TYPE i32_min_max(IMAGE *im, G_TYPE *pg);



/* pointop.c */
extern ERROR_TYPE bitwise_op(IMAGE *im1, IMAGE *im2, int op);
extern ERROR_TYPE negation(IMAGE *im);
extern ERROR_TYPE arith(IMAGE *, IMAGE *, int);
extern ERROR_TYPE arithcst(IMAGE *, G_TYPE, int);
extern ERROR_TYPE imabs(IMAGE *), imsqrt(IMAGE *), imlog(IMAGE *), power2p(IMAGE *);
extern ERROR_TYPE imatan(IMAGE *), imacos(IMAGE *), imasin(IMAGE *);
extern ERROR_TYPE imatan(IMAGE *), imcos(IMAGE *), imsin(IMAGE *);
extern ERROR_TYPE thresh(IMAGE *, G_TYPE, G_TYPE, G_TYPE, G_TYPE);
extern ERROR_TYPE setlevel(IMAGE *im, G_TYPE gt1, G_TYPE gt2, G_TYPE gval);
extern ERROR_TYPE modulo(IMAGE *im, int val);
extern ERROR_TYPE complement(IMAGE *im);
extern ERROR_TYPE blank(IMAGE *im, G_TYPE gval);
extern ERROR_TYPE shift(IMAGE *im, int val);
extern ERROR_TYPE setrange(IMAGE *im, G_TYPE gt1, G_TYPE gt2);
extern ERROR_TYPE FindPixWithVal(IMAGE *, G_TYPE, unsigned long int *);
extern ERROR_TYPE IsPartitionEqual(IMAGE *, IMAGE *, int *);
extern ERROR_TYPE swap(IMAGE *im);

extern ERROR_TYPE i32_arithcst(IMAGE *, INT32, int);
extern ERROR_TYPE f_arithcst(IMAGE *, MIALFLOAT, int);
extern ERROR_TYPE us_blank(IMAGE *, USHORT);
extern ERROR_TYPE i32_blank(IMAGE *, INT32);
extern ERROR_TYPE u32_blank(IMAGE *, UINT32);
extern ERROR_TYPE f_blank(IMAGE *, MIALFLOAT);
extern ERROR_TYPE generic_blank(IMAGE *, UCHAR);
extern ERROR_TYPE i32_blank(IMAGE *, INT32);
extern ERROR_TYPE generic_setlevel(IMAGE *im, GENERICPIX t1, GENERICPIX t2, GENERICPIX val);
extern ERROR_TYPE us_setlevel(IMAGE *im, USHORT t1, USHORT t2, USHORT val);
extern ERROR_TYPE s_setlevel(IMAGE *im, SHORT t1, SHORT t2, SHORT val);
extern ERROR_TYPE us_setrange(IMAGE *im, USHORT t1, USHORT t2);
extern ERROR_TYPE generic_thresh(IMAGE *, UCHAR, UCHAR, UCHAR, UCHAR);
extern ERROR_TYPE f_thresh(IMAGE *, MIALFLOAT, MIALFLOAT, MIALFLOAT, MIALFLOAT);

/* efedt.c */
extern IMAGE *sqedt(IMAGE *);
extern IMAGE *iz(IMAGE *);


/* format.c */
extern ERROR_TYPE to_uchar(IMAGE *);
extern IMAGE *to_ushort(IMAGE *), *to_int32(IMAGE *), *to_float(IMAGE *), *to_double(IMAGE *) ;
extern ERROR_TYPE dbltofloat(IMAGE *), uint32_to_float(IMAGE *);

extern IMAGE *to_tiff1bitpp(IMAGE *im);
extern IMAGE *to_tiff4bitpp(IMAGE *im);


/* geom.c */
extern ERROR_TYPE framebox(IMAGE *im, int *box, G_TYPE gval);
extern ERROR_TYPE addframebox(IMAGE *im, int *box, G_TYPE gval);
extern ERROR_TYPE subframebox(IMAGE *im, int *box);
extern IMAGE *getframebox(IMAGE *im, int *box);
extern ERROR_TYPE setframebox(IMAGE *im, IMAGE *imframe, int *box);
extern ERROR_TYPE imputop(IMAGE *im1, IMAGE *im2, int x, int y, int z, int op);
extern ERROR_TYPE imputcompose(IMAGE *im1, IMAGE *imlbl, IMAGE *im2, int x, int y, int z, int val);
extern IMAGE *imcut(IMAGE *im, int x1, int y1, int z1, int x2, int y2, int z2);
extern IMAGE *getboundingbox(IMAGE *im);
extern IMAGE *magnify(IMAGE *im, int n);
extern IMAGE **rotatecoor(IMAGE *im, double theta);

extern ERROR_TYPE generic_framebox(IMAGE *im, int *box,  GENERICPIX gval);
extern ERROR_TYPE uc_framebox(IMAGE *im, int *box,  GENERICPIX gval);
extern ERROR_TYPE us_framebox(IMAGE *im, int *box,  USHORT gval);
extern ERROR_TYPE i32_framebox(IMAGE *im, int *box,  INT32 gval);
extern ERROR_TYPE u32_framebox(IMAGE *im, int *box,  UINT32 gval);
extern ERROR_TYPE f_framebox(IMAGE *im, int *box,  MIALFLOAT gval);
extern ERROR_TYPE generic_addframebox(IMAGE *im, int *box,  GENERICPIX gval);
extern ERROR_TYPE us_addframebox(IMAGE *im, int *box,  USHORT gval);
extern ERROR_TYPE i32_addframebox(IMAGE *im, int *box,  INT32 gval);
extern ERROR_TYPE u32_addframebox(IMAGE *im, int *box,  UINT32 gval);

/* indexx.c */
extern void indexx(size_t , double [], size_t []);

/* miscel.c */
extern IMAGE *deinterleave(IMAGE *im);
/* miscel.c but not wrapped in mialisp */
extern G_TYPE getpixval(IMAGE *im, unsigned long offset);
extern void swap_long(INT32 *pl);

/* setshft.c */
extern ERROR_TYPE szcompat(IMAGE *, IMAGE *);
extern ERROR_TYPE szgeocompat(IMAGE *, IMAGE *);
extern ERROR_TYPE set_shift(long int nx, long int ny, long int nz, long int graph, long int *shift);
extern ERROR_TYPE set_seq_shift(long int , long int , long int , long int , long int *);
extern void set_shift_and_box(unsigned char *, int *, long int, long int, long int *);
extern void set_shift_and_box_and_weight(unsigned char *im1, MIALFLOAT *im2, int *box, long int x, long int y, long int *shift, MIALFLOAT *weight);
extern ERROR_TYPE setinvseqshift(long int nx, long int ny, long int nz, long int graph, long int *shft);
extern long int objectpix(IMAGE *im);
extern long int get_offset_first_pixel(long int nx, long int ny, long int nz, int graph);
extern long int get_offset_last_pixel(long int nx, long int ny, long int nz, int graph);

/* erodil.c */
extern ERROR_TYPE erode4(IMAGE *im, int ox, int oy);
extern ERROR_TYPE dilate4(IMAGE *im, int ox, int oy);
extern IMAGE *erode(IMAGE *im, IMAGE *imse, int ox, int oy, int oz, int trflag);
extern IMAGE *dilate(IMAGE *im, IMAGE *imse, int ox, int oy, int oz, int trflag);
extern IMAGE *volerode(IMAGE *im, IMAGE *imse, IMAGE *imweight, int ox, int oy, int oz);
extern IMAGE *rank(IMAGE *im, IMAGE *imse, int rank, int ox, int oy, int oz, int trflag);

/* lerodil.c */
extern ERROR_TYPE linero(IMAGE *im, int dx, int dy, int n, int line_type);
extern ERROR_TYPE lindil(IMAGE *im, int dx, int dy, int n,int line_type);

/* bresenham.c */
extern ERROR_TYPE plotline(IMAGE *im, int x1, int y1, int x2, int y2, int val);

/* rminmax.c */
extern IMAGE *minima(IMAGE *imin, int graph);

/* wshed.c */
extern IMAGE *sort_offset(IMAGE *im, IMAGE *imrsum);



#endif /* miallib.h */
