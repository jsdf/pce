/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/dos/path.c                                          *
 * Created:     2013-01-01 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013-2015 Hampa Hug <hampa@hampa.ch>                     *
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
#include "dos.h"
#include "path.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static
int buf_set_size (void **buf, unsigned *cnt, unsigned *max, unsigned size)
{
	unsigned tmp;
	void     *ptr;

	if (size <= *cnt) {
		return (0);
	}

	if (size <= *max) {
		*cnt = size;
		return (0);
	}

	tmp = *max;

	while (tmp < size) {
		tmp = (tmp < 16) ? 16 : (tmp + tmp / 4);
	}

	if ((ptr = realloc (*buf, tmp)) == NULL) {
		return (1);
	}

	*buf = ptr;
	*cnt = size;
	*max = tmp;

	return (0);
}

void sim_buf_init (void **buf, unsigned *cnt, unsigned *max)
{
	*buf = NULL;
	*cnt = 0;
	*max = 0;
}

int sim_buf_add (void **buf, unsigned *cnt, unsigned *max, unsigned char val)
{
	unsigned char *ptr;

	if (buf_set_size (buf, cnt, max, *cnt + 1)) {
		return (1);
	}

	ptr = *buf;
	ptr[*cnt - 1] = val;

	return (0);
}

char *sim_get_dir_name (const char *src, char sep)
{
	unsigned i, n;
	char     *ret;

	i = 0;
	n = 0;
	while (src[i] != 0) {
		if (src[i] == sep) {
			n = i;
		}

		i += 1;
	}

	if (src[n] != sep) {
		src = ".";
		n = 1;
	}
	else if (n == 0) {
		n = 1;
	}

	if ((ret = malloc (n + 1)) == NULL) {
		return (NULL);
	}

	memcpy (ret, src, n);

	ret[n] = 0;

	return (ret);
}

char *sim_make_path (const char *s1, const char *s2)
{
	size_t n1, n2;
	char   *ret;

	n1 = strlen (s1);
	n2 = strlen (s2);

	while ((n1 > 0) && (s1[n1 - 1] == '/')) {
		n1 -= 1;
	}

	while ((n2 > 0) && (s2[0] == '/')) {
		s2 += 1;
		n2 -= 1;
	}

	if ((ret = malloc (n1 + n2 + 2)) == NULL) {
		return (NULL);
	}

	memcpy (ret, s1, n1);
	memcpy (ret + n1 + 1, s2, n2);
	ret[n1] = '/';
	ret[n1 + n2 + 1] = 0;

	return (ret);
}

int sim_get_dos_basename (char *dst, const char *src, char sep)
{
	unsigned   i;
	const char *tmp;

	tmp = src;

	while (*tmp != 0) {
		if (*tmp == sep) {
			src = tmp + 1;
		}

		tmp += 1;
	}

	for (i = 0; i < 8; i++) {
		if ((*src == 0) || (*src == '.')) {
			dst[i] = ' ';
		}
		else if (*src == '*') {
			dst[i] = '?';
		}
		else {
			dst[i] = *src;
			src += 1;
		}
	}

	if (*src == '*') {
		src += 1;
	}

	if (*src == '.') {
		src += 1;

		for (i = 0; i < 3; i++) {
			if (*src == 0) {
				dst[8 + i] = ' ';
			}
			else if (*src == '*') {
				dst[8 + i] = '?';
			}
			else {
				dst[8 + i] = *src;
				src += 1;
			}
		}
	}
	else {
		dst[8] = ' ';
		dst[9] = ' ';
		dst[10] = ' ';
	}

	if (*src == '*') {
		src += 1;
	}

	if (*src != 0) {
		return (1);
	}

	return (0);
}

char *sim_get_dos_full_name (dos_t *sim, const char *name)
{
	unsigned      i, j;
	unsigned char c;
	char          *ret;
	size_t        n;

	n = strlen (name);

	ret = malloc (n + 4);

	i = 0;
	j = 0;

	if ((name[0] != 0) && (name[1] == ':')) {
		ret[j++] = toupper (name[i++]);
		ret[j++] = name[i++];
	}
	else {
		ret[j++] = sim->cur_drive + 'A';
		ret[j++] = ':';
	}

	if ((name[i] == '/') || (name[i] == '\\')) {
		i += 1;
	}

	ret[j++] = '\\';

	while (name[i] != 0) {
		if ((name[i] == '/') || (name[i] == '\\')) {
			c = '\\';
		}
		else {
			c = toupper (name[i]);
		}

		ret[j++] = c;
		i += 1;
	}

	ret[j] = 0;

	return (ret);
}

char *sim_get_host_name (dos_t *sim, const char *dosname)
{
	unsigned char c;
	int           issep;
	unsigned      drv;
	const char    *s;
	void          *ret;
	unsigned      cnt, max;

	drv = sim->cur_drive;

	if ((dosname[0] != 0) && (dosname[1] == ':')) {
		c = toupper (*dosname);

		if ((c >= 'A') && (c <= 'Z')) {
			drv = c - 'A';
		}
		else {
			return (NULL);
		}

		dosname += 2;
	}

	if ((drv >= sim->drive_cnt) || (sim->drive[drv] == NULL)) {
		return (NULL);
	}

	sim_buf_init (&ret, &cnt, &max);

	issep = 0;

	s = sim->drive[drv];

	while (*s != 0) {
		if (*s == '/') {
			while (s[1] == '/') {
				s += 1;
			}

			issep = 1;
		}
		else {
			issep = 0;
		}

		sim_buf_add (&ret, &cnt, &max, *s);

		s += 1;
	}

	if (issep == 0) {
		sim_buf_add (&ret, &cnt, &max, '/');
		issep = 1;
	}

	while (*dosname != 0) {
		c = *dosname;

		if (c == '\\') {
			c = '/';
		}
		else {
			c = tolower (c);
		}

		if ((c != '/') || (issep == 0)) {
			sim_buf_add (&ret, &cnt, &max, c);
		}

		s += 1;
		issep = (c == '/');

		dosname += 1;
	}

	sim_buf_add (&ret, &cnt, &max, 0);

	return (ret);
}
