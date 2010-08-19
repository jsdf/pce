/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/82xx/e8253.h                                     *
 * Created:     2001-05-04 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2001-2010 Hampa Hug <hampa@hampa.ch>                     *
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


/* PIT 8253 / 8254 */


#ifndef PCE_E8253_H
#define PCE_E8253_H 1


/*!***************************************************************************
 * @short The PIT 8253 counter structure
 *****************************************************************************/
typedef struct {
	/* counter register */
	unsigned char   cr[2];
	unsigned char   cr_wr;

	/* output latch */
	unsigned char   ol[2];
	unsigned char   ol_rd;
	unsigned char   cnt_rd;

	unsigned char   sr;
	unsigned char   rw;
	unsigned char   mode;
	unsigned char   bcd;

	unsigned char   counting;

	unsigned char   gate;

	void            *out_ext;
	void            (*out) (void *ext, unsigned char val);
	unsigned char   out_val;

	unsigned short val;
} e8253_counter_t;


/*!***************************************************************************
 * @short The PIT 8253 structure
 *****************************************************************************/
typedef struct {
	e8253_counter_t counter[3];
} e8253_t;


/*!***************************************************************************
 * @short Initialize a PIT structure
 * @param pit The PIT structure
 *****************************************************************************/
void e8253_init (e8253_t *pit);

/*!***************************************************************************
 * @short  Create and initialize a PIT structure
 * @return The PIT structure or NULL on error
 *****************************************************************************/
e8253_t *e8253_new (void);

/*!***************************************************************************
 * @short Free the resources used by a PIT structure
 * @param pit The PIT structure
 *****************************************************************************/
void e8253_free (e8253_t *pit);

/*!***************************************************************************
 * @short Delete a PIT structure
 * @param pit The PIT structure
 *****************************************************************************/
void e8253_del (e8253_t *pit);


/*!***************************************************************************
 * @short Set the output function for a PIT counter
 * @param pit  The PIT structure
 * @param cntr The counter index (0 <= cntr <= 2)
 * @param ext  The transparent parameter for the output function
 * @param fct  The function that is called to set the counter output signal
 *****************************************************************************/
void e8253_set_out_fct (e8253_t *pit, unsigned cntr, void *ext, void *fct);


/*!***************************************************************************
 * @short Set gate input for a PIT counter
 * @param pit  The PIT structure
 * @param cntr The counter index (0 <= cntr <= 2)
 * @param val  The new gate value
 *****************************************************************************/
void e8253_set_gate (e8253_t *pit, unsigned cntr, unsigned char val);

void e8253_set_gate0 (e8253_t *pit, unsigned char val);
void e8253_set_gate1 (e8253_t *pit, unsigned char val);
void e8253_set_gate2 (e8253_t *pit, unsigned char val);

unsigned char e8253_get_uint8 (e8253_t *pit, unsigned long addr);
unsigned short e8253_get_uint16 (e8253_t *pit, unsigned long addr);
unsigned long e8253_get_uint32 (e8253_t *pit, unsigned long addr);

void e8253_set_uint8 (e8253_t *pit, unsigned long addr, unsigned char val);
void e8253_set_uint16 (e8253_t *pit, unsigned long addr, unsigned short val);
void e8253_set_uint32 (e8253_t *pit, unsigned long addr, unsigned long val);

/*!***************************************************************************
 * @short Reset a PIT
 *****************************************************************************/
void e8253_reset (e8253_t *pit);

void e8253_clock (e8253_t *pit, unsigned n);



#endif
