/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/msgdsk.c                                             *
 * Created:     2019-06-22 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2019 Hampa Hug <hampa@hampa.ch>                          *
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


#include <stdio.h>
#include <stdlib.h>

#include <drivers/block/block.h>
#include <drivers/block/blkpbi.h>
#include <drivers/block/blkqed.h>

#include <lib/console.h>
#include <lib/log.h>
#include <lib/msg.h>
#include <lib/msgdsk.h>
#include <lib/path.h>


void msg_dsk_print_help (void)
{
	pce_puts (
		"disk.commit          [<id>]\n"
		"disk.eject           [<id>]\n"
		"disk.id              [<id>]\n"
		"disk.insert          <filename>\n"
		"disk.ro              [<id>]\n"
		"disk.rw              [<id>]\n"
	);
}

void dsk_print_info (disk_t *dsk)
{
	unsigned p;

	pce_printf ("ID=%-3u T=%-2u BLK=%-8lu",
		dsk->drive, dsk->type, dsk_get_block_cnt (dsk)
	);

	p = pce_printf ("CHS=%u/%u/%u",
		(unsigned long) dsk->c,
		(unsigned long) dsk->h,
		(unsigned long) dsk->s
	);

	while (p++ < 15) {
		pce_putc (' ');
	}

	pce_printf ("%-2s FILE=%s\n",
		dsk_get_readonly (dsk) ? "RO" : "RW",
		dsk_get_fname (dsk)
	);
}

void dsks_print_info (disks_t *dsks)
{
	unsigned   i;
	disk_t     *dsk;
	disk_pbi_t *pbi;
	disk_qed_t *qed;

	for (i = 0; i < dsks->cnt; i++) {
		dsk = dsks->dsk[i];

		while (dsk != NULL) {
			dsk_print_info (dsk);

			if (dsk->type == PCE_DISK_PBI) {
				pbi = dsk->ext;
				dsk = pbi->next;
			}
			else if (dsk->type == PCE_DISK_QED) {
				qed = dsk->ext;
				dsk = qed->next;
			}
			else {
				dsk = NULL;
			}
		}
	}
}

int msg_dsk_get_disk_id (const char *val, unsigned *id)
{
	unsigned tmp;

	if ((val == NULL) || (*val == 0)) {
		return (0);
	}

	if (msg_get_uint (val, &tmp)) {
		pce_log (MSG_ERR, "*** bad disk id (%s)\n", val);
		return (1);
	}

	*id = tmp;

	return (0);
}

int msg_dsk_emu_disk_commit (const char *val, disks_t *dsks, unsigned *id)
{
	if (msg_dsk_get_disk_id (val, id)) {
		return (1);
	}

	pce_log (MSG_INF, "commiting disk %u\n", *id);

	if (dsks_set_msg (dsks, *id, "commit", NULL)) {
		pce_log (MSG_ERR, "*** disk %u commit error\n", *id);
		return (1);
	}

	return (0);
}

int msg_dsk_emu_disk_eject (const char *val, disks_t *dsks, unsigned *id)
{
	disk_t *dsk;

	if (msg_dsk_get_disk_id (val, id)) {
		return (1);
	}

	dsk = dsks_get_disk (dsks, *id);

	if (dsk == NULL) {
		pce_log (MSG_ERR,
			"*** disk %u eject error: no such disk\n", *id
		);
	}
	else {
		pce_log (MSG_INF, "ejecting disk %u\n", *id);

		dsks_rmv_disk (dsks, dsk);
		dsk_del (dsk);
	}

	return (0);
}

int msg_dsk_emu_disk_id (const char *val, unsigned *id)
{
	if ((val == NULL) || (*val == 0)) {
		pce_log (MSG_INF, "current disk id is %u\n", *id);
		return (0);
	}

	if (msg_dsk_get_disk_id (val, id)) {
		return (1);
	}

	return (0);
}

int msg_dsk_emu_disk_insert (const char *val, disks_t *dsks, unsigned id)
{
	disk_t *dsk, *old;
	char   *path;

	if ((old = dsks_get_disk (dsks, id)) != NULL) {
		pce_log (MSG_INF, "ejecting disk %u\n", id);
		dsks_rmv_disk (dsks, old);
		dsk_del (old);
	}

	path = pce_path_get (val);

	pce_log (MSG_INF, "inserting disk %u (%s)\n", id, path);

	if ((dsk = dsk_auto_open (path, 0, 0)) == NULL) {
		pce_log (MSG_ERR,
			"*** disk %u insert error: failed to open image (%s)\n",
			id, path
		);
		free (path);
		return (1);
	}

	dsk_set_drive (dsk, id);

	free (path);

	if (dsks_add_disk (dsks, dsk)) {
		dsk_del (dsk);
		return (1);
	}

	return (0);
}

int msg_dsk_emu_disk_ro (const char *val, disks_t *dsks, unsigned *id)
{
	disk_t *dsk;

	if (msg_dsk_get_disk_id (val, id)) {
		return (1);
	}

	dsk = dsks_get_disk (dsks, *id);

	if (dsk != NULL) {
		pce_log (MSG_INF, "setting disk %u to read-only\n", *id);

		dsk_set_readonly (dsk, 1);
	}

	return (0);
}

int msg_dsk_emu_disk_rw (const char *val, disks_t *dsks, unsigned *id)
{
	disk_t *dsk;

	if (msg_dsk_get_disk_id (val, id)) {
		return (1);
	}

	dsk = dsks_get_disk (dsks, *id);

	if (dsk != NULL) {
		pce_log (MSG_INF, "setting disk %u to read/write\n", *id);

		dsk_set_readonly (dsk, 0);
	}

	return (0);
}

int msg_dsk_set_msg (const char *msg, const char *val, disks_t *dsks, unsigned *id)
{
	if (msg_is_message ("disk.commit", msg)) {
		return (msg_dsk_emu_disk_commit (val, dsks, id));
	}
	else if (msg_is_message ("disk.eject", msg)) {
		return (msg_dsk_emu_disk_eject (val, dsks, id));
	}
	else if (msg_is_message ("disk.id", msg)) {
		return (msg_dsk_emu_disk_id (val, id));
	}
	else if (msg_is_message ("disk.insert", msg)) {
		return (msg_dsk_emu_disk_insert (val, dsks, *id));
	}
	else if (msg_is_message ("disk.ro", msg)) {
		return (msg_dsk_emu_disk_ro (val, dsks, id));
	}
	else if (msg_is_message ("disk.rw", msg)) {
		return (msg_dsk_emu_disk_rw (val, dsks, id));
	}

	return (-1);
}
