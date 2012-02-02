/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfdc/pfdc-io.c                                   *
 * Created:     2012-02-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012 Hampa Hug <hampa@hampa.ch>                          *
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

#include "pfdc-io.h"


unsigned pfdc_get_uint16_be (const void *buf, unsigned idx)
{
	unsigned            val;
	const unsigned char *tmp;

	tmp = (const unsigned char *) buf + idx;

	val = tmp[0] & 0xff;
	val = (val << 8) | (tmp[1] & 0xff);

	return (val);
}

unsigned long pfdc_get_uint32_be (const void *buf, unsigned idx)
{
	unsigned long       val;
	const unsigned char *tmp;

	tmp = (const unsigned char *) buf + idx;

	val = tmp[0] & 0xff;
	val = (val << 8) | (tmp[1] & 0xff);
	val = (val << 8) | (tmp[2] & 0xff);
	val = (val << 8) | (tmp[3] & 0xff);

	return (val);
}

void pfdc_set_uint16_be (void *buf, unsigned idx, unsigned val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = (val >> 8) & 0xff;
	tmp[1] = val & 0xff;
}

void pfdc_set_uint32_be (void *buf, unsigned idx, unsigned long val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = (val >> 24) & 0xff;
	tmp[1] = (val >> 16) & 0xff;
	tmp[2] = (val >> 8) & 0xff;
	tmp[3] = val & 0xff;
}

unsigned pfdc_get_uint16_le (const void *buf, unsigned idx)
{
	unsigned            val;
	const unsigned char *tmp;

	tmp = (const unsigned char *) buf + idx;

	val = tmp[1] & 0xff;
	val = (val << 8) | (tmp[0] & 0xff);

	return (val);
}

void pfdc_set_uint16_le (void *buf, unsigned idx, unsigned val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = val & 0xff;
	tmp[1] = (val >> 8) & 0xff;
}


int pfdc_read (FILE *fp, void *buf, unsigned long cnt)
{
	if (fread (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}

int pfdc_read_ofs (FILE *fp, unsigned long ofs, void *buf, unsigned long cnt)
{
	if (fseek (fp, ofs, SEEK_SET) != 0) {
		return (1);
	}

	if (fread (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}

int pfdc_write (FILE *fp, const void *buf, unsigned long cnt)
{
	if (fwrite (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}

int pfdc_skip (FILE *fp, unsigned long cnt)
{
	unsigned long n;
	unsigned char buf[256];

	while (cnt > 0) {
		n = (cnt < 256) ? cnt : 256;

		if (pfdc_read (fp, buf, n)) {
			return (1);
		}

		cnt -= n;
	}

	return (0);
}
