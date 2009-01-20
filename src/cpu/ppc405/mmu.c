/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/ppc405/mmu.c                                         *
 * Created:     2003-11-17 by Hampa Hug <hampa@hampa.ch>                     *
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

#include "ppc405.h"
#include "internal.h"


void p405_tlb_init (p405_tlb_t *tlb)
{
	unsigned i;

	for (i = 0; i < P405_TLB_ENTRIES; i++) {
		tlb->entry[i].tlbhi = 0;
		tlb->entry[i].tlblo = 0;
		tlb->entry[i].tid = 0;
		tlb->entry[i].mask = 0xffffffffUL;
		tlb->entry[i].vaddr = 0;
		tlb->entry[i].endian = 0;

		tlb->entry[i].idx = i;

		if ((i + 1) < P405_TLB_ENTRIES) {
			tlb->entry[i].next = &tlb->entry[i + 1];
		}
		else {
			tlb->entry[i].next = NULL;
		}
	}

	tlb->first = &tlb->entry[0];

	tlb->tbuf_exec = NULL;
	tlb->tbuf_read = NULL;
	tlb->tbuf_write = NULL;
}

void p405_tbuf_clear (p405_t *c)
{
	c->tlb.tbuf_exec = NULL;
	c->tlb.tbuf_read = NULL;
	c->tlb.tbuf_write = NULL;
}

static inline
int p405_tlb_match (p405_tlbe_t *ent, uint32_t ea, uint32_t pid)
{
	if ((ent->tlbhi & P405_TLBHI_V) == 0) {
		return (0);
	}

	if ((ea & ent->mask) != ent->vaddr) {
		return (0);
	}

	if ((ent->tid != 0) && (ent->tid != pid)) {
		return (0);
	}

	return (1);
}

static inline
int p405_tlb_match_valid (p405_tlbe_t *ent, uint32_t ea, uint32_t pid)
{
	if ((ea & ent->mask) != ent->vaddr) {
		return (0);
	}

	if ((ent->tid != 0) && (ent->tid != pid)) {
		return (0);
	}

	return (1);
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
	ent->endian = (tlbhi & P405_TLBHI_E) != 0;

	p405_tbuf_clear (c);
}

void p405_set_tlb_entry_lo (p405_t *c, unsigned idx, uint32_t tlblo)
{
	c->tlb.entry[idx % P405_TLB_ENTRIES].tlblo = tlblo;

	p405_tbuf_clear (c);
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

	p405_tbuf_clear (c);
}

int p405_translate (p405_t *c, uint32_t *ea, int *e, unsigned xlat)
{
	p405_tlbe_t *ent;

	if ((xlat == P405_XLAT_VIRTUAL) || ((xlat == P405_XLAT_CPU) && p405_get_msr_dr (c))) {
		ent = p405_get_tlb_entry_ea (c, *ea);
		if (ent == NULL) {
			return (1);
		}

		*ea = (*ea & ~ent->mask) | (ent->tlblo & ent->mask);
		*e = ent->endian;
	}
	else {
		*e = 0;
	}

	return (0);
}

int p405_translate_read (p405_t *c, uint32_t *ea, int *e)
{
	p405_tlbe_t *ent;

	if (p405_get_msr_dr (c) == 0) {
		*e = 0;
		return (0);
	}

	if (c->tlb.tbuf_read != NULL) {
		ent = c->tlb.tbuf_read;
		if (p405_tlb_match_valid (ent, *ea, c->pid)) {
			*ea = (*ea & ~ent->mask) | (ent->tlblo & ent->mask);
			*e = ent->endian;
			return (0);
		}
	}

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
	*e = ent->endian;

	c->tlb.tbuf_read = ent;

	return (0);
}

int p405_translate_write (p405_t *c, uint32_t *ea, int *e)
{
	p405_tlbe_t *ent;

	if (p405_get_msr_dr (c) == 0) {
		*e = 0;
		return (0);
	}

	if (c->tlb.tbuf_write != NULL) {
		ent = c->tlb.tbuf_write;
		if (p405_tlb_match_valid (ent, *ea, c->pid)) {
			*ea = (*ea & ~ent->mask) | (ent->tlblo & ent->mask);
			*e = ent->endian;
			return (0);
		}
	}

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
	*e = ent->endian;

	c->tlb.tbuf_write = ent;

	return (0);
}

int p405_translate_exec (p405_t *c, uint32_t *ea, int *e)
{
	p405_tlbe_t *ent;

	if (p405_get_msr_ir (c) == 0) {
		*e = 0;
		return (0);
	}

	if (c->tlb.tbuf_exec != NULL) {
		ent = c->tlb.tbuf_exec;
		if (p405_tlb_match_valid (ent, *ea, c->pid)) {
			*ea = (*ea & ~ent->mask) | (ent->tlblo & ent->mask);
			*e = ent->endian;
			return (0);
		}
	}

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
	*e = ent->endian;

	c->tlb.tbuf_exec = ent;

	return (0);
}

int p405_ifetch (p405_t *c, uint32_t addr, uint32_t *val)
{
	int e;
#ifdef P405_LOG_MEM
	uint32_t vaddr = addr;
#endif

	if (p405_translate_exec (c, &addr, &e)) {
		return (1);
	}

	addr &= ~0x03UL;

	if (addr < c->ram_cnt) {
		unsigned char *mem = &c->ram[addr];

		if (e) {
			*val = (mem[3] << 24) | (mem[2] << 16) | (mem[1] << 8) | mem[0];
		}
		else {
			*val = (mem[0] << 24) | (mem[1] << 16) | (mem[2] << 8) | mem[3];
		}
	}
	else if (c->get_uint32 != NULL) {
		*val = c->get_uint32 (c->mem_ext, addr);
		if (e) {
			*val = p405_br32 (*val);
		}
	}
	else {
		*val = 0xffffffffUL;
	}

#ifdef P405_LOG_MEM
	if (c->log_mem != NULL) {
		c->log_mem (c->log_ext, P405_MMU_MODE_X | 0x04, addr, vaddr, *val);
	}
#endif

	return (0);
}

int p405_dload8 (p405_t *c, uint32_t addr, uint8_t *val)
{
	int e;
#ifdef P405_LOG_MEM
	uint32_t vaddr = addr;
#endif

	if (p405_translate_read (c, &addr, &e)) {
		return (1);
	}

	if (addr < c->ram_cnt) {
		*val = c->ram[addr];
	}
	else if (c->get_uint8 != NULL) {
		*val = c->get_uint8 (c->mem_ext, addr);
	}
	else {
		*val = 0xff;
	}

#ifdef P405_LOG_MEM
	if (c->log_mem != NULL) {
		c->log_mem (c->log_ext, P405_MMU_MODE_R | 0x01, addr, vaddr, *val);
	}
#endif

	return (0);
}

int p405_dload16 (p405_t *c, uint32_t addr, uint16_t *val)
{
	int e;
#ifdef P405_LOG_MEM
	uint32_t vaddr = addr;
#endif

	if (addr & 1) {
		if ((addr >> 10) != ((addr + 1) >> 10)) {
			uint8_t tmp[2];

			if (p405_dload8 (c, addr + 0, tmp + 0)) {
				return (1);
			}
			if (p405_dload8 (c, addr + 1, tmp + 1)) {
				return (1);
			}

			*val = (tmp[0] << 8) | tmp[1];

			return (0);
		}
	}

	if (p405_translate_read (c, &addr, &e)) {
		return (1);
	}

	if (addr < c->ram_cnt) {
		unsigned char *mem = &c->ram[addr];

		if (e) {
			*val = (mem[1] << 8) | mem[0];
		}
		else {
			*val = (mem[0] << 8) | mem[1];
		}
	}
	else if (c->get_uint16 != NULL) {
		*val = c->get_uint16 (c->mem_ext, addr);
		if (e) {
			*val = p405_br16 (*val);
		}
	}
	else {
		*val = 0xffffU;
	}

#ifdef P405_LOG_MEM
	if (c->log_mem != NULL) {
		c->log_mem (c->log_ext, P405_MMU_MODE_R | 0x02, addr, vaddr, *val);
	}
#endif

	return (0);
}

int p405_dload32 (p405_t *c, uint32_t addr, uint32_t *val)
{
	int e;
#ifdef P405_LOG_MEM
	uint32_t vaddr = addr;
#endif

	if (addr & 3) {
		if ((addr >> 10) != ((addr + 3) >> 10)) {
			uint8_t tmp[4];

			if (p405_dload8 (c, addr + 0, tmp + 0)) {
				return (1);
			}
			if (p405_dload8 (c, addr + 1, tmp + 1)) {
				return (1);
			}
			if (p405_dload8 (c, addr + 2, tmp + 2)) {
				return (1);
			}
			if (p405_dload8 (c, addr + 3, tmp + 3)) {
				return (1);
			}

			*val = (tmp[0] << 24) | (tmp[1] << 16) | (tmp[2] << 8) | tmp[3];

			return (0);
		}
	}

	if (p405_translate_read (c, &addr, &e)) {
		return (1);
	}

	if (addr < c->ram_cnt) {
		unsigned char *mem = &c->ram[addr];

		if (e) {
			*val = (mem[3] << 24) | (mem[2] << 16) | (mem[1] << 8) | mem[0];
		}
		else {
			*val = (mem[0] << 24) | (mem[1] << 16) | (mem[2] << 8) | mem[3];
		}
	}
	else if (c->get_uint32 != NULL) {
		*val = c->get_uint32 (c->mem_ext, addr);
		if (e) {
			*val = p405_br32 (*val);
		}
	}
	else {
		*val = 0xffffffffUL;
	}

#ifdef P405_LOG_MEM
	if (c->log_mem != NULL) {
		c->log_mem (c->log_ext, P405_MMU_MODE_R | 0x04, addr, vaddr, *val);
	}
#endif

	return (0);
}

int p405_dstore8 (p405_t *c, uint32_t addr, uint8_t val)
{
	int e;
#ifdef P405_LOG_MEM
	uint32_t vaddr = addr;
#endif

	if (p405_translate_write (c, &addr, &e)) {
		return (1);
	}

	if (addr < c->ram_cnt) {
		c->ram[addr] = val;
	}
	else if (c->set_uint8 != NULL) {
		c->set_uint8 (c->mem_ext, addr, val);
	}

#ifdef P405_LOG_MEM
	if (c->log_mem != NULL) {
		c->log_mem (c->log_ext, P405_MMU_MODE_W | 0x01, addr, vaddr, val);
	}
#endif

	return (0);
}

int p405_dstore16 (p405_t *c, uint32_t addr, uint16_t val)
{
	int e;
#ifdef P405_LOG_MEM
	uint32_t vaddr = addr;
#endif

	if (addr & 1) {
		if ((addr >> 10) != ((addr + 1) >> 10)) {
			if (p405_dstore8 (c, addr + 0, (val >> 8) & 0xff)) {
				return (1);
			}
			if (p405_dstore8 (c, addr + 1, val & 0xff)) {
				return (1);
			}
			return (0);
		}
	}

	if (p405_translate_write (c, &addr, &e)) {
		return (1);
	}

	if (addr < c->ram_cnt) {
		unsigned char *mem = &c->ram[addr];

		if (e) {
			mem[0] = val & 0xff;
			mem[1] = (val >> 8) & 0xff;
		}
		else {
			mem[0] = (val >> 8) & 0xff;
			mem[1] = val & 0xff;
		}
	}
	else if (c->set_uint16 != NULL) {
		if (e) {
			c->set_uint16 (c->mem_ext, addr, p405_br16 (val));
		}
		else {
			c->set_uint16 (c->mem_ext, addr, val);
		}
	}

#ifdef P405_LOG_MEM
	if (c->log_mem != NULL) {
		c->log_mem (c->log_ext, P405_MMU_MODE_W | 0x02, addr, vaddr, val);
	}
#endif

	return (0);
}

int p405_dstore32 (p405_t *c, uint32_t addr, uint32_t val)
{
	int e;
#ifdef P405_LOG_MEM
	uint32_t vaddr = addr;
#endif

	if (addr & 3) {
		if ((addr >> 10) != ((addr + 3) >> 10)) {
			if (p405_dstore8 (c, addr + 0, (val >> 24) & 0xff)) {
				return (1);
			}
			if (p405_dstore8 (c, addr + 1, (val >> 16) & 0xff)) {
				return (1);
			}
			if (p405_dstore8 (c, addr + 2, (val >> 8) & 0xff)) {
				return (1);
			}
			if (p405_dstore8 (c, addr + 3, val & 0xff)) {
				return (1);
			}
			return (0);
		}
	}

	if (p405_translate_write (c, &addr, &e)) {
		return (1);
	}

	if (addr < c->ram_cnt) {
		unsigned char *mem = &c->ram[addr];

		if (e) {
			mem[0] = val & 0xff;
			mem[1] = (val >> 8) & 0xff;
			mem[2] = (val >> 16) & 0xff;
			mem[3] = (val >> 24) & 0xff;
		}
		else {
			mem[0] = (val >> 24) & 0xff;
			mem[1] = (val >> 16) & 0xff;
			mem[2] = (val >> 8) & 0xff;
			mem[3] = val & 0xff;
		}
	}
	else if (c->set_uint32 != NULL) {
		if (e) {
			c->set_uint32 (c->mem_ext, addr, p405_br32 (val));
		}
		else {
			c->set_uint32 (c->mem_ext, addr, val);
		}
	}

#ifdef P405_LOG_MEM
	if (c->log_mem != NULL) {
		c->log_mem (c->log_ext, P405_MMU_MODE_W | 0x04, addr, vaddr, val);
	}
#endif

	return (0);
}

int p405_get_xlat8 (p405_t *c, uint32_t addr, unsigned xlat, uint8_t *val)
{
	int e;

	if (p405_translate (c, &addr, &e, xlat)) {
		return (1);
	}

	*val = p405_get_mem8 (c, addr);

	return (0);
}

int p405_get_xlat16 (p405_t *c, uint32_t addr, unsigned xlat, uint16_t *val)
{
	int e;

	if (p405_translate (c, &addr, &e, xlat)) {
		return (1);
	}

	*val = p405_get_mem16 (c, addr);

	if (e) {
		*val = p405_br16 (*val);
	}

	return (0);
}

int p405_get_xlat32 (p405_t *c, uint32_t addr, unsigned xlat, uint32_t *val)
{
	int e;

	if (p405_translate (c, &addr, &e, xlat)) {
		return (1);
	}

	*val = p405_get_mem32 (c, addr);

	if (e) {
		*val = p405_br32 (*val);
	}

	return (0);
}

int p405_set_xlat8 (p405_t *c, uint32_t addr, unsigned xlat, uint8_t val)
{
	int e;

	if (p405_translate (c, &addr, &e, xlat)) {
		return (1);
	}

	p405_set_mem8 (c, addr, val);

	return (0);
}

int p405_set_xlat16 (p405_t *c, uint32_t addr, unsigned xlat, uint16_t val)
{
	int e;

	if (p405_translate (c, &addr, &e, xlat)) {
		return (1);
	}

	if (e) {
		val = p405_br16 (val);
	}

	p405_set_mem16 (c, addr, val);

	return (0);
}

int p405_set_xlat32 (p405_t *c, uint32_t addr, unsigned xlat, uint32_t val)
{
	int e;

	if (p405_translate (c, &addr, &e, xlat)) {
		return (1);
	}

	if (e) {
		val = p405_br32 (val);
	}

	p405_set_mem32 (c, addr, val);

	return (0);
}
