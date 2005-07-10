/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/ibmpc/msg.c                                       *
 * Created:       2004-09-25 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2005-06-04 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004-2005 Hampa Hug <hampa@hampa.ch>                   *
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


#include "main.h"

static
unsigned long msg_get_ulng (const char *str)
{
  char          *end;
  unsigned long val;

  val = strtoul (str, &end, 0);

  if ((end == str) || (*end != 0)) {
    return (0);
  }

  return (val);
}

int pc_set_msg (ibmpc_t *pc, const char *msg, const char *val)
{
  /* a hack, for debugging only */
  if (pc == NULL) {
    pc = par_pc;
  }

  if (msg == NULL) {
    msg = "";
  }

  if (val == NULL) {
    val = "";
  }

  if (strcmp (msg, "emu.stop") == 0) {
    pc->brk = PCE_BRK_STOP;
    return (0);
  }
  else if (strcmp (msg, "emu.exit") == 0) {
    pc->brk = PCE_BRK_ABORT;
    return (0);
  }
  else if (strcmp (msg, "emu.pause") == 0) {
    pc->pause = (msg_get_ulng (val) != 0);
    return (0);
  }
  else if (strcmp (msg, "emu.pause.toggle") == 0) {
    pc->pause = (pc->pause == 0);
    return (0);
  }

  pce_log (MSG_DEB, "msg (\"%s\", \"%s\")\n", msg, val);

  if (strcmp (msg, "emu.idle") == 0) {
    par_int28 = 1000UL * msg_get_ulng (val);
    return (0);
  }
  else if (strcmp (msg, "emu.idle.toggle") == 0) {
    par_int28 = (par_int28 == 0) ? 10000UL : 0UL;
    return (0);
  }
  else if (strcmp (msg, "emu.config.save") == 0) {
    if (ini_write (pc->cfg, val)) {
      return (1);
    }

    return (0);
  }
  else if (strcmp (msg, "emu.realtime") == 0) {
    pc->pit_real = (msg_get_ulng (val) != 0);
    return (0);
  }
  else if (strcmp (msg, "emu.realtime.toggle") == 0) {
    pc->pit_real = !pc->pit_real;
    return (0);
  }
  else if (strcmp (msg, "cpu.model") == 0) {
    if (strcmp (val, "8086") == 0) {
      pc_set_cpu_model (pc, PCE_CPU_8086);
    }
    else if (strcmp (val, "8088") == 0) {
      pc_set_cpu_model (pc, PCE_CPU_8088);
    }
    else if (strcmp (val, "v20") == 0) {
      pc_set_cpu_model (pc, PCE_CPU_V20);
    }
    else if (strcmp (val, "v30") == 0) {
      pc_set_cpu_model (pc, PCE_CPU_V30);
    }
    else if (strcmp (val, "80186") == 0) {
      pc_set_cpu_model (pc, PCE_CPU_80186);
    }
    else if (strcmp (val, "80188") == 0) {
      pc_set_cpu_model (pc, PCE_CPU_80188);
    }
    else if (strcmp (val, "80286") == 0) {
      pc_set_cpu_model (pc, PCE_CPU_80286);
    }
    else {
      return (1);
    }

    return (0);
  }
  else if (strcmp (msg, "video.redraw") == 0) {
    pce_video_update (pc->video);
    return (0);
  }
  else if (strcmp (msg, "video.screenshot") == 0) {
    if (strcmp (val, "") == 0) {
      pc_screenshot (pc, NULL);
    }
    else {
      pc_screenshot (pc, val);
    }
    return (0);
  }
  else if (strcmp (msg, "video.size") == 0) {
    unsigned w, h;
    char     *tmp1, *tmp2;

    w = strtoul (val, &tmp1, 0);
    if ((w == 0) || (tmp1 == val) || (*tmp1 == 0)) {
      return (1);
    }

    h = strtoul (tmp1, &tmp2, 0);
    if ((h == 0) || (tmp2 == tmp1) || (*tmp2 != 0)) {
      return (1);
    }

    trm_set_size (pc->trm, w, h);
    pce_video_update (pc->video);

    return (0);
  }
  else if (strcmp (msg, "disk.boot") == 0) {
    pc->bootdrive = strtoul (val, NULL, 0);
    return (0);
  }
  else if (strcmp (msg, "disk.commit") == 0) {
    if (strcmp (val, "") == 0) {
      if (dsks_commit (pc->dsk, 1)) {
        pce_log (MSG_ERR, "commit failed for at least one disk\n");
        return (1);
      }
    }
    else {
      unsigned d;

      d = strtoul (val, NULL, 0);

      if (dsks_commit_disk (pc->dsk, d, 1)) {
        pce_log (MSG_ERR, "commit failed (%s)\n", val);
        return (1);
      }
    }

    return (0);
  }
  else if (strcmp (msg, "disk.clean") == 0) {
    if (strcmp (val, "") == 0) {
      if (dsks_commit (pc->dsk, 0)) {
        pce_log (MSG_ERR, "clean failed for at least one disk\n");
        return (1);
      }
    }
    else {
      unsigned d;

      d = strtoul (val, NULL, 0);

      if (dsks_commit_disk (pc->dsk, d, 0)) {
        pce_log (MSG_ERR, "clean failed (%s)\n", val);
        return (1);
      }
    }

    return (0);
  }

  pce_log (MSG_INF, "unhandled message (\"%s\", \"%s\")\n", msg, val);

  return (1);
}

static
int pc_copy_msg (char *dst, const char *src, unsigned max)
{
  dst[0] = 0;

  if ((strlen (src) + 1) > max) {
    return (1);
  }

  strcpy (dst, src);

  return (0);
}

int pc_get_msg (ibmpc_t *pc, const char *msg, char *val, unsigned max)
{
  const char    *str;
  char          buf[256];
  unsigned long tmp;

  if ((msg == NULL) || (val == NULL)) {
    return (1);
  }

  if (strcmp (msg, "emu.config.get_string") == 0) {
    str = ini_get_str_def (pc->cfg, val, "");
    return (pc_copy_msg (val, str, max));
  }
  else if (strcmp (msg, "pit.realtime") == 0) {
    sprintf (buf, "%d", (pc->pit_real != 0));
    return (pc_copy_msg (val, buf, max));
  }

  if (pc_get_msgul (pc, msg, &tmp) == 0) {
    sprintf (buf, "%lu", tmp);
    return (pc_copy_msg (val, buf, max));
  }

  return (1);
}

int pc_set_msgul (ibmpc_t *pc, const char *msg, unsigned long val)
{
  char buf[256];

  sprintf (buf, "%lu", val);

  return (pc_set_msg (pc, msg, buf));
}

int pc_get_msgul (ibmpc_t *pc, const char *msg, unsigned long *val)
{
  if ((msg == NULL) || (val == NULL)) {
    return (1);
  }

  if (strcmp (msg, "emu.idle") == 0) {
    *val = par_int28;
    return (0);
  }
  else if (strcmp (msg, "emu.pause") == 0) {
    *val = (pc->pause != 0);
    return (0);
  }
  else if (strcmp (msg, "pit.realtime") == 0) {
    *val = (pc->pit_real != 0);
    return (0);
  }

  return (1);
}
