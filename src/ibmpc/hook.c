/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/hook.c                                           *
 * Created:       2003-09-02 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-10-28 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003 by Hampa Hug <hampa@hampa.ch>                     *
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

/* $Id: hook.c,v 1.10 2003/10/28 13:19:41 hampa Exp $ */


#include "pce.h"

#include <time.h>


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
  switch (e86_get_ah (pc->cpu)) {
    case 0x87:
      {
        unsigned short seg, ofs;
        unsigned long  size, src, dst;

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
      }
      e86_set_cf (pc->cpu, 1);
      e86_set_ah (pc->cpu, 0x86);
      break;

    case 0x88:
      /* No extended memory. Only XMS. */
      e86_set_cf (pc->cpu, 0);
      e86_set_ax (pc->cpu, 0x0000);
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
  time_t    tm;
  struct tm *tval;

  switch (e86_get_ah (pc->cpu)) {
    case 0x00:
      e86_set_dx (pc->cpu, e86_get_mem16 (pc->cpu, 0x40, 0x6c));
      e86_set_cx (pc->cpu, e86_get_mem16 (pc->cpu, 0x40, 0x6e));
      e86_set_al (pc->cpu, e86_get_mem8 (pc->cpu, 0x40, 0x70));
      e86_set_mem8 (pc->cpu, 0x40, 0x70, 0);
      e86_set_cf (pc->cpu, 0);
      break;

    case 0x01:
      e86_set_mem16 (pc->cpu, 0x40, 0x6c, e86_get_dx (pc->cpu));
      e86_set_mem16 (pc->cpu, 0x40, 0x6e, e86_get_cx (pc->cpu));
      e86_set_cf (pc->cpu, 0);
      break;

    case 0x02:
      tm = time (NULL);
      tval = localtime (&tm);
      e86_set_ch (pc->cpu, get_bcd_8 (tval->tm_hour));
      e86_set_cl (pc->cpu, get_bcd_8 (tval->tm_min));
      e86_set_dh (pc->cpu, get_bcd_8 (tval->tm_sec));
      e86_set_cf (pc->cpu, 0);
      break;

    case 0x03:
      break;

    case 0x04:
      tm = time (NULL);
      tval = localtime (&tm);
      e86_set_ch (pc->cpu, get_bcd_8 ((1900 + tval->tm_year) / 100));
      e86_set_cl (pc->cpu, get_bcd_8 (1900 + tval->tm_year));
      e86_set_dh (pc->cpu, get_bcd_8 (tval->tm_mon + 1));
      e86_set_dl (pc->cpu, get_bcd_8 (tval->tm_mday));
      e86_set_cf (pc->cpu, 0);
      break;

    case 0x05:
      break;

    default:
      e86_set_cf (pc->cpu, 1);
      break;
  }
}

void pc_e86_hook (void *ext, unsigned char op1, unsigned char op2)
{
  ibmpc_t *pc;

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
      pce_log (MSG_INF, "set boot drive to %u\n", e86_get_al (pc->cpu));
      par_boot = e86_get_al (pc->cpu);
      break;

    case PCEH_SET_INT28:
      par_int28 = 1000UL * e86_get_ax (pc->cpu);
      break;

    case PCEH_SET_CPU:
      if (pc_set_cpu_model (pc, e86_get_ax (pc->cpu))) {
        pce_log (MSG_ERR, "can't set cpu model to %u\n", e86_get_ax (pc->cpu));
      }
      break;

    case PCEH_SET_AMSK:
      {
        unsigned long msk;
        msk = (e86_get_dx (pc->cpu) << 16) + e86_get_ax (pc->cpu);
        e86_set_addr_mask (pc->cpu, msk);
      }
      break;

    case PCEH_GET_BOOT:
      e86_set_al (pc->cpu, par_boot);
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
      e86_set_ax (pc->cpu, pc->video->type);
      break;

    case PCEH_GET_INT28:
      e86_set_ax (pc->cpu, par_int28 / 1000UL);
      break;

    case PCEH_GET_CPU:
      e86_set_ax (pc->cpu, pc->cpu_model);
      break;

    case PCEH_GET_AMSK:
      {
        unsigned long msk;
        msk = e86_get_addr_mask (pc->cpu);
        e86_set_ax (pc->cpu, msk & 0xffff);
        e86_set_dx (pc->cpu, (msk >> 16) & 0xffff);
      }
      break;

    case PCEH_GET_VERS:
      e86_set_ax (pc->cpu, (PCE_VERSION_MAJ << 8) | PCE_VERSION_MIN);
      e86_set_dx (pc->cpu, (PCE_VERSION_MIC << 8) | PCE_VERSION_REL);
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

    default:
      e86_set_cf (pc->cpu, 1);
      pc_hook_log (pc, op1, op2);
      break;
  }
}
