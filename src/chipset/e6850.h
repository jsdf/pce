/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/e6850.h                                          *
 * Created:     2013-05-31 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013 Hampa Hug <hampa@hampa.ch>                          *
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


/* ACIA 6850 */


#ifndef PCE_CHIPSET_E6850_H
#define PCE_CHIPSET_E6850_H 1


typedef struct {
	unsigned char cr;
	unsigned char sr;
	unsigned char rdr;
	unsigned char tdr;
	unsigned char rsr;
	unsigned char tsr;

	unsigned char clock_div;
	unsigned char data_bits;
	unsigned char stop_bits;
	unsigned char char_bits;

	unsigned long recv_timer;
	unsigned long send_timer;

	void          *send_ext;
	void          (*send_fct) (void *ext, unsigned char val);

	void          *recv_ext;
	int           (*recv_fct) (void *ext, unsigned char *val);

	unsigned char irq_val;
	void          *irq_ext;
	void          (*irq_fct) (void *ext, unsigned char val);
} e6850_t;


void e6850_init (e6850_t *acia);
void e6850_free (e6850_t *acia);

void e6850_set_irq_fct (e6850_t *acia, void *ext, void *fct);
void e6850_set_send_fct (e6850_t *acia, void *ext, void *fct);
void e6850_set_recv_fct (e6850_t *acia, void *ext, void *fct);

unsigned char e6850_get_status (e6850_t *ucia);
unsigned char e6850_get_data (e6850_t *ucia);

void e6850_set_control (e6850_t *ucia, unsigned char val);
void e6850_set_data (e6850_t *ucia, unsigned char val);

unsigned char e6850_get_uint8 (e6850_t *ucia, unsigned long addr);
unsigned short e6850_get_uint16 (e6850_t *ucia, unsigned long addr);
unsigned long e6850_get_uint32 (e6850_t *ucia, unsigned long addr);

void e6850_set_uint8 (e6850_t *ucia, unsigned long addr, unsigned char val);
void e6850_set_uint16 (e6850_t *ucia, unsigned long addr, unsigned short val);
void e6850_set_uint32 (e6850_t *ucia, unsigned long addr, unsigned long val);

void e6850_receive (e6850_t *acia, unsigned char val);

void e6850_reset (e6850_t *ucia);

void e6850_clock (e6850_t *ucia, unsigned cnt);


#endif
