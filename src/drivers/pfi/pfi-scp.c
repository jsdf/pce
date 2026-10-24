/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/pfi-scp.c                                    *
 * Created:     2014-01-30 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2014-2019 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pfi.h"
#include "pfi-io.h"
#include "pfi-scp.h"


#define SCP_CLOCK 40000000


typedef struct {
	FILE          *fp;

	unsigned char disk_type;
	unsigned char heads;
	unsigned char cylinders;

	unsigned      revolutions;

	unsigned long checksum;
	unsigned long offset;
} scp_save_t;


static
int scp_calc_checksum (FILE *fp, unsigned long *cksum)
{
	unsigned      i, n;
	unsigned long val;
	unsigned char buf[256];

	if (pfi_set_pos (fp, 0x10)) {
		return (1);
	}

	val = 0;

	while ((n = fread (buf, 1, 256, fp)) > 0) {
		for (i = 0; i < n; i++) {
			val += buf[i];
		}
	}

	*cksum = val & 0xffffffff;

	return (0);
}

static
int scp_load_track (FILE *fp, pfi_img_t *img, unsigned c, unsigned h, unsigned revs)
{
	unsigned      i, t;
	unsigned      val;
	unsigned long j;
	unsigned long ofs;
	unsigned long dcnt, dofs, idx, oflow;
	unsigned char buf[16];
	pfi_trk_t     *trk;

	t = (c << 1) | (h & 1);

	if (pfi_read_ofs (fp, 0x10 + 4 * t, buf, 4)) {
		return (1);
	}

	ofs = pfi_get_uint32_le (buf, 0);

	if (ofs == 0) {
		return (0);
	}

	if (pfi_read_ofs (fp, ofs, buf, 4)) {
		return (1);
	}

	if ((buf[0] != 'T') || (buf[1] != 'R') || (buf[2] != 'K')) {
		return (1);
	}

	if (buf[3] != t) {
		fprintf (stderr, "scp: bad track in track header (%u/%u %u)\n",
			c, h, buf[3]
		);
		/* return (1); */
	}

	if ((trk = pfi_img_get_track (img, c, h, 1)) == NULL) {
		return (1);
	}

	pfi_trk_set_clock (trk, SCP_CLOCK);

	idx = 0;

	if (pfi_trk_add_index (trk, idx)) {
		return (1);
	}

	oflow = 0;

	for (i = 0; i < revs; i++) {
		if (pfi_read_ofs (fp, ofs + 4 + 12 * i, buf, 12)) {
			return (1);
		}

		idx += pfi_get_uint32_le (buf, 0);
		dcnt = pfi_get_uint32_le (buf, 4);
		dofs = pfi_get_uint32_le (buf, 8);

		if (pfi_trk_add_index (trk, idx)) {
			return (1);
		}

		if (pfi_set_pos (fp, ofs + dofs)) {
			return (1);
		}

		for (j = 0; j < dcnt; j++) {
			if (pfi_read (fp, buf, 2)) {
				return (1);
			}

			val = pfi_get_uint16_be (buf, 0);

			if (val == 0) {
				oflow += 65536;
				continue;
			}

			if (pfi_trk_add_pulse (trk, oflow + val)) {
				return (1);
			}

			oflow = 0;
		}
	}

	pfi_trk_clean (trk);

	return (0);
}

static
int scp_load_image (FILE *fp, pfi_img_t *img)
{
	unsigned      t, t1, t2;
	unsigned long ck1, ck2;
	unsigned char buf[16];

	if (pfi_read_ofs (fp, 0, buf, 16)) {
		return (1);
	}

	if ((buf[0] != 'S') || (buf[1] != 'C') || (buf[2] != 'P')) {
		return (1);
	}

	if ((buf[9] != 0) && (buf[9] != 16)) {
		return (1);
	}

	if (scp_calc_checksum (fp, &ck1)) {
		return (1);
	}

	ck2 = pfi_get_uint32_le (buf, 12);

	if (ck1 != ck2) {
		fprintf (stderr, "scp: bad checksum\n");
		/* return (1); */
	}

	t1 = buf[6];
	t2 = buf[7];

	for (t = t1; t <= t2; t++) {
		if (scp_load_track (fp, img, t >> 1, t & 1, buf[5])) {
			return (1);
		}
	}

	return (0);
}

pfi_img_t *pfi_load_scp (FILE *fp)
{
	pfi_img_t *img;

	if ((img = pfi_img_new()) == NULL) {
		return (NULL);
	}

	if (scp_load_image (fp, img)) {
		pfi_img_del (img);
		return (NULL);
	}

	return (img);
}


static
void scp_get_disk_type (scp_save_t *scp, pfi_img_t *img)
{
	unsigned        i;
	unsigned        rev;
	const pfi_trk_t *trk;

	scp->disk_type = 0x40;
	scp->revolutions = 0;
	scp->heads = 0;
	scp->cylinders = (img->cyl_cnt < 84) ? img->cyl_cnt : 84;

	for (i = 0; i < 168; i++) {
		trk = pfi_img_get_track (img, i >> 1, i & 1, 0);

		if ((trk == NULL) || (trk->index_cnt < 2)) {
			continue;
		}

		scp->heads |= 1 << (i & 1);

		rev = trk->index_cnt - 1;

		if ((scp->revolutions == 0) || (rev < scp->revolutions)) {
			scp->revolutions = rev;
		}
	}

	scp->heads = 3;
}

static
int scp_save_header (scp_save_t *scp)
{
	unsigned char flags;
	unsigned char buf[16];

	flags = 0x00;

	if (scp->cylinders >= 60) {
		flags |= 0x02;
	}

	buf[0] = 'S';
	buf[1] = 'C';
	buf[2] = 'P';
	buf[3] = 0x15;	/* version */
	buf[4] = scp->disk_type;
	buf[5] = scp->revolutions;
	buf[6] = 0;
	buf[7] = 2 * scp->cylinders - 1;
	buf[8] = flags;
	buf[9] = 0x00;
	buf[10] = (scp->heads == 3) ? 0 : scp->heads;
	buf[11] = 0;

	pfi_set_uint32_le (buf, 12, scp->checksum);

	if (pfi_write_ofs (scp->fp, 0, buf, 16)) {
		return (1);
	}

	return (0);
}

static
int scp_write (scp_save_t *scp, const void *buf, unsigned long cnt)
{
	unsigned long       i;
	const unsigned char *p = buf;

	for (i = 0; i < cnt; i++) {
		scp->checksum += p[i];
	}

	return (pfi_write (scp->fp, buf, cnt));
}

static
int scp_write_ofs (scp_save_t *scp, unsigned long ofs, const void *buf, unsigned long cnt)
{
	unsigned long       i;
	const unsigned char *p = buf;

	for (i = 0; i < cnt; i++) {
		scp->checksum += p[i];
	}

	return (pfi_write_ofs (scp->fp, ofs, buf, cnt));
}

static
int scp_write_track_header (scp_save_t *scp, unsigned c, unsigned h)
{
	unsigned char buf[4];

	buf[0] = 'T';
	buf[1] = 'R';
	buf[2] = 'K';
	buf[3] = 2 * c + h;

	if (scp_write_ofs (scp, scp->offset, buf, 4)) {
		return (1);
	}

	return (0);
}

static
int scp_write_timestamp (scp_save_t *scp, unsigned long ofs)
{
	time_t    t;
	struct tm *tm;
	char      buf[128];

	time (&t);
	tm = gmtime (&t);

	sprintf (buf, "%04d-%02d-%02d %02d:%02d:%02d UTC",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec
	);

	if (scp_write_ofs (scp, ofs, buf, strlen (buf))) {
		return (1);
	}

	return (0);
}

static
int scp_write_rev_header (scp_save_t *scp, unsigned long hofs, unsigned long clock, unsigned long pcnt, unsigned long dofs)
{
	unsigned char buf[12];

	pfi_set_uint32_le (buf, 0, clock);
	pfi_set_uint32_le (buf, 4, pcnt);
	pfi_set_uint32_le (buf, 8, dofs);

	if (scp_write_ofs (scp, hofs, buf, 12)) {
		return (1);
	}

	return (0);
}

static
int scp_save_track (scp_save_t *scp, pfi_trk_t *trk, unsigned c, unsigned h)
{
	unsigned           index;
	unsigned long      head_ofs, curr_ofs, flux_ofs;
	uint32_t           pulse, idx;
	unsigned long      rem;
	unsigned long      pcnt, clock;
	unsigned long long tmp;
	unsigned char      buf[2];

	if (scp_write_track_header (scp, c, h)) {
		return (1);
	}

	head_ofs = scp->offset + 4;
	curr_ofs = scp->offset + 4 + 12 * scp->revolutions;
	flux_ofs = curr_ofs;

	if (pfi_set_pos (scp->fp, curr_ofs)) {
		return (1);
	}

	pfi_trk_rewind (trk);

	index = 0;
	pcnt = 0;
	rem = 0;

	while (pfi_trk_get_pulse (trk, &pulse, &idx) == 0) {
		if ((pulse == 0) || (idx < pulse)) {
			if (index > 0) {
				clock = trk->index[index] - trk->index[index - 1];
				clock = ((unsigned long long) SCP_CLOCK * clock + trk->clock / 2) / trk->clock;

				if (scp_write_rev_header (scp, head_ofs, clock, pcnt, flux_ofs - scp->offset)) {
					return (1);
				}

				if (pfi_set_pos (scp->fp, curr_ofs)) {
					return (1);
				}

				head_ofs += 12;
			}

			index += 1;

			if (index > scp->revolutions) {
				break;
			}

			pcnt = 0;
			flux_ofs = curr_ofs;
		}

		if ((index < 1) || (pulse == 0)) {
			continue;
		}

		tmp = (unsigned long long) SCP_CLOCK * pulse + rem;
		rem = tmp % trk->clock;
		pulse = tmp / trk->clock;

		if (pulse == 65536) {
			pulse = 65535;
		}

		while (pulse > 65535) {
			pfi_set_uint16_be (buf, 0, 0);

			if (scp_write (scp, buf, 2)) {
				return (1);
			}

			curr_ofs += 2;
			pulse -= 65536;
			pcnt += 1;
		}

		pfi_set_uint16_be (buf, 0, pulse);

		if (scp_write (scp, buf, 2)) {
			return (1);
		}

		curr_ofs += 2;
		pcnt += 1;
	}

	scp->offset = curr_ofs;

	return (0);
}

int pfi_save_scp (FILE *fp, pfi_img_t *img)
{
	unsigned      i;
	unsigned char buf[4 * 168];
	pfi_trk_t     *trk;
	scp_save_t    scp;

	scp.fp = fp;
	scp.offset = 16 + 168 * 4;
	scp.checksum = 0;

	scp_get_disk_type (&scp, img);

	if (scp.revolutions == 0) {
		return (1);
	}

	for (i = 0; i < 168; i++) {
		trk = pfi_img_get_track (img, i >> 1, i & 1, 0);

		if (trk == NULL) {
			pfi_set_uint32_le (buf, 4 * i, 0);
			continue;
		}

		pfi_set_uint32_le (buf, 4 * i, scp.offset);

		if (scp_save_track (&scp, trk, i >> 1, i & 1)) {
			return (1);
		}
	}

	if (scp_write_timestamp (&scp, scp.offset)) {
		return (1);
	}

	if (scp_write_ofs (&scp, 16, buf, 4 * 168)) {
		return (1);
	}

	if (scp_save_header (&scp)) {
		return (1);
	}

	return (0);
}


int pfi_probe_scp_fp (FILE *fp)
{
	unsigned char buf[4];

	if (pfi_read_ofs (fp, 0, buf, 4)) {
		return (0);
	}

	if ((buf[0] != 'S') || (buf[1] != 'C') || (buf[2] != 'P')) {
		return (0);
	}

	return (1);
}

int pfi_probe_scp (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = pfi_probe_scp_fp (fp);

	fclose (fp);

	return (r);
}
