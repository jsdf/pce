/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/dos/exec.c                                          *
 * Created:     2012-12-31 by Hampa Hug <hampa@hampa.ch>                     *
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
#include "exec.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static
unsigned short get_uint16_le (const void *buf, unsigned addr)
{
	const unsigned char *tmp = (const unsigned char *) buf + addr;
	unsigned short      ret;

	ret = tmp[1];
	ret = (ret << 8) | tmp[0];

	return (ret);
}

static
int sim_reloc_exe (dos_t *sim, FILE *fp, unsigned base, unsigned para, unsigned relofs, unsigned relcnt)
{
	unsigned       i;
	unsigned short rseg, rofs, val;
	unsigned char  buf[4];

	if (fseek (fp, relofs, SEEK_SET)) {
		return (1);
	}

	for (i = 0; i < relcnt; i++) {
		if (fread (buf, 4, 1, fp) != 1) {
			return (1);
		}

		rofs = ((unsigned) buf[1] << 8) | buf[0];
		rseg = ((unsigned) buf[3] << 8) | buf[2];

		val = sim_get_uint16 (sim, base + rseg, rofs);
		val += base;
		sim_set_uint16 (sim, base + rseg, rofs, val);
	}

	return (0);
}

static
int sim_exec_exe (dos_t *sim, FILE *fp)
{
	unsigned short para;
	unsigned short size1, size2;
	unsigned long  size;
	unsigned long  imgsize, imgbase;
	unsigned short relocofs, reloccnt;
	unsigned short memmin, memmax;
	unsigned long  para_min, para_max;
	unsigned short hsize;
	unsigned short cs, ip, ss, sp;
	unsigned char  hdr[32];

	if (fread (hdr, 1, 32, fp) != 32) {
		return (1);
	}

	if ((hdr[0] != 'M') || (hdr[1] != 'Z')) {
		return (1);
	}

	size2 = get_uint16_le (hdr, 0x02);
	size1 = get_uint16_le (hdr, 0x04);
	reloccnt = get_uint16_le (hdr, 0x06);
	hsize = get_uint16_le (hdr, 0x08);
	memmin = get_uint16_le (hdr, 0x0a);
	memmax = get_uint16_le (hdr, 0x0c);
	ss = get_uint16_le (hdr, 0x0e);
	sp = get_uint16_le (hdr, 0x10);
	ip = get_uint16_le (hdr, 0x14);
	cs = get_uint16_le (hdr, 0x16);
	relocofs = get_uint16_le (hdr, 0x18);

	size = 512UL * size1 - ((512 - size2) & 511);
	para = (size + 15) >> 4;
	imgbase = 16UL * hsize;
	imgsize = size - imgbase;

	para_min = para + 16UL + memmin;
	para_max = para + 16UL + memmax;

	if (para_min > 0xffff) {
		para_min = 0xffff;
	}

	if (para_max > 0xffff) {
		para_max = 0xffff;
	}

	sim->psp = sim_mem_alloc (sim, para_min, para_max);

	if (sim->psp == 0) {
		return (1);
	}

	if (fseek (fp, imgbase, SEEK_SET)) {
		return (1);
	}

	if (fread (sim->mem + 16UL * sim->psp + 256, 1, imgsize, fp) != imgsize) {
		return (1);
	}

	if (reloccnt > 0) {
		if (sim_reloc_exe (sim, fp, sim->psp + 16, para, relocofs, reloccnt)) {
			return (1);
		}
	}

	cs += sim->psp + 16;
	ss += sim->psp + 16;

	e86_set_cs (&sim->cpu, cs);
	e86_set_ip (&sim->cpu, ip);
	e86_set_ss (&sim->cpu, ss);
	e86_set_sp (&sim->cpu, sp);

	e86_set_ds (&sim->cpu, sim->psp);
	e86_set_es (&sim->cpu, sim->psp);

	return (0);
}

static
int sim_exec_com (dos_t *sim, FILE *fp)
{
	unsigned long  size;
	unsigned short sp;

	if (fseek (fp, 0, SEEK_END)) {
		return (1);
	}

	size = ftell (fp);

	rewind (fp);

	if (size > (65536 - 512)) {
		return (1);
	}

	sim->psp = sim_mem_alloc (sim, (size + 512 + 15) / 16, 0xffff);

	if (sim->psp == 0) {
		return (1);
	}

	if (fread (sim->mem + 16UL * sim->psp + 256, 1, size, fp) != size) {
		return (1);
	}

	sp = sim_mem_get_size (sim, sim->psp);
	sp = (sp < 0x1000) ? (sp << 4) : 0;
	sp = (sp - 2) & 0xffff;
	sim_set_uint16 (sim, sim->psp, sp, 0);

	e86_set_cs (&sim->cpu, sim->psp);
	e86_set_ip (&sim->cpu, 0x0100);
	e86_set_ss (&sim->cpu, sim->psp);
	e86_set_sp (&sim->cpu, sp);

	e86_set_ds (&sim->cpu, sim->psp);
	e86_set_es (&sim->cpu, sim->psp);

	return (0);
}

int sim_exec (dos_t *sim, const char *name)
{
	int           r;
	unsigned      magic;
	FILE          *fp;
	unsigned char buf[16];

	if ((fp = fopen (name, "rb")) == NULL) {
		return (1);
	}

	if (fread (buf, 1, 16, fp) != 16) {
		fclose (fp);
		return (1);
	}

	rewind (fp);

	e86_set_ax (&sim->cpu, 0);
	e86_set_bx (&sim->cpu, 0);
	e86_set_cx (&sim->cpu, 0);
	e86_set_dx (&sim->cpu, 0);
	e86_set_si (&sim->cpu, 0);
	e86_set_di (&sim->cpu, 0);
	e86_set_bp (&sim->cpu, 0);

	magic = get_uint16_le (buf, 0);

	if ((magic == 0x5a4d) || (magic == 0x4d5a)) {
		r = sim_exec_exe (sim, fp);
	}
	else {
		r = sim_exec_com (sim, fp);
	}

	fclose (fp);

	if (r) {
		return (1);
	}

	sim_init_psp (sim);

	sim->dta[0] = 0x0080;
	sim->dta[1] = sim->psp;

	return (0);
}
