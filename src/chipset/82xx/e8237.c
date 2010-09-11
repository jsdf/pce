/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/82xx/e8237.c                                     *
 * Created:     2003-09-11 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2010 Hampa Hug <hampa@hampa.ch>                     *
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

#include "e8237.h"


static void e8237_chn_set_dack (e8237_chn_t *chn, unsigned char val);
static void e8237_chn_set_tc (e8237_chn_t *chn, unsigned char val);
static void e8237_set_hreq (e8237_t *dma, unsigned char val);


static
void e8237_chn_init (e8237_chn_t *chn, e8237_t *dma)
{
	chn->dma = dma;

	chn->base_addr = 0;
	chn->base_cnt = 0;
	chn->cur_addr = 0;
	chn->cur_cnt = 0;

	chn->mode = 0;
	chn->state = E8237_STATE_MASK;

	chn->dack_val = 0;
	chn->tc_val = 0;

	chn->dack_ext = NULL;
	chn->dack = NULL;

	chn->tc_ext = NULL;
	chn->tc = NULL;

	chn->memwr_ext = NULL;
	chn->memwr = NULL;

	chn->memrd_ext = NULL;
	chn->memrd = NULL;

	chn->iowr_ext = NULL;
	chn->iowr = NULL;

	chn->iord_ext = NULL;
	chn->iord = NULL;
}

static
void e8237_chn_reset (e8237_chn_t *chn)
{
	chn->base_addr = 0;
	chn->base_cnt = 0;
	chn->cur_addr = 0;
	chn->cur_cnt = 0;

	chn->mode = 0;
	chn->state = E8237_STATE_MASK;

	e8237_chn_set_dack (chn, 0);
	e8237_chn_set_tc (chn, 0);
}

static
void e8237_chn_set_dack (e8237_chn_t *chn, unsigned char val)
{
	if (chn->dack_val != val) {
		chn->dack_val = val;
		if (chn->dack != NULL) {
			chn->dack (chn->dack_ext, val);
		}
	}
}

static
void e8237_chn_set_tc (e8237_chn_t *chn, unsigned char val)
{
	if (chn->tc_val != val) {
		chn->tc_val = val;
		if (chn->tc != NULL) {
			chn->tc (chn->tc_ext, val);
		}
	}
}

static
void e8237_chn_set_addr (e8237_chn_t *chn, unsigned char val, unsigned char *ff)
{
	if (*ff) {
		chn->cur_addr &= 0x00ff;
		chn->cur_addr |= (val & 0xff) << 8;
	}
	else {
		chn->cur_addr &= 0xff00;
		chn->cur_addr |= val & 0xff;
	}

	chn->base_addr = chn->cur_addr;

	*ff = !*ff;
}

static
unsigned char e8237_chn_get_addr (e8237_chn_t *chn, unsigned char *ff)
{
	*ff = !*ff;

	if (*ff) {
		return (chn->cur_addr & 0xff);
	}
	else {
		return ((chn->cur_addr >> 8) & 0xff);
	}
}

static
void e8237_chn_set_cnt (e8237_chn_t *chn, unsigned char val, unsigned char *ff)
{
	if (*ff) {
		chn->cur_cnt &= 0x00ff;
		chn->cur_cnt |= (val & 0xff) << 8;
	}
	else {
		chn->cur_cnt &= 0xff00;
		chn->cur_cnt |= val & 0xff;
	}

	chn->base_cnt = chn->cur_cnt;

	*ff = !*ff;
}

static
unsigned char e8237_chn_get_cnt (e8237_chn_t *chn, unsigned char *ff)
{
	*ff = !*ff;

	if (*ff) {
		return (chn->cur_cnt & 0xff);
	}
	else {
		return ((chn->cur_cnt >> 8) & 0xff);
	}
}

static
void e8237_chn_set_mode (e8237_chn_t *chn, unsigned char val)
{
	chn->mode = val;
}

static
void e8237_chn_set_mask (e8237_chn_t *chn, unsigned char val)
{
	if (val) {
		chn->state |= E8237_STATE_MASK;
	}
	else {
		chn->state &= ~E8237_STATE_MASK;
		if (chn->state & (E8237_STATE_DREQ | E8237_STATE_SREQ)) {
			chn->dma->check = 1;
		}
	}
}

static
void e8237_chn_set_sreq (e8237_chn_t *chn, unsigned char val)
{
	if (val) {
		chn->state |= (E8237_STATE_PREQ | E8237_STATE_SREQ);
		if ((chn->state & E8237_STATE_MASK) == 0) {
			chn->dma->check = 1;
		}
	}
	else {
		chn->state &= ~(E8237_STATE_PREQ | E8237_STATE_SREQ);
	}
}

static
void e8237_chn_tc (e8237_chn_t *chn)
{
	chn->state |= E8237_STATE_TC;

	if (chn->mode & E8237_MODE_AUTO) {
		chn->cur_addr = chn->base_addr;
		chn->cur_cnt = chn->base_cnt;
	}
	else {
		chn->state |= (E8237_STATE_MASK);
	}

	e8237_chn_set_tc (chn, 1);
	e8237_chn_set_tc (chn, 0);
}

static
unsigned char e8237_chn_get_status (e8237_chn_t *chn)
{
	unsigned char ret;

	ret = (chn->state & E8237_STATE_PREQ) ? 0x10 : 0x00;
	ret |= (chn->state & E8237_STATE_TC) ? 0x01 : 0x00;

	chn->state &= ~E8237_STATE_TC;

	return (ret);
}

static
int e8237_chn_transfer (e8237_chn_t *chn)
{
	unsigned char val;

	e8237_chn_set_dack (chn, 1);

	if ((chn->mode & E8237_MODE_MODE) == E8237_MODE_CASCADE) {
		return (0);
	}

	switch (chn->mode & E8237_MODE_TYPE) {
	case E8237_MODE_READ:
		if (chn->memrd != NULL) {
			val = chn->memrd (chn->memrd_ext, chn->cur_addr);
		}
		else {
			val = 0;
		}

		if (chn->iowr != NULL) {
			chn->iowr (chn->iowr_ext, val);
		}
		break;

	case E8237_MODE_WRITE:
		if (chn->iord != NULL) {
			val = chn->iord (chn->iord_ext);
		}
		else {
			val = 0;
		}

		if (chn->memwr != NULL) {
			chn->memwr (chn->memwr_ext, chn->cur_addr, val);
		}
		break;

	case E8237_MODE_VERIFY:
		if (chn->iord != NULL) {
			chn->iord (chn->iord_ext);
		}

		if (chn->memrd != NULL) {
			chn->memrd (chn->memrd_ext, chn->cur_addr);
		}
		break;
	}

	chn->cur_addr += (chn->mode & E8237_MODE_ADDRDEC) ? 0xffff : 0x0001;
	chn->cur_addr &= 0xffff;

	chn->cur_cnt = (chn->cur_cnt - 1) & 0xffff;

	if (chn->cur_cnt == 0xffff) {
		e8237_chn_tc (chn);
		return (1);
	}

	if ((chn->mode & E8237_MODE_MODE) == E8237_MODE_BLOCK) {
		return (0);
	}

	return (1);
}

static
void e8237_chn_set_dreq (e8237_chn_t *chn, unsigned char val)
{
	if (val) {
		chn->state |= (E8237_STATE_PREQ | E8237_STATE_DREQ);
		if ((chn->state & E8237_STATE_MASK) == 0) {
			chn->dma->check = 1;
		}
	}
	else {
		chn->state &= ~(E8237_STATE_PREQ | E8237_STATE_DREQ);
		e8237_chn_set_dack (chn, 0);
	}
}

static
void e8237_chn_set_eop (e8237_chn_t *chn, unsigned char val)
{
	if (val) {
		e8237_chn_tc (chn);
	}
}


void e8237_init (e8237_t *dma)
{
	unsigned i;

	for (i = 0; i < 4; i++) {
		e8237_chn_init (&dma->chn[i], dma);
	}

	dma->check = 0;

	dma->cmd = 0;
	dma->flipflop = 0;
	dma->priority = 0;

	dma->hreq_val = 0;
	dma->hlda_val = 0;

	dma->hreq_ext = NULL;
	dma->hreq = NULL;
}

e8237_t *e8237_new (void)
{
	e8237_t *dma;

	dma = malloc (sizeof (e8237_t));
	if (dma == NULL) {
		return (NULL);
	}

	e8237_init (dma);

	return (dma);
}

void e8237_free (e8237_t *dma)
{
}

void e8237_del (e8237_t *dma)
{
	if (dma != NULL) {
		e8237_free (dma);
		free (dma);
	}
}


void e8237_set_dack_fct (e8237_t *dma, unsigned chn, void *ext, void *fct)
{
	if (chn < 4) {
		dma->chn[chn].dack_ext = ext;
		dma->chn[chn].dack = fct;
	}
}

void e8237_set_tc_fct (e8237_t *dma, unsigned chn, void *ext, void *fct)
{
	if (chn < 4) {
		dma->chn[chn].tc_ext = ext;
		dma->chn[chn].tc = fct;
	}
}


unsigned char e8237_get_command (e8237_t *dma)
{
	return (dma->cmd);
}

unsigned char e8237_get_priority (e8237_t *dma)
{
	return (dma->priority);
}

unsigned char e8237_get_mode (e8237_t *dma, unsigned i)
{
	if (i < 4) {
		return (dma->chn[i].mode);
	}

	return (0);
}

unsigned short e8237_get_state (e8237_t *dma, unsigned i)
{
	if (i < 4) {
		return (dma->chn[i].state);
	}

	return (0);
}

unsigned short e8237_get_addr (e8237_t *dma, unsigned i)
{
	if (i < 4) {
		return (dma->chn[i].cur_addr);
	}

	return (0);
}

unsigned short e8237_get_addr_base (e8237_t *dma, unsigned i)
{
	if (i < 4) {
		return (dma->chn[i].base_addr);
	}

	return (0);
}

unsigned short e8237_get_cnt (e8237_t *dma, unsigned i)
{
	if (i < 4) {
		return (dma->chn[i].cur_cnt);
	}

	return (0);
}

unsigned short e8237_get_cnt_base (e8237_t *dma, unsigned i)
{
	if (i < 4) {
		return (dma->chn[i].base_cnt);
	}

	return (0);
}


void e8237_set_hlda (e8237_t *dma, unsigned char val)
{
	dma->hlda_val = (val != 0);
}

void e8237_set_dreq0 (e8237_t *dma, unsigned char val)
{
	e8237_chn_set_dreq (&dma->chn[0], val);
}

void e8237_set_dreq1 (e8237_t *dma, unsigned char val)
{
	e8237_chn_set_dreq (&dma->chn[1], val);
}

void e8237_set_dreq2 (e8237_t *dma, unsigned char val)
{
	e8237_chn_set_dreq (&dma->chn[2], val);
}

void e8237_set_dreq3 (e8237_t *dma, unsigned char val)
{
	e8237_chn_set_dreq (&dma->chn[3], val);
}

void e8237_set_eop0 (e8237_t *dma, unsigned char val)
{
	e8237_chn_set_eop (&dma->chn[0], val);
}

void e8237_set_eop1 (e8237_t *dma, unsigned char val)
{
	e8237_chn_set_eop (&dma->chn[1], val);
}

void e8237_set_eop2 (e8237_t *dma, unsigned char val)
{
	e8237_chn_set_eop (&dma->chn[2], val);
}

void e8237_set_eop3 (e8237_t *dma, unsigned char val)
{
	e8237_chn_set_eop (&dma->chn[3], val);
}


static
void e8237_set_command (e8237_t *dma, unsigned char val)
{
	dma->cmd = val;

	dma->check = ((val & E8237_CMD_DISABLE) == 0);
}

static
unsigned char e8237_get_status (e8237_t *dma)
{
	unsigned char ret;

	ret = e8237_chn_get_status (&dma->chn[0]);
	ret |= (e8237_chn_get_status (&dma->chn[1]) << 1);
	ret |= (e8237_chn_get_status (&dma->chn[2]) << 2);
	ret |= (e8237_chn_get_status (&dma->chn[3]) << 3);

	return (ret);
}

static
unsigned char e8237_get_temporary (e8237_t *dma)
{
	return (0);
}

static
void e8237_set_hreq (e8237_t *dma, unsigned char val)
{
	if (dma->hreq_val != val) {
		dma->hreq_val = val;
		if (dma->hreq != NULL) {
			dma->hreq (dma->hreq_ext, val);
		}
	}
}


void e8237_set_uint8 (e8237_t *dma, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case 0x00:
		e8237_chn_set_addr (&dma->chn[0], val, &dma->flipflop);
		break;

	case 0x01:
		e8237_chn_set_cnt (&dma->chn[0], val, &dma->flipflop);
		break;

	case 0x02:
		e8237_chn_set_addr (&dma->chn[1], val, &dma->flipflop);
		break;

	case 0x03:
		e8237_chn_set_cnt (&dma->chn[1], val, &dma->flipflop);
		break;

	case 0x04:
		e8237_chn_set_addr (&dma->chn[2], val, &dma->flipflop);
		break;

	case 0x05:
		e8237_chn_set_cnt (&dma->chn[2], val, &dma->flipflop);
		break;

	case 0x06:
		e8237_chn_set_addr (&dma->chn[3], val, &dma->flipflop);
		break;

	case 0x07:
		e8237_chn_set_cnt (&dma->chn[3], val, &dma->flipflop);
		break;

	case 0x08:
		e8237_set_command (dma, val);
		break;

	case 0x09:
		e8237_chn_set_sreq (&dma->chn[val & 0x03], val);
		break;

	case 0x0a:
		e8237_chn_set_mask (&dma->chn[val & 0x03], (val & 0x04) != 0);
		break;

	case 0x0b:
		e8237_chn_set_mode (&dma->chn[val & 0x03], val);
		break;

	case 0x0c:
		dma->flipflop = 0;
		break;

	case 0x0d:
		e8237_reset (dma);
		break;

	case 0x0e:
		e8237_chn_set_mask (&dma->chn[0], 0);
		e8237_chn_set_mask (&dma->chn[1], 0);
		e8237_chn_set_mask (&dma->chn[2], 0);
		e8237_chn_set_mask (&dma->chn[3], 0);
		break;

	case 0x0f:
		e8237_chn_set_mask (&dma->chn[0], (val & 0x01) != 0);
		e8237_chn_set_mask (&dma->chn[1], (val & 0x02) != 0);
		e8237_chn_set_mask (&dma->chn[2], (val & 0x04) != 0);
		e8237_chn_set_mask (&dma->chn[3], (val & 0x08) != 0);
		break;
	}
}

void e8237_set_uint16 (e8237_t *dma, unsigned long addr, unsigned short val)
{
}

void e8237_set_uint32 (e8237_t *dma, unsigned long addr, unsigned long val)
{
}

unsigned char e8237_get_uint8 (e8237_t *dma, unsigned long addr)
{
	switch (addr) {
	case 0x00:
		return (e8237_chn_get_addr (&dma->chn[0], &dma->flipflop));

	case 0x01:
		return (e8237_chn_get_cnt (&dma->chn[0], &dma->flipflop));

	case 0x02:
		return (e8237_chn_get_addr (&dma->chn[1], &dma->flipflop));

	case 0x03:
		return (e8237_chn_get_cnt (&dma->chn[1], &dma->flipflop));

	case 0x04:
		return (e8237_chn_get_addr (&dma->chn[2], &dma->flipflop));

	case 0x05:
		return (e8237_chn_get_cnt (&dma->chn[2], &dma->flipflop));

	case 0x06:
		return (e8237_chn_get_addr (&dma->chn[3], &dma->flipflop));

	case 0x07:
		return (e8237_chn_get_cnt (&dma->chn[3], &dma->flipflop));

	case 0x08:
		return (e8237_get_status (dma));

	case 0x0f:
		return (e8237_get_temporary (dma));
	}

	return (0xff);
}

unsigned short e8237_get_uint16 (e8237_t *dma, unsigned long addr)
{
	return (0xffff);
}

unsigned long e8237_get_uint32 (e8237_t *dma, unsigned long addr)
{
	return (0xffffffff);
}

void e8237_reset (e8237_t *dma)
{
	e8237_chn_reset (&dma->chn[0]);
	e8237_chn_reset (&dma->chn[1]);
	e8237_chn_reset (&dma->chn[2]);
	e8237_chn_reset (&dma->chn[3]);

	dma->check = 0;

	dma->cmd = 0;
	dma->flipflop = 0;
	dma->priority = 0;

	e8237_set_hreq (dma, 0);
}

static
void e8237_check (e8237_t *dma)
{
	unsigned    i, j;
	e8237_chn_t *chn;

	dma->check = 0;

	j = dma->priority;

	for (i = 0; i < 4; i++) {
		chn = &dma->chn[j];

		if (chn->state & (E8237_STATE_DREQ | E8237_STATE_SREQ)) {
			if ((chn->state & E8237_STATE_MASK) == 0) {
				dma->check = 1;

				e8237_set_hreq (dma, 1);

				if (dma->hlda_val == 0) {
					return;
				}

				if (e8237_chn_transfer (chn)) {
					e8237_set_hreq (dma, 0);

					if (dma->cmd & E8237_CMD_ROTPRI) {
						dma->priority = (j + 1) & 0x03;
					}
					else {
						dma->priority = 0;
					}
				}
				else {
					dma->priority = j;
				}
			}
		}

		j = (j + 1) & 0x03;
	}

	e8237_set_hreq (dma, 0);
}

void e8237_clock (e8237_t *dma, unsigned n)
{
	if (dma->cmd & E8237_CMD_DISABLE) {
		return;
	}

	while (n > 0) {
		if (dma->check == 0) {
			return;
		}

		e8237_check (dma);

		n -= 1;
	}
}
