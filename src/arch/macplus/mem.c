/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/mem.c                                       *
 * Created:     2007-11-13 by Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"
#include "macplus.h"
#include "mem.h"


void mac_set_overlay (macplus_t *sim, int overlay)
{
	if (sim->overlay == (overlay != 0)) {
		return;
	}

#ifdef DEBUG_MEM
	mac_log_deb ("mem: overlay = %d\n", overlay);
#endif

	if (overlay) {
		mem_rmv_blk (sim->mem, sim->ram);
		mem_add_blk (sim->mem, sim->rom_ovl, 0);
		mem_add_blk (sim->mem, sim->ram_ovl, 0);

		e68_set_ram (sim->cpu, NULL, 0);

		sim->overlay = 1;
	}
	else {
		mem_rmv_blk (sim->mem, sim->rom_ovl);
		mem_rmv_blk (sim->mem, sim->ram_ovl);
		mem_add_blk (sim->mem, sim->ram, 0);

		e68_set_ram (sim->cpu,
			mem_blk_get_data (sim->ram),
			mem_blk_get_size (sim->ram)
		);

		sim->overlay = 0;
	}
}


static
int mac_addr_map (macplus_t *sim, unsigned long *addr)
{
	unsigned long val;

	/* repeated RAM images */
	if (*addr < 0x400000) {
		if (sim->ram == NULL) {
			return (0);
		}

		val = mem_blk_get_size (sim->ram);

		if (val == 0) {
			return (0);
		}

		val = *addr % val;

		if (*addr == val) {
			return (0);
		}

		*addr = val;

		return (1);
	}

	/* repeated ROM images */
	if ((*addr >= 0x400000) && (*addr < 0x580000)) {
		if (sim->rom == NULL) {
			return (0);
		}

		val = mem_blk_get_size (sim->rom);

		if (val == 0) {
			return (0);
		}

		val = 0x400000 + (*addr % val);

		if (*addr == val) {
			return (0);
		}

		*addr = val;

		return (1);
	}

	return (0);
}

unsigned char mac_mem_get_uint8 (void *ext, unsigned long addr)
{
	macplus_t *sim = ext;

	if (mac_addr_map (sim, &addr)) {
		return (mem_get_uint8 (sim->mem, addr));
	}

	if ((addr >= 0x580000) && (addr < 0x600000)) {
		return (0x00);
	}

	if ((addr >= 0xc00000) && (addr < 0xe00000)) {
		return (mac_iwm_get_uint8 (&sim->iwm, addr - 0xc00000));
	}

	if ((addr & 0xf00000) == 0xd00000) {
		return (0xaa);
	}

#ifdef DEBUG_MEM
	mac_log_deb ("mem: get  8: %06lX -> 00\n", addr);
#endif

	return (0);
}

unsigned short mac_mem_get_uint16 (void *ext, unsigned long addr)
{
	macplus_t *sim = ext;

	if (mac_addr_map (sim, &addr)) {
		return (mem_get_uint16_be (sim->mem, addr));
	}

#ifdef DEBUG_MEM
	mac_log_deb ("mem: get 16: %06lX -> 0000\n", addr);
#endif

	return (0);
}

unsigned long mac_mem_get_uint32 (void *ext, unsigned long addr)
{
	macplus_t *sim = ext;

	if (mac_addr_map (sim, &addr)) {
		return (mem_get_uint32_be (sim->mem, addr));
	}

#ifdef DEBUG_MEM
	mac_log_deb ("mem: get 32: %06lX -> 00000000\n", addr);
#endif

	return (0);
}

void mac_mem_set_uint8 (void *ext, unsigned long addr, unsigned char val)
{
	macplus_t *sim = ext;

	if (mac_addr_map (sim, &addr)) {
		mem_set_uint8 (sim->mem, addr, val);
	}

	if ((addr >= 0x580000) && (addr < 0x600000)) {
		return;
	}

	if ((addr >= 0xc00000) && (addr < 0xe00000)) {
		mac_iwm_set_uint8 (&sim->iwm, addr - 0xc00000, val);
		return;
	}

#ifdef DEBUG_MEM
	mac_log_deb ("mem: set  8: %06lX <- %02X\n", addr, val);
#endif
}

void mac_mem_set_uint16 (void *ext, unsigned long addr, unsigned short val)
{
	macplus_t *sim = ext;

	if (mac_addr_map (sim, &addr)) {
		mem_set_uint16_be (sim->mem, addr, val);
	}

#ifdef DEBUG_MEM
	mac_log_deb ("mem: set 16: %06lX <- %02X\n", addr, val);
#endif
}

void mac_mem_set_uint32 (void *ext, unsigned long addr, unsigned long val)
{
	macplus_t *sim = ext;

	if (mac_addr_map (sim, &addr)) {
		mem_set_uint32_be (sim->mem, addr, val);
	}

#ifdef DEBUG_MEM
	mac_log_deb ("mem: set 32: %06lX <- %02lX\n", addr, val);
#endif
}
