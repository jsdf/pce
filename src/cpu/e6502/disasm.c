/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e6502/disasm.c                                       *
 * Created:     2004-05-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2010 Hampa Hug <hampa@hampa.ch>                     *
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


#include <string.h>

#include "e6502.h"
#include "internal.h"


#define E6502_MODE_IMP       1
#define E6502_MODE_IMM       2
#define E6502_MODE_ZPG       3
#define E6502_MODE_ZPG_X     4
#define E6502_MODE_ZPG_Y     5
#define E6502_MODE_ABS       6
#define E6502_MODE_ABS_Y     7
#define E6502_MODE_ABS_X     8
#define E6502_MODE_IDX_IND_X 9
#define E6502_MODE_IND_IDX_Y 10
#define E6502_MODE_DB        11
#define E6502_MODE_BRA       12
#define E6502_MODE_JMP       13
#define E6502_MODE_A         14


typedef struct {
	unsigned char opcode;
	const char    *mnemonic;
	unsigned char mode;
	unsigned char size;
	unsigned char cycles;
	unsigned      flags;
} e6502_dop_t;

static
e6502_dop_t doptab[256] = {
	{ 0x00, "BRK", E6502_MODE_IMP,       1, 7, 0 },
	{ 0x01, "ORA", E6502_MODE_IDX_IND_X, 2, 6, 0 },
	{ 0x02, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x03, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x04, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x05, "ORA", E6502_MODE_ZPG,       2, 3, 0 },
	{ 0x06, "ASL", E6502_MODE_ZPG,       2, 5, 0 },
	{ 0x07, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x08, "PHP", E6502_MODE_IMP,       1, 3, 0 },
	{ 0x09, "ORA", E6502_MODE_IMM,       2, 2, 0 },
	{ 0x0a, "ASL", E6502_MODE_A,         1, 2, 0 },
	{ 0x0b, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x0c, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x0d, "ORA", E6502_MODE_ABS,       3, 4, 0 },
	{ 0x0e, "ASL", E6502_MODE_ABS,       3, 6, 0 },
	{ 0x0f, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x10, "BPL", E6502_MODE_BRA,       2, 2, E6502_OPF_BRA },
	{ 0x11, "ORA", E6502_MODE_IND_IDX_Y, 2, 5, 0 },
	{ 0x12, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x13, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x14, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x15, "ORA", E6502_MODE_ZPG_X,     2, 4, 0 },
	{ 0x16, "ASL", E6502_MODE_ZPG_X,     2, 6, 0 },
	{ 0x17, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x18, "CLC", E6502_MODE_IMP,       1, 2, 0 },
	{ 0x19, "ORA", E6502_MODE_ABS_Y,     3, 4, 0 },
	{ 0x1a, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x1b, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x1c, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x1d, "ORA", E6502_MODE_ABS_X,     3, 4, 0 },
	{ 0x1e, "ASL", E6502_MODE_ABS_X,     3, 7, 0 },
	{ 0x1f, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x20, "JSR", E6502_MODE_JMP,       3, 6, E6502_OPF_JSR },
	{ 0x21, "AND", E6502_MODE_IDX_IND_X, 2, 6, 0 },
	{ 0x22, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x23, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x24, "BIT", E6502_MODE_ZPG,       2, 3, 0 },
	{ 0x25, "AND", E6502_MODE_ZPG,       2, 3, 0 },
	{ 0x26, "ROL", E6502_MODE_ZPG,       2, 5, 0 },
	{ 0x27, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x28, "PLP", E6502_MODE_IMP,       1, 4, 0 },
	{ 0x29, "AND", E6502_MODE_IMM,       2, 2, 0 },
	{ 0x2a, "ROL", E6502_MODE_A,         1, 2, 0 },
	{ 0x2b, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x2c, "BIT", E6502_MODE_ABS,       3, 4, 0 },
	{ 0x2d, "AND", E6502_MODE_ABS,       3, 4, 0 },
	{ 0x2e, "ROL", E6502_MODE_ABS,       3, 6, 0 },
	{ 0x2f, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x30, "BMI", E6502_MODE_BRA,       2, 2, E6502_OPF_BRA },
	{ 0x31, "AND", E6502_MODE_IND_IDX_Y, 2, 5, 0 },
	{ 0x32, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x33, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x34, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x35, "AND", E6502_MODE_ZPG_X,     2, 4, 0 },
	{ 0x36, "ROL", E6502_MODE_ZPG_X,     2, 6, 0 },
	{ 0x37, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x38, "SEC", E6502_MODE_IMP,       1, 2, 0 },
	{ 0x39, "AND", E6502_MODE_ABS_Y,     3, 4, 0 },
	{ 0x3a, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x3b, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x3c, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x3d, "AND", E6502_MODE_ABS_X,     3, 4, 0 },
	{ 0x3e, "ROL", E6502_MODE_ABS_X,     3, 7, 0 },
	{ 0x3f, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x40, "RTI", E6502_MODE_IMP,       1, 6, E6502_OPF_RTI },
	{ 0x41, "EOR", E6502_MODE_IDX_IND_X, 2, 6, 0 },
	{ 0x42, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x43, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x44, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x45, "EOR", E6502_MODE_ZPG,       2, 3, 0 },
	{ 0x46, "LSR", E6502_MODE_ZPG,       2, 5, 0 },
	{ 0x47, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x48, "PHA", E6502_MODE_IMP,       1, 3, 0 },
	{ 0x49, "EOR", E6502_MODE_IMM,       2, 2, 0 },
	{ 0x4a, "LSR", E6502_MODE_A,         1, 2, 0 },
	{ 0x4b, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x4c, "JMP", E6502_MODE_JMP,       3, 3, E6502_OPF_BRA },
	{ 0x4d, "EOR", E6502_MODE_ABS,       3, 4, 0 },
	{ 0x4e, "LSR", E6502_MODE_ABS,       3, 6, 0 },
	{ 0x4f, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x50, "BVC", E6502_MODE_BRA,       2, 2, E6502_OPF_BRA },
	{ 0x51, "EOR", E6502_MODE_IND_IDX_Y, 2, 5, 0 },
	{ 0x52, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x53, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x54, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x55, "EOR", E6502_MODE_ZPG_X,     2, 4, 0 },
	{ 0x56, "LSR", E6502_MODE_ZPG_X,     2, 6, 0 },
	{ 0x57, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x58, "CLI", E6502_MODE_IMP,       1, 2, 0 },
	{ 0x59, "EOR", E6502_MODE_ABS_Y,     3, 4, 0 },
	{ 0x5a, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x5b, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x5c, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x5d, "EOR", E6502_MODE_IDX_IND_X, 3, 4, 0 },
	{ 0x5e, "LSR", E6502_MODE_ABS_X,     3, 7, 0 },
	{ 0x5f, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x60, "RTS", E6502_MODE_IMP,       1, 6, E6502_OPF_RTS },
	{ 0x61, "ADC", E6502_MODE_IDX_IND_X, 2, 6, 0 },
	{ 0x62, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x63, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x64, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x65, "ADC", E6502_MODE_ZPG,       2, 3, 0 },
	{ 0x66, "ROR", E6502_MODE_ZPG,       2, 5, 0 },
	{ 0x67, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x68, "PLA", E6502_MODE_IMP,       1, 4, 0 },
	{ 0x69, "ADC", E6502_MODE_IMM,       2, 2, 0 },
	{ 0x6a, "ROR", E6502_MODE_A,         1, 2, 0 },
	{ 0x6b, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x6c, "JMP", E6502_MODE_ABS,       3, 5, E6502_OPF_BRA },
	{ 0x6d, "ADC", E6502_MODE_ABS,       3, 4, 0 },
	{ 0x6e, "ROR", E6502_MODE_ABS,       3, 6, 0 },
	{ 0x6f, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x70, "BVS", E6502_MODE_BRA,       2, 2, E6502_OPF_BRA },
	{ 0x71, "ADC", E6502_MODE_IND_IDX_Y, 2, 5, 0 },
	{ 0x72, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x73, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x74, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x75, "ADC", E6502_MODE_ZPG_X,     2, 4, 0 },
	{ 0x76, "ROR", E6502_MODE_ZPG_X,     2, 6, 0 },
	{ 0x77, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x78, "SEI", E6502_MODE_IMP,       1, 2, 0 },
	{ 0x79, "ADC", E6502_MODE_ABS_Y,     3, 4, 0 },
	{ 0x7a, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x7b, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x7c, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x7d, "ADC", E6502_MODE_ABS_X,     3, 4, 0 },
	{ 0x7e, "ROR", E6502_MODE_ABS_X,     3, 7, 0 },
	{ 0x7f, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x80, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x81, "STA", E6502_MODE_IDX_IND_X, 2, 6, 0 },
	{ 0x82, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x83, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x84, "STY", E6502_MODE_ZPG,       2, 3, 0 },
	{ 0x85, "STA", E6502_MODE_ZPG,       2, 3, 0 },
	{ 0x86, "STX", E6502_MODE_ZPG,       2, 3, 0 },
	{ 0x87, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x88, "DEY", E6502_MODE_IMP,       1, 2, 0 },
	{ 0x89, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x8a, "TXA", E6502_MODE_IMP,       1, 2, 0 },
	{ 0x8b, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x8c, "STY", E6502_MODE_ABS,       3, 4, 0 },
	{ 0x8d, "STA", E6502_MODE_ABS,       3, 4, 0 },
	{ 0x8e, "STX", E6502_MODE_ABS,       3, 4, 0 },
	{ 0x8f, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x90, "BCC", E6502_MODE_BRA,       2, 2, E6502_OPF_BRA },
	{ 0x91, "STA", E6502_MODE_IND_IDX_Y, 2, 6, 0 },
	{ 0x92, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x93, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x94, "STY", E6502_MODE_ZPG_X,     2, 4, 0 },
	{ 0x95, "STA", E6502_MODE_ZPG_X,     2, 4, 0 },
	{ 0x96, "STX", E6502_MODE_ZPG_Y,     2, 4, 0 },
	{ 0x97, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x98, "TYA", E6502_MODE_IMP,       1, 2, 0 },
	{ 0x99, "STA", E6502_MODE_ABS_Y,     3, 5, 0 },
	{ 0x9a, "TXS", E6502_MODE_IMP,       1, 2, 0 },
	{ 0x9b, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x9c, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x9d, "STA", E6502_MODE_ABS_X,     3, 5, 0 },
	{ 0x9e, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0x9f, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xa0, "LDY", E6502_MODE_IMM,       2, 2, 0 },
	{ 0xa1, "LDA", E6502_MODE_IDX_IND_X, 2, 6, 0 },
	{ 0xa2, "LDX", E6502_MODE_IMM,       2, 2, 0 },
	{ 0xa3, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xa4, "LDY", E6502_MODE_ZPG,       2, 3, 0 },
	{ 0xa5, "LDA", E6502_MODE_ZPG,       2, 3, 0 },
	{ 0xa6, "LDX", E6502_MODE_ZPG,       2, 3, 0 },
	{ 0xa7, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xa8, "TAY", E6502_MODE_IMP,       1, 2, 0 },
	{ 0xa9, "LDA", E6502_MODE_IMM,       2, 2, 0 },
	{ 0xaa, "TAX", E6502_MODE_IMP,       1, 2, 0 },
	{ 0xab, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xac, "LDY", E6502_MODE_ABS,       3, 4, 0 },
	{ 0xad, "LDA", E6502_MODE_ABS,       3, 4, 0 },
	{ 0xae, "LDX", E6502_MODE_ABS,       3, 4, 0 },
	{ 0xaf, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xb0, "BCS", E6502_MODE_BRA,       2, 2, E6502_OPF_BRA },
	{ 0xb1, "LDA", E6502_MODE_IND_IDX_Y, 2, 5, 0 },
	{ 0xb2, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xb3, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xb4, "LDY", E6502_MODE_ZPG_X,     2, 4, 0 },
	{ 0xb5, "LDA", E6502_MODE_ZPG_X,     2, 4, 0 },
	{ 0xb6, "LDX", E6502_MODE_ZPG_Y,     2, 4, 0 },
	{ 0xb7, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xb8, "CLV", E6502_MODE_IMP,       1, 2, 0 },
	{ 0xb9, "LDA", E6502_MODE_ABS_Y,     3, 4, 0 },
	{ 0xba, "TSX", E6502_MODE_IMP,       1, 2, 0 },
	{ 0xbb, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xbc, "LDY", E6502_MODE_ABS_X,     3, 4, 0 },
	{ 0xbd, "LDA", E6502_MODE_ABS_X,     3, 4, 0 },
	{ 0xbe, "LDX", E6502_MODE_ABS_Y,     3, 4, 0 },
	{ 0xbf, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xc0, "CPY", E6502_MODE_IMM,       2, 2, 0 },
	{ 0xc1, "CMP", E6502_MODE_IDX_IND_X, 2, 6, 0 },
	{ 0xc2, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xc3, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xc4, "CPY", E6502_MODE_ZPG,       2, 3, 0 },
	{ 0xc5, "CMP", E6502_MODE_ZPG,       2, 3, 0 },
	{ 0xc6, "DEC", E6502_MODE_ZPG,       2, 5, 0 },
	{ 0xc7, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xc8, "INY", E6502_MODE_IMP,       1, 2, 0 },
	{ 0xc9, "CMP", E6502_MODE_IMM,       2, 2, 0 },
	{ 0xca, "DEX", E6502_MODE_IMP,       1, 2, 0 },
	{ 0xcb, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xcc, "CPY", E6502_MODE_ABS,       3, 4, 0 },
	{ 0xcd, "CMP", E6502_MODE_ABS,       3, 4, 0 },
	{ 0xce, "DEC", E6502_MODE_ABS,       3, 6, 0 },
	{ 0xcf, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xd0, "BNE", E6502_MODE_BRA,       2, 2, E6502_OPF_BRA },
	{ 0xd1, "CMP", E6502_MODE_IND_IDX_Y, 2, 5, 0 },
	{ 0xd2, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xd3, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xd4, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xd5, "CMP", E6502_MODE_ZPG_X,     2, 4, 0 },
	{ 0xd6, "DEC", E6502_MODE_ZPG_X,     2, 6, 0 },
	{ 0xd7, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xd8, "CLD", E6502_MODE_IMP,       1, 2, 0 },
	{ 0xd9, "CMP", E6502_MODE_ABS_Y,     3, 4, 0 },
	{ 0xda, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xdb, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xdc, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xdd, "CMP", E6502_MODE_ABS_X,     3, 4, 0 },
	{ 0xde, "DEC", E6502_MODE_ABS_X,     3, 7, 0 },
	{ 0xdf, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xe0, "CPX", E6502_MODE_IMM,       2, 2, 0 },
	{ 0xe1, "SBC", E6502_MODE_IDX_IND_X, 2, 6, 0 },
	{ 0xe2, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xe3, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xe4, "CPX", E6502_MODE_ZPG,       2, 3, 0 },
	{ 0xe5, "SBC", E6502_MODE_ZPG,       2, 3, 0 },
	{ 0xe6, "INC", E6502_MODE_ZPG,       2, 5, 0 },
	{ 0xe7, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xe8, "INX", E6502_MODE_IMP,       1, 2, 0 },
	{ 0xe9, "SBC", E6502_MODE_IMM,       2, 2, 0 },
	{ 0xea, "NOP", E6502_MODE_IMP,       1, 2, 0 },
	{ 0xeb, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xec, "CPX", E6502_MODE_ABS,       3, 4, 0 },
	{ 0xed, "SBC", E6502_MODE_ABS,       3, 4, 0 },
	{ 0xee, "INC", E6502_MODE_ABS,       3, 6, 0 },
	{ 0xef, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xf0, "BEQ", E6502_MODE_BRA,       2, 2, E6502_OPF_BRA },
	{ 0xf1, "SBC", E6502_MODE_IND_IDX_Y, 2, 5, 0 },
	{ 0xf2, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xf3, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xf4, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xf5, "SBC", E6502_MODE_ZPG_X,     2, 4, 0 },
	{ 0xf6, "INC", E6502_MODE_ZPG_X,     2, 6, 0 },
	{ 0xf7, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xf8, "SED", E6502_MODE_IMP,       1, 2, 0 },
	{ 0xf9, "SBC", E6502_MODE_ABS_Y,     3, 4, 0 },
	{ 0xfa, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xfb, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xfc, "DB",  E6502_MODE_DB,        1, 1, 0 },
	{ 0xfd, "SBC", E6502_MODE_ABS_X,     3, 4, 0 },
	{ 0xfe, "INC", E6502_MODE_ABS_X,     3, 7, 0 },
	{ 0xff, "DB",  E6502_MODE_DB,        1, 1, 0 }
};


void e6502_disasm (e6502_disasm_t *op, unsigned char *src, unsigned short pc)
{
	unsigned    i;
	unsigned    opc;
	e6502_dop_t *tab;

	opc = src[0] & 0xff;
	tab = &doptab[opc];

	op->pc = pc;
	op->flags = tab->flags;
	op->dat_n = tab->size;
	op->arg_n = 1;

	strcpy (op->op, tab->mnemonic);

	for (i = 0; i < tab->size; i++) {
		op->dat[i] = src[i];
	}

	switch (tab->mode) {
	case E6502_MODE_IDX_IND_X:
		sprintf (op->arg1, "[[%02x + X]]", (unsigned) src[1]);
		break;

	case E6502_MODE_ZPG:
		sprintf (op->arg1, "[%02x]", (unsigned) src[1]);
		break;

	case E6502_MODE_IMM:
		sprintf (op->arg1, "#%02X", src[1] & 0xff);
		break;

	case E6502_MODE_ABS:
		sprintf (op->arg1, "[%04X]", (unsigned) e6502_mk_uint16 (src[1], src[2]));
		break;

	case E6502_MODE_IND_IDX_Y:
		sprintf (op->arg1, "[[%02X] + Y]", (unsigned) src[1]);
		break;

	case E6502_MODE_ZPG_X:
		sprintf (op->arg1, "[%02X + X]", (unsigned) src[1]);
		break;

	case E6502_MODE_ZPG_Y:
		sprintf (op->arg1, "[%02X + Y]", (unsigned) src[1]);
		break;

	case E6502_MODE_ABS_Y:
		sprintf (op->arg1, "[%04X + Y]", (unsigned) e6502_mk_uint16 (src[1], src[2]));
		break;

	case E6502_MODE_ABS_X:
		sprintf (op->arg1, "[%04X + X]", (unsigned) e6502_mk_uint16 (src[1], src[2]));
		break;

	case E6502_MODE_IMP:
		op->arg_n = 0;
		break;

	case E6502_MODE_DB:
		sprintf (op->arg1, "#%02X", src[0] & 0xff);
		break;

	case E6502_MODE_BRA:
		sprintf (op->arg1, "%04X",
			(unsigned) ((op->pc + 2 + e6502_mk_sint16 (src[1])) & 0xffff)
		);
		break;

	case E6502_MODE_JMP:
		sprintf (op->arg1, "%04X", (unsigned) e6502_mk_uint16 (src[1], src[2]));
		break;

	case E6502_MODE_A:
		strcpy (op->arg1, "A");
		break;

	default:
		strcpy (op->arg1, "???");
		break;
	}
}

void e6502_disasm_mem (e6502_t *c, e6502_disasm_t *op, unsigned short pc)
{
	unsigned      i;
	unsigned char src[4];

	for (i = 0; i < 4; i++) {
		src[i] = e6502_get_mem8 (c, (pc + i) & 0xffff);
	}

	e6502_disasm (op, src, pc);
}

void e6502_disasm_cur (e6502_t *c, e6502_disasm_t *op)
{
	e6502_disasm_mem (c, op, e6502_get_pc (c));
}
