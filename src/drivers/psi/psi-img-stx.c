/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi-img-stx.c                                *
 * Created:     2013-06-06 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013 Hampa Hug <hampa@hampa.ch>                          *
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "psi.h"
#include "psi-io.h"
#include "psi-img-stx.h"


#define STX_MAGIC 0x52535900


static
unsigned mfm_crc (unsigned crc, const void *buf, unsigned cnt)
{
	unsigned            i;
	const unsigned char *src;

	src = buf;

	while (cnt > 0) {
		crc ^= (unsigned) *src << 8;

		for (i = 0; i < 8; i++) {
			if (crc & 0x8000) {
				crc = (crc << 1) ^ 0x1021;
			}
			else {
				crc = crc << 1;
			}
		}

		src += 1;
		cnt -= 1;
	}

	return (crc & 0xffff);
}

static
int stx_save_track_image (FILE *fp, unsigned long ofs, unsigned flag, unsigned c, unsigned h)
{
	FILE          *dst;
	unsigned      cnt;
	unsigned char buf[2];
	char          name[256];

	if ((flag & 0x40) == 0) {
		/* no track image */
		return (0);
	}

	if (flag & 0x80) {
		/* track image contains sync offset */
		ofs += 2;
	}

	if (psi_read_ofs (fp, ofs, buf, 2)) {
		return (1);
	}

	cnt = psi_get_uint16_le (buf, 0);

	sprintf (name, "track-%03u-%u.dat", c, h);

	if ((dst = fopen (name, "wb")) == NULL) {
		return (1);
	}

	while (cnt > 0) {
		c = fgetc (fp);
		fputc (c, dst);
		cnt -= 1;
	}

	fclose (dst);

	return (0);
}

static
int stx_load_sector (FILE *fp, psi_trk_t *trk, unsigned long hpos, unsigned long dpos)
{
	unsigned      c, h, s, n;
	unsigned      size, spos, status, time;
	unsigned      crc1, crc2;
	unsigned long dofs;
	unsigned char buf[16];
	psi_sct_t     *sct;

	if (psi_read_ofs (fp, hpos, buf, 16)) {
		return (1);
	}

	dofs = psi_get_uint32_le (buf, 0);
	spos = psi_get_uint16_le (buf, 4);
	time = psi_get_uint16_le (buf, 6);
	c = buf[8];
	h = buf[9];
	s = buf[10];
	n = buf[11];
	crc1 = psi_get_uint16_be (buf, 12);
	status = buf[14];

	crc2 = mfm_crc (0xffff, "\xa1\xa1\xa1\xfe", 4);
	crc2 = mfm_crc (crc2, buf + 8, 4);

	size = 128U << (n & 3);

	if (0) {
		if (status != 0) {
			fprintf (stderr,
				"S: [%02X %02X %02X %02X] S=%02X  T=%u  P=%u\n",
				c, h, s, n, status, time, spos
			);
		}
	}

	if ((sct = psi_sct_new (c, h, s, size)) == NULL) {
		return (1);
	}

	psi_trk_add_sector (trk, sct);

	psi_sct_set_position (sct, spos);
	psi_sct_set_mfm_size (sct, n);
	psi_sct_set_read_time (sct, (time + 2) / 4);

	if (status & 0x08) {
		/* data crc error */
		psi_sct_set_flags (sct, PSI_FLAG_CRC_DATA, 1);
	}

	if (crc1 != crc2) {
		psi_sct_set_flags (sct, PSI_FLAG_CRC_ID, 1);
	}

	if (status & 0x10) {
		/* record not found */
		psi_sct_set_size (sct, 0, 0);
		psi_sct_set_flags (sct, PSI_FLAG_NO_DAM, 1);
		psi_sct_set_read_time (sct, 0);
	}
	else {
		if (psi_read_ofs (fp, dpos + dofs, sct->data, size)) {
			return (1);
		}
	}

	return (0);
}

static
int stx_load_track (FILE *fp, psi_img_t *img, unsigned long *ofs)
{
	unsigned      i, c, h;
	unsigned long cnt, fuz, dpos;
	unsigned      scnt, flag;
	unsigned char buf[16];
	psi_trk_t    *trk;

	if (psi_read_ofs (fp, *ofs, buf, 16)) {
		return (1);
	}

	cnt = psi_get_uint32_le (buf, 0);
	fuz = psi_get_uint32_le (buf, 4);
	scnt = psi_get_uint16_le (buf, 8);
	flag = psi_get_uint16_le (buf, 10);
	c = buf[14] & 0x7f;
	h = (buf[14] >> 7) & 1;

	if (fuz != 0) {
		fprintf (stderr, "stx: fuz != 0\n");
	}

	dpos = *ofs + 16 + 16 * scnt;

	if ((trk = psi_img_get_track (img, c, h, 1)) == NULL) {
		return (1);
	}

	for (i = 0; i < scnt; i++) {
		if (stx_load_sector (fp, trk, *ofs + 16 + 16 * i, dpos)) {
			return (1);
		}
	}

	if (0) {
		if (stx_save_track_image (fp, *ofs + 16 + 16 * scnt, flag, c, h)) {
			return (1);
		}
	}

	*ofs += cnt;

	return (0);
}

static
int stx_load_fp (FILE *fp, psi_img_t *img)
{
	unsigned      i, n;
	unsigned long ofs;
	unsigned char buf[16];

	if (psi_read_ofs (fp, 0, buf, 16)) {
		return (1);
	}

	if (psi_get_uint32_be (buf, 0) != STX_MAGIC) {
		return (1);
	}

	n = buf[10];

	ofs = 16;

	for (i = 0; i < n; i++) {
		if (stx_load_track (fp, img, &ofs)) {
			return (1);
		}
	}

	return (0);
}

psi_img_t *psi_load_stx (FILE *fp)
{
	psi_img_t *img;

	if ((img = psi_img_new()) == NULL) {
		return (NULL);
	}

	if (stx_load_fp (fp, img)) {
		psi_img_del (img);
		return (NULL);
	}

	return (img);
}


int psi_save_stx (FILE *fp, const psi_img_t *img)
{
	return (1);
}

int psi_probe_stx_fp (FILE *fp)
{
	unsigned char buf[16];

	if (psi_read_ofs (fp, 0, buf, 8)) {
		return (0);
	}

	if (psi_get_uint32_be (buf, 0) != STX_MAGIC) {
		return (0);
	}

	return (1);
}

int psi_probe_stx (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = psi_probe_stx_fp (fp);

	fclose (fp);

	return (r);
}
