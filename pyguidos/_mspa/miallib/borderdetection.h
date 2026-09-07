/***********************************************************************
Author(s): Dominik Brunner and Pierre Soille
Copyright (C) 2004-2020 European Union (Joint Research Centre)

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

/***************************************************************************
                          borderdetection.h  -  description
                             -------------------
    begin                : Thu May 13 2004
 ***************************************************************************/

#ifndef init_borderdetection
#define init_borderdetection

#include "miallib.h"
#include <zlib.h>
#include <shapefil.h>

#define NORTH 0x01
#define EAST 0x02
#define SOUTH 0x03
#define WEST 0x04
#define LEFT 0x05
#define RIGHT 0x06
#define STRAIGHT 0x07
#define DIAGONALBACK 0x08

struct LINEPOOL{
  int size, crtPos, step;
  struct LINE ** lines;
};

struct POINT {
  int x;
  int y;
  int useFlag;
};

struct LINE {
  int size, crtPos, step;
  struct POINT ** points;
};


struct REGION {
  int size, crtPos, step;
  struct LINE ** lines;
  int * colorValues;
  int channelSize;
  int isFirstLineSegmentClosed;
};

struct LINE * initLine(struct LINE *line, int n);
int isRegionClosed(struct REGION *region);
int isLineSegmentClosed(struct LINE * line);
void freeLine(struct LINE * line);
void freeRegion(struct REGION * region);
struct REGION * addLineToRegion(struct REGION * region, struct LINE * line);
struct LINE * addPointToLine(struct LINE * line, struct POINT * point);
struct REGION * initRegion(struct REGION * region, int lineSize, int pointSize, int channelSize);
ERROR_TYPE savePixel(int oldDir, int newDir, USHORT pixX, USHORT pixY, struct LINE * line);
long int getNextOffset(IMAGE * im, int oldDir, int newDir, struct POINT * point);
ERROR_TYPE detectBorders(IMAGE * inputIm, struct REGION ** regions, int regionNumber, struct LINEPOOL * linepool);
struct LINEPOOL * initLinePool(struct LINEPOOL * linepool, int size);
struct LINE * getLine(struct LINEPOOL * linepool, int startX, int startY, int middleX, int middleY, int endX, int endY);
struct LINE * getLineWith2Points(struct LINEPOOL * linepool, int startX, int startY, int endX, int endY);
struct LINEPOOL * addLineToLinePool(struct LINEPOOL * linepool, struct LINE * line);
void freeLinePool(struct LINEPOOL * linepool);
struct POINT * getNextPoint(struct REGION * region, struct POINT * point, int * trace);
int arePointsEqual(struct POINT * point1, struct POINT * point2);
struct LINE ** getLineSegment(struct REGION * region, struct POINT * pointSearch1, struct POINT * pointSearch2);
void setUseFlags(struct LINEPOOL * linepool, int value);
void correctCoordinateValues(struct LINEPOOL * linepool, int correctX, int correctY);
#endif

