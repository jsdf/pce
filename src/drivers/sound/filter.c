/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/sound/filter.c                                   *
 * Created:     2010-08-27 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010 Hampa Hug <hampa@hampa.ch>                          *
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


#include <config.h>

#include <stdint.h>
#include <math.h>

#include <drivers/sound/filter.h>


#define SND_IIR_MUL 8192


void snd_iir2_init (sound_iir2_t *iir)
{
	unsigned i;

	for (i = 0; i < 3; i++) {
		iir->a[i] = 0;
		iir->b[i] = 0;
		iir->x[i] = 0;
		iir->y[i] = 0;
	}

	iir->a[0] = SND_IIR_MUL;
}

void snd_iir2_reset (sound_iir2_t *iir)
{
	iir->x[0] = 0;
	iir->x[1] = 0;
	iir->x[2] = 0;

	iir->y[0] = 0;
	iir->y[1] = 0;
	iir->y[2] = 0;
}

void snd_iir2_set_lowpass (sound_iir2_t *iir, unsigned long freq, unsigned long srate)
{
	double om, b0;

	snd_iir2_init (iir);

	if ((freq == 0) || (srate == 0)) {
		return;
	}

	if ((2 * freq) >= srate) {
		freq = (srate / 2) - 1;
	}

	om = 1.0 / tan ((3.14159265358979312 * freq) / srate);
	b0 = om * (om + sqrt(2.0)) + 1.0;

	iir->a[0] = (long) (SND_IIR_MUL * 1.0 / b0);
	iir->a[1] = (long) (SND_IIR_MUL * 2.0 / b0);
	iir->a[2] = (long) (SND_IIR_MUL * 1.0 / b0);

	iir->b[0] = (long) (SND_IIR_MUL * 1.0);
	iir->b[1] = (long) (SND_IIR_MUL * 2.0 * (1.0 - om * om) / b0);
	iir->b[2] = (long) (SND_IIR_MUL * (om * (om - sqrt(2.0)) + 1.0) / b0);
}

void snd_iir2_filter (sound_iir2_t *iir, uint16_t *dst, const uint16_t *src,
	unsigned cnt, unsigned ofs, int sign)
{
	long     v;
	uint16_t sig;

	sig = sign ? 0x8000 : 0;

	while (cnt > 0) {
		v = *src ^ sig;
		v -= 32768;

		iir->x[2] = iir->x[1];
		iir->x[1] = iir->x[0];
		iir->x[0] = v;

		iir->y[2] = iir->y[1];
		iir->y[1] = iir->y[0];

		iir->y[0] = iir->a[0] * iir->x[0];
		iir->y[0] += iir->a[1] * iir->x[1] + iir->a[2] * iir->x[2];
		iir->y[0] -= iir->b[1] * iir->y[1] + iir->b[2] * iir->y[2];

		iir->y[0] = iir->y[0] / SND_IIR_MUL;

		v = iir->y[0];
		v += 32768;

		if (v < 0) {
			v = 0;
		}
		else if (v > 65535) {
			v = 0xffff;
		}
		else {
			v &= 0xffff;
		}

		*dst = ((uint16_t) v) ^ sig;

		src += ofs;
		dst += ofs;
		cnt -= 1;
	}
}
