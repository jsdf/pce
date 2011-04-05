/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/arm/copr15.c                                         *
 * Created:     2004-11-09 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2011 Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2006 Lukas Ruf <ruf@lpr.ch>                         *
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


#include "arm.h"
#include "internal.h"


static int cp15_reset (arm_t *c, arm_copr_t *p);
static int cp15_exec (arm_t *c, arm_copr_t *p);


void cp15_init (arm_copr15_t *p)
{
	unsigned i;

	arm_copr_init (&p->copr);

	p->copr.ext = p;
	p->copr.reset = cp15_reset;
	p->copr.exec = cp15_exec;

	p->reg[0] = ARM_C15_ID;
	p->reg[1] = ARM_C15_CR_P | ARM_C15_CR_D | ARM_C15_CR_L;

	for (i = 2; i < 16; i++) {
		p->reg[i] = 0;
	}

	p->cache_type = 0;
	p->auxiliary_control = 0;
}

arm_copr_t *cp15_new (void)
{
	arm_copr15_t *c;

	c = malloc (sizeof (arm_copr15_t));
	if (c == NULL) {
		return (NULL);
	}

	cp15_init (c);

	return (&c->copr);
}

void cp15_free (arm_copr15_t *p)
{
}

void cp15_del (arm_copr15_t *p)
{
	if (p != NULL) {
		cp15_free (p);
	}

	free (p);
}


/*
 * Get CP15/0 (ID)
 */
static
int cp15_get_reg0 (arm_t *c, arm_copr15_t *p, unsigned op2, uint32_t *val)
{
	switch (op2) {
	case 0:
		/* ID */
		*val = p->reg[0];
		return (0);

	case 1:
		/* XScale Cache Type */
		*val = p->cache_type;
		return (0);
	}

	return (1);
}

/*
 * Get CP15/1 (Control)
 */
static
int cp15_get_reg1 (arm_t *c, arm_copr15_t *p, unsigned op2, uint32_t *val)
{
	switch (op2) {
	case 0:
		/* Control */
		*val = p->reg[1];
		return (0);

	case 1:
		/* XScale Auxiliary Control */
		*val = p->auxiliary_control;
		return (0);
	}

	return (1);
}

/*
 * Set CP15/1 (Control)
 */
static
int cp15_set_reg1 (arm_t *c, arm_copr15_t *p, unsigned op2, uint32_t val)
{
	switch (op2) {
	case 0:
		/* Control */
		val &= ~ARM_C15_CR_C;
		val &= ~ARM_C15_CR_W;
		val |= ARM_C15_CR_P;
		val |= ARM_C15_CR_D;
		val |= ARM_C15_CR_L;
		arm_set_bits (val, ARM_C15_CR_B, c->bigendian);
		p->reg[1] = val & 0xffffffff;

		c->exception_base = (val & ARM_C15_CR_V) ? 0xffff0000 : 0x00000000;

		return (0);

	case 1:
		/* XScale Auxiliary Control */
		p->auxiliary_control = val;

		return (0);
	}

	return (1);
}

/* cache functions */
static
int cp15_set_reg7 (arm_t *c, arm_copr15_t *p)
{
	unsigned rm, op2;

	rm = arm_ir_rm (c->ir);
	op2 = arm_get_bits (c->ir, 5, 3);

	if ((rm == 7) && (op2 == 0)) {
		/* invalidate all caches */
		return (0);
	}
	else if ((rm == 2) && (op2 == 5)) {
		/* ??? */
		return (0);
	}
	else if (rm == 5) {
		switch (op2) {
		case 0x00:
			/* invalidate entire instruction cache */
			return (0);

		case 0x01:
			/* invalidate instruction cache line */
			return (0);

		case 0x02:
			/* invalidate instruction cache line */
			return (0);

		case 0x04:
			/* flush prefetch buffer */
			return (0);

		case 0x06:
			/* flush entire branch target cache */
			return (0);

		case 0x07:
			/* flush branch target cache entry */
			return (0);
		}

		return (1);
	}
	else if (rm == 6) {
		switch (op2) {
		case 0x00:
			/* invalidate entire data cache */
			return (0);

		case 0x01:
			/* invalidate data cache line */
			return (0);

		case 0x02:
			/* invalidate data cache line */
			return (0);
		}
	}
	else if (rm == 10) {
		switch (op2) {
		case 0x01:
			/* clean data cache line */
			return (0);

		case 0x02:
			/* clean data cache line */
			return (0);

		case 0x04:
			/* drain write buffer */
			return (0);
		}

		return (1);
	}

	return (1);
}

/* TLB functions */
static
int cp15_set_reg8 (arm_t *c, arm_copr15_t *p)
{
	unsigned rm, op2;

	rm = arm_ir_rm (c->ir);
	op2 = arm_get_bits (c->ir, 5, 3);

	if (rm == 5) {
		switch (op2) {
		case 0x00:
			/* invalidate entire instruction tlb */
			return (0);

		case 0x01:
			/* invalidate instruction tlb single entry */
			return (0);
		}
	}
	else if (rm == 6) {
		switch (op2) {
		case 0x00:
			/* invalidate entire data tlb */
			return (0);

		case 0x01:
			/* invalidate data tlb single entry */
			return (0);
		}
	}
	else if (rm == 7) {
		switch (op2) {
		case 0x00:
			/* invalidate entire unified tlb */
			return (0);

		case 0x01:
			/* invalidate unified tlb single entry */
			return (0);
		}
	}

	return (1);
}

static
int cp15_set_reg15 (arm_t *c, arm_copr15_t *p, uint32_t val)
{
	unsigned rm;

	rm = arm_ir_rm (c->ir);
	/* op2 = arm_get_bits (c->ir, 5, 3); */

	if (rm == 1) {
		/* xscale: coprocessor access register */
		p->reg[15] = val & 0x00003fff;
		return (0);
	}

	return (1);
}

static
int cp15_op_mrc (arm_t *c, arm_copr_t *p)
{
	arm_copr15_t *p15;
	unsigned     op2;
	uint32_t     val;

	p15 = p->ext;

	op2 = arm_get_bits (c->ir, 5, 3);

	switch (arm_ir_rn (c->ir)) {
	case 0x00: /* ID register */
		if (cp15_get_reg0 (c, p15, op2, &val)) {
			return (1);
		}
		break;

	case 0x01: /* control register */
		if (cp15_get_reg1 (c, p15, op2, &val)) {
			return (1);
		}
		break;

	case 0x02: /* translation table base */
		val = p15->reg[2];
		break;

	case 0x03: /* domain access control */
		val = p15->reg[2];
		break;

	case 0x05: /* fault status */
		val = p15->reg[5];
		break;

	case 0x06: /* fault address */
		val = p15->reg[6];
		break;

	case 0x0f: /* implementation defined */
		val = p15->reg[15];
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
int cp15_op_mcr (arm_t *c, arm_copr_t *p)
{
	arm_copr15_t *p15;
	unsigned     op2;
	uint32_t     val;

	p15 = p->ext;

	op2 = arm_get_bits (c->ir, 5, 3);

	val = arm_get_rd (c, c->ir);

	/* conservative flushing of translation buffer */
	arm_tbuf_flush (c);

	switch (arm_ir_rn (c->ir)) {
	case 0x00: /* id register */
		return (1);

	case 0x01: /* control register */
		return (cp15_set_reg1 (c, p15, op2, val));

	case 0x02: /* translation table base */
		p15->reg[2] = val & 0xffffc000;
		break;

	case 0x03: /* domain access control */
		p15->reg[3] = val & 0xffffffff;
		break;

	case 0x07:
		return (cp15_set_reg7 (c, p15));

	case 0x08:
		return (cp15_set_reg8 (c, p15));

	case 0x0f: /* implementation defined */
		return (cp15_set_reg15 (c, p15, val));

	default:
		return (1);
	}

	return (0);
}

static
int cp15_reset (arm_t *c, arm_copr_t *p)
{
	unsigned     i;
	arm_copr15_t *p15;

	p15 = p->ext;

	if (c->bigendian) {
		p15->reg[1] |= ARM_C15_CR_B;
	}
	else {
		p15->reg[1] &= ~ARM_C15_CR_B;
	}

	for (i = 2; i < 16; i++) {
		p15->reg[i] = 0;
	}

	return (0);
}

static
int cp15_exec (arm_t *c, arm_copr_t *p)
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
		r = cp15_op_mcr (c, p);
		break;

	case 0x00100010: /* mrc */
		op = "R";
		r = cp15_op_mrc (c, p);
		break;

	default:
		r = 1;
		break;
	}

	if (r == 0) {
		arm_set_clk (c, 4, 1);
	}

	if (r) {
		fprintf (stderr, "%08lX C15: %s Rd=%u Rn=%u Rm=%u op2=%u\n",
			pc, op,
			(unsigned) arm_ir_rd (c->ir),
			(unsigned) arm_ir_rn (c->ir),
			(unsigned) arm_ir_rm (c->ir),
			(unsigned) arm_get_bits (c->ir, 5, 3)
		); fflush (stderr);
	}

	return (r);
}
