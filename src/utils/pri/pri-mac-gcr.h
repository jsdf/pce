/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/pri-mac-gcr.h                                  *
 * Created:     2021-09-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2021-2022 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PRI_MAC_GCR_H
#define PRI_MAC_GCR_H 1


#include <drivers/pri/pri.h>


#define PRI_MAC_ALIGN_NONE       0
#define PRI_MAC_ALIGN_GAP        1
#define PRI_MAC_ALIGN_GAP_END    2
#define PRI_MAC_ALIGN_SECTOR     3
#define PRI_MAC_ALIGN_SECTOR_END 4
#define PRI_MAC_ALIGN_INDEX      5


int pri_mac_get_nibble (pri_trk_t *trk, unsigned char *val, unsigned *cnt);

int pri_mac_find_first (pri_trk_t *trk, unsigned long *pos);
int pri_mac_find_end_last (pri_trk_t *trk, unsigned long *pos);
int pri_mac_find_id_low (pri_trk_t *trk, unsigned long *pos, unsigned long *end);
int pri_mac_find_gap (pri_trk_t *trk, unsigned long *pos1, unsigned long *pos2);

int pri_mac_align_mode (const char *str, unsigned *mode);
int pri_mac_align_pos (pri_trk_t *trk, unsigned mode, unsigned long *pos);


#endif
