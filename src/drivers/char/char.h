/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/char/char.h                                      *
 * Created:     2009-03-06 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_DRIVERS_CHAR_CHAR_H
#define PCE_DRIVERS_CHAR_CHAR_H 1


#include <stdio.h>


#define PCE_CHAR_DTR 0x0001
#define PCE_CHAR_RTS 0x0002

#define PCE_CHAR_DSR 0x0100
#define PCE_CHAR_CTS 0x0200
#define PCE_CHAR_CD  0x0400
#define PCE_CHAR_RI  0x0800


/*!***************************************************************************
 * @short The character driver context
 *****************************************************************************/
typedef struct char_drv_t {
	void          *ext;

	unsigned long bps;
	unsigned      bpc;
	unsigned      parity;
	unsigned      stop;

	unsigned      ctl_inp;
	unsigned      ctl_out;

	unsigned      log_cnt;
	unsigned char log_buf[16];
	int           log_out;
	FILE          *log_fp;

	void (*close) (struct char_drv_t *cdrv);

	unsigned (*read) (struct char_drv_t *cdrv, void *buf, unsigned cnt);
	unsigned (*write) (struct char_drv_t *cdrv, const void *buf, unsigned cnt);

	int (*get_ctl) (struct char_drv_t *cdrv, unsigned *ctl);
	int (*set_ctl) (struct char_drv_t *cdrv, unsigned ctl);

	int (*set_params) (struct char_drv_t *cdrv,
		unsigned long bps, unsigned bpc, unsigned parity, unsigned stop
	);
} char_drv_t;


void chr_init (char_drv_t *cdrv, void *ext);

void chr_close (char_drv_t *cdrv);

unsigned chr_read (char_drv_t *cdrv, void *buf, unsigned cnt);
unsigned chr_write (char_drv_t *cdrv, const void *buf, unsigned cnt);

int chr_get_ctl (char_drv_t *cdrv, unsigned *ctl);
int chr_set_ctl (char_drv_t *cdrv, unsigned ctl);

int chr_set_params (char_drv_t *cdrv, unsigned long bps, unsigned bpc, unsigned parity, unsigned stop);

int chr_set_log (char_drv_t *cdrv, const char *fname);

char_drv_t *chr_open (const char *name);
char_drv_t *chr_null_open (const char *name);
char_drv_t *chr_posix_open (const char *name);
char_drv_t *chr_ppp_open (const char *name);
char_drv_t *chr_pty_open (const char *name);
char_drv_t *chr_slip_open (const char *name);
char_drv_t *chr_stdio_open (const char *name);
char_drv_t *chr_tcp_open (const char *name);
char_drv_t *chr_tios_open (const char *name);


#endif
