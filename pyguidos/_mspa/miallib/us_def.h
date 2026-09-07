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

/* macro definitions for unsigned short char data types */
#define FLOATING 0
#define SIGNED 0
#define OVFL_TEST 1
#define t_PIX_TYPE t_USHORT
#define PIX_TYPE  USHORT
#define PIX_MIN   USHORT_MIN
#define PIX_MAX   USHORT_MAX
#define PIX_MAXP1 (USHORT_MAX+1)
#define PIX_MSB   0x8000
#define PIX_RANGE PIX_MAX
#define BitPerPixel 16
