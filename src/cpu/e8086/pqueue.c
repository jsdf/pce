/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8086/pqueue.c                                       *
 * Created:     2003-09-20 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#include "e8086.h"
#include "internal.h"


/*
 * Before an instruction is executed the prefetch buffer (pq_buf) is
 * filled up to pq_fill bytes. After the instruction is executed
 * the instruction bytes are discarded and the remaining bytes up
 * to pq_size are copied to the front of the queue. Bytes between
 * pq_size and pq_fill are discarded (and possibly read again from
 * RAM).
 *
 * The prefetch buffer is filled with pq_fill instead of pq_size bytes
 * so that there is always at least one entire instruction in the
 * prefetch buffer. Yes, this is ugly.
 */


void e86_pq_init (e8086_t *c)
{
	c->pq_cnt = 0;
}

/*
 * Fill the prefetch queue
 */
void e86_pq_fill (e8086_t *c)
{
	unsigned       i;
	unsigned short val;
	unsigned short seg, ofs;
	unsigned       cnt;
	unsigned long  addr;

	seg = e86_get_cs (c);
	ofs = e86_get_ip (c);

	cnt = c->pq_fill;

	if (ofs <= (0xffff - cnt)) {
		/* all within one segment */

		addr = e86_get_linear (seg, ofs) & c->addr_mask;

		if ((addr + cnt) <= c->ram_cnt) {
			for (i = c->pq_cnt; i < cnt; i++) {
				c->pq[i] = c->ram[addr + i];
			}
		}
		else {
			i = c->pq_cnt;
			while (i < cnt) {
				val = c->mem_get_uint16 (c->mem, (addr + i) & c->addr_mask);
				c->pq[i] = val & 0xff;
				c->pq[i + 1] = (val >> 8) & 0xff;
				i += 2;
			}
		}
	}
	else {
		i = c->pq_cnt;
		while (i < cnt) {
			val = e86_get_mem16 (c, seg, ofs + i);
			c->pq[i] = val & 0xff;
			c->pq[i + 1] = (val >> 8) & 0xff;

			i += 2;
		}
	}

	c->pq_cnt = c->pq_size;
}

/*
 * Remove cnt bytes from the prefetch queue and copy the rest to the front
 */
void e86_pq_adjust (e8086_t *c, unsigned cnt)
{
	unsigned      n;
	unsigned char *d, *s;

	if (cnt >= c->pq_cnt) {
		c->pq_cnt = 0;
		return;
	}

	n = c->pq_cnt - cnt;
	s = c->pq + cnt;
	d = c->pq;

	while (n > 0) {
		*(d++) = *(s++);
		n -= 1;
	}

	c->pq_cnt -= cnt;
}
