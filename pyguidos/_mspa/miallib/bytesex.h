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

/* define BYTE_ORDER for BIG_ and LITTLE_ENDIAN architectures */


/* #if defined(__i386__) || defined(__i486__) || defined(__alpha__) || defined(__TURBOC__)\ */
/* 	|| (defined(__mips__) && (defined(MIPSEL) || defined (__MIPSEL__))) */
/* #define BYTE_ORDER	1234 */
/* #elif defined(__mc68000__) || defined (__sparc__) \ */
/* 	|| (defined(__mips__) && (defined(MIPSEB) || defined (__MIPSEB__))) */
/* #define BYTE_ORDER	4321 */
/* #else */
/*  #error architecture not supported by the Linux C library */
/* #endif */
