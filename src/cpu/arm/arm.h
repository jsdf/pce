/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/arm/arm.h                                            *
 * Created:     2004-11-03 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2009 Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2006 Lukas Ruf <ruf@lpr.ch>                         *
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


#ifndef ARM_H
#define ARM_H 1


#include <stdint.h>


/*****************************************************************************
 * ARM
 *****************************************************************************/

/* PSR fields */
#define ARM_PSR_N (1UL << 31)
#define ARM_PSR_Z (1UL << 30)
#define ARM_PSR_C (1UL << 29)
#define ARM_PSR_V (1UL << 28)
#define ARM_PSR_CC 0xf0000000UL
#define ARM_PSR_Q (1UL << 27)
#define ARM_PSR_I (1UL << 7)
#define ARM_PSR_F (1UL << 6)
#define ARM_PSR_T (1UL << 5)
#define ARM_PSR_M 0x1fUL


#define ARM_MODE_USR 0x10
#define ARM_MODE_FIQ 0x11
#define ARM_MODE_IRQ 0x12
#define ARM_MODE_SVC 0x13
#define ARM_MODE_ABT 0x17
#define ARM_MODE_UND 0x1b
#define ARM_MODE_SYS 0x1f


/* enable big endian mode */
#define ARM_FLAG_BIGENDIAN 1
#define ARM_FLAG_T         2		/* thumb mode */
#define ARM_FLAG_M         4		/* multiply instructions */
#define ARM_FLAG_E         8		/* enhanced dsp instructions */
#define ARM_FLAG_CPAR      16		/* coprocessor access register */
#define ARM_FLAG_XSCALE    (ARM_FLAG_T | ARM_FLAG_M | ARM_FLAG_E | ARM_FLAG_CPAR)
#define ARM_FLAG_ALL       0xffff


#define ARM_C15_ID_XSCALE_V5TE 0x69052000UL
#define ARM_C15_ID_IXP2400     0x69054190UL

#define ARM_C15_ID   0x69052000UL
#define ARM_C15_CR_M 0x00000001UL
#define ARM_C15_CR_A 0x00000002UL
#define ARM_C15_CR_C 0x00000004UL
#define ARM_C15_CR_W 0x00000008UL
#define ARM_C15_CR_P 0x00000010UL
#define ARM_C15_CR_D 0x00000020UL
#define ARM_C15_CR_L 0x00000040UL
#define ARM_C15_CR_B 0x00000080UL
#define ARM_C15_CR_S 0x00000100UL
#define ARM_C15_CR_R 0x00000200UL
#define ARM_C15_CR_F 0x00000400UL
#define ARM_C15_CR_Z 0x00000800UL
#define ARM_C15_CR_I 0x00001000UL
#define ARM_C15_CR_V 0x00002000UL


#define ARM_XLAT_CPU     0
#define ARM_XLAT_REAL    1
#define ARM_XLAT_VIRTUAL 2


#define ARM_REG_ALT_CNT 24
#define ARM_SPSR_CNT    6

#define arm_set_bits(var, bits, val) do { \
		if (val) (var) |= (bits); else (var) &= ~(bits); \
	} while (0)

#define arm_get_gpr(c, n) ((c)->reg[(n)])
#define arm_get_pc(c) ((c)->reg[15])
#define arm_get_lr(c) ((c)->reg[14])
#define arm_get_cpsr(c) ((c)->cpsr)
#define arm_get_spsr(c) ((c)->spsr)
#define arm_get_last_pc(c) ((c)->lastpc[1])

#define arm_set_gpr(c, n, v) do { (c)->reg[(n)] = (v); } while (0)
#define arm_set_pc(c, v) do { (c)->reg[15] = (v); } while (0)
#define arm_set_lr(c, v) do { (c)->reg[14] = (v); } while (0)
#define arm_set_cpsr(c, v) do { (c)->cpsr = (v); } while (0)
#define arm_set_spsr(c, v) do { (c)->spsr = (v); } while (0)

#define arm_get_cc_n(c) (((c)->cpsr & ARM_PSR_N) != 0)
#define arm_get_cc_z(c) (((c)->cpsr & ARM_PSR_Z) != 0)
#define arm_get_cc_c(c) (((c)->cpsr & ARM_PSR_C) != 0)
#define arm_get_cc_v(c) (((c)->cpsr & ARM_PSR_V) != 0)
#define arm_get_cc_q(c) (((c)->cpsr & ARM_PSR_Q) != 0)

#define arm_get_cpsr_i(c) (((c)->cpsr & ARM_PSR_I) != 0)
#define arm_get_cpsr_f(c) (((c)->cpsr & ARM_PSR_F) != 0)
#define arm_get_cpsr_t(c) (((c)->cpsr & ARM_PSR_T) != 0)
#define arm_get_cpsr_m(c) ((c)->cpsr & ARM_PSR_M)

#define arm_set_cc_n(c, v) arm_set_bits ((c)->cpsr, ARM_PSR_N, (v))
#define arm_set_cc_z(c, v) arm_set_bits ((c)->cpsr, ARM_PSR_Z, (v))
#define arm_set_cc_c(c, v) arm_set_bits ((c)->cpsr, ARM_PSR_C, (v))
#define arm_set_cc_v(c, v) arm_set_bits ((c)->cpsr, ARM_PSR_V, (v))
#define arm_set_cc_q(c, v) arm_set_bits ((c)->cpsr, ARM_PSR_Q, (v))

#define arm_set_cpsr_i(c, v) arm_set_bits ((c)->cpsr, ARM_PSR_I, (v))
#define arm_set_cpsr_f(c, v) arm_set_bits ((c)->cpsr, ARM_PSR_F, (v))
#define arm_set_cpsr_t(c, v) arm_set_bits ((c)->cpsr, ARM_PSR_T, (v))
#define arm_set_cpsr_m(c, v) do { \
	(c)->cpsr &= ~0x1fUL; \
	(c)->cpsr |= (v) & 0x1f; \
	} while (0)

#define arm_get_mmu(c) (&(c)->copr15)


struct arm_s;
struct arm_copr_s;


typedef unsigned char (*arm_get_uint8_f) (void *ext, unsigned long addr);
typedef unsigned short (*arm_get_uint16_f) (void *ext, unsigned long addr);
typedef unsigned long (*arm_get_uint32_f) (void *ext, unsigned long addr);

typedef void (*arm_set_uint8_f) (void *ext, unsigned long addr, unsigned char val);
typedef void (*arm_set_uint16_f) (void *ext, unsigned long addr, unsigned short val);
typedef void (*arm_set_uint32_f) (void *ext, unsigned long addr, unsigned long val);

typedef void (*arm_opcode_f) (struct arm_s *c);


/*****************************************************************************
 * @short The ARM coprocessor context
 *****************************************************************************/
typedef struct arm_copr_s {
	unsigned copr_idx;

	int      (*reset) (struct arm_s *c, struct arm_copr_s *p);
	int      (*exec) (struct arm_s *c, struct arm_copr_s *p);

	void     *ext;
} arm_copr_t;


typedef struct {
	int      valid;

	uint32_t vaddr;
	uint32_t vmask;

	uint32_t raddr;
	uint32_t rmask;
} arm_tbuf_t;


typedef struct {
	arm_copr_t copr;

	arm_tbuf_t tbuf_exec;
	arm_tbuf_t tbuf_read;
	arm_tbuf_t tbuf_write;

	uint32_t   reg[16];

	uint32_t   cache_type;
	uint32_t   auxiliary_control;
} arm_copr15_t;


typedef struct {
	arm_copr_t copr;

	uint32_t   cclkcfg;
	uint32_t   pwrmode;
} arm_copr14_t;


/*****************************************************************************
 * @short The ARM CPU context
 *****************************************************************************/
typedef struct arm_s {
	unsigned           flags;

	void               *mem_ext;

	arm_get_uint8_f    get_uint8;
	arm_get_uint16_f   get_uint16;
	arm_get_uint32_f   get_uint32;

	arm_set_uint8_f    set_uint8;
	arm_set_uint16_f   set_uint16;
	arm_set_uint32_f   set_uint32;

	unsigned char      *ram;
	unsigned long      ram_cnt;

	void               *log_ext;
	int                (*log_opcode) (void *ext, unsigned long ir);
	void               (*log_undef) (void *ext, unsigned long ir);
	void               (*log_exception) (void *ext, unsigned long addr);

	uint32_t           cpsr;

	uint32_t           spsr;
	uint32_t           spsr_alt[ARM_SPSR_CNT];

	uint32_t           reg[16];
	uint32_t           reg_alt[ARM_REG_ALT_CNT];

	uint32_t           lastpc[2];

	/* the current register mapping */
	unsigned           reg_map;

	arm_copr_t         *copr[16];

	arm_copr14_t       copr14;

	/* system control coprocessor */
	arm_copr15_t       copr15;

	uint32_t           ir;

	uint32_t           exception_base;

	/* this reflects the B bit in copr15 reg 1 */
	int                bigendian;

	/* cpu is in a privileged mode */
	int                privileged;

	unsigned char      irq;
	unsigned char      fiq;
	unsigned char      irq_or_fiq;

	unsigned long      delay;

	unsigned long long oprcnt;
	unsigned long long clkcnt;

	arm_opcode_f       opcodes[256];
} arm_t;



/*****************************************************************************
 * MMU
 *****************************************************************************/

int arm_translate_extern (arm_t *c, uint32_t *addr, unsigned xlat,
	unsigned *domn, unsigned *perm
);

int arm_get_mem8 (arm_t *c, uint32_t addr, unsigned xlat, uint8_t *val);
int arm_get_mem16 (arm_t *c, uint32_t addr, unsigned xlat, uint16_t *val);
int arm_get_mem32 (arm_t *c, uint32_t addr, unsigned xlat, uint32_t *val);

int arm_set_mem8 (arm_t *c, uint32_t addr, unsigned xlat, uint8_t val);
int arm_set_mem16 (arm_t *c, uint32_t addr, unsigned xlat, uint16_t val);
int arm_set_mem32 (arm_t *c, uint32_t addr, unsigned xlat, uint32_t val);


/*!***************************************************************************
 * @short Initialize an ARM context
 *****************************************************************************/
void arm_init (arm_t *c);

/*!***************************************************************************
 * @short  Create and initialize an ARM context
 * @return The ARM context or NULL on error
 *****************************************************************************/
arm_t *arm_new (void);

/*!***************************************************************************
 * @short Free the resources used by an ARM context
 *****************************************************************************/
void arm_free (arm_t *c);

/*!***************************************************************************
 * @short Delete an ARM context
 *****************************************************************************/
void arm_del (arm_t *c);

/*!***************************************************************************
 * @short Set the memory access functions
 *****************************************************************************/
void arm_set_mem_fct (arm_t *c, void *ext,
	void *get8, void *get16, void *get32,
	void *set8, void *set16, void *set32
);

void arm_set_ram (arm_t *c, unsigned char *ram, unsigned long cnt);


/*!***************************************************************************
 * @short  Get CPU flags
 * @short  c     The cpu context
 * @short  flags The flags to get
 * @return The current cpu flags masked by flags.
 *****************************************************************************/
unsigned arm_get_flags (const arm_t *c, unsigned flags);

/*!***************************************************************************
 * @short Set CPU flags
 * @short c     The cpu context
 * @short flags The flags to set (ARM_FLAG_*)
 * @short val   Set the flags if non-zero, clear them otherwise
 *
 * Flags must be set before arm_reset() is called.
 *****************************************************************************/
void arm_set_flags (arm_t *c, unsigned flags, int val);

unsigned long arm_get_id (arm_t *c);
void arm_set_id (arm_t *c, unsigned long id);


int arm_get_reg (arm_t *c, const char *reg, unsigned long *val);
int arm_set_reg (arm_t *c, const char *reg, unsigned long val);


/*!***************************************************************************
 * @short Get the number of executed instructions
 *****************************************************************************/
unsigned long long arm_get_opcnt (arm_t *c);

/*!***************************************************************************
 * @short Get the number of clock cycles
 *****************************************************************************/
unsigned long long arm_get_clkcnt (arm_t *c);

/*!***************************************************************************
 * @short Get the previous instruction delay
 *****************************************************************************/
unsigned long arm_get_delay (arm_t *c);


/*!***************************************************************************
 * @short Initialize a coprocessor context
 *****************************************************************************/
void arm_copr_init (arm_copr_t *p);

/*!***************************************************************************
 * @short Free a coprocessor context
 *****************************************************************************/
void arm_copr_free (arm_copr_t *p);

/*!***************************************************************************
 * @short Check if coprocessor is present and enabled
 * @param i The coprocessor index
 * @return Zero if coprocessor i is present and enabled, non-zero otherwise.
 *****************************************************************************/
int arm_copr_check (arm_t *c, unsigned i);

/*!***************************************************************************
 * @short Set a coprocessor
 * @param c The ARM CPU context
 * @param i The coprocessor index
 * @param p The coprocessor context
 *
 * The coprocessor context p is owned by the caller.
 *****************************************************************************/
void arm_set_copr (arm_t *c, unsigned i, arm_copr_t *p);


void cp14_init (arm_copr14_t *c);
void cp14_free (arm_copr14_t *p);
arm_copr_t *cp14_new (void);
void cp14_del (arm_copr14_t *p);


/*!***************************************************************************
 * @short Initialize a system control coprocessor context
 *****************************************************************************/
void cp15_init (arm_copr15_t *c);
void cp15_free (arm_copr15_t *p);
arm_copr_t *cp15_new (void);
void cp15_del (arm_copr15_t *p);


void arm_set_reg_map (arm_t *arm, unsigned mode);


/*!***************************************************************************
 * @short Reset an arm cpu core
 *****************************************************************************/
void arm_reset (arm_t *c);

/*!***************************************************************************
 * @short Execute an exception
 *****************************************************************************/
void arm_exception (arm_t *c, uint32_t addr, uint32_t ret, unsigned mode);

void arm_exception_reset (arm_t *c);

void arm_exception_undefined (arm_t *c);

void arm_exception_swi (arm_t *c);

void arm_exception_prefetch_abort (arm_t *c);

void arm_exception_data_abort (arm_t *c);

void arm_exception_irq (arm_t *c);

void arm_exception_fiq (arm_t *c);

/*!***************************************************************************
 * @short The external interrupt input signal
 *****************************************************************************/
void arm_set_irq (arm_t *c, unsigned char val);

/*!***************************************************************************
 * @short The external fast interrupt input signal
 *****************************************************************************/
void arm_set_fiq (arm_t *c, unsigned char val);

/*!***************************************************************************
 * @short Execute one instruction
 *****************************************************************************/
void arm_execute (arm_t *c);

/*!***************************************************************************
 * @short Clock an ARM cpu core
 *****************************************************************************/
void arm_clock (arm_t *c, unsigned long n);


/*****************************************************************************
 * disasm
 *****************************************************************************/

#define ARM_DFLAG_PRIV 0x0001
#define ARM_DFLAG_TLBM 0x0002
#define ARM_DFLAG_CALL 0x0100

typedef struct {
	unsigned flags;

	uint32_t pc;
	uint32_t ir;

	unsigned argn;

	char     op[64];
	char     arg[8][64];
} arm_dasm_t;


typedef void (*arm_dasm_f) (arm_dasm_t *da);


void arm_dasm (arm_dasm_t *dis, uint32_t pc, uint32_t ir);
void arm_dasm_mem (arm_t *c, arm_dasm_t *da, uint32_t pc, unsigned xlat);


#endif
