/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/spectrum/snapshot.c                                 *
 * Created:     2022-02-08 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2022-2024 Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"
#include "snapshot.h"
#include "spectrum.h"

#include <stdlib.h>
#include <string.h>

#include <devices/memory.h>

#include <lib/ciff.h>
#include <lib/endian.h>
#include <lib/log.h>


#define SNAP_CKID_SNAP 0x534e4150
#define SNAP_CKID_RAM  0x52414d20
#define SNAP_CKID_ROM  0x524f4d20
#define SNAP_CKID_SPEC 0x53504543
#define SNAP_CKID_Z80A 0x5a383041
#define SNAP_CKID_END  0x454e4420

#define SNAP_MODEL_SPECTRUM_16 0
#define SNAP_MODEL_SPECTRUM_48 1


static
int snap_load_snap (spectrum_t *sim, ciff_t *ciff)
{
	unsigned long version;
	unsigned char buf[4];

	if (ciff_read (ciff, buf, 4)) {
		return (1);
	}

	version = get_uint32_be (buf, 0);

	if (version != 0) {
		fprintf (stderr, "snap: bad version (%lu)\n", version);
		return (1);
	}

	return (0);
}

static
int snap_load_spec (spectrum_t *sim, ciff_t *ciff)
{
	unsigned char buf[4];

	if (ciff_read (ciff, buf, 4)) {
		return (1);
	}

	if (buf[0] == SNAP_MODEL_SPECTRUM_16) {
		;
	}
	else if (buf[0] == SNAP_MODEL_SPECTRUM_48) {
		;
	}
	else {
		fprintf (stderr, "snap: bad hardware (%u)\n", buf[0]);
		return (1);
	}

	spec_set_port_fe (sim, buf[3]);

	return (0);
}

static
int snap_load_z80a (spectrum_t *sim, ciff_t *ciff)
{
	unsigned char buf[32];

	if (ciff_read (ciff, buf, 30)) {
		return (1);
	}

	e8080_set_pc (sim->cpu, get_uint16_le (buf, 0));
	e8080_set_sp (sim->cpu, get_uint16_le (buf, 2));

	e8080_set_c (sim->cpu, buf[4]);
	e8080_set_b (sim->cpu, buf[5]);
	e8080_set_e (sim->cpu, buf[6]);
	e8080_set_d (sim->cpu, buf[7]);
	e8080_set_l (sim->cpu, buf[8]);
	e8080_set_h (sim->cpu, buf[9]);
	e8080_set_psw (sim->cpu, buf[10]);
	e8080_set_a (sim->cpu, buf[11]);

	e8080_set_c2 (sim->cpu, buf[12]);
	e8080_set_b2 (sim->cpu, buf[13]);
	e8080_set_e2 (sim->cpu, buf[14]);
	e8080_set_d2 (sim->cpu, buf[15]);
	e8080_set_l2 (sim->cpu, buf[16]);
	e8080_set_h2 (sim->cpu, buf[17]);
	e8080_set_psw2 (sim->cpu, buf[18]);
	e8080_set_a2 (sim->cpu, buf[19]);

	e8080_set_ixl (sim->cpu, buf[20]);
	e8080_set_ixh (sim->cpu, buf[21]);
	e8080_set_iyl (sim->cpu, buf[22]);
	e8080_set_iyh (sim->cpu, buf[23]);
	e8080_set_r (sim->cpu, buf[24]);
	e8080_set_i (sim->cpu, buf[25]);

	e8080_set_iff1 (sim->cpu, buf[26]);
	e8080_set_iff2 (sim->cpu, buf[27]);
	e8080_set_im (sim->cpu, buf[28]);
	e8080_set_halt (sim->cpu, buf[29]);

	return (0);
}

static
int snap_load_ram (spectrum_t *sim, ciff_t *ciff, int rom)
{
	unsigned long addr;
	unsigned char *mem;
	unsigned char buf[4];

	if (ciff_read (ciff, buf, 4)) {
		return (1);
	}

	addr = get_uint32_be (buf, 0);

	if (ciff->size == 0) {
		return (0);
	}

	if ((mem = mem_get_ptr (sim->mem, addr, ciff->size)) == NULL) {
		fprintf (stderr, "snap: no ram (%04lX + %04lX)\n",
			addr, (unsigned long) ciff->size
		);
		return (1);
	}

	if (ciff_read (ciff, mem, ciff->size)) {
		return (1);
	}

	return (0);
}

static
int snap_load_ciff (spectrum_t *sim, ciff_t *ciff)
{
	if (ciff_read_id (ciff)) {
		return (1);
	}

	if (ciff->ckid != SNAP_CKID_SNAP) {
		return (1);
	}

	if (snap_load_snap (sim, ciff)) {
		return (1);
	}

	while (ciff_read_id (ciff) == 0) {
		switch (ciff->ckid) {
		case SNAP_CKID_END:
			if (ciff_read_crc (ciff)) {
				return (1);
			}
			return (0);

		case SNAP_CKID_SPEC:
			if (snap_load_spec (sim, ciff)) {
				return (1);
			}
			break;

		case SNAP_CKID_Z80A:
			if (snap_load_z80a (sim, ciff)) {
				return (1);
			}
			break;

		case SNAP_CKID_RAM:
			if (snap_load_ram (sim, ciff, 0)) {
				return (1);
			}
			break;

		case SNAP_CKID_ROM:
			if (snap_load_ram (sim, ciff, 1)) {
				return (1);
			}
			break;
		}
	}

	return (1);
}

int spec_snap_load_fp (spectrum_t *sim, FILE *fp)
{
	int    r;
	ciff_t ciff;

	ciff_init (&ciff, fp, 1);

	r = snap_load_ciff (sim, &ciff);

	if (ciff.crc_error) {
		fprintf (stderr, "snap: crc error\n");
	}

	ciff_free (&ciff);

	return (r);
}


int spec_snap_load (spectrum_t *sim, const char *fname)
{
	int  r;
	FILE *fp;

	pce_log_inf ("Loading snapshot '%s'\n", fname);

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (1);
	}

	r = spec_snap_load_fp (sim, fp);

	fclose (fp);

	if (r) {
		pce_log_err ("*** error loading snapshot (%s)\n", fname);
	}

	return (r);
}

static
int snap_save_snap (spectrum_t *sim, ciff_t *ciff)
{
	unsigned char buf[4];

	set_uint32_be (buf, 0, 0);

	if (ciff_write_chunk (ciff, SNAP_CKID_SNAP, buf, 4)) {
		return (1);
	}

	return (0);
}

static
int snap_save_spec (spectrum_t *sim, ciff_t *ciff)
{
	unsigned char buf[4];

	if (sim->model == SPEC_MODEL_16) {
		buf[0] = SNAP_MODEL_SPECTRUM_16;
	}
	else {
		buf[0] = SNAP_MODEL_SPECTRUM_48;
	}

	buf[1] = 0;
	buf[2] = 0;
	buf[3] = sim->port_fe;

	if (ciff_write_chunk (ciff, SNAP_CKID_SPEC, buf, 4)) {
		return (1);
	}

	return (0);
}

static
int snap_save_z80a (spectrum_t *sim, ciff_t *ciff)
{
	unsigned      pc;
	unsigned char buf[64];

	pc = e8080_get_pc (sim->cpu);

	set_uint16_le (buf, 0, pc);
	set_uint16_le (buf, 2, e8080_get_sp (sim->cpu));
	buf[4] = e8080_get_c (sim->cpu);
	buf[5] = e8080_get_b (sim->cpu);
	buf[6] = e8080_get_e (sim->cpu);
	buf[7] = e8080_get_d (sim->cpu);
	buf[8] = e8080_get_l (sim->cpu);
	buf[9] = e8080_get_h (sim->cpu);
	buf[10] = e8080_get_psw (sim->cpu);
	buf[11] = e8080_get_a (sim->cpu);

	buf[12] = e8080_get_c2 (sim->cpu);
	buf[13] = e8080_get_b2 (sim->cpu);
	buf[14] = e8080_get_e2 (sim->cpu);
	buf[15] = e8080_get_d2 (sim->cpu);
	buf[16] = e8080_get_l2 (sim->cpu);
	buf[17] = e8080_get_h2 (sim->cpu);
	buf[18] = e8080_get_psw2 (sim->cpu);
	buf[19] = e8080_get_a2 (sim->cpu);

	buf[20] = e8080_get_ixl (sim->cpu);
	buf[21] = e8080_get_ixh (sim->cpu);
	buf[22] = e8080_get_iyl (sim->cpu);
	buf[23] = e8080_get_iyh (sim->cpu);
	buf[24] = e8080_get_r (sim->cpu);
	buf[25] = e8080_get_i (sim->cpu);
	buf[26] = e8080_get_iff1 (sim->cpu);
	buf[27] = e8080_get_iff2 (sim->cpu);
	buf[28] = e8080_get_im (sim->cpu);
	buf[29] = e8080_get_halt (sim->cpu);

	if (ciff_write_chunk (ciff, SNAP_CKID_Z80A, buf, 30)) {
		return (1);
	}

	return (0);
}

static
int snap_save_ciff (spectrum_t *sim, ciff_t *ciff)
{
	unsigned            i, addr;
	unsigned char       buf[256];
	const unsigned char *mem;

	if (snap_save_snap (sim, ciff)) {
		return (1);
	}

	if (snap_save_spec (sim, ciff)) {
		return (1);
	}

	if (snap_save_z80a (sim, ciff)) {
		return (1);
	}

	for (i = 1; i < 4; i++) {
		addr = 16384 * i;

		if ((mem = mem_get_ptr (sim->mem, addr, 0x4000)) == NULL) {
			continue;
		}

		set_uint32_be (buf, 0, addr);

		if (ciff_write_id (ciff, SNAP_CKID_RAM, 16388)) {
			return (1);
		}

		if (ciff_write (ciff, buf, 4)) {
			return (1);
		}

		if (ciff_write (ciff, mem, 16384)) {
			return (1);
		}

		if (ciff_write_crc (ciff)) {
			return (1);
		}
	}

	if (ciff_write_chunk (ciff, SNAP_CKID_END, NULL, 0)) {
		return (1);
	}

	return (0);
}

int spec_snap_save_fp (spectrum_t *sim, FILE *fp)
{
	int    r;
	ciff_t ciff;

	ciff_init (&ciff, fp, 1);

	r = snap_save_ciff (sim, &ciff);

	ciff_free (&ciff);

	return (r);
}

int spec_snap_save (spectrum_t *sim, const char *fname)
{
	int  r;
	FILE *fp;

	pce_log_inf ("Saving snapshot '%s'\n", fname);

	if ((fp = fopen (fname, "wb")) == NULL) {
		return (1);
	}

	r = spec_snap_save_fp (sim, fp);

	fclose (fp);

	if (r) {
		pce_log_err ("*** error saving snapshot '%s'\n", fname);
	}

	return (r);
}

int spec_snapshot (spectrum_t *sim)
{
	char name[256];

	sprintf (name, "pce%04u.snap", sim->snap_count++);

	if (spec_snap_save (sim, name)) {
		return (1);
	}

	return (0);
}
