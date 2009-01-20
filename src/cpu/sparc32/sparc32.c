/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/sparc32/sparc32.c                                    *
 * Created:     2004-09-27 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sparc32.h"
#include "internal.h"


void s32_init (sparc32_t *c)
{
	c->mem_ext = NULL;

	c->get_uint8 = NULL;
	c->get_uint16 = NULL;
	c->get_uint32 = NULL;

	c->set_uint8 = NULL;
	c->set_uint16 = NULL;
	c->set_uint32 = NULL;

	c->log_ext = NULL;
	c->log_opcode = NULL;
	c->log_undef = NULL;
	c->log_exception = NULL;

	s32_set_opcodes (c);

	c->nwindows = 4;

	c->psr = 0;

	c->oprcnt = 0;
	c->clkcnt = 0;
}

sparc32_t *s32_new (void)
{
	sparc32_t *c;

	c = (sparc32_t *) malloc (sizeof (sparc32_t));
	if (c == NULL) {
		return (NULL);
	}

	s32_init (c);

	return (c);
}

void s32_free (sparc32_t *c)
{
}

void s32_del (sparc32_t *c)
{
	if (c != NULL) {
		s32_free (c);
		free (c);
	}
}


void s32_set_mem_fct (sparc32_t *c, void *ext,
	void *get8, void *get16, void *get32,
	void *set8, void *set16, void *set32)
{
	c->mem_ext = ext;

	c->get_uint8 = get8;
	c->get_uint16 = get16;
	c->get_uint32 = get32;

	c->set_uint8 = set8;
	c->set_uint16 = set16;
	c->set_uint32 = set32;
}

void s32_set_nwindows (sparc32_t *c, unsigned n)
{
	if (n < 2) {
		c->nwindows = 2;
	}
	else if (n > 31) {
		c->nwindows = 32;
	}
	else {
		c->nwindows = n;
	}
}

static
int s32_get_reg_idx (const char *reg, unsigned *idx, unsigned max)
{
	unsigned n;

	n = 0;

	if ((reg[0] < '0') || (reg[0] > '9')) {
		return (1);
	}

	while ((reg[0] >= '0') && (reg[0] <= '9')) {
		n = 10 * n + (unsigned) (reg[0] - '0');
		reg += 1;
	}

	if (reg[0] != 0) {
		return (1);
	}

	if (n > max) {
		return (1);
	}

	*idx = n;

	return (0);
}

int s32_get_reg (sparc32_t *c, const char *reg, unsigned long *val)
{
	unsigned idx;

	if (reg[0] == '%') {
		reg += 1;
	}

	if (strcmp (reg, "npc") == 0) {
		*val = s32_get_npc (c);
		return (0);
	}
	else if (strcmp (reg, "psr") == 0) {
		*val = s32_get_psr (c);
		return (0);
	}
	else if (strcmp (reg, "pc") == 0) {
		*val = s32_get_pc (c);
		return (0);
	}
	else if (strcmp (reg, "tbr") == 0) {
		*val = s32_get_tbr (c);
		return (0);
	}
	else if (strcmp (reg, "wim") == 0) {
		*val = s32_get_wim (c);
		return (0);
	}
	else if (strcmp (reg, "y") == 0) {
		*val = s32_get_y (c);
		return (0);
	}

	if (reg[0] == 'r') {
		if (s32_get_reg_idx (reg + 1, &idx, 31)) {
			return (1);
		}

		*val = s32_get_gpr (c, idx);

		return (0);
	}

	return (1);
}

int s32_set_reg (sparc32_t *c, const char *reg, unsigned long val)
{
	unsigned idx;

	if (reg[0] == '%') {
		reg += 1;
	}

	if (strcmp (reg, "npc") == 0) {
		s32_set_npc (c, val);
		return (0);
	}
	else if (strcmp (reg, "psr") == 0) {
		s32_set_psr (c, val);
		return (0);
	}
	else if (strcmp (reg, "pc") == 0) {
		s32_set_pc (c, val);
		return (0);
	}
	else if (strcmp (reg, "tbr") == 0) {
		s32_set_tbr (c, val);
		return (0);
	}
	else if (strcmp (reg, "wim") == 0) {
		s32_set_wim (c, val);
		return (0);
	}
	else if (strcmp (reg, "y") == 0) {
		s32_set_y (c, val);
		return (0);
	}

	if (reg[0] == 'r') {
		if (s32_get_reg_idx (reg + 1, &idx, 31)) {
			return (1);
		}

		s32_set_gpr (c, idx, val);

		return (0);
	}

	return (1);
}

unsigned long long s32_get_opcnt (sparc32_t *c)
{
	return (c->oprcnt);
}

unsigned long long s32_get_clkcnt (sparc32_t *c)
{
	return (c->clkcnt);
}

unsigned long s32_get_delay (sparc32_t *c)
{
	return (c->delay);
}


void s32_reset (sparc32_t *c)
{
	unsigned i, n;

	for (i = 0; i < 32; i++) {
		c->reg[i] = 0;
	}

	n = 16 * c->nwindows;
	for (i = 0; i < n; i++) {
		c->regstk[i] = 0;
	}

	c->psr = S32_PSR_S;

	c->pc = 0x00000000UL;
	c->npc = 0x00000004UL;
	c->tbr = 0;
	c->wim = 0;
	c->y = 0;

	c->asi = 0;
	c->asi_text = S32_ASI_STEXT;
	c->asi_data = S32_ASI_SDATA;

	c->ir = 0;

	c->interrupt = 0;

	c->delay = 1;

	c->oprcnt = 0;
	c->clkcnt = 0;
}

void s32_regstk_load (sparc32_t *c, unsigned wdw)
{
	if (wdw == 0) {
		memcpy (c->reg + 16, c->regstk, 16 * sizeof (uint32_t));
		memcpy (c->reg + 8, c->regstk + (16 * c->nwindows - 8), 8 * sizeof (uint32_t));
	}
	else {
		memcpy (c->reg + 8, c->regstk + (16 * wdw - 8), 24 * sizeof (uint32_t));
	}
}

void s32_regstk_save (sparc32_t *c, unsigned wdw)
{
	if (wdw == 0) {
		memcpy (c->regstk, c->reg + 16, 16 * sizeof (uint32_t));
		memcpy (c->regstk + (16 * c->nwindows - 8), c->reg + 8, 8 * sizeof (uint32_t));
	}
	else {
		memcpy (c->regstk + (16 * wdw - 8), c->reg + 8, 24 * sizeof (uint32_t));
	}
}

int s32_save (sparc32_t *c, int check)
{
	unsigned cwp1, cwp2;

	cwp1 = s32_get_cwp (c);
	cwp2 = (cwp1 == 0) ? (c->nwindows - 1) : (cwp1 - 1);

	if (check) {
		if (s32_get_wim (c) & (1UL << cwp2)) {
			return (1);
		}
	}

	s32_regstk_save (c, cwp1);
	s32_set_cwp (c, cwp2);
	s32_regstk_load (c, cwp2);

	if (s32_get_wim (c) & (1UL << cwp2)) {
		return (1);
	}

	return (0);
}

int s32_restore (sparc32_t *c, int check)
{
	unsigned cwp1, cwp2;

	cwp1 = s32_get_cwp (c);
	cwp2 = cwp1 + 1;
	if (cwp2 >= c->nwindows) {
		cwp2 = 0;
	}

	if (check) {
		if (s32_get_wim (c) & (1UL << cwp2)) {
			return (1);
		}
	}

	s32_regstk_save (c, cwp1);
	s32_set_cwp (c, cwp2);
	s32_regstk_load (c, cwp2);

	if (s32_get_wim (c) & (1UL << cwp2)) {
		return (1);
	}

	return (0);
}

void s32_undefined (sparc32_t *c)
{
	if (c->log_undef != NULL) {
		c->log_undef (c->log_ext, c->ir);
	}
}

void s32_trap (sparc32_t *c, unsigned tn)
{
	uint32_t psr;

	tn &= 0xff;

	if (c->log_exception != NULL) {
		c->log_exception (c->log_ext, tn);
	}

	c->delay += 1;

	psr = s32_get_psr (c);

	if ((psr & S32_PSR_ET) == 0) {
		/* oops... */
		fprintf (stderr, "sparc32: trap 0x%02x while ET=0\n", tn);
	}

	psr &= ~S32_PSR_ET;

	if (psr & S32_PSR_S) {
		psr |= S32_PSR_PS;
	}
	else {
		psr &= ~S32_PSR_PS;
		psr |= S32_PSR_S;

		c->asi_text = S32_ASI_STEXT;
		c->asi_data = S32_ASI_SDATA;
	}

	s32_set_psr (c, psr);

	s32_save (c, 0);

	s32_set_gpr (c, 17, s32_get_pc (c));
	s32_set_gpr (c, 18, s32_get_npc (c));

	s32_set_tbr_tt (c, tn);

	s32_set_pc (c, s32_get_tbr (c));
	s32_set_npc (c, (s32_get_tbr (c) + 4) & 0xffffffffUL);
}

void s32_interrupt (sparc32_t *c, unsigned char val)
{
	c->interrupt = (val != 0);
}

void s32_execute (sparc32_t *c)
{
	if (s32_ifetch (c, c->pc, c->asi_text, &c->ir)) {
		return;
	}

	if (c->log_opcode != NULL) {
		c->log_opcode (c->log_ext, c->ir);
	}

	switch ((c->ir >> 30) & 0x03) {
	case 0:
		c->opcodes[0][(c->ir >> 22) & 0x07] (c);
		break;

	case 1:
		c->opcodes[1][0] (c);
		break;

	case 2:
		c->opcodes[2][(c->ir >> 19) & 0x3f] (c);
		break;

	case 3:
		c->opcodes[3][(c->ir >> 19) & 0x3f] (c);
		break;
	}

	c->oprcnt += 1;

	if (c->interrupt) {
		if (s32_get_psr_et (c)) {
			/* ... */
		}
	}
}

void s32_clock (sparc32_t *c, unsigned long n)
{
	while (n >= c->delay) {
		if (c->delay == 0) {
			fprintf (stderr, "warning: delay == 0 at %08lx\n", (unsigned long) s32_get_pc (c));
			fflush (stderr);
			c->delay = 1;
		}

		n -= c->delay;

		c->clkcnt += c->delay;
		c->delay = 0;

		s32_execute (c);
	}

	c->clkcnt += n;
	c->delay -= n;
}
