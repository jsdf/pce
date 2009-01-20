/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/82xx/e8255.h                                     *
 * Created:     2003-04-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2009 Hampa Hug <hampa@hampa.ch>                     *
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


/* PPI 8255 */
/* only mode 0 is supported */


#ifndef PCE_E8255_H
#define PCE_E8255_H 1


typedef struct {
	unsigned char val_inp;
	unsigned char val_out;

	unsigned char inp;

	void          *read_ext;
	unsigned char (*read) (void *ext);

	void          *write_ext;
	void          (*write) (void *ext, unsigned char val);
} e8255_port_t;


typedef struct {
	unsigned char group_a_mode;
	unsigned char group_b_mode;

	unsigned char mode;

	e8255_port_t  port[3];
} e8255_t;


void e8255_init (e8255_t *ppi);
e8255_t *e8255_new (void);
void e8255_free (e8255_t *ppi);
void e8255_del (e8255_t *ppi);

void e8255_set_inp (e8255_t *ppi, unsigned p, unsigned char val);
void e8255_set_out (e8255_t *ppi, unsigned p, unsigned char val);
unsigned char e8255_get_inp (e8255_t *ppi, unsigned p);
unsigned char e8255_get_out (e8255_t *ppi, unsigned p);

void e8255_set_inp_a (e8255_t *ppi, unsigned char val);
void e8255_set_inp_b (e8255_t *ppi, unsigned char val);
void e8255_set_inp_c (e8255_t *ppi, unsigned char val);

void e8255_set_out_a (e8255_t *ppi, unsigned char val);
void e8255_set_out_b (e8255_t *ppi, unsigned char val);
void e8255_set_out_c (e8255_t *ppi, unsigned char val);

unsigned char e8255_get_inp_a (e8255_t *ppi);
unsigned char e8255_get_inp_b (e8255_t *ppi);
unsigned char e8255_get_inp_c (e8255_t *ppi);

unsigned char e8255_get_out_a (e8255_t *ppi);
unsigned char e8255_get_out_b (e8255_t *ppi);
unsigned char e8255_get_out_c (e8255_t *ppi);

void e8255_set_uint8 (e8255_t *ppi, unsigned long addr, unsigned char val);
void e8255_set_uint16 (e8255_t *ppi, unsigned long addr, unsigned short val);
void e8255_set_uint32 (e8255_t *ppi, unsigned long addr, unsigned long val);

unsigned char e8255_get_uint8 (e8255_t *ppi, unsigned long addr);
unsigned short e8255_get_uint16 (e8255_t *ppi, unsigned long addr);
unsigned long e8255_get_uint32 (e8255_t *ppi, unsigned long addr);


#endif
