/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/sim405/hook.c                                       *
 * Created:     2018-12-22 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2018 Hampa Hug <hampa@hampa.ch>                          *
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
#include "hook.h"
#include "msg.h"
#include "sim405.h"

#include <cpu/ppc405/ppc405.h>
#include <devices/memory.h>


#define PCE_HOOK_CHECK           0x0000
#define PCE_HOOK_GET_VERSION     0x0001
#define PCE_HOOK_GET_VERSION_STR 0x0002
#define PCE_HOOK_INIT            0x0100
#define PCE_HOOK_PUTC            0x0101
#define PCE_HOOK_GETC            0x0102
#define PCE_HOOK_STOP            0x0200
#define PCE_HOOK_ABORT           0x0201
#define PCE_HOOK_MSG             0x0300
#define PCE_HOOK_GET_TIME_UNIX   0x0400
#define PCE_HOOK_READ_OPEN       0x0500
#define PCE_HOOK_READ_CLOSE      0x0501
#define PCE_HOOK_READ            0x0502
#define PCE_HOOK_WRITE_OPEN      0x0600
#define PCE_HOOK_WRITE_CLOSE     0x0601
#define PCE_HOOK_WRITE           0x0602


void s405_hook_init (sim405_t *sim)
{
	sim->hook_read = NULL;
	sim->hook_write = NULL;

	sim->hook_idx = 0;
	sim->hook_cnt = 0;
}

void s405_hook_free (sim405_t *sim)
{
	if (sim->hook_write != NULL) {
		fclose (sim->hook_write);
	}

	if (sim->hook_read != NULL) {
		fclose (sim->hook_read);
	}
}

static
int hook_get_str (sim405_t *sim, char *str, unsigned max)
{
	if (sim->hook_cnt >= max) {
		return (1);
	}

	memcpy (str, sim->hook_buf, sim->hook_cnt);

	str[sim->hook_cnt] = 0;

	return (0);
}

static
int hook_get_str2 (sim405_t *sim, char *str1, char *str2, unsigned max)
{
	unsigned i, idx;
	char     *dst;

	dst = str1;
	idx = 0;

	for (i = 0; i < sim->hook_cnt; i++) {
		if (idx >= max) {
			return (1);
		}

		if (sim->hook_buf[i] == 0) {
			dst[idx] = 0;
			dst = str2;
			idx = 0;
		}
		else {
			dst[idx++] = sim->hook_buf[i];
		}
	}

	if (idx >= max) {
		return (1);
	}

	dst[idx] = 0;

	return (0);
}

static
int hook_set_str (sim405_t *sim, const char *str)
{
	unsigned n;

	n = strlen (str);

	if (n > 256) {
		return (1);
	}

	memcpy (sim->hook_buf, str, n);

	sim->hook_idx = 0;
	sim->hook_cnt = n;

	return (0);
}

static
void s405_hook_log (sim405_t *sim)
{
	pce_log (MSG_DEB,
		"%08lX: hook (R3=%08lX R4=%08lX)\n",
		(unsigned long) p405_get_pc (sim->ppc),
		(unsigned long) p405_get_gpr (sim->ppc, 3),
		(unsigned long) p405_get_gpr (sim->ppc, 4)
	);
}

static
void s405_hook_check (sim405_t *sim)
{
	p405_set_gpr (sim->ppc, 3, 0xffffffce);
}

static
void s405_hook_get_version (sim405_t *sim)
{
	p405_set_gpr (sim->ppc, 4, PCE_VERSION_MAJ);
	p405_set_gpr (sim->ppc, 5, PCE_VERSION_MIN);
	p405_set_gpr (sim->ppc, 6, PCE_VERSION_MIC);

	p405_set_gpr (sim->ppc, 3, 0);
}

static
void s405_hook_get_version_str (sim405_t *sim)
{
	p405_set_gpr (sim->ppc, 3, -1);

	if (hook_set_str (sim, PCE_VERSION_STR)) {
		return;
	}

	p405_set_gpr (sim->ppc, 3, 0);
}

static
void s405_hook_initc (sim405_t *sim)
{
	sim->hook_idx = 0;
	sim->hook_cnt = 0;

	p405_set_gpr (sim->ppc, 3, 0);
}

static
void s405_hook_putc (sim405_t *sim)
{
	p405_set_gpr (sim->ppc, 3, -1);

	if (sim->hook_cnt >= 256) {
		return;
	}

	sim->hook_buf[sim->hook_cnt++] = p405_get_gpr (sim->ppc, 4) & 0xff;

	p405_set_gpr (sim->ppc, 3, 0);
}

static
void s405_hook_getc (sim405_t *sim)
{
	if (sim->hook_idx >= sim->hook_cnt) {
		p405_set_gpr (sim->ppc, 3, -1);
		return;
	}

	p405_set_gpr (sim->ppc, 3, sim->hook_buf[sim->hook_idx++]);
}

static
void s405_hook_stop (sim405_t *sim)
{
	sim->brk = 1;
	p405_set_gpr (sim->ppc, 3, 0);
}

static
void s405_hook_abort (sim405_t *sim)
{
	sim->brk = 2;
	p405_set_gpr (sim->ppc, 3, 0);
}

static
void s405_hook_msg (sim405_t *sim)
{
	char msg[256], val[256];

	p405_set_gpr (sim->ppc, 3, -1);

	if (hook_get_str2 (sim, msg, val, 256)) {
		return;
	}

	if (s405_set_msg (sim, msg, val)) {
		return;
	}

	p405_set_gpr (sim->ppc, 3, 0);
}

static
void s405_hook_get_time_unix (sim405_t *sim)
{
	unsigned long long tm;

	tm = (unsigned long long) time (NULL);

	p405_set_gpr (sim->ppc, 4, (tm >> 32) & 0xffffffff);
	p405_set_gpr (sim->ppc, 5, tm & 0xffffffff);

	p405_set_gpr (sim->ppc, 3, 0);
}

static
void s405_hook_read_open (sim405_t *sim)
{
	char str[256];

	p405_set_gpr (sim->ppc, 3, 0xffffffff);

	if (hook_get_str (sim, str, 256)) {
		return;
	}

	if (sim->hook_read != NULL) {
		fclose (sim->hook_read);
	}

	if ((sim->hook_read = fopen (str, "rb")) == NULL) {
		return;
	}

	p405_set_gpr (sim->ppc, 3, 0);
}

static
void s405_hook_read_close (sim405_t *sim)
{
	p405_set_gpr (sim->ppc, 3, 0xffffffff);

	if (sim->hook_read != NULL) {
		fclose (sim->hook_read);
		sim->hook_read = NULL;
	}

	p405_set_gpr (sim->ppc, 3, 0);
}

static
void s405_hook_read (sim405_t *sim)
{
	p405_t        *c;
	unsigned      n;
	unsigned char buf[16];

	c = sim->ppc;

	if (sim->hook_read == NULL) {
		p405_set_gpr (c, 3, 0xffffffff);
		return;
	}

	memset (buf, 0, 16);

	n = fread (buf, 1, 16, sim->hook_read);

	p405_set_gpr (c, 4, buf_get_uint32_be (buf, 0));
	p405_set_gpr (c, 5, buf_get_uint32_be (buf, 4));
	p405_set_gpr (c, 6, buf_get_uint32_be (buf, 8));
	p405_set_gpr (c, 7, buf_get_uint32_be (buf, 12));

	p405_set_gpr (c, 3, n);
}

static
void s405_hook_write_open (sim405_t *sim)
{
	char str[256];

	p405_set_gpr (sim->ppc, 3, 0xffffffff);

	if (hook_get_str (sim, str, 256)) {
		return;
	}

	if (sim->hook_write != NULL) {
		fclose (sim->hook_write);
	}

	if ((sim->hook_write = fopen (str, "wb")) == NULL) {
		return;
	}

	p405_set_gpr (sim->ppc, 3, 0);
}

static
void s405_hook_write_close (sim405_t *sim)
{
	p405_set_gpr (sim->ppc, 3, 0xffffffff);

	if (sim->hook_write != NULL) {
		fclose (sim->hook_write);
		sim->hook_write = NULL;
	}

	p405_set_gpr (sim->ppc, 3, 0);
}

static
void s405_hook_write (sim405_t *sim)
{
	unsigned      n;
	unsigned char buf[16];

	p405_set_gpr (sim->ppc, 3, 0xffffffff);

	if (sim->hook_write == NULL) {
		return;
	}

	n = p405_get_gpr (sim->ppc, 8);

	if ((n == 0) || (n > 16)) {
		return;
	}

	buf_set_uint32_be (buf, 0, p405_get_gpr (sim->ppc, 4));
	buf_set_uint32_be (buf, 4, p405_get_gpr (sim->ppc, 5));
	buf_set_uint32_be (buf, 8, p405_get_gpr (sim->ppc, 6));
	buf_set_uint32_be (buf, 12, p405_get_gpr (sim->ppc, 7));

	n = fwrite (buf, 1, n, sim->hook_write);

	p405_set_gpr (sim->ppc, 3, n);
}

void s405_hook (sim405_t *sim, unsigned long ir)
{
	switch (p405_get_gpr (sim->ppc, 3)) {
	case PCE_HOOK_CHECK:
		s405_hook_check (sim);
		break;

	case PCE_HOOK_GET_VERSION:
		s405_hook_get_version (sim);
		break;

	case PCE_HOOK_GET_VERSION_STR:
		s405_hook_get_version_str (sim);
		break;

	case PCE_HOOK_INIT:
		s405_hook_initc (sim);
		break;

	case PCE_HOOK_PUTC:
		s405_hook_putc (sim);
		break;

	case PCE_HOOK_GETC:
		s405_hook_getc (sim);
		break;

	case PCE_HOOK_STOP:
		s405_hook_stop (sim);
		break;

	case PCE_HOOK_ABORT:
		s405_hook_abort (sim);
		break;

	case PCE_HOOK_MSG:
		s405_hook_msg (sim);
		break;

	case PCE_HOOK_GET_TIME_UNIX:
		s405_hook_get_time_unix (sim);
		break;

	case PCE_HOOK_READ_OPEN:
		s405_hook_read_open (sim);
		break;

	case PCE_HOOK_READ_CLOSE:
		s405_hook_read_close (sim);
		break;

	case PCE_HOOK_READ:
		s405_hook_read (sim);
		break;

	case PCE_HOOK_WRITE_OPEN:
		s405_hook_write_open (sim);
		break;

	case PCE_HOOK_WRITE_CLOSE:
		s405_hook_write_close (sim);
		break;

	case PCE_HOOK_WRITE:
		s405_hook_write (sim);
		break;

	default:
		s405_hook_log (sim);
		p405_set_gpr (sim->ppc, 3, 0xffffffff);
		break;
	}
}
