/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/cpu/arm/arm.h                                          *
 * Created:       2004-11-03 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-11-09 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004 Hampa Hug <hampa@hampa.ch>                        *
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


#ifndef ARM_H
#define ARM_H 1


#if defined (HAVE_STDINT_H)
#include <stdint.h>
#elif defined (HAVE_INTTYPES_H)
#include <inttypes.h>
#else
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;
#endif


struct arm_s;


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
#define ARM_MODE_UND 0x1a
#define ARM_MODE_SYS 0x1f


#define ARM_XLAT_CPU     0
#define ARM_XLAT_REAL    1
#define ARM_XLAT_VIRTUAL 2


#define ARM_REG_ALT_CNT 24
#define ARM_SPSR_CNT    6

#define arm_set_bits(var, bits, val) do { \
    if (val) (var) |= (bits); else (var) &= ~(bits); \
  } while (0)

#define arm_get_reg(c, n) ((c)->reg[(n)])
#define arm_get_pc(c) ((c)->reg[15])
#define arm_get_lr(c) ((c)->reg[14])
#define arm_get_cpsr(c) ((c)->cpsr)
#define arm_get_spsr(c) ((c)->spsr)

#define arm_set_reg(c, n, v) do { (c)->reg[(n)] = (v); } while (0)
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


typedef unsigned char (*arm_get_uint8_f) (void *ext, unsigned long addr);
typedef unsigned short (*arm_get_uint16_f) (void *ext, unsigned long addr);
typedef unsigned long (*arm_get_uint32_f) (void *ext, unsigned long addr);

typedef void (*arm_set_uint8_f) (void *ext, unsigned long addr, unsigned char val);
typedef void (*arm_set_uint16_f) (void *ext, unsigned long addr, unsigned short val);
typedef void (*arm_set_uint32_f) (void *ext, unsigned long addr, unsigned long val);

typedef void (*arm_opcode_f) (struct arm_s *c);


typedef struct arm_s {
  void               *mem_ext;

  arm_get_uint8_f    get_uint8;
  arm_get_uint16_f   get_uint16;
  arm_get_uint32_f   get_uint32;

  arm_set_uint8_f    set_uint8;
  arm_set_uint16_f   set_uint16;
  arm_set_uint32_f   set_uint32;

  void               *log_ext;
  void               (*log_opcode) (void *ext, unsigned long ir);
  void               (*log_undef) (void *ext, unsigned long ir);
  void               (*log_exception) (void *ext, unsigned long addr);

  uint32_t           cpsr;

  uint32_t           spsr;
  uint32_t           spsr_alt[ARM_SPSR_CNT];

  uint32_t           reg[16];
  uint32_t           reg_alt[ARM_REG_ALT_CNT];

  unsigned           reg_map;

  uint32_t           ir;

  unsigned char      interrupt;

  unsigned long      delay;

  unsigned long long oprcnt;
  unsigned long long clkcnt;

  arm_opcode_f       opcodes[256];
} arm_t;



/*****************************************************************************
 * MMU
 *****************************************************************************/

int arm_get_mem8 (arm_t *c, uint32_t addr, unsigned xlat, uint8_t *val);
int arm_get_mem16 (arm_t *c, uint32_t addr, unsigned xlat, uint16_t *val);
int arm_get_mem32 (arm_t *c, uint32_t addr, unsigned xlat, uint32_t *val);

int arm_set_mem8 (arm_t *c, uint32_t addr, unsigned xlat, uint8_t val);
int arm_set_mem16 (arm_t *c, uint32_t addr, unsigned xlat, uint16_t val);
int arm_set_mem32 (arm_t *c, uint32_t addr, unsigned xlat, uint32_t val);


/*!***************************************************************************
 * @short Initialize an arm context struct
 *****************************************************************************/
void arm_init (arm_t *c);

/*!***************************************************************************
 * @short  Create and initialize an arm context struct
 * @return The arm struct or NULL on error
 *****************************************************************************/
arm_t *arm_new (void);

/*!***************************************************************************
 * @short Free the resources used by an arm struct
 *****************************************************************************/
void arm_free (arm_t *c);

/*!***************************************************************************
 * @short Delete an arm struct
 *****************************************************************************/
void arm_del (arm_t *c);

void arm_set_mem_fct (arm_t *c, void *ext,
  void *get8, void *get16, void *get32,
  void *set8, void *set16, void *set32
);

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


void arm_set_reg_map (arm_t *arm, unsigned mode);

/*!***************************************************************************
 * @short Reset an arm cpu core
 *****************************************************************************/
void arm_reset (arm_t *c);

/*!***************************************************************************
 * @short Log an undefined instruction
 *****************************************************************************/
void arm_undefined (arm_t *c);

/*!***************************************************************************
 * @short Execute an exception
 *****************************************************************************/
void arm_exception (arm_t *c, uint32_t addr, uint32_t ret, unsigned mode);

void arm_exception_reset (arm_t *c);

void arm_exception_undefined (arm_t *c);

void arm_exception_swi (arm_t *c);

void arm_exception_prefetch_abort (arm_t *c);

void arm_exception_data_abort (arm_t *c);

/*!***************************************************************************
 * @short The external interrupt input signal
 *****************************************************************************/
void arm_interrupt (arm_t *c, unsigned char val);

/*!***************************************************************************
 * @short Execute one instruction
 *****************************************************************************/
void arm_execute (arm_t *c);

/*!***************************************************************************
 * @short Clock a sparc32 cpu core
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
