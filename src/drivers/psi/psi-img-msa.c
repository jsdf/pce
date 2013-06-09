/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi-img-msa.c                                *
 * Created:     2013-06-03 by Hampa Hug <hampa@hampa.ch>                     *
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
#include "psi-img-msa.h"


#define MSA_MAGIC 0x0e0f


/*
 * Read and decode a track of decoded size cnt.
 */
static
int msa_read_track (FILE *fp, unsigned char *buf, unsigned cnt)
{
	int      c, n1, n2;
	unsigned fcnt, rcnt;

	if (psi_read (fp, buf, 2)) {
		return (1);
	}

	fcnt = psi_get_uint16_be (buf, 0);

	if (fcnt == cnt) {
		if (psi_read (fp, buf, cnt)) {
			return (1);
		}

		return (0);
	}

	while (cnt > 0) {
		if (fcnt < 1) {
			return (1);
		}

		if ((c = fgetc (fp)) == EOF) {
			return (1);
		}

		fcnt -= 1;

		if (c == 0xe5) {
			if (fcnt < 3) {
				return (1);
			}

			c = fgetc (fp);
			n1 = fgetc (fp);
			n2 = fgetc (fp);

			if ((c == EOF) || (n1 == EOF) || (n2 == EOF)) {
				return (1);
			}

			rcnt = ((unsigned) (n1 & 0xff) << 8) | (n2 & 0xff);

			while ((cnt > 0) && (rcnt > 0)) {
				*(buf++) = c;
				cnt -= 1;
				rcnt -= 1;
			}

			fcnt -= 3;
		}
		else {
			*(buf++) = c;
			cnt -= 1;
		}
	}

	return (0);
}

static
int msa_load_fp (FILE *fp, psi_img_t *img)
{
	unsigned      c, h, s, hn, sn, c0, c1;
	unsigned long tcnt;
	unsigned char *tbuf;
	psi_trk_t    *trk;
	psi_sct_t    *sct;
	unsigned char buf[10];

	if (psi_read_ofs (fp, 0, buf, 10)) {
		return (0);
	}

	if (psi_get_uint16_be (buf, 0) != MSA_MAGIC) {
		return (0);
	}

	sn = psi_get_uint16_be (buf, 2);
	hn = psi_get_uint16_be (buf, 4) + 1;
	c0 = psi_get_uint16_be (buf, 6);
	c1 = psi_get_uint16_be (buf, 8);

	if ((sn < 1) || (sn > 127)) {
		return (1);
	}

	if ((hn > 16) || (c1 < c0)) {
		return (1);
	}

	tcnt = 512UL * sn;

	if ((tbuf = malloc (tcnt)) == NULL) {
		return (1);
	}

	for (c = c0; c <= c1; c++) {
		for (h = 0; h < hn; h++) {
			trk = psi_img_get_track (img, c, h, 1);

			if (trk == NULL) {
				free (tbuf);
				return (1);
			}

			if (msa_read_track (fp, tbuf, tcnt)) {
				free (tbuf);
				return (1);
			}

			for (s = 0; s < sn; s++) {
				sct = psi_sct_new (c, h, s + 1, 512);

				if (sct == NULL) {
					free (tbuf);
					return (1);
				}

				psi_sct_set_encoding (sct, PSI_ENC_MFM_DD);

				if (psi_trk_add_sector (trk, sct)) {
					psi_sct_del (sct);
					return (1);
				}

				memcpy (sct->data, tbuf + 512UL * s, 512);
			}
		}
	}

	free (tbuf);

	return (0);
}

psi_img_t *psi_load_msa (FILE *fp)
{
	psi_img_t *img;

	if ((img = psi_img_new()) == NULL) {
		return (NULL);
	}

	if (msa_load_fp (fp, img)) {
		psi_img_del (img);
		return (NULL);
	}

	return (img);
}


int psi_save_msa (FILE *fp, const psi_img_t *img)
{
	unsigned         c, h, s;
	const psi_cyl_t *cyl;
	const psi_trk_t *trk;
	const psi_sct_t *sct;
	unsigned char    buf[10];

	if ((cyl = psi_img_get_cylinder ((psi_img_t *) img, 0, 0)) == NULL) {
		return (1);
	}

	if ((trk = psi_img_get_track ((psi_img_t *) img, 0, 0, 0)) == NULL) {
		return (1);
	}

	psi_set_uint16_be (buf, 0, MSA_MAGIC);
	psi_set_uint16_be (buf, 2, trk->sct_cnt);
	psi_set_uint16_be (buf, 4, cyl->trk_cnt - 1);
	psi_set_uint16_be (buf, 6, 0);
	psi_set_uint16_be (buf, 8, img->cyl_cnt - 1);

	if (psi_write (fp, buf, 10)) {
		return (1);
	}

	psi_set_uint16_be (buf, 0, 512U * trk->sct_cnt);

	for (c = 0; c < img->cyl_cnt; c++) {
		for (h = 0; h < cyl->trk_cnt; h++) {
			if (psi_write (fp, buf, 2)) {
				return (1);
			}

			for (s = 0; s < trk->sct_cnt; s++) {
				sct = psi_img_get_sector ((psi_img_t *) img, c, h, s + 1, 0);

				if (sct == NULL) {
					return (1);
				}

				if (sct->n != 512) {
					return (1);
				}

				if (psi_write (fp, sct->data, 512)) {
					return (1);
				}
			}
		}
	}

	fflush (fp);

	return (0);
}

int psi_probe_msa_fp (FILE *fp)
{
	unsigned char buf[10];

	if (psi_read_ofs (fp, 0, buf, 10)) {
		return (0);
	}

	if (psi_get_uint16_be (buf, 0) != MSA_MAGIC) {
		return (0);
	}

	return (1);
}

int psi_probe_msa (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = psi_probe_msa_fp (fp);

	fclose (fp);

	return (r);
}
