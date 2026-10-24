/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8080/disasm.c                                       *
 * Created:     2012-11-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2014 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include "e8080.h"
#include "internal.h"

#include <string.h>


static
e8080_dop_t doptab[256] = {
	{ 0x00, "NOP",  NULL, NULL, ARG_NONE, ARG_NONE, 1,  4, 0 },
	{ 0x01, "LXI",  "BC", NULL, ARG_STR,  ARG_IM16, 3, 10, 0 },
	{ 0x02, "STAX", "BC", NULL, ARG_STR,  ARG_NONE, 1,  7, 0 },
	{ 0x03, "INX",  "BC", NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x04, "INR",  "B",  NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x05, "DCR",  "B",  NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x06, "MVI",  "B",  NULL, ARG_STR,  ARG_IMM8, 2,  7, 0 },
	{ 0x07, "RLC",  NULL, NULL, ARG_NONE, ARG_NONE, 1,  4, 0 },
	{ 0x08, "DB",   NULL, NULL, ARG_OPC8, ARG_NONE, 1,  4, E8080_OPF_UND },
	{ 0x09, "DAD",  "BC", NULL, ARG_STR,  ARG_NONE, 1, 10, 0 },
	{ 0x0a, "LDAX", "BC", NULL, ARG_STR,  ARG_NONE, 1,  7, 0 },
	{ 0x0b, "DCX",  "BC", NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x0c, "INR",  "C",  NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x0d, "DCR",  "C",  NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x0e, "MVI",  "C",  NULL, ARG_STR,  ARG_IMM8, 2,  7, 0 },
	{ 0x0f, "RRC",  NULL, NULL, ARG_NONE, ARG_NONE, 1,  4, 0 },
	{ 0x10, "DB",   NULL, NULL, ARG_OPC8, ARG_NONE, 1,  4, E8080_OPF_UND },
	{ 0x11, "LXI",  "DE", NULL, ARG_STR,  ARG_IM16, 3, 10, 0 },
	{ 0x12, "STAX", "DE", NULL, ARG_STR,  ARG_NONE, 1,  7, 0 },
	{ 0x13, "INX",  "DE", NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x14, "INR",  "D",  NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x15, "DCR",  "D",  NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x16, "MVI",  "D",  NULL, ARG_STR,  ARG_IMM8, 2,  7, 0 },
	{ 0x17, "RAL",  NULL, NULL, ARG_NONE, ARG_NONE, 1,  4, 0 },
	{ 0x18, "DB",   NULL, NULL, ARG_OPC8, ARG_NONE, 1,  4, E8080_OPF_UND },
	{ 0x19, "DAD",  "DE", NULL, ARG_STR,  ARG_NONE, 1, 10, 0 },
	{ 0x1a, "LDAX", "DE", NULL, ARG_STR,  ARG_NONE, 1,  7, 0 },
	{ 0x1b, "DCX",  "DE", NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x1c, "INR",  "E",  NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x1d, "DCR",  "E",  NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x1e, "MVI",  "E",  NULL, ARG_STR,  ARG_IMM8, 2,  7, 0 },
	{ 0x1f, "RAR",  NULL, NULL, ARG_NONE, ARG_NONE, 1,  4, 0 },
	{ 0x20, "DB",   NULL, NULL, ARG_OPC8, ARG_NONE, 1,  4, E8080_OPF_UND },
	{ 0x21, "LXI",  "HL", NULL, ARG_STR,  ARG_IM16, 3, 10, 0 },
	{ 0x22, "SHLD", NULL, NULL, ARG_AD16, ARG_NONE, 3, 16, 0 },
	{ 0x23, "INX",  "HL", NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x24, "INR",  "H",  NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x25, "DCR",  "H",  NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x26, "MVI",  "H",  NULL, ARG_STR,  ARG_IMM8, 2,  7, 0 },
	{ 0x27, "DAA",  NULL, NULL, ARG_NONE, ARG_NONE, 1,  4, 0 },
	{ 0x28, "DB",   NULL, NULL, ARG_OPC8, ARG_NONE, 1,  4, E8080_OPF_UND },
	{ 0x29, "DAD",  "HL", NULL, ARG_STR,  ARG_NONE, 1, 10, 0 },
	{ 0x2a, "LHLD", NULL, NULL, ARG_AD16, ARG_NONE, 3, 16, 0 },
	{ 0x2b, "DCX",  "HL", NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x2c, "INR",  "L",  NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x2d, "DCR",  "L",  NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x2e, "MVI",  "L",  NULL, ARG_STR,  ARG_IMM8, 2,  7, 0 },
	{ 0x2f, "CMA",  NULL, NULL, ARG_NONE, ARG_NONE, 1,  4, 0 },
	{ 0x30, "DB",   NULL, NULL, ARG_OPC8, ARG_NONE, 1,  4, E8080_OPF_UND },
	{ 0x31, "LXI",  "SP", NULL, ARG_STR,  ARG_IM16, 3, 10, 0 },
	{ 0x32, "STA",  NULL, NULL, ARG_AD16, ARG_NONE, 3, 13, 0 },
	{ 0x33, "INX",  "SP", NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x34, "INR",  "M",  NULL, ARG_STR,  ARG_NONE, 1, 10, 0 },
	{ 0x35, "DCR",  "M",  NULL, ARG_STR,  ARG_NONE, 1, 10, 0 },
	{ 0x36, "MVI",  "M",  NULL, ARG_STR,  ARG_IMM8, 2, 10, 0 },
	{ 0x37, "STC",  NULL, NULL, ARG_NONE, ARG_NONE, 1,  4, 0 },
	{ 0x38, "DB",   NULL, NULL, ARG_OPC8, ARG_NONE, 1,  4, E8080_OPF_UND },
	{ 0x39, "DAD",  "SP", NULL, ARG_STR,  ARG_NONE, 1, 10, 0 },
	{ 0x3a, "LDA",  NULL, NULL, ARG_AD16, ARG_NONE, 3, 13, 0 },
	{ 0x3b, "DCX",  "SP", NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x3c, "INR",  "A",  NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x3d, "DCR",  "A",  NULL, ARG_STR,  ARG_NONE, 1,  5, 0 },
	{ 0x3e, "MVI",  "A",  NULL, ARG_STR,  ARG_IMM8, 2,  7, 0 },
	{ 0x3f, "CMC",  NULL, NULL, ARG_NONE, ARG_NONE, 1,  4, 0 },
	{ 0x40, "MOV",  "B",  "B",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x41, "MOV",  "B",  "C",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x42, "MOV",  "B",  "D",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x43, "MOV",  "B",  "E",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x44, "MOV",  "B",  "H",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x45, "MOV",  "B",  "L",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x46, "MOV",  "B",  "M",  ARG_STR,  ARG_STR,  1,  7, 0 },
	{ 0x47, "MOV",  "B",  "A",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x48, "MOV",  "C",  "B",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x49, "MOV",  "C",  "C",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x4a, "MOV",  "C",  "D",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x4b, "MOV",  "C",  "E",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x4c, "MOV",  "C",  "H",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x4d, "MOV",  "C",  "L",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x4e, "MOV",  "C",  "M",  ARG_STR,  ARG_STR,  1,  7, 0 },
	{ 0x4f, "MOV",  "C",  "A",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x50, "MOV",  "D",  "B",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x51, "MOV",  "D",  "C",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x52, "MOV",  "D",  "D",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x53, "MOV",  "D",  "E",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x54, "MOV",  "D",  "H",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x55, "MOV",  "D",  "L",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x56, "MOV",  "D",  "M",  ARG_STR,  ARG_STR,  1,  7, 0 },
	{ 0x57, "MOV",  "D",  "A",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x58, "MOV",  "E",  "B",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x59, "MOV",  "E",  "C",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x5a, "MOV",  "E",  "D",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x5b, "MOV",  "E",  "E",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x5c, "MOV",  "E",  "H",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x5d, "MOV",  "E",  "L",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x5e, "MOV",  "E",  "M",  ARG_STR,  ARG_STR,  1,  7, 0 },
	{ 0x5f, "MOV",  "E",  "A",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x60, "MOV",  "H",  "B",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x61, "MOV",  "H",  "C",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x62, "MOV",  "H",  "D",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x63, "MOV",  "H",  "E",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x64, "MOV",  "H",  "H",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x65, "MOV",  "H",  "L",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x66, "MOV",  "H",  "M",  ARG_STR,  ARG_STR,  1,  7, 0 },
	{ 0x67, "MOV",  "H",  "A",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x68, "MOV",  "L",  "B",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x69, "MOV",  "L",  "C",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x6a, "MOV",  "L",  "D",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x6b, "MOV",  "L",  "E",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x6c, "MOV",  "L",  "H",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x6d, "MOV",  "L",  "L",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x6e, "MOV",  "L",  "M",  ARG_STR,  ARG_STR,  1,  7, 0 },
	{ 0x6f, "MOV",  "L",  "A",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x70, "MOV",  "M",  "B",  ARG_STR,  ARG_STR,  1,  7, 0 },
	{ 0x71, "MOV",  "M",  "C",  ARG_STR,  ARG_STR,  1,  7, 0 },
	{ 0x72, "MOV",  "M",  "D",  ARG_STR,  ARG_STR,  1,  7, 0 },
	{ 0x73, "MOV",  "M",  "E",  ARG_STR,  ARG_STR,  1,  7, 0 },
	{ 0x74, "MOV",  "M",  "H",  ARG_STR,  ARG_STR,  1,  7, 0 },
	{ 0x75, "MOV",  "M",  "L",  ARG_STR,  ARG_STR,  1,  7, 0 },
	{ 0x76, "HLT",  NULL, NULL, ARG_NONE, ARG_NONE, 1,  7, 0 },
	{ 0x77, "MOV",  "M",  "A",  ARG_STR,  ARG_STR,  1,  7, 0 },
	{ 0x78, "MOV",  "A",  "B",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x79, "MOV",  "A",  "C",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x7a, "MOV",  "A",  "D",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x7b, "MOV",  "A",  "E",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x7c, "MOV",  "A",  "H",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x7d, "MOV",  "A",  "L",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x7e, "MOV",  "A",  "M",  ARG_STR,  ARG_STR,  1,  7, 0 },
	{ 0x7f, "MOV",  "A",  "A",  ARG_STR,  ARG_STR,  1,  5, 0 },
	{ 0x80, "ADD",  "B",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x81, "ADD",  "C",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x82, "ADD",  "D",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x83, "ADD",  "E",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x84, "ADD",  "H",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x85, "ADD",  "L",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x86, "ADD",  "M",  NULL, ARG_STR,  ARG_NONE, 1,  7, 0 },
	{ 0x87, "ADD",  "A",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x88, "ADC",  "B",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x89, "ADC",  "C",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x8a, "ADC",  "D",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x8b, "ADC",  "E",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x8c, "ADC",  "H",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x8d, "ADC",  "L",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x8e, "ADC",  "M",  NULL, ARG_STR,  ARG_NONE, 1,  7, 0 },
	{ 0x8f, "ADC",  "A",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x90, "SUB",  "B",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x91, "SUB",  "C",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x92, "SUB",  "D",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x93, "SUB",  "E",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x94, "SUB",  "H",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x95, "SUB",  "L",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x96, "SUB",  "M",  NULL, ARG_STR,  ARG_NONE, 1,  7, 0 },
	{ 0x97, "SUB",  "A",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x98, "SBB",  "B",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x99, "SBB",  "C",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x9a, "SBB",  "D",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x9b, "SBB",  "E",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x9c, "SBB",  "H",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x9d, "SBB",  "L",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0x9e, "SBB",  "M",  NULL, ARG_STR,  ARG_NONE, 1,  7, 0 },
	{ 0x9f, "SBB",  "A",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xa0, "ANA",  "B",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xa1, "ANA",  "C",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xa2, "ANA",  "D",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xa3, "ANA",  "E",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xa4, "ANA",  "H",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xa5, "ANA",  "L",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xa6, "ANA",  "M",  NULL, ARG_STR,  ARG_NONE, 1,  7, 0 },
	{ 0xa7, "ANA",  "A",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xa8, "XRA",  "B",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xa9, "XRA",  "C",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xaa, "XRA",  "D",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xab, "XRA",  "E",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xac, "XRA",  "H",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xad, "XRA",  "L",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xae, "XRA",  "M",  NULL, ARG_STR,  ARG_NONE, 1,  7, 0 },
	{ 0xaf, "XRA",  "A",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xb0, "ORA",  "B",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xb1, "ORA",  "C",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xb2, "ORA",  "D",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xb3, "ORA",  "E",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xb4, "ORA",  "H",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xb5, "ORA",  "L",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xb6, "ORA",  "M",  NULL, ARG_STR,  ARG_NONE, 1,  7, 0 },
	{ 0xb7, "ORA",  "A",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xb8, "CMP",  "B",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xb9, "CMP",  "C",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xba, "CMP",  "D",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xbb, "CMP",  "E",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xbc, "CMP",  "H",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xbd, "CMP",  "L",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xbe, "CMP",  "M",  NULL, ARG_STR,  ARG_NONE, 1,  7, 0 },
	{ 0xbf, "CMP",  "A",  NULL, ARG_STR,  ARG_NONE, 1,  4, 0 },
	{ 0xc0, "RNZ",  NULL, NULL, ARG_NONE, ARG_NONE, 1, 11, E8080_OPF_RET | E8080_OPF_COND },
	{ 0xc1, "POP",  "BC", NULL, ARG_STR,  ARG_NONE, 1, 10, 0 },
	{ 0xc2, "JNZ",  NULL, NULL, ARG_JP16, ARG_NONE, 3, 10, E8080_OPF_JMP | E8080_OPF_COND },
	{ 0xc3, "JMP",  NULL, NULL, ARG_JP16, ARG_NONE, 3, 10, E8080_OPF_JMP },
	{ 0xc4, "CNZ",  NULL, NULL, ARG_JP16, ARG_NONE, 3, 17, E8080_OPF_CALL | E8080_OPF_COND },
	{ 0xc5, "PUSH", "BC", NULL, ARG_STR,  ARG_NONE, 1, 11, 0 },
	{ 0xc6, "ADI",  NULL, NULL, ARG_IMM8, ARG_NONE, 2,  7, 0 },
	{ 0xc7, "RST",  "0",  NULL, ARG_STR,  ARG_NONE, 1, 11, E8080_OPF_CALL },
	{ 0xc8, "RZ",   NULL, NULL, ARG_NONE, ARG_NONE, 1, 11, E8080_OPF_RET | E8080_OPF_COND },
	{ 0xc9, "RET",  NULL, NULL, ARG_NONE, ARG_NONE, 1, 10, E8080_OPF_RET },
	{ 0xca, "JZ",   NULL, NULL, ARG_JP16, ARG_NONE, 3, 10, E8080_OPF_JMP | E8080_OPF_COND },
	{ 0xcb, "DB",   NULL, NULL, ARG_OPC8, ARG_NONE, 1,  4, E8080_OPF_UND },
	{ 0xcc, "CZ",   NULL, NULL, ARG_JP16, ARG_NONE, 3, 17, E8080_OPF_CALL | E8080_OPF_COND },
	{ 0xcd, "CALL", NULL, NULL, ARG_JP16, ARG_NONE, 3, 17, E8080_OPF_CALL },
	{ 0xce, "ACI",  NULL, NULL, ARG_IMM8, ARG_NONE, 2,  7, 0 },
	{ 0xcf, "RST",  "1",  NULL, ARG_STR,  ARG_NONE, 1, 11, E8080_OPF_CALL },
	{ 0xd0, "RNC",  NULL, NULL, ARG_NONE, ARG_NONE, 1, 11, E8080_OPF_RET | E8080_OPF_COND },
	{ 0xd1, "POP",  "DE", NULL, ARG_STR,  ARG_NONE, 1, 10, 0 },
	{ 0xd2, "JNC",  NULL, NULL, ARG_JP16, ARG_NONE, 3, 10, E8080_OPF_JMP | E8080_OPF_COND },
	{ 0xd3, "OUT",  NULL, NULL, ARG_IMM8, ARG_NONE, 2, 10, 0 },
	{ 0xd4, "CNC",  NULL, NULL, ARG_JP16, ARG_NONE, 3, 17, E8080_OPF_CALL | E8080_OPF_COND },
	{ 0xd5, "PUSH", "DE", NULL, ARG_STR,  ARG_NONE, 1, 11, 0 },
	{ 0xd6, "SUI",  NULL, NULL, ARG_IMM8, ARG_NONE, 2,  7, 0 },
	{ 0xd7, "RST",  "2",  NULL, ARG_STR,  ARG_NONE, 1, 11, E8080_OPF_CALL },
	{ 0xd8, "RC",   NULL, NULL, ARG_NONE, ARG_NONE, 1, 11, E8080_OPF_RET | E8080_OPF_COND },
	{ 0xd9, "DB",   NULL, NULL, ARG_OPC8, ARG_NONE, 1,  4, E8080_OPF_UND },
	{ 0xda, "JC",   NULL, NULL, ARG_JP16, ARG_NONE, 3, 10, E8080_OPF_JMP | E8080_OPF_COND },
	{ 0xdb, "IN",   NULL, NULL, ARG_IMM8, ARG_NONE, 2, 10, 0 },
	{ 0xdc, "CC",   NULL, NULL, ARG_JP16, ARG_NONE, 3, 17, E8080_OPF_CALL | E8080_OPF_COND },
	{ 0xdd, "DB",   NULL, NULL, ARG_OPC8, ARG_NONE, 1,  4, E8080_OPF_UND },
	{ 0xde, "SBI",  NULL, NULL, ARG_IMM8, ARG_NONE, 2,  7, 0 },
	{ 0xdf, "RST",  "3",  NULL, ARG_STR,  ARG_NONE, 1, 11, E8080_OPF_CALL },
	{ 0xe0, "RPO",  NULL, NULL, ARG_NONE, ARG_NONE, 1, 11, E8080_OPF_RET | E8080_OPF_COND },
	{ 0xe1, "POP",  "HL", NULL, ARG_STR,  ARG_NONE, 1, 10, 0 },
	{ 0xe2, "JPO",  NULL, NULL, ARG_JP16, ARG_NONE, 3, 10, E8080_OPF_JMP | E8080_OPF_COND },
	{ 0xe3, "XTHL", NULL, NULL, ARG_NONE, ARG_NONE, 1, 18, 0 },
	{ 0xe4, "CPO",  NULL, NULL, ARG_JP16, ARG_NONE, 3, 17, E8080_OPF_CALL | E8080_OPF_COND },
	{ 0xe5, "PUSH", "HL", NULL, ARG_STR,  ARG_NONE, 1, 11, 0 },
	{ 0xe6, "ANI",  NULL, NULL, ARG_IMM8, ARG_NONE, 2,  7, 0 },
	{ 0xe7, "RST",  "4",  NULL, ARG_STR,  ARG_NONE, 1, 11, E8080_OPF_CALL },
	{ 0xe8, "RPE",  NULL, NULL, ARG_NONE, ARG_NONE, 1, 11, E8080_OPF_RET | E8080_OPF_COND },
	{ 0xe9, "PCHL", NULL, NULL, ARG_NONE, ARG_NONE, 1,  5, E8080_OPF_JMP },
	{ 0xea, "JPE",  NULL, NULL, ARG_JP16, ARG_NONE, 3, 10, E8080_OPF_JMP | E8080_OPF_COND },
	{ 0xeb, "XCHG", NULL, NULL, ARG_NONE, ARG_NONE, 1,  4, 0 },
	{ 0xec, "CPE",  NULL, NULL, ARG_JP16, ARG_NONE, 3, 17, E8080_OPF_CALL | E8080_OPF_COND },
	{ 0xed, "DB",   NULL, NULL, ARG_OPC8, ARG_NONE, 1,  4, E8080_OPF_UND },
	{ 0xee, "XRI",  NULL, NULL, ARG_IMM8, ARG_NONE, 2,  7, 0 },
	{ 0xef, "RST",  "5",  NULL, ARG_STR,  ARG_NONE, 1, 11, E8080_OPF_CALL },
	{ 0xf0, "RP",   NULL, NULL, ARG_NONE, ARG_NONE, 1, 11, E8080_OPF_RET | E8080_OPF_COND },
	{ 0xf1, "POP",  "AF", NULL, ARG_STR,  ARG_NONE, 1, 10, 0 },
	{ 0xf2, "JP",   NULL, NULL, ARG_JP16, ARG_NONE, 3, 10, E8080_OPF_JMP | E8080_OPF_COND },
	{ 0xf3, "DI",   NULL, NULL, ARG_NONE, ARG_NONE, 1,  4, 0 },
	{ 0xf4, "CP",   NULL, NULL, ARG_JP16, ARG_NONE, 3, 17, E8080_OPF_CALL | E8080_OPF_COND },
	{ 0xf5, "PUSH", "AF", NULL, ARG_STR,  ARG_NONE, 1, 11, 0 },
	{ 0xf6, "ORI",  NULL, NULL, ARG_IMM8, ARG_NONE, 2,  7, 0 },
	{ 0xf7, "RST",  "6",  NULL, ARG_STR,  ARG_NONE, 1, 11, E8080_OPF_CALL },
	{ 0xf8, "RM",   NULL, NULL, ARG_NONE, ARG_NONE, 1, 11, E8080_OPF_RET | E8080_OPF_COND },
	{ 0xf9, "SPHL", NULL, NULL, ARG_NONE, ARG_NONE, 1,  5, 0 },
	{ 0xfa, "JM",   NULL, NULL, ARG_JP16, ARG_NONE, 3, 10, E8080_OPF_JMP | E8080_OPF_COND },
	{ 0xfb, "EI",   NULL, NULL, ARG_NONE, ARG_NONE, 1,  4, 0 },
	{ 0xfc, "CM",   NULL, NULL, ARG_JP16, ARG_NONE, 3, 17, E8080_OPF_CALL | E8080_OPF_COND },
	{ 0xfd, "DB",   NULL, NULL, ARG_OPC8, ARG_NONE, 1,  4, E8080_OPF_UND },
	{ 0xfe, "CPI",  NULL, NULL, ARG_IMM8, ARG_NONE, 2,  7, 0 },
	{ 0xff, "RST",  "7",  NULL, ARG_STR,  ARG_NONE, 1, 11, E8080_OPF_CALL }
};


static
int e8080_disasm_arg (char *dst, const unsigned char *src, const char *str, unsigned arg)
{
	switch (arg) {
	case ARG_STR:
		strcpy (dst, str);
		return (0);

	case ARG_OPC8:
		sprintf (dst, "%02X", src[0]);
		return (0);

	case ARG_IMM8:
		sprintf (dst, "%02X", src[1]);
		return (0);

	case ARG_AD16:
	case ARG_IM16:
	case ARG_JP16:
		sprintf (dst, "%04X", e8080_uint16 (src[1], src[2]));
		return (0);
	}

	return (1);
}

void e8080_disasm (e8080_disasm_t *op, const unsigned char *src, unsigned short pc)
{
	unsigned    i, n;
	e8080_dop_t *tab;

	tab = &doptab[src[0] & 0xff];

	op->pc = pc;
	op->flags = tab->flags;
	op->data_cnt = tab->size;

	for (i = 0; i < tab->size; i++) {
		op->data[i] = src[i];
	}

	strcpy (op->op, tab->mnemonic);

	n = 0;

	if (e8080_disasm_arg (op->arg[n], src, tab->arg1s, tab->arg1) == 0) {
		n += 1;
	}

	if (e8080_disasm_arg (op->arg[n], src, tab->arg2s, tab->arg2) == 0) {
		n += 1;
	}

	op->arg_cnt = n;
}

void e8080_disasm_mem (e8080_t *c, e8080_disasm_t *op, unsigned short pc)
{
	unsigned      i;
	unsigned char src[4];

	for (i = 0; i < 4; i++) {
		src[i] = e8080_get_mem8 (c, (pc + i) & 0xffff);
	}

	e8080_disasm (op, src, pc);
}

void e8080_disasm_cur (e8080_t *c, e8080_disasm_t *op)
{
	e8080_disasm_mem (c, op, e8080_get_pc (c));
}
