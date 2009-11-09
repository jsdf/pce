/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/char/char.c                                      *
 * Created:     2009-03-06 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009 Hampa Hug <hampa@hampa.ch>                          *
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

#include <drivers/char/char.h>


struct chr_drv_list {
	const char *prefix;
	char_drv_t *(*open) (const char *name);
};

struct chr_drv_list drvtab[] = {
	{ "null", chr_null_open },
#ifdef PCE_ENABLE_CHAR_POSIX
	{ "posix", chr_posix_open },
	{ "sercon", chr_posix_open },
#endif
#ifdef PCE_ENABLE_CHAR_PTY
	{ "pty", chr_pty_open },
#endif
#ifdef PCE_ENABLE_CHAR_SLIP
	{ "slip", chr_slip_open },
#endif
	{ "stdio", chr_stdio_open },
#ifdef PCE_ENABLE_CHAR_TCP
	{ "tcp", chr_tcp_open },
#endif
#ifdef PCE_ENABLE_CHAR_TIOS
	{ "tios", chr_tios_open },
#endif
	{ NULL, NULL }
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

int chr_set_log (char_drv_t *cdrv, const char *fname)
{
	if (cdrv->log_fp != NULL) {
		chr_log_flush (cdrv);
		fclose (cdrv->log_fp);
	}

	cdrv->log_fp = fopen (fname, "w");

	if (cdrv->log_fp == NULL) {
		return (1);
	}

	return (0);
}


static
const char *chr_skip_option (const char *str)
{
	while (*str != 0) {
		if (*str == ':') {
			if (*(str + 1) == ':') {
				str += 2;
			}
			else {
				return (str + 1);
			}
		}
		else {
			str += 1;
		}
	}

	return (NULL);
}

static
const char *chr_skip_space (const char *str)
{
	while (1) {
		if (*str == ' ') {
			str += 1;
		}
		else if (*str == '\t') {
			str += 1;
		}
		else {
			return (str);
		}
	}
}

static
const char *chr_get_option_value (const char *str, const char *name)
{
	while (*name != 0) {
		if (*str != *name) {
			return (NULL);
		}

		str += 1;
		name += 1;
	}

	if (*str == '=') {
		str = chr_skip_space (str + 1);

		return (str);
	}

	return (NULL);
}

static
const char *chr_get_option_name (const char *str)
{
	unsigned i;

	str = chr_skip_space (str);

	i = 0;

	while (str[i] != 0) {
		if (str[i] == '=') {
			return (str);
		}
		else if (str[i] == ':') {
			if (str[i + 1] == ':') {
				i += 2;
			}
			else {
				return (NULL);
			}
		}
		else {
			i += 1;
		}
	}

	return (NULL);
}

static
char *chr_option_dup (const char *str)
{
	unsigned i, j, n;
	char     *ret;

	n = 0;
	while (str[n] != 0) {
		if (str[n] == ':') {
			if (str[n + 1] == ':') {
				n += 2;
			}
			else {
				break;
			}
		}
		else {
			n += 1;
		}
	}

	while (n > 0) {
		if (str[n - 1] == ' ') {
			n -= 1;
		}
		else if (str[n - 1] == '\t') {
			n -= 1;
		}
		else {
			break;
		}
	}

	ret = malloc (n + 1);
	if (ret == NULL) {
		return (NULL);
	}

	i = 0;
	j = 0;

	while (i < n) {
		if ((str[i] == ':') && (str[i + 1] == ':')) {
			ret[j] = ':';
			i += 2;
		}
		else {
			ret[j] = str[i];
			i += 1;
		}

		j += 1;
	}

	ret[j] = 0;

	return (ret);
}

char *chr_get_named_option (const char *str, const char *name)
{
	const char *val;

	while (str != NULL) {
		val = chr_get_option_value (str, name);

		if (val != NULL) {
			return (chr_option_dup (val));
		}

		str = chr_skip_option (str);
	}

	return (NULL);
}

char *chr_get_indexed_option (const char *str, unsigned idx)
{
	while (str != NULL) {
		if (chr_get_option_name (str) == NULL) {
			if (idx == 0) {
				str = chr_skip_space (str);

				return (chr_option_dup (str));
			}

			idx -= 1;
		}

		str = chr_skip_option (str);
	}

	return (NULL);
}

char *chr_get_option (const char *str, const char *name, unsigned idx)
{
	char *ret;

	if (name != NULL) {
		ret = chr_get_named_option (str, name);

		if (ret != NULL) {
			return (ret);
		}
	}

	if (idx != 0xffff) {
		ret = chr_get_indexed_option (str, idx);

		if (ret != NULL) {
			return (ret);
		}
	}

	return (NULL);
}

int chr_get_option_bool (const char *str, const char *name, unsigned idx, int def)
{
	int  r;
	char *s;

	s = chr_get_option (str, name, idx);

	if (s == NULL) {
		return (def);
	}

	r = def;

	if (strcmp (s, "1") == 0) {
		r = 1;
	}
	else if (strcmp (s, "true") == 0) {
		r = 1;
	}
	else if (strcmp (s, "yes") == 0) {
		r = 1;
	}
	else if (strcmp (s, "0") == 0) {
		r = 0;
	}
	else if (strcmp (s, "false") == 0) {
		r = 0;
	}
	else if (strcmp (s, "no") == 0) {
		r = 0;
	}

	free (s);

	return (r);
}

unsigned long chr_get_option_uint (const char *str, const char *name, unsigned idx, unsigned long def)
{
	unsigned long val;
	char          *end;
	char          *s;

	s = chr_get_option (str, name, idx);

	if (s == NULL) {
		return (def);
	}

	val = strtoul (s, &end, 0);

	if ((end != NULL) && (*end != 0)) {
		free (s);
		return (def);
	}

	free (s);

	return (val);
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

	cdrv->close = NULL;

	cdrv->read = NULL;
	cdrv->write = NULL;

	cdrv->get_ctl = NULL;
	cdrv->set_ctl = NULL;

	cdrv->set_params = NULL;
}

void chr_close (char_drv_t *cdrv)
{
	if (cdrv == NULL) {
		return;
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

static
char_drv_t *chr_open_cdrv (char_drv_t *cdrv, const char *name)
{
	char *str;

	if (cdrv == NULL) {
		return (NULL);
	}

	str = chr_get_option (name, "log", 0xffff);

	if (str != NULL) {
		chr_set_log (cdrv, str);

		free (str);
	}

	return (cdrv);
}

char_drv_t *chr_open (const char *name)
{
	unsigned   i;
	const char *s, *d;

	i = 0;

	while (drvtab[i].prefix != NULL) {
		s = name;
		d = drvtab[i].prefix;

		while ((*d != 0) && (*d == *s)) {
			d += 1;
			s += 1;
		}

		if ((*d == 0) && ((*s == ':') || (*s == 0))) {
			return (chr_open_cdrv (drvtab[i].open (name), name));
		}

		i += 1;
	}

	return (NULL);
}
