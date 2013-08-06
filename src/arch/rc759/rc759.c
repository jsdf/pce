/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/rc759.c                                       *
 * Created:     2012-06-29 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2013 Hampa Hug <hampa@hampa.ch>                     *
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
#include "fdc.h"
#include "msg.h"
#include "parport.h"
#include "rc759.h"
#include "rtc.h"
#include "video.h"

#include <stdlib.h>
#include <string.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <lib/brkpt.h>
#include <lib/inidsk.h>
#include <lib/iniram.h>
#include <lib/initerm.h>
#include <lib/load.h>
#include <lib/log.h>
#include <lib/string.h>
#include <lib/sysdep.h>

#include <chipset/80186/dma.h>
#include <chipset/80186/icu.h>
#include <chipset/80186/tcu.h>

#include <chipset/82xx/e8250.h>
#include <chipset/82xx/e8255.h>
#include <chipset/82xx/e8259.h>

#include <cpu/e8086/e8086.h>

#include <devices/memory.h>

#include <drivers/block/block.h>

#include <drivers/video/terminal.h>

#include <libini/libini.h>


#define PCE_IBMPC_SLEEP 25000


static char *par_intlog[256];


static
unsigned char rc759_get_port8 (rc759_t *sim, unsigned long addr)
{
	unsigned char val;

	if ((addr >= 0x80) && (addr <= 0xff)) {
		unsigned idx;

		idx = (sim->ppi_port_c << 2) & 0xc0;
		idx |= ((addr - 0x80) >> 1) & 0x3f;

		val = rc759_nvm_get_uint4 (&sim->nvm, idx);

		return (val & 0x0f);
	}

	if ((addr >= 0x180) && (addr <= 0x1be)) {
		if (addr & 1) {
			return (0);
		}

		val = e82730_get_palette (&sim->crt, (addr - 0x180) >> 1);

		return (val);
	}

	switch (addr) {
	case 0x0000:
		val = e8259_get_uint8 (&sim->pic, 0);
		break;

	case 0x0002:
		val = e8259_get_uint8 (&sim->pic, 1);
		break;

	case 0x0020:
		val = rc759_kbd_get_key (&sim->kbd);
		break;

	case 0x0056:
		val = 0xff;
		break;

	case 0x005c:
		val = rc759_rtc_get_addr (&sim->rtc);
		break;

	case 0x0070:
		val = e8255_get_uint8 (&sim->ppi, 0);
		break;

	case 0x0072:
		val = e8255_get_uint8 (&sim->ppi, 1);
		break;

	case 0x0074:
		val = e8255_get_uint8 (&sim->ppi, 2);
		break;

	case 0x0076:
		val = e8255_get_uint8 (&sim->ppi, 3);
		break;

	case 0x0250:
		val = rc759_par_get_data (&sim->par[0]);
		break;

	case 0x0260:
		val = rc759_par_get_status (&sim->par[0]);
		break;

	case 0x0280:
		val = wd179x_get_status (&sim->fdc.wd179x);
		break;

	case 0x0282:
		val = wd179x_get_track (&sim->fdc.wd179x);
		break;

	case 0x0284:
		val = wd179x_get_sector (&sim->fdc.wd179x);
		break;

	case 0x0286:
		val = wd179x_get_data (&sim->fdc.wd179x);
		break;

	case 0x028a:
		val = rc759_par_get_data (&sim->par[1]);
		break;

	case 0x028c:
		val = rc759_par_get_status (&sim->par[1]);
		break;

	case 0x028e:
		val = rc759_fdc_get_reserve (&sim->fdc);
		break;

	case 0x0292:
		val = rc759_par_get_reserve (&sim->par[1]);
		break;

	case 0x0300:
		val = 0;
		break;

	case 0x0302:
		val = 0;
		break;

	default:
		val = 0xff;
		sim_log_deb ("get port 8 %04lX <- %02X\n", addr, val);
		break;
	}

	return (val);
}

static
unsigned short rc759_get_port16 (rc759_t *sim, unsigned long addr)
{
	unsigned short val;

	switch (addr) {
	case 0xff24:
		val = e80186_icu_get_poll (&sim->icu);
		break;

	case 0xff26:
		val = e80186_icu_get_pollst (&sim->icu);
		break;

	case 0xff28:
		val = e80186_icu_get_imr (&sim->icu);
		break;

	case 0xff2a:
		val = e80186_icu_get_pmr (&sim->icu);
		break;

	case 0xff2c:
		val = e80186_icu_get_isr (&sim->icu);
		break;

	case 0xff2e:
		val = e80186_icu_get_irr (&sim->icu);
		break;

	case 0xff32:
		val = e80186_icu_get_icon (&sim->icu, 0);
		break;

	case 0xff34:
		val = e80186_icu_get_icon (&sim->icu, 2);
		break;

	case 0xff36:
		val = e80186_icu_get_icon (&sim->icu, 3);
		break;

	case 0xff38:
		val = e80186_icu_get_icon (&sim->icu, 4);
		break;

	case 0xff3a:
		val = e80186_icu_get_icon (&sim->icu, 5);
		break;

	case 0xff3c:
		val = e80186_icu_get_icon (&sim->icu, 6);
		break;

	case 0xff3e:
		val = e80186_icu_get_icon (&sim->icu, 7);
		break;

	case 0xff50:
		val = e80186_tcu_get_count (&sim->tcu, 0);
		break;

	case 0xff52:
		val = e80186_tcu_get_max_count_a (&sim->tcu, 0);
		break;

	case 0xff54:
		val = e80186_tcu_get_max_count_b (&sim->tcu, 0);
		break;

	case 0xff56:
		val = e80186_tcu_get_control (&sim->tcu, 0);
		break;

	case 0xff58:
		val = e80186_tcu_get_count (&sim->tcu, 1);
		break;

	case 0xff5a:
		val = e80186_tcu_get_max_count_a (&sim->tcu, 1);
		break;

	case 0xff5c:
		val = e80186_tcu_get_max_count_b (&sim->tcu, 1);
		break;

	case 0xff5e:
		val = e80186_tcu_get_control (&sim->tcu, 1);
		break;

	case 0xff60:
		val = e80186_tcu_get_count (&sim->tcu, 2);
		break;

	case 0xff62:
		val = e80186_tcu_get_max_count_a (&sim->tcu, 2);
		break;

	case 0xff66:
		val = e80186_tcu_get_control (&sim->tcu, 2);
		break;

	case 0xffc0:
		val = e80186_dma_get_src_lo (&sim->dma, 0);
		break;

	case 0xffc2:
		val = e80186_dma_get_src_hi (&sim->dma, 0);
		break;

	case 0xffc4:
		val = e80186_dma_get_dst_lo (&sim->dma, 0);
		break;

	case 0xffc6:
		val = e80186_dma_get_dst_hi (&sim->dma, 0);
		break;

	case 0xffc8:
		val = e80186_dma_get_count (&sim->dma, 0);
		break;

	case 0xffca:
		val = e80186_dma_get_control (&sim->dma, 0);
		break;

	case 0xffd0:
		val = e80186_dma_get_src_lo (&sim->dma, 1);
		break;

	case 0xffd2:
		val = e80186_dma_get_src_hi (&sim->dma, 1);
		break;

	case 0xffd4:
		val = e80186_dma_get_dst_lo (&sim->dma, 1);
		break;

	case 0xffd6:
		val = e80186_dma_get_dst_hi (&sim->dma, 1);
		break;

	case 0xffd8:
		val = e80186_dma_get_count (&sim->dma, 1);
		break;

	case 0xffda:
		val = e80186_dma_get_control (&sim->dma, 1);
		break;

	default:
		val = 0xffff;
		sim_log_deb ("get port 16 %04lX -> %02X\n", addr, val);
		break;
	}

	return (val);
}

static
void rc759_set_port8 (rc759_t *sim, unsigned long addr, unsigned char val)
{
	if ((addr >= 0x80) && (addr <= 0xff)) {
		unsigned idx;

		idx = (sim->ppi_port_c << 2) & 0xc0;
		idx |= ((addr - 0x80) >> 1) & 0x3f;

		rc759_nvm_set_uint4 (&sim->nvm, idx, val);

		return;
	}

	if ((addr >= 0x180) && (addr <= 0x1be)) {
		if (addr & 1) {
			return;
		}

		e82730_set_palette (&sim->crt, (addr - 0x180) >> 1, val);

		return;
	}

	switch (addr) {
	case 0x0000:
		e8259_set_uint8 (&sim->pic, 0, val);
		break;

	case 0x0002:
		e8259_set_uint8 (&sim->pic, 1, val);
		break;

	case 0x0056:
		break;

	case 0x005a:
		rc759_rtc_set_data (&sim->rtc, val);
		break;

	case 0x005c:
		rc759_rtc_set_addr (&sim->rtc, val);
		break;

	case 0x0070:
		e8255_set_uint8 (&sim->ppi, 0, val);
		break;

	case 0x0072:
		e8255_set_uint8 (&sim->ppi, 1, val);
		break;

	case 0x0074:
		e8255_set_uint8 (&sim->ppi, 2, val);
		break;

	case 0x0076:
		e8255_set_uint8 (&sim->ppi, 3, val);
		break;

	case 0xcece:
		rc759_set_msg (sim, "emu.stop", "1");
		break;

	case 0x0230:
		e82730_set_srst (&sim->crt);
		break;

	case 0x0240:
		e82730_set_ca (&sim->crt);
		break;

	case 0x0250:
		rc759_par_set_data (&sim->par[0], val);
		break;

	case 0x0260:
		rc759_par_set_control (&sim->par[0], val);
		break;

	case 0x0280:
		wd179x_set_cmd (&sim->fdc.wd179x, val);
		break;

	case 0x0282:
		wd179x_set_track (&sim->fdc.wd179x, val);
		break;

	case 0x0284:
		wd179x_set_sector (&sim->fdc.wd179x, val);
		break;

	case 0x0286:
		wd179x_set_data (&sim->fdc.wd179x, val);
		break;

	case 0x0288:
		rc759_fdc_set_fcr (&sim->fdc, val);
		break;

	case 0x028a:
		rc759_par_set_data (&sim->par[1], val);
		break;

	case 0x028c:
		rc759_par_set_control (&sim->par[1], val);
		break;

	case 0x028e:
		rc759_fdc_set_reserve (&sim->fdc, 1);
		break;

	case 0x0290:
		rc759_fdc_set_reserve (&sim->fdc, 0);
		break;

	case 0x0292:
		rc759_par_set_reserve (&sim->par[1], 1);
		break;

	case 0x0294:
		rc759_par_set_reserve (&sim->par[1], 0);
		break;

	case 0xff28:
		e80186_icu_set_imr (&sim->icu, val);
		break;

	default:
		sim_log_deb ("set port 8 %04lX <- %02X\n", addr, val);
		break;
	}
}

static
void rc759_set_port16 (rc759_t *sim, unsigned long addr, unsigned short val)
{
	switch (addr) {
	case 0x0002:
		e8259_set_uint8 (&sim->pic, 1, val & 0xff);
		break;

	case 0x0230:
		e82730_set_srst (&sim->crt);
		break;

	case 0x0240:
		e82730_set_ca (&sim->crt);
		break;

	case 0xcece:
		rc759_set_msg (sim, "emu.stop", "1");
		break;

	case 0xff22:
		e80186_icu_set_eoi (&sim->icu, val);
		break;

	case 0xff28:
		e80186_icu_set_imr (&sim->icu, val);
		break;

	case 0xff2a:
		e80186_icu_set_pmr (&sim->icu, val);
		break;

	case 0xff2c:
		e80186_icu_set_isr (&sim->icu, val);
		break;

	case 0xff2e:
		e80186_icu_set_irr (&sim->icu, val);
		break;

	case 0xff32:
		e80186_icu_set_icon (&sim->icu, 0, val);
		break;

	case 0xff34:
		e80186_icu_set_icon (&sim->icu, 2, val);
		break;

	case 0xff36:
		e80186_icu_set_icon (&sim->icu, 3, val);
		break;

	case 0xff38:
		e80186_icu_set_icon (&sim->icu, 4, val);
		break;

	case 0xff3a:
		e80186_icu_set_icon (&sim->icu, 5, val);
		break;

	case 0xff3c:
		e80186_icu_set_icon (&sim->icu, 6, val);
		break;

	case 0xff3e:
		e80186_icu_set_icon (&sim->icu, 7, val);
		break;

	case 0xff50:
		e80186_tcu_set_count (&sim->tcu, 0, val);
		break;

	case 0xff52:
		e80186_tcu_set_max_count_a (&sim->tcu, 0, val);
		break;

	case 0xff54:
		e80186_tcu_set_max_count_b (&sim->tcu, 0, val);
		break;

	case 0xff56:
		e80186_tcu_set_control (&sim->tcu, 0, val);
		break;

	case 0xff58:
		e80186_tcu_set_count (&sim->tcu, 1, val);
		break;

	case 0xff5a:
		e80186_tcu_set_max_count_a (&sim->tcu, 1, val);
		break;

	case 0xff5c:
		e80186_tcu_set_max_count_b (&sim->tcu, 1, val);
		break;

	case 0xff5e:
		e80186_tcu_set_control (&sim->tcu, 1, val);
		break;

	case 0xff60:
		e80186_tcu_set_count (&sim->tcu, 2, val);
		break;

	case 0xff62:
		e80186_tcu_set_max_count_a (&sim->tcu, 2, val);
		break;

	case 0xff66:
		e80186_tcu_set_control (&sim->tcu, 2, val);
		break;

	case 0xffa0: /* UMCS */
		break;

	case 0xffa2: /* LMCS */
		break;

	case 0xffa4: /* PACS */
		break;

	case 0xffa6: /* MMCS */
		break;

	case 0xffa8: /* MPCS */
		break;

	case 0xffc0:
		e80186_dma_set_src_lo (&sim->dma, 0, val);
		break;

	case 0xffc2:
		e80186_dma_set_src_hi (&sim->dma, 0, val);
		break;

	case 0xffc4:
		e80186_dma_set_dst_lo (&sim->dma, 0, val);
		break;

	case 0xffc6:
		e80186_dma_set_dst_hi (&sim->dma, 0, val);
		break;

	case 0xffc8:
		e80186_dma_set_count (&sim->dma, 0, val);
		break;

	case 0xffca:
		e80186_dma_set_control (&sim->dma, 0, val);
		break;

	case 0xffd0:
		e80186_dma_set_src_lo (&sim->dma, 1, val);
		break;

	case 0xffd2:
		e80186_dma_set_src_hi (&sim->dma, 1, val);
		break;

	case 0xffd4:
		e80186_dma_set_dst_lo (&sim->dma, 1, val);
		break;

	case 0xffd6:
		e80186_dma_set_dst_hi (&sim->dma, 1, val);
		break;

	case 0xffd8:
		e80186_dma_set_count (&sim->dma, 1, val);
		break;

	case 0xffda:
		e80186_dma_set_control (&sim->dma, 1, val);
		break;

	default:
		sim_log_deb ("set port 16 %04lX <- %04X\n", addr, val);
		break;
	}
}


static
void rc759_set_timer0_out (rc759_t *sim, unsigned char val)
{
	sim->ppi_port_a |= 1;

	if (sim->ppi_port_c & 1) {
		if (val) {
			sim->ppi_port_a &= 0xfe;
		}
	}

	e8255_set_inp_a (&sim->ppi, sim->ppi_port_a);
}

static
void rc759_set_timer1_out (rc759_t *sim, unsigned char val)
{
	rc759_spk_set_out (&sim->spk, val);
}

static
void rc759_set_ppi_port_c (rc759_t *sim, unsigned char val)
{
	sim->ppi_port_c = val;

	rc759_kbd_set_enable (&sim->kbd, val & 0x80);
	e82730_set_graphic (&sim->crt, (val & 0x40) == 0);
}

static
void rc759_set_mouse (void *ext, int dx, int dy, unsigned button)
{
	rc759_t *sim = ext;

	chr_mouse_set (dx, dy, button);

	rc759_kbd_set_mouse (&sim->kbd, dx, dy, button);
}


static
void rc759_setup_system (rc759_t *sim, ini_sct_t *ini)
{
	int           mem2;
	unsigned long clock;
	ini_sct_t     *sct;

	sim->flags = 0;

	sim->current_int = 0;

	sim->brk = 0;
	sim->pause = 0;

	sct = ini_next_sct (ini, NULL, "system");

	ini_get_bool (sct, "alt_mem_size", &mem2, 0);
	ini_get_uint32 (sct, "clock", &clock, 6000000);

	pce_log_tag (MSG_INF, "SYSTEM:", "model=rc759 clock=%lu alt_mem_size=%d\n",
		clock, mem2
	);

	sim->cpu_clock_frq = clock;
	sim->cpu_clock_cnt = 0;

	if (mem2) {
		sim->flags |= RC759_FLAG_MEM2;
	}
}

static
void rc759_setup_mem (rc759_t *sim, ini_sct_t *ini)
{
	sim->mem = mem_new();

	ini_get_ram (sim->mem, ini, &sim->ram);
	ini_get_rom (sim->mem, ini);
}

static
void rc759_setup_ports (rc759_t *sim, ini_sct_t *ini)
{
	sim->iop = mem_new();

	mem_set_fct (sim->iop, sim,
		rc759_get_port8, rc759_get_port16, NULL,
		rc759_set_port8, rc759_set_port16, NULL
	);
}

static
void rc759_setup_cpu (rc759_t *sim, ini_sct_t *ini)
{
	pce_log_tag (MSG_INF, "CPU:", "model=80186\n");

	sim->cpu = e86_new();

	e86_set_80186 (sim->cpu);

	e86_set_mem (sim->cpu, sim->mem,
		(e86_get_uint8_f) mem_get_uint8,
		(e86_set_uint8_f) mem_set_uint8,
		(e86_get_uint16_f) mem_get_uint16_le,
		(e86_set_uint16_f) mem_set_uint16_le
	);

	e86_set_prt (sim->cpu, sim->iop,
		(e86_get_uint8_f) mem_get_uint8,
		(e86_set_uint8_f) mem_set_uint8,
		(e86_get_uint16_f) mem_get_uint16_le,
		(e86_set_uint16_f) mem_set_uint16_le
	);

	if (sim->ram != NULL) {
		e86_set_ram (sim->cpu, sim->ram->data, sim->ram->size);
	}
	else {
		e86_set_ram (sim->cpu, NULL, 0);
	}
}

static
void rc759_setup_icu (rc759_t *sim, ini_sct_t *ini)
{
	e80186_icu_init (&sim->icu);
	e80186_icu_set_intr_fct (&sim->icu, sim->cpu, e86_irq);
	e86_set_inta_fct (sim->cpu, &sim->icu, e80186_icu_inta);

	e8259_init (&sim->pic);
	e8259_set_int_fct (&sim->pic, &sim->icu, e80186_icu_set_irq_int0);
	e80186_icu_set_inta0_fct (&sim->icu, &sim->pic, e8259_inta);
}

static
void rc759_setup_tcu (rc759_t *sim, ini_sct_t *ini)
{
	e80186_tcu_init (&sim->tcu);

	e80186_tcu_set_input (&sim->tcu, 0, 1);
	e80186_tcu_set_input (&sim->tcu, 1, 1);
	e80186_tcu_set_input (&sim->tcu, 2, 1);

	e80186_tcu_set_int_fct (&sim->tcu, 0, &sim->icu, e80186_icu_set_irq_tmr0);
	e80186_tcu_set_int_fct (&sim->tcu, 1, &sim->icu, e80186_icu_set_irq_tmr1);
	e80186_tcu_set_int_fct (&sim->tcu, 2, &sim->icu, e80186_icu_set_irq_tmr2);

	e80186_tcu_set_out_fct (&sim->tcu, 0, sim, rc759_set_timer0_out);
	e80186_tcu_set_out_fct (&sim->tcu, 1, sim, rc759_set_timer1_out);
}

static
void rc759_setup_dma (rc759_t *sim, ini_sct_t *ini)
{
	e80186_dma_init (&sim->dma);
	e80186_dma_set_getmem_fct (&sim->dma, sim->mem, mem_get_uint8, mem_get_uint16_le);
	e80186_dma_set_setmem_fct (&sim->dma, sim->mem, mem_set_uint8, mem_set_uint16_le);
	e80186_dma_set_getio_fct (&sim->dma, sim->iop, mem_get_uint8, mem_get_uint16_le);
	e80186_dma_set_setio_fct (&sim->dma, sim->iop, mem_set_uint8, mem_set_uint16_le);
	e80186_dma_set_int_fct (&sim->dma, 0, &sim->icu, e80186_icu_set_irq_dma0);
	e80186_dma_set_int_fct (&sim->dma, 1, &sim->icu, e80186_icu_set_irq_dma1);
}

static
void rc759_setup_ppi (rc759_t *sim, ini_sct_t *ini)
{
	e8255_init (&sim->ppi);

	sim->ppi_port_a = 0x02;
	sim->ppi_port_b = 0x87;

	if (sim->ram != NULL) {
		if (sim->flags & RC759_FLAG_MEM2) {
			if (sim->ram->size >= (832* 1024)) {
				sim->ppi_port_a |= 0x00;
			}
			else if (sim->ram->size >= (640 * 1024)) {
				sim->ppi_port_a |= 0x10;
			}
			else if (sim->ram->size >= (512 * 1024)) {
				sim->ppi_port_a |= 0x30;
			}
		}
		else {
			if (sim->ram->size >= (768 * 1024)) {
				sim->ppi_port_a |= 0x20;
			}
			else if (sim->ram->size >= (640 * 1024)) {
				sim->ppi_port_a |= 0x00;
			}
			else if (sim->ram->size >= (384 * 1024)) {
				sim->ppi_port_a |= 0x10;
			}
			else if (sim->ram->size >= (256 * 1024)) {
				sim->ppi_port_a |= 0x30;
			}
		}
	}

	e8255_set_inp_a (&sim->ppi, sim->ppi_port_a);
	e8255_set_inp_b (&sim->ppi, sim->ppi_port_b);

	sim->ppi.port[2].write_ext = sim;
	sim->ppi.port[2].write = (void *) rc759_set_ppi_port_c;
}

static
void rc759_setup_kbd (rc759_t *sim, ini_sct_t *ini)
{
	rc759_kbd_init (&sim->kbd);

	rc759_kbd_set_irq_fct (&sim->kbd, &sim->pic, e8259_set_irq1);
}

static
void rc759_setup_nvm (rc759_t *sim, ini_sct_t *ini)
{
	const char *nvm;
	int        sanitize;
	ini_sct_t  *sct;

	sct = ini_next_sct (ini, NULL, "system");

	ini_get_string (sct, "nvm", &nvm, "nvm.dat");
	ini_get_bool (sct, "sanitize_nvm", &sanitize, 0);

	pce_log_tag (MSG_INF, "NVM:",
		"file=%s sanitize=%d\n", nvm, sanitize
	);

	rc759_nvm_init (&sim->nvm);
	rc759_nvm_set_fname (&sim->nvm, nvm);

	if (rc759_nvm_load (&sim->nvm)) {
		pce_log (MSG_ERR, "*** error loading the NVM (%s)\n",
			(nvm != NULL) ? nvm : "<none>"
		);
	}

	if (sanitize) {
		rc759_nvm_sanitize (&sim->nvm);
		rc759_nvm_fix_checksum (&sim->nvm);
	}
}

static
void rc759_setup_rtc (rc759_t *sim, ini_sct_t *ini)
{
	rc759_rtc_init (&sim->rtc);
	rc759_rtc_set_time_now (&sim->rtc);
	rc759_rtc_set_irq_fct (&sim->rtc, &sim->pic, e8259_set_irq3);
	rc759_rtc_set_input_clock (&sim->rtc, sim->cpu_clock_frq);
}

static
void rc759_setup_fdc (rc759_t *sim, ini_sct_t *ini)
{
	const char *fname0, *fname1;
	ini_sct_t  *sct;

	sct = ini_next_sct (ini, NULL, "fdc");

	ini_get_string (sct, "file0", &fname0, NULL);
	ini_get_string (sct, "file1", &fname1, NULL);

	pce_log_tag (MSG_INF, "FDC:",
		"file0=%s file1=%s\n",
		(fname0 != NULL) ? fname0 : "<none>",
		(fname1 != NULL) ? fname1 : "<none>"
	);

	rc759_fdc_init (&sim->fdc);

	wd179x_set_irq_fct (&sim->fdc.wd179x, &sim->pic, e8259_set_irq0);
	wd179x_set_drq_fct (&sim->fdc.wd179x, &sim->dma, e80186_dma_set_dreq0);

	wd179x_set_input_clock (&sim->fdc.wd179x, sim->cpu_clock_frq);
	wd179x_set_bit_clock (&sim->fdc.wd179x, 2000000);

	rc759_fdc_set_disks (&sim->fdc, sim->dsks);

	rc759_fdc_set_fname (&sim->fdc, 0, fname0);
	rc759_fdc_set_fname (&sim->fdc, 1, fname1);

	rc759_fdc_set_disk_id (&sim->fdc, 0, 0);
	rc759_fdc_set_disk_id (&sim->fdc, 1, 1);

	rc759_fdc_load (&sim->fdc, 0);
	rc759_fdc_load (&sim->fdc, 1);
}

static
void rc759_setup_speaker (rc759_t *sim, ini_sct_t *ini)
{
	const char    *driver;
	unsigned      volume;
	unsigned long srate;
	unsigned long lowpass;
	ini_sct_t     *sct;

	rc759_spk_init (&sim->spk);
	rc759_spk_set_clk_fct (&sim->spk, sim, rc759_get_cpu_clock);
	rc759_spk_set_input_clock (&sim->spk, sim->cpu_clock_frq);

	sct = ini_next_sct (ini, NULL, "speaker");

	if (sct == NULL) {
		return;
	}

	ini_get_string (sct, "driver", &driver, NULL);
	ini_get_uint16 (sct, "volume", &volume, 500);
	ini_get_uint32 (sct, "sample_rate", &srate, 44100);
	ini_get_uint32 (sct, "lowpass", &lowpass, 0);

	pce_log_tag (MSG_INF, "SPEAKER:", "volume=%u srate=%lu lowpass=%lu driver=%s\n",
		volume, srate, lowpass,
		(driver != NULL) ? driver : "<none>"
	);

	if (driver != NULL) {
		if (rc759_spk_set_driver (&sim->spk, driver, srate)) {
			pce_log (MSG_ERR,
				"*** setting sound driver failed (%s)\n",
				driver
			);
		}
	}

	rc759_spk_set_lowpass (&sim->spk, lowpass);
	rc759_spk_set_volume (&sim->spk, volume);
}

static
void rc759_setup_terminal (rc759_t *sim, ini_sct_t *ini)
{
	sim->trm = ini_get_terminal (ini, par_terminal);

	if (sim->trm == NULL) {
		return;
	}

	trm_set_key_fct (sim->trm, &sim->kbd, rc759_kbd_set_key);
	trm_set_mouse_fct (sim->trm, sim, rc759_set_mouse);
	trm_set_msg_fct (sim->trm, sim, rc759_set_msg);
}

static
void rc759_setup_video (rc759_t *sim, ini_sct_t *ini)
{
	unsigned  min_h;
	int       mono, hires;
	ini_sct_t *sct;

	sct = ini_next_sct (ini, NULL, "video");

	if (sct == NULL) {
		sct = ini;
	}

	ini_get_uint16 (sct, "min_h", &min_h, 0);
	ini_get_bool (sct, "mono", &mono, 0);
	ini_get_bool (sct, "hires", &hires, 0);

	if (par_video != NULL) {
		if (strcmp (par_video, "mono") == 0) {
			mono = 1;
		}
		else if (strcmp (par_video, "color") == 0) {
			mono = 0;
		}
		else {
			pce_log (MSG_ERR,
				"*** unknown video type (%s)\n",
				par_video
			);
		}
	}

	e82730_init (&sim->crt);

	e82730_set_getmem_fct (&sim->crt, sim->mem, mem_get_uint8, mem_get_uint16_le);
	e82730_set_setmem_fct (&sim->crt, sim->mem, mem_set_uint8, mem_set_uint16_le);
	e82730_set_sint_fct (&sim->crt, &sim->pic, e8259_set_irq4);
	e82730_set_terminal (&sim->crt, sim->trm);
	e82730_set_monochrome (&sim->crt, mono);
	e82730_set_min_h (&sim->crt, min_h);

	pce_log_tag (MSG_INF, "VIDEO:", "monochrome=%d 22KHz=%d min_h=%u\n",
		mono, hires, min_h
	);

	if (hires) {
		sim->ppi_port_b |= 0x40;
		e82730_set_clock (&sim->crt, 1250000, sim->cpu_clock_frq);
	}
	else {
		sim->ppi_port_b &= ~0x40;
		e82730_set_clock (&sim->crt, 750000, sim->cpu_clock_frq);
	}

	if (mono) {
		sim->ppi_port_b |= 0x20;
	}
	else {
		sim->ppi_port_b &= ~0x20;
	}

	e8255_set_inp_b (&sim->ppi, sim->ppi_port_b);

	if (sim->trm != NULL) {
		if (hires) {
			trm_open (sim->trm, 720, 341);
		}
		else {
			trm_open (sim->trm, 560, 260);
		}

		trm_set_msg_trm (sim->trm, "term.title", "pce-rc759");
	}
}

static
void rc759_setup_disks (rc759_t *sim, ini_sct_t *ini)
{
	ini_sct_t *sct;
	disk_t    *dsk;

	sim->dsks = dsks_new();

	sct = NULL;
	while ((sct = ini_next_sct (ini, sct, "disk")) != NULL) {
		if (ini_get_disk (sct, &dsk)) {
			pce_log (MSG_ERR, "*** loading drive failed\n");
			continue;
		}

		if (dsk == NULL) {
			continue;
		}

		dsks_add_disk (sim->dsks, dsk);
	}
}

static
void rc759_setup_parport (rc759_t *sim, ini_sct_t *ini)
{
	const char *driver1, *driver2;
	ini_sct_t  *sct;

	sct = ini_next_sct (ini, NULL, "system");

	ini_get_string (sct, "parport", &driver1, NULL);
	ini_get_string (sct, "parport1", &driver1, driver1);
	ini_get_string (sct, "parport2", &driver2, NULL);

	pce_log_tag (MSG_INF,
		"PARPORT1:", "type=local driver=%s\n",
		(driver1 == NULL) ? "<none>" : driver1
	);

	rc759_par_init (&sim->par[0]);
	rc759_par_set_irq_fct (&sim->par[0], &sim->pic, e8259_set_irq6);

	if (driver1 != NULL) {
		if (rc759_par_set_driver (&sim->par[0], driver1)) {
			pce_log (MSG_ERR, "*** can't open driver (%s)\n",
				driver1
			);
		}
	}

	pce_log_tag (MSG_INF,
		"PARPORT2:", "type=remote driver=%s\n",
		(driver2 == NULL) ? "<none>" : driver2
	);

	rc759_par_init (&sim->par[1]);
	rc759_par_set_irq_fct (&sim->par[1], &sim->pic, e8259_set_irq2);

	if (driver2 != NULL) {
		if (rc759_par_set_driver (&sim->par[1], driver2)) {
			pce_log (MSG_ERR, "*** can't open driver (%s)\n",
				driver2
			);
		}
	}
}

rc759_t *rc759_new (ini_sct_t *ini)
{
	rc759_t *sim;

	sim = malloc (sizeof (rc759_t));

	if (sim == NULL) {
		return (NULL);
	}

	memset (sim, 0, sizeof (rc759_t));

	sim->cfg = ini;

	bps_init (&sim->bps);
	rc759_setup_system (sim, ini);
	rc759_setup_mem (sim, ini);
	rc759_setup_ports (sim, ini);
	rc759_setup_cpu (sim, ini);
	rc759_setup_icu (sim, ini);
	rc759_setup_tcu (sim, ini);
	rc759_setup_dma (sim, ini);
	rc759_setup_ppi (sim, ini);
	rc759_setup_kbd (sim, ini);
	rc759_setup_nvm (sim, ini);
	rc759_setup_rtc (sim, ini);
	rc759_setup_speaker (sim, ini);
	rc759_setup_terminal (sim, ini);
	rc759_setup_video (sim, ini);
	rc759_setup_disks (sim, ini);
	rc759_setup_fdc (sim, ini);
	rc759_setup_parport (sim, ini);

	pce_load_mem_ini (sim->mem, ini);

	mem_move_to_front (sim->mem, 0xf8000);

	rc759_clock_reset (sim);

	return (sim);
}

void rc759_del (rc759_t *sim)
{
	if (sim == NULL) {
		return;
	}

	bps_free (&sim->bps);
	rc759_par_free (&sim->par[1]);
	rc759_par_free (&sim->par[0]);
	rc759_fdc_free (&sim->fdc);
	dsks_del (sim->dsks);
	trm_del (sim->trm);
	rc759_rtc_free (&sim->rtc);
	rc759_nvm_free (&sim->nvm);
	rc759_spk_free (&sim->spk);
	e8255_free (&sim->ppi);
	e80186_dma_free (&sim->dma);
	e80186_tcu_free (&sim->tcu);
	e8259_free (&sim->pic);
	e80186_icu_free (&sim->icu);
	e86_del (sim->cpu);
	mem_del (sim->mem);
	mem_del (sim->iop);
	ini_sct_del (sim->cfg);

	free (sim);
}

int rc759_set_parport_driver (rc759_t *sim, unsigned port, const char *driver)
{
	if (port > 1) {
		return (1);
	}

	if (rc759_par_set_driver (&sim->par[port], driver)) {
		return (1);
	}

	return (0);
}

int rc759_set_parport_file (rc759_t *sim, unsigned port, const char *fname)
{
	int  r;
	char *driver;

	if (port > 1) {
		return (1);
	}

	driver = str_cat_alloc ("stdio:file=", fname);

	r = rc759_set_parport_driver (sim, port, driver);

	free (driver);

	return (r);
}

const char *rc759_intlog_get (rc759_t *sim, unsigned n)
{
	return (par_intlog[n & 0xff]);
}

void rc759_intlog_set (rc759_t *sim, unsigned n, const char *expr)
{
	char **str;

	str = &par_intlog[n & 0xff];

	free (*str);

	if ((expr == NULL) || (*expr == 0)) {
		*str = NULL;
		return;
	}

	*str = str_copy_alloc (expr);
}

int rc759_intlog_check (rc759_t *sim, unsigned n)
{
	unsigned long val;
	const char    *str;
	cmd_t         cmd;

	str = par_intlog[n & 0xff];

	if (str == NULL) {
		return (0);
	}

	cmd_set_str (&cmd, str);

	if (cmd_match_uint32 (&cmd, &val)) {
		if (val) {
			return (1);
		}
	}

	return (0);
}

void rc759_reset (rc759_t *sim)
{
	sim_log_deb ("reset system\n");

	e86_reset (sim->cpu);
	e82730_reset (&sim->crt);
	e8259_reset (&sim->pic);
	e80186_tcu_reset (&sim->tcu);
	e80186_dma_reset (&sim->dma);
	e80186_icu_reset (&sim->icu);
	rc759_kbd_reset (&sim->kbd);
	rc759_rtc_reset (&sim->rtc);
	rc759_fdc_reset (&sim->fdc);
	rc759_par_reset (&sim->par[0]);
	rc759_par_reset (&sim->par[1]);
}

void rc759_set_cpu_clock (rc759_t *sim, unsigned long clk)
{
	if (sim->cpu_clock_frq == clk) {
		return;
	}

	pce_log_tag (MSG_INF, "CPU:", "setting clock to %lu\n", clk);

	sim->cpu_clock_frq = clk;

	wd179x_set_input_clock (&sim->fdc.wd179x, sim->cpu_clock_frq);
	rc759_rtc_set_input_clock (&sim->rtc, clk);
	rc759_spk_set_input_clock (&sim->spk, sim->cpu_clock_frq);

	if (sim->ppi_port_b & 0x40) {
		/* hires */
		e82730_set_clock (&sim->crt, 1250000, sim->cpu_clock_frq);
	}
	else {
		e82730_set_clock (&sim->crt, 750000, sim->cpu_clock_frq);
	}
}

void rc759_set_speed (rc759_t *sim, unsigned factor)
{
	rc759_set_cpu_clock (sim, (4 + factor) * 1000000);
}

unsigned long rc759_get_cpu_clock (rc759_t *sim)
{
	return (sim->cpu_clock_cnt);
}


void rc759_clock_reset (rc759_t *sim)
{
	sim->sync_clock_sim = 0;
	sim->sync_clock_real = 0;

	pce_get_interval_us (&sim->sync_interval);

	sim->cpu_clock_cnt = 0;
	sim->cpu_clock_rem8 = 0;
	sim->cpu_clock_rem1024 = 0;
	sim->cpu_clock_rem32768 = 0;
}

void rc759_clock_discontinuity (rc759_t *sim)
{
	sim->sync_clock_real = sim->sync_clock_sim;
	pce_get_interval_us (&sim->sync_interval);
}

/*
 * Synchronize the system clock with real time
 */
static
void rc759_clock_delay (rc759_t *sim)
{
	unsigned long vclk;
	unsigned long rclk;
	unsigned long us;

	vclk = sim->sync_clock_sim;

	rclk = pce_get_interval_us (&sim->sync_interval);
	rclk = (sim->cpu_clock_frq * (unsigned long long) rclk) / 1000000;
	rclk += sim->sync_clock_real;

	if (vclk < rclk) {
		sim->sync_clock_sim = 0;
		sim->sync_clock_real = rclk - vclk;

		if (sim->sync_clock_real > sim->cpu_clock_frq) {
			sim->sync_clock_real = 0;
			pce_log (MSG_INF, "host system too slow, skipping 1 second.\n");
		}

		return;
	}

	vclk -= rclk;

	sim->sync_clock_sim = vclk;
	sim->sync_clock_real = 0;

	us = (1000000 * (unsigned long long) vclk) / sim->cpu_clock_frq;

	if (us > PCE_IBMPC_SLEEP) {
		pce_usleep (us);
	}
}

void rc759_clock (rc759_t *sim, unsigned cnt)
{
	unsigned long clk;

	if (cnt == 0) {
		cnt = 4;
	}

	e86_clock (sim->cpu, cnt);
	e80186_tcu_clock (&sim->tcu, cnt);
	e80186_dma_clock (&sim->dma, cnt);

	sim->sync_clock_sim += cnt;
	sim->cpu_clock_cnt += cnt;
	sim->cpu_clock_rem8 += cnt;

	if (sim->cpu_clock_rem8 < 8) {
		return;
	}

	clk = sim->cpu_clock_rem8;
	sim->cpu_clock_rem8 &= 7;
	clk -= sim->cpu_clock_rem8;

	e82730_clock (&sim->crt, clk);
	rc759_fdc_clock (&sim->fdc.wd179x, clk);
	rc759_rtc_clock (&sim->rtc, clk);

	sim->cpu_clock_rem1024 += clk;

	if (sim->cpu_clock_rem1024 < 1024) {
		return;
	}

	clk = sim->cpu_clock_rem1024;
	sim->cpu_clock_rem1024 &= 1023;
	clk -= sim->cpu_clock_rem1024;

	if (sim->trm != NULL) {
		trm_check (sim->trm);
	}

	rc759_kbd_clock (&sim->kbd, clk);
	rc759_spk_clock (&sim->spk, clk);

	sim->cpu_clock_rem32768 += clk;

	if (sim->cpu_clock_rem32768 < 32768) {
		return;
	}

	clk = sim->cpu_clock_rem32768;
	sim->cpu_clock_rem32768 &= 32767;
	clk -= sim->cpu_clock_rem32768;

	rc759_clock_delay (sim);
}
