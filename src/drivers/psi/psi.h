/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi.h                                        *
 * Created:     2010-08-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PSI_PSI_H
#define PSI_PSI_H 1


#define PSI_FLAG_CRC_ID   0x01
#define PSI_FLAG_CRC_DATA 0x02
#define PSI_FLAG_DEL_DAM  0x04
#define PSI_FLAG_NO_DAM   0x08

#define PSI_ENC_MASK      0x0fff
#define PSI_ENC_UNKNOWN   0x0000
#define PSI_ENC_FM        0x0001
#define PSI_ENC_FM_DD     0x0001
#define PSI_ENC_FM_HD     0x8001
#define PSI_ENC_MFM       0x0002
#define PSI_ENC_MFM_DD    0x0002
#define PSI_ENC_MFM_HD    0x8002
#define PSI_ENC_MFM_ED    0x4002
#define PSI_ENC_GCR       0x0003

#define PSI_TAGS_MAX      16


typedef struct psi_sct_t {
	struct psi_sct_t *next;

	unsigned short   c;
	unsigned short   h;
	unsigned short   s;
	unsigned short   n;

	unsigned long    flags;

	unsigned short   encoding;

	unsigned         cur_alt;

	unsigned char    *data;

	unsigned short   tag_cnt;
	unsigned char    tag[PSI_TAGS_MAX];

	unsigned long    position;
	unsigned long    read_time;

	char             have_mfm_size;
	unsigned char    mfm_size;

	char             have_gcr_format;
	unsigned char    gcr_format;
} psi_sct_t;


typedef struct {
	unsigned short h;
	unsigned short sct_cnt;
	psi_sct_t      **sct;
} psi_trk_t;


typedef struct {
	unsigned short c;
	unsigned short trk_cnt;
	psi_trk_t      **trk;
} psi_cyl_t;


typedef struct {
	unsigned short cyl_cnt;
	psi_cyl_t      **cyl;

	unsigned       comment_size;
	unsigned char  *comment;
} psi_img_t;


psi_sct_t *psi_sct_new (unsigned c, unsigned h, unsigned s, unsigned n);

void psi_sct_del (psi_sct_t *sct);

psi_sct_t *psi_sct_clone (const psi_sct_t *sct, int deep);

void psi_sct_add_alternate (psi_sct_t *sct, psi_sct_t *alt);

psi_sct_t *psi_sct_get_alternate (psi_sct_t *sct, unsigned idx);

int psi_sct_set_size (psi_sct_t *sct, unsigned size, unsigned filler);
unsigned psi_sct_get_size (const psi_sct_t *sct);

void psi_sct_fill (psi_sct_t *sct, unsigned val);

int psi_sct_uniform (const psi_sct_t *sct);

void psi_sct_set_flags (psi_sct_t *sct, unsigned long flags, int set);

void psi_sct_set_encoding (psi_sct_t *sct, unsigned enc);

void psi_sct_set_position (psi_sct_t *sct, unsigned long val);
unsigned long psi_sct_get_position (const psi_sct_t *sct);

void psi_sct_set_read_time (psi_sct_t *sct, unsigned long val);
unsigned long psi_sct_get_read_time (const psi_sct_t *sct);

void psi_sct_set_mfm_size (psi_sct_t *sct, unsigned char val);
unsigned psi_sct_get_mfm_size (const psi_sct_t *sct);

void psi_sct_set_gcr_format (psi_sct_t *sct, unsigned char val);
unsigned psi_sct_get_gcr_format (const psi_sct_t *sct);

unsigned psi_sct_set_tags (psi_sct_t *sct, const void *buf, unsigned cnt);

unsigned psi_sct_get_tags (const psi_sct_t *sct, void *buf, unsigned cnt);


psi_trk_t *psi_trk_new (unsigned h);

void psi_trk_free (psi_trk_t *trk);

void psi_trk_del (psi_trk_t *trk);

int psi_trk_add_sector (psi_trk_t *trk, psi_sct_t *sct);

psi_sct_t *psi_trk_get_indexed_sector (psi_trk_t *trk, unsigned idx, int phy);

int psi_trk_interleave (psi_trk_t *trk, unsigned il);


psi_cyl_t *psi_cyl_new (unsigned c);

void psi_cyl_free (psi_cyl_t *cyl);

void psi_cyl_del (psi_cyl_t *cyl);

int psi_cyl_add_track (psi_cyl_t *cyl, psi_trk_t *trk);

psi_trk_t *psi_cyl_get_track (psi_cyl_t *cyl, unsigned h, int alloc);


psi_img_t *psi_img_new (void);

void psi_img_free (psi_img_t *img);

void psi_img_del (psi_img_t *img);

void psi_img_erase (psi_img_t *img);

int psi_img_add_cylinder (psi_img_t *img, psi_cyl_t *cyl);

int psi_img_add_track (psi_img_t *img, psi_trk_t *trk, unsigned c);

int psi_img_add_sector (psi_img_t *img, psi_sct_t *sct, unsigned c, unsigned h);

void psi_img_remove_sector (psi_img_t *img, const psi_sct_t *sct);

psi_cyl_t *psi_img_get_cylinder (psi_img_t *img, unsigned c, int alloc);

psi_trk_t *psi_img_get_track (psi_img_t *img, unsigned c, unsigned h, int alloc);

psi_sct_t *psi_img_get_sector (psi_img_t *img, unsigned c, unsigned h, unsigned s, int phy);

int psi_img_map_sector (psi_img_t *img, unsigned long idx, unsigned *pc, unsigned *ph, unsigned *ps);

int psi_img_add_comment (psi_img_t *img, const unsigned char *buf, unsigned cnt);

int psi_img_set_comment (psi_img_t *img, const unsigned char *buf, unsigned cnt);

void psi_img_clean_comment (psi_img_t *img);

unsigned long psi_img_get_sector_count (const psi_img_t *img);


#endif
