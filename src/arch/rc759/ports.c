/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/ports.c                                       *
 * Created:     2021-08-21 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2021 Hampa Hug <hampa@hampa.ch>                          *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include "main.h"
#include "fdc.h"
#include "keyboard.h"
#include "msg.h"
#include "nvm.h"
#include "parport.h"
#include "rc759.h"
#include "rtc.h"
#include "video.h"

#include <chipset/80186/dma.h>
#include <chipset/80186/icu.h>
#include <chipset/80186/tcu.h>

#include <chipset/82xx/e8250.h>
#include <chipset/82xx/e8255.h>
#include <chipset/82xx/e8259.h>


unsigned char rc759_get_port8 (rc759_t *sim, unsigned long addr)
{
	unsigned char val;

	val = 0xff;

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

	if ((addr >= 0x300) && (addr <= 0x31f)) {
		/* iSBX */
		return (0xff);
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

	case 0x0200: /* ? */
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

	case 0x02fa: /* ? */
		break;

	case 0x03fa: /* ? */
		break;

	default:
		sim_log_deb ("get port 8 %04lX <- %02X\n", addr, val);
		break;
	}

	return (val);
}

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

	if ((addr >= 0x300) && (addr <= 0x31f)) {
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

	case 0x02a0: /* ? */
		break;

	case 0xff28:
		e80186_icu_set_imr (&sim->icu, val);
		break;

	default:
		sim_log_deb ("set port 8 %04lX <- %02X\n", addr, val);
		break;
	}
}

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
