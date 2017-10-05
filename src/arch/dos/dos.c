/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/dos/dos.c                                           *
 * Created:     2012-12-30 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2017 Hampa Hug <hampa@hampa.ch>                     *
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
#include "int.h"

#include <cpu/e8086/e8086.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define DEBUG_MEM 0


static
unsigned char sim_get_mem8 (void *ext, unsigned long addr)
{
	dos_t *sim = ext;

#if DEBUG_MEM
	if (addr < 0x60) {
		fprintf (stderr, "get 8: %04lX\n", addr);
	}
#endif

	if (addr < sim->mem_cnt) {
		return (sim->mem[addr]);
	}

	return (0);
}

static
unsigned short sim_get_mem16 (void *ext, unsigned long addr)
{
	unsigned short val;
	dos_t       *sim = ext;

#if DEBUG_MEM
	if (addr < 0x60) {
		fprintf (stderr, "get 16: %04lX\n", addr);
	}
#endif

	if ((addr + 1) < sim->mem_cnt) {
		return (((unsigned) sim->mem[addr + 1] << 8) | sim->mem[addr]);
	}

	val = (sim_get_mem8 (sim, addr + 1) << 8) | sim_get_mem8 (sim, addr);

	return (val);
}

static
void sim_set_mem8 (void *ext, unsigned long addr, unsigned char val)
{
	dos_t *sim = ext;

#if DEBUG_MEM
	if (addr < 0x60) {
		fprintf (stderr, "set 8: %04lX\n", addr);
	}
#endif

	if (addr < sim->mem_cnt) {
		sim->mem[addr] = val;
	}
}

static
void sim_set_mem16 (void *ext, unsigned long addr, unsigned short val)
{
	dos_t *sim = ext;

#if DEBUG_MEM
	if (addr < 0x60) {
		fprintf (stderr, "set 16: %04lX\n", addr);
	}
#endif

	if (addr < sim->mem_cnt) {
		sim->mem[addr] = val & 0xff;
	}

	if ((addr + 1) < sim->mem_cnt) {
		sim->mem[addr + 1] = (val >> 8) & 0xff;
	}
}

static
unsigned char sim_get_port8 (void *ext, unsigned long addr)
{
	if (addr == 0x21) {
		return (0);
	}

	fprintf (stderr, "unknown port8 read: %04lX\n", addr);
	exit (1);

	return (0);
}

static
unsigned short sim_get_port16 (void *ext, unsigned long addr)
{
	fprintf (stderr, "unknown port16 read: %04lX\n", addr);
	exit (1);

	return (0);
}

static
void sim_set_port8 (void *ext, unsigned long addr, unsigned char val)
{
	fprintf (stderr, "unknown port8 write: %04lX <- %02X\n", addr, val);
	exit (1);
}

static
void sim_set_port16 (void *ext, unsigned long addr, unsigned short val)
{
	fprintf (stderr, "unknown port16 write: %04lX <- %04X\n", addr, val);
	exit (1);
}

unsigned char sim_get_uint8 (dos_t *sim, unsigned short seg, unsigned short ofs)
{
	unsigned long addr;

	addr = ((unsigned long) seg << 4) + ofs;

	return (sim_get_mem8 (sim, addr));
}

unsigned short sim_get_uint16 (dos_t *sim, unsigned short seg, unsigned short ofs)
{
	unsigned long addr;

	addr = ((unsigned long) seg << 4) + ofs;

	return (sim_get_mem16 (sim, addr));
}

void sim_set_uint8 (dos_t *sim, unsigned short seg, unsigned short ofs, unsigned char val)
{
	unsigned long addr;

	addr = ((unsigned long) seg << 4) + ofs;

	sim_set_mem8 (sim, addr, val);
}

void sim_set_uint16 (dos_t *sim, unsigned short seg, unsigned short ofs, unsigned short val)
{
	unsigned long addr;

	addr = ((unsigned long) seg << 4) + ofs;

	sim_set_mem16 (sim, addr, val);
}

int sim_get_asciiz (dos_t *sim, unsigned short seg, unsigned short ofs, char *dst, unsigned max)
{
	unsigned i;

	for (i = 0; i < max; i++) {
		if ((dst[i] = sim_get_uint8 (sim, seg, ofs)) == 0) {
			break;
		}

		ofs = (ofs + 1) & 0xffff;
	}

	if (i >= max) {
		return (1);
	}

	dst[i] = 0;

	return (0);
}

void sim_print_state_cpu (dos_t *sim, FILE *fp)
{
	e8086_t *c;

	c = &sim->cpu;

	fprintf (fp,
		"%04X:%04X AX=%04X BX=%04X CX=%04X DX=%04X "
		"BP=%04X SI=%04X:%04X DI=%04X:%04X "
		"SP=%04X:%04X F=%04X",
		e86_get_cs (c), e86_get_ip (c),
		e86_get_ax (c), e86_get_bx (c), e86_get_cx (c), e86_get_dx (c),
		e86_get_bp (c),
		e86_get_ds (c), e86_get_si (c),
		e86_get_es (c), e86_get_di (c),
		e86_get_ss (c), e86_get_sp (c),
		c->flg
	);

	fprintf (fp,
		"[%c%c%c%c%c%c%c%c]",
		e86_get_if (c) ? 'I' : '-',
		e86_get_df (c) ? 'D' : '-',
		e86_get_of (c) ? 'O' : '-',
		e86_get_sf (c) ? 'S' : '-',
		e86_get_zf (c) ? 'Z' : '-',
		e86_get_af (c) ? 'A' : '-',
		e86_get_pf (c) ? 'P' : '-',
		e86_get_cf (c) ? 'C' : '-'
	);

	if (e86_get_halt (c)) {
		fprintf (fp, " HALT=1");
	}

	fputc ('\n', fp);

	fflush (fp);
}

int sim_set_drive (dos_t *sim, unsigned drive, const char *path)
{
	unsigned n;

	if (drive >= sim->drive_cnt) {
		return (1);
	}

	if (sim->drive[drive] != NULL) {
		free (sim->drive[drive]);
	}

	n = strlen (path);

	if ((sim->drive[drive] = malloc (n + 1)) == NULL) {
		return (1);
	}

	strcpy (sim->drive[drive], path);

	return (0);
}

int sim_init_env (dos_t *sim, const char *prog)
{
	unsigned i;
	size_t   n;
	unsigned para;

	n = 4 + strlen (prog);

	para = (n + 15) >> 4;
	para += 1;

	sim->env = sim_mem_alloc (sim, para, para);

	if (sim->env == 0) {
		return (1);
	}

	sim_set_uint16 (sim, sim->env, 0, 0);
	sim_set_uint16 (sim, sim->env, 2, 0x0001);

	i = 0;
	while (prog[i] != 0) {
		sim_set_uint16 (sim, sim->env, 4 + i, prog[i]);
		i += 1;
	}

	return (0);
}

void sim_init_psp (dos_t *sim)
{
	unsigned i;

	for (i = 0; i < 256; i++) {
		sim_set_uint8 (sim, sim->psp, i, 0);
	}

	sim_set_uint16 (sim, sim->psp, 0x0000, 0x20cd);
	sim_set_uint16 (sim, sim->psp, 0x0002, sim->psp + sim_get_uint16 (sim, sim->psp - 1, 3));
	sim_set_uint16 (sim, sim->psp, 0x002c, sim->env);
}

int sim_init_args (dos_t *sim, const char **argv)
{
	unsigned      i;
	unsigned char c;
	int           ispath;
	const char    *s;

	i = 0;

	while (*argv != NULL) {
		s = *argv;

		if (i > 0) {
			sim_set_uint8 (sim, sim->psp, 0x81 + i, ' ');

			if ((i += 1) > 127) {
				return (1);
			}
		}

		ispath = 0;

		while (*s != 0) {
			while (*s == '%') {
				s += 1;

				if (*s == '%') {
					break;
				}

				ispath = !ispath;
			}

			c = *s;

			if (ispath) {
				if (c == '/') {
					c = '\\';
				}
			}

			sim_set_uint8 (sim, sim->psp, 0x81 + i, c);

			s += 1;

			if ((i += 1) > 127) {
				return (1);
			}
		}

		argv += 1;
	}

	sim_set_uint8 (sim, sim->psp, 0x80, i);

	return (0);
}

static
int sim_init_mem (dos_t *sim, unsigned kb)
{
	unsigned i;

	sim->mem_cnt = 1024UL * kb;
	sim->mem = malloc (sim->mem_cnt);

	if (sim->mem == NULL) {
		return (1);
	}

	sim->env = 0;
	sim->psp = 0;

	memset (sim->mem, 0, sim->mem_cnt);

	sim->mem_start = 0x60;

	sim_set_uint8 (sim, sim->mem_start, 0, 0x5a);
	sim_set_uint16 (sim, sim->mem_start, 1, 0);
	sim_set_uint16 (sim, sim->mem_start, 3, (sim->mem_cnt >> 4) - sim->mem_start - 1);

	sim->mem[0x510] = 0xcf;

	for (i = 0; i < 256; i++) {
		sim_set_uint16 (sim, 0, 4 * i + 0, 0x0010);
		sim_set_uint16 (sim, 0, 4 * i + 2, 0x0050);
	}

	sim_set_uint16 (sim, 0x40, 0x13, kb);

	return (0);
}

int sim_init (dos_t *sim, unsigned kb)
{
	unsigned i;

	if (sim_init_mem (sim, kb)) {
		return (1);
	}

	e86_init (&sim->cpu);

	e86_set_prt (&sim->cpu, sim, sim_get_port8, sim_set_port8, sim_get_port16, sim_set_port16);
	e86_set_mem (&sim->cpu, sim, sim_get_mem8, sim_set_mem8, sim_get_mem16, sim_set_mem16);
	e86_set_ram (&sim->cpu, sim->mem, sim->mem_cnt);

	sim->cpu.op_ext = sim;
	sim->cpu.op_int = (void *) sim_int;

	e86_reset (&sim->cpu);

	sim->cpu.state = 0;

	sim->log_int = 0;
	sim->cur_drive = 2;

	sim->file_cnt = DOS_FILES_MAX;

	for (i = 0; i < sim->file_cnt; i++) {
		sim->file[i] = NULL;
	}

	sim->file[0] = stdin;
	sim->file[1] = stdout;
	sim->file[2] = stderr;

	sim->drive_cnt = DOS_DRIVES_MAX;

	for (i = 0; i < sim->drive_cnt; i++) {
		sim->drive[i] = NULL;
	}

	sim_set_drive (sim, 2, ".");

	sim->search_dir = NULL;
	sim->search_dir_name = NULL;

	return (0);
}

void sim_free (dos_t *sim)
{
	free (sim->mem);

	e86_free (&sim->cpu);
}

void sim_run (dos_t *sim)
{
	while (1) {
		e86_clock (&sim->cpu, 64);
	}
}
