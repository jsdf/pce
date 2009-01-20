/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/ppc405/ppc405.c                                      *
 * Created:     2003-11-07 by Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ppc405.h"
#include "internal.h"


void p405_init (p405_t *c)
{
	c->mem_ext = NULL;

	c->get_uint8 = NULL;
	c->get_uint16 = NULL;
	c->get_uint32 = NULL;

	c->set_uint8 = NULL;
	c->set_uint16 = NULL;
	c->set_uint32 = NULL;

	c->ram = NULL;
	c->ram_cnt = 0;

	c->dcr_ext = NULL;
	c->get_dcr = NULL;
	c->set_dcr = NULL;

	c->log_ext = NULL;
	c->log_opcode = NULL;
	c->log_undef = NULL;
	c->log_exception = NULL;
	c->log_mem = NULL;

	c->hook_ext = NULL;
	c->hook = NULL;

	p405_set_opcodes (c);
	p405_tlb_init (&c->tlb);

	c->pvr = P405_PVR_405GP;

	c->timer_scale = 1;
}

p405_t *p405_new (void)
{
	p405_t *c;

	c = malloc (sizeof (p405_t));
	if (c == NULL) {
		return (NULL);
	}

	p405_init (c);

	return (c);
}

void p405_free (p405_t *c)
{
}

void p405_del (p405_t *c)
{
	if (c != NULL) {
		p405_free (c);
		free (c);
	}
}


void p405_set_mem_fct (p405_t *c, void *ext,
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

void p405_set_ram (p405_t *c, unsigned char *ram, unsigned long cnt)
{
	c->ram = ram;
	c->ram_cnt = cnt;
}

void p405_set_dcr_fct (p405_t *c, void *ext, void *get, void *set)
{
	c->dcr_ext = ext;
	c->get_dcr = get;
	c->set_dcr = set;
}

void p405_set_hook_fct (p405_t *c, void *ext, void *fct)
{
	c->hook_ext = ext;
	c->hook = fct;
}


void p405_set_timer_scale (p405_t *c, unsigned scale)
{
	c->timer_scale = scale;
}

unsigned long long p405_get_opcnt (p405_t *c)
{
	return (c->oprcnt);
}

unsigned long long p405_get_clkcnt (p405_t *c)
{
	return (c->clkcnt);
}

unsigned long p405_get_delay (p405_t *c)
{
	return (c->delay);
}

static
int p405_get_reg_idx (const char *reg, unsigned *idx, unsigned max)
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

int p405_get_reg (p405_t *c, const char *reg, unsigned long *val)
{
	unsigned idx;

	if (reg[0] == '%') {
		reg += 1;
	}

	if (strcmp (reg, "cr") == 0) {
		*val = p405_get_cr (c);
		return (0);
	}
	else if (strcmp (reg, "ctr") == 0) {
		*val = p405_get_ctr (c);
		return (0);
	}
	else if (strcmp (reg, "dear") == 0) {
		*val = p405_get_dear (c);
		return (0);
	}
	else if (strcmp (reg, "esr") == 0) {
		*val = p405_get_esr (c);
		return (0);
	}
	else if (strcmp (reg, "evpr") == 0) {
		*val = p405_get_evpr (c);
		return (0);
	}
	else if (strcmp (reg, "lr") == 0) {
		*val = p405_get_lr (c);
		return (0);
	}
	else if (strcmp (reg, "msr") == 0) {
		*val = p405_get_msr (c);
		return (0);
	}
	else if (strcmp (reg, "pc") == 0) {
		*val = p405_get_pc (c);
		return (0);
	}
	else if (strcmp (reg, "pid") == 0) {
		*val = p405_get_pid (c);
		return (0);
	}
	else if (strcmp (reg, "pit1") == 0) {
		*val = p405_get_pit (c, 1);
		return (0);
	}
	else if (strcmp (reg, "pit") == 0) {
		*val = p405_get_pit (c, 0);
		return (0);
	}
	else if (strcmp (reg, "tbl") == 0) {
		*val = p405_get_tbl (c);
		return (0);
	}
	else if (strcmp (reg, "tbu") == 0) {
		*val = p405_get_tbu (c);
		return (0);
	}
	else if (strcmp (reg, "tcr") == 0) {
		*val = p405_get_tcr (c);
		return (0);
	}
	else if (strcmp (reg, "tsr") == 0) {
		*val = p405_get_tsr (c);
		return (0);
	}
	else if (strcmp (reg, "xer") == 0) {
		*val = p405_get_xer (c);
		return (0);
	}
	else if (strcmp (reg, "zpr") == 0) {
		*val = p405_get_zpr (c);
		return (0);
	}

	if (strncmp (reg, "sprg", 4) == 0) {
		if (p405_get_reg_idx (reg + 4, &idx, 7)) {
			return (1);
		}

		*val = p405_get_sprg (c, idx);

		return (0);
	}

	if (strncmp (reg, "srr", 3) == 0) {
		if (p405_get_reg_idx (reg + 3, &idx, 3)) {
			return (1);
		}

		*val = p405_get_srr (c, idx);

		return (0);
	}

	if (reg[0] == 'r') {
		if (p405_get_reg_idx (reg + 1, &idx, 31)) {
			return (1);
		}

		*val = p405_get_gpr (c, idx);

		return (0);
	}

	return (1);
}

int p405_set_reg (p405_t *c, const char *reg, unsigned long val)
{
	unsigned idx;

	if (reg[0] == '%') {
		reg += 1;
	}

	if (strcmp (reg, "cr") == 0) {
		p405_set_cr (c, val);
		return (0);
	}
	else if (strcmp (reg, "ctr") == 0) {
		p405_set_ctr (c, val);
		return (0);
	}
	else if (strcmp (reg, "dear") == 0) {
		p405_set_dear (c, val);
		return (0);
	}
	else if (strcmp (reg, "esr") == 0) {
		p405_set_esr (c, val);
		return (0);
	}
	else if (strcmp (reg, "evpr") == 0) {
		p405_set_evpr (c, val);
		return (0);
	}
	else if (strcmp (reg, "lr") == 0) {
		p405_set_lr (c, val);
		return (0);
	}
	else if (strcmp (reg, "msr") == 0) {
		p405_set_msr (c, val);
		return (0);
	}
	else if (strcmp (reg, "pc") == 0) {
		p405_set_pc (c, val);
		return (0);
	}
	else if (strcmp (reg, "pid") == 0) {
		p405_set_pid (c, val);
		return (0);
	}
	else if (strcmp (reg, "pit1") == 0) {
		p405_set_pit (c, 1, val);
		return (0);
	}
	else if (strcmp (reg, "pit") == 0) {
		p405_set_pit (c, 0, val);
		return (0);
	}
	else if (strcmp (reg, "tbl") == 0) {
		p405_set_tbl (c, val);
		return (0);
	}
	else if (strcmp (reg, "tbu") == 0) {
		p405_set_tbu (c, val);
		return (0);
	}
	else if (strcmp (reg, "tcr") == 0) {
		p405_set_tcr (c, val);
		return (0);
	}
	else if (strcmp (reg, "tsr") == 0) {
		p405_set_tsr (c, val);
		return (0);
	}
	else if (strcmp (reg, "xer") == 0) {
		p405_set_xer (c, val);
		return (0);
	}
	else if (strcmp (reg, "zpr") == 0) {
		p405_set_zpr (c, val);
		return (0);
	}

	if (strncmp (reg, "sprg", 4) == 0) {
		if (p405_get_reg_idx (reg + 4, &idx, 7)) {
			return (1);
		}

		p405_set_sprg (c, idx, val);

		return (0);
	}

	if (strncmp (reg, "srr", 3) == 0) {
		if (p405_get_reg_idx (reg + 3, &idx, 3)) {
			return (1);
		}

		p405_set_srr (c, idx, val);

		return (0);
	}

	if ((reg[0] == 'r') || (reg[0] == 'R')) {
		if (p405_get_reg_idx (reg + 1, &idx, 31)) {
			return (1);
		}

		p405_set_gpr (c, idx, val);

		return (0);
	}

	return (1);
}


uint8_t p405_get_mem8 (p405_t *c, uint32_t addr)
{
	if (c->get_uint8 != NULL) {
		return (c->get_uint8 (c->mem_ext, addr));
	}

	return (0xff);
}

uint16_t p405_get_mem16 (p405_t *c, uint32_t addr)
{
	if (c->get_uint16 != NULL) {
		return (c->get_uint16 (c->mem_ext, addr));
	}

	return (0xffff);
}

uint32_t p405_get_mem32 (p405_t *c, uint32_t addr)
{
	if (c->get_uint32 != NULL) {
		return (c->get_uint32 (c->mem_ext, addr));
	}

	return (0xffffffff);
}

void p405_set_mem8 (p405_t *c, uint32_t addr, uint8_t val)
{
	if (c->set_uint8 != NULL) {
		c->set_uint8 (c->mem_ext, addr, val);
	}
}

void p405_set_mem16 (p405_t *c, uint32_t addr, uint16_t val)
{
	if (c->set_uint16 != NULL) {
		c->set_uint16 (c->mem_ext, addr, val);
	}
}

void p405_set_mem32 (p405_t *c, uint32_t addr, uint32_t val)
{
	if (c->set_uint32 != NULL) {
		c->set_uint32 (c->mem_ext, addr, val);
	}
}

unsigned long p405_get_dcr (p405_t *c, unsigned long dcrn)
{
	if (c->get_dcr != NULL) {
		return (c->get_dcr (c->dcr_ext, dcrn));
	}

	return (0);
}

void p405_set_dcr (p405_t *c, unsigned long dcrn, unsigned long val)
{
	if (c->set_dcr != NULL) {
		c->set_dcr (c->dcr_ext, dcrn, val);
	}
}

void p405_undefined (p405_t *c)
{
	if (c->log_undef != NULL) {
		c->log_undef (c->log_ext, c->ir);
	}
}

static
void p405_exception (p405_t *c, uint32_t ofs)
{
	p405_tbuf_clear (c);

	if (c->log_exception != NULL) {
		c->log_exception (c->log_ext, ofs);
	}

	c->delay += 1;

	p405_set_pc (c, (p405_get_evpr (c) & 0xffff0000) | ofs);
}

void p405_exception_data_store (p405_t *c, uint32_t ea, int store, int zone)
{
	p405_set_srr (c, 0, p405_get_pc (c));
	p405_set_srr (c, 1, p405_get_msr (c));

	p405_set_dear (c, ea);

	c->msr &= ~P405_EXCPT_MSR;
	c->esr &= P405_ESR_MCI;

	if (store) {
		c->esr |= P405_ESR_DST;
	}

	if (zone) {
		c->esr |= P405_ESR_DIZ;
	}

	p405_exception (c, 0x300);
}

void p405_exception_instr_store (p405_t *c, int zone)
{
	p405_set_srr (c, 0, p405_get_pc (c));
	p405_set_srr (c, 1, p405_get_msr (c));

	c->msr &= ~P405_EXCPT_MSR;
	c->esr &= P405_ESR_MCI;

	if (zone) {
		c->esr |= P405_ESR_DIZ;
	}

	p405_exception (c, 0x400);
}

void p405_exception_external (p405_t *c)
{
	p405_set_srr (c, 0, p405_get_pc (c));
	p405_set_srr (c, 1, p405_get_msr (c));

	c->msr &= ~P405_EXCPT_MSR;

	p405_exception (c, 0x500);
}

void p405_exception_program (p405_t *c, uint32_t esr)
{
	p405_set_srr (c, 0, p405_get_pc (c));
	p405_set_srr (c, 1, p405_get_msr (c));

	c->msr &= ~P405_EXCPT_MSR;
	p405_set_esr (c, esr);

	p405_exception (c, 0x700);
}

void p405_exception_program_fpu (p405_t *c)
{
	p405_exception_program (c, P405_ESR_PEU);
}

void p405_exception_syscall (p405_t *c)
{
	p405_set_srr (c, 0, p405_get_pc (c) + 4);
	p405_set_srr (c, 1, p405_get_msr (c));

	c->msr &= ~P405_EXCPT_MSR;

	p405_exception (c, 0xc00);
}

void p405_exception_pit (p405_t *c)
{
	p405_set_srr (c, 0, p405_get_pc (c));
	p405_set_srr (c, 1, p405_get_msr (c));

	c->msr &= ~P405_EXCPT_MSR;

	p405_exception (c, 0x1000);
}

void p405_exception_tlb_miss_data (p405_t *c, uint32_t ea, int store)
{
	p405_set_srr (c, 0, p405_get_pc (c));
	p405_set_srr (c, 1, p405_get_msr (c));

	p405_set_dear (c, ea);

	c->msr &= ~P405_EXCPT_MSR;
	c->esr = (c->esr & P405_ESR_MCI) | ((store) ? P405_ESR_DST : 0);

	p405_exception (c, 0x1100);
}

void p405_exception_tlb_miss_instr (p405_t *c)
{
	p405_set_srr (c, 0, p405_get_pc (c));
	p405_set_srr (c, 1, p405_get_msr (c));

	c->msr &= ~P405_EXCPT_MSR;

	p405_exception (c, 0x1200);
}

void p405_interrupt (p405_t *c, unsigned char val)
{
	c->interrupt = (val != 0);
}

void p405_reset (p405_t *c)
{
	unsigned i;

	for (i = 0; i < 32; i++) {
		c->gpr[i] = 0;
	}

	for (i = 0; i < 8; i++) {
		c->sprg[i] = 0;
	}

	c->pc = 0xfffffffcUL;

	c->cr = 0;
	c->ctr = 0;
	c->dbcr0 = 0;
	c->dbcr1 = 0;
	c->dbsr = 0;
	c->dcwr = 0;
	c->dear = 0;
	c->esr = 0;
	c->evpr = 0;
	c->lr = 0;
	c->msr = 0;
	c->pid = 0;
	c->pit[0] = 0;
	c->pit[1] = 0;
	c->srr[0] = 0;
	c->srr[1] = 0;
	c->srr[2] = 0;
	c->srr[3] = 0;
	c->tbl = 0;
	c->tbu = 0;
	c->tcr = 0;
	c->tsr = 0;
	c->xer = 0;
	c->zpr = 0;

	c->ir = 0;

	c->reserve = 0;

	c->interrupt = 0;

	c->delay = 1;

	c->oprcnt = 0;
	c->clkcnt = 0;

	p405_tlb_init (&c->tlb);
}

void p405_execute (p405_t *c)
{
	unsigned op;

	if (p405_ifetch (c, c->pc, &c->ir)) {
		return;
	}

#ifdef P405_LOG_OPCODE
	if (c->log_opcode != NULL) {
		c->log_opcode (c->log_ext, c->ir);
	}
#endif

	op = (c->ir >> 26) & 0x3f;

	c->opcodes.op[op] (c);

	c->oprcnt += 1;

	if (c->interrupt) {
		if (p405_get_msr (c) & P405_MSR_EE) {
			p405_exception_external (c);
		}
	}
}

void p405_clock_tb (p405_t *c, unsigned long n)
{
	c->tbl = (c->tbl + n) & 0xffffffff;

	if (c->tbl < n) {
		c->tbu += 1;
	}

	if (c->pit[0] > 0) {
		if (n >= c->pit[0]) {
			if (c->tcr & P405_TCR_ARE) {
				c->pit[0] = c->pit[1] - (n % c->pit[1]);
			}
			else {
				c->pit[0] = 0;
			}

			/* interrupt */
			if (p405_get_msr (c) & P405_MSR_EE) {
				p405_exception_pit (c);
			}
		}
		else {
			c->pit[0] -= n;
		}
	}
}

void p405_clock (p405_t *c, unsigned long n)
{
	while (n >= c->delay) {
#if P405_DEBUG
		if (c->delay == 0) {
			fprintf (stderr, "warning: delay == 0 at %08lx\n",
				(unsigned long) p405_get_pc (c)
			);
			fflush (stderr);
			c->delay = 1;
		}
#endif
		n -= c->delay;

		c->clkcnt += c->delay;

		p405_clock_tb (c, c->timer_scale * c->delay);

		c->delay = 0;

		p405_execute (c);
	}

	if (n > 0) {
		c->clkcnt += n;
		p405_clock_tb (c, n);

		c->delay -= n;
	}
}
