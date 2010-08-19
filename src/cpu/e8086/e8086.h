/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8086/e8086.h                                        *
 * Created:     1996-04-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 1996-2010 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_E8086_H
#define PCE_E8086_H 1


#include <stdio.h>


/* CPU options */
#define E86_CPU_REP_BUG    0x01         /* enable rep/seg bug */
#define E86_CPU_MASK_SHIFT 0x02         /* mask shift count */
#define E86_CPU_PUSH_FIRST 0x04         /* push sp before decrementing it */
#define E86_CPU_INT6       0x08         /* throw illegal opcode exception */
#define E86_CPU_INT7       0x10		/* throw escape opcode exception */
#define E86_CPU_FLAGS286   0x20         /* Allow clearing flags 12-15 */
#define E86_CPU_8BIT       0x40		/* 16 bit accesses take more time */

/* CPU flags */
#define E86_FLG_C 0x0001
#define E86_FLG_P 0x0004
#define E86_FLG_A 0x0010
#define E86_FLG_Z 0x0040
#define E86_FLG_S 0x0080
#define E86_FLG_T 0x0100
#define E86_FLG_I 0x0200
#define E86_FLG_D 0x0400
#define E86_FLG_O 0x0800

/* 16 bit register values */
#define E86_REG_AX 0
#define E86_REG_CX 1
#define E86_REG_DX 2
#define E86_REG_BX 3
#define E86_REG_SP 4
#define E86_REG_BP 5
#define E86_REG_SI 6
#define E86_REG_DI 7

/* 8 bit register values */
#define E86_REG_AL 0
#define E86_REG_CL 1
#define E86_REG_DL 2
#define E86_REG_BL 3
#define E86_REG_AH 4
#define E86_REG_CH 5
#define E86_REG_DH 6
#define E86_REG_BH 7

/* Segment register values */
#define E86_REG_ES 0
#define E86_REG_CS 1
#define E86_REG_SS 2
#define E86_REG_DS 3

#define E86_PREFIX_NEW  0x0001
#define E86_PREFIX_SEG  0x0002
#define E86_PREFIX_REP  0x0004
#define E86_PREFIX_REPN 0x0008
#define E86_PREFIX_LOCK 0x0010

#define E86_PQ_MAX 16


struct e8086_t;


typedef unsigned char (*e86_get_uint8_f) (void *ext, unsigned long addr);
typedef unsigned short (*e86_get_uint16_f) (void *ext, unsigned long addr);
typedef void (*e86_set_uint8_f) (void *ext, unsigned long addr, unsigned char val);
typedef void (*e86_set_uint16_f) (void *ext, unsigned long addr, unsigned short val);

typedef unsigned (*e86_opcode_f) (struct e8086_t *c);


typedef struct e8086_t {
	unsigned         cpu;

	unsigned short   dreg[8];
	unsigned short   sreg[4];
	unsigned short   ip;
	unsigned short   flg;

	void             *mem;
	e86_get_uint8_f  mem_get_uint8;
	e86_set_uint8_f  mem_set_uint8;
	e86_get_uint16_f mem_get_uint16;
	e86_set_uint16_f mem_set_uint16;

	void             *prt;
	e86_get_uint8_f  prt_get_uint8;
	e86_set_uint8_f  prt_set_uint8;
	e86_get_uint16_f prt_get_uint16;
	e86_set_uint16_f prt_set_uint16;

	unsigned char    *ram;
	unsigned long    ram_cnt;

	unsigned long    addr_mask;

	void             *inta_ext;
	unsigned char    (*inta) (void *ext);

	void             *op_ext;
	void             (*op_hook) (void *ext, unsigned char op1, unsigned char op2);
	void             (*op_stat) (void *ext, unsigned char op1, unsigned char op2);
	void             (*op_undef) (void *ext, unsigned char op1, unsigned char op2);
	void             (*op_int) (void *ext, unsigned char n);

	unsigned short   cur_ip;

	unsigned         pq_size;
	unsigned         pq_fill;
	unsigned         pq_cnt;
	unsigned char    pq[E86_PQ_MAX];

	unsigned         prefix;

	unsigned short   seg_override;

	int              halt;

	int              irq;

	e86_opcode_f     op[256];

	struct {
		int            is_mem;
		unsigned char  *data;
		unsigned short seg;
		unsigned short ofs;
		unsigned short cnt;
		unsigned long  delay;
	} ea;

	unsigned long    delay;

	unsigned long long clocks;
	unsigned long long instructions;
} e8086_t;


#define e86_get_reg8(cpu, reg) \
	((((reg) & 4) ? ((cpu)->dreg[(reg) & 3] >> 8) : (cpu)->dreg[(reg) & 3]) & 0xff)

#define e86_get_al(cpu) ((cpu)->dreg[E86_REG_AX] & 0xff)
#define e86_get_bl(cpu) ((cpu)->dreg[E86_REG_BX] & 0xff)
#define e86_get_cl(cpu) ((cpu)->dreg[E86_REG_CX] & 0xff)
#define e86_get_dl(cpu) ((cpu)->dreg[E86_REG_DX] & 0xff)
#define e86_get_ah(cpu) (((cpu)->dreg[E86_REG_AX] >> 8) & 0xff)
#define e86_get_bh(cpu) (((cpu)->dreg[E86_REG_BX] >> 8) & 0xff)
#define e86_get_ch(cpu) (((cpu)->dreg[E86_REG_CX] >> 8) & 0xff)
#define e86_get_dh(cpu) (((cpu)->dreg[E86_REG_DX] >> 8) & 0xff)


#define e86_get_reg16(cpu, reg) ((cpu)->dreg[(reg) & 7])
#define e86_get_ax(cpu) ((cpu)->dreg[E86_REG_AX])
#define e86_get_bx(cpu) ((cpu)->dreg[E86_REG_BX])
#define e86_get_cx(cpu) ((cpu)->dreg[E86_REG_CX])
#define e86_get_dx(cpu) ((cpu)->dreg[E86_REG_DX])
#define e86_get_sp(cpu) ((cpu)->dreg[E86_REG_SP])
#define e86_get_bp(cpu) ((cpu)->dreg[E86_REG_BP])
#define e86_get_si(cpu) ((cpu)->dreg[E86_REG_SI])
#define e86_get_di(cpu) ((cpu)->dreg[E86_REG_DI])


#define e86_get_sreg(cpu, reg) ((cpu)->sreg[(reg) & 3])
#define e86_get_cs(cpu) ((cpu)->sreg[E86_REG_CS])
#define e86_get_ds(cpu) ((cpu)->sreg[E86_REG_DS])
#define e86_get_es(cpu) ((cpu)->sreg[E86_REG_ES])
#define e86_get_ss(cpu) ((cpu)->sreg[E86_REG_SS])


#define e86_set_reg8(cpu, reg, val) \
	do { \
		unsigned char v = (val) & 0xff; \
		unsigned      r = (reg); \
		if (r & 4) { \
			(cpu)->dreg[r & 3] &= 0x00ff; \
			(cpu)->dreg[r & 3] |= v << 8; \
		} \
		else { \
			(cpu)->dreg[r & 3] &= 0xff00; \
			(cpu)->dreg[r & 3] |= v; \
		} \
	} while (0)

#define e86_set_al(cpu, val) e86_set_reg8 (cpu, E86_REG_AL, val)
#define e86_set_bl(cpu, val) e86_set_reg8 (cpu, E86_REG_BL, val)
#define e86_set_cl(cpu, val) e86_set_reg8 (cpu, E86_REG_CL, val)
#define e86_set_dl(cpu, val) e86_set_reg8 (cpu, E86_REG_DL, val)
#define e86_set_ah(cpu, val) e86_set_reg8 (cpu, E86_REG_AH, val)
#define e86_set_bh(cpu, val) e86_set_reg8 (cpu, E86_REG_BH, val)
#define e86_set_ch(cpu, val) e86_set_reg8 (cpu, E86_REG_CH, val)
#define e86_set_dh(cpu, val) e86_set_reg8 (cpu, E86_REG_DH, val)


#define e86_set_reg16(cpu, reg, val) \
	do { (cpu)->dreg[(reg) & 7] = (val) & 0xffff; } while (0)

#define e86_set_ax(cpu, val) do { (cpu)->dreg[E86_REG_AX] = (val) & 0xffff; } while (0)
#define e86_set_bx(cpu, val) do { (cpu)->dreg[E86_REG_BX] = (val) & 0xffff; } while (0)
#define e86_set_cx(cpu, val) do { (cpu)->dreg[E86_REG_CX] = (val) & 0xffff; } while (0)
#define e86_set_dx(cpu, val) do { (cpu)->dreg[E86_REG_DX] = (val) & 0xffff; } while (0)
#define e86_set_sp(cpu, val) do { (cpu)->dreg[E86_REG_SP] = (val) & 0xffff; } while (0)
#define e86_set_bp(cpu, val) do { (cpu)->dreg[E86_REG_BP] = (val) & 0xffff; } while (0)
#define e86_set_si(cpu, val) do { (cpu)->dreg[E86_REG_SI] = (val) & 0xffff; } while (0)
#define e86_set_di(cpu, val) do { (cpu)->dreg[E86_REG_DI] = (val) & 0xffff; } while (0)


#define e86_set_sreg(cpu, reg, val) \
	do { (cpu)->sreg[(reg) & 3] = (val) & 0xffff; } while (0)

#define e86_set_cs(cpu, val) do { (cpu)->sreg[E86_REG_CS] = (val) & 0xffff; } while (0)
#define e86_set_ds(cpu, val) do { (cpu)->sreg[E86_REG_DS] = (val) & 0xffff; } while (0)
#define e86_set_es(cpu, val) do { (cpu)->sreg[E86_REG_ES] = (val) & 0xffff; } while (0)
#define e86_set_ss(cpu, val) do { (cpu)->sreg[E86_REG_SS] = (val) & 0xffff; } while (0)


#define e86_get_ip(cpu) ((cpu)->ip)
#define e86_set_ip(cpu, val) do { (cpu)->ip = (val) & 0xffff; } while (0)


#define e86_get_flags(cpu) ((cpu)->flg)
#define e86_get_f(cpu, f) (((cpu)->flg & (f)) != 0)
#define e86_get_cf(cpu) (((cpu)->flg & E86_FLG_C) != 0)
#define e86_get_pf(cpu) (((cpu)->flg & E86_FLG_P) != 0)
#define e86_get_af(cpu) (((cpu)->flg & E86_FLG_A) != 0)
#define e86_get_zf(cpu) (((cpu)->flg & E86_FLG_Z) != 0)
#define e86_get_of(cpu) (((cpu)->flg & E86_FLG_O) != 0)
#define e86_get_sf(cpu) (((cpu)->flg & E86_FLG_S) != 0)
#define e86_get_df(cpu) (((cpu)->flg & E86_FLG_D) != 0)
#define e86_get_if(cpu) (((cpu)->flg & E86_FLG_I) != 0)
#define e86_get_tf(cpu) (((cpu)->flg & E86_FLG_T) != 0)


#define e86_set_flags(c, v) do { (c)->flg = (v) & 0xffffU; } while (0)

#define e86_set_f(c, f, v) \
	do { if (v) (c)->flg |= (f); else (c)->flg &= ~(f); } while (0)

#define e86_set_cf(c, v) e86_set_f (c, E86_FLG_C, v)
#define e86_set_pf(c, v) e86_set_f (c, E86_FLG_P, v)
#define e86_set_af(c, v) e86_set_f (c, E86_FLG_A, v)
#define e86_set_zf(c, v) e86_set_f (c, E86_FLG_Z, v)
#define e86_set_of(c, v) e86_set_f (c, E86_FLG_O, v)
#define e86_set_sf(c, v) e86_set_f (c, E86_FLG_S, v)
#define e86_set_df(c, v) e86_set_f (c, E86_FLG_D, v)
#define e86_set_if(c, v) e86_set_f (c, E86_FLG_I, v)
#define e86_set_tf(c, v) e86_set_f (c, E86_FLG_T, v)


#define e86_get_linear(seg, ofs) \
	((((seg) & 0xffffUL) << 4) + ((ofs) & 0xffff))


static inline
unsigned char e86_get_mem8 (e8086_t *c, unsigned short seg, unsigned short ofs)
{
	unsigned long addr = e86_get_linear (seg, ofs) & c->addr_mask;
	return ((addr < c->ram_cnt) ? c->ram[addr] : c->mem_get_uint8 (c->mem, addr));
}

static inline
void e86_set_mem8 (e8086_t *c, unsigned short seg, unsigned short ofs, unsigned char val)
{
	unsigned long addr = e86_get_linear (seg, ofs) & c->addr_mask;

	if (addr < c->ram_cnt) {
		c->ram[addr] = val;
	}
	else {
		c->mem_set_uint8 (c->mem, addr, val);
	}
}

static inline
unsigned short e86_get_mem16 (e8086_t *c, unsigned short seg, unsigned short ofs)
{
	unsigned long addr = e86_get_linear (seg, ofs) & c->addr_mask;

	if ((c->cpu & E86_CPU_8BIT) || (addr & 1)) {
		c->delay += 4;
	}

	if ((addr + 1) < c->ram_cnt) {
		return (c->ram[addr] + (c->ram[addr + 1] << 8));
	}
	else {
		return (c->mem_get_uint16 (c->mem, addr));
	}
}

static inline
void e86_set_mem16 (e8086_t *c, unsigned short seg, unsigned short ofs, unsigned short val)
{
	unsigned long addr = e86_get_linear (seg, ofs) & c->addr_mask;

	if ((c->cpu & E86_CPU_8BIT) || (addr & 1)) {
		c->delay += 4;
	}

	if ((addr + 1) < c->ram_cnt) {
		c->ram[addr] = val & 0xff;
		c->ram[addr + 1] = (val >> 8) & 0xff;
	}
	else {
		c->mem_set_uint16 (c->mem, addr, val);
	}
}

#define e86_get_prt8(cpu, ofs) \
	(cpu)->prt_get_uint8 ((cpu)->prt, ofs)

#define e86_get_prt16(cpu, ofs) \
	(cpu)->prt_get_uint16 ((cpu)->prt, ofs)

#define e86_set_prt8(cpu, ofs, val) \
	do { (cpu)->prt_set_uint8 ((cpu)->prt, ofs, val); } while (0)

#define e86_set_prt16(cpu, ofs, val) \
	do { (cpu)->prt_set_uint16 ((cpu)->prt, ofs, val); } while (0)

#define e86_get_delay(c) ((c)->delay)


void e86_init (e8086_t *c);
void e86_free (e8086_t *c);

e8086_t *e86_new (void);
void e86_del (e8086_t *c);

void e86_set_8086 (e8086_t *c);
void e86_set_8088 (e8086_t *c);
void e86_set_80186 (e8086_t *c);
void e86_set_80188 (e8086_t *c);
void e86_set_80286 (e8086_t *c);
void e86_set_v30 (e8086_t *c);
void e86_set_v20 (e8086_t *c);

/*!***************************************************************************
 * @short Set the prefetch queue size
 * @param size The emulated prefetch queue size
 *****************************************************************************/
void e86_set_pq_size (e8086_t *c, unsigned size);

/*!***************************************************************************
 * @short Set CPU options
 * @param opt A bit mask indicating the desired options
 * @param set If true, the options in opt are set otherwise they are reset
 *****************************************************************************/
void e86_set_options (e8086_t *c, unsigned opt, int set);

void e86_set_addr_mask (e8086_t *c, unsigned long msk);
unsigned long e86_get_addr_mask (e8086_t *c);

void e86_set_inta_fct (e8086_t *c, void *ext, void *fct);

void e86_set_ram (e8086_t *c, unsigned char *ram, unsigned long cnt);

void e86_set_mem (e8086_t *c, void *mem,
	e86_get_uint8_f get8, e86_set_uint8_f set8,
	e86_get_uint16_f get16, e86_set_uint16_f set16
);

void e86_set_prt (e8086_t *c, void *prt,
	e86_get_uint8_f get8, e86_set_uint8_f set8,
	e86_get_uint16_f get16, e86_set_uint16_f set16
);

int e86_get_reg (e8086_t *c, const char *reg, unsigned long *val);
int e86_set_reg (e8086_t *c, const char *reg, unsigned long val);

void e86_irq (e8086_t *cpu, unsigned char val);

int e86_interrupt (e8086_t *cpu, unsigned n);

unsigned e86_undefined (e8086_t *c);

unsigned long long e86_get_clock (e8086_t *c);

unsigned long long e86_get_opcnt (e8086_t *c);

void e86_reset (e8086_t *c);

void e86_execute (e8086_t *c);

void e86_clock (e8086_t *c, unsigned n);


void e86_push (e8086_t *c, unsigned short val);
unsigned short e86_pop (e8086_t *c);
void e86_trap (e8086_t *c, unsigned n);

void e86_pq_init (e8086_t *c);
void e86_pq_fill (e8086_t *c);


#define E86_DFLAGS_186  0x0001
#define E86_DFLAGS_CALL 0x0100
#define E86_DFLAGS_LOOP 0x0200

typedef struct {
	unsigned       flags;

	unsigned       seg;

	unsigned short ip;
	unsigned       dat_n;
	unsigned char  dat[16];

	char           op[64];

	unsigned       arg_n;
	char           arg1[64];
	char           arg2[64];
} e86_disasm_t;


void e86_disasm (e86_disasm_t *op, unsigned char *src, unsigned short ip);
void e86_disasm_mem (e8086_t *c, e86_disasm_t *op, unsigned short, unsigned short ip);
void e86_disasm_cur (e8086_t *c, e86_disasm_t *op);


#endif
