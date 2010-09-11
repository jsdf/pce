/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/82xx/e8237.h                                     *
 * Created:     2003-09-11 by Hampa Hug <hampa@hampa.ch>                     *
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


/* DMAC 8237A */


#ifndef PCE_E8237_H
#define PCE_E8237_H 1


#define E8237_CMD_MEMTOMEM 0x01
#define E8237_CMD_ADDRHOLD 0x02
#define E8237_CMD_DISABLE  0x04
#define E8237_CMD_CTIMING  0x08
#define E8237_CMD_ROTPRI   0x10

#define E8237_MODE_TYPE    0x0c
#define E8237_MODE_VERIFY  0x00
#define E8237_MODE_WRITE   0x04
#define E8237_MODE_READ    0x08
#define E8237_MODE_AUTO    0x10
#define E8237_MODE_ADDRDEC 0x20
#define E8237_MODE_MODE    0xc0
#define E8237_MODE_DEMAND  0x00
#define E8237_MODE_SINGLE  0x40
#define E8237_MODE_BLOCK   0x80
#define E8237_MODE_CASCADE 0xc0

#define E8237_STATE_DREQ 0x01
#define E8237_STATE_SREQ 0x02
#define E8237_STATE_PREQ 0x04
#define E8237_STATE_TC   0x08
#define E8237_STATE_MASK 0x10


struct e8237_s;


typedef struct {
	struct e8237_s *dma;

	unsigned short base_addr;
	unsigned short base_cnt;

	unsigned short cur_addr;
	unsigned short cur_cnt;

	unsigned short mode;
	unsigned short state;

	unsigned char  dack_val;
	unsigned char  tc_val;

	struct e8237_s *cascade;

	void           *dack_ext;
	void           (*dack) (void *ext, unsigned char val);

	void           *tc_ext;
	void           (*tc) (void *ext, unsigned char val);

	void           *memwr_ext;
	void           (*memwr) (void *ext, unsigned long addr, unsigned char val);

	void           *memrd_ext;
	unsigned char  (*memrd) (void *ext, unsigned long addr);

	void           *iowr_ext;
	void           (*iowr) (void *ext, unsigned char val);

	void           *iord_ext;
	unsigned char  (*iord) (void *ext);
} e8237_chn_t;


typedef struct e8237_s {
	e8237_chn_t   chn[4];

	unsigned char check;

	unsigned char cmd;
	unsigned char flipflop;
	unsigned      priority;

	unsigned char hreq_val;
	unsigned char hlda_val;

	void          *hreq_ext;
	void          (*hreq) (void *ext, unsigned char val);
} e8237_t;


void e8237_init (e8237_t *dma);
e8237_t *e8237_new (void);

void e8237_free (e8237_t *dma);
void e8237_del (e8237_t *dma);

void e8237_set_dack_fct (e8237_t *dma, unsigned chn, void *ext, void *fct);
void e8237_set_tc_fct (e8237_t *dma, unsigned chn, void *ext, void *fct);

unsigned char e8237_get_command (e8237_t *dma);
unsigned char e8237_get_priority (e8237_t *dma);
unsigned char e8237_get_mode (e8237_t *dma, unsigned i);
unsigned short e8237_get_state (e8237_t *dma, unsigned i);
unsigned short e8237_get_addr (e8237_t *dma, unsigned i);
unsigned short e8237_get_addr_base (e8237_t *dma, unsigned i);
unsigned short e8237_get_cnt (e8237_t *dma, unsigned i);
unsigned short e8237_get_cnt_base (e8237_t *dma, unsigned i);

void e8237_set_hlda (e8237_t *dma, unsigned char val);
void e8237_set_dreq0 (e8237_t *dma, unsigned char val);
void e8237_set_dreq1 (e8237_t *dma, unsigned char val);
void e8237_set_dreq2 (e8237_t *dma, unsigned char val);
void e8237_set_dreq3 (e8237_t *dma, unsigned char val);
void e8237_set_eop0 (e8237_t *dma, unsigned char val);
void e8237_set_eop1 (e8237_t *dma, unsigned char val);
void e8237_set_eop2 (e8237_t *dma, unsigned char val);
void e8237_set_eop3 (e8237_t *dma, unsigned char val);

unsigned char e8237_get_uint8 (e8237_t *dma, unsigned long addr);
unsigned short e8237_get_uint16 (e8237_t *dma, unsigned long addr);
unsigned long e8237_get_uint32 (e8237_t *dma, unsigned long addr);

void e8237_set_uint8 (e8237_t *dma, unsigned long addr, unsigned char val);
void e8237_set_uint16 (e8237_t *dma, unsigned long addr, unsigned short val);
void e8237_set_uint32 (e8237_t *dma, unsigned long addr, unsigned long val);

/*****************************************************************************
 * @short Reset a 8237 DMAC
 *****************************************************************************/
void e8237_reset (e8237_t *dma);

void e8237_clock (e8237_t *dma, unsigned n);


#endif
