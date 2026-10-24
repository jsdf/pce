/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/inidsk.c                                             *
 * Created:     2004-12-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2019 Hampa Hug <hampa@hampa.ch>                     *
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

#include <lib/inidsk.h>
#include <lib/log.h>
#include <lib/path.h>

#include <drivers/block/blkchd.h>
#include <drivers/block/blkcow.h>
#include <drivers/block/blkdosem.h>
#include <drivers/block/blkpart.h>
#include <drivers/block/blkpbi.h>
#include <drivers/block/blkpce.h>
#include <drivers/block/blkpri.h>
#include <drivers/block/blkpsi.h>
#include <drivers/block/blkqed.h>
#include <drivers/block/blkram.h>
#include <drivers/block/blkraw.h>
#include <drivers/pri/pri-img.h>
#include <drivers/psi/psi-img.h>


int dsk_insert (disks_t *dsks, const char *str, int eject)
{
	unsigned i;
	unsigned drv;
	char     buf[16];
	disk_t   *dsk;
	char     *path;

	i = 0;
	while ((i < 16) && (str[i] != 0)) {
		if (str[i] == ':') {
			buf[i] = 0;
			break;
		}

		buf[i] = str[i];

		i += 1;
	}

	if ((i >= 16) || (i == 0) || (str[i] == 0)) {
		return (1);
	}

	drv = strtoul (buf, NULL, 0);
	str = str + i + 1;

	path = pce_path_get (str);

	dsk = dsk_auto_open (path, 0, 0);
	if (dsk == NULL) {
		free (path);
		return (1);
	}

	free (path);

	dsk_set_drive (dsk, drv);

	if (eject) {
		disk_t *old;

		old = dsks_get_disk (dsks, drv);
		if (old != NULL) {
			dsks_rmv_disk (dsks, old);
			dsk_del (old);
		}
	}

	if (dsks_add_disk (dsks, dsk)) {
		dsk_del (dsk);
		return (1);
	}

	return (0);
}

static
int file_exists (const char *name)
{
	FILE *fp;

	if ((fp = fopen (name, "rb")) == NULL) {
		return (0);
	}

	fclose (fp);

	return (1);
}

disk_t *ini_get_cow (ini_sct_t *sct, disk_t *dsk)
{
	disk_t     *cow;
	ini_val_t  *val;
	const char *cname;

	val = NULL;
	while ((val = ini_next_val (sct, val, "cow")) != NULL) {
		cname = ini_val_get_str (val);
		if (cname == NULL) {
			dsk_del (dsk);
			return (NULL);
		}

		if (file_exists (cname) == 0) {
			cow = dsk_create_cow (dsk, cname, 16384);
		}
		else {
			cow = dsk_open_cow (dsk, cname);
		}

		if (cow == NULL) {
			pce_log_tag (MSG_ERR,
				"DISK:", "*** cow failed (drive=%u file=%s)\n",
				dsk_get_drive (dsk), cname
			);

			dsk_del (dsk);

			return (NULL);
		}
		else {
			dsk_set_readonly (cow, 0);

			dsk = cow;
		}

		pce_log_tag (MSG_INF,
			"DISK:", "drive=%u type=cow file=%s\n",
			dsk_get_drive (dsk), cname
		);
	}

	return (dsk);
}

static
void ini_get_vchs (ini_sct_t *sct, disk_t *dsk)
{
	unsigned long vc, vh, vs;

	ini_get_uint32 (sct, "visible_c", &vc, 0);
	ini_get_uint32 (sct, "visible_h", &vh, 0);
	ini_get_uint32 (sct, "visible_s", &vs, 0);

	vc = (vc == 0) ? dsk->c : vc;
	vh = (vh == 0) ? dsk->h : vh;
	vs = (vs == 0) ? dsk->s : vs;

	dsk_set_visible_chs (dsk, vc, vh, vs);

	if ((dsk->c != vc) || (dsk->h != vh) || (dsk->s != vs)) {
		pce_log_tag (MSG_INF,
			"DISK:", "drive=%u vchs=%lu/%lu/%lu\n",
			dsk_get_drive (dsk),
			(unsigned long) dsk->visible_c,
			(unsigned long) dsk->visible_h,
			(unsigned long) dsk->visible_s
		);
	}
}

static
disk_t *ini_get_disk_part (ini_sct_t *sct,
	unsigned long c, unsigned long h, unsigned long s, int ro)
{
	disk_t        *dsk;
	ini_sct_t     *p;
	unsigned long start;
	unsigned long blk_i, blk_n;
	const char    *fname;
	char          *path;

	dsk = dsk_part_open (c, h, s, ro);

	if (dsk == NULL) {
		return (NULL);
	}

	p = NULL;
	while ((p = ini_next_sct (sct, p, "partition")) != NULL) {
		ini_get_uint32 (p, "offset", &start, 0);
		ini_get_uint32 (p, "block_start", &blk_i, 0);
		ini_get_uint32 (p, "block_count", &blk_n, 0);
		ini_get_string (p, "file", &fname, NULL);
		ini_get_bool (p, "readonly", &ro, 0);

		if (fname == NULL) {
			dsk_del (dsk);
			return (NULL);
		}

		path = pce_path_get (fname);

		if (dsk_part_add_partition (dsk, path, start, blk_i, blk_n, ro)) {
			free (path);
			dsk_del (dsk);
			return (NULL);
		}

		free (path);
	}

	return (dsk);
}

int ini_get_disk (ini_sct_t *sct, disk_t **ret)
{
	disk_t        *dsk;
	ini_val_t     *val;
	FILE          *fp;
	unsigned      drive;
	unsigned long c, h, s, n;
	unsigned long ofs;
	int           ro;
	int           optional;
	const char    *type, *fname;
	char          *path;

	ini_get_uint16 (sct, "drive", &drive, 0);
	ini_get_string (sct, "type", &type, "auto");
	ini_get_uint32 (sct, "offset", &ofs, 0);

	ini_get_uint32 (sct, "c", &c, 0);
	ini_get_uint32 (sct, "h", &h, 0);
	ini_get_uint32 (sct, "s", &s, 0);

	if (ini_get_uint32 (sct, "blocks", &n, 0) == 0) {
		;
	}
	else if (ini_get_uint32 (sct, "size", &n, 0) == 0) {
		n = n & ~511UL;
	}
	else if (ini_get_uint32 (sct, "sizek", &n, 0) == 0) {
		n = 2 * n;
	}
	else if (ini_get_uint32 (sct, "sizem", &n, 0) == 0) {
		n = 2048 * n;
	}
	else if (ini_get_uint32 (sct, "sizeg", &n, 0) == 0) {
		n = 2048UL * 1024UL * n;
	}
	else {
		n = 0;
	}

	ini_get_bool (sct, "readonly", &ro, 0);
	ini_get_bool (sct, "optional", &optional, 0);

	val = NULL;
	dsk = NULL;
	path = NULL;

	while ((val = ini_next_val (sct, val, "file")) != NULL) {
		fname = ini_val_get_str (val);

		if (fname == NULL) {
			continue;
		}

		free (path);
		path = pce_path_get (fname);

		if (path == NULL) {
			continue;
		}

		fp = fopen (path, "rb");

		if (fp == NULL) {
			continue;
		}

		fclose (fp);

		if (strcmp (type, "auto") == 0) {
			dsk = dsk_auto_open (path, ofs, ro);
		}
		else if (strcmp (type, "ram") == 0) {
			dsk = dsk_ram_open (path, n, c, h, s, ro);
		}
		else if (strcmp (type, "image") == 0) {
			dsk = dsk_img_open (path, ofs, ro);
		}
		else if (strcmp (type, "dosemu") == 0) {
			dsk = dsk_dosemu_open (path, ro);
		}
		else if (strcmp (type, "pbi") == 0) {
			dsk = dsk_pbi_open (path, ro);
		}
		else if (strcmp (type, "pce") == 0) {
			dsk = dsk_pce_open (path, ro);
		}
		else if (strcmp (type, "qed") == 0) {
			dsk = dsk_qed_open (path, ro);
		}
		else if (strcmp (type, "chd") == 0) {
			dsk = dsk_chd_open (path, ro);
		}
		else if (strcmp (type, "partition") == 0) {
			dsk = ini_get_disk_part (sct, c, h, s, ro);
		}
		else if (strcmp (type, "anadisk") == 0) {
			dsk = dsk_psi_open (path, PSI_FORMAT_ANADISK, ro);
		}
		else if (strcmp (type, "cp2") == 0) {
			dsk = dsk_psi_open (path, PSI_FORMAT_CP2, ro);
		}
		else if (strcmp (type, "dc42") == 0) {
			dsk = dsk_psi_open (path, PSI_FORMAT_DC42, ro);
		}
		else if (strcmp (type, "imagedisk") == 0) {
			dsk = dsk_psi_open (path, PSI_FORMAT_IMD, ro);
		}
		else if (strcmp (type, "imd") == 0) {
			dsk = dsk_psi_open (path, PSI_FORMAT_IMD, ro);
		}
		else if (strcmp (type, "pfdc") == 0) {
			dsk = dsk_psi_open (path, PSI_FORMAT_PFDC, ro);
		}
		else if (strcmp (type, "pfdc-auto") == 0) {
			dsk = dsk_psi_open (path, PSI_FORMAT_NONE, ro);
		}
		else if (strcmp (type, "pri") == 0) {
			dsk = dsk_pri_open (path, PRI_FORMAT_PRI, ro);
		}
		else if (strcmp (type, "psi") == 0) {
			dsk = dsk_psi_open (path, PSI_FORMAT_PSI, ro);
		}
		else if (strcmp (type, "tc") == 0) {
			dsk = dsk_pri_open (path, PRI_FORMAT_TC, ro);
		}
		else if (strcmp (type, "teledisk") == 0) {
			dsk = dsk_psi_open (path, PSI_FORMAT_TD0, ro);
		}
		else if (strcmp (type, "woz") == 0) {
			dsk = dsk_pri_open (path, PRI_FORMAT_WOZ, ro);
		}

		if (dsk != NULL) {
			break;
		}
	}

	if (dsk == NULL) {
		*ret = NULL;

		free (path);

		if (optional == 0) {
			pce_log (MSG_ERR, "*** loading drive 0x%02x failed\n", drive);
			return (1);
		}

		return (0);
	}

	dsk_set_drive (dsk, drive);

	if ((c != 0) || (h != 0) || (s != 0)) {
		dsk_set_geometry (dsk, dsk_get_block_cnt (dsk), c, h, s);
	}

	pce_log_tag (MSG_INF,
		"DISK:", "drive=%u type=%s blocks=%lu chs=%lu/%lu/%lu %s file=%s\n",
		drive, type,
		(unsigned long) dsk->blocks,
		(unsigned long) dsk->c,
		(unsigned long) dsk->h,
		(unsigned long) dsk->s,
		(ro ? "ro" : "rw"),
		(path != NULL) ? path : "<>"
	);

	free (path);

	ini_get_vchs (sct, dsk);

	dsk = ini_get_cow (sct, dsk);

	if (dsk == NULL) {
		*ret = NULL;

		if (optional == 0) {
			pce_log (MSG_ERR,
				"*** loading drive 0x%02x failed (cow)\n",
				drive
			);

			return (1);
		}

		return (0);
	}

	*ret = dsk;

	return (0);
}

disks_t *ini_get_disks (ini_sct_t *ini)
{
	ini_sct_t *sct;
	disk_t    *dsk;
	disks_t   *dsks;
	unsigned  drive;

	dsks = dsks_new();

	sct = NULL;
	while ((sct = ini_next_sct (ini, sct, "disk")) != NULL) {
		ini_get_uint16 (sct, "drive", &drive, 0);

		if (dsks_get_disk (dsks, drive) != NULL) {
			continue;
		}

		if (ini_get_disk (sct, &dsk) == 0) {
			if (dsk != NULL) {
				dsks_add_disk (dsks, dsk);
			}
		}
	}

	return (dsks);
}
