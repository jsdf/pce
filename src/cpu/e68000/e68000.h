/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e68000/e68000.h                                      *
 * Created:     2005-07-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2005-2020 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_E68000_H
#define PCE_E68000_H 1


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


/* #define E68000_LOG_MEM 1 */


struct e68000_s;


/*****************************************************************************
 * 68000
 *****************************************************************************/

#define E68_FLAG_NOADDR  1
#define E68_FLAG_68010   2
#define E68_FLAG_68020   4
#define E68_FLAG_NORESET 8

#define E68_LAST_PC_CNT 32

#define E68_SR_C 0x0001
#define E68_SR_V 0x0002
#define E68_SR_Z 0x0004
#define E68_SR_N 0x0008
#define E68_SR_X 0x0010
#define E68_SR_I 0x0700
#define E68_SR_S 0x2000
#define E68_SR_T 0x8000

#define e68_get_dreg8(c, n) ((c)->dreg[(n) & 7] & 0xff)
#define e68_get_dreg16(c, n) ((c)->dreg[(n) & 7] & 0xffff)
#define e68_get_dreg32(c, n) ((c)->dreg[(n) & 7] & 0xffffffff)
#define e68_get_areg16(c, n) (((c)->areg[(n) & 7]) & 0xffff)
#define e68_get_areg32(c, n) ((c)->areg[(n) & 7] & 0xffffffff)
#define e68_get_pc(c) ((c)->pc & 0xffffffff)
#define e68_get_sp(c) ((c)->areg[7] & 0xffffffff)
#define e68_get_ir_pc(c) ((c)->ir_pc & 0xffffffff)
#define e68_get_usp(c) (((c)->supervisor ? (c)->usp : (c)->areg[7]) & 0xffffffff)
#define e68_get_ssp(c) (((c)->supervisor ? (c)->areg[7] : (c)->ssp) & 0xffffffff)
#define e68_get_sr(c) ((c)->sr & 0xffff)
#define e68_get_ccr(c) ((c)->sr & 0xff)
#define e68_get_vbr(c) ((c)->vbr & 0xffffffff)
#define e68_get_sfc(c) ((c)->sfc & 0x00000003)
#define e68_get_dfc(c) ((c)->dfc & 0x00000003)
#define e68_get_cacr(c) ((c)->vbr & 0xffffffff)
#define e68_get_caar(c) ((c)->vbr & 0xffffffff)
#define e68_get_ipl(c) ((c)->int_ipl)
#define e68_get_iml(c) (((c)->sr >> 8) & 7)

#define e68_set_pc(c, v) do { (c)->pc = (v) & 0xffffffff; } while (0)
#define e68_set_ir_pc(c, v) do { (c)->ir_pc = (v) & 0xffffffff; } while (0)
#define e68_set_vbr(c, v) do { (c)->vbr = (v) & 0xffffffff; } while (0)
#define e68_set_sfc(c, v) do { (c)->sfc = (v) & 0x00000003; } while (0)
#define e68_set_dfc(c, v) do { (c)->dfc = (v) & 0x00000003; } while (0)
#define e68_set_cacr(c, v) do { (c)->cacr = (v) & 0xffffffff; } while (0)
#define e68_set_caar(c, v) do { (c)->cacr = (v) & 0xffffffff; } while (0)

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
#define e68_set_sr_xc(c, v) e68_set_cc ((c), E68_SR_X | E68_SR_C, (v))


typedef void (*e68_opcode_f) (struct e68000_s *c);


typedef struct e68000_s {
	unsigned       flags;

	void           *mem_ext;

	unsigned char  (*get_uint8) (void *ext, unsigned long addr);
	unsigned short (*get_uint16) (void *ext, unsigned long addr);
	unsigned long  (*get_uint32) (void *ext, unsigned long addr);

	void           (*set_uint8) (void *ext, unsigned long addr, unsigned char val);
	void           (*set_uint16) (void *ext, unsigned long addr, unsigned short val);
	void           (*set_uint32) (void *ext, unsigned long addr, unsigned long val);

	unsigned char  *ram;
	unsigned long  ram_cnt;

	void           *reset_ext;
	void           (*reset) (void *ext, unsigned char val);
	unsigned char  reset_val;

	unsigned char  inta_val;
	void           *inta_ext;
	unsigned       (*inta) (void *ext, unsigned level);

	void           *hook_ext;
	int            (*hook) (void *ext, unsigned val);

	void           *trap_ext;
	int            (*trap) (void *ext, unsigned n);

	void           *log_ext;
	void           (*log_opcode) (void *ext, unsigned long ir);
	void           (*log_undef) (void *ext, unsigned long ir);
	void           (*log_exception) (void *ext, unsigned tn);
	void           (*log_mem) (void *ext, unsigned long addr, unsigned type);

	uint32_t       dreg[8];
	uint32_t       areg[8];
	uint32_t       pc;
	uint32_t       ir_pc;
	uint16_t       ir[3];
	uint16_t       sr;
	uint32_t       usp;
	uint32_t       ssp;
	uint32_t       vbr;
	uint32_t       sfc;
	uint32_t       dfc;
	uint32_t       cacr;
	uint32_t       caar;

	unsigned       last_pc_idx;
	uint32_t       last_pc[E68_LAST_PC_CNT];
	uint16_t       last_trap_a;
	uint16_t       last_trap_f;

	uint16_t       trace_sr;

	char           supervisor;
	unsigned char  halt;
	char           bus_error;
	char           exception;

	unsigned       ea_typ;
	uint32_t       ea_val;
	unsigned long  ea_bf_ofs;
	unsigned       ea_bf_width;
	unsigned char  ea_bf_val[5];

	unsigned       int_ipl;
	char           int_nmi;

	unsigned long  delay;

	unsigned       except_cnt;
	uint32_t       except_addr;
	unsigned       except_vect;
	const char     *except_name;

	unsigned long  oprcnt;
	unsigned long  clkcnt;

	e68_opcode_f   opcodes[1024];
	e68_opcode_f   op49c0[8];
} e68000_t;



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

	c->areg[reg] = val & 0xffff;

	if (val & 0x8000) {
		c->areg[reg] |= 0xffff0000;
	}
}

static inline
void e68_set_areg32 (e68000_t *c, unsigned reg, uint32_t val)
{
	c->areg[reg & 7] = val & 0xffffffff;
}

static inline
uint8_t e68_get_mem8 (e68000_t *c, uint32_t addr)
{
#ifdef E68000_LOG_MEM
	if (c->log_mem != NULL) {
		c->log_mem (c->log_ext, addr, 2);
	}
#endif

	addr &= 0x00ffffff;

	if (addr < c->ram_cnt) {
		return (c->ram[addr]);
	}

	return (c->get_uint8 (c->mem_ext, addr & 0x00ffffff));
}

static inline
uint16_t e68_get_mem16 (e68000_t *c, uint32_t addr)
{
#ifdef E68000_LOG_MEM
	if (c->log_mem != NULL) {
		c->log_mem (c->log_ext, addr, 4);
	}
#endif

	addr &= 0x00ffffff;

	if ((addr + 1) < c->ram_cnt) {
		return ((c->ram[addr] << 8) | c->ram[addr + 1]);
	}

	return (c->get_uint16 (c->mem_ext, addr));
}

static inline
uint32_t e68_get_mem32 (e68000_t *c, uint32_t addr)
{
	uint32_t val;

#ifdef E68000_LOG_MEM
	if (c->log_mem != NULL) {
		c->log_mem (c->log_ext, addr, 8);
	}
#endif

	addr &= 0x00ffffff;

	if ((addr + 3) < c->ram_cnt) {
		val = c->ram[addr];
		val = (val << 8) | c->ram[addr + 1];
		val = (val << 8) | c->ram[addr + 2];
		val = (val << 8) | c->ram[addr + 3];

		return (val);
	}

	return (c->get_uint32 (c->mem_ext, addr));
}

static inline
void e68_set_mem8 (e68000_t *c, uint32_t addr, uint8_t val)
{
#ifdef E68000_LOG_MEM
	if (c->log_mem != NULL) {
		c->log_mem (c->log_ext, addr, 3);
	}
#endif

	addr &= 0x00ffffff;

	if (addr < c->ram_cnt) {
		c->ram[addr] = val;
	}
	else {
		c->set_uint8 (c->mem_ext, addr, val);
	}
}

static inline
void e68_set_mem16 (e68000_t *c, uint32_t addr, uint16_t val)
{
#ifdef E68000_LOG_MEM
	if (c->log_mem != NULL) {
		c->log_mem (c->log_ext, addr, 5);
	}
#endif

	addr &= 0x00ffffff;

	if ((addr + 1) < c->ram_cnt) {
		c->ram[addr] = (val >> 8) & 0xff;
		c->ram[addr + 1] = val & 0xff;
	}
	else {
		c->set_uint16 (c->mem_ext, addr, val);
	}
}

static inline
void e68_set_mem32 (e68000_t *c, uint32_t addr, uint32_t val)
{
#ifdef E68000_LOG_MEM
	if (c->log_mem != NULL) {
		c->log_mem (c->log_ext, addr, 9);
	}
#endif

	addr &= 0x00ffffff;

	if ((addr + 3) < c->ram_cnt) {
		c->ram[addr] = (val >> 24) & 0xff;
		c->ram[addr + 1] = (val >> 16) & 0xff;
		c->ram[addr + 2] = (val >> 8) & 0xff;
		c->ram[addr + 3] = val & 0xff;
	}
	else {
		c->set_uint32 (c->mem_ext, addr, val);
	}
}


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

/*!***************************************************************************
 * @short Set the memory access functions
 *****************************************************************************/
void e68_set_mem_fct (e68000_t *c, void *ext,
	void *get8, void *get16, void *get32,
	void *set8, void *set16, void *set32
);

void e68_set_ram (e68000_t *c, unsigned char *ram, unsigned long cnt);

void e68_set_reset_fct (e68000_t *c, void *ext, void *fct);

void e68_set_inta_fct (e68000_t *c, void *ext, void *fct);

void e68_set_hook_fct (e68000_t *c, void *ext, void *fct);

void e68_set_trap_fct (e68000_t *c, void *ext, void *fct);

void e68_set_flags (e68000_t *c, unsigned flags, int set);

void e68_set_address_check (e68000_t *c, int check);

void e68_set_68000 (e68000_t *c);

void e68_set_68010 (e68000_t *c);

void e68_set_68020 (e68000_t *c);

/*!***************************************************************************
 * @short Get the number of executed instructions
 *****************************************************************************/
unsigned long e68_get_opcnt (const e68000_t *c);

/*!***************************************************************************
 * @short Get the number of clock cycles
 *****************************************************************************/
unsigned long e68_get_clkcnt (const e68000_t *c);

/*!***************************************************************************
 * @short Get the previous instruction delay
 *****************************************************************************/
unsigned long e68_get_delay (e68000_t *c);

unsigned e68_get_halt (e68000_t *c);
void e68_set_halt (e68000_t *c, unsigned val);

void e68_set_bus_error (e68000_t *c, int val);

/*!***************************************************************************
 * @short Get the number of exceptions
 *****************************************************************************/
unsigned e68_get_exception_cnt (const e68000_t *c);

/*!***************************************************************************
 * @short Get the last exception number
 *****************************************************************************/
unsigned e68_get_exception (const e68000_t *c);

/*!***************************************************************************
 * @short Get the last exception name
 *****************************************************************************/
const char *e68_get_exception_name (const e68000_t *c);

/*!***************************************************************************
 * @short Get the last PC
 *****************************************************************************/
unsigned long e68_get_last_pc (e68000_t *pc, unsigned idx);

/*!***************************************************************************
 * @short Get the last a-line trap number
 *****************************************************************************/
unsigned short e68_get_last_trap_a (e68000_t *c);

/*!***************************************************************************
 * @short Get the last f-line trap number
 *****************************************************************************/
unsigned short e68_get_last_trap_f (e68000_t *c);


/*!***************************************************************************
 * @short Get a named register
 *****************************************************************************/
int e68_get_reg (e68000_t *c, const char *reg, unsigned long *val);

/*!***************************************************************************
 * @short Set a named register
 *****************************************************************************/
int e68_set_reg (e68000_t *c, const char *reg, unsigned long val);

/*!***************************************************************************
 * @short Set the PC and prefetch
 *****************************************************************************/
void e68_set_pc_prefetch (e68000_t *c, unsigned long val);

void e68_exception_reset (e68000_t *c);

void e68_exception_bus (e68000_t *c, uint32_t addr, int data, int wr);

void e68_exception_address (e68000_t *c, uint32_t addr, int data, int wr);

void e68_exception_illegal (e68000_t *c);

void e68_exception_divzero (e68000_t *c);

void e68_exception_check (e68000_t *c);

void e68_exception_overflow (e68000_t *c);

void e68_exception_privilege (e68000_t *c);

void e68_exception_axxx (e68000_t *c);

void e68_exception_fxxx (e68000_t *c);

void e68_exception_format (e68000_t *c);

void e68_exception_avec (e68000_t *c, unsigned level);

void e68_exception_trap (e68000_t *c, unsigned n);

void e68_exception_intr (e68000_t *c, unsigned level, unsigned vect);

/*!***************************************************************************
 * @short Set the interrupt priority level input
 *****************************************************************************/
void e68_interrupt (e68000_t *c, unsigned level);

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


/*****************************************************************************
 * disasm
 *****************************************************************************/

#define E68_DFLAG_PRIV   0x0001
#define E68_DFLAG_JUMP   0x0002
#define E68_DFLAG_CALL   0x0004
#define E68_DFLAG_RTE    0x0008
#define E68_DFLAG_RTS    0x0010
#define E68_DFLAG_DEP_CC 0x0020
#define E68_DFLAG_68010  0x0040
#define E68_DFLAG_68020  0x0080

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
