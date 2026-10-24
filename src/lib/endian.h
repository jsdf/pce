/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/endian.h                                             *
 * Created:     2022-02-12 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2022 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_LIB_ENDIAN_H
#define PCE_LIB_ENDIAN_H 1


#include <stdint.h>


static inline
void set_uint8 (void *buf, unsigned idx, uint8_t val)
{
	uint8_t *p = (uint8_t *) buf + idx;

	p[0] = val & 0xff;
}

static inline
void set_uint16_be (void *buf, unsigned idx, uint16_t val)
{
	uint8_t *p = (uint8_t *) buf + idx;

	p[0] = (val >> 8) & 0xff;
	p[1] = val & 0xff;
}

static inline
void set_uint16_le (void *buf, unsigned idx, uint16_t val)
{
	uint8_t *p = (uint8_t *) buf + idx;

	p[0] = val & 0xff;
	p[1] = (val >> 8) & 0xff;
}

static inline
void set_uint32_be (void *buf, unsigned idx, uint32_t val)
{
	uint8_t *p = (uint8_t *) buf + idx;

	p[0] = (val >> 24) & 0xff;
	p[1] = (val >> 16) & 0xff;
	p[2] = (val >> 8) & 0xff;
	p[3] = val & 0xff;
}

static inline
void set_uint32_le (void *buf, unsigned idx, uint32_t val)
{
	uint8_t *p = (uint8_t *) buf + idx;

	p[0] = val & 0xff;
	p[1] = (val >> 8) & 0xff;
	p[2] = (val >> 16) & 0xff;
	p[3] = (val >> 24) & 0xff;
}

static inline
uint8_t get_uint8 (const void *buf, unsigned idx)
{
	const uint8_t *p = (const uint8_t *) buf + idx;

	return (*p & 0xff);
}

static inline
uint16_t get_uint16_be (const void *buf, unsigned idx)
{
	uint16_t      val;
	const uint8_t *p = (const uint8_t *) buf + idx;

	val = p[0] & 0xff;
	val = (val << 8) | (p[1] & 0xff);

	return (val);
}

static inline
uint16_t get_uint16_le (const void *buf, unsigned idx)
{
	uint16_t      val;
	const uint8_t *p = (const uint8_t *) buf + idx;

	val = p[1] & 0xff;
	val = (val << 8) | (p[0] & 0xff);

	return (val);
}

static inline
uint32_t get_uint32_be (const void *buf, unsigned idx)
{
	uint32_t      val;
	const uint8_t *p = (const uint8_t *) buf + idx;

	val = p[0] & 0xff;
	val = (val << 8) | (p[1] & 0xff);
	val = (val << 8) | (p[2] & 0xff);
	val = (val << 8) | (p[3] & 0xff);

	return (val);
}

static inline
uint32_t get_uint32_le (const void *buf, unsigned idx)
{
	uint32_t      val;
	const uint8_t *p = (const uint8_t *) buf + idx;

	val = p[3] & 0xff;
	val = (val << 8) | (p[2] & 0xff);
	val = (val << 8) | (p[1] & 0xff);
	val = (val << 8) | (p[0] & 0xff);

	return (val);
}


#endif
