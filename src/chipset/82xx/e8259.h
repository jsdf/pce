/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/82xx/e8259.h                                     *
 * Created:     2003-04-21 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2010 Hampa Hug <hampa@hampa.ch>                     *
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


/* PIC 8259A */


#ifndef PCE_E8259_H
#define PCE_E8259_H 1


#define E8259_ICW1_A7   0x80
#define E8259_ICW1_A6   0x40
#define E8259_ICW1_A5   0x20
#define E8259_ICW1_MARK 0x10
#define E8259_ICW1_LTIM 0x08
#define E8259_ICW1_ADI  0x04
#define E8259_ICW1_SNGL 0x02
#define E8259_ICW1_IC4  0x01

#define E8259_ICW4_SFNM 0x10
#define E8259_ICW4_BUF  0x08
#define E8259_ICW4_MS   0x04
#define E8259_ICW4_AEOI 0x02
#define E8259_ICW4_UPM  0x01

#define E8259_OCW2_R    0x80
#define E8259_OCW2_SL   0x40
#define E8259_OCW2_EOI  0x20
#define E8259_OCW2_L    0x07

#define E8259_OCW3_ESMM 0x40
#define E8259_OCW3_SMM  0x20
#define E8259_OCW3_P    0x04
#define E8259_OCW3_RR   0x02
#define E8259_OCW3_RIS  0x01


struct e8259_s;


typedef void (*e8259_irq_f) (struct e8259_s *pic, unsigned char val);


typedef struct e8259_s {
	/* initialization control words */
	unsigned char icw[4];

	/* operation command words */
	unsigned char ocw[3];

	/* interrupt request register */
	unsigned char irr;

	/* interrupt mask register */
	unsigned char imr;

	/* interrupt service register */
	unsigned char isr;

	/* eight irq input states */
	unsigned char irq_inp;

	/* irq0, irq1, ... trigger interrupts base, base + 1, ... */
	unsigned      base;

	/* the next expected icw */
	unsigned      next_icw;

	/* select irr or isr for reading */
	int           read_irr;

	/* highest priority irq */
	unsigned      priority;

	/* rotate on automatic EOI mode */
	int           rot_on_aeoi;

	/* interrupt counts */
	unsigned long irq_cnt[8];

	/* the interrupt function */
	void          *intr_ext;
	void          (*intr) (void *ext, unsigned char val);
	unsigned char intr_val;
} e8259_t;


/*!***************************************************************************
 * @short Initialize a PIC 8259 structure
 * @param pic The PIC 8259 structure
 *****************************************************************************/
void e8259_init (e8259_t *pic);

/*!***************************************************************************
 * @short  Create and initialize a PIC 8259 structure
 * @return A PIC 8259 structure or NULL on error
 *****************************************************************************/
e8259_t *e8259_new (void);

/*!***************************************************************************
 * @short Free the resources used by a PIC 8259 structure
 * @param pic The PIC 8259 structure
 *****************************************************************************/
void e8259_free (e8259_t *pic);

/*!***************************************************************************
 * @short Delete a PIC 8259 structure
 * @param pic The PIC 8259 structure
 *****************************************************************************/
void e8259_del (e8259_t *pic);

/*!***************************************************************************
 * @short  Get an IRQ function
 * @param  pic The PIC 8259 structure
 * @param  irq The IRQ number
 * @return The function that is to be called to trigger IRQ irq.
 *
 * The return value is a pointer to one of the e8259_set_irq?() functions.
 *****************************************************************************/
e8259_irq_f e8259_get_irq_f (e8259_t *pic, unsigned irq);

/*!***************************************************************************
 * @short Set the interrupt function
 * @param pic The PIC 8259 structure
 * @param ext The transparent parameter for the interrupt function
 * @param fct The interrupt function
 *****************************************************************************/
void e8259_set_int_fct (e8259_t *pic, void *ext, void *fct);

/*!***************************************************************************
 * @short Set an IRQ signal
 * @param pic The PIC 8259 structure
 * @param irq The IRQ input number
 * @param val The signal polarity
 *****************************************************************************/
void e8259_set_irq (e8259_t *pic, unsigned irq, unsigned char val);

void e8259_set_irq0 (e8259_t *pic, unsigned char val);
void e8259_set_irq1 (e8259_t *pic, unsigned char val);
void e8259_set_irq2 (e8259_t *pic, unsigned char val);
void e8259_set_irq3 (e8259_t *pic, unsigned char val);
void e8259_set_irq4 (e8259_t *pic, unsigned char val);
void e8259_set_irq5 (e8259_t *pic, unsigned char val);
void e8259_set_irq6 (e8259_t *pic, unsigned char val);
void e8259_set_irq7 (e8259_t *pic, unsigned char val);

/*!***************************************************************************
 * @short  Acknowledge an IRQ
 * @param  pic The PIC 8259 structure
 * @return The IRQ that occurred
 *****************************************************************************/
unsigned char e8259_inta (e8259_t *pic);

unsigned char e8259_get_irr (e8259_t *pic);
unsigned char e8259_get_imr (e8259_t *pic);
unsigned char e8259_get_isr (e8259_t *pic);
unsigned char e8259_get_icw (e8259_t *pic, unsigned i);
unsigned char e8259_get_ocw (e8259_t *pic, unsigned i);

void e8259_set_uint8 (e8259_t *pic, unsigned long addr, unsigned char val);
void e8259_set_uint16 (e8259_t *pic, unsigned long addr, unsigned short val);
void e8259_set_uint32 (e8259_t *pic, unsigned long addr, unsigned long val);

unsigned char e8259_get_uint8 (e8259_t *pic, unsigned long addr);
unsigned short e8259_get_uint16 (e8259_t *pic, unsigned long addr);
unsigned long e8259_get_uint32 (e8259_t *pic, unsigned long addr);

/*!***************************************************************************
 * @short Reset a PIC 8259
 *****************************************************************************/
void e8259_reset (e8259_t *pic);

void e8259_clock (e8259_t *pic);


#endif
