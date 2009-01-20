/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e6502/e6502.h                                        *
 * Created:     2004-05-02 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_E6502_H
#define PCE_E6502_H 1


#include <stdio.h>


/* CPU flags */
#define E6502_FLG_N 0x80
#define E6502_FLG_V 0x40
#define E6502_FLG_R 0x20
#define E6502_FLG_B 0x10
#define E6502_FLG_D 0x08
#define E6502_FLG_I 0x04
#define E6502_FLG_Z 0x02
#define E6502_FLG_C 0x01


struct e6502_t;


typedef unsigned char (*e6502_get_uint8_f) (void *ext, unsigned long addr);
typedef void (*e6502_set_uint8_f) (void *ext, unsigned long addr, unsigned char val);

typedef void (*e6502_opcode_f) (struct e6502_t *c);


typedef struct e6502_t {
	unsigned           cpu;

	unsigned short     pc;
	unsigned char      a;
	unsigned char      x;
	unsigned char      y;
	unsigned char      s;
	unsigned char      p;

	unsigned short     ea;
	char               ea_page;

	unsigned char      rst_val;
	unsigned char      irq_val;
	unsigned char      nmi_val;
	unsigned char      nmi_pnd;

	void               *mem;
	e6502_get_uint8_f  mem_get_uint8;
	e6502_set_uint8_f  mem_set_uint8;

	unsigned char      *ram;
	unsigned short     ram_lo;
	unsigned short     ram_hi;

	void               *op_ext;
	void               (*op_hook) (void *ext, unsigned char op);
	void               (*op_stat) (void *ext, unsigned char op);
	void               (*op_undef) (void *ext, unsigned char op);

	unsigned char      inst[4];

	e6502_opcode_f     op[256];

	unsigned long      delay;
	unsigned long long clkcnt;
	unsigned long long inscnt;
} e6502_t;


#define e6502_get_pc(c) ((c)->pc)
#define e6502_get_a(c) ((c)->a)
#define e6502_get_x(c) ((c)->x)
#define e6502_get_y(c) ((c)->y)
#define e6502_get_p(c) ((c)->p)
#define e6502_get_s(c) ((c)->s)

#define e6502_set_pc(c, v) do { (c)->pc = (v) & 0xffffU; } while (0)
#define e6502_set_a(c, v) do { (c)->a = (v) & 0xff; } while (0)
#define e6502_set_x(c, v) do { (c)->x = (v) & 0xff; } while (0)
#define e6502_set_y(c, v) do { (c)->y = (v) & 0xff; } while (0)
#define e6502_set_p(c, v) do { (c)->p = (v) & 0xff; } while (0)
#define e6502_set_s(c, v) do { (c)->s = (v) & 0xff; } while (0)

#define e6502_get_nf(c) (((c)->p & E6502_FLG_N) != 0)
#define e6502_get_vf(c) (((c)->p & E6502_FLG_V) != 0)
#define e6502_get_rf(c) (((c)->p & E6502_FLG_R) != 0)
#define e6502_get_bf(c) (((c)->p & E6502_FLG_B) != 0)
#define e6502_get_df(c) (((c)->p & E6502_FLG_D) != 0)
#define e6502_get_if(c) (((c)->p & E6502_FLG_I) != 0)
#define e6502_get_zf(c) (((c)->p & E6502_FLG_Z) != 0)
#define e6502_get_cf(c) (((c)->p & E6502_FLG_C) != 0)

#define e6502_set_flag(c, f, v) \
	do { if (v) (c)->p |= (f); else (c)->p &= ~(f); } while (0)

#define e6502_set_nf(c, v) e6502_set_flag (c, E6502_FLG_N, v)
#define e6502_set_vf(c, v) e6502_set_flag (c, E6502_FLG_V, v)
#define e6502_set_rf(c, v) e6502_set_flag (c, E6502_FLG_R, v)
#define e6502_set_bf(c, v) e6502_set_flag (c, E6502_FLG_B, v)
#define e6502_set_df(c, v) e6502_set_flag (c, E6502_FLG_D, v)
#define e6502_set_if(c, v) e6502_set_flag (c, E6502_FLG_I, v)
#define e6502_set_zf(c, v) e6502_set_flag (c, E6502_FLG_Z, v)
#define e6502_set_cf(c, v) e6502_set_flag (c, E6502_FLG_C, v)


static inline
unsigned char e6502_get_mem8 (e6502_t *c, unsigned short addr)
{
	if ((addr >= c->ram_lo) && (addr <= c->ram_hi)) {
		return (c->ram[addr]);
	}

	return (c->mem_get_uint8 (c->mem, addr));
}

static inline
void e6502_set_mem8 (e6502_t *c, unsigned short addr, unsigned char val)
{
	if ((addr >= c->ram_lo) && (addr <= c->ram_hi)) {
		c->ram[addr] = val;
	}
	else {
		c->mem_set_uint8 (c->mem, addr, val);
	}
}

#define e6502_get_mem16(c, addr) ( \
	(e6502_get_mem8 ((c), (addr) & 0xffffU) & 0xff) | \
	((e6502_get_mem8 ((c), ((addr) + 1) & 0xffffU) & 0xff) << 8) )

#define e6502_set_mem16(c, addr, v) do { \
	e6502_set_mem8 ((c), (addr) & 0xffffU, (v) & 0xff); \
	e6502_set_mem8 ((c), ((addr) + 1) & 0xffffU, ((v) >> 8) & 0xff); \
	} while (0)


/*****************************************************************************
 * @short Initialize a 6502 context
 *****************************************************************************/
void e6502_init (e6502_t *c);

/*****************************************************************************
 * @short Create and initialize a new 6502 context
 *****************************************************************************/
e6502_t *e6502_new (void);

/*****************************************************************************
 * @short Free the resources used by a 6502 context
 *****************************************************************************/
void e6502_free (e6502_t *c);

/*****************************************************************************
 * @short Delete a 6502 context
 *****************************************************************************/
void e6502_del (e6502_t *c);


void e6502_set_ram (e6502_t *c, unsigned char *ram,
	unsigned short lo, unsigned short hi
);

void e6502_set_mem_f (e6502_t *c, void *mem, void *get8, void *set8);


int e6502_get_reg (e6502_t *c, const char *reg, unsigned long *val);
int e6502_set_reg (e6502_t *c, const char *reg, unsigned long val);


/*****************************************************************************
 * @short Get the number of executed clock cycles
 *****************************************************************************/
unsigned long long e6502_get_clock (e6502_t *c);

/*****************************************************************************
 * @short Get the number of executed instructions
 *****************************************************************************/
unsigned long long e6502_get_opcnt (e6502_t *c);

/*****************************************************************************
 * @short Get the current delay
 *****************************************************************************/
unsigned long e6502_get_delay (e6502_t *c);


void e6502_undefined (e6502_t *c);

/*****************************************************************************
 * @short Set the 6502 RST line
 *****************************************************************************/
void e6502_set_reset (e6502_t *c, unsigned char val);

/*****************************************************************************
 * @short Set the 6502 IRQ line
 *****************************************************************************/
void e6502_set_irq (e6502_t *c, unsigned char val);

/*****************************************************************************
 * @short Set the 6502 NMI line
 *****************************************************************************/
void e6502_set_nmi (e6502_t *c, unsigned char val);


/*****************************************************************************
 * @short Reset the 6502
 *****************************************************************************/
void e6502_reset (e6502_t *c);

/*****************************************************************************
 * @short Execute one instruction
 *****************************************************************************/
void e6502_execute (e6502_t *c);

/*****************************************************************************
 * @short Clock the 6502
 * @param c The 6502 context
 * @param n The number of clock cycles
 *****************************************************************************/
void e6502_clock (e6502_t *c, unsigned n);



#define E6502_OPF_BRA 0x0001
#define E6502_OPF_JSR 0x0002
#define E6502_OPF_RTI 0x0004
#define E6502_OPF_RTS 0x0008

typedef struct {
	unsigned       flags;

	unsigned short pc;
	unsigned       dat_n;
	unsigned char  dat[16];

	char           op[64];

	unsigned       arg_n;
	char           arg1[64];
} e6502_disasm_t;


void e6502_disasm (e6502_disasm_t *op, unsigned char *src, unsigned short pc);
void e6502_disasm_mem (e6502_t *c, e6502_disasm_t *op, unsigned short pc);
void e6502_disasm_cur (e6502_t *c, e6502_disasm_t *op);


#endif
