/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/arm/copr14.c                                         *
 * Created:     2007-02-16 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#include "arm.h"
#include "internal.h"


static int cp14_reset (arm_t *c, arm_copr_t *p);
static int cp14_exec (arm_t *c, arm_copr_t *p);


void cp14_init (arm_copr14_t *c)
{
	arm_copr_init (&c->copr);

	c->copr.ext = c;
	c->copr.reset = cp14_reset;
	c->copr.exec = cp14_exec;

	c->cclkcfg = 0;
	c->pwrmode = 0;
}

arm_copr_t *cp14_new (void)
{
	arm_copr14_t *c;

	c = malloc (sizeof (arm_copr14_t));
	if (c == NULL) {
		return (NULL);
	}

	cp14_init (c);

	return (&c->copr);
}

void cp14_free (arm_copr14_t *p)
{
}

void cp14_del (arm_copr14_t *p)
{
	if (p != NULL) {
		cp14_free (p);
	}

	free (p);
}

static
int cp14_op_mrc (arm_t *c, arm_copr_t *p)
{
	arm_copr14_t *p14;
	unsigned     rn, rm;
	uint32_t     val;

	p14 = p->ext;

	rn = arm_ir_rn (c->ir);
	rm = arm_ir_rm (c->ir);
	/* op2 = arm_get_bits (c->ir, 5, 3); */

	switch ((rm << 4) | rn) {
	case 0x00: /* xsc1 performance monitor control register */
		val = 0;
		break;

	case 0x06: /* CCLKCFG */
		val = p14->cclkcfg;
		break;

	case 0x07: /* PWRMODE */
		val = p14->pwrmode;
		break;

	default:
		return (1);
	}

	if (arm_rd_is_pc (c->ir)) {
		arm_set_cpsr (c, (arm_get_cpsr (c) & ~ARM_PSR_CC) | (val & ARM_PSR_CC));
	}
	else {
		arm_set_rd (c, c->ir, val & 0xffffffff);
	}

	return (0);
}

static
int cp14_op_mcr (arm_t *c, arm_copr_t *p)
{
	arm_copr14_t *p14;
	unsigned     rn, rm;
	uint32_t     val;

	p14 = p->ext;

	rn = arm_ir_rn (c->ir);
	rm = arm_ir_rm (c->ir);
	/* op2 = arm_get_bits (c->ir, 5, 3); */

	val = arm_get_rd (c, c->ir);

	switch ((rm << 4) | rn) {
	case 0x00: /* xsc1 performance monitor control register */
		break;

	case 0x06: /* CCLKCFG */
		p14->cclkcfg = val & 0x0000000f;
		break;

	case 0x07: /* PWRMODE */
		p14->pwrmode = val & 0x0000000f;
		break;

	default:
		return (1);
	}

	return (0);
}

int cp14_reset (arm_t *c, arm_copr_t *p)
{
	arm_copr14_t *p14;

	p14 = p->ext;

	p14->cclkcfg = 0;
	p14->pwrmode = 0;

	return (0);
}

static
int cp14_exec (arm_t *c, arm_copr_t *p)
{
	int           r;
	unsigned long pc;
	char          *op;

	pc = arm_get_pc (c);

	if (arm_is_privileged (c) == 0) {
		return (1);
	}

	op = "?";

	switch (c->ir & 0x00f00010) {
	case 0x00000010: /* mcr */
		op = "W";
		r = cp14_op_mcr (c, p);
		break;

	case 0x00100010: /* mrc */
		op = "R";
		r = cp14_op_mrc (c, p);
		break;

	default:
		r = 1;
		break;
	}

	if (r == 0) {
		arm_set_clk (c, 4, 1);
	}

	if (r) {
		fprintf (stderr, "%08lX C14: %s Rd=%u Rn=%u Rm=%u op2=%u\n",
			pc, op,
			(unsigned) arm_ir_rd (c->ir),
			(unsigned) arm_ir_rn (c->ir),
			(unsigned) arm_ir_rm (c->ir),
			(unsigned) arm_get_bits (c->ir, 5, 3)
		); fflush (stderr);
	}

	return (r);
}
