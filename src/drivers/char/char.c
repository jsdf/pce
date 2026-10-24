/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/char/char.c                                      *
 * Created:     2009-03-06 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009-2020 Hampa Hug <hampa@hampa.ch>                     *
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

#include <stdlib.h>
#include <string.h>

#include <drivers/options.h>
#include <drivers/char/char.h>


struct driver_list {
	struct driver_list *next;
	const char         *name;
	chr_open_f         open;
};

struct driver_tab {
	const char *name;
	chr_open_f open;
};


char_drv_t *chr_mouse_open (const char *name);
char_drv_t *chr_null_open (const char *name);
char_drv_t *chr_posix_open (const char *name);
char_drv_t *chr_ppp_open (const char *name);
char_drv_t *chr_pty_open (const char *name);
char_drv_t *chr_slip_open (const char *name);
char_drv_t *chr_stdio_open (const char *name);
char_drv_t *chr_tcp_open (const char *name);
char_drv_t *chr_tios_open (const char *name);
char_drv_t *chr_wincom_open (const char *name);


static struct driver_list *drivers = NULL;


static struct driver_tab default_drivers[] = {
#ifdef PCE_ENABLE_CHAR_POSIX
	{ "posix", chr_posix_open },
	{ "sercon", chr_posix_open },
#endif
#ifdef PCE_ENABLE_CHAR_PPP
	{ "ppp", chr_ppp_open },
#endif
#ifdef PCE_ENABLE_CHAR_PTY
	{ "pty", chr_pty_open },
#endif
#ifdef PCE_ENABLE_CHAR_SLIP
	{ "slip", chr_slip_open },
#endif
#ifdef PCE_ENABLE_CHAR_TCP
	{ "tcp", chr_tcp_open },
#endif
#ifdef PCE_ENABLE_CHAR_TIOS
	{ "tios", chr_tios_open },
#endif
#ifdef PCE_ENABLE_CHAR_WINCOM
	{ "wincom", chr_wincom_open },
#endif
	{ "stdio", chr_stdio_open },
	{ "mouse", chr_mouse_open },
	{ "null", chr_null_open }
};


static
void chr_log_bytes (char_drv_t *cdrv, int out, const unsigned char *buf, unsigned cnt)
{
	int      c;
	unsigned i, n;

	while (cnt > 0) {
		n = (cnt < 16) ? cnt : 16;

		fprintf (cdrv->log_fp, "%s %02X", out ? "->" : "<-", *buf);

		for (i = 1; i < n; i++) {
			fprintf (cdrv->log_fp, " %02X", buf[i]);
		}

		for (i = n; i < 16; i++) {
			fputs ("   ", cdrv->log_fp);
		}

		fputs ("  ", cdrv->log_fp);

		for (i = 0; i < n; i++) {
			c = buf[i];

			if ((c < 0x20) || (c > 0x7e)) {
				c = '.';
			}

			fputc (c, cdrv->log_fp);
		}

		fputs ("\n", cdrv->log_fp);

		buf += n;
		cnt -= n;
	}

	fflush (cdrv->log_fp);
}

static
void chr_log_flush (char_drv_t *cdrv)
{
	if (cdrv->log_cnt > 0) {
		chr_log_bytes (cdrv, cdrv->log_out, cdrv->log_buf, cdrv->log_cnt);
	}

	cdrv->log_cnt = 0;
	cdrv->log_out = 0;
}

static
void chr_log_data (char_drv_t *cdrv, int out, const unsigned char *buf, unsigned cnt)
{
	unsigned n;

	if (cdrv->log_fp == NULL) {
		return;
	}

	if (cnt == 0) {
		return;
	}

	if ((cdrv->log_cnt > 0) && (cdrv->log_out != out)) {
		chr_log_flush (cdrv);
	}

	while (cnt > 0) {
		n = 16 - cdrv->log_cnt;

		if (cnt < n) {
			n = cnt;
		}

		memcpy (cdrv->log_buf + cdrv->log_cnt, buf, n);

		cdrv->log_cnt += n;
		cdrv->log_out = out;

		if (cdrv->log_cnt >= 16) {
			chr_log_flush (cdrv);
		}

		buf += n;
		cnt -= n;
	}
}

static
void chr_log_params (char_drv_t *cdrv)
{
	const char *par;

	if (cdrv->log_fp == NULL) {
		return;
	}

	chr_log_flush (cdrv);

	switch (cdrv->parity) {
	case 0:
		par = "N";
		break;

	case 1:
		par = "O";
		break;

	case 2:
		par = "E";
		break;

	default:
		par = "?";
		break;
	}

	fprintf (cdrv->log_fp, "-- %lu %u%s%u\n",
		cdrv->bps, cdrv->bpc, par, cdrv->stop
	);

	fflush (cdrv->log_fp);
}

static
void chr_log_signal (char_drv_t *cdrv, const char *name, unsigned msk, unsigned old, unsigned new)
{
	if ((old ^ new) & msk) {
		fprintf (cdrv->log_fp, "-- %s=%d\n", name, (new & msk) != 0);
	}
}

static
void chr_log_ctl (char_drv_t *cdrv, unsigned old, unsigned new)
{
	if ((cdrv->log_fp == NULL) || (old == new)) {
		return;
	}

	chr_log_flush (cdrv);

	chr_log_signal (cdrv, "CTS", PCE_CHAR_CTS, old, new);
	chr_log_signal (cdrv, "DTR", PCE_CHAR_DTR, old, new);
	chr_log_signal (cdrv, "RTS", PCE_CHAR_RTS, old, new);
	chr_log_signal (cdrv, "DSR", PCE_CHAR_DSR, old, new);
	chr_log_signal (cdrv, "CD", PCE_CHAR_CD, old, new);
	chr_log_signal (cdrv, "RI", PCE_CHAR_RI, old, new);
}

static
void chr_cap_data (char_drv_t *cdrv, const unsigned char *buf, unsigned cnt)
{
	if (cdrv->cap_fp == NULL) {
		return;
	}

	if (cnt == 0) {
		return;
	}

	fwrite (buf, 1, cnt, cdrv->cap_fp);
	fflush (cdrv->cap_fp);
}

void chr_init (char_drv_t *cdrv, void *ext)
{
	cdrv->ext = ext;

	cdrv->bps = 0;
	cdrv->bpc = 0;
	cdrv->parity = 0;
	cdrv->stop = 0;

	cdrv->ctl_inp = 0;
	cdrv->ctl_out = 0;

	cdrv->log_cnt = 0;
	cdrv->log_out = 0;
	cdrv->log_fp = NULL;

	cdrv->cap_fp = NULL;

	cdrv->close = NULL;

	cdrv->read = NULL;
	cdrv->write = NULL;

	cdrv->get_ctl = NULL;
	cdrv->set_ctl = NULL;

	cdrv->set_params = NULL;
}

static
int chr_match_name (struct driver_list *lst, const char *name)
{
	const char *s, *d;

	s = name;
	d = lst->name;

	while ((*d != 0) && (*d == *s)) {
		d += 1;
		s += 1;
	}

	if ((*d == 0) && ((*s == ':') || (*s == 0))) {
		return (1);
	}

	return (0);
}

static
char_drv_t *chr_open_cdrv (char_drv_t *cdrv, const char *name)
{
	char *str;

	if (cdrv == NULL) {
		return (NULL);
	}

	str = drv_get_option (name, "log");

	if (str != NULL) {
		chr_set_log (cdrv, str);

		free (str);
	}

	str = drv_get_option (name, "cap");

	if (str != NULL) {
		chr_set_cap (cdrv, str);
		free (str);
	}

	return (cdrv);
}

char_drv_t *chr_open (const char *name)
{
	struct driver_list *lst;

	if (drivers == NULL) {
		/*
		 * If no drivers have been registered yet, register the
		 * default set.
		 */
		chr_register_default();
	}

	lst = drivers;

	while (lst != NULL) {
		if (chr_match_name (lst, name)) {
			return (chr_open_cdrv (lst->open (name), name));
		}

		lst = lst->next;
	}

	return (NULL);
}

void chr_close (char_drv_t *cdrv)
{
	if (cdrv == NULL) {
		return;
	}

	if (cdrv->cap_fp != NULL) {
		fclose (cdrv->cap_fp);
	}

	if (cdrv->log_fp != NULL) {
		chr_log_flush (cdrv);
		fclose (cdrv->log_fp);
	}

	if (cdrv->close == NULL) {
		return;
	}

	cdrv->close (cdrv);
}

int chr_register (const char *name, chr_open_f open)
{
	struct driver_list *lst;

	if ((lst = malloc (sizeof (struct driver_list))) == NULL) {
		return (1);
	}

	lst->next = drivers;
	lst->name = name;
	lst->open = open;

	drivers = lst;

	return (0);
}

int chr_unregister (const char *name)
{
	int                r;
	struct driver_list *src, *dst, *drv, *next;

	r = 1;

	src = drivers;
	dst = NULL;
	drv = NULL;

	while (src != NULL) {
		next = src->next;

		if ((name == NULL) || (strcmp (src->name, name) == 0)) {
			free (src);
			r = 0;
		}
		else {
			if (dst != NULL) {
				dst->next = src;
			}
			else {
				drv = src;
			}

			dst = src;
		}

		src = next;
	}

	if (dst != NULL) {
		dst->next = NULL;
	}

	drivers = drv;

	return (r);
}

int chr_register_default (void)
{
	int      r;
	unsigned i, n;

	r = 0;
	n = sizeof (default_drivers) / sizeof (struct driver_tab);

	for (i = 0; i < n; i++) {
		r |= chr_register (default_drivers[i].name, default_drivers[i].open);
	}

	return (r);
}

unsigned chr_read (char_drv_t *cdrv, void *buf, unsigned cnt)
{
	unsigned ret;

	if ((cdrv == NULL) || (cdrv->read == NULL)) {
		return (0);
	}

	ret = cdrv->read (cdrv, buf, cnt);

	chr_log_data (cdrv, 0, buf, ret);

	return (ret);
}

unsigned chr_write (char_drv_t *cdrv, const void *buf, unsigned cnt)
{
	unsigned ret;

	if ((cdrv == NULL) || (cdrv->write == NULL)) {
		return (cnt);
	}

	ret = cdrv->write (cdrv, buf, cnt);

	if (cdrv->cap_fp != NULL) {
		chr_cap_data (cdrv, buf, ret);
	}

	chr_log_data (cdrv, 1, buf, ret);

	return (ret);
}

int chr_get_ctl (char_drv_t *cdrv, unsigned *ctl)
{
	if (cdrv == NULL) {
		return (1);
	}

	if (cdrv->get_ctl == NULL) {
		*ctl = PCE_CHAR_CD;

		if (cdrv->ctl_out & PCE_CHAR_DTR) {
			*ctl |= PCE_CHAR_DSR;
		}

		if (cdrv->ctl_out & PCE_CHAR_RTS) {
			*ctl |= PCE_CHAR_CTS;
		}
	}
	else {
		if (cdrv->get_ctl (cdrv, ctl)) {
			return (1);
		}
	}

	chr_log_ctl (cdrv, cdrv->ctl_inp, *ctl);

	cdrv->ctl_inp = *ctl;

	return (0);
}

int chr_set_ctl (char_drv_t *cdrv, unsigned ctl)
{
	if (cdrv == NULL) {
		return (1);
	}

	if (cdrv->ctl_out == ctl) {
		return (0);
	}

	chr_log_ctl (cdrv, cdrv->ctl_out, ctl);

	cdrv->ctl_out = ctl;

	if (cdrv->set_ctl == NULL) {
		return (0);
	}

	return (cdrv->set_ctl (cdrv, ctl));
}

static
int chr_check_params (char_drv_t *cdrv, unsigned long bps, unsigned bpc, unsigned parity, unsigned stop)
{
	if (cdrv->bps != bps) {
		return (0);
	}

	if (cdrv->bpc != bpc) {
		return (0);
	}

	if (cdrv->parity != parity) {
		return (0);
	}

	if (cdrv->stop != stop) {
		return (0);
	}

	return (1);
}

int chr_set_params (char_drv_t *cdrv, unsigned long bps, unsigned bpc, unsigned parity, unsigned stop)
{
	if (cdrv == NULL) {
		return (1);
	}

	if (chr_check_params (cdrv, bps, bpc, parity, stop)) {
		return (0);
	}

	cdrv->bps = bps;
	cdrv->bpc = bpc;
	cdrv->parity = parity;
	cdrv->stop = stop;

	chr_log_params (cdrv);

	if (cdrv->set_params == NULL) {
		return (1);
	}

	return (cdrv->set_params (cdrv, bps, bpc, parity, stop));
}

FILE *chr_open_file (const char *name, const char *mode, const char *modeapp)
{
	if (name == NULL) {
		return (NULL);
	}

	if (*name == '@') {
		name += 1;
		return (fopen (name, modeapp));
	}

	return (fopen (name, mode));
}

int chr_set_log (char_drv_t *cdrv, const char *fname)
{
	if (cdrv->log_fp != NULL) {
		chr_log_flush (cdrv);
		fclose (cdrv->log_fp);
	}

	if ((cdrv->log_fp = chr_open_file (fname, "w", "a")) == NULL) {
		return (1);
	}

	return (0);
}

int chr_set_cap (char_drv_t *cdrv, const char *fname)
{
	if (cdrv->cap_fp != NULL) {
		fclose (cdrv->cap_fp);
	}

	if ((cdrv->cap_fp = chr_open_file (fname, "wb", "ab")) == NULL) {
		return (1);
	}

	return (0);
}
