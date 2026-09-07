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
                          mcisrg.h  -  description
                             -------------------
    begin                : Thu Apr 22 2004
 ***************************************************************************/

#ifndef init_mcisrg
#define init_mcisrg

#define WHOLE_REGION 0x00
#define ORIGINAL_SEED 0x01
#define PIXEL_NEIGHBOUR 0x02

ERROR_TYPE mcisrg(IMAGE **imap, int nc, IMAGE *seedsIm, int connectivity, long int regionNumber, int version);
#endif
