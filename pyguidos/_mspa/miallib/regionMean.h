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
                          regionMean.h  -  description
                             -------------------
    begin                : Thu Apr 22 2004
 ***************************************************************************/

#ifndef init_regionMean
#define init_regionMean

#include "miallib.h"

struct regionMean
{
	float * meanValueOriginal;
	float * meanValue;

	int nc; //number of channels
	long int count; //number values stored in meanValues
	long int countOriginal; // numbers of values stored in meanValueOriginal
};


struct regionMean * rmInit(struct regionMean *rm, int nc);

int rmAddValue(struct regionMean * rm, IMAGE **imap, long int offset);
int uc_rmAddValue(struct regionMean * rm, IMAGE **imap, long int offset);

int rmAddValueOriginal(struct regionMean * rm, IMAGE **imap, long int offset);
int uc_rmAddValueOriginal(struct regionMean * rm, IMAGE **imap, long int offset);

double rmGetDistanceToRM(struct regionMean * rm, IMAGE **imap, long int offset);
double uc_rmGetDistanceToRM(struct regionMean * rm, IMAGE **imap, long int offset);

double rmGetDistanceToOriginalRM(struct regionMean * rm, IMAGE **imap, long int offset);
double uc_rmGetDistanceToOriginalRM(struct regionMean * rm, IMAGE **imap, long int offset);

double rmGetContrastCoefficient(struct regionMean * rm, IMAGE **imap, long int offset);
double uc_rmGetContrastCoefficient(struct regionMean * rm, IMAGE **imap, long int offset);

void clearRegionMean(struct regionMean * rm);

void freeRegionMean(struct regionMean * rm);

#endif
