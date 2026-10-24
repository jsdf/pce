/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8080/e8080.h                                        *
 * Created:     2012-11-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2024 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 *****************************************************************************/


#ifndef PCE_E8080_H
#define PCE_E8080_H 1


#include <stdio.h>


#define E8080_FLAG_Z80 1


/* CPU flags */
#define E8080_FLG_C 0x01
#define E8080_FLG_N 0x02
#define E8080_FLG_P 0x04
#define E8080_FLG_3 0x08
#define E8080_FLG_A 0x10
#define E8080_FLG_5 0x20
#define E8080_FLG_Z 0x40
#define E8080_FLG_S 0x80


struct e8080_t;


typedef void (*e8080_opcode_f) (struct e8080_t *c);


typedef struct e8080_t {
	unsigned       flags;

	unsigned short pc;
	unsigned short sp;
	unsigned short ix;
	unsigned short iy;
	unsigned char  psw;
	unsigned char  psw2;
	unsigned char  reg[8];
	unsigned char  reg2[8];
	unsigned char  i;
	unsigned char  r;

	unsigned char  iff;
	unsigned char  iff2;

	unsigned char  im;

	unsigned char  halt;

	unsigned char  int_val;
	unsigned char  nmi_val;
	unsigned char  int_req;

	unsigned       int_cnt;
	unsigned short int_pc;

	void           *mem_rd_ext;
	void           *mem_wr_ext;

	unsigned char  (*get_uint8) (void *ext, unsigned long addr);
	void           (*set_uint8) (void *ext, unsigned long addr, unsigned char val);

	unsigned char  *mem_map_rd[64];
	unsigned char  *mem_map_wr[64];

	void           *port_rd_ext;
	void           *port_wr_ext;

	unsigned char  (*get_port8) (void *ext, unsigned long addr);
	void           (*set_port8) (void *ext, unsigned long addr, unsigned char val);

	void           *hook_ext;
	int            (*hook_all) (void *ext, unsigned char op);
	int            (*hook_undef) (void *ext, unsigned char op);
	int            (*hook_rst) (void *ext, unsigned char op);

	unsigned char  inst[4];

	e8080_opcode_f op[256];

	unsigned       delay;
	unsigned long  clkcnt;
	unsigned long  inscnt;
} e8080_t;


#define e8080_get_a(c) ((c)->reg[7])
#define e8080_get_b(c) ((c)->reg[0])
#define e8080_get_c(c) ((c)->reg[1])
#define e8080_get_d(c) ((c)->reg[2])
#define e8080_get_e(c) ((c)->reg[3])
#define e8080_get_h(c) ((c)->reg[4])
#define e8080_get_l(c) ((c)->reg[5])
#define e8080_get_bc(c) e8080_get_rp ((c), 0, 1)
#define e8080_get_de(c) e8080_get_rp ((c), 2, 3)
#define e8080_get_hl(c) e8080_get_rp ((c), 4, 5)
#define e8080_get_pc(c) ((c)->pc)
#define e8080_get_sp(c) ((c)->sp)
#define e8080_get_ix(c) ((c)->ix)
#define e8080_get_iy(c) ((c)->iy)
#define e8080_get_ixl(c) ((c)->ix & 0xff)
#define e8080_get_ixh(c) (((c)->ix >> 8) & 0xff)
#define e8080_get_iyl(c) ((c)->iy & 0xff)
#define e8080_get_iyh(c) (((c)->iy >> 8) & 0xff)
#define e8080_get_i(c) ((c)->i)
#define e8080_get_r(c) ((c)->r)
#define e8080_get_psw(c) ((c)->psw)
#define e8080_get_reg8(c, r) ((c)->reg[(r) & 7])

#define e8080_get_a2(c) ((c)->reg2[7]);
#define e8080_get_b2(c) ((c)->reg2[0]);
#define e8080_get_c2(c) ((c)->reg2[1]);
#define e8080_get_d2(c) ((c)->reg2[2]);
#define e8080_get_e2(c) ((c)->reg2[3]);
#define e8080_get_h2(c) ((c)->reg2[4]);
#define e8080_get_l2(c) ((c)->reg2[5]);
#define e8080_get_psw2(c) ((c)->psw2)

#define e8080_get_halt(c) ((c)->halt)
#define e8080_get_iff1(c) ((c)->iff)
#define e8080_get_iff2(c) ((c)->iff2)
#define e8080_get_im(c) ((c)->im)
#define e8080_get_int_cnt(c) ((c)->int_cnt)
#define e8080_get_int_pc(c) ((c)->int_pc)

#define e8080_set_a(c, v) do { (c)->reg[7] = (v) & 0xff; } while (0)
#define e8080_set_b(c, v) do { (c)->reg[0] = (v) & 0xff; } while (0)
#define e8080_set_c(c, v) do { (c)->reg[1] = (v) & 0xff; } while (0)
#define e8080_set_d(c, v) do { (c)->reg[2] = (v) & 0xff; } while (0)
#define e8080_set_e(c, v) do { (c)->reg[3] = (v) & 0xff; } while (0)
#define e8080_set_h(c, v) do { (c)->reg[4] = (v) & 0xff; } while (0)
#define e8080_set_l(c, v) do { (c)->reg[5] = (v) & 0xff; } while (0)
#define e8080_set_bc(c, v) e8080_set_rp ((c), 0, 1, (v))
#define e8080_set_de(c, v) e8080_set_rp ((c), 2, 3, (v))
#define e8080_set_hl(c, v) e8080_set_rp ((c), 4, 5, (v))
#define e8080_set_pc(c, v) do { (c)->pc = (v) & 0xffff; } while (0)
#define e8080_set_sp(c, v) do { (c)->sp = (v) & 0xffff; } while (0)
#define e8080_set_ix(c, v) do { (c)->ix = (v) & 0xffff; } while (0)
#define e8080_set_iy(c, v) do { (c)->iy = (v) & 0xffff; } while (0)
#define e8080_set_ixl(c, v) do { (c)->ix = ((c)->ix & 0xff00) | ((v) & 0xff); } while (0)
#define e8080_set_ixh(c, v) do { (c)->ix = ((c)->ix & 0x00ff) | (((v) & 0xff) << 8); } while (0)
#define e8080_set_iyl(c, v) do { (c)->iy = ((c)->iy & 0xff00) | ((v) & 0xff); } while (0)
#define e8080_set_iyh(c, v) do { (c)->iy = ((c)->iy & 0x00ff) | (((v) & 0xff) << 8); } while (0)
#define e8080_set_i(c, v) do { (c)->i = (v) & 0xff; } while (0)
#define e8080_set_r(c, v) do { (c)->r = (v) & 0xff; } while (0)
#define e8080_set_psw(c, v) do { (c)->psw = (v) & 0xff; } while (0)
#define e8080_set_reg8(c, r, v) do { (c)->reg[(r) & 7] = (v) & 0xff; } while (0)

#define e8080_set_a2(c, v) do { (c)->reg2[7] = (v) & 0xff; } while (0)
#define e8080_set_b2(c, v) do { (c)->reg2[0] = (v) & 0xff; } while (0)
#define e8080_set_c2(c, v) do { (c)->reg2[1] = (v) & 0xff; } while (0)
#define e8080_set_d2(c, v) do { (c)->reg2[2] = (v) & 0xff; } while (0)
#define e8080_set_e2(c, v) do { (c)->reg2[3] = (v) & 0xff; } while (0)
#define e8080_set_h2(c, v) do { (c)->reg2[4] = (v) & 0xff; } while (0)
#define e8080_set_l2(c, v) do { (c)->reg2[5] = (v) & 0xff; } while (0)
#define e8080_set_psw2(c, v) do { (c)->psw2 = (v) & 0xff; } while (0)

#define e8080_set_halt(c, v) do { (c)->halt = ((v) != 0); } while (0)
#define e8080_set_iff1(c, v) do { (c)->iff = ((v) != 0); } while (0)
#define e8080_set_iff2(c, v) do { (c)->iff2 = ((v) != 0); } while (0)
#define e8080_set_im(c, v) do { (c)->im = (v); } while (0)
#define e8080_set_int_cnt(c, v) do { (c)->int_cnt = (v); } while (0)
#define e8080_set_int_pc(c, v) do { (c)->int_pc = (v) & 0xffff; } while (0)

#define e8080_get_cf(c) (((c)->psw & E8080_FLG_C) != 0)
#define e8080_get_nf(c) (((c)->psw & E8080_FLG_N) != 0)
#define e8080_get_pf(c) (((c)->psw & E8080_FLG_P) != 0)
#define e8080_get_af(c) (((c)->psw & E8080_FLG_A) != 0)
#define e8080_get_zf(c) (((c)->psw & E8080_FLG_Z) != 0)
#define e8080_get_sf(c) (((c)->psw & E8080_FLG_S) != 0)

#define e8080_set_flag(c, f, v) \
	do { if (v) (c)->psw |= (f); else (c)->psw &= ~(f); } while (0)
#define e8080_set_cf(c, v) e8080_set_flag (c, E8080_FLG_C, v)
#define e8080_set_nf(c, v) e8080_set_flag (c, E8080_FLG_N, v)
#define e8080_set_pf(c, v) e8080_set_flag (c, E8080_FLG_P, v)
#define e8080_set_af(c, v) e8080_set_flag (c, E8080_FLG_A, v)
#define e8080_set_zf(c, v) e8080_set_flag (c, E8080_FLG_Z, v)
#define e8080_set_sf(c, v) e8080_set_flag (c, E8080_FLG_S, v)


static inline
unsigned e8080_get_rp (const e8080_t *c, unsigned r1, unsigned r2)
{
	return (((unsigned) c->reg[r1] << 8) | c->reg[r2]);
}

static inline
void e8080_set_rp (e8080_t *c, unsigned r1, unsigned r2, unsigned val)
{
	c->reg[r1] = (val >> 8) & 0xff;
	c->reg[r2] = val & 0xff;
}

static inline
unsigned char e8080_get_mem8 (e8080_t *c, unsigned short addr)
{
	const unsigned char *p;

	p = c->mem_map_rd[(addr >> 10) & 0x3f];

	if (p != NULL) {
		return (p[addr & 0x3ff]);
	}

	return (c->get_uint8 (c->mem_rd_ext, addr));
}

static inline
void e8080_set_mem8 (e8080_t *c, unsigned short addr, unsigned char val)
{
	unsigned char *p;

	p = c->mem_map_wr[(addr >> 10) & 0x3f];

	if (p != NULL) {
		p[addr & 0x3ff] = val;
	}
	else {
		c->set_uint8 (c->mem_wr_ext, addr, val);
	}
}

static inline
unsigned short e8080_get_mem16 (e8080_t *c, unsigned short addr)
{
	unsigned short val;

	val = e8080_get_mem8 (c, addr) & 0xff;
	val |= (unsigned) (e8080_get_mem8 (c, addr + 1) & 0xff) << 8;

	return (val);
}

static inline
void e8080_set_mem16 (e8080_t *c, unsigned short addr, unsigned short val)
{
	e8080_set_mem8 (c, addr, val);
	e8080_set_mem8 (c, addr + 1, val >> 8);
}


/*****************************************************************************
 * @short Initialize a 8080 context
 *****************************************************************************/
void e8080_init (e8080_t *c);

/*****************************************************************************
 * @short Create and initialize a new 8080 context
 *****************************************************************************/
e8080_t *e8080_new (void);

/*****************************************************************************
 * @short Free the resources used by a 8080 context
 *****************************************************************************/
void e8080_free (e8080_t *c);

/*****************************************************************************
 * @short Delete a 8080 context
 *****************************************************************************/
void e8080_del (e8080_t *c);

void e8080_set_mem_map_rd (e8080_t *c, unsigned addr1, unsigned addr2, unsigned char *p);
void e8080_set_mem_map_wr (e8080_t *c, unsigned addr1, unsigned addr2, unsigned char *p);

void e8080_set_8080 (e8080_t *c);
void e8080_set_z80 (e8080_t *c);

/*****************************************************************************
 * @short Get the CPU flags
 *****************************************************************************/
unsigned e8080_get_flags (e8080_t *c);

/*****************************************************************************
 * @short Set the CPU flags
 *****************************************************************************/
void e8080_set_flags (e8080_t *c, unsigned flags);

void e8080_set_mem_read_fct (e8080_t *c, void *ext, void *get8);
void e8080_set_mem_write_fct (e8080_t *c, void *ext, void *set8);
void e8080_set_mem_fct (e8080_t *c, void *mem, void *get8, void *set8);

void e8080_set_port_read_fct (e8080_t *c, void *ext, void *get8);
void e8080_set_port_write_fct (e8080_t *c, void *ext, void *set8);
void e8080_set_port_fct (e8080_t *c, void *ext, void *get8, void *set8);

void e8080_set_hook_all_fct (e8080_t *c, void *ext, void *fct);
void e8080_set_hook_undef_fct (e8080_t *c, void *ext, void *fct);
void e8080_set_hook_rst_fct (e8080_t *c, void *ext, void *fct);

void e8080_rst (e8080_t *c, unsigned val);

void e8080_set_int (e8080_t *c, unsigned char val);

unsigned char e8080_get_port8 (e8080_t *c, unsigned addr);
void e8080_set_port8 (e8080_t *c, unsigned addr, unsigned char val);

int e8080_get_reg (e8080_t *c, const char *reg, unsigned long *val);
int e8080_set_reg (e8080_t *c, const char *reg, unsigned long val);


/*****************************************************************************
 * @short Get the number of executed clock cycles
 *****************************************************************************/
unsigned long e8080_get_clock (e8080_t *c);

/*****************************************************************************
 * @short Get the number of executed instructions
 *****************************************************************************/
unsigned long e8080_get_opcnt (e8080_t *c);

/*****************************************************************************
 * @short Get the current delay
 *****************************************************************************/
unsigned e8080_get_delay (e8080_t *c);


/*****************************************************************************
 * @short Reset the 8080
 *****************************************************************************/
void e8080_reset (e8080_t *c);

/*****************************************************************************
 * @short Execute one instruction
 *****************************************************************************/
void e8080_execute (e8080_t *c);

/*****************************************************************************
 * @short Clock the 8080
 * @param c The 8080 context
 * @param n The number of clock cycles
 *****************************************************************************/
void e8080_clock (e8080_t *c, unsigned n);



#define E8080_OPF_COND  0x0001
#define E8080_OPF_JMP   0x0002
#define E8080_OPF_CALL  0x0004
#define E8080_OPF_RET   0x0008
#define E8080_OPF_UND   0x0010
#define E8080_OPF_Z80   0x0020

typedef struct {
	unsigned short flags;

	unsigned short pc;
	unsigned       data_cnt;
	unsigned char  data[8];

	char           op[16];

	unsigned       arg_cnt;
	char           arg[2][16];
} e8080_disasm_t;


void e8080_disasm (e8080_disasm_t *op, const unsigned char *src, unsigned short pc);
void e8080_disasm_mem (e8080_t *c, e8080_disasm_t *op, unsigned short pc);
void e8080_disasm_cur (e8080_t *c, e8080_disasm_t *op);

void z80_disasm (e8080_disasm_t *op, const unsigned char *src, unsigned short pc);
void z80_disasm_mem (e8080_t *c, e8080_disasm_t *op, unsigned short pc);
void z80_disasm_cur (e8080_t *c, e8080_disasm_t *op);


#endif
