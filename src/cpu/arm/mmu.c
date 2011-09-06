/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/arm/mmu.c                                            *
 * Created:     2004-11-03 by Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdio.h>
#include <stdlib.h>

#include "arm.h"
#include "internal.h"


#if defined(PCE_HOST_PPC) || defined (PCE_HOST_SPARC)
#define ARM_HOST_BE
#endif

#if defined(PCE_HOST_IA32)
#define ARM_HOST_LE
#endif


static
void arm_mmu_fault (arm_t *c, uint32_t addr, unsigned status, unsigned domn)
{
	arm_copr15_t *mmu;

	mmu = arm_get_mmu (c);

	mmu->reg[6] = addr;
	mmu->reg[5] = ((domn & 0x0f) << 4) | (status & 0x0f);

	arm_exception_data_abort (c);
}

static
void arm_mmu_translation_fault (arm_t *c, uint32_t addr, unsigned domn, int sect)
{
	arm_mmu_fault (c, addr, sect ? 0x05 : 0x07, domn);
}

static
void arm_mmu_domain_fault (arm_t *c, uint32_t addr, unsigned domn, int sect)
{
	arm_mmu_fault (c, addr, sect ? 0x09 : 0x0b, domn);
}

static
void arm_mmu_permission_fault (arm_t *c, uint32_t addr, unsigned domn, int sect)
{
	arm_mmu_fault (c, addr, sect ? 0x0d : 0x0f, domn);
}


static inline
void arm_tbuf_set (arm_tbuf_t *tb, uint32_t vaddr, uint32_t raddr, uint32_t mask)
{
	tb->vaddr = vaddr & mask;
	tb->vmask = mask;
	tb->raddr = raddr & mask;
	tb->rmask = ~mask;
	tb->valid = 1;
}


/*!***************************************************************************
 * @short Check access permissions for reading
 * @param cr    The control register (coprocessor 15 register 1)
 * @param perm  The page or section permission bits
 * @param priv  Check for privileged access if true
 * @return Zero if access violation, non-zero if ok
 *****************************************************************************/
static
int arm_mmu_check_perm_read (uint32_t cr, unsigned perm, int priv)
{
	if (perm == 0) {
		switch (cr & (ARM_C15_CR_S | ARM_C15_CR_R)) {
		case 0x00: /* no access */
			return (0);

		case ARM_C15_CR_S: /* priv: read only, user: no access */
			return (priv);

		case ARM_C15_CR_R: /* read only */
			return (1);

		case ARM_C15_CR_S | ARM_C15_CR_R: /* undefined */
			return (0);
		}

		return (0);
	}

	if (priv) {
		return (1);
	}

	switch (perm) {
	case 0x01: /* no access */
		return (0);

	case 0x02: /* read only */
		return (1);

	case 0x03: /* read/write */
		return (1);
	}

	return (0);
}

/*!***************************************************************************
 * @short Check access permissions for writing
 * @param cr    The control register (coprocessor 15 register 1)
 * @param perm  The page or section permission bits
 * @param priv  Check for privileged access if true
 * @return Zero if access violation, non-zero if ok
 *****************************************************************************/
static
int arm_mmu_check_perm_write (uint32_t cr, unsigned perm, int priv)
{
	if (perm == 0) {
		switch (cr & (ARM_C15_CR_S | ARM_C15_CR_R)) {
		case 0x00: /* no access */
			return (0);

		case ARM_C15_CR_S: /* priv: read only, user: no access */
			return (0);

		case ARM_C15_CR_R: /* read only */
			return (0);

		case ARM_C15_CR_S | ARM_C15_CR_R: /* undefined */
			return (0);
		}

		return (0);
	}

	if (priv) {
		return (1);
	}

	switch (perm) {
	case 0x01: /* no access */
		return (0);

	case 0x02: /* read only */
		return (0);

	case 0x03: /* read/write */
		return (1);
	}

	return (0);
}

/*!***************************************************************************
 * @short Translate a virtual address
 * @param  c     The ARM context
 * @param  addr  Virtual address input, real address output
 * @retval domn  Returns the domain number
 * @retval perm  The page or section permission bits
 * @retval sect  The virtual address is in a section if true, in a page if false
 * @return Non-zero on translation fault
 *****************************************************************************/
static
int arm_translate (arm_t *c, uint32_t *addr, uint32_t *mask,
	unsigned *domn, unsigned *perm, int *sect)
{
	arm_copr15_t *mmu;
	unsigned     ap;
	uint32_t     addr1, addr2;
	uint32_t     desc1, desc2;

	mmu = arm_get_mmu (c);

	addr1 = (mmu->reg[2] & 0xffffc000) | ((*addr >> 18) & 0x00003ffc);
	desc1 = c->get_uint32 (c->mem_ext, addr1);

	addr2 = 0;

	switch (desc1 & 0x03) {
	case 0x00:
		/* section translation fault */
		*mask = 0;
		*domn = 0;
		*sect = 1;
		return (1);

	case 0x01:
		/* coarse */
		addr2 = (desc1 & 0xfffffc00) | ((*addr >> 10) & 0x000003fc);
		*domn = arm_get_bits (desc1, 5, 4);
		break;

	case 0x02:
		/* section */
		*addr = (desc1 & 0xfff00000) | (*addr & 0x000fffff);
		*mask = 0xfff00000;
		*domn = arm_get_bits (desc1, 5, 4);
		*perm = arm_get_bits (desc1, 10, 2);
		*sect = 1;
		return (0);

	case 0x03:
		/* fine */
		addr2 = (desc1 & 0xfffff000) | ((*addr >> 8) & 0x00000ffc);
		*domn = arm_get_bits (desc1, 5, 4);
		break;
	}

	desc2 = c->get_uint32 (c->mem_ext, addr2);

	*sect = 0;

	switch (desc2 & 0x03) {
	case 0x00:
		/* page translation fault */
		*mask = 0;
		*perm = 0;
		return (1);

	case 0x01:
		/* large page */
		ap = 4 + 2 * arm_get_bits (*addr, 14, 2);
		*addr = (desc2 & 0xffff0000) | (*addr & 0x0000ffff);
		*mask = 0xffff0000;
		*perm = arm_get_bits (desc2, ap, 2);
		return (0);

	case 0x02:
		/* small page */
		ap = 4 + 2 * arm_get_bits (*addr, 10, 2);
		*addr = (desc2 & 0xfffff000) | (*addr & 0x00000fff);
		*mask = 0xfffff000;
		*perm = arm_get_bits (desc2, ap, 2);
		return (0);

	case 0x03:
		if ((desc1 & 0x03) == 0x01) {
			/* xscale extended small page */
			*addr = (desc2 & 0xfffff000) | (*addr & 0x00000fff);
			*mask = 0xfffff000;
			*perm = arm_get_bits (desc2, 4, 2);
			return (0);
		}

		/* tiny page */
		*perm = arm_get_bits (desc2, 4, 2);
		*addr = (desc2 & 0xfffffc00) | (*addr & 0x000003ff);
		*mask = 0xfffffc00;
		return (0);
	}

	/* non-reachable */

	*perm = 0;

	return (1);
}

static
int arm_translate_exec (arm_t *c, uint32_t *addr, int priv)
{
	arm_copr15_t *mmu;
	unsigned     domn, perm;
	int          sect;
	uint32_t     vaddr, mask;

	mmu = arm_get_mmu (c);

	if ((mmu->reg[1] & ARM_C15_CR_M) == 0) {
		return (0);
	}

	vaddr = *addr;

	if (mmu->tbuf_exec.valid) {
		arm_tbuf_t *tb = &mmu->tbuf_exec;

		if ((vaddr & tb->vmask) == tb->vaddr) {
			*addr = tb->raddr | (vaddr & tb->rmask);
			return (0);
		}
	}

	if (arm_translate (c, addr, &mask, &domn, &perm, &sect)) {
		arm_exception_prefetch_abort (c);
		return (1);
	}

	/* check domain */
	switch ((mmu->reg[3] >> (2 * domn)) & 0x03) {
	case 0x00: /* no access */
		arm_mmu_domain_fault (c, vaddr, domn, sect);
		return (1);

	case 0x01: /* client */
		if (arm_mmu_check_perm_read (mmu->reg[1], perm, priv) == 0) {
			arm_exception_prefetch_abort (c);
			return (1);
		}
		arm_tbuf_set (&mmu->tbuf_exec, vaddr, *addr, mask);
		return (0);

	case 0x02: /* undefined */
		return (0);

	case 0x03: /* manager */
		arm_tbuf_set (&mmu->tbuf_exec, vaddr, *addr, mask);
		return (0);
	}

	return (0);
}

static
int arm_translate_read (arm_t *c, uint32_t *addr, int priv)
{
	arm_copr15_t *mmu;
	unsigned     domn, perm;
	int          sect;
	uint32_t     vaddr, mask;

	mmu = arm_get_mmu (c);

	if ((mmu->reg[1] & ARM_C15_CR_M) == 0) {
		return (0);
	}

	vaddr = *addr;

	if (mmu->tbuf_read.valid) {
		arm_tbuf_t *tb = &mmu->tbuf_read;

		if ((vaddr & tb->vmask) == tb->vaddr) {
			*addr = tb->raddr | (vaddr & tb->rmask);
			return (0);
		}
	}

	if (arm_translate (c, addr, &mask, &domn, &perm, &sect)) {
		arm_mmu_translation_fault (c, vaddr, domn, sect);
		return (1);
	}

	/* check domain */
	switch ((mmu->reg[3] >> (2 * domn)) & 0x03) {
	case 0x00: /* no access */
		arm_mmu_domain_fault (c, vaddr, domn, sect);
		return (1);

	case 0x01: /* client */
		if (arm_mmu_check_perm_read (mmu->reg[1], perm, priv) == 0) {
			arm_mmu_permission_fault (c, vaddr, domn, sect);
			return (1);
		}
		arm_tbuf_set (&mmu->tbuf_read, vaddr, *addr, mask);
		return (0);

	case 0x02: /* undefined */
		return (0);

	case 0x03: /* manager */
		arm_tbuf_set (&mmu->tbuf_read, vaddr, *addr, mask);
		return (0);
	}

	return (0);
}

static
int arm_translate_write (arm_t *c, uint32_t *addr, int priv)
{
	arm_copr15_t *mmu;
	unsigned     domn, perm;
	int          sect;
	uint32_t     vaddr, mask;

	mmu = arm_get_mmu (c);

	if ((mmu->reg[1] & ARM_C15_CR_M) == 0) {
		return (0);
	}

	vaddr = *addr;

	if (mmu->tbuf_write.valid) {
		arm_tbuf_t *tb = &mmu->tbuf_write;

		if ((vaddr & tb->vmask) == tb->vaddr) {
			*addr = tb->raddr | (vaddr & tb->rmask);
			return (0);
		}
	}

	if (arm_translate (c, addr, &mask, &domn, &perm, &sect)) {
		arm_mmu_translation_fault (c, vaddr, domn, sect);
		return (1);
	}

	/* check domain */
	switch ((mmu->reg[3] >> (2 * domn)) & 0x03) {
	case 0x00: /* no access */
		arm_mmu_domain_fault (c, vaddr, domn, sect);
		return (1);

	case 0x01: /* client */
		if (arm_mmu_check_perm_write (mmu->reg[1], perm, priv) == 0) {
			arm_mmu_permission_fault (c, vaddr, domn, sect);
			return (1);
		}

		arm_tbuf_set (&mmu->tbuf_write, vaddr, *addr, mask);

		return (0);

	case 0x02: /* undefined */
		return (0);

	case 0x03: /* manager */
		arm_tbuf_set (&mmu->tbuf_write, vaddr, *addr, mask);
		return (0);
	}

	return (0);
}

/* translate without causing exceptions */
int arm_translate_extern (arm_t *c, uint32_t *addr, unsigned xlat,
	unsigned *domn, unsigned *perm)
{
	arm_copr15_t *mmu;
	unsigned     domn1, perm1;
	int          sect;
	uint32_t     mask;

	if (domn == NULL) {
		domn = &domn1;
	}

	if (perm == NULL) {
		perm = &perm1;
	}

	*domn = 0;
	*perm = 0;

	if (xlat == ARM_XLAT_REAL) {
		return (0);
	}

	mmu = arm_get_mmu (c);

	if ((xlat == ARM_XLAT_CPU) && ((mmu->reg[1] & ARM_C15_CR_M) == 0)) {
		return (0);
	}

	if (arm_translate (c, addr, &mask, domn, perm, &sect)) {
		return (1);
	}

	return (0);
}


int arm_ifetch (arm_t *c, uint32_t addr, uint32_t *val)
{
	uint32_t tmp;

	addr &= ~0x03UL;

	if (arm_translate_exec (c, &addr, arm_is_privileged (c))) {
		return (1);
	}

	if (addr < c->ram_cnt) {
		unsigned char *p = &c->ram[addr];

		if (c->bigendian) {
#ifdef ARM_HOST_BE
			tmp = *(uint32_t *)p;
#else
			tmp = (uint32_t) p[0] << 24;
			tmp |= (uint32_t) p[1] << 16;
			tmp |= (uint32_t) p[2] << 8;
			tmp |= p[3];
#endif
		}
		else {
#ifdef ARM_HOST_LE
			tmp = *(uint32_t *)p;
#else
			tmp = p[0];
			tmp |= (uint32_t) p[1] << 8;
			tmp |= (uint32_t) p[2] << 16;
			tmp |= (uint32_t) p[3] << 24;
#endif
		}
	}
	else {
		tmp = c->get_uint32 (c->mem_ext, addr);
	}

	*val = tmp;

	return (0);
}

int arm_dload8 (arm_t *c, uint32_t addr, uint8_t *val)
{
	if (arm_translate_read (c, &addr, arm_is_privileged (c))) {
		return (1);
	}

	if (addr < c->ram_cnt) {
		*val = c->ram[addr];
	}
	else {
		*val = c->get_uint8 (c->mem_ext, addr);
	}

	return (0);
}

int arm_dload16 (arm_t *c, uint32_t addr, uint16_t *val)
{
	if (arm_translate_read (c, &addr, arm_is_privileged (c))) {
		return (1);
	}

	if ((addr + 1) < c->ram_cnt) {
		unsigned char *p = &c->ram[addr];

		if (c->bigendian) {
#ifdef ARM_HOST_BE
			*val = *(uint16_t *) p;
#else
			*val = ((uint16_t) p[0] << 8) | p[1];
#endif
		}
		else {
#ifdef ARM_HOST_LE
			*val = *(uint16_t *) p;
#else
			*val = ((uint16_t) p[1] << 8) | p[0];
#endif
		}
	}
	else {
		*val = c->get_uint16 (c->mem_ext, addr);
	}

	return (0);
}

int arm_dload32 (arm_t *c, uint32_t addr, uint32_t *val)
{
	if (arm_translate_read (c, &addr, arm_is_privileged (c))) {
		return (1);
	}

	if ((addr + 3) < c->ram_cnt) {
		unsigned char *p = &c->ram[addr];

		if (c->bigendian) {
#ifdef ARM_HOST_BE
			*val = *(uint32_t *) p;
#else
			*val = (uint32_t) p[0] << 24;
			*val |= (uint32_t) p[1] << 16;
			*val |= (uint32_t) p[2] << 8;
			*val |= p[3];
#endif
		}
		else {
#ifdef ARM_HOST_LE
			*val = *(uint32_t *) p;
#else
			*val = p[0];
			*val |= (uint32_t) p[1] << 8;
			*val |= (uint32_t) p[2] << 16;
			*val |= (uint32_t) p[3] << 24;
#endif
		}
	}
	else {
		*val = c->get_uint32 (c->mem_ext, addr);
	}

	return (0);
}

int arm_dstore8 (arm_t *c, uint32_t addr, uint8_t val)
{
	if (arm_translate_write (c, &addr, arm_is_privileged (c))) {
		return (1);
	}

	if (addr < c->ram_cnt) {
		c->ram[addr] = val;
	}
	else {
		c->set_uint8 (c->mem_ext, addr, val);
	}

	return (0);
}

int arm_dstore16 (arm_t *c, uint32_t addr, uint16_t val)
{
	if (arm_translate_write (c, &addr, arm_is_privileged (c))) {
		return (1);
	}

	if ((addr + 1) < c->ram_cnt) {
		unsigned char *p = &c->ram[addr];

		if (c->bigendian) {
#ifdef ARM_HOST_BE
			*(uint16_t *) p = val;
#else
			p[0] = (val >> 8) & 0xff;
			p[1] = val & 0xff;
#endif
		}
		else {
#ifdef ARM_HOST_LE
			*(uint16_t *) p = val;
#else
			p[0] = val & 0xff;
			p[1] = (val >> 8) & 0xff;
#endif
		}
	}
	else {
		c->set_uint16 (c->mem_ext, addr, val);
	}

	return (0);
}

int arm_dstore32 (arm_t *c, uint32_t addr, uint32_t val)
{
	if (arm_translate_write (c, &addr, arm_is_privileged (c))) {
		return (1);
	}

	if ((addr + 3) < c->ram_cnt) {
		unsigned char *p = &c->ram[addr];

		if (c->bigendian) {
#ifdef ARM_HOST_BE
			*(uint32_t *) p = val;
#else
			p[0] = (val >> 24) & 0xff;
			p[1] = (val >> 16) & 0xff;
			p[2] = (val >> 8) & 0xff;
			p[3] = val & 0xff;
#endif
		}
		else {
#ifdef ARM_HOST_LE
			*(uint32_t *) p = val;
#else
			p[0] = val & 0xff;
			p[1] = (val >> 8) & 0xff;
			p[2] = (val >> 16) & 0xff;
			p[3] = (val >> 24) & 0xff;
#endif
		}
	}
	else {
		c->set_uint32 (c->mem_ext, addr, val);
	}

	return (0);
}

int arm_dload8_t (arm_t *c, uint32_t addr, uint8_t *val)
{
	if (arm_translate_read (c, &addr, 0)) {
		return (1);
	}

	*val = c->get_uint8 (c->mem_ext, addr);

	return (0);
}

int arm_dload16_t (arm_t *c, uint32_t addr, uint16_t *val)
{
	if (arm_translate_read (c, &addr, 0)) {
		return (1);
	}

	*val = c->get_uint16 (c->mem_ext, addr);

	return (0);
}

int arm_dload32_t (arm_t *c, uint32_t addr, uint32_t *val)
{
	if (arm_translate_read (c, &addr, 0)) {
		return (1);
	}

	*val = c->get_uint32 (c->mem_ext, addr);

	return (0);
}

int arm_dstore8_t (arm_t *c, uint32_t addr, uint8_t val)
{
	if (arm_translate_write (c, &addr, 0)) {
		return (1);
	}

	c->set_uint8 (c->mem_ext, addr, val);

	return (0);
}

int arm_dstore16_t (arm_t *c, uint32_t addr, uint16_t val)
{
	if (arm_translate_write (c, &addr, 0)) {
		return (1);
	}

	c->set_uint16 (c->mem_ext, addr, val);

	return (0);
}

int arm_dstore32_t (arm_t *c, uint32_t addr, uint32_t val)
{
	if (arm_translate_write (c, &addr, 0)) {
		return (1);
	}

	c->set_uint32 (c->mem_ext, addr, val);

	return (0);
}


int arm_get_mem8 (arm_t *c, uint32_t addr, unsigned xlat, uint8_t *val)
{
	if (arm_translate_extern (c, &addr, xlat, NULL, NULL)) {
		return (1);
	}

	if (c->get_uint8 != NULL) {
		*val = c->get_uint8 (c->mem_ext, addr);
	}
	else {
		*val = 0xff;
	}

	return (0);
}

int arm_get_mem16 (arm_t *c, uint32_t addr, unsigned xlat, uint16_t *val)
{
	if (arm_translate_extern (c, &addr, xlat, NULL, NULL)) {
		return (1);
	}

	if (c->get_uint16 != NULL) {
		*val = c->get_uint16 (c->mem_ext, addr);
	}
	else {
		*val = 0xffff;
	}

	return (0);
}

int arm_get_mem32 (arm_t *c, uint32_t addr, unsigned xlat, uint32_t *val)
{
	if (arm_translate_extern (c, &addr, xlat, NULL, NULL)) {
		return (1);
	}

	if (c->get_uint32 != NULL) {
		*val = c->get_uint32 (c->mem_ext, addr);
	}
	else {
		*val = 0xffffffff;
	}

	return (0);
}

int arm_set_mem8 (arm_t *c, uint32_t addr, unsigned xlat, uint8_t val)
{
	if (arm_translate_extern (c, &addr, xlat, NULL, NULL)) {
		return (1);
	}

	if (c->set_uint8 != NULL) {
		c->set_uint8 (c->mem_ext, addr, val);
	}

	return (0);
}

int arm_set_mem16 (arm_t *c, uint32_t addr, unsigned xlat, uint16_t val)
{
	if (arm_translate_extern (c, &addr, xlat, NULL, NULL)) {
		return (1);
	}

	if (c->set_uint16 != NULL) {
		c->set_uint16 (c->mem_ext, addr, val);
	}

	return (0);
}

int arm_set_mem32 (arm_t *c, uint32_t addr, unsigned xlat, uint32_t val)
{
	if (arm_translate_extern (c, &addr, xlat, NULL, NULL)) {
		return (1);
	}

	if (c->set_uint32 != NULL) {
		c->set_uint32 (c->mem_ext, addr, val);
	}

	return (0);
}
