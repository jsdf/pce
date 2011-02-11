/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/sound/sound.h                                    *
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


#ifndef PCE_DRIVERS_SOUND_FILTER_H
#define PCE_DRIVERS_SOUND_FILTER_H 1


#include <stdint.h>


/*!***************************************************************************
 * @short A second order IIR filter
 *****************************************************************************/
typedef struct {
	long a[3];
	long b[3];
	long x[3];
	long y[3];
} sound_iir2_t;


/*!***************************************************************************
 * @short Initialize an IIR2 filter
 *****************************************************************************/
void snd_iir2_init (sound_iir2_t *iir);

/*!***************************************************************************
 * @short Reset an IIR2 filter
 *****************************************************************************/
void snd_iir2_reset (sound_iir2_t *iir);

/*!***************************************************************************
 * @short Initialize a low-pass IIR filter
 * @param freq   The cut-off frequency
 * @param srate  The sample rate
 *****************************************************************************/
void snd_iir2_set_lowpass (sound_iir2_t *iir,
	unsigned long freq, unsigned long srate
);

/*!***************************************************************************
 * @short Filter samples with an IIR2 filter
 * @param dst   The destination buffer
 * @param src   The source buffer
 * @param cnt   The sample count
 * @param ofs   The sample offset in both src and dst
 * @param sign  The sample signedness in both src and dst
 *
 * The source and destination buffer can be the same.
 *****************************************************************************/
void snd_iir2_filter (sound_iir2_t *iir,
	uint16_t *dst, const uint16_t *src, unsigned cnt, unsigned ofs, int sign
);


#endif
