/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/aym/play.c                                         *
 * Created:     2015-05-21 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2015 Hampa Hug <hampa@hampa.ch>                          *
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


#include "main.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <arch/atarist/psg.h>
#include <drivers/sound/sound.h>


static
void aym_delay (st_psg_t *psg, unsigned long long val)
{
	while (val > 255) {
		st_psg_clock (psg, 8 * 256);
		val -= 256;
	}

	while (val > 0) {
		st_psg_clock (psg, 8);
		val -= 1;
	}
}

static
int aym_play_psg (FILE *fp, st_psg_t *psg)
{
	int           c1, c2;
	unsigned char v1, v2;
	unsigned long vers;
	unsigned char buf[8];

	if (fread (buf, 1, 8, fp) != 8) {
		return (1);
	}

	if (aym_get_uint32_be (buf, 0) != AYM_MAGIC) {
		fprintf (stderr, "%s: not an aym file\n", arg0);
		return (1);
	}

	if ((vers = aym_get_uint32_be (buf, 4)) != 0) {
		fprintf (stderr, "%s: unknown aym version (%lu)\n", arg0, vers);
		return (1);
	}

	while (1) {
		c1 = fgetc (fp);
		c2 = fgetc (fp);

		if ((c1 == EOF) || (c2 == EOF)) {
			fprintf (stderr, "%s: premature end of file\n", arg0);
			return (0);
		}

		v1 = c1 & 0xff;
		v2 = c2 & 0xff;

		switch ((v1 >> 4) & 0x0f) {
		case 0:
			st_psg_set_select (psg, v1 & 0x0f);
			st_psg_set_data (psg, v2);
			break;

		case 1:
			aym_delay (psg, ((v1 & 0x0f) << 8) | v2);
			break;

		case 2:
			aym_delay (psg, (unsigned long long) (((v1 & 0x0f) << 8) | v2) << 12);
			break;

		case 3:
			aym_delay (psg, (unsigned long long) (((v1 & 0x0f) << 8) | v2) << 24);
			break;

		case 8:
			printf ("TXT %u: ", v1 & 0x0f);

			while (v2 > 0) {
				if ((c1 = fgetc (fp)) == EOF) {
					return (1);
				}

				fputc (c1, stdout);

				v2 -= 1;
			}
			fputc ('\n', stdout);
			break;

		case 15:
			if ((v1 == 0xff) && (v2 == 0xff)) {
				aym_delay (psg, 250000);
				return (0);
			}
		}
	}

	return (0);
}

static
int aym_play_fp (FILE *fp, const char *snddrv, unsigned long lp, int hp)
{
	int         r;
	sound_drv_t *drv;
	st_psg_t    psg;

	st_psg_init (&psg);

	st_psg_set_srate (&psg, par_srate);

	if (st_psg_set_driver (&psg, snddrv)) {
		fprintf (stderr, "%s: bad sound driver (%s)\n", arg0, snddrv);
		st_psg_free (&psg);
		return (1);
	}

	drv = st_psg_get_driver (&psg);

	snd_set_opts (drv, SND_OPT_NONBLOCK, 0);

	st_psg_set_lowpass (&psg, lp);
	st_psg_set_highpass (&psg, hp);

	st_psg_reset (&psg);

	r = aym_play_psg (fp, &psg);

	st_psg_free (&psg);

	return (r);
}

int aym_play (const char *fname, const char *snddrv, unsigned long lp, int hp)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		fprintf (stderr, "%s: can't open aym file (%s)\n", arg0, fname);
		return (1);
	}

	r = aym_play_fp (fp, snddrv, lp, hp);

	fclose (fp);

	return (r);
}
