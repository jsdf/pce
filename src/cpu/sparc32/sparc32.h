/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/sparc32/sparc32.h                                    *
 * Created:     2004-09-27 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef SPARC32_H
#define SPARC32_H 1


#include <stdint.h>


struct sparc32_s;


/*****************************************************************************
 * sparc32
 *****************************************************************************/

/* PSR fields */
#define S32_PSR_CWP  0x0000001fUL
#define S32_PSR_ET   0x00000020UL
#define S32_PSR_PS   0x00000040UL
#define S32_PSR_S    0x00000080UL
#define S32_PSR_PIL  0x00000700UL
#define S32_PSR_EF   0x00000800UL
#define S32_PSR_EC   0x00001000UL
#define S32_PSR_ICC  0x00f00000UL
#define S32_PSR_VER  0x0f000000UL
#define S32_PSR_IMPL 0xf0000000UL

#define S32_PSR_C (1UL << 20)
#define S32_PSR_V (1UL << 21)
#define S32_PSR_Z (1UL << 22)
#define S32_PSR_N (1UL << 23)


#define S32_TBR_TT   0x000003fcUL


#define S32_TRAP_RESET                        0x00
#define S32_TRAP_INSTRUCTION_ACCESS_EXCEPTION 0x01
#define S32_TRAP_ILLEGAL_INSTRUCTION          0x02
#define S32_TRAP_PRIVILEGED_INSTRUCTION       0x03
#define S32_TRAP_FP_DISABLED                  0x04
#define S32_TRAP_WINDOW_OVERFLOW              0x05
#define S32_TRAP_WINDOW_UNDERFLOW             0x06
#define S32_TRAP_MEM_ADDRESS_NOT_ALIGNED      0x07
#define S32_TRAP_FP_EXCEPTION                 0x08
#define S32_TRAP_TAG_OVERFLOW                 0x0a
#define S32_TRAP_CP_DISABLED                  0x24
#define S32_TRAP_DIVISION_BY_ZERO             0x2a


#define S32_XLAT_CPU     0
#define S32_XLAT_REAL    1
#define S32_XLAT_VIRTUAL 2


/* maximum number of register windows */
#define S32_MWINDOWS 32


#define s32_set_bits(var, bits, val) do { \
		if (val) (var) |= (bits); else (var) &= ~(bits); \
	} while (0)

#define s32_get_gpr(c, n) (((n) == 0) ? 0 : (c)->reg[(n)])
#define s32_get_pc(c) ((c)->pc)
#define s32_get_npc(c) ((c)->npc)
#define s32_get_psr(c) ((c)->psr)
#define s32_get_wim(c) ((c)->wim)
#define s32_get_tbr(c) ((c)->tbr)
#define s32_get_y(c) ((c)->y)

#define s32_set_gpr(c, n, v) do { if ((n) != 0) (c)->reg[(n)] = (v); } while (0)
#define s32_set_pc(c, v) do { (c)->pc = (v); } while (0)
#define s32_set_npc(c, v) do { (c)->npc = (v); } while (0)
#define s32_set_psr(c, v) do { (c)->psr = (v); } while (0)
#define s32_set_wim(c, v) do { (c)->wim = (v); } while (0)
#define s32_set_tbr(c, v) do { (c)->tbr = (v); } while (0)
#define s32_set_y(c, v) do { (c)->y = (v); } while (0)

#define s32_get_asi(c) ((c)->asi)
#define s32_set_asi(c, v) do { (c)->asi = (v); } while (0)

#define s32_get_psr_et(c) (((c)->psr & S32_PSR_ET) != 0)
#define s32_get_psr_ps(c) (((c)->psr & S32_PSR_PS) != 0)
#define s32_get_psr_s(c) (((c)->psr & S32_PSR_S) != 0)
#define s32_get_psr_c(c) (((c)->psr & S32_PSR_C) != 0)
#define s32_get_psr_v(c) (((c)->psr & S32_PSR_V) != 0)
#define s32_get_psr_z(c) (((c)->psr & S32_PSR_Z) != 0)
#define s32_get_psr_n(c) (((c)->psr & S32_PSR_N) != 0)

#define s32_set_psr_et(c, v) s32_set_bits ((c)->psr, S32_PSR_ET, (v))
#define s32_set_psr_ps(c, v) s32_set_bits ((c)->psr, S32_PSR_pS, (v))
#define s32_set_psr_s(c, v) s32_set_bits ((c)->psr, S32_PSR_S, (v))
#define s32_set_psr_c(c, v) s32_set_bits ((c)->psr, S32_PSR_C, (v))
#define s32_set_psr_v(c, v) s32_set_bits ((c)->psr, S32_PSR_V, (v))
#define s32_set_psr_z(c, v) s32_set_bits ((c)->psr, S32_PSR_Z, (v))
#define s32_set_psr_n(c, v) s32_set_bits ((c)->psr, S32_PSR_N, (v))

#define s32_get_tbr_tt(c) (((c)->tbr & S32_TBR_TT) >> 4)
#define s32_set_tbr_tt(c, v) do { \
	(c)->tbr &= 0xfffffc00UL; \
	(c)->tbr |= ((v) & 0xff) << 4; \
	} while (0)

#define s32_get_cwp(c) ((c)->psr & S32_PSR_CWP)
#define s32_set_cwp(c, v) do { \
	(c)->psr &= ~S32_PSR_CWP; \
	(c)->psr |= (v) & S32_PSR_CWP; \
	} while (0)


typedef unsigned char (*s32_get_uint8_f) (void *ext, unsigned long addr);
typedef unsigned short (*s32_get_uint16_f) (void *ext, unsigned long addr);
typedef unsigned long (*s32_get_uint32_f) (void *ext, unsigned long addr);

typedef void (*s32_set_uint8_f) (void *ext, unsigned long addr, unsigned char val);
typedef void (*s32_set_uint16_f) (void *ext, unsigned long addr, unsigned short val);
typedef void (*s32_set_uint32_f) (void *ext, unsigned long addr, unsigned long val);

typedef void (*s32_opcode_f) (struct sparc32_s *c);


typedef struct sparc32_s {
	void               *mem_ext;

	s32_get_uint8_f    get_uint8;
	s32_get_uint16_f   get_uint16;
	s32_get_uint32_f   get_uint32;

	s32_set_uint8_f    set_uint8;
	s32_set_uint16_f   set_uint16;
	s32_set_uint32_f   set_uint32;

	void               *log_ext;
	void               (*log_opcode) (void *ext, unsigned long ir);
	void               (*log_undef) (void *ext, unsigned long ir);
	void               (*log_exception) (void *ext, unsigned tn);

	uint32_t           reg[32];
	uint32_t           pc;
	uint32_t           npc;
	uint32_t           psr;
	uint32_t           wim;
	uint32_t           tbr;
	uint32_t           y;

	unsigned           nwindows;
	uint32_t           regstk[16 * S32_MWINDOWS];

	uint8_t            asi;
	uint8_t            asi_text;
	uint8_t            asi_data;

	uint32_t           ir;

	unsigned char      interrupt;

	unsigned long      delay;

	unsigned long long oprcnt;
	unsigned long long clkcnt;

	s32_opcode_f       opcodes[4][64];
} sparc32_t;



/*****************************************************************************
 * MMU
 *****************************************************************************/

int s32_get_mem8 (sparc32_t *c, uint32_t addr, uint8_t asi, unsigned xlat, uint8_t *val);
int s32_get_mem16 (sparc32_t *c, uint32_t addr, uint8_t asi, unsigned xlat, uint16_t *val);
int s32_get_mem32 (sparc32_t *c, uint32_t addr, uint8_t asi, unsigned xlat, uint32_t *val);

int s32_set_mem8 (sparc32_t *c, uint32_t addr, uint8_t asi, unsigned xlat, uint8_t val);
int s32_set_mem16 (sparc32_t *c, uint32_t addr, uint8_t asi, unsigned xlat, uint16_t val);
int s32_set_mem32 (sparc32_t *c, uint32_t addr, uint8_t asi, unsigned xlat, uint32_t val);


/*!***************************************************************************
 * @short Initialize a sparc32 context struct
 *****************************************************************************/
void s32_init (sparc32_t *c);

/*!***************************************************************************
 * @short  Create and initialize a sparc32 context struct
 * @return The sparc32 struct or NULL on error
 *****************************************************************************/
sparc32_t *s32_new (void);

/*!***************************************************************************
 * @short Free the resources used by a sparc32 struct
 *****************************************************************************/
void s32_free (sparc32_t *c);

/*!***************************************************************************
 * @short Delete a sparc32 struct
 *****************************************************************************/
void s32_del (sparc32_t *c);

void s32_set_mem_fct (sparc32_t *c, void *ext,
	void *get8, void *get16, void *get32,
	void *set8, void *set16, void *set32
);

/*!***************************************************************************
 * @short Set the number of register windows
 * @param c The sparc32 context struct
 * @param n The number of register windows. 2 <= n <= 32.
 *
 * s32_reset() must be called after this.
 *****************************************************************************/
void s32_set_nwindows (sparc32_t *c, unsigned n);

int s32_get_reg (sparc32_t *c, const char *reg, unsigned long *val);
int s32_set_reg (sparc32_t *c, const char *reg, unsigned long val);

/*!***************************************************************************
 * @short Get the number of executed instructions
 *****************************************************************************/
unsigned long long s32_get_opcnt (sparc32_t *c);

/*!***************************************************************************
 * @short Get the number of clock cycles
 *****************************************************************************/
unsigned long long s32_get_clkcnt (sparc32_t *c);

/*!***************************************************************************
 * @short Get the previous instruction delay
 *****************************************************************************/
unsigned long s32_get_delay (sparc32_t *c);


/*!***************************************************************************
 * @short Reset a sparc32 cpu core
 *****************************************************************************/
void s32_reset (sparc32_t *c);

/*!***************************************************************************
 * @short Perform a save
 * @param c     The sparc32 context struct
 * @param check Don't modify CWP on overflows if non-zero
 * @return Non-zero if an overflow occurred (checked or not)
 *****************************************************************************/
int s32_save (sparc32_t *c, int check);

/*!***************************************************************************
 * @short Perform a restore
 * @param c     The sparc32 context struct
 * @param check Don't modify CWP on underflows if non-zero
 * @return Non-zero if an underflow occurred (checked or not)
 *****************************************************************************/
int s32_restore (sparc32_t *c, int check);

/*!***************************************************************************
 * @short Log an undefined instruction
 *****************************************************************************/
void s32_undefined (sparc32_t *c);

/*!***************************************************************************
 * @short Execute a trap
 * @param tn The trap number (0 <= tn <= 255)
 *****************************************************************************/
void s32_trap (sparc32_t *c, unsigned tn);

/*!***************************************************************************
 * @short The external interrupt input signal
 *****************************************************************************/
void s32_interrupt (sparc32_t *c, unsigned char val);

/*!***************************************************************************
 * @short Execute one instruction
 *****************************************************************************/
void s32_execute (sparc32_t *c);

/*!***************************************************************************
 * @short Clock a sparc32 cpu core
 *****************************************************************************/
void s32_clock (sparc32_t *c, unsigned long n);


/*****************************************************************************
 * disasm
 *****************************************************************************/

#define S32_DFLAG_PRIV 0x0001
#define S32_DFLAG_TLBM 0x0002
#define S32_DFLAG_CALL 0x0100
#define S32_DFLAG_RETT 0x0101

typedef struct {
	unsigned flags;

	uint32_t pc;
	uint32_t ir;

	unsigned argn;

	char     op[64];
	char     arg1[64];
	char     arg2[64];
	char     arg3[64];
} s32_dasm_t;


typedef void (*s32_dasm_f) (s32_dasm_t *da);


void s32_dasm (s32_dasm_t *dis, uint32_t pc, uint32_t ir);
void s32_dasm_mem (sparc32_t *c, s32_dasm_t *da, uint32_t pc, uint8_t asi, unsigned xlat);


#endif
