/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/ppc405/ppc405.h                                      *
 * Created:     2003-11-07 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2009 Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2006 Lukas Ruf <ruf@lpr.ch>                         *
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

/*****************************************************************************
 * This software was developed at the Computer Engineering and Networks      *
 * Laboratory (TIK), Swiss Federal Institute of Technology (ETH) Zurich.     *
 *****************************************************************************/


#ifndef PPC405_H
#define PPC405_H 1


#include <stdint.h>


/* #define P405_DEBUG      1 */
/* #define P405_LOG_MEM    1 */
/* #define P405_LOG_OPCODE 1 */


struct p405_s;


/*****************************************************************************
 * MMU
 *****************************************************************************/

#define P405_TLB_ENTRIES 64

#define P405_XLAT_CPU     0
#define P405_XLAT_REAL    1
#define P405_XLAT_VIRTUAL 2

#define P405_MMU_MODE_X 0x0000
#define P405_MMU_MODE_R 0x0100
#define P405_MMU_MODE_W 0x0200

#define P405_TLBHI_EPN  0xfffffc00UL
#define P405_TLBHI_SIZE 0x00000380UL
#define P405_TLBHI_V    0x00000040UL
#define P405_TLBHI_E    0x00000020UL
#define P405_TLBHI_U0   0x00000010UL

#define P405_TLBLO_RPN  0xfffffc00UL
#define P405_TLBLO_EX   0x00000200UL
#define P405_TLBLO_WR   0x00000100UL
#define P405_TLBLO_ZSEL 0x000000f0UL
#define P405_TLBLO_W    0x00000008UL
#define P405_TLBLO_I    0x00000004UL
#define P405_TLBLO_M    0x00000002UL
#define P405_TLBLO_G    0x00000001UL


#define p405_get_tlbe_epn(e) ((e)->tlbhi & P405_TLBHI_EPN)
#define p405_get_tlbe_size(e) (((e)->tlbhi & P405_TLBHI_SIZE) >> 7)
#define p405_get_tlbe_sizeb(e) (0x400UL << (((e)->tlbhi & P405_TLBHI_SIZE) >> 6))
#define p405_get_tlbe_v(e) (((e)->tlbhi & P405_TLBHI_V) != 0)
#define p405_get_tlbe_e(e) (((e)->tlbhi & P405_TLBHI_E) != 0)
#define p405_get_tlbe_u0(e) (((e)->tlbhi & P405_TLBHI_U0) != 0)

#define p405_get_tlbe_rpn(e) ((e)->tlblo & P405_TLBLO_RPN)
#define p405_get_tlbe_ex(e) (((e)->tlblo & P405_TLBLO_EX) != 0)
#define p405_get_tlbe_wr(e) (((e)->tlblo & P405_TLBLO_WR) != 0)
#define p405_get_tlbe_zsel(e) (((e)->tlblo & P405_TLBLO_ZSEL) >> 4)
#define p405_get_tlbe_w(e) (((e)->tlblo & P405_TLBLO_W) != 0)
#define p405_get_tlbe_i(e) (((e)->tlblo & P405_TLBLO_I) != 0)
#define p405_get_tlbe_m(e) (((e)->tlblo & P405_TLBLO_M) != 0)
#define p405_get_tlbe_g(e) (((e)->tlblo & P405_TLBLO_G) != 0)

#define p405_get_tlbe_tid(e) ((e)->tid)


typedef struct p405_tlbe_s {
	uint32_t           tlbhi;
	uint32_t           tlblo;
	uint8_t            tid;
	uint32_t           mask;
	uint32_t           vaddr;

	unsigned char      endian;

	unsigned           idx;
	struct p405_tlbe_s *next;
} p405_tlbe_t;


typedef struct {
	p405_tlbe_t entry[P405_TLB_ENTRIES];
	p405_tlbe_t *first;

	p405_tlbe_t *tbuf_exec;
	p405_tlbe_t *tbuf_read;
	p405_tlbe_t *tbuf_write;
} p405_tlb_t;


/*****************************************************************************
 * PPC
 *****************************************************************************/

/* CRn fields */
#define P405_CR_LT 0x08
#define P405_CR_GT 0x04
#define P405_CR_EQ 0x02
#define P405_CR_SO 0x01

#define P405_CR0_LT 0x80000000UL
#define P405_CR0_GT 0x40000000UL
#define P405_CR0_EQ 0x20000000UL
#define P405_CR0_SO 0x10000000UL

/* XER fields */
#define P405_XER_SO 0x80000000UL
#define P405_XER_OV 0x40000000UL
#define P405_XER_CA 0x20000000UL

/* MSR fields */
#define P405_MSR_AP  0x02000000UL
#define P405_MSR_APE 0x00080000UL
#define P405_MSR_WE  0x00040000UL
#define P405_MSR_CE  0x00020000UL
#define P405_MSR_EE  0x00008000UL
#define P405_MSR_PR  0x00004000UL
#define P405_MSR_FP  0x00002000UL
#define P405_MSR_ME  0x00001000UL
#define P405_MSR_FE0 0x00000800UL
#define P405_MSR_DWE 0x00000400UL
#define P405_MSR_DE  0x00000200UL
#define P405_MSR_FE1 0x00000100UL
#define P405_MSR_IR  0x00000020UL
#define P405_MSR_DR  0x00000010UL

/* Exception Syndrome Register fields */
#define P405_ESR_MCI 0x80000000UL
#define P405_ESR_PIL 0x08000000UL
#define P405_ESR_PPR 0x04000000UL
#define P405_ESR_PTR 0x02000000UL
#define P405_ESR_PEU 0x01000000UL
#define P405_ESR_DST 0x00800000UL
#define P405_ESR_DIZ 0x00800000UL
#define P405_ESR_PFP 0x00080000UL
#define P405_ESR_PAP 0x00040000UL
#define P405_ESR_U0F 0x00008000UL

/* Timer Status Register fields */
#define P405_TSR_ENW 0x80000000UL
#define P405_TSR_WIS 0x40000000UL
#define P405_TSR_WRS 0x30000000UL
#define P405_TSR_PIS 0x08000000UL
#define P405_TSR_FIT 0x04000000UL

/* Timer Control Register fields */
#define P405_TCR_WP  0xc0000000UL
#define P405_TCR_WRC 0x30000000UL
#define P405_TCR_WIE 0x08000000UL
#define P405_TCR_PIE 0x04000000UL
#define P405_TCR_FP  0x03000000UL
#define P405_TCR_FIE 0x00800000UL
#define P405_TCR_ARE 0x00400000UL


/* special purpose register numbers */
#define P405_SPRN_CCR0   0x3b3
#define P405_SPRN_CTR    0x009
#define P405_SPRN_DAC1   0x3f6
#define P405_SPRN_DAC2   0x3f7
#define P405_SPRN_DBCR0  0x3f2
#define P405_SPRN_DBCR1  0x3bd
#define P405_SPRN_DBSR   0x3f0
#define P405_SPRN_DCCR   0x3fa
#define P405_SPRN_DCWR   0x3ba
#define P405_SPRN_DVC1   0x3b6
#define P405_SPRN_DVC2   0x3b7
#define P405_SPRN_DEAR   0x3d5
#define P405_SPRN_ESR    0x3d4
#define P405_SPRN_EVPR   0x3d6
#define P405_SPRN_IAC1   0x3f4
#define P405_SPRN_IAC2   0x3f5
#define P405_SPRN_IAC3   0x3b4
#define P405_SPRN_IAC4   0x3b5
#define P405_SPRN_ICCR   0x3fb
#define P405_SPRN_ICDBDR 0x3d3
#define P405_SPRN_LR     0x008
#define P405_SPRN_PID    0x3b1
#define P405_SPRN_PIT    0x3db
#define P405_SPRN_PVR    0x11f
#define P405_SPRN_SGR    0x3b9
#define P405_SPRN_SLER   0x3bb
#define P405_SPRN_SPRG0  0x110
#define P405_SPRN_SPRG1  0x111
#define P405_SPRN_SPRG2  0x112
#define P405_SPRN_SPRG3  0x113
#define P405_SPRN_SPRG4  0x114
#define P405_SPRN_SPRG5  0x115
#define P405_SPRN_SPRG6  0x116
#define P405_SPRN_SPRG7  0x117
#define P405_SPRN_SPRG4R 0x104
#define P405_SPRN_SPRG5R 0x105
#define P405_SPRN_SPRG6R 0x106
#define P405_SPRN_SPRG7R 0x107
#define P405_SPRN_SRR0   0x01a
#define P405_SPRN_SRR1   0x01b
#define P405_SPRN_SRR2   0x3de
#define P405_SPRN_SRR3   0x3df
#define P405_SPRN_SU0R   0x3bc
#define P405_SPRN_TBL    0x11c
#define P405_SPRN_TBU    0x11d
#define P405_SPRN_TCR    0x3da
#define P405_SPRN_TSR    0x3d8
#define P405_SPRN_USPRG0 0x100
#define P405_SPRN_XER    0x001
#define P405_SPRN_ZPR    0x3b0

#define P405_TBRN_TBL    0x10c
#define P405_TBRN_TBU    0x10d


#define p405_get_gpr(c, n) ((c)->gpr[(n)])
#define p405_set_gpr(c, n, v) do { (c)->gpr[(n)] = (v); } while (0)

#define p405_set_bits(var, bits, val) do { \
		if (val) (var) |= (bits); else (var) &= ~(bits); \
	} while (0)

/* register read macros */
#define p405_get_cr(c) ((c)->cr)
#define p405_get_crf(c, n) (((c)->cr >> (28 - 4 * (n))) & 0x0f)
#define p405_get_cr_lt(c, n) (((c)->cr & (P405_CR0_LT >> (4 * (n)))) != 0)
#define p405_get_cr_gt(c, n) (((c)->cr & (P405_CR0_GT >> (4 * (n)))) != 0)
#define p405_get_cr_eq(c, n) (((c)->cr & (P405_CR0_EQ >> (4 * (n)))) != 0)
#define p405_get_cr_so(c, n) (((c)->cr & (P405_CR0_SO >> (4 * (n)))) != 0)
#define p405_get_ctr(c) ((c)->ctr)
#define p405_get_dbcr0(c) ((c)->dbcr0)
#define p405_get_dbcr1(c) ((c)->dbcr1)
#define p405_get_dbsr(c) ((c)->dbsr)
#define p405_get_dccr(c) ((c)->dccr)
#define p405_get_dcwr(c) ((c)->dcwr)
#define p405_get_dear(c) ((c)->dear)
#define p405_get_esr(c) ((c)->esr)
#define p405_get_evpr(c) ((c)->evpr)
#define p405_get_iccr(c) ((c)->iccr)
#define p405_get_lr(c) ((c)->lr)
#define p405_get_msr(c) ((c)->msr)
#define p405_get_msr_ee(c) (((c)->msr & P405_MSR_EE) != 0)
#define p405_get_msr_pr(c) (((c)->msr & P405_MSR_PR) != 0)
#define p405_get_msr_ir(c) (((c)->msr & P405_MSR_IR) != 0)
#define p405_get_msr_dr(c) (((c)->msr & P405_MSR_DR) != 0)
#define p405_get_pc(c) ((c)->pc)
#define p405_get_pid(c) ((c)->pid)
#define p405_get_pit(c, n) ((c)->pit[(n) & 0x01])
#define p405_get_pvr(c) ((c)->pvr)
#define p405_get_sprg(c, n) ((c)->sprg[(n) & 0x07])
#define p405_get_srr(c, n) ((c)->srr[(n) & 0x03])
#define p405_get_tbl(c) ((c)->tbl)
#define p405_get_tbu(c) ((c)->tbu)
#define p405_get_tcr(c) ((c)->tcr)
#define p405_get_tsr(c) ((c)->tsr)
#define p405_get_xer(c) ((c)->xer)
#define p405_get_xer_so(c) (((c)->xer & P405_XER_SO) != 0)
#define p405_get_xer_ov(c) (((c)->xer & P405_XER_OV) != 0)
#define p405_get_xer_ca(c) (((c)->xer & P405_XER_CA) != 0)
#define p405_get_zpr(c) ((c)->zpr)
#define p405_get_zprf(c, n) (((c)->zpr >> (30 - 2 * (n))) & 0x03)

/* register write macros */
#define p405_set_cr(c, v) do { (c)->cr = (v); } while (0)
#define p405_set_crf(c, n, v) do { \
	(c)->cr &= ~(0x0fUL << (28 - 4 * (n))); \
	(c)->cr |= (v) << (28 - 4 * (n)); } while (0)
#define p405_set_cr_lt(c, n, v) p405_set_bits ((c)->cr, P405_CR0_LT >> (4 * (n)), (v))
#define p405_set_cr_gt(c, n, v) p405_set_bits ((c)->cr, P405_CR0_GT >> (4 * (n)), (v))
#define p405_set_cr_eq(c, n, v) p405_set_bits ((c)->cr, P405_CR0_EQ >> (4 * (n)), (v))
#define p405_set_cr_so(c, n, v) p405_set_bits ((c)->cr, P405_CR0_SO >> (4 * (n)), (v))
#define p405_set_ctr(c, v) do { (c)->ctr = (v); } while (0)
#define p405_set_dbcr0(c, v) do { (c)->dbcr0 = (v); } while (0)
#define p405_set_dbcr1(c, v) do { (c)->dbcr1 = (v); } while (0)
#define p405_set_dbsr(c, v) do { (c)->dbsr = (v); } while (0)
#define p405_set_dccr(c, v) do { (c)->dccr = (v); } while (0)
#define p405_set_dcwr(c, v) do { (c)->dcwr = (v); } while (0)
#define p405_set_dear(c, v) do { (c)->dear = (v); } while (0)
#define p405_set_esr(c, v) do { (c)->esr = (v); } while (0)
#define p405_set_evpr(c, v) do { (c)->evpr = (v); } while (0)
#define p405_set_iccr(c, v) do { (c)->iccr = (v); } while (0)
#define p405_set_lr(c, v) do { (c)->lr = (v); } while (0)
#define p405_set_msr(c, v) do { (c)->msr = (v); } while (0)
#define p405_set_msr_bits(c, bits, v) p405_set_bits ((c)->msr, bits, v)
#define p405_set_msr_ee(c, v) p405_set_msr_bits (c, P405_MSR_EE, v)
#define p405_set_msr_pr(c, v) p405_set_msr_bits (c, P405_MSR_PR, v)
#define p405_set_msr_ir(c, v) p405_set_msr_bits (c, P405_MSR_IR, v)
#define p405_set_msr_dr(c, v) p405_set_msr_bits (c, P405_MSR_DR, v)
#define p405_set_pc(c, v) do { (c)->pc = (v); } while (0)
#define p405_set_pid(c, v) do { (c)->pid = (v) & 0xff; } while (0)
#define p405_set_pit(c, n, v) do { (c)->pit[(n) & 0x01] = (v); } while (0)
#define p405_set_pvr(c, v) do { (c)->pvr = (v); } while (0)
#define p405_set_sprg(c, n, v) do { (c)->sprg[(n) & 0x07] = (v); } while (0)
#define p405_set_srr(c, n, v) do { (c)->srr[(n) & 0x03] = (v); } while (0)
#define p405_set_tbl(c, v) do { (c)->tbl = (v); } while (0)
#define p405_set_tbu(c, v) do { (c)->tbu = (v); } while (0)
#define p405_set_tcr(c, v) do { (c)->tcr = (v); } while (0)
#define p405_set_tsr(c, v) do { (c)->tsr = (v); } while (0)
#define p405_set_xer(c, v) do { (c)->xer = (v); } while (0)
#define p405_set_xer_bits(c, bits, v) p405_set_bits ((c)->xer, bits, v)
#define p405_set_xer_so(c, v) p405_set_xer_bits (c, P405_XER_SO, v)
#define p405_set_xer_ov(c, v) p405_set_xer_bits (c, P405_XER_OV, v)
#define p405_set_xer_ca(c, v) p405_set_xer_bits (c, P405_XER_CA, v)
#define p405_set_zpr(c, v) do { (c)->zpr = (v); } while (0)


typedef unsigned char (*p405_get_uint8_f) (void *ext, unsigned long addr);
typedef unsigned short (*p405_get_uint16_f) (void *ext, unsigned long addr);
typedef unsigned long (*p405_get_uint32_f) (void *ext, unsigned long addr);

typedef void (*p405_set_uint8_f) (void *ext, unsigned long addr, unsigned char val);
typedef void (*p405_set_uint16_f) (void *ext, unsigned long addr, unsigned short val);
typedef void (*p405_set_uint32_f) (void *ext, unsigned long addr, unsigned long val);

typedef void (*p405_opcode_f) (struct p405_s *c);


typedef struct {
	p405_opcode_f op[64];
	p405_opcode_f op13[1024];
	p405_opcode_f op1f[1024];
} p405_opcode_map_t;


typedef struct p405_s {
	void               *mem_ext;

	p405_get_uint8_f   get_uint8;
	p405_get_uint16_f  get_uint16;
	p405_get_uint32_f  get_uint32;

	p405_set_uint8_f   set_uint8;
	p405_set_uint16_f  set_uint16;
	p405_set_uint32_f  set_uint32;

	unsigned char      *ram;
	unsigned long      ram_cnt;

	void               *dcr_ext;
	p405_get_uint32_f  get_dcr;
	p405_set_uint32_f  set_dcr;

	void *log_ext;
	void (*log_opcode) (void *ext, unsigned long ir);
	void (*log_undef) (void *ext, unsigned long ir);
	void (*log_exception) (void *ext, unsigned long offs);
	void (*log_mem) (void *ext, unsigned mode,
		unsigned long raddr, unsigned long vaddr, unsigned long val);

	void               *hook_ext;
	void               (*hook) (void *ext, unsigned long ir);

	uint32_t           pc;
	uint32_t           gpr[32];

	uint32_t           cr;
	uint32_t           ctr;
	uint32_t           dbcr0;
	uint32_t           dbcr1;
	uint32_t           dbsr;
	uint32_t           dccr;
	uint32_t           dcwr;
	uint32_t           dear;
	uint32_t           esr;
	uint32_t           evpr;
	uint32_t           iccr;
	uint32_t           lr;
	uint32_t           msr;
	uint32_t           pid;
	uint32_t           pit[2];
	uint32_t           pvr;
	uint32_t           sprg[8];
	uint32_t           srr[4];
	uint32_t           tbl;
	uint32_t           tbu;
	uint32_t           tcr;
	uint32_t           tsr;
	uint32_t           xer;
	uint32_t           zpr;

	uint32_t           ir;

	char               reserve;

	unsigned char      interrupt;

	p405_tlb_t         tlb;

	unsigned           timer_scale;

	unsigned long      delay;

	unsigned long long oprcnt;
	unsigned long long clkcnt;

	p405_opcode_map_t  opcodes;
} p405_t;


/*!***************************************************************************
 * @short  Get the index of the TLB entry corresponding to an EA
 * @param  c  The cpu context
 * @param  ea The virtual address
 * @return The entry index or P405_TLB_ENTRIES if there is no entry
 *****************************************************************************/
unsigned p405_get_tlb_index (p405_t *c, uint32_t ea);

/*!***************************************************************************
 * @short  Get the TLB entry corresponding to an EA
 * @param  c  The cpu context
 * @param  ea The virtual address
 * @return The entry or NULL if there is no entry
 *****************************************************************************/
p405_tlbe_t *p405_get_tlb_entry_ea (p405_t *c, uint32_t ea);

/*!***************************************************************************
 * @short  Get a TLB entry by index
 * @param  c   The cpu context
 * @param  idx The TLB entry index
 * @return The entry or NULL if the index is out of range
 *****************************************************************************/
p405_tlbe_t *p405_get_tlb_entry_idx (p405_t *c, unsigned idx);

/*!***************************************************************************
 * @short  Get the number of TLB entries
 * @param  c The cpu context
 * @return The number of TLB entries
 *****************************************************************************/
unsigned p405_get_tlb_entry_cnt (p405_t *c);

/*!***************************************************************************
 * @short  Translate a virtual address
 * @param  c    The cpu context
 * @retval ea   On input the virtual address, on output the real address
 * @retval e    Zero if the address is big-endian, non-zero if it is
 *              little-endian
 * @param  xlat The translation mode (P405_XLAT_VIRTUAL, P405_XLAT_REAL or
 *              P405_XLAT_CPU)
 * @return Zero if successful, non-zero otherwise
 *****************************************************************************/
int p405_translate (p405_t *c, uint32_t *ea, int *e, unsigned xlat);

/*!***************************************************************************
 * @short Get byte from translated address
 *****************************************************************************/
int p405_get_xlat8 (p405_t *c, uint32_t addr, unsigned xlat, uint8_t *val);

int p405_get_xlat16 (p405_t *c, uint32_t addr, unsigned xlat, uint16_t *val);
int p405_get_xlat32 (p405_t *c, uint32_t addr, unsigned xlat, uint32_t *val);

int p405_set_xlat8 (p405_t *c, uint32_t addr, unsigned xlat, uint8_t val);
int p405_set_xlat16 (p405_t *c, uint32_t addr, unsigned xlat, uint16_t val);
int p405_set_xlat32 (p405_t *c, uint32_t addr, unsigned xlat, uint32_t val);


/*!***************************************************************************
 * @short Initialize a PowerPC 405 context struct
 *****************************************************************************/
void p405_init (p405_t *c);

/*!***************************************************************************
 * @short  Create and initialize a PowerPC 405 context struct
 * @return The ppc405 struct or NULL on error
 *****************************************************************************/
p405_t *p405_new (void);

/*!***************************************************************************
 * @short Free the resources used by a ppc405 struct
 *****************************************************************************/
void p405_free (p405_t *c);

/*!***************************************************************************
 * @short Delete a ppc405 struct
 *****************************************************************************/
void p405_del (p405_t *c);

void p405_set_mem_fct (p405_t *c, void *ext,
	void *get8, void *get16, void *get32,
	void *set8, void *set16, void *set32
);

void p405_set_ram (p405_t *c, unsigned char *ram, unsigned long cnt);

/*!***************************************************************************
 * @short Set the DCR access functions
 * @param c The cpu context
 *****************************************************************************/
void p405_set_dcr_fct (p405_t *c, void *ext, void *get, void *set);

/*!***************************************************************************
 * @short Set the hook function
 * @param c The cpu context
 *****************************************************************************/
void p405_set_hook_fct (p405_t *c, void *ext, void *fct);


/*!***************************************************************************
 * @short Set the internal timer scale
 * @param c     The cpu context
 * @param scale The timer scale. The timer runs this much faster than the cpu.
 *****************************************************************************/
void p405_set_timer_scale (p405_t *c, unsigned scale);

/*!***************************************************************************
 * @short Get the number of executed instructions
 *****************************************************************************/
unsigned long long p405_get_opcnt (p405_t *c);

/*!***************************************************************************
 * @short Get the number of executed clock cycles
 *****************************************************************************/
unsigned long long p405_get_clkcnt (p405_t *c);

/*!***************************************************************************
 * @short Get the current delay before the next instruction
 *****************************************************************************/
unsigned long p405_get_delay (p405_t *c);


int p405_get_reg (p405_t *c, const char *reg, unsigned long *val);
int p405_set_reg (p405_t *c, const char *reg, unsigned long val);


/*!***************************************************************************
 * @short Get a byte from a physical address
 *****************************************************************************/
uint8_t p405_get_mem8 (p405_t *c, uint32_t addr);

/*!***************************************************************************
 * @short Get a 16 bit word from a physical address
 *****************************************************************************/
uint16_t p405_get_mem16 (p405_t *c, uint32_t addr);

/*!***************************************************************************
 * @short Get a 32 bit word from a physical address
 *****************************************************************************/
uint32_t p405_get_mem32 (p405_t *c, uint32_t addr);

/*!***************************************************************************
 * @short Set an 8 bit word at a physical address
 *****************************************************************************/
void p405_set_mem8 (p405_t *c, uint32_t addr, uint8_t val);

/*!***************************************************************************
 * @short Set a 16 bit word at a physical address
 *****************************************************************************/
void p405_set_mem16 (p405_t *c, uint32_t addr, uint16_t val);

/*!***************************************************************************
 * @short Set a 32 bit word at a physical address
 *****************************************************************************/
void p405_set_mem32 (p405_t *c, uint32_t addr, uint32_t val);

unsigned long p405_get_dcr (p405_t *c, unsigned long dcrn);
void p405_set_dcr (p405_t *c, unsigned long dcrn, unsigned long val);

void p405_undefined (p405_t *c);

/*!***************************************************************************
 * @short The external interrupt input signal
 *****************************************************************************/
void p405_interrupt (p405_t *c, unsigned char val);

void p405_reset (p405_t *c);
void p405_execute (p405_t *c);
void p405_clock_tb (p405_t *c, unsigned long n);
void p405_clock (p405_t *c, unsigned long n);


/*****************************************************************************
 * disasm
 *****************************************************************************/

#define P405_DFLAG_PRIV 0x0001
#define P405_DFLAG_TLBM 0x0002
#define P405_DFLAG_CALL 0x0100
#define P405_DFLAG_RFI  0x0200

typedef struct {
	unsigned flags;

	uint32_t pc;
	uint32_t ir;

	unsigned argn;

	char     op[64];
	char     arg1[64];
	char     arg2[64];
	char     arg3[64];
	char     arg4[64];
	char     arg5[64];
} p405_disasm_t;


typedef void (*p405_disasm_f) (p405_disasm_t *dis);


void p405_disasm (p405_disasm_t *dis, uint32_t pc, uint32_t ir);
void p405_disasm_mem (p405_t *c, p405_disasm_t *dis, uint32_t pc, unsigned xlat);


#endif
