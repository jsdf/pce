/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/pfdc.h                                     *
 * Created:     2010-08-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PFDC_PFDC_H
#define PFDC_PFDC_H 1


#define PFDC_MAGIC_PFDC    0x50464443

#define PFDC_FLAG_CRC_ID   0x04
#define PFDC_FLAG_CRC_DATA 0x08
#define PFDC_FLAG_DEL_DAM  0x10

#define PFDC_ENC_UNKNOWN   0x00
#define PFDC_ENC_FM        0x01
#define PFDC_ENC_MFM       0x02
#define PFDC_ENC_GCR       0x03

#define PFDC_TAGS_MAX      16


typedef struct pfdc_sct_t {
	struct pfdc_sct_t *next;

	unsigned short     c;
	unsigned short     h;
	unsigned short     s;
	unsigned short     n;

	unsigned long      flags;

	unsigned short     encoding;
	unsigned long      data_rate;

	unsigned           cur_alt;

	unsigned char      *data;

	unsigned short     tag_cnt;
	unsigned char      tag[PFDC_TAGS_MAX];
} pfdc_sct_t;


typedef struct {
	unsigned short h;
	unsigned short sct_cnt;
	pfdc_sct_t     **sct;
} pfdc_trk_t;


typedef struct {
	unsigned short c;
	unsigned short trk_cnt;
	pfdc_trk_t     **trk;
} pfdc_cyl_t;


typedef struct {
	unsigned short cyl_cnt;
	pfdc_cyl_t     **cyl;

	unsigned       comment_size;
	unsigned char  *comment;
} pfdc_img_t;


unsigned long pfdc_get_uint32_be (const void *buf, unsigned idx);
unsigned pfdc_get_uint16_be (const void *buf, unsigned idx);
void pfdc_set_uint16_be (void *buf, unsigned idx, unsigned val);
void pfdc_set_uint32_be (void *buf, unsigned idx, unsigned long val);

unsigned pfdc_get_uint16_le (const void *buf, unsigned idx);
void pfdc_set_uint16_le (void *buf, unsigned idx, unsigned val);


pfdc_sct_t *pfdc_sct_new (unsigned c, unsigned h, unsigned s, unsigned n);

void pfdc_sct_del (pfdc_sct_t *sct);

pfdc_sct_t *pfdc_sct_clone (const pfdc_sct_t *sct, int deep);

void pfdc_sct_add_alternate (pfdc_sct_t *sct, pfdc_sct_t *alt);

pfdc_sct_t *pfdc_sct_get_alternate (pfdc_sct_t *sct, unsigned idx);

int pfdc_sct_set_size (pfdc_sct_t *sct, unsigned size, unsigned filler);

void pfdc_sct_fill (pfdc_sct_t *sct, unsigned val);

int pfdc_sct_uniform (const pfdc_sct_t *sct);

void pfdc_sct_set_flags (pfdc_sct_t *sct, unsigned long flags, int set);

void pfdc_sct_set_encoding (pfdc_sct_t *sct, unsigned enc, unsigned long rate);

unsigned pfdc_sct_set_tags (pfdc_sct_t *sct, const void *buf, unsigned cnt);

unsigned pfdc_sct_get_tags (const pfdc_sct_t *sct, void *buf, unsigned cnt);


pfdc_trk_t *pfdc_trk_new (unsigned h);

void pfdc_trk_free (pfdc_trk_t *trk);

void pfdc_trk_del (pfdc_trk_t *trk);

int pfdc_trk_add_sector (pfdc_trk_t *trk, pfdc_sct_t *sct);

pfdc_sct_t *pfdc_trk_get_indexed_sector (pfdc_trk_t *trk, unsigned idx, int phy);


pfdc_cyl_t *pfdc_cyl_new (unsigned c);

void pfdc_cyl_free (pfdc_cyl_t *cyl);

void pfdc_cyl_del (pfdc_cyl_t *cyl);

int pfdc_cyl_add_track (pfdc_cyl_t *cyl, pfdc_trk_t *trk);

pfdc_trk_t *pfdc_cyl_get_track (pfdc_cyl_t *cyl, unsigned h, int alloc);


pfdc_img_t *pfdc_img_new (void);

void pfdc_img_free (pfdc_img_t *img);

void pfdc_img_del (pfdc_img_t *img);

void pfdc_img_erase (pfdc_img_t *img);

int pfdc_img_add_cylinder (pfdc_img_t *img, pfdc_cyl_t *cyl);

int pfdc_img_add_sector (pfdc_img_t *img, pfdc_sct_t *sct, unsigned c, unsigned h);

void pfdc_img_remove_sector (pfdc_img_t *img, const pfdc_sct_t *sct);

pfdc_cyl_t *pfdc_img_get_cylinder (pfdc_img_t *img, unsigned c, int alloc);

pfdc_trk_t *pfdc_img_get_track (pfdc_img_t *img, unsigned c, unsigned h, int alloc);

pfdc_sct_t *pfdc_img_get_sector (pfdc_img_t *img, unsigned c, unsigned h, unsigned s, int phy);

int pfdc_img_map_sector (pfdc_img_t *img, unsigned long idx, unsigned *pc, unsigned *ph, unsigned *ps);

int pfdc_img_add_comment (pfdc_img_t *img, const unsigned char *buf, unsigned cnt);

int pfdc_img_set_comment (pfdc_img_t *img, const unsigned char *buf, unsigned cnt);

unsigned long pfdc_img_get_sector_count (const pfdc_img_t *img);


#endif
