/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/video.c                                       *
 * Created:     2012-06-29 by Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"
#include "video.h"

#include <stdio.h>
#include <stdlib.h>


#define E82730_STATUS_VDIP 0x0100
#define E82730_STATUS_DIP  0x0080
#define E82730_STATUS_RDC  0x0040
#define E82730_STATUS_RCC  0x0020
#define E82730_STATUS_EOF  0x0008


#ifndef DEBUG_E82730
#define DEBUG_E82730 0
#endif


static unsigned char e82730_rgb[4 * 16] = {
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xaa, 0x00,
	0x00, 0xaa, 0x00, 0x00,
	0x00, 0xaa, 0xaa, 0x00,
	0xaa, 0x00, 0x00, 0x00,
	0xaa, 0x00, 0xaa, 0x00,
	0xaa, 0xaa, 0x00, 0x00,
	0xaa, 0xaa, 0xaa, 0x00,
	0x55, 0x55, 0x55, 0x00,
	0x55, 0x55, 0xff, 0x00,
	0x55, 0xff, 0x55, 0x00,
	0x55, 0xff, 0xff, 0x00,
	0xff, 0x55, 0x55, 0x00,
	0xff, 0x55, 0xff, 0x00,
	0xff, 0xff, 0x55, 0x00,
	0xff, 0xff, 0xff, 0x00
};

static unsigned char e82730_rgb_mono[4 * 16] = {
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0xaa, 0xaa, 0xaa, 0x00,
	0xaa, 0xaa, 0xaa, 0x00,
	0xaa, 0xaa, 0xaa, 0x00,
	0xaa, 0xaa, 0xaa, 0x00,
	0x55, 0x55, 0x55, 0x00,
	0x55, 0x55, 0x55, 0x00,
	0x55, 0x55, 0x55, 0x00,
	0x55, 0x55, 0x55, 0x00,
	0xff, 0xff, 0xff, 0x00,
	0xff, 0xff, 0xff, 0x00,
	0xff, 0xff, 0xff, 0x00,
	0xff, 0xff, 0xff, 0x00
};


static void e82730_adjust_buffer (e82730_t *crt);


void e82730_init (e82730_t *crt)
{
	crt->scb = 0;
	crt->ibp = 0;
	crt->cbp = 0;
	crt->pixel_mem = 0x000d0000;

	crt->graphic = 0;
	crt->monochrome = 0;

	crt->first_ca = 1;

	crt->addr_mask = 0x000fffff;

	crt->rbp[0] = &crt->rowbuf[0];
	crt->rbp[1] = &crt->rowbuf[1];

	crt->rgb = e82730_rgb;

	crt->buf = NULL;
	crt->buf_cnt = 0;
	crt->buf_w = 0;
	crt->buf_h = 0;
	crt->buf_min_h = 0;
	crt->buf_y = 0;

	crt->input_clock_mul = 1;
	crt->input_clock_div = 1;

	crt->trm = NULL;

	crt->set_mem_ext = NULL;
	crt->set_mem8 = NULL;
	crt->set_mem16 = NULL;

	crt->get_mem_ext = NULL;
	crt->get_mem8 = NULL;
	crt->get_mem16 = NULL;

	crt->sint_val = 0;
	crt->sint_ext = NULL;
	crt->sint = NULL;
}

void e82730_free (e82730_t *crt)
{
	free (crt->buf);
}

e82730_t *e82730_new (void)
{
	e82730_t *crt;

	crt = malloc (sizeof (e82730_t));

	if (crt == NULL) {
		return (NULL);
	}

	e82730_init (crt);

	return (crt);
}

void e82730_del (e82730_t *crt)
{
	if (crt != NULL) {
		e82730_free (crt);
		free (crt);
	}
}

void e82730_set_getmem_fct (e82730_t *crt, void *ext, void *get8, void *get16)
{
	crt->get_mem_ext = ext;
	crt->get_mem8 = get8;
	crt->get_mem16 = get16;
}

void e82730_set_setmem_fct (e82730_t *crt, void *ext, void *set8, void *set16)
{
	crt->set_mem_ext = ext;
	crt->set_mem8 = set8;
	crt->set_mem16 = set16;
}

void e82730_set_sint_fct (e82730_t *crt, void *ext, void *fct)
{
	crt->sint_ext = ext;
	crt->sint = fct;
}

void e82730_set_clock (e82730_t *crt, unsigned long mul, unsigned long div)
{
	crt->input_clock_mul = mul;
	crt->input_clock_div = div;
}

void e82730_set_terminal (e82730_t *crt, terminal_t *trm)
{
	crt->trm = trm;
}

void e82730_set_monochrome (e82730_t *crt, int val)
{
	crt->monochrome = (val != 0);

	if (val) {
		crt->rgb = e82730_rgb_mono;
	}
	else {
		crt->rgb = e82730_rgb;
	}
}

void e82730_set_graphic (e82730_t *crt, int val)
{
	crt->graphic = (val != 0);
}

void e82730_set_min_h (e82730_t *crt, unsigned val)
{
	crt->buf_min_h = val;

	e82730_adjust_buffer (crt);
}

static
void e82730_set_sint (e82730_t *crt, int val)
{
	val = (val != 0);

	if (crt->sint_val == val) {
		return;
	}

	crt->sint_val = val;

	if (crt->sint != NULL) {
		crt->sint (crt->sint_ext, val);
	}
}

void e82730_set_mem8 (e82730_t *crt, unsigned long addr, unsigned char val)
{
	if (crt->set_mem8 != NULL) {
		crt->set_mem8 (crt->set_mem_ext, addr & crt->addr_mask, val);
	}
}

void e82730_set_mem16 (e82730_t *crt, unsigned long addr, unsigned short val)
{
	if (crt->set_mem16 != NULL) {
		crt->set_mem16 (crt->set_mem_ext, addr & crt->addr_mask, val);
	}
}

unsigned char e82730_get_mem8 (e82730_t *crt, unsigned long addr)
{
	if (crt->get_mem8 != NULL) {
		return (crt->get_mem8 (crt->get_mem_ext, addr & crt->addr_mask));
	}

	return (0);
}

unsigned short e82730_get_mem16 (e82730_t *crt, unsigned long addr)
{
	if (crt->get_mem16 != NULL) {
		return (crt->get_mem16 (crt->get_mem_ext, addr & crt->addr_mask));
	}

	return (0);
}

unsigned long e82730_get_mem32 (e82730_t *crt, unsigned long addr)
{
	unsigned long val, val1, val2;

	val = 0;

	if (crt->get_mem16 != NULL) {
		val1 = crt->get_mem16 (crt->get_mem_ext, addr & crt->addr_mask);
		val2 = crt->get_mem16 (crt->get_mem_ext, (addr + 2) & crt->addr_mask);
		val = (val2 << 16) | val1;
	}

	return (val);
}

static
void e82730_check_int (e82730_t *crt)
{
	unsigned short intstat;

	intstat = crt->status & ~crt->intmask & ~(E82730_STATUS_VDIP | E82730_STATUS_DIP);

	e82730_set_mem16 (crt, crt->cbp + 20, intstat);

	if (intstat) {
		e82730_set_sint (crt, 1);
	}
}

void e82730_reset (e82730_t *crt)
{
	unsigned i;

	crt->scb = 0;
	crt->ibp = 0;
	crt->cbp = 0;

	crt->intmask = 0xffff;

	crt->first_ca = 1;
	crt->mode_set = 0;
	crt->frame_int_count[0] = 0;
	crt->frame_int_count[1] = 0;
	crt->channel_attention = 0;
	crt->blank_row = 0;

	crt->field_attrib = 0;

	for (i = 0; i < 2; i++) {
		crt->cursor_enable[i] = 0;
		crt->cursor_reverse[i] = 0;
		crt->cursor_blink[i] = 0;
		crt->cursor_x[i] = 0xff;
		crt->cursor_y[i] = 0xff;
		crt->cursor_start[i] = 0xff;
		crt->cursor_stop[i] = 0xff;
	}

	crt->cursor_cnt = 0;
	crt->cursor_duty = 2;
	crt->cursor_max = 28;

	crt->status = 0;

	crt->stridx = 0;

	crt->rbp[0]->ready = 0;
	crt->rbp[0]->cnt = 0;

	crt->rbp[1]->ready = 0;
	crt->rbp[1]->cnt = 0;

	for (i = 0; i < 31; i++) {
		crt->palette[i] = 0x70;
	}

	crt->buf_w = 0;
	crt->buf_h = 0;
	crt->buf_y = 0;

	e82730_set_sint (crt, 0);
}

void e82730_set_palette (e82730_t *crt, unsigned idx, unsigned char val)
{
	if (idx < 64) {
		crt->palette[idx] = val;
	}
}

unsigned char e82730_get_palette (const e82730_t *crt, unsigned idx)
{
	if (idx < 64) {
		return (crt->palette[idx]);
	}

	return (0);
}

static
void e82730_set_buf_size (e82730_t *crt, unsigned w, unsigned h)
{
	unsigned long cnt;
	unsigned char *buf;

	cnt = 3 * (unsigned long) w * (unsigned long) h;

	if (crt->buf_cnt != cnt) {
		buf = realloc (crt->buf, cnt);

		if (buf == NULL) {
			return;
		}

		crt->buf = buf;
		crt->buf_cnt = cnt;
	}

	crt->buf_w = w;
	crt->buf_h = h;
	crt->buf_y = 0;
}

static
void e82730_adjust_buffer (e82730_t *crt)
{
	unsigned w, h;

	w = 16 * (crt->mode_hfldstp - crt->mode_hfldstrt);
	h = crt->mode_vfldstp - crt->mode_vfldstrt + 64;

	if (h < crt->buf_min_h) {
		h = crt->buf_min_h;
	}

	e82730_set_buf_size (crt, w, h);
}

static
void e82730_cmd_start_display (e82730_t *crt)
{
	if (crt->mode_set == 0) {
		return;
	}

	crt->status |= E82730_STATUS_DIP;

#if DEBUG_E82730 >= 1
	sim_log_deb ("82730: CMD: START DISPLAY\n");
#endif
}

static
void e82730_cmd_stop_display (e82730_t *crt)
{
	crt->status &= ~(E82730_STATUS_VDIP | E82730_STATUS_DIP);

#if DEBUG_E82730 >= 1
	sim_log_deb ("82730: CMD: STOP DISPLAY\n");
#endif
}

static
void e82730_cmd_modeset (e82730_t *crt)
{
	unsigned short v;
	unsigned long  mp;

	mp = e82730_get_mem32 (crt, crt->cbp + 30);

	v = e82730_get_mem16 (crt, mp + 2);
	crt->mode_line_length = (v >> 8) & 0xff;
	v = e82730_get_mem16 (crt, mp + 4);
	crt->mode_hfldstrt = (v >> 8) & 0xff;
	crt->mode_hfldstp = v & 0xff;

	v = e82730_get_mem16 (crt, mp + 10);
	crt->mode_lines_per_row = v & 0x1f;
	crt->blank_row = (v & 0x400) != 0;

	v = e82730_get_mem16 (crt, mp + 18);
	crt->cursor_start[0] = (v >> 8) & 0x1f;
	crt->cursor_stop[0] = v & 0x1f;

	v = e82730_get_mem16 (crt, mp + 20);
	crt->cursor_start[1] = (v >> 8) & 0x1f;
	crt->cursor_stop[1] = v & 0x1f;

	crt->mode_frame_length = e82730_get_mem16 (crt, mp + 26) & 0x07ff;
	crt->mode_vfldstrt = e82730_get_mem16 (crt, mp + 30) & 0x07ff;
	crt->mode_vfldstp = e82730_get_mem16 (crt, mp + 32) & 0x07ff;

	v = e82730_get_mem16 (crt, mp + 38);
	crt->frame_int_count[0] = v & 15;
	crt->cursor_max = 4 * ((v >> 8) & 0x1f);
	crt->cursor_duty = ((v >> 13) & 0x07);

	v = e82730_get_mem16 (crt, mp + 40);
	crt->cursor_blink[0] = v & 1;
	crt->cursor_blink[1] = (v >> 1) & 1;

	v = e82730_get_mem16 (crt, mp + 42);
	crt->cursor_enable[0] = v & 1;
	crt->cursor_enable[1] = (v >> 1) & 1;
	crt->cursor_reverse[0] = (v >> 2) & 1;
	crt->cursor_reverse[1] = (v >> 3) & 1;

	e82730_adjust_buffer (crt);

	crt->mode_set = 1;

#if DEBUG_E82730 >= 1
	sim_log_deb ("82730: CMD: MODE SET\n");
#endif
}

static
void e82730_cmd_load_cbp (e82730_t *crt)
{
	crt->cbp = e82730_get_mem32 (crt, crt->cbp + 14);

#if DEBUG_E82730 >= 1
	sim_log_deb ("82730: CMD: LOAD CBP (%08lX)\n", crt->cbp);
#endif
}

static
void e82730_cmd_load_intmask (e82730_t *crt)
{
	crt->intmask = e82730_get_mem16 (crt, crt->cbp + 22);

#if DEBUG_E82730 >= 1
	sim_log_deb ("82730: CMD: LOAD INTMASK (%04X)\n", crt->intmask);
#endif
}

static
void e82730_cmd_read_status (e82730_t *crt)
{
	e82730_set_mem16 (crt, crt->cbp + 18, crt->status);

#if DEBUG_E82730 >= 2
	sim_log_deb ("82730: CMD: READ STATUS (%04X)\n", crt->status);
#endif
}

static
void e82730_cmd_ld_cur_pos (e82730_t *crt)
{
	unsigned short cur1, cur2;

	cur1 = e82730_get_mem16 (crt, crt->cbp + 26);
	cur2 = e82730_get_mem16 (crt, crt->cbp + 28);

	crt->cursor_x[0] = cur1 & 0xff;
	crt->cursor_y[0] = (cur1 >> 8) & 0xff;

	crt->cursor_x[1] = cur2 & 0xff;
	crt->cursor_y[1] = (cur2 >> 8) & 0xff;

#if DEBUG_E82730 >= 2
	sim_log_deb ("82730: CMD: LD CUR POS (%04X %04X)\n", cur1, cur2);
#endif
}

static
void e82730_cmd_unknown (e82730_t *crt, unsigned cmd)
{
	crt->status |= E82730_STATUS_RCC;

	e82730_check_int (crt);

#if DEBUG_E82730 >= 1
	sim_log_deb ("82730: CMD: UNKNOWN (%02X)\n", cmd);
#endif
}

static
void e82730_check_ca (e82730_t *crt)
{
	unsigned short cmd, val;

	if (crt->channel_attention == 0) {
		return;
	}

	crt->channel_attention = 0;

	if (crt->first_ca) {
		crt->first_ca = 0;
		crt->scb = e82730_get_mem8 (crt, 0xfffffff6);
		crt->ibp = e82730_get_mem32 (crt, 0xfffffffc);
		crt->scb = e82730_get_mem8 (crt, crt->ibp);
		crt->cbp = e82730_get_mem32 (crt, crt->ibp + 2);

#if DEBUG_E82730 >= 1
		sim_log_deb ("82730: first ca\n");
		sim_log_deb ("82730: SYSBUS: %02X\n", crt->scb);
		sim_log_deb ("82730: IBP: %08lX\n", crt->ibp);
		sim_log_deb ("82730: SCB: %02X\n", crt->scb);
		sim_log_deb ("82730: CBP: %08lX\n", crt->cbp);
#endif
	}

	cmd = e82730_get_mem8 (crt, crt->cbp + 1);

	if (cmd == 0x05) {
		e82730_cmd_load_cbp (crt);

		cmd = e82730_get_mem8 (crt, crt->cbp + 1);
	}

	val = e82730_get_mem16 (crt, crt->cbp + 2);
	crt->auto_lf = (val & 0x40) != 0;
	crt->list_switch = (val & 0x80) != 0;

	val = e82730_get_mem16 (crt, crt->cbp + 4);
	crt->max_dma = val & 0xff;

	switch (cmd) {
	case 0x00:
		break;

	case 0x01:
		e82730_cmd_start_display (crt);
		break;

	case 0x03:
		e82730_cmd_stop_display (crt);
		break;

	case 0x04:
		e82730_cmd_modeset (crt);
		break;

	case 0x05:
		e82730_cmd_load_cbp (crt);
		break;

	case 0x06:
		e82730_cmd_load_intmask (crt);
		break;

	case 0x08:
		e82730_cmd_read_status (crt);
		break;

	case 0x09:
		e82730_cmd_ld_cur_pos (crt);
		break;

	default:
		e82730_cmd_unknown (crt, cmd);
		break;
	}

	e82730_set_mem8 (crt, crt->cbp, 0);
}

void e82730_set_ca (e82730_t *crt)
{
	crt->channel_attention = 1;

	if ((crt->status & E82730_STATUS_DIP) == 0) {
		e82730_check_ca (crt);
	}
}

void e82730_set_srst (e82730_t *crt)
{
	e82730_set_sint (crt, 0);
}

static
void e82730_dscmd_endrow (e82730_t *crt, e82730_rowbuf_t *rb)
{
	rb->ready = 1;
}

static
void e82730_dscmd_eof (e82730_t *crt, e82730_rowbuf_t *rb)
{
	crt->end_of_frame = 1;
}

static
void e82730_dscmd_end_string_row (e82730_t *crt, e82730_rowbuf_t *rb)
{
#if DEBUG_E82730 >= 2
	sim_log_deb ("82730: DS CMD: END STRING/ROW (%08lX)\n", crt->strptr - 2);
#endif

	crt->strptr = e82730_get_mem32 (crt, crt->lbase);
	crt->lbase += 4;

	rb->ready = 1;
}

static
void e82730_dscmd_fullrowdescrpt (e82730_t *crt, e82730_rowbuf_t *rb, unsigned cmd)
{
	unsigned i, n;

	n = cmd & 0xff;

	if (n > 7) {
		return;
	}

#if DEBUG_E82730 >= 2
	sim_log_deb ("82730: DS CMD: FULLROWDESCRPT (%u)\n", n);
#endif

	for (i = 0; i < n; i++) {
		rb->fullrowdesc[i] = e82730_get_mem16 (crt, crt->strptr);
		crt->strptr += 2;
	}

	rb->fullrowdesc_cnt = n;
}

static
void e82730_dscmd_ld_max_dma_count (e82730_t *crt, e82730_rowbuf_t *rb, unsigned cmd)
{
#if DEBUG_E82730 >= 2
	sim_log_deb ("82730: DS CMD: LD MAX DMA COUNT (%u)\n", cmd & 0xff);
#endif

	crt->max_dma = cmd & 0xff;
}

static
void e82730_dscmd_endstrg (e82730_t *crt, e82730_rowbuf_t *rb, unsigned cmd)
{
#if DEBUG_E82730 >= 2
	sim_log_deb ("82730: DS CMD: ENDSTRG (%08lX)\n", crt->strptr - 2);
#endif

	crt->strptr = e82730_get_mem32 (crt, crt->lbase);
	crt->lbase += 4;
}

static
void e82730_dscmd_repeat (e82730_t *crt, e82730_rowbuf_t *rb, unsigned cmd)
{
	unsigned       cnt;
	unsigned short val;

	cnt = cmd & 0xff;

	val = e82730_get_mem16 (crt, crt->strptr);

	crt->strptr += 2;

#if DEBUG_E82730 >= 2
	sim_log_deb ("82730: DS CMD: REPEAT (%u * %04X)\n", cnt, val);
#endif

	while (cnt > 0) {
		rb->buf[rb->cnt++] = val;

		if (rb->cnt >= crt->max_dma) {
			rb->ready = 1;
			return;
		}

		cnt -= 1;
	}
}

static
void e82730_dscmd_set_field_attrib (e82730_t *crt, e82730_rowbuf_t *rb)
{
	crt->field_attrib = e82730_get_mem16 (crt, crt->strptr);

	crt->strptr += 2;

#if DEBUG_E82730 >= 2
	sim_log_deb ("82730: DS CMD: SET FIELD ATTRIB (%04X)\n", crt->field_attrib);
#endif
}

static
void e82730_dscmd (e82730_t *crt, e82730_rowbuf_t *rb, unsigned short cmd)
{
	if ((cmd & 0xc000) == 0xc000) {
		return;
	}

	switch ((cmd >> 8) & 0xff) {
	case 0x80:
		e82730_dscmd_endrow (crt, rb);
		break;

	case 0x81:
		e82730_dscmd_eof (crt, rb);
		break;

	case 0x82:
		e82730_dscmd_end_string_row (crt, rb);
		break;

	case 0x83:
		e82730_dscmd_fullrowdescrpt (crt, rb, cmd);
		break;

	case 0x87:
		e82730_dscmd_ld_max_dma_count (crt, rb, cmd);
		break;

	case 0x88:
		e82730_dscmd_endstrg (crt, rb, cmd);
		break;

	case 0x8a:
		e82730_dscmd_repeat (crt, rb, cmd);
		break;

	case 0x8e:
		e82730_dscmd_set_field_attrib (crt, rb);
		break;

	default:
#if DEBUG_E82730 >= 1
		fprintf (stderr,
			"82730: unknown data stream command (%08lX: %04X)\n",
			crt->strptr - 2, cmd
		);
#endif
		break;
	}
}

static
void e82730_fetch_row (e82730_t *crt, e82730_rowbuf_t *rb)
{
	unsigned short val;

	if (crt->end_of_frame) {
		rb->ready = 1;
		return;
	}

	while (rb->ready == 0) {
		val = e82730_get_mem16 (crt, crt->strptr);

		crt->strptr += 2;

		if (val & 0x8000) {
			e82730_dscmd (crt, rb, val);
		}
		else {
			rb->buf[rb->cnt++] = val | crt->field_attrib;

			if ((rb->cnt >= crt->max_dma) || (rb->cnt > 200)) {
				rb->ready = 1;

				if (crt->auto_lf == 0) {
					crt->strptr = e82730_get_mem32 (crt, crt->lbase);
					crt->lbase += 4;
				}
			}
		}
	}
}

static
void e82730_line_empty (e82730_t *crt, e82730_rowbuf_t *rb, unsigned char *p)
{
	unsigned i;

	for (i = 0; i < crt->buf_w; i++) {
		if (((crt->buf_y & 0x0f) < 8) == ((i & 0x0f) < 8)) {
			*(p++) = 0x80;
			*(p++) = 0x80;
			*(p++) = 0x80;
		}
		else {
			*(p++) = 0x00;
			*(p++) = 0x00;
			*(p++) = 0x00;
		}
	}
}

static
void e82730_line_blank (e82730_t *crt, unsigned char *p)
{
	unsigned i;

	for (i = 0; i < crt->buf_w; i++) {
		*(p++) = 0x00;
		*(p++) = 0x00;
		*(p++) = 0x00;
	}
}

static
int e82730_is_cursor (e82730_t *crt, unsigned idx, unsigned x)
{
	unsigned duty;

	if (crt->cursor_y[idx] != crt->cur_row) {
		return (0);
	}

	if (crt->cursor_x[idx] != x) {
		return (0);
	}

	if (crt->cur_row_idx < crt->cursor_start[idx]) {
		return (0);
	}

	if (crt->cur_row_idx > crt->cursor_stop[idx]) {
		return (0);
	}

	if (crt->cursor_blink[idx] == 0) {
		return (1);
	}

	duty = (crt->cursor_duty == 0) ? 4 : crt->cursor_duty;
	duty = (duty * crt->cursor_max) / 4;

	if (crt->cursor_cnt > duty) {
		return (0);
	}

	return (1);
}

static
void e82730_line_text (e82730_t *crt, e82730_rowbuf_t *rb, unsigned char *p)
{
	int                 curs;
	unsigned            i;
	unsigned            idx, val, adr, msk, atr;
	const unsigned char *fg, *bg;

	idx = 0;
	val = 0;
	msk = 0;

	curs = 0;

	fg = &crt->rgb[4 * 15];
	bg = &crt->rgb[4 * 0];

	for (i = 0; i < crt->buf_w; i++) {
		if (msk == 0) {
			if (idx < rb->cnt) {
				atr = (rb->buf[idx] >> 10) & 0x1f;
				atr = crt->palette[atr];

				fg = &crt->rgb[(atr >> 2) & 0x3c];
				bg = &crt->rgb[(atr << 2) & 0x3c];

				adr = rb->buf[idx] & 0x3ff;
				adr = ((adr << 4) | crt->cur_row_idx) << 1;
				val = e82730_get_mem16 (crt, crt->pixel_mem + adr);

				msk = (~val << 1) | 0xfe00;
			}
			else {
				val = 0;
				msk = 1;
			}

			idx += 1;

			if (crt->graphic) {
				msk = 1;
			}

			curs = 0;

			if (e82730_is_cursor (crt, 0, idx - 1)) {
				curs = 1;
			}
			else if (e82730_is_cursor (crt, 1, idx - 1)) {
				curs = 1;
			}

			if (curs) {
				const unsigned char *tmp;

				tmp = fg;
				fg = bg;
				bg = tmp;
			}
		}

		if (val & 0x8000) {
			*(p++) = fg[0];
			*(p++) = fg[1];
			*(p++) = fg[2];
		}
		else {
			*(p++) = bg[0];
			*(p++) = bg[1];
			*(p++) = bg[2];
		}

		val = (val << 1) & 0xffff;
		msk = (msk << 1) & 0xffff;
	}
}

static
void e82730_line (e82730_t *crt)
{
	e82730_rowbuf_t *rb;
	unsigned char   *p;

	if (crt->buf_y >= crt->buf_h) {
		return;
	}

	rb = crt->rbp[0];

	p = crt->buf + crt->buf_y * 3UL * crt->buf_w;

	if (rb->ready == 0) {
		e82730_line_empty (crt, rb, p);
	}
	else if (crt->blank_row) {
		e82730_line_blank (crt, p);
	}
	else {
		e82730_line_text (crt, rb, p);
	}

	crt->buf_y += 1;
}

static
void e82730_rowbuf_init (e82730_t *crt)
{
	crt->rbp[0]->ready = 0;
	crt->rbp[0]->cnt = 0;
	crt->rbp[0]->fullrowdesc_cnt = 0;

	crt->rbp[1]->ready = 0;
	crt->rbp[1]->cnt = 0;
	crt->rbp[1]->fullrowdesc_cnt = 0;
}

static
void e82730_rowbuf_switch (e82730_t *crt)
{
	e82730_rowbuf_t *rb;

	rb = crt->rbp[0];
	crt->rbp[0] = crt->rbp[1];
	crt->rbp[1] = rb;

	rb = crt->rbp[0];

	if (rb->fullrowdesc_cnt > 0) {
		crt->mode_lines_per_row = rb->fullrowdesc[0] & 0x1f;
		crt->blank_row = (rb->fullrowdesc[0] & 0x400) != 0;
	}

	if (rb->fullrowdesc_cnt >= 5) {
		crt->cursor_start[0] = (rb->fullrowdesc[4] >> 8) & 0x1f;
		crt->cursor_stop[0] = rb->fullrowdesc[4] & 0x1f;
	}

	if (rb->fullrowdesc_cnt >= 6) {
		crt->cursor_start[1] = (rb->fullrowdesc[5] >> 8) & 0x1f;
		crt->cursor_stop[1] = rb->fullrowdesc[5] & 0x1f;
	}

	crt->rbp[0]->fullrowdesc_cnt = 0;

	crt->rbp[1]->ready = 0;
	crt->rbp[1]->cnt = 0;
	crt->rbp[1]->fullrowdesc_cnt = 0;
}

static
void e82730_clock_line (e82730_t *crt)
{
	if (crt->status & E82730_STATUS_DIP) {
		if (crt->rbp[1]->ready == 0) {
			e82730_fetch_row (crt, crt->rbp[1]);
		}
	}

	if (crt->cur_line < crt->mode_vfldstrt) {
		if (crt->cur_line == 0) {
			crt->lbase = e82730_get_mem32 (crt, crt->cbp + 6 + (crt->list_switch ? 4 : 0));
			crt->strptr = e82730_get_mem32 (crt, crt->lbase);
			crt->lbase += 4;

			e82730_rowbuf_init (crt);

			crt->field_attrib = 0;

			crt->status &= E82730_STATUS_VDIP | E82730_STATUS_DIP;
		}
	}
	else if (crt->cur_line < crt->mode_vfldstp) {
		if (crt->cur_line == crt->mode_vfldstrt) {
			crt->cur_row = 0;
			crt->cur_row_idx = 0;

			crt->buf_y = 0;

			e82730_rowbuf_switch (crt);
		}

		e82730_line (crt);

		crt->cur_row_idx += 1;

		if (crt->cur_row_idx > crt->mode_lines_per_row) {
			crt->cur_row += 1;
			crt->cur_row_idx = 0;

			e82730_rowbuf_switch (crt);

			if (crt->cur_line == (crt->mode_vfldstp - 1)) {
				crt->rbp[1]->ready = 1;
			}
		}
	}
	else if (crt->cur_line == crt->mode_vfldstp) {
		crt->cur_row_idx = 0;
		crt->end_of_frame = 0;
		crt->strptr = e82730_get_mem32 (crt, crt->cbp + 34);
		e82730_rowbuf_switch (crt);
	}
	else if (crt->cur_line <= (crt->mode_vfldstp + crt->mode_lines_per_row + 1)) {
		if (crt->cur_row_idx == 0) {
			e82730_rowbuf_switch (crt);
			crt->rbp[1]->ready = 1;
		}

		e82730_line (crt);

		crt->cur_row_idx += 1;

		if (crt->cur_row_idx > crt->mode_lines_per_row) {
			crt->cur_row += 1;
			crt->cur_row_idx = 0;
		}
	}

	e82730_check_ca (crt);

	crt->cur_line += 1;

	if (crt->cur_line >= crt->mode_frame_length) {
		while ((crt->buf_y < crt->buf_min_h) && (crt->buf_y < crt->buf_h)) {
			e82730_line_blank (crt, crt->buf + crt->buf_y * 3UL * crt->buf_w);
			crt->buf_y += 1;
		}

		if (crt->trm != NULL) {
			trm_set_size (crt->trm, crt->buf_w, crt->buf_y);
			trm_set_lines (crt->trm, crt->buf, 0, crt->buf_y);
			trm_update (crt->trm);
		}

		crt->cur_line = 0;
		crt->end_of_frame = 0;

		crt->cursor_cnt += 1;

		if (crt->cursor_cnt >= crt->cursor_max) {
			crt->cursor_cnt = 0;
		}

		e82730_check_ca (crt);

		if (crt->frame_int_count[1] > 0) {
			crt->frame_int_count[1] -= 1;
		}

		if (crt->frame_int_count[1] == 0) {
			crt->status |= E82730_STATUS_EOF;
			crt->frame_int_count[1] = crt->frame_int_count[0];
		}

		e82730_check_int (crt);
	}
}

void e82730_clock (e82730_t *crt, unsigned cnt)
{
	unsigned long ll;

	if (crt->mode_set == 0) {
		return;
	}

	crt->rclk += crt->input_clock_mul * cnt;

	ll = crt->input_clock_div * crt->mode_line_length;

	while (crt->rclk >= ll) {
		e82730_clock_line (crt);
		crt->rclk -= ll;
	}
}
