/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/dos/dos.h                                           *
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


#ifndef PCE_DOS_DOS_H
#define PCE_DOS_DOS_H 1


#include <config.h>

#include <cpu/e8086/e8086.h>

#include <dirent.h>


#define DOS_FILES_MAX  32
#define DOS_DRIVES_MAX 26


typedef struct {
	e8086_t        cpu;

	unsigned long  mem_cnt;
	unsigned short mem_start;
	unsigned char  *mem;

	char           log_int;

	unsigned short env;
	unsigned short psp;
	unsigned short dta[2];
	unsigned       cur_drive;

	unsigned       file_cnt;
	FILE           *file[DOS_FILES_MAX];

	unsigned       drive_cnt;
	char           *drive[DOS_DRIVES_MAX];

	DIR            *search_dir;
	unsigned char  search_attr;
	char           *search_dir_name;
	char           search_name[12];
} dos_t;


unsigned char sim_get_uint8 (dos_t *sim, unsigned short seg, unsigned short ofs);
unsigned short sim_get_uint16 (dos_t *sim, unsigned short seg, unsigned short ofs);
void sim_set_uint8 (dos_t *sim, unsigned short seg, unsigned short ofs, unsigned char val);
void sim_set_uint16 (dos_t *sim, unsigned short seg, unsigned short ofs, unsigned short val);
int sim_get_asciiz (dos_t *sim, unsigned short seg, unsigned short ofs, char *dst, unsigned max);

void sim_print_state_cpu (dos_t *sim, FILE *fp);

int sim_set_drive (dos_t *sim, unsigned drive, const char *path);

int sim_init_env (dos_t *sim, const char *prog, const unsigned char *env, unsigned envcnt);
void sim_init_psp (dos_t *sim);
int sim_init_args (dos_t *sim, const char **argv);
int sim_init (dos_t *sim, unsigned kb);
void sim_free (dos_t *sim);

void sim_run (dos_t *sim);


#endif
