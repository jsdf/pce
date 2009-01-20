/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/ppc405/opcode13.c                                    *
 * Created:     2003-11-10 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2009 Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2006 Lukas Ruf <ruf@lpr.ch>                         *
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

/*****************************************************************************
 * This software was developed at the Computer Engineering and Networks      *
 * Laboratory (TIK), Swiss Federal Institute of Technology (ETH) Zurich.     *
 *****************************************************************************/


#include <stdlib.h>

#include "ppc405.h"
#include "internal.h"


#define crop_and  ((0x0c & 0x0a) & 0x0f)
#define crop_andc ((0x0c & ~0x0a) & 0x0f)
#define crop_nand (~(0x0c & 0x0a) & 0x0f)
#define crop_xor  ((0x0c ^ 0x0a) & 0x0f)
#define crop_eqv  (~(0x0c ^ 0x0a) & 0x0f)
#define crop_or   ((0x0c | 0x0a) & 0x0f)
#define crop_orc  ((0x0c | ~0x0a) & 0x0f)
#define crop_nor  (~(0x0c | 0x0a) & 0x0f)


/* 13 000: mcrf bf, bfa */
static
void op_13_000 (p405_t *c)
{
	unsigned bft, bfa;

	if (p405_check_reserved (c, 0x0063f801UL)) {
		return;
	}

	bft = p405_get_ir_rt (c->ir) >> 2;
	bfa = p405_get_ir_ra (c->ir) >> 2;

	p405_set_crf (c, bft, p405_get_crf (c, bfa));

	p405_set_clk (c, 4, 1);
}

/* 13 010: bclr/bclrl */
static
void op_13_010 (p405_t *c)
{
	p405_op_branch (c,
		p405_get_lr (c) & 0xfffffffcUL,
		(c->ir >> 21) & 0x1f, (c->ir >> 16) & 0x1f,
		1, (c->ir & P405_IR_LK) != 0
	);
}

/* 13 021: crnor bt, ba, bb */
static
void op_13_021 (p405_t *c)
{
	p405_op_crop (c,
		p405_get_ir_rt (c->ir), p405_get_ir_ra (c->ir), p405_get_ir_rb (c->ir),
		crop_nor
	);
}

/* 13 032: rfi */
static
void op_13_032 (p405_t *c)
{
	if (p405_check_reserved (c, 0x03fff801UL)) {
		return;
	}

	if (p405_check_privilege (c)) {
		return;
	}

	p405_set_pc (c, p405_get_srr (c, 0));
	p405_set_msr (c, p405_get_srr (c, 1));

	p405_set_clk (c, 0, 1);
}

/* 13 033: rfci */
static
void op_13_033 (p405_t *c)
{
	if (p405_check_reserved (c, 0x03fff801UL)) {
		return;
	}

	if (p405_check_privilege (c)) {
		return;
	}

	p405_set_pc (c, p405_get_srr (c, 2));
	p405_set_msr (c, p405_get_srr (c, 3));

	p405_set_clk (c, 0, 1);
}

/* 13 081: crandc bt, ba, bb */
static
void op_13_081 (p405_t *c)
{
	p405_op_crop (c,
		p405_get_ir_rt (c->ir), p405_get_ir_ra (c->ir), p405_get_ir_rb (c->ir),
		crop_andc
	);
}

/* 13 096: isync */
static
void op_13_096 (p405_t *c)
{
	if (p405_check_reserved (c, 0x03fff800UL)) {
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 13 0c1: crxor bt, ba, bb */
static
void op_13_0c1 (p405_t *c)
{
	p405_op_crop (c,
		p405_get_ir_rt (c->ir), p405_get_ir_ra (c->ir), p405_get_ir_rb (c->ir),
		crop_xor
	);
}

/* 13 0e1: crnand bt, ba, bb */
static
void op_13_0e1 (p405_t *c)
{
	p405_op_crop (c,
		p405_get_ir_rt (c->ir), p405_get_ir_ra (c->ir), p405_get_ir_rb (c->ir),
		crop_xor
	);
}

/* 13 101: crand bt, ba, bb */
static
void op_13_101 (p405_t *c)
{
	p405_op_crop (c,
		p405_get_ir_rt (c->ir), p405_get_ir_ra (c->ir), p405_get_ir_rb (c->ir),
		crop_and
	);
}

/* 13 121: creqv bt, ba, bb */
static
void op_13_121 (p405_t *c)
{
	p405_op_crop (c,
		p405_get_ir_rt (c->ir), p405_get_ir_ra (c->ir), p405_get_ir_rb (c->ir),
		crop_eqv
	);
}

/* 13 1a1: crorc bt, ba, bb */
static
void op_13_1a1 (p405_t *c)
{
	p405_op_crop (c,
		p405_get_ir_rt (c->ir), p405_get_ir_ra (c->ir), p405_get_ir_rb (c->ir),
		crop_orc
	);
}

/* 13 1c1: cror bt, ba, bb */
static
void op_13_1c1 (p405_t *c)
{
	p405_op_crop (c,
		p405_get_ir_rt (c->ir), p405_get_ir_ra (c->ir), p405_get_ir_rb (c->ir),
		crop_or
	);
}

/* 13 210: bcctr/bcctrl */
static
void op_13_210 (p405_t *c)
{
	p405_op_branch (c,
		p405_get_ctr (c) & 0xfffffffcUL,
		(c->ir >> 21) & 0x1f, (c->ir >> 16) & 0x1f,
		1, (c->ir & P405_IR_LK) != 0
	);
}

/* 13: */
static
void op_13 (p405_t *c)
{
	unsigned op2;

	op2 = (c->ir >> 1) & 0x3ff;

	c->opcodes.op13[op2] (c);
}


p405_opcode_f p405_opcode13[1024] = {
	&op_13_000,       NULL,       NULL,       NULL,   /* 000 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
	&op_13_010,       NULL,       NULL,       NULL,   /* 010 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL, &op_13_021,       NULL,       NULL,   /* 020 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL, &op_13_032, &op_13_033,   /* 030 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 040 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 050 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 060 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 070 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL, &op_13_081,       NULL,       NULL,   /* 080 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 090 */
				NULL,       NULL, &op_13_096,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 0a0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 0b0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL, &op_13_0c1,       NULL,       NULL,   /* 0c0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 0d0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL, &op_13_0e1,       NULL,       NULL,   /* 0e0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 0f0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL, &op_13_101,       NULL,       NULL,   /* 100 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 110 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL, &op_13_121,       NULL,       NULL,   /* 120 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 130 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 140 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 150 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 160 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 170 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 180 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 190 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL, &op_13_1a1,       NULL,       NULL,   /* 1a0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 1b0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL, &op_13_1c1,       NULL,       NULL,   /* 1c0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 1d0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 1e0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 1f0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 200 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
	&op_13_210,       NULL,       NULL,       NULL,   /* 210 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 220 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 230 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 240 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 250 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 260 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 270 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 280 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 290 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 2a0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 2b0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 2c0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 2d0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 2e0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 2f0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 300 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 310 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 320 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 330 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 340 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 350 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 360 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 370 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 380 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 390 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 3a0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 3b0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 3c0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 3d0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 3e0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,   /* 3f0 */
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL,
				NULL,       NULL,       NULL,       NULL
};

void p405_set_opcode13 (p405_t *c)
{
	unsigned i;

	c->opcodes.op[0x13] = &op_13;

	for (i = 0; i < 1024; i++) {
		if (p405_opcode13[i] != NULL) {
			c->opcodes.op13[i] = p405_opcode13[i];
		}
		else {
			c->opcodes.op13[i] = &p405_op_undefined;
		}
	}
}
