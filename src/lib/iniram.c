/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/iniram.c                                             *
 * Created:     2005-07-24 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2005-2010 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdlib.h>

#include <lib/iniram.h>
#include <lib/load.h>
#include <lib/log.h>
#include <lib/path.h>
#include <libini/libini.h>


int ini_get_ram (memory_t *mem, ini_sct_t *ini, mem_blk_t **addr0)
{
	ini_sct_t     *sct;
	mem_blk_t     *ram;
	const char    *fname;
	char          *path;
	unsigned      val;
	unsigned long base, size;

	if (addr0 != NULL) {
		*addr0 = NULL;
	}

	sct = NULL;
	while ((sct = ini_next_sct (ini, sct, "ram")) != NULL) {
		ini_get_string (sct, "file", &fname, NULL);
		if (ini_get_uint32 (sct, "address", &base, 0)) {
			ini_get_uint32 (sct, "base", &base, 0);
		}

		if (ini_get_uint32 (sct, "sizem", &size, 0) == 0) {
			size *= 1024UL * 1024UL;
		}
		else if (ini_get_uint32 (sct, "sizek", &size, 0) == 0) {
			size *= 1024UL;
		}
		else {
			ini_get_uint32 (sct, "size", &size, 65536);
		}

		ini_get_uint16 (sct, "default", &val, 0);

		path = pce_path_get (fname);

		pce_log_tag (MSG_INF, "RAM:", "addr=0x%08lx size=%lu file=%s\n",
			base, size, (path == NULL) ? "<none>" : path
		);

		ram = mem_blk_new (base, size, 1);
		if (ram == NULL) {
			pce_log (MSG_ERR, "*** memory block creation failed\n");
			free (path);
			return (1);
		}

		mem_blk_clear (ram, val);
		mem_blk_set_readonly (ram, 0);
		mem_add_blk (mem, ram, 1);

		if ((addr0 != NULL) && (base == 0)) {
			*addr0 = ram;
		}

		if (path != NULL) {
			if (pce_load_blk_bin (ram, path)) {
				pce_log (MSG_ERR,
					"*** loading ram failed (%s)\n", path
				);
				free (path);
				return (1);
			}
		}

		free (path);
	}

	return (0);
}

int ini_get_rom (memory_t *mem, ini_sct_t *ini)
{
	ini_sct_t     *sct;
	mem_blk_t     *rom;
	const char    *fname;
	char          *path;
	unsigned      val;
	unsigned long base, size;

	sct = NULL;
	while ((sct = ini_next_sct (ini, sct, "rom")) != NULL) {
		ini_get_string (sct, "file", &fname, NULL);
		if (ini_get_uint32 (sct, "address", &base, 0)) {
			ini_get_uint32 (sct, "base", &base, 0);
		}

		if (ini_get_uint32 (sct, "sizem", &size, 0) == 0) {
			size *= 1024UL * 1024UL;
		}
		else if (ini_get_uint32 (sct, "sizek", &size, 0) == 0) {
			size *= 1024UL;
		}
		else {
			ini_get_uint32 (sct, "size", &size, 65536);
		}

		ini_get_uint16 (sct, "default", &val, 0);

		path = pce_path_get (fname);

		pce_log_tag (MSG_INF, "ROM:", "addr=0x%08lx size=%lu file=%s\n",
			base, size, (path != NULL) ? path : "<none>"
		);

		rom = mem_blk_new (base, size, 1);
		if (rom == NULL) {
			pce_log (MSG_ERR, "*** memory block creation failed\n");
			free (path);
			return (1);
		}

		mem_blk_clear (rom, val);
		mem_blk_set_readonly (rom, 1);
		mem_add_blk (mem, rom, 1);

		if (path != NULL) {
			if (pce_load_blk_bin (rom, path)) {
				pce_log (MSG_ERR,
					"*** loading rom failed (%s)\n", path
				);
				free (path);
				return (1);
			}
		}

		free (path);
	}

	return (0);
}
