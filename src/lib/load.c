/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/load.c                                               *
 * Created:     2004-08-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2022 Hampa Hug <hampa@hampa.ch>                     *
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
#include <stdio.h>
#include <string.h>

#include <libini/libini.h>

#include <lib/log.h>
#include <lib/ihex.h>
#include <lib/mhex.h>
#include <lib/srec.h>
#include <lib/thex.h>
#include <lib/load.h>
#include <lib/path.h>


int pce_load_blk_bin (mem_blk_t *blk, const char *fname)
{
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (1);
	}

	(void) fread (blk->data, 1, blk->size, fp);

	fclose (fp);

	return (0);
}

int pce_load_mem_ihex (memory_t *mem, const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "r")) == NULL) {
		return (1);
	}

	r = ihex_load_fp (fp, mem, (ihex_set_f) mem_set_uint8_rw);

	fclose (fp);

	return (r);
}

int pce_load_mem_mhex (memory_t *mem, const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "r")) == NULL) {
		return (1);
	}

	r = mhex_load_fp (fp, mem, (mhex_set_f) mem_set_uint8_rw);

	fclose (fp);

	return (r);
}

int pce_load_mem_srec (memory_t *mem, const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "r")) == NULL) {
		return (1);
	}

	r = srec_load_fp (fp, mem, (ihex_set_f) mem_set_uint8_rw);

	fclose (fp);

	return (r);
}

int pce_load_mem_thex (memory_t *mem, const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "r")) == NULL) {
		return (1);
	}

	r = thex_load_fp (fp, mem, (thex_set_f) mem_set_uint8_rw);

	fclose (fp);

	return (r);
}

int pce_load_mem_bin (memory_t *mem, const char *fname, unsigned long addr)
{
	int  c;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (1);
	}

	while ((c = fgetc (fp)) != EOF) {
		mem_set_uint8_rw (mem, addr, c & 0xff);
		addr += 1;
	}

	fclose (fp);

	return (0);
}

int pce_load_mem (memory_t *mem, const char *fname, const char *fmt, unsigned long addr)
{
	unsigned   i;
	const char *ext;

	if (fname == NULL) {
		return (1);
	}

	if ((fmt == NULL) || (strcmp (fmt, "auto") == 0)) {
		i = 0;
		ext = NULL;

		while (fname[i] != 0) {
			if (fname[i] == '.') {
				ext = fname + i + 1;
			}
			else if (fname[i] == PCE_DIR_SEP) {
				ext = NULL;
			}

			i += 1;
		}

		if (ext == NULL) {
			return (1);
		}

		if ((strcasecmp (ext, "ihex") == 0) || (strcasecmp (ext, "ihx") == 0)) {
			fmt = "ihex";
		}
		else if (strcasecmp (ext, "mhex") == 0) {
			fmt = "mhex";
		}
		else if (strcasecmp (ext, "srec") == 0) {
			fmt = "srec";
		}
		else if (strcasecmp (ext, "thex") == 0) {
			fmt = "thex";
		}
		else if (strcasecmp (ext, "bin") == 0) {
			fmt = "binary";
		}
		else if (strcasecmp (ext, "rom") == 0) {
			fmt = "binary";
		}
		else {
			return (1);
		}
	}

	if (strcmp (fmt, "binary") == 0) {
		return (pce_load_mem_bin (mem, fname, addr));
	}
	else if (strcmp (fmt, "ihex") == 0) {
		return (pce_load_mem_ihex (mem, fname));
	}
	else if (strcmp (fmt, "mhex") == 0) {
		return (pce_load_mem_mhex (mem, fname));
	}
	else if (strcmp (fmt, "srec") == 0) {
		return (pce_load_mem_srec (mem, fname));
	}
	else if (strcmp (fmt, "thex") == 0) {
		return (pce_load_mem_thex (mem, fname));
	}

	return (1);
}

int pce_load_mem_ini (memory_t *mem, ini_sct_t *ini)
{
	int           r;
	const char    *fmt;
	const char    *fname;
	char          *path;
	unsigned long addr;
	ini_sct_t     *sct;

	r = 0;

	sct = NULL;

	while ((sct = ini_next_sct (ini, sct, "load")) != NULL) {
		ini_get_string (sct, "format", &fmt, "auto");
		ini_get_string (sct, "file", &fname, NULL);

		if (ini_get_uint32 (sct, "address", &addr, 0)) {
			ini_get_uint32 (sct, "base", &addr, 0);
		}

		if (fname != NULL) {
			path = pce_path_get (fname);

			pce_log_tag (MSG_INF, "Load:", "file=%s format=%s\n",
				fname, fmt
			);

			if (pce_load_mem (mem, path, fmt, addr)) {
				r = 1;
				pce_log (MSG_ERR, "*** loading failed (%s)\n",
					path
				);
			}

			free (path);
		}
	}

	return (r);
}
