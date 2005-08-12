/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/lib/iniram.c                                           *
 * Created:       2005-07-24 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2005-08-12 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2005 Hampa Hug <hampa@hampa.ch>                        *
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


#include <lib/iniram.h>
#include <lib/load.h>
#include <lib/log.h>


void ini_get_ram (memory_t *mem, ini_sct_t *ini, mem_blk_t **addr0)
{
  ini_sct_t     *sct;
  mem_blk_t     *ram;
  const char    *fname;
  unsigned long base, size;

  if (addr0 != NULL) {
    *addr0 = NULL;
  }

  sct = ini_sct_find_sct (ini, "ram");

  while (sct != NULL) {
    fname = ini_get_str (sct, "file");
    base = ini_get_lng_def (sct, "base", 0);
    size = ini_get_lng_def (sct, "size", 65536L);

    pce_log (MSG_INF, "RAM:\tbase=0x%08lx size=%lu file=%s\n",
      base, size, (fname == NULL) ? "<none>" : fname
    );

    ram = mem_blk_new (base, size, 1);
    mem_blk_clear (ram, 0x00);
    mem_blk_set_readonly (ram, 0);
    mem_add_blk (mem, ram, 1);

    if ((addr0 != NULL) && (base == 0)) {
      *addr0 = ram;
    }

    if (fname != NULL) {
      if (pce_load_blk_bin (ram, fname)) {
        pce_log (MSG_ERR, "*** loading ram failed (%s)\n", fname);
      }
    }

    sct = ini_sct_find_next (sct, "ram");
  }
}

void ini_get_rom (memory_t *mem, ini_sct_t *ini)
{
  ini_sct_t     *sct;
  mem_blk_t     *rom;
  const char    *fname;
  unsigned long base, size;

  sct = ini_sct_find_sct (ini, "rom");

  while (sct != NULL) {
    fname = ini_get_str (sct, "file");
    base = ini_get_lng_def (sct, "base", 0);
    size = ini_get_lng_def (sct, "size", 65536L);

    pce_log (MSG_INF, "ROM:\tbase=0x%08lx size=%lu file=%s\n",
      base, size, (fname != NULL) ? fname : "<none>"
    );

    rom = mem_blk_new (base, size, 1);
    mem_blk_clear (rom, 0x00);
    mem_blk_set_readonly (rom, 1);
    mem_add_blk (mem, rom, 1);

    if (fname != NULL) {
      if (pce_load_blk_bin (rom, fname)) {
        pce_log (MSG_ERR, "*** loading rom failed (%s)\n", fname);
      }
    }

    sct = ini_sct_find_next (sct, "rom");
  }
}
