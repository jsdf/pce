/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/speaker.c                                        *
 * Created:       2003-08-30 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-08-30 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: speaker.c,v 1.1 2003/12/20 01:01:34 hampa Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "pce.h"


speaker_t *spk_new (ibmpc_t *pc, ini_sct_t *sct)
{
  speaker_t *spk;

  spk = (speaker_t *) malloc (sizeof (speaker_t));
  if (spk == NULL) {
    return (NULL);
  }

  spk->pc = pc;
  spk->ppi_val = 0;
  spk->pit_val = 0;
  spk->cur_val = 0;
  spk->cnt = 0;

  return (spk);
}

void spk_del (speaker_t *spk)
{
  if (spk != NULL) {
    free (spk);
  }
}

void spk_flip (speaker_t *spk)
{
  unsigned long long cnt;

  cnt = spk->pc->clk_cnt - spk->clk_cnt;
  spk->clk_cnt = spk->pc->clk_cnt;

  if (cnt > spk->clk_max) {
    cnt = spk->clk_max;
  }
}

void spk_set_ppi (speaker_t *spk, unsigned char val)
{
  unsigned char newval;

  spk->ppi_val = val;

  if (((spk->ppi_val & 0x02) && spk->pit_val) != spk->cur_val) {
    spk_flip (spk);
  }
}

void spk_set_pit (speaker_t *spk, unsigned char val)
{
  spk->pit_val = val;

  if (((spk->ppi_val & 0x02) && spk->pit_val) != spk->cur_val) {
    spk_flip (spk);
  }
}
