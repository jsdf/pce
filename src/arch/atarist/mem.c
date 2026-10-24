/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/mem.c                                       *
 * Created:     2011-03-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2018 Hampa Hug <hampa@hampa.ch>                     *
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
#include "atarist.h"
#include "dma.h"
#include "mem.h"
#include "psg.h"

#include <chipset/e6850.h>
#include <drivers/char/char.h>


unsigned char st_mem_get_uint8 (void *ext, unsigned long addr)
{
	atari_st_t *sim = ext;

	addr &= 0x00ffffff;

	if (addr < 0xf00000) {
		return (0);
	}

	if ((addr >= 0xff8900) && (addr < 0xff8930)) {
		/* DMA sound */

		if ((sim->model & PCE_ST_STE) == 0) {
			e68_set_bus_error (sim->cpu, 1);
		}

		return (0);
	}

	if ((addr >= 0xff8a00) && (addr < 0xff8a40)) {
		/* blitter */
		e68_set_bus_error (sim->cpu, 1);
		return (0);
	}

	if ((addr >= 0xfffc20) && (addr < 0xfffc40)) {
		if (sim->model & (PCE_ST_MEGA | PCE_ST_RTC)) {
			return (rp5c15_get_uint8 (&sim->rtc, (addr - 0xfffc20) / 2));
		}
		else {
			return (0);
		}
	}

	switch (addr) {
	case 0xff8001: /* memory configuration */
		return (sim->memcfg);

	case 0xff8609:
		return (st_dma_get_addr (&sim->dma, 0));

	case 0xff860b:
		return (st_dma_get_addr (&sim->dma, 1));

	case 0xff860d:
		return (st_dma_get_addr (&sim->dma, 2));

	case 0xff860f: /* FDC density */
		return (0);

	case 0xff8800:
		/* YM2149 */
		return (st_psg_get_data (&sim->psg));

	case 0xff8802:
		/* YM2149 */
		return (st_psg_get_select (&sim->psg));

	case 0xfffc00:
		return (e6850_get_uint8 (&sim->acia0, 0));

	case 0xfffc02:
		return (e6850_get_uint8 (&sim->acia0, 1));

	case 0xfffc04:
		return (e6850_get_uint8 (&sim->acia1, 0));

	case 0xfffc06:
		return (e6850_get_uint8 (&sim->acia1, 1));

	default:
		e68_set_bus_error (sim->cpu, 1);
		st_log_deb ("mem: get  8: %06lX -> 00\n", addr);
		break;
	}

	return (0);
}

unsigned short st_mem_get_uint16 (void *ext, unsigned long addr)
{
	atari_st_t *sim = ext;

	addr &= 0x00ffffff;

	if (addr < 0xf00000) {
		return (0);
	}

	if ((addr >= 0xff8900) && (addr < 0xff8930)) {
		/* dma sound */

		if ((sim->model & PCE_ST_STE) == 0) {
			e68_set_bus_error (sim->cpu, 1);
		}

		return (0);
	}

	if ((addr >= 0xff8a00) && (addr < 0xff8a40)) {
		/* blitter */
		e68_set_bus_error (sim->cpu, 1);
		return (0);
	}

	switch (addr) {
	case 0xfa0000: /* cartridge */
	case 0xfa0002:
		break;

	case 0xff8604:
		return (st_dma_get_disk (&sim->dma));

	case 0xff8606:
		return (st_dma_get_status (&sim->dma));

	case 0xff8608:
		return (st_dma_get_addr (&sim->dma, 0));

	case 0xff860a:
		return (st_dma_get_addr (&sim->dma, 1));

	case 0xff860c:
		return (st_dma_get_addr (&sim->dma, 2));

	case 0xff8800:
	case 0xff8802:
	case 0xfffc00:
	case 0xfffc02:
	case 0xfffc04:
	case 0xfffc06:
		return (st_mem_get_uint8 (ext, addr) << 8);

	default:
		e68_set_bus_error (sim->cpu, 1);
		st_log_deb ("mem: get 16: %06lX -> 0000\n", addr);
		break;
	}

	return (0);
}

unsigned long st_mem_get_uint32 (void *ext, unsigned long addr)
{
	unsigned long val;
	atari_st_t    *sim = ext;

	val = mem_get_uint16_be (sim->mem, addr);
	val = (val << 16) | mem_get_uint16_be (sim->mem, addr + 2);

	return (val);
}

void st_mem_set_uint8 (void *ext, unsigned long addr, unsigned char val)
{
	atari_st_t *sim = ext;

	addr &= 0x00ffffff;

	if (addr < 0xf00000) {
		return;
	}

	if ((addr >= 0xff8808) && (addr < 0xff8900)) {
		/* psg */
		st_log_deb ("mem: set  8: %06lX <- %02X\n", addr, val);
		return;
	}

	if ((addr >= 0xff8900) && (addr < 0xff8930)) {
		/* dma sound */

		if ((sim->model & PCE_ST_STE) == 0) {
			e68_set_bus_error (sim->cpu, 1);
		}

		return;
	}

	if ((addr >= 0xff8a00) && (addr < 0xff8a40)) {
		/* blitter */
		e68_set_bus_error (sim->cpu, 1);
		return;
	}

	if ((addr >= 0xfffc20) && (addr < 0xfffc40)) {
		if (sim->model & (PCE_ST_MEGA | PCE_ST_RTC)) {
			rp5c15_set_uint8 (&sim->rtc, (addr - 0xfffc20) / 2, val);
		}

		return;
	}

	switch (addr) {
	case 0xff8001: /* memory configuration */
		sim->memcfg = val;
		break;

	case 0xff8609:
		st_dma_set_addr (&sim->dma, 0, val);
		break;

	case 0xff860b:
		st_dma_set_addr (&sim->dma, 1, val);
		break;

	case 0xff860d:
		st_dma_set_addr (&sim->dma, 2, val);
		break;

	case 0xff860f: /* FDC density */
		break;

	case 0xff8800:
	case 0xff8804:
		st_psg_set_select (&sim->psg, val);
		break;

	case 0xff8802:
	case 0xff8806:
		st_psg_set_data (&sim->psg, val);
		break;

	case 0xfffc00:
		e6850_set_uint8 (&sim->acia0, 0, val);
		break;

	case 0xfffc02:
		e6850_set_uint8 (&sim->acia0, 1, val);
		break;

	case 0xfffc04:
		e6850_set_uint8 (&sim->acia1, 0, val);
		break;

	case 0xfffc06:
		e6850_set_uint8 (&sim->acia1, 1, val);
		break;

	default:
		e68_set_bus_error (sim->cpu, 1);
		st_log_deb ("mem: set  8: %06lX <- %02X\n", addr, val);
		break;
	}
}

void st_mem_set_uint16 (void *ext, unsigned long addr, unsigned short val)
{
	atari_st_t *sim = ext;

	addr &= 0x00ffffff;

	if (addr < 0xf00000) {
		return;
	}

	if ((addr >= 0xff8804) && (addr < 0xff8900)) {
		/* psg */
		st_log_deb ("mem: set 16: %06lX <- %04X\n", addr, val);
		return;
	}

	if ((addr >= 0xff8900) && (addr < 0xff8930)) {
		/* dma sound */

		if ((sim->model & PCE_ST_STE) == 0) {
			e68_set_bus_error (sim->cpu, 1);
		}

		return;
	}

	if ((addr >= 0xff8a00) && (addr < 0xff8a40)) {
		/* blitter */
		e68_set_bus_error (sim->cpu, 1);
		return;
	}

	switch (addr) {
	case 0xff8604:
		st_dma_set_disk (&sim->dma, val);
		break;

	case 0xff8606:
		st_dma_set_mode (&sim->dma, val);
		break;

	case 0xff8800:
		st_psg_set_select (&sim->psg, val >> 8);
		break;

	case 0xff8802:
		st_psg_set_data (&sim->psg, val >> 8);
		break;

	case 0xfffc00:
	case 0xfffc02:
	case 0xfffc04:
	case 0xfffc06:
		st_mem_set_uint8 (ext, addr, val >> 8);
		break;

	default:
		e68_set_bus_error (sim->cpu, 1);
		st_log_deb ("mem: set 16: %06lX <- %04X\n", addr, val);
		break;
	}
}

void st_mem_set_uint32 (void *ext, unsigned long addr, unsigned long val)
{
	atari_st_t *sim = ext;

	mem_set_uint16_be (sim->mem, addr, val >> 16);
	mem_set_uint16_be (sim->mem, addr + 2, val & 0xffff);
}
