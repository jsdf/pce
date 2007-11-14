/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/cpu/e68000/e68000.h                                    *
 * Created:       2005-07-17 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2005-2007 Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id$ */


#ifndef PCE_E68000_H
#define PCE_E68000_H 1


#include <stdint.h>


struct e68000_s;


/*****************************************************************************
 * 68000
 *****************************************************************************/

#define E68_SR_C 0x0001
#define E68_SR_V 0x0002
#define E68_SR_Z 0x0004
#define E68_SR_N 0x0008
#define E68_SR_X 0x0010
#define E68_SR_I 0x0700
#define E68_SR_S 0x2000
#define E68_SR_T 0x8000

#define e68_get_dreg8(c, n) ((c)->dreg[(n)] & 0xff)
#define e68_get_dreg16(c, n) ((c)->dreg[(n)] & 0xffff)
#define e68_get_dreg32(c, n) ((c)->dreg[(n)] & 0xffffffff)
#define e68_get_areg16(c, n) (((c)->areg[(n)]) & 0xffff)
#define e68_get_areg32(c, n) ((c)->areg[(n)] & 0xffffffff)
#define e68_get_pc(c) ((c)->pc & 0xffffffff)
#define e68_get_usp(c) (((c)->supervisor ? (c)->usp : (c)->areg[7]) & 0xffffffff)
#define e68_get_ssp(c) (((c)->supervisor ? (c)->areg[7] : (c)->ssp) & 0xffffffff)
#define e68_get_sr(c) ((c)->sr & 0xffff)
#define e68_get_ccr(c) ((c)->sr & 0xff)
#define e68_get_int(c) (((c)->sr >> 8) & 7)

#define e68_set_pc(c, v) do { (c)->pc = (v) & 0xffffffff; } while (0)

#define e68_set_usp(c, v) do { \
	if ((c)->supervisor) (c)->usp = (v); else (c)->areg[7] = (v); \
	} while (0)

#define e68_set_ssp(c, v) do { \
	if ((c)->supervisor) (c)->areg[7] = (v); else (c)->ssp = (v); \
	} while (0)

#define e68_get_sr_c(c) (((c)->sr & E68_SR_C) != 0)
#define e68_get_sr_v(c) (((c)->sr & E68_SR_V) != 0)
#define e68_get_sr_z(c) (((c)->sr & E68_SR_Z) != 0)
#define e68_get_sr_n(c) (((c)->sr & E68_SR_N) != 0)
#define e68_get_sr_x(c) (((c)->sr & E68_SR_X) != 0)
#define e68_get_sr_s(c) (((c)->sr & E68_SR_S) != 0)
#define e68_get_sr_t(c) (((c)->sr & E68_SR_T) != 0)

#define e68_set_cc(c, m, v) do { \
    if (v) (c)->sr |= (m); else (c)->sr &= ~(m); \
  } while (0)

#define e68_set_sr_c(c, v) e68_set_cc ((c), E68_SR_C, (v))
#define e68_set_sr_v(c, v) e68_set_cc ((c), E68_SR_V, (v))
#define e68_set_sr_z(c, v) e68_set_cc ((c), E68_SR_Z, (v))
#define e68_set_sr_n(c, v) e68_set_cc ((c), E68_SR_N, (v))
#define e68_set_sr_x(c, v) e68_set_cc ((c), E68_SR_X, (v))
#define e68_set_sr_s(c, v) e68_set_cc ((c), E68_SR_S, (v))
#define e68_set_sr_t(c, v) e68_set_cc ((c), E68_SR_T, (v))

#define e68_set_int(c, v) do { \
	(c)->sr &= ~(7U << 8); \
	(c)->sr |= ((v) & 7) << 8; \
	} while (0)

typedef unsigned char (*e68_get_uint8_f) (void *ext, unsigned long addr);
typedef unsigned short (*e68_get_uint16_f) (void *ext, unsigned long addr);
typedef unsigned long (*e68_get_uint32_f) (void *ext, unsigned long addr);

typedef void (*e68_set_uint8_f) (void *ext, unsigned long addr, unsigned char val);
typedef void (*e68_set_uint16_f) (void *ext, unsigned long addr, unsigned short val);
typedef void (*e68_set_uint32_f) (void *ext, unsigned long addr, unsigned long val);

typedef unsigned (*e68_opcode_f) (struct e68000_s *c);


typedef struct e68000_s {
	void               *mem_ext;

	e68_get_uint8_f    get_uint8;
	e68_get_uint16_f   get_uint16;
	e68_get_uint32_f   get_uint32;

	e68_set_uint8_f    set_uint8;
	e68_set_uint16_f   set_uint16;
	e68_set_uint32_f   set_uint32;

	void               *log_ext;
	void               (*log_opcode) (void *ext, unsigned long ir);
	void               (*log_undef) (void *ext, unsigned long ir);
	void               (*log_exception) (void *ext, unsigned tn);

	uint32_t           dreg[8];
	uint32_t           areg[8];
	uint32_t           pc;
	uint16_t           sr;

	uint32_t           usp;
	uint32_t           ssp;

	char               supervisor;
	unsigned char      halt;

	unsigned           ircnt;
	uint16_t           ir[4];

	unsigned           ea_typ;
	uint32_t           ea_val;

	unsigned char      interrupt;

	unsigned long      delay;

	unsigned           excptn;
	const char         *excpts;

	unsigned long long oprcnt;
	unsigned long long clkcnt;

	e68_opcode_f       opcodes[1024];
} e68000_t;



/*!***************************************************************************
 * @short Initialize a 68000 context struct
 *****************************************************************************/
void e68_init (e68000_t *c);

/*!***************************************************************************
 * @short  Create and initialize a 68000 context struct
 * @return The 68000 struct or NULL on error
 *****************************************************************************/
e68000_t *e68_new (void);

/*!***************************************************************************
 * @short Free the resources used by a 68000 struct
 *****************************************************************************/
void e68_free (e68000_t *c);

/*!***************************************************************************
 * @short Delete a 68000 struct
 *****************************************************************************/
void e68_del (e68000_t *c);

void e68_set_mem_fct (e68000_t *c, void *ext,
  void *get8, void *get16, void *get32,
  void *set8, void *set16, void *set32
);

/*!***************************************************************************
 * @short Get the number of executed instructions
 *****************************************************************************/
unsigned long long e68_get_opcnt (e68000_t *c);

/*!***************************************************************************
 * @short Get the number of clock cycles
 *****************************************************************************/
unsigned long long e68_get_clkcnt (e68000_t *c);

/*!***************************************************************************
 * @short Get the previous instruction delay
 *****************************************************************************/
unsigned long e68_get_delay (e68000_t *c);

unsigned e68_get_halt (e68000_t *c);
void e68_set_halt (e68000_t *c, unsigned val);

unsigned e68_get_exception (e68000_t *c);
const char *e68_get_exception_name (e68000_t *c);

int e68_get_reg (e68000_t *c, const char *reg, unsigned long *val);
int e68_set_reg (e68000_t *c, const char *reg, unsigned long val);


/*!***************************************************************************
 * @short Set the execution mode
 *****************************************************************************/
void e68_set_mode (e68000_t *c, int supervisor);

/*!***************************************************************************
 * @short Set the status register
 *****************************************************************************/
void e68_set_sr (e68000_t *c, unsigned short val);

void e68_push16 (e68000_t *c, uint16_t val);
void e68_push32 (e68000_t *c, uint32_t val);

/*!***************************************************************************
 * @short Execute an exception
 * @param n The exception number (0 <= n <= 255)
 *****************************************************************************/
void e68_exception (e68000_t *c, unsigned n, const char *name);

void e68_exception_reset (e68000_t *c);

void e68_exception_address (e68000_t *c, uint32_t addr);

void e68_exception_illegal (e68000_t *c);

void e68_exception_divzero (e68000_t *c);

void e68_exception_check (e68000_t *c);

void e68_exception_overflow (e68000_t *c);

void e68_exception_privilege (e68000_t *c);

void e68_exception_axxx (e68000_t *c);

void e68_exception_fxxx (e68000_t *c);

void e68_exception_trap (e68000_t *c, unsigned n);

/*!***************************************************************************
 * @short The external interrupt input signal
 *****************************************************************************/
void e68_interrupt (e68000_t *c, unsigned char val);

/*!***************************************************************************
 * @short Reset a 68000 cpu core
 *****************************************************************************/
void e68_reset (e68000_t *c);

/*!***************************************************************************
 * @short Execute one instruction
 *****************************************************************************/
void e68_execute (e68000_t *c);

/*!***************************************************************************
 * @short Clock a 68000 cpu core
 *****************************************************************************/
void e68_clock (e68000_t *c, unsigned long n);


static inline
void e68_set_dreg8 (e68000_t *c, unsigned reg, uint8_t val)
{
	reg &= 7;
	c->dreg[reg] = (c->dreg[reg] & 0xffffff00) | (val & 0x000000ff);
}

static inline
void e68_set_dreg16 (e68000_t *c, unsigned reg, uint16_t val)
{
	reg &= 7;
	c->dreg[reg] = (c->dreg[reg] & 0xffff0000) | (val & 0x0000ffff);
}

static inline
void e68_set_dreg32 (e68000_t *c, unsigned reg, uint32_t val)
{
	c->dreg[reg & 7] = val & 0xffffffff;
}

static inline
void e68_set_areg16 (e68000_t *c, unsigned reg, uint16_t val)
{
	reg &= 7;

	c->areg[reg] = val & ((reg == 7) ? 0xfffe : 0xffff);

	if (val & 0x8000) {
		c->areg[reg] |= 0xffff0000;
	}
}

static inline
void e68_set_areg32 (e68000_t *c, unsigned reg, uint32_t val)
{
	c->areg[reg & 7] = val & ((reg == 7) ? 0xfffffffe : 0xffffffff);
}

static inline
void e68_set_ccr (e68000_t *c, uint8_t val)
{
	c->sr = (c->sr & 0xff00) | (val & 0x00ff);
}


static inline
uint8_t e68_get_mem8 (e68000_t *c, uint32_t addr)
{
	return (c->get_uint8 (c->mem_ext, addr & 0x00ffffff));
}

static inline
uint16_t e68_get_mem16 (e68000_t *c, uint32_t addr)
{
	return (c->get_uint16 (c->mem_ext, addr & 0x00ffffff));
}

static inline
uint32_t e68_get_mem32 (e68000_t *c, uint32_t addr)
{
	return (c->get_uint32 (c->mem_ext, addr & 0x00ffffff));
}

static inline
void e68_set_mem8 (e68000_t *c, uint32_t addr, uint8_t val)
{
	c->set_uint8 (c->mem_ext, addr & 0x00ffffff, val);
}

static inline
void e68_set_mem16 (e68000_t *c, uint32_t addr, uint16_t val)
{
	c->set_uint16 (c->mem_ext, addr & 0x00ffffff, val);
}

static inline
void e68_set_mem32 (e68000_t *c, uint32_t addr, uint32_t val)
{
	c->set_uint32 (c->mem_ext, addr & 0x00ffffff, val);
}


/*****************************************************************************
 * disasm
 *****************************************************************************/

#define E68_DFLAG_PRIV 0x0001
#define E68_DFLAG_CALL 0x0100
#define E68_DFLAG_RTE  0x0200

typedef struct {
	unsigned flags;

	uint32_t pc;

	unsigned irn;
	uint16_t ir[8];

	unsigned argn;

	char     op[64];
	char     arg1[64];
	char     arg2[64];
	char     arg3[64];
	char     comm[64];
} e68_dasm_t;


void e68_dasm (e68_dasm_t *da, uint32_t pc, const unsigned char *src);
void e68_dasm_mem (e68000_t *c, e68_dasm_t *da, uint32_t pc);
void e68_dasm_cur (e68000_t *c, e68_dasm_t *da);



#endif
