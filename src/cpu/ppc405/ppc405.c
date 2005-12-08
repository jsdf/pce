/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/cpu/ppc405/ppc405.c                                    *
 * Created:       2003-11-07 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2005-12-08 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003-2005 Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id$ */


#include <stdio.h>
#include <stdlib.h>

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

  p405_set_opcodes (c);

  c->pvr = P405_PVR_405GP;
}

p405_t *p405_new (void)
{
  p405_t *c;

  c = (p405_t *) malloc (sizeof (p405_t));
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

  c->get_uint8 = (p405_get_uint8_f) get8;
  c->get_uint16 = (p405_get_uint16_f) get16;
  c->get_uint32 = (p405_get_uint32_f) get32;

  c->set_uint8 = (p405_set_uint8_f) set8;
  c->set_uint16 = (p405_set_uint16_f) set16;
  c->set_uint32 = (p405_set_uint32_f) set32;
}

void p405_set_ram (p405_t *c, unsigned char *ram, unsigned long cnt)
{
  c->ram = ram;
  c->ram_cnt = cnt;
}

void p405_set_dcr_fct (p405_t *c, void *ext, void *get, void *set)
{
  c->dcr_ext = ext;
  c->get_dcr = (p405_get_uint32_f) get;
  c->set_dcr = (p405_set_uint32_f) set;
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

  return (0xffffffffUL);
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
  if (c->log_exception != NULL) {
    c->log_exception (c->log_ext, ofs);
  }

  c->delay += 1;

  p405_set_pc (c, (p405_get_evpr (c) & 0xffff0000UL) | ofs);
}

void p405_exception_data_store (p405_t *c, uint32_t ea, int store, int zone)
{
  p405_set_srr (c, 0, p405_get_pc (c));
  p405_set_srr (c, 1, p405_get_msr (c));

  c->msr &= ~P405_EXCPT_MSR;
  p405_set_dear (c, ea);
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
  c->srr[0] = c->pc;
  c->srr[1] = c->msr;
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

  c->msr &= ~P405_EXCPT_MSR;
  c->dear = ea;
  c->esr = (c->esr & P405_ESR_MCI) | ((store) ? P405_ESR_DST : 0);

  p405_exception (c, 0x1100);
}

void p405_exception_tlb_miss_instr (p405_t *c)
{
  c->srr[0] = c->pc;
  c->srr[1] = c->msr;

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

  p405_tlb_init (&c->tlb);

  c->delay = 1;

  c->oprcnt = 0;
  c->clkcnt = 0;
}

void p405_execute (p405_t *c)
{
  unsigned op;

  if (p405_ifetch (c, c->pc, &c->ir)) {
    return;
  }

  if (c->log_opcode != NULL) {
    c->log_opcode (c->log_ext, c->ir);
  }

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
  c->tbl = (c->tbl + n) & 0xffffffffUL;

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
    if (c->delay == 0) {
      fprintf (stderr, "warning: delay == 0 at %08lx\n", (unsigned long) p405_get_pc (c));
      fflush (stderr);
      c->delay = 1;
    }

    n -= c->delay;

    c->clkcnt += c->delay;

    /* artificially speed up timer */
    p405_clock_tb (c, 16 * c->delay);

    c->delay = 0;

    p405_execute (c);
  }

  c->clkcnt += n;
  p405_clock_tb (c, n);

  c->delay -= n;
}
