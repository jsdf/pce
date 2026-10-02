/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/text.h                                         *
 * Created:     2017-10-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2017-2024 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PRI_TEXT_H
#define PRI_TEXT_H 1


#include <config.h>

#include <stdio.h>

#include <drivers/pri/pri.h>
#include <lib/text.h>


typedef struct {
	unsigned long pos;
	char          wrap;
	pri_evt_t     *evt;
} pri_text_pos_t;


typedef struct {
	pce_text_t     txt;

	pri_img_t      *img;
	pri_trk_t      *trk;
	unsigned long  c;
	unsigned long  h;
	unsigned       s;

	unsigned long  track_size;

	unsigned long  bit_cnt;
	unsigned long  bit_max;

	unsigned long  shift;
	unsigned       shift_cnt;

	char           free_track;

	unsigned char  last_val;

	unsigned short encoding;

	unsigned long  index_position;
	unsigned long  rotate;

	unsigned long  offset;

	unsigned       column;
	char           need_nl;

	unsigned       mac_gap_size;
	char           mac_check_active;
	unsigned       mac_check_cnt;
	unsigned char  mac_check_buf[3];
	unsigned short mac_check_chk[3];
	unsigned       mac_align;
	char           mac_no_slip;
	char           mac_nibble;

	unsigned short crc;
} pri_text_t;


void txt_save_pos (const pri_text_t *ctx, pri_text_pos_t *pos);
void txt_restore_pos (pri_text_t *ctx, const pri_text_pos_t *pos);

int txt_dec_match (pri_text_t *ctx, const void *buf, unsigned cnt);

void txt_dec_bits (pri_text_t *ctx, unsigned cnt);
void txt_dec_event (pri_text_t *ctx, unsigned long type, unsigned long val);

int txt_enc_bits_raw (pri_text_t *ctx, unsigned long val, unsigned cnt);

int txt_raw_dec_track (pri_text_t *ctx);
int txt_fm_dec_track (pri_text_t *ctx);
int txt_mfm_dec_track (pri_text_t *ctx);
int txt_mac_dec_track (pri_text_t *ctx);

int txt_encode_pri0_fm (pri_text_t *ctx);
int txt_encode_pri0_mfm (pri_text_t *ctx);
int txt_encode_pri0_mac (pri_text_t *ctx);
int txt_encode_pri0_raw (pri_text_t *ctx);


#endif
