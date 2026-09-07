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



#ifndef _MIALLIB_IO_H
#define _MIALLIB_IO_H       1

#include "mialtypes.h"


/* imio_gdal.c */
extern int GDAL2MIALDataType(int aGDALDataType);
extern IMAGE *GDALInfoJIP(char *imfn);
extern IMAGE *GDALRead(char *imfn, int band, int nXOff, int nYOff, int nXSize, int nYSize, int nBufXSize, int nBufYSize);

/* imio.c */
extern IMAGE *read_all(char *fn, int nx, int ny, int nz, int data_type, int header_size, int pc);
extern IMAGE *read_image(char *fn);
extern IMAGE *read_image_to_type(char *fn, int data_type);
// extern ERROR_TYPE read_image_data(FILE *fp, IMAGE *im, int pc);
// extern ERROR_TYPE write_image_data(FILE *fp, IMAGE *im, int pc);
extern ERROR_TYPE write_ColorMap_tiff(IMAGE *im, char *fn);
extern ERROR_TYPE write_tiff(IMAGE *im, char *fn);
extern ERROR_TYPE writeTiffOneStripPerLine(IMAGE *im, char *fn, char *desc);

/* imio2.c */
extern IMAGE *GetGeoKey(char *fname, char *keyname);
extern IMAGE *GetTIFFTagGeo(char *fn, char *tagname);
extern IMAGE *read_image2(char *fn, int x, int y, int szx, int szy, int scale);
extern IMAGE *readTiffSubset(char *fn, int x, int y, unsigned szx, unsigned szy);
extern ERROR_TYPE tiffinfo(char *fn, char *field, float *val);
extern IMAGE *tiffinfoJIP(char *fn);
// extern ERROR_TYPE read_image_data2(FILE *fp, IMAGE *im, int x, int y, int inx, int scale);
extern ERROR_TYPE writeGeoTiffOneStripPerLine(IMAGE *im, char *fn, int PCSCode, double xoff, double yoff, double scale, unsigned short RasterType, int nodata_flag, int nodata_val, int metadata_flag, char *metadata_str);
extern ERROR_TYPE writeMBGeoTiffOneStripPerLine(IMAGE **imap, int nc, char *fn, int PCSCode, double xoff, double yoff, double scale, unsigned short RasterType, int nodata_flag, int nodata_val, int metadata_flag, char *metadata_str);
extern void print_mia_banner();


#endif /* miallib_io.h */
