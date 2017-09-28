/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/dos/int21.c                                         *
 * Created:     2012-12-30 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2015 Hampa Hug <hampa@hampa.ch>                     *
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
#include "dosmem.h"
#include "int21.h"
#include "path.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>


static
unsigned sim_get_free_fd (dos_t *sim)
{
	unsigned i;

	for (i = 0; i < sim->file_cnt; i++) {
		if (sim->file[i] == NULL) {
			return (i);
		}
	}

	return (0xffff);
}

static
FILE *int21_get_fp (dos_t *sim, unsigned fd)
{
	if (fd >= sim->file_cnt) {
		return (NULL);
	}

	return (sim->file[fd]);
}

static
void int21_find_done (dos_t *sim)
{
	if (sim->search_dir != NULL) {
		closedir (sim->search_dir);
		sim->search_dir = NULL;
	}

	if (sim->search_dir_name != NULL) {
		free (sim->search_dir_name);
		sim->search_dir_name = NULL;
	}
}

static
int int21_find_start (dos_t *sim, char *name, unsigned attr)
{
	int21_find_done (sim);

	sim->search_attr = attr;

	if (sim_get_dos_basename (sim->search_name, name, '/')) {
		return (1);
	}

	if ((sim->search_dir_name = sim_get_dir_name (name, '/')) == NULL) {
		return (1);
	}

	if ((sim->search_dir = opendir (sim->search_dir_name)) == NULL) {
		return (1);
	}

	return (0);
}

static
int int21_find_match (dos_t *sim, const char *name)
{
	unsigned i;
	char     c1, c2;
	char     dosname[12];

	if (sim_get_dos_basename (dosname, name, '/')) {
		return (0);
	}

	for (i = 0; i < 11; i++) {
		c1 = toupper (sim->search_name[i]);
		c2 = toupper (dosname[i]);

		if ((c1 != c2) && (c1 != '?')) {
			return (0);
		}
	}

	return (1);
}

static
int get_dos_date (time_t t, unsigned short *date, unsigned short *time)
{
	struct tm *tm;

	tm = localtime (&t);

	*time = tm->tm_sec / 2;
	*time |= (tm->tm_min) << 5;
	*time |= (tm->tm_hour) << 11;

	*date = tm->tm_mday & 0x3f;
	*date |= (tm->tm_mon + 1) << 5;
	*date |= (tm->tm_year - 80) << 9;

	return (0);
}

static
int int21_find_cont (dos_t *sim)
{
	unsigned       i;
	unsigned char  c;
	unsigned short seg, ofs;
	unsigned short date, time;
	unsigned char  attr;
	char           *str;
	struct stat    st;
	struct dirent  *ent;

	if ((sim->search_dir == NULL) || (sim->search_dir_name == NULL)) {
		return (1);
	}

	while ((ent = readdir (sim->search_dir)) != NULL) {
		if ((str = sim_make_path (sim->search_dir_name, ent->d_name)) == NULL) {
			continue;
		}

		if (stat (str, &st)) {
			free (str);
			continue;
		}

		free (str);

		if (S_ISDIR (st.st_mode)) {
			attr = 0x10;
		}
		else {
			attr = (st.st_mode & S_IWUSR) ? 0x20 : 0x21;
		}

		if (attr & ~sim->search_attr & ~0x20) {
			continue;
		}

		if (int21_find_match (sim, ent->d_name)) {
			break;
		}
	}

	if (ent == NULL) {
		int21_find_done (sim);
		return (1);
	}

	ofs = sim->dta[0];
	seg = sim->dta[1];

	get_dos_date (st.st_mtime, &date, &time);

	sim_set_uint8 (sim, seg, ofs + 21, attr);
	sim_set_uint16 (sim, seg, ofs + 22, time);
	sim_set_uint16 (sim, seg, ofs + 24, date);
	sim_set_uint16 (sim, seg, ofs + 26, st.st_size & 0xffff);
	sim_set_uint16 (sim, seg, ofs + 28, (st.st_size >> 16) & 0xffff);

	for (i = 0; i < 12; i++) {
		c = toupper (ent->d_name[i]);

		sim_set_uint8 (sim, seg, ofs + 30 + i, c);

		if (c == 0) {
			break;
		}
	}

	sim_set_uint8 (sim, seg, ofs + 30 + 12, 0);

	return (0);
}

static
int int21_ret (dos_t *sim, int cf, unsigned ax)
{
	e86_set_ax (&sim->cpu, ax);
	e86_set_cf (&sim->cpu, cf);

	return (0);
}

/*
 * 00: Terminate
 */
static
int int21_fct_00 (dos_t *sim)
{
	exit (0);
}

/*
 * 01: Console input
 */
static
int int21_fct_01 (dos_t *sim)
{
	int  c;
	FILE *fp;

	if ((fp = int21_get_fp (sim, 0)) == NULL) {
		return (int21_ret (sim, 1, 0x0000));
	}

	if ((c = fgetc (fp)) == EOF) {
		return (int21_ret (sim, 1, 0x0000));
	}

	return (int21_ret (sim, 0, c));
}

/*
 * 02: Console output
 */
static
int int21_fct_02 (dos_t *sim)
{
	FILE *fp;

	if ((fp = int21_get_fp (sim, 1)) == NULL) {
		return (int21_ret (sim, 1, 6));
	}

	fputc (e86_get_dl (&sim->cpu), fp);
	fflush (fp);

	int21_ret (sim, 0, 0x0000);

	return (0);
}

/*
 * 09: Print string
 */
static
int int21_fct_09 (dos_t *sim)
{
	unsigned char  c;
	unsigned short ds, dx;
	FILE           *fp;

	if ((fp = int21_get_fp (sim, 1)) == NULL) {
		return (int21_ret (sim, 1, 0x0006));
	}

	ds = e86_get_ds (&sim->cpu);
	dx = e86_get_dx (&sim->cpu);

	while (1) {
		c = sim_get_uint8 (sim, ds, dx);

		if (c == '$') {
			break;
		}

		fputc (c, fp);

		dx = (dx + 1) & 0xffff;
	}

	fflush (fp);

	int21_ret (sim, 0, 0x0000);

	return (0);
}

/*
 * 0A: Read line
 */
static
int int21_fct_0a (dos_t *sim)
{
	int            c;
	unsigned       cnt, max;
	unsigned short seg, ofs;
	FILE           *fp;

	if ((fp = int21_get_fp (sim, 0)) == NULL) {
		return (int21_ret (sim, 1, 0));
	}

	seg = e86_get_ds (&sim->cpu);
	ofs = e86_get_dx (&sim->cpu);

	max = sim_get_uint8 (sim, seg, ofs);
	cnt = 0;

	while (cnt < max) {
		if ((c = fgetc (fp)) == EOF) {
			break;
		}

		if ((c == 0x0d) || (c == 0x0a)) {
			sim_set_uint8 (sim, seg, ofs + cnt + 2, 0x0d);
			break;
		}

		sim_set_uint8 (sim, seg, ofs + cnt + 2, c);
		cnt += 1;
	}

	sim_set_uint8 (sim, seg, ofs + 1, cnt);

	int21_ret (sim, 0, 0);

	return (0);
}

/*
 * 0B: Check keyboard status
 */
static
int int21_fct_0b (dos_t *sim)
{
	return (int21_ret (sim, 0, 0x0000));
}

/*
 * 0C: Read keyboard
 */
static
int int21_fct_0c (dos_t *sim)
{
	int  c;
	FILE *fp;

	if ((fp = int21_get_fp (sim, 0)) == NULL) {
		return (int21_ret (sim, 1, 0x0000));
	}

	switch (e86_get_al (&sim->cpu)) {
	case 1:
	case 6:
	case 7:
	case 8:
		c = fgetc (fp);
		break;

	default:
		c = 0;
		break;
	}

	if (c == EOF) {
		return (int21_ret (sim, 0, 0x0000));
	}

	return (int21_ret (sim, 0, c));
}

/*
 * 19: Get default drive
 */
static
int int21_fct_19 (dos_t *sim)
{
	return (int21_ret (sim, 0, sim->cur_drive));
}

/*
 * 1A: Set DTA
 */
static
int int21_fct_1a (dos_t *sim)
{
	sim->dta[0] = e86_get_dx (&sim->cpu);
	sim->dta[1] = e86_get_ds (&sim->cpu);

	int21_ret (sim, 0, 0x0000);

	return (0);
}

/*
 * 25: Set interrupt vector
 */
static
int int21_fct_25 (dos_t *sim)
{
	unsigned v;

	v = e86_get_al (&sim->cpu);

	sim_set_uint16 (sim, 0, 4 * v + 0, e86_get_dx (&sim->cpu));
	sim_set_uint16 (sim, 0, 4 * v + 2, e86_get_ds (&sim->cpu));

	return (0);
}

/*
 * 2A: Get date
 */
static
int int21_fct_2a (dos_t *sim)
{
	time_t    t;
	struct tm *tm;

	t = time (NULL);
	tm = localtime (&t);

	e86_set_cx (&sim->cpu, tm->tm_year + 1900);
	e86_set_dh (&sim->cpu, tm->tm_mon + 1);
	e86_set_dl (&sim->cpu, tm->tm_mday);

	int21_ret (sim, 0, tm->tm_wday);

	return (0);
}

/*
 * 2B: Set date
 */
static
int int21_fct_2b (dos_t *sim)
{
	return (int21_ret (sim, 0, 0x0000));
}

/*
 * 2C: Get time
 */
static
int int21_fct_2c (dos_t *sim)
{
	time_t    t;
	struct tm *tm;

	t = time (NULL);
	tm = localtime (&t);

	e86_set_ch (&sim->cpu, tm->tm_hour);
	e86_set_cl (&sim->cpu, tm->tm_min);
	e86_set_dh (&sim->cpu, tm->tm_sec);
	e86_set_dl (&sim->cpu, 0);

	int21_ret (sim, 0, 0x0000);

	return (0);
}

/*
 * 2D: Set time
 */
static
int int21_fct_2d (dos_t *sim)
{
	return (int21_ret (sim, 0, 0x0000));
}

/*
 * 2E: Set verify flag
 */
static
int int21_fct_2e (dos_t *sim)
{
	return (int21_ret (sim, 0, 0x0000));
}

/*
 * 2F: Get DTA
 */
static
int int21_fct_2f (dos_t *sim)
{
	e86_set_bx (&sim->cpu, sim->dta[0]);
	e86_set_es (&sim->cpu, sim->dta[1]);

	return (int21_ret (sim, 0, 0x0000));
}

/*
 * 30: Get DOS version
 */
static
int int21_fct_30 (dos_t *sim)
{
	e86_set_bx (&sim->cpu, 0);
	e86_set_cx (&sim->cpu, 0);

	int21_ret (sim, 0, 0x1e03);

	return (0);
}

/*
 * 3300: Get CTRL-C check
 */
static
int int21_fct_3300 (dos_t *sim)
{
	e86_set_dl (&sim->cpu, 1);
	int21_ret (sim, 0, 0x0000);

	return (0);
}

/*
 * 3301: Set CTRL-C check
 */
static
int int21_fct_3301 (dos_t *sim)
{
	int21_ret (sim, 0, 0x0000);

	return (0);
}

/*
 * 33: Various
 */
static
int int21_fct_33 (dos_t *sim)
{
	switch (e86_get_al (&sim->cpu)) {
	case 0:
		return (int21_fct_3300 (sim));

	case 1:
		return (int21_fct_3301 (sim));
	}

	return (1);
}

/*
 * 35: Get interrupt vector
 */
static
int int21_fct_35 (dos_t *sim)
{
	unsigned v;

	v = e86_get_al (&sim->cpu);

	e86_set_bx (&sim->cpu, sim_get_uint16 (sim, 0, 4 * v + 0));
	e86_set_es (&sim->cpu, sim_get_uint16 (sim, 0, 4 * v + 2));

	return (0);
}

/*
 * 36: Get free disk space
 */
static
int int21_fct_36 (dos_t *sim)
{
	e86_set_bx (&sim->cpu, 0x8000);
	e86_set_cx (&sim->cpu, 512);
	e86_set_dx (&sim->cpu, 0xc000);

	return (int21_ret (sim, 0, 0x0008));
}

/*
 * 37: Get/set switch character
 */
static
int int21_fct_37 (dos_t *sim)
{
	unsigned char al;

	al = e86_get_al (&sim->cpu);

	if (al == 0) {
		e86_set_dl (&sim->cpu, '/');
		int21_ret (sim, 0, 0x0000);
	}
	else {
		return (1);
	}

	return (0);
}

/*
 * 38: Get country information
 */
static
int int21_fct_38 (dos_t *sim)
{
	int21_ret (sim, 1, 0x0001);

	return (0);
}

/*
 * 3C: Create
 */
static
int int21_fct_3c (dos_t *sim)
{
	unsigned       fd;
	unsigned short ds, dx;
	char           *name;
	char           dosname[256];

	if ((fd = sim_get_free_fd (sim)) & 0x8000) {
		return (int21_ret (sim, 1, 0x0001));
	}

	ds = e86_get_ds (&sim->cpu);
	dx = e86_get_dx (&sim->cpu);

	if (sim_get_asciiz (sim, ds, dx, dosname, sizeof (dosname))) {
		return (int21_ret (sim, 1, 0x0001));
	}

	if ((name = sim_get_host_name (sim, dosname)) == NULL) {
		return (int21_ret (sim, 1, 0x0001));
	}

	if (strcasecmp (dosname, "nul") == 0) {
		sim->file[fd] = NULL;
	}
	else {
		sim->file[fd] = fopen (name, "w+b");
	}

	if (sim->file[fd] == NULL) {
		fprintf (stderr, "%s: create failed: %s\n", arg0, name);
		free (name);
		return (int21_ret (sim, 1, 0x0001));
	}

	free (name);

	int21_ret (sim, 0, fd);

	return (0);
}

/*
 * 3D: Open
 */
static
int int21_fct_3d (dos_t *sim)
{
	unsigned       fd;
	const char     *mode;
	unsigned short ds, dx;
	char           dosname[256];
	char           *name;

	if ((fd = sim_get_free_fd (sim)) & 0x8000) {
		return (int21_ret (sim, 1, 1));
	}

	switch (e86_get_al (&sim->cpu) & 0x0f) {
	case 0:
		mode = "rb";
		break;

	case 1:
		mode = "r+b";
		break;

	case 2:
		mode = "r+b";
		break;

	default:
		return (int21_ret (sim, 1, 0x000c));
	}

	ds = e86_get_ds (&sim->cpu);
	dx = e86_get_dx (&sim->cpu);

	if (sim_get_asciiz (sim, ds, dx, dosname, sizeof (dosname))) {
		return (int21_ret (sim, 1, 0x0001));
	}

	if ((name = sim_get_host_name (sim, dosname)) == NULL) {
		return (int21_ret (sim, 1, 0x0001));
	}

	if (strcasecmp (dosname, "nul") == 0) {
		sim->file[fd] = fopen ("/dev/null", mode);
	}
	else {
		sim->file[fd] = fopen (name, mode);
	}

	if (sim->file[fd] == NULL) {
		free (name);

		return (int21_ret (sim, 1, 0x0002));
	}

	free (name);

	int21_ret (sim, 0, fd);

	return (0);
}

/*
 * 3E: Close
 */
static
int int21_fct_3e (dos_t *sim)
{
	FILE     *fp;
	unsigned fd;

	if ((fd = e86_get_bx (&sim->cpu)) >= sim->file_cnt) {
		return (int21_ret (sim, 1, 0x006));
	}

	if ((fp = sim->file[fd]) == NULL) {
		return (int21_ret (sim, 1, 0x0006));
	}

	if ((fp != stdin) && (fp != stdout) && (fp != stderr)) {
		fclose (fp);
	}

	sim->file[fd] = NULL;

	int21_ret (sim, 0, 0);

	return (0);
}

/*
 * 3F: Read
 */
static
int int21_fct_3f (dos_t *sim)
{
	int            c;
	unsigned       i, cnt;
	unsigned short seg, ofs;
	FILE           *fp;

	if ((fp = int21_get_fp (sim, e86_get_bx (&sim->cpu))) == NULL) {
		return (int21_ret (sim, 1, 6));
	}

	cnt = e86_get_cx (&sim->cpu);
	seg = e86_get_ds (&sim->cpu);
	ofs = e86_get_dx (&sim->cpu);

	for (i = 0; i < cnt; i++) {
		if ((c = fgetc (fp)) == EOF) {
			break;
		}

		sim_set_uint8 (sim, seg, ofs, c);

		ofs = (ofs + 1) & 0xffff;
	}

	int21_ret (sim, 0, i);

	return (0);
}

/*
 * 40: Write
 */
static
int int21_fct_40 (dos_t *sim)
{
	int            c;
	unsigned       i, cnt;
	unsigned short seg, ofs;
	FILE           *fp;

	if ((fp = int21_get_fp (sim, e86_get_bx (&sim->cpu))) == NULL) {
		return (int21_ret (sim, 1, 6));
	}

	cnt = e86_get_cx (&sim->cpu);
	seg = e86_get_ds (&sim->cpu);
	ofs = e86_get_dx (&sim->cpu);

	if (cnt == 0) {
		ftruncate (fileno (fp), ftell (fp));
	}

	for (i = 0; i < cnt; i++) {
		c = sim_get_uint8 (sim, seg, ofs);
		fputc (c, fp);
		ofs = (ofs + 1) & 0xfffff;
	}

	fflush (fp);

	int21_ret (sim, 0, i);

	return (0);
}

/*
 * 41: Delete
 */
static
int int21_fct_41 (dos_t *sim)
{
	unsigned short ds, dx;
	char           *name;
	char           dosname[256];

	ds = e86_get_ds (&sim->cpu);
	dx = e86_get_dx (&sim->cpu);

	if (sim_get_asciiz (sim, ds, dx, dosname, sizeof (dosname))) {
		return (int21_ret (sim, 1, 0x0001));
	}

	if ((name = sim_get_host_name (sim, dosname)) == NULL) {
		return (int21_ret (sim, 1, 0x0001));
	}

	if (remove (name)) {
		free (name);
		return (int21_ret (sim, 1, 0x0002));
	}

	free (name);

	int21_ret (sim, 0, 0x0000);

	return (0);
}

/*
 * 42: Seek
 */
static
int int21_fct_42 (dos_t *sim)
{
	FILE          *fp;
	unsigned long ofs;
	int           whence;

	if ((fp = int21_get_fp (sim, e86_get_bx (&sim->cpu))) == NULL) {
		return (int21_ret (sim, 1, 6));
	}

	ofs = e86_get_cx (&sim->cpu);
	ofs = (ofs << 16) | e86_get_dx (&sim->cpu);

	switch (e86_get_al (&sim->cpu)) {
	case 0:
		whence = SEEK_SET;
		break;
	case 1:
		whence = SEEK_CUR;
		break;
	case 2:
		whence = SEEK_END;
		break;
	default:
		return (int21_ret (sim, 1, 0x0001));
	}

	if (fseek (fp, ofs, whence)) {
		return (int21_ret (sim, 1, 0x0001));
	}

	ofs = ftell (fp);

	e86_set_dx (&sim->cpu, ofs >> 16);

	int21_ret (sim, 0, ofs & 0xffff);

	return (0);
}

/*
 * 43: Get/set file attributes
 */
static
int int21_fct_43 (dos_t *sim)
{
	unsigned short ds, dx;
	unsigned short val;
	char           *name;
	char           dosname[256];
	struct stat    st;

	ds = e86_get_ds (&sim->cpu);
	dx = e86_get_dx (&sim->cpu);

	if (sim_get_asciiz (sim, ds, dx, dosname, sizeof (dosname))) {
		return (int21_ret (sim, 1, 0x0001));
	}

	if ((name = sim_get_host_name (sim, dosname)) == NULL) {
		return (int21_ret (sim, 1, 0x0001));
	}

	if (stat (name, &st)) {
		free (name);
		return (int21_ret (sim, 1, 0x0002));
	}

	free (name);

	switch (e86_get_al (&sim->cpu)) {
	case 0:
		if (S_ISDIR (st.st_mode)) {
			val = 0x0010;
		}
		else {
			if (st.st_mode & S_IWUSR) {
				val = 0x0020;
			}
			else {
				val = 0x0021;
			}
		}
		e86_set_cx (&sim->cpu, val);
		break;

	case 1:
		break;

	default:
		return (int21_ret (sim, 1, 0x0001));
	}

	int21_ret (sim, 0, 0x0000);

	return (0);
}

/*
 * 4400: Get device data
 */
static
int int21_fct_4400 (dos_t *sim)
{
	unsigned short val;
	FILE           *fp;

	if ((fp = int21_get_fp (sim, e86_get_bx (&sim->cpu))) == NULL) {
		return (int21_ret (sim, 1, 6));
	}

	if (fp == stdin) {
		val = 0x80d3;
	}
	else if ((fp == stdout) || (fp == stderr)) {
		val = 0x80d3;
	}
	else {
		val = 0;
	}

	e86_set_dx (&sim->cpu, val);

	int21_ret (sim, 0, 0x0000);

	return (0);
}

/*
 * 4401: Set device data
 */
static
int int21_fct_4401 (dos_t *sim)
{
	FILE *fp;

	if ((fp = int21_get_fp (sim, e86_get_bx (&sim->cpu))) == NULL) {
		return (int21_ret (sim, 1, 6));
	}

	return (int21_ret (sim, 0, 0x0000));
}

/*
 * 4408: Is device removable
 */
static
int int21_fct_4408 (dos_t *sim)
{
	return (int21_ret (sim, 0, 0x0001));
}

/*
 * 44: Various
 */
static
int int21_fct_44 (dos_t *sim)
{
	switch (e86_get_al (&sim->cpu)) {
	case 0:
		return (int21_fct_4400 (sim));

	case 1:
		return (int21_fct_4401 (sim));

	case 8:
		return (int21_fct_4408 (sim));
	}

	return (1);
}

/*
 * 45: Dup
 */
static
int int21_fct_45 (dos_t *sim)
{
	int      fd1, fd2;
	unsigned fd;
	FILE     *fp;

	if ((fp = int21_get_fp (sim, e86_get_bx (&sim->cpu))) == NULL) {
		return (int21_ret (sim, 1, 0x0006));
	}

	if ((fd = sim_get_free_fd (sim)) & 0x8000) {
		return (int21_ret (sim, 1, 0x0001));
	}

	fd1 = fileno (fp);
	fd2 = dup (fd1);

	if (fd2 < 0) {
		return (int21_ret (sim, 1, 0x0001));
	}

	sim->file[fd] = fdopen (fd2, "r+b");

	if (sim->file[fd] == NULL) {
		sim->file[fd] = fdopen (fd2, "rb");
	}

	if (sim->file[fd] == NULL) {
		sim->file[fd] = fdopen (fd2, "wb");
	}

	if (sim->file[fd] == NULL) {
		close (fd2);
		return (int21_ret (sim, 1, 0x0001));
	}

	return (int21_ret (sim, 0, fd));
}

/*
 * 47: Get directory
 */
static
int int21_fct_47 (dos_t *sim)
{
	unsigned       drv;
	unsigned short ds, si;

	drv = e86_get_dl (&sim->cpu);
	drv = (drv == 0) ? sim->cur_drive : (drv - 1);

	ds = e86_get_ds (&sim->cpu);
	si = e86_get_si (&sim->cpu);

	sim_set_uint8 (sim, ds, si, '\\');
	sim_set_uint8 (sim, ds, si + 1, 0);

	return (int21_ret (sim, 0, 0x0000));
}

/*
 * 48: Alloc
 */
static
int int21_fct_48 (dos_t *sim)
{
	unsigned short blk, para;

	para = e86_get_bx (&sim->cpu);

	blk = sim_mem_alloc (sim, para, para);

	if (blk == 0) {
		e86_set_bx (&sim->cpu, sim_mem_get_max (sim));
		return (int21_ret (sim, 1, 0x0008));
	}

	int21_ret (sim, 0, blk);

	return (0);
}

/*
 * 49: Free
 */
static
int int21_fct_49 (dos_t *sim)
{
	unsigned short blk;

	blk = e86_get_es (&sim->cpu);

	if (sim_mem_free (sim, blk)) {
		return (int21_ret (sim, 1, 0x0001));
	}

	int21_ret (sim, 0, 0x0000);

	return (0);
}

/*
 * 4A: Resize
 */
static
int int21_fct_4a (dos_t *sim)
{
	unsigned short blk;

	blk = sim_mem_resize (sim, e86_get_es (&sim->cpu), e86_get_bx (&sim->cpu));

	if (blk == 0) {
		e86_set_bx (&sim->cpu, sim_mem_get_max (sim));
		return (int21_ret (sim, 1, 0x0008));
	}

	int21_ret (sim, 0, blk);

	return (0);
}

/*
 * 4C: Exit
 */
static
int int21_fct_4c (dos_t *sim)
{
	exit (e86_get_al (&sim->cpu));
}

/*
 * 4E: Find first
 */
static
int int21_fct_4e (dos_t *sim)
{
	unsigned short ds, dx;
	char           dosname[256];
	char           *name;

	ds = e86_get_ds (&sim->cpu);
	dx = e86_get_dx (&sim->cpu);

	if (sim_get_asciiz (sim, ds, dx, dosname, sizeof (dosname))) {
		return (int21_ret (sim, 1, 0x0001));
	}

	if ((name = sim_get_host_name (sim, dosname)) == NULL) {
		return (int21_ret (sim, 1, 0x0001));
	}

	if (int21_find_start (sim, name, e86_get_cx (&sim->cpu))) {
		free (name);
		return (int21_ret (sim, 1, 0x0001));
	}

	free (name);

	if (int21_find_cont (sim)) {
		int21_ret (sim, 1, 0x0012);
	}
	else {
		int21_ret (sim, 0, 0x0000);
	}

	return (0);
}

/*
 * 4F: Find next
 */
static
int int21_fct_4f (dos_t *sim)
{
	if (sim->search_dir == NULL) {
		return (int21_ret (sim, 1, 0x0001));
	}

	if (int21_find_cont (sim)) {
		int21_ret (sim, 1, 0x0012);
	}
	else {
		int21_ret (sim, 0, 0x0000);
	}

	return (0);
}

/*
 * 51: Get PSP address
 */
static
int int21_fct_51 (dos_t *sim)
{
	e86_set_bx (&sim->cpu, sim->psp);

	return (int21_ret (sim, 0, 0x0000));
}

/*
 * 54: Get verify state
 */
static
int int21_fct_54 (dos_t *sim)
{
	return (int21_ret (sim, 0, 0x0000));
}

/*
 * 56: Rename
 */
static
int int21_fct_56 (dos_t *sim)
{
	unsigned short ds, dx, es, di;
	char           dosname[256];
	char           *src, *dst;

	ds = e86_get_ds (&sim->cpu);
	dx = e86_get_dx (&sim->cpu);
	es = e86_get_es (&sim->cpu);
	di = e86_get_di (&sim->cpu);

	if (sim_get_asciiz (sim, ds, dx, dosname, sizeof (dosname))) {
		return (int21_ret (sim, 1, 0x0001));
	}

	if ((src = sim_get_host_name (sim, dosname)) == NULL) {
		return (int21_ret (sim, 1, 0x0001));
	}

	if (sim_get_asciiz (sim, es, di, dosname, sizeof (dosname))) {
		free (src);
		return (int21_ret (sim, 1, 0x0001));
	}

	if ((dst = sim_get_host_name (sim, dosname)) == NULL) {
		free (src);
		return (int21_ret (sim, 1, 0x0001));
	}

	if (rename (src, dst)) {
		int21_ret (sim, 1, 0x0001);
	}
	else {
		int21_ret (sim, 0, 0x0000);
	}

	free (dst);
	free (src);

	return (0);
}

/*
 * 5700: Get mtime
 */
static
int int21_fct_5700 (dos_t *sim)
{
	unsigned    val;
	FILE        *fp;
	struct stat st;
	struct tm   *tm;

	if ((fp = int21_get_fp (sim, e86_get_bx (&sim->cpu))) == NULL) {
		return (int21_ret (sim, 1, 0x0006));
	}

	if (fstat (fileno (fp), &st)) {
		return (int21_ret (sim, 1, 0x0001));
	}

	tm = localtime (&st.st_mtime);

	val = tm->tm_sec / 2;
	val |= (tm->tm_min) << 5;
	val |= (tm->tm_hour) << 11;
	e86_set_cx (&sim->cpu, val);

	val = tm->tm_mday & 0x3f;
	val |= (tm->tm_mon + 1) << 5;
	val |= (tm->tm_year - 80) << 9;
	e86_set_dx (&sim->cpu, val);

	return (int21_ret (sim, 0, 0x0000));
}

/*
 * 5701: Set mtime
 */
static
int int21_fct_5701 (dos_t *sim)
{
	unsigned short date, time;
	FILE           *fp;
	struct tm      tm;
	struct timeval tv[2];

	if ((fp = int21_get_fp (sim, e86_get_bx (&sim->cpu))) == NULL) {
		return (int21_ret (sim, 1, 0x0006));
	}

	fflush (fp);

	date = e86_get_dx (&sim->cpu);
	time = e86_get_cx (&sim->cpu);

	tm.tm_sec = 2 * (time & 0x1f);
	tm.tm_min = (time >> 5) & 0x3f;
	tm.tm_hour = (time >> 11) & 0x1f;
	tm.tm_mday = (date & 0x1f);
	tm.tm_mon = ((date >> 5) & 0x0f) - 1;
	tm.tm_year = ((date >> 9) & 0x7f) + 80;
	tm.tm_isdst = -1;

	tv[0].tv_sec = mktime (&tm);
	tv[0].tv_usec = 0;
	tv[1].tv_sec = tv[0].tv_sec;
	tv[1].tv_usec = 0;

#ifdef HAVE_FUTIMES
	if (futimes (fileno (fp), tv)) {
		return (int21_ret (sim, 1, 0x0001));
	}
#endif

	int21_ret (sim, 0, 0x0000);

	return (0);
}

/*
 * 57: Get/set mtime
 */
static
int int21_fct_57 (dos_t *sim)
{
	switch (e86_get_al (&sim->cpu)) {
	case 0:
		return (int21_fct_5700 (sim));

	case 1:
		return (int21_fct_5701 (sim));
	}

	return (1);
}

/*
 * 58: Get/set allocation strategy
 */
static
int int21_fct_58 (dos_t *sim)
{
	switch (e86_get_al (&sim->cpu)) {
	case 0:
		int21_ret (sim, 0, 0x0000);
		return (0);

	case 1:
		int21_ret (sim, 0, 0x0000);
		return (int21_fct_5701 (sim));
	}

	int21_ret (sim, 0, 0x0000);

	return (0);
}

int sim_int21 (dos_t *sim)
{
	switch (e86_get_ah (&sim->cpu)) {
	case 0x00:
		return (int21_fct_00 (sim));

	case 0x01:
		return (int21_fct_01 (sim));

	case 0x02:
		return (int21_fct_02 (sim));

	case 0x09:
		return (int21_fct_09 (sim));

	case 0x0a:
		return (int21_fct_0a (sim));

	case 0x0b:
		return (int21_fct_0b (sim));

	case 0x0c:
		return (int21_fct_0c (sim));

	case 0x19:
		return (int21_fct_19 (sim));

	case 0x1a:
		return (int21_fct_1a (sim));

	case 0x25:
		return (int21_fct_25 (sim));

	case 0x2a:
		return (int21_fct_2a (sim));

	case 0x2b:
		return (int21_fct_2b (sim));

	case 0x2c:
		return (int21_fct_2c (sim));

	case 0x2d:
		return (int21_fct_2d (sim));

	case 0x2e:
		return (int21_fct_2e (sim));

	case 0x2f:
		return (int21_fct_2f (sim));

	case 0x30:
		return (int21_fct_30 (sim));

	case 0x33:
		return (int21_fct_33 (sim));

	case 0x35:
		return (int21_fct_35 (sim));

	case 0x36:
		return (int21_fct_36 (sim));

	case 0x37:
		return (int21_fct_37 (sim));

	case 0x38:
		return (int21_fct_38 (sim));

	case 0x3c:
		return (int21_fct_3c (sim));

	case 0x3d:
		return (int21_fct_3d (sim));

	case 0x3e:
		return (int21_fct_3e (sim));

	case 0x3f:
		return (int21_fct_3f (sim));

	case 0x40:
		return (int21_fct_40 (sim));

	case 0x41:
		return (int21_fct_41 (sim));

	case 0x42:
		return (int21_fct_42 (sim));

	case 0x43:
		return (int21_fct_43 (sim));

	case 0x45:
		return (int21_fct_45 (sim));

	case 0x44:
		return (int21_fct_44 (sim));

	case 0x47:
		return (int21_fct_47 (sim));

	case 0x48:
		return (int21_fct_48 (sim));

	case 0x49:
		return (int21_fct_49 (sim));

	case 0x4a:
		return (int21_fct_4a (sim));

	case 0x4c:
		return (int21_fct_4c (sim));

	case 0x4e:
		return (int21_fct_4e (sim));

	case 0x4f:
		return (int21_fct_4f (sim));

	case 0x51:
		return (int21_fct_51 (sim));

	case 0x54:
		return (int21_fct_54 (sim));

	case 0x56:
		return (int21_fct_56 (sim));

	case 0x57:
		return (int21_fct_57 (sim));

	case 0x58:
		return (int21_fct_58 (sim));

	case 0x5a:
	case 0x60:
	case 0x63:
	case 0xe3:
		return (int21_ret (sim, 1, 0x0001));
	}

	return (1);
}
