/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/cpu/ppc405/mmu.c                                       *
 * Created:       2003-11-17 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-02-20 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003-2004 Hampa Hug <hampa@hampa.ch>                   *
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

#define p405_tlb_match(ent, ea, pid) ( \
    (((ent)->tlbhi & P405_TLBHI_V) != 0) && \
    (((ea) & (ent)->mask) == (ent)->vaddr) && \
    (((ent)->tid == 0) || ((ent)->tid == (pid))) )


void p405_tlb_init (p405_tlb_t *tlb)
{
  unsigned i;

  for (i = 0; i < P405_TLB_ENTRIES; i++) {
    tlb->entry[i].tlbhi = 0;
    tlb->entry[i].tlblo = 0;
    tlb->entry[i].tid = 0;
    tlb->entry[i].mask = 0xffffffffUL;
    tlb->entry[i].vaddr = 0;

    tlb->entry[i].idx = i;

    if ((i + 1) < P405_TLB_ENTRIES) {
      tlb->entry[i].next = &tlb->entry[i + 1];
    }
    else {
      tlb->entry[i].next = NULL;
    }
  }

  tlb->first = &tlb->entry[0];
}

inline
unsigned p405_get_tlb_index (p405_t *c, uint32_t ea)
{
  p405_tlbe_t *ent, *tmp;

  ent = c->tlb.first;

  if (p405_tlb_match (ent, ea, c->pid)) {
    return (ent->idx);
  }

  tmp = ent;
  ent = ent->next;

  while (ent != NULL) {
    if (p405_tlb_match (ent, ea, c->pid)) {
      tmp->next = ent->next;
      ent->next = c->tlb.first;
      c->tlb.first = ent;

      return (ent->idx);
    }

    tmp = ent;
    ent = ent->next;
  }

  return (P405_TLB_ENTRIES);
}

p405_tlbe_t *p405_get_tlb_entry_ea (p405_t *c, uint32_t ea)
{
  unsigned idx;

  idx = p405_get_tlb_index (c, ea);

  return ((idx < P405_TLB_ENTRIES) ? &c->tlb.entry[idx] : NULL);
}

p405_tlbe_t *p405_get_tlb_entry_idx (p405_t *c, unsigned idx)
{
  if (idx < P405_TLB_ENTRIES) {
    return (&c->tlb.entry[idx]);
  }

  return (NULL);
}

unsigned p405_get_tlb_entry_cnt (p405_t *c)
{
  return (P405_TLB_ENTRIES);
}

void p405_set_tlb_entry_hi (p405_t *c, unsigned idx, uint32_t tlbhi, uint8_t pid)
{
  p405_tlbe_t *ent;

  ent = &c->tlb.entry[idx % P405_TLB_ENTRIES];

  ent->tlbhi = tlbhi;
  ent->tid = pid;
  ent->mask = 0xfffffc00UL << (2 * p405_get_tlbe_size (ent));
  ent->vaddr = tlbhi & ent->mask;
}

void p405_set_tlb_entry_lo (p405_t *c, unsigned idx, uint32_t tlblo)
{
  c->tlb.entry[idx % P405_TLB_ENTRIES].tlblo = tlblo;
}

uint32_t p405_get_tlb_entry_hi (p405_t *c, unsigned idx)
{
  return (c->tlb.entry[idx % P405_TLB_ENTRIES].tlbhi);
}

uint32_t p405_get_tlb_entry_lo (p405_t *c, unsigned idx)
{
  return (c->tlb.entry[idx % P405_TLB_ENTRIES].tlblo);
}

uint32_t p405_get_tlb_entry_tid (p405_t *c, unsigned idx)
{
  return (c->tlb.entry[idx % P405_TLB_ENTRIES].tid);
}


void p405_tlb_invalidate_all (p405_t *c)
{
  unsigned i;

  for (i = 0; i < P405_TLB_ENTRIES; i++) {
    c->tlb.entry[i].tlbhi &= ~P405_TLBHI_V;
  }
}

int p405_translate_read (p405_t *c, uint32_t *ea)
{
  p405_tlbe_t *ent;

  if (p405_get_msr_dr (c)) {
    ent = p405_get_tlb_entry_ea (c, *ea);
    if (ent == NULL) {
      p405_exception_tlb_miss_data (c, *ea, 0);
      return (1);
    }

    if (p405_get_msr_pr (c)) {
      switch (p405_get_zprf (c, p405_get_tlbe_zsel (ent))) {
        case 0x00:
          /* no access */
          p405_exception_data_store (c, *ea, 0, 1);
          return (1);
      }
    }

    *ea = (*ea & ~ent->mask) | (ent->tlblo & ent->mask);
  }

  return (0);
}

int p405_translate (p405_t *c, uint32_t *ea, unsigned xlat)
{
  p405_tlbe_t *ent;

  if ((xlat == P405_XLAT_VIRTUAL) || ((xlat == P405_XLAT_CPU) && p405_get_msr_dr (c))) {
    ent = p405_get_tlb_entry_ea (c, *ea);
    if (ent == NULL) {
      return (1);
    }

    *ea = (*ea & ~ent->mask) | (ent->tlblo & ent->mask);
  }

  return (0);
}

int p405_translate_write (p405_t *c, uint32_t *ea)
{
  p405_tlbe_t *ent;

  if (p405_get_msr_dr (c)) {
    ent = p405_get_tlb_entry_ea (c, *ea);
    if (ent == NULL) {
      p405_exception_tlb_miss_data (c, *ea, 1);
      return (1);
    }

    if (p405_get_msr_pr (c)) {
      switch (p405_get_zprf (c, p405_get_tlbe_zsel (ent))) {
        case 0x00:
          /* no access */
          p405_exception_data_store (c, *ea, 1, 1);
          return (1);

        case 0x01:
        case 0x02:
          /* use tlb bits */
          if (p405_get_tlbe_wr (ent) == 0) {
            p405_exception_data_store (c, *ea, 1, 0);
            return (1);
          }
          break;

        case 0x03:
          /* full access */
          break;
      }
    }
    else {
      switch (p405_get_zprf (c, p405_get_tlbe_zsel (ent))) {
        case 0x00:
        case 0x01:
          /* use tlb bits */
          if (p405_get_tlbe_wr (ent) == 0) {
            p405_exception_data_store (c, *ea, 1, 0);
            return (1);
          }
          break;

        case 0x02:
        case 0x03:
          /* full access */
          break;
      }
    }

    *ea = (*ea & ~ent->mask) | (ent->tlblo & ent->mask);
  }

  return (0);
}

int p405_translate_exec (p405_t *c, uint32_t *ea)
{
  p405_tlbe_t *ent;

  if (p405_get_msr_ir (c)) {
    ent = p405_get_tlb_entry_ea (c, *ea);
    if (ent == NULL) {
      p405_exception_tlb_miss_instr (c);
      return (1);
    }

    if (p405_get_msr_pr (c)) {
      switch (p405_get_zprf (c, p405_get_tlbe_zsel (ent))) {
        case 0x00:
          /* no access */
          p405_exception_instr_store (c, 1);
          return (1);

        case 0x01:
        case 0x02:
          /* use tlb bits */
          if (p405_get_tlbe_ex (ent) == 0) {
            p405_exception_instr_store (c, 0);
            return (1);
          }
          break;

        case 0x03:
          /* full access */
          break;
      }
    }
    else {
      switch (p405_get_zprf (c, p405_get_tlbe_zsel (ent))) {
        case 0x00:
        case 0x01:
          /* use tlb bits */
          if (p405_get_tlbe_ex (ent) == 0) {
            p405_exception_instr_store (c, 0);
            return (1);
          }
          break;

        case 0x02:
        case 0x03:
          /* full access */
          break;
      }
    }

    *ea = (*ea & ~ent->mask) | (ent->tlblo & ent->mask);
  }

  return (0);
}

int p405_ifetch (p405_t *c, uint32_t addr, uint32_t *val)
{
  uint32_t vaddr = addr;

  if (p405_translate_exec (c, &addr)) {
    return (1);
  }

  *val = p405_get_mem32 (c, addr & ~0x03);

  if (c->log_mem != NULL) {
    c->log_mem (c->log_ext, P405_MMU_MODE_X | 0x04, addr, vaddr, *val);
  }

  return (0);
}

int p405_dload8 (p405_t *c, uint32_t addr, uint8_t *val)
{
  uint32_t vaddr = addr;

  if (p405_translate_read (c, &addr)) {
    return (1);
  }

  *val = p405_get_mem8 (c, addr);

  if (c->log_mem != NULL) {
    c->log_mem (c->log_ext, P405_MMU_MODE_R | 0x01, addr, vaddr, *val);
  }

  return (0);
}

int p405_dload16 (p405_t *c, uint32_t addr, uint16_t *val)
{
  uint32_t vaddr = addr;

  if (p405_translate_read (c, &addr)) {
    return (1);
  }

  *val = p405_get_mem16 (c, addr);

  if (c->log_mem != NULL) {
    c->log_mem (c->log_ext, P405_MMU_MODE_R | 0x02, addr, vaddr, *val);
  }

  return (0);
}

int p405_dload32 (p405_t *c, uint32_t addr, uint32_t *val)
{
  uint32_t vaddr = addr;

  if (p405_translate_read (c, &addr)) {
    return (1);
  }

  *val = p405_get_mem32 (c, addr);

  if (c->log_mem != NULL) {
    c->log_mem (c->log_ext, P405_MMU_MODE_R | 0x04, addr, vaddr, *val);
  }

  return (0);
}

int p405_dstore8 (p405_t *c, uint32_t addr, uint8_t val)
{
  uint32_t vaddr = addr;

  if (p405_translate_write (c, &addr)) {
    return (1);
  }

  p405_set_mem8 (c, addr, val);

  if (c->log_mem != NULL) {
    c->log_mem (c->log_ext, P405_MMU_MODE_W | 0x01, addr, vaddr, val);
  }

  return (0);
}

int p405_dstore16 (p405_t *c, uint32_t addr, uint16_t val)
{
  uint32_t vaddr = addr;

  if (p405_translate_write (c, &addr)) {
    return (1);
  }

  p405_set_mem16 (c, addr, val);

  if (c->log_mem != NULL) {
    c->log_mem (c->log_ext, P405_MMU_MODE_W | 0x02, addr, vaddr, val);
  }

  return (0);
}

int p405_dstore32 (p405_t *c, uint32_t addr, uint32_t val)
{
  uint32_t vaddr = addr;

  if (p405_translate_write (c, &addr)) {
    return (1);
  }

  p405_set_mem32 (c, addr, val);

  if (c->log_mem != NULL) {
    c->log_mem (c->log_ext, P405_MMU_MODE_W | 0x04, addr, vaddr, val);
  }

  return (0);
}

int p405_get_xlat8 (p405_t *c, uint32_t addr, unsigned xlat, uint8_t *val)
{
  if (p405_translate (c, &addr, xlat)) {
    return (1);
  }

  *val = p405_get_mem8 (c, addr);

  return (0);
}

int p405_get_xlat16 (p405_t *c, uint32_t addr, unsigned xlat, uint16_t *val)
{
  if (p405_translate (c, &addr, xlat)) {
    return (1);
  }

  *val = p405_get_mem16 (c, addr);

  return (0);
}

int p405_get_xlat32 (p405_t *c, uint32_t addr, unsigned xlat, uint32_t *val)
{
  if (p405_translate (c, &addr, xlat)) {
    return (1);
  }

  *val = p405_get_mem32 (c, addr);

  return (0);
}

int p405_set_xlat8 (p405_t *c, uint32_t addr, unsigned xlat, uint8_t val)
{
  if (p405_translate (c, &addr, xlat)) {
    return (1);
  }

  p405_set_mem8 (c, addr, val);

  return (0);
}

int p405_set_xlat16 (p405_t *c, uint32_t addr, unsigned xlat, uint16_t val)
{
  if (p405_translate (c, &addr, xlat)) {
    return (1);
  }

  p405_set_mem16 (c, addr, val);

  return (0);
}

int p405_set_xlat32 (p405_t *c, uint32_t addr, unsigned xlat, uint32_t val)
{
  if (p405_translate (c, &addr, xlat)) {
    return (1);
  }

  p405_set_mem32 (c, addr, val);

  return (0);
}
