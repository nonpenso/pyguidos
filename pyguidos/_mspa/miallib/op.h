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


/* op definitions */
#define ADD_op           0 /* arith */
#define SUB_op           1
#define MULT_op          2
#define DIV_op           3
#define INF_op           4
#define SUP_op           5
#define MASK_op          6
#define ADD_op_ovfl      7
#define SUB_op_ovfl      8
#define MULT_op_ovfl     9
#define AND_op           10 /* logical */
#define OR_op            11
#define XOR_op           12
#define CMP_op           13
#define ABSSUB_op        14
#define MASK_op2         15
#define SUBSWAP_op       16
#define SUBSWAP_op_ovfl  17
#define EQUAL_op         18
#define OVW_op           19 /* overwrite used for imput */
#define POW_op           20
#define NDI_op           21 /* normalised difference index */
#define SUBSWAPCST_op    22 /* change sign */
#define FirstBitOn_op    23 /* only retains first bit on */
#define NAND_op          24 /* logical */
#define ATAN_op          25
#define COS_op           26
#define SIN_op           27
