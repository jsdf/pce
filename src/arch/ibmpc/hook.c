/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/hook.c                                        *
 * Created:     2003-09-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2010 Hampa Hug <hampa@hampa.ch>                     *
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
#include "ems.h"
#include "hook.h"
#include "ibmpc.h"
#include "int13.h"
#include "msg.h"
#include "xms.h"

#include <time.h>

#include <cpu/e8086/e8086.h>

#include <lib/log.h>


static
void pc_hook_log (ibmpc_t *pc, unsigned char op1, unsigned char op2)
{
	pce_log (MSG_DEB,
		"pce: hook [%02x %02x] AX=%04X BX=%04X CX=%04X DX=%04X\n",
		op1, op2,
		e86_get_ax (pc->cpu), e86_get_bx (pc->cpu),
		e86_get_cx (pc->cpu), e86_get_dx (pc->cpu)
	);
}

void pc_int_15 (ibmpc_t *pc)
{
	unsigned short seg, ofs;
	unsigned long  size, src, dst;

	switch (e86_get_ah (pc->cpu)) {
	case 0x87:
		seg = e86_get_es (pc->cpu);
		ofs = e86_get_si (pc->cpu);
		size = e86_get_cx (pc->cpu) * 2;
		src = e86_get_mem16 (pc->cpu, seg, ofs + 0x12);
		src |= (e86_get_mem8 (pc->cpu, seg, ofs + 0x14) & 0xff) << 16;
		dst = e86_get_mem16 (pc->cpu, seg, ofs + 0x1a);
		dst |= (e86_get_mem8 (pc->cpu, seg, ofs + 0x1c) & 0xff) << 16;
		pce_log (MSG_DEB, "int15: copy %08x -> %08x, %04x\n",
			src, dst, size
		);
		e86_set_cf (pc->cpu, 1);
		e86_set_ah (pc->cpu, 0x86);
		break;

	case 0x88:
		/* No extended memory. Only XMS. */
		e86_set_cf (pc->cpu, 0);
		e86_set_ax (pc->cpu, 0x0000);
		e86_set_cf (pc->cpu, 1);
		break;

	default:
		e86_set_ah (pc->cpu, 0x86);
		e86_set_cf (pc->cpu, 1);
		break;
	}
}

static
unsigned get_bcd_8 (unsigned n)
{
	return ((n % 10) + 16 * ((n / 10) % 10));
}

void pc_int_1a (ibmpc_t *pc)
{
	unsigned ah;

	ah = e86_get_ah (pc->cpu);

	e86_set_cf (pc->cpu, 1);

	if (ah == 0x00) {
		e86_set_dx (pc->cpu, e86_get_mem16 (pc->cpu, 0x40, 0x6c));
		e86_set_cx (pc->cpu, e86_get_mem16 (pc->cpu, 0x40, 0x6e));
		e86_set_al (pc->cpu, e86_get_mem8 (pc->cpu, 0x40, 0x70));
		e86_set_mem8 (pc->cpu, 0x40, 0x70, 0);
		e86_set_cf (pc->cpu, 0);
	}
	else if (ah == 0x01) {
		e86_set_mem16 (pc->cpu, 0x40, 0x6c, e86_get_dx (pc->cpu));
		e86_set_mem16 (pc->cpu, 0x40, 0x6e, e86_get_cx (pc->cpu));
		e86_set_cf (pc->cpu, 0);
	}
	else if (pc->support_rtc) {
		time_t    tm;
		struct tm *tval;

		if (ah == 0x02) {
			tm = time (NULL);
			tval = localtime (&tm);
			e86_set_ch (pc->cpu, get_bcd_8 (tval->tm_hour));
			e86_set_cl (pc->cpu, get_bcd_8 (tval->tm_min));
			e86_set_dh (pc->cpu, get_bcd_8 (tval->tm_sec));
			e86_set_cf (pc->cpu, 0);
		}
		else if (ah == 0x03) {
			e86_set_cf (pc->cpu, 0);
			return;
		}
		else if (ah == 0x04) {
			tm = time (NULL);
			tval = localtime (&tm);
			e86_set_ch (pc->cpu, get_bcd_8 ((1900 + tval->tm_year) / 100));
			e86_set_cl (pc->cpu, get_bcd_8 (1900 + tval->tm_year));
			e86_set_dh (pc->cpu, get_bcd_8 (tval->tm_mon + 1));
			e86_set_dl (pc->cpu, get_bcd_8 (tval->tm_mday));
			e86_set_cf (pc->cpu, 0);
		}
		else if (ah == 0x05) {
			e86_set_cf (pc->cpu, 0);
		}
	}
}

static
void pc_hook_version (ibmpc_t *pc)
{
	unsigned short es, di;
	const char     *str;

	es = e86_get_es (pc->cpu);
	di = e86_get_di (pc->cpu);
	str = PCE_VERSION_STR;

	e86_set_ax (pc->cpu, (PCE_VERSION_MAJ << 8) | PCE_VERSION_MIN);
	e86_set_dx (pc->cpu, (PCE_VERSION_MIC << 8));

	if ((es == 0) && (di == 0)) {
		return;
	}

	while (*str != 0) {
		e86_set_mem8 (pc->cpu, es, di, *str);
		di = (di + 1) & 0xffff;
		str += 1;
	}

	e86_set_mem8 (pc->cpu, es, di, 0);
}

static
void pc_hook_msg (ibmpc_t *pc)
{
	unsigned       i, p;
	unsigned short ds;
	char           msg[256];
	char           val[256];

	e86_set_cf (pc->cpu, 1);

	ds = e86_get_ds (pc->cpu);

	p = e86_get_si (pc->cpu);
	for (i = 0; i < 256; i++) {
		msg[i] = e86_get_mem16 (pc->cpu, ds, p++);
		if (msg[i] == 0) {
			break;
		}
	}

	if (i >= 256) {
		return;
	}

	p = e86_get_di (pc->cpu);
	for (i = 0; i < 256; i++) {
		val[i] = e86_get_mem16 (pc->cpu, ds, p++);
		if (val[i] == 0) {
			break;
		}
	}

	if (i >= 256) {
		return;
	}

	if (pc_set_msg (pc, msg, val)) {
		e86_set_cf (pc->cpu, 1);
		e86_set_ax (pc->cpu, 0x0001);
	}
	else {
		e86_set_cf (pc->cpu, 0);
		e86_set_ax (pc->cpu, 0x0000);
	}
}

void pc_e86_hook (void *ext, unsigned char op1, unsigned char op2)
{
	ibmpc_t       *pc;
	unsigned long msk;

	pc = (ibmpc_t *) ext;

	switch (op1) {
	case (PCEH_INT & 0xff):
		if (op2 == 0x13) {
			dsk_int13 (pc->dsk, pc->cpu);
			return;
		}
		else if (op2 == 0x1a) {
			pc_int_1a (pc);
			return;
		}
		else if (op2 == 0x15) {
			pc_int_15 (pc);
			return;
		}
		break;

	case (PCEH_CHECK_INT & 0xff):
		e86_set_ax (pc->cpu, 0);

		if (op2 == 0x13) {
			dsk_int_13_check (pc);
		}
		return;
	}

	e86_set_cf (pc->cpu, 0);

	switch ((op2 << 8) | op1) {
	case PCEH_STOP:
		pc->brk = 1;
		break;

	case PCEH_ABORT:
		pc->brk = 2;
		break;

	case PCEH_SET_BOOT:
		pc_set_bootdrive (pc, e86_get_al (pc->cpu));
		break;

	case PCEH_SET_INT28:
		/* not supported anymore */
		break;

	case PCEH_SET_CPU:
		/* not supported anymore */
		break;

	case PCEH_SET_AMSK:
		msk = (e86_get_dx (pc->cpu) << 16) + e86_get_ax (pc->cpu);
		e86_set_addr_mask (pc->cpu, msk);
		break;

	case PCEH_GET_BOOT:
		e86_set_al (pc->cpu, pc_get_bootdrive (pc));
		break;

	case PCEH_GET_COM:
		e86_set_ax (pc->cpu, (pc->serport[0] != NULL) ? pc->serport[0]->io : 0);
		e86_set_bx (pc->cpu, (pc->serport[1] != NULL) ? pc->serport[1]->io : 0);
		e86_set_cx (pc->cpu, (pc->serport[2] != NULL) ? pc->serport[2]->io : 0);
		e86_set_dx (pc->cpu, (pc->serport[3] != NULL) ? pc->serport[3]->io : 0);
		break;

	case PCEH_GET_LPT:
		e86_set_ax (pc->cpu, (pc->parport[0] != NULL) ? pc->parport[0]->io : 0);
		e86_set_bx (pc->cpu, (pc->parport[1] != NULL) ? pc->parport[1]->io : 0);
		e86_set_cx (pc->cpu, (pc->parport[2] != NULL) ? pc->parport[2]->io : 0);
		e86_set_dx (pc->cpu, (pc->parport[3] != NULL) ? pc->parport[3]->io : 0);
		break;

	case PCEH_GET_VIDEO:
		/* not supported anymore */
		e86_set_ax (pc->cpu, 0);
		break;

	case PCEH_GET_INT28:
		/* not supported anymore */
		e86_set_ax (pc->cpu, 0);
		break;

	case PCEH_GET_CPU:
		/* not supported anymore */
		e86_set_ax (pc->cpu, 0);
		break;

	case PCEH_GET_AMSK:
		msk = e86_get_addr_mask (pc->cpu);
		e86_set_ax (pc->cpu, msk & 0xffff);
		e86_set_dx (pc->cpu, (msk >> 16) & 0xffff);
		break;

	case PCEH_GET_FDCNT:
		e86_set_ax (pc->cpu, pc->fd_cnt);
		break;

	case PCEH_GET_HDCNT:
		e86_set_ax (pc->cpu, pc->hd_cnt);
		break;

	case PCEH_GET_VERS:
		pc_hook_version (pc);
		break;

	case PCEH_XMS:
		xms_handler (pc->xms, pc->cpu);
		break;

	case PCEH_XMS_INFO:
		xms_info (pc->xms, pc->cpu);
		break;

	case PCEH_EMS:
		ems_handler (pc->ems, pc->cpu);
		break;

	case PCEH_EMS_INFO:
		ems_info (pc->ems, pc->cpu);
		break;

	case PCEH_MSG:
		pc_hook_msg (pc);
		break;

	default:
		e86_set_cf (pc->cpu, 1);
		pc_hook_log (pc, op1, op2);
		break;
	}
}
