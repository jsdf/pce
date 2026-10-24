/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/parport.h                                        *
 * Created:     2003-04-29 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2019 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DEVICES_PARPORT_H
#define PCE_DEVICES_PARPORT_H 1


#include <stdio.h>

#include <devices/memory.h>

#include <drivers/char/char.h>


#define PARPORT_STR 0x01

#define PARPORT_BSY 0x80
#define PARPORT_ACK 0x40
#define PARPORT_OFF 0x10
#define PARPORT_ERR 0x08


typedef struct {
	mem_blk_t     port;

	unsigned long io;

	unsigned char status;
	unsigned char control;
	unsigned char data;

	void *set_data_ext;
	void (*set_data) (void *ext, unsigned char val);

	void *set_ctrl_ext;
	void (*set_ctrl) (void *ext, unsigned char val);

	void *get_status_ext;
	unsigned char (*get_status) (void *ext);

	char_drv_t    *cdrv;
} parport_t;


void parport_init (parport_t *par, unsigned long base);
parport_t *parport_new (unsigned long base);
void parport_free (parport_t *par);
void parport_del (parport_t *par);

void parport_set_data_fct (parport_t *par, void *ext, void *fct);
void parport_set_ctrl_fct (parport_t *par, void *ext, void *fct);
void parport_set_status_fct (parport_t *par, void *ext, void *fct);

mem_blk_t *parport_get_reg (parport_t *par);

int parport_set_driver (parport_t *par, const char *name);

void parport_set_control (parport_t *par, unsigned char val);

void parport_set_uint8 (parport_t *par, unsigned long addr, unsigned char val);
void parport_set_uint16 (parport_t *par, unsigned long addr, unsigned short val);
unsigned char parport_get_uint8 (parport_t *par, unsigned long addr);
unsigned short parport_get_uint16 (parport_t *par, unsigned long addr);


#endif
