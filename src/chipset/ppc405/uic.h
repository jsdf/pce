/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/ppc405/uic.h                                     *
 * Created:     2004-02-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2009 Hampa Hug <hampa@hampa.ch>                     *
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


/* PowerPC 405GP UIC */


#ifndef PPC_UIC_H
#define PPC_UIC_H 1


#include <stdint.h>


#define P405UIC_INT_U0   0              /* UART0 */
#define P405UIC_INT_U1   1              /* UART1 */
#define P405UIC_INT_IIC  2              /* IIC */
#define P405UIC_INT_EM   3              /* Extrnal Master */
#define P405UIC_INT_PCI  4              /* PCI ExternalCommand Write */
#define P405UIC_INT_D0   5              /* DMA Channel 0 */
#define P405UIC_INT_D1   6              /* DMA Channel 1 */
#define P405UIC_INT_D2   7              /* DMA Channel 2 */
#define P405UIC_INT_D3   8              /* DMA Channel 3 */
#define P405UIC_INT_EW   9              /* Ethernet Wake Up */
#define P405UIC_INT_MS   10             /* MAL System Error (SERR) */
#define P405UIC_INT_MTE  11             /* MAL TX End of Buffer (TXEOB0) */
#define P405UIC_INT_MRE  12             /* MAL RX End of Buffer (RXEOB) */
#define P405UIC_INT_MTD  13             /* MAL TX Descriptor Error (TXDE) */
#define P405UIC_INT_MRD  14             /* MAL RX Descriptor Error (RXDE) */
#define P405UIC_INT_E    15             /* Ethernet */
#define P405UIC_INT_EPS  16             /* External PCI SERR */
#define P405UIC_INT_EC   17             /* ECC Correctable Error */
#define P405UIC_INT_PPM  18             /* PCI Power Management */
#define P405UIC_INT_EIR0 25             /* External IRQ 0 */
#define P405UIC_INT_EIR1 26             /* External IRQ 1 */
#define P405UIC_INT_EIR2 27             /* External IRQ 2 */
#define P405UIC_INT_EIR3 28             /* External IRQ 3 */
#define P405UIC_INT_EIR4 29             /* External IRQ 4 */
#define P405UIC_INT_EIR5 30             /* External IRQ 5 */
#define P405UIC_INT_EIR6 31             /* External IRQ 6 */

#define P405UIC_VCR_PRO 0x00000001UL


struct p405_uic_s;


typedef struct p405_uic_s {
	uint32_t      sr;
	uint32_t      er;
	uint32_t      msr;
	uint32_t      cr;
	uint32_t      pr;
	uint32_t      tr;
	uint32_t      vcr;
	uint32_t      vr;

	uint32_t      levels;
	uint32_t      invert;
	uint32_t      vr_msk;

	void          (*nint) (void *ext, unsigned char val);
	void          *nint_ext;
	unsigned char nint_val;

	void          (*cint) (void *ext, unsigned char val);
	void          *cint_ext;
	unsigned char cint_val;

	unsigned long intcnt[32];
} p405_uic_t;


void p405uic_init (p405_uic_t *uic);
p405_uic_t *p405uic_new (void);
void p405uic_free (p405_uic_t *uic);
void p405uic_del (p405_uic_t *uic);

/*!***************************************************************************
 * @short Set the UIC input inversion mask
 * @param uic The UIC context struct
 * @param inv The inversion mask
 *****************************************************************************/
void p405uic_set_invert (p405_uic_t *uic, unsigned long inv);

void p405uic_set_cint_fct (p405_uic_t *uic, void *ext, void *fct);
void p405uic_set_nint_fct (p405_uic_t *uic, void *ext, void *fct);

void *p405uic_get_irq_fct (p405_uic_t *uic, unsigned irq);

uint32_t p405uic_get_levels (const p405_uic_t *uic);

uint32_t p405uic_get_sr (p405_uic_t *uic);
void p405uic_set_sr (p405_uic_t *uic, uint32_t val);

uint32_t p405uic_get_er (p405_uic_t *uic);
void p405uic_set_er (p405_uic_t *uic, uint32_t val);

uint32_t p405uic_get_msr (p405_uic_t *uic);

uint32_t p405uic_get_cr (p405_uic_t *uic);
void p405uic_set_cr (p405_uic_t *uic, uint32_t val);

uint32_t p405uic_get_pr (p405_uic_t *uic);
void p405uic_set_pr (p405_uic_t *uic, uint32_t val);

uint32_t p405uic_get_tr (p405_uic_t *uic);
void p405uic_set_tr (p405_uic_t *uic, uint32_t val);

uint32_t p405uic_get_vcr (p405_uic_t *uic);
void p405uic_set_vcr (p405_uic_t *uic, uint32_t val);

uint32_t p405uic_get_vr (p405_uic_t *uic);

unsigned long p405uic_get_int_cnt (p405_uic_t *uic, unsigned i);


void p405uic_set_irq (p405_uic_t *uic, unsigned i, unsigned char val);
void p405uic_set_irq0 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq1 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq2 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq3 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq4 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq5 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq6 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq7 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq8 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq9 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq10 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq11 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq12 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq13 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq14 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq15 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq16 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq17 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq18 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq19 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq20 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq21 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq22 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq23 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq24 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq25 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq26 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq27 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq28 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq29 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq30 (p405_uic_t *uic, unsigned char val);
void p405uic_set_irq31 (p405_uic_t *uic, unsigned char val);


#endif
