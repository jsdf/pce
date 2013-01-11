/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/simarm/cmd_arm.c                                    *
 * Created:     2004-11-04 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"
#include "cmd_arm.h"
#include "simarm.h"

#include <cpu/arm/arm.h>

#include <lib/console.h>
#include <lib/log.h>
#include <lib/monitor.h>
#include <lib/sysdep.h>


static mon_cmd_t par_cmd[] = {
	{ "c", "[cnt]", "clock" },
	{ "gb", "[addr...]", "run with breakpoints" },
	{ "g", "", "run" },
	{ "key", "[val...]", "send keycodes to the serial console" },
	{ "p", "[cnt]", "execute cnt instructions, skip calls [1]" },
	{ "r", "reg [val]", "get or set a register" },
	{ "s", "[what]", "print status (cpu|intc|mem|mmu|timer)" },
	{ "t", "[cnt]", "execute cnt instructions [1]" },
	{ "u", "[addr [cnt]]", "disassemble" },
	{ "x", "[c|r|v]", "set the translation mode (cpu, real, virtual)" },
	{ "xx", "[addr...]", "translate a virtual address" }
};


static const char *arm_modes[32] = {
	"0x00", "0x01", "0x02", "0x03",
	"0x04", "0x05", "0x06", "0x07",
	"0x08", "0x09", "0x0a", "0x0b",
	"0x0c", "0x0d", "0x0e", "0x0f",
	"usr",  "fiq",  "irq",  "svc",
	"0x14", "0x15", "0x16", "abt",
	"0x18", "0x19", "und",  "0x1b",
	"0x1c", "0x1d", "0x1e", "sys"
};

void sarm_dasm_str (char *dst, arm_dasm_t *op)
{
	unsigned i, j;

	if (op->argn == 0) {
		sprintf (dst, "%08lX  %s", (unsigned long) op->ir, op->op);
	}
	else {
		j = sprintf (dst, "%08lX  %-8s %s", (unsigned long) op->ir, op->op, op->arg[0]);

		for (i = 1; i < op->argn; i++) {
			j += sprintf (dst + j, ", %s", op->arg[i]);
		}
	}
}

void sarm_prt_state_cpu (arm_t *c, FILE *fp)
{
	unsigned long long opcnt, clkcnt;
	unsigned long      delay;
	arm_dasm_t         op;
	char               str[256];

	pce_prt_sep ("ARM");

	opcnt = arm_get_opcnt (c);
	clkcnt = arm_get_clkcnt (c);
	delay = arm_get_delay (c);

	fprintf (fp, "CLK=%llx  OP=%llx  DLY=%lx  CPI=%.4f\n",
		clkcnt, opcnt, delay,
		(opcnt > 0) ? ((double) (clkcnt + delay) / (double) opcnt) : 1.0
	);

	fprintf (fp, "r00=%08lX  r04=%08lX  r08=%08lX  r12=%08lX  CPSR=%08lX\n",
		(unsigned long) arm_get_gpr (c, 0),
		(unsigned long) arm_get_gpr (c, 4),
		(unsigned long) arm_get_gpr (c, 8),
		(unsigned long) arm_get_gpr (c, 12),
		(unsigned long) arm_get_cpsr (c)
	);

	fprintf (fp, "r01=%08lX  r05=%08lX  r09=%08lX  r13=%08lX  SPSR=%08lX\n",
		(unsigned long) arm_get_gpr (c, 1),
		(unsigned long) arm_get_gpr (c, 5),
		(unsigned long) arm_get_gpr (c, 9),
		(unsigned long) arm_get_gpr (c, 13),
		(unsigned long) arm_get_spsr (c)
	);

	fprintf (fp, "r02=%08lX  r06=%08lX  r10=%08lX  r14=%08lX    CC=[%c%c%c%c]\n",
		(unsigned long) arm_get_gpr (c, 2),
		(unsigned long) arm_get_gpr (c, 6),
		(unsigned long) arm_get_gpr (c, 10),
		(unsigned long) arm_get_gpr (c, 14),
		(arm_get_cc_n (c)) ? 'N' : '-',
		(arm_get_cc_z (c)) ? 'Z' : '-',
		(arm_get_cc_c (c)) ? 'C' : '-',
		(arm_get_cc_v (c)) ? 'V' : '-'
	);

	fprintf (fp, "r03=%08lX  r07=%08lX  r11=%08lX  r15=%08lX     M=%02X (%s)\n",
		(unsigned long) arm_get_gpr (c, 3),
		(unsigned long) arm_get_gpr (c, 7),
		(unsigned long) arm_get_gpr (c, 11),
		(unsigned long) arm_get_gpr (c, 15),
		(unsigned) arm_get_cpsr_m (c),
		arm_modes[arm_get_cpsr_m (c) & 0x1f]
	);

	arm_dasm_mem (c, &op, arm_get_pc (c), par_xlat);
	sarm_dasm_str (str, &op);

	fprintf (fp, "%08lX  %s\n", (unsigned long) arm_get_pc (c), str);
}

void sarm_prt_state_mmu (arm_t *c, FILE *fp)
{
	arm_copr15_t *p;

	pce_prt_sep ("ARM MMU");

	p = c->copr[15]->ext;

	fprintf (fp, "CR=[%c%c%c%c%c%c%c%c%c%c%c%c%c]\n",
		(p->reg[1] & ARM_C15_CR_V) ? 'V' : 'v',
		(p->reg[1] & ARM_C15_CR_I) ? 'I' : 'i',
		(p->reg[1] & ARM_C15_CR_Z) ? 'Z' : 'z',
		(p->reg[1] & ARM_C15_CR_R) ? 'R' : 'r',
		(p->reg[1] & ARM_C15_CR_S) ? 'S' : 's',
		(p->reg[1] & ARM_C15_CR_B) ? 'B' : 'b',
		(p->reg[1] & ARM_C15_CR_L) ? 'L' : 'l',
		(p->reg[1] & ARM_C15_CR_D) ? 'D' : 'd',
		(p->reg[1] & ARM_C15_CR_P) ? 'P' : 'p',
		(p->reg[1] & ARM_C15_CR_W) ? 'W' : 'w',
		(p->reg[1] & ARM_C15_CR_C) ? 'C' : 'c',
		(p->reg[1] & ARM_C15_CR_A) ? 'A' : 'a',
		(p->reg[1] & ARM_C15_CR_M) ? 'M' : 'm'
	);

	fprintf (fp, "c00=%08lX  c04=%08lX  c08=%08lX  c12=%08lX\n",
		(unsigned long) p->reg[0],
		(unsigned long) p->reg[4],
		(unsigned long) p->reg[8],
		(unsigned long) p->reg[12]
	);

	fprintf (fp, "c01=%08lX  c05=%08lX  c09=%08lX  c13=%08lX\n",
		(unsigned long) p->reg[1],
		(unsigned long) p->reg[5],
		(unsigned long) p->reg[9],
		(unsigned long) p->reg[13]
	);

	fprintf (fp, "c02=%08lX  c06=%08lX  c10=%08lX  c14=%08lX\n",
		(unsigned long) p->reg[2],
		(unsigned long) p->reg[6],
		(unsigned long) p->reg[10],
		(unsigned long) p->reg[14]
	);

	fprintf (fp, "c03=%08lX  c07=%08lX  c11=%08lX  c15=%08lX\n",
		(unsigned long) p->reg[3],
		(unsigned long) p->reg[7],
		(unsigned long) p->reg[11],
		(unsigned long) p->reg[15]
	);
}

void sarm_prt_state_timer (ixp_timer_t *tmr, FILE *fp)
{
	unsigned            i;
	ixp_timer_counter_t *cnt;

	pce_prt_sep ("IXP TIMER");

	fprintf (fp, "ADDR=%08lX\n", tmr->base);

	for (i = 0; i < 4; i++) {
		cnt = &tmr->cntr[i];

		fprintf (fp, "T%u: %c VAL=%08lX LOAD=%08lX CTL=%08lX IRQ=%d\n",
			i,
			tmr_get_active (tmr, i) ? 'R' : 'S',
			(unsigned long) cnt->status,
			(unsigned long) cnt->load,
			(unsigned long) cnt->ctrl,
			cnt->irq_val != 0
		);
	}
}

void sarm_prt_state_intc (simarm_t *sim, FILE *fp)
{
	ixp_intc_t *ic;

	pce_prt_sep ("IXP INTC");

	ic = sim->intc;

	fprintf (fp, "inp raw: %08lx\n", ic->status_raw);
	fprintf (fp, "ena irq: %08lx\n", ic->enable_irq);
	fprintf (fp, "out irq: %08lx (%d)\n", ic->status_irq, (int) ic->irq_val);
	fprintf (fp, "\n");
	fprintf (fp, "inp raw: %08lx\n", ic->status_raw);
	fprintf (fp, "ena fiq: %08lx\n", ic->enable_fiq);
	fprintf (fp, "out fiq: %08lx (%d)\n", ic->status_fiq, (int) ic->fiq_val);
}

void sarm_prt_state_mem (simarm_t *sim, FILE *fp)
{
	pce_prt_sep ("ARM MEM");
	mem_prt_state (sim->mem, fp);
}

void prt_state (simarm_t *sim, FILE *fp, const char *str)
{
	cmd_t cmd;

	cmd_set_str (&cmd, str);

	if (cmd_match_eol (&cmd)) {
		return;
	}

	while (!cmd_match_eol (&cmd)) {
		if (cmd_match (&cmd, "cpu")) {
			sarm_prt_state_cpu (sim->cpu, fp);
		}
		else if (cmd_match (&cmd, "mmu")) {
			sarm_prt_state_mmu (sim->cpu, fp);
		}
		else if (cmd_match (&cmd, "timer")) {
			sarm_prt_state_timer (sim->timer, fp);
		}
		else if (cmd_match (&cmd, "intc")) {
			sarm_prt_state_intc (sim, fp);
		}
		else if (cmd_match (&cmd, "mem")) {
			sarm_prt_state_mem (sim, fp);
		}
		else {
			printf ("unknown component (%s)\n", cmd_get_str (&cmd));
			return;
		}
	}
}


static
void sarm_exec (simarm_t *sim)
{
	unsigned long long old;

	old = arm_get_opcnt (sim->cpu);

	while (arm_get_opcnt (sim->cpu) == old) {
		sarm_clock (sim, 1);
	}
}

static
int sarm_check_break (simarm_t *sim)
{
	if (bps_check (&sim->bps, 0, arm_get_pc (sim->cpu), stdout)) {
		return (1);
	}

	if (sim->brk) {
		return (1);
	}

	return (0);
}

static
int sarm_exec_to (simarm_t *sim, unsigned long addr)
{
	while (arm_get_pc (sim->cpu) != addr) {
		sarm_clock (sim, 1);

		if (sim->brk) {
			return (1);
		}
	}

	return (0);
}

static
int sarm_exec_off (simarm_t *sim, unsigned long addr)
{
	while (arm_get_pc (sim->cpu) == addr) {
		sarm_clock (sim, 1);

		if (sim->brk) {
			return (1);
		}
	}

	return (0);
}

void sarm_run (simarm_t *sim)
{
	pce_start (&sim->brk);

	sarm_clock_discontinuity (sim);

	while (1) {
		sarm_clock (sim, 16);

		if (sim->brk) {
			break;
		}
	}

	pce_stop();
}


#if 0
static
int sarm_log_opcode (void *ext, unsigned long ir)
{
	simarm_t *sim = ext;

	return (0);
}
#endif

static
void sarm_log_undef (void *ext, unsigned long ir)
{
	simarm_t *sim = ext;

	if ((ir & 0x0f000000UL) == 0x0c000000UL) {
		return;
	}

	if ((ir & 0x0f000000UL) == 0x0d000000UL) {
		return;
	}

	pce_log (MSG_DEB,
		"%08lX: undefined operation [%08lX]\n",
		(unsigned long) arm_get_pc (sim->cpu), ir
	);
}

static
void sarm_log_trap (void *ext, unsigned long addr)
{
	simarm_t *sim = ext;
	char     *name;

	switch (addr & 0xff) {
		case 0x04:
			name = "undefined operation";
			name = NULL;
			break;

		case 0x08:
			name = "swi";
			name = NULL;
			break;

		case 0x0c:
			name = "prefetch abort";
			name = NULL;
			break;

		case 0x10:
			name = "data abort";
			name = NULL;
			break;

		case 0x18:
			name = "irq";
			name = NULL;
			break;

		case 0x1c:
			name = "fiq";
			break;

		default:
			name = "unknown";
			break;
	}

	if (name != NULL) {
		pce_log (MSG_DEB, "%08lX (%08lX): exception %lx (%s)\n",
			(unsigned long) arm_get_pc (sim->cpu),
			(unsigned long) arm_get_last_pc (sim->cpu),
			addr, name
		);
	}
}

static
void do_c (cmd_t *cmd, simarm_t *sim)
{
	unsigned long cnt;

	cnt = 1;

	cmd_match_uint32 (cmd, &cnt);

	if (!cmd_match_end (cmd)) {
		return;
	}

	sarm_clock_discontinuity (sim);

	while (cnt > 0) {
		sarm_clock (sim, 1);
		cnt -= 1;
	}

	sarm_prt_state_cpu (sim->cpu, stdout);
}

static
void do_g_b (cmd_t *cmd, simarm_t *sim)
{
	unsigned long addr;
	breakpoint_t  *bp;

	while (cmd_match_uint32 (cmd, &addr)) {
		bp = bp_addr_new (addr);
		bps_bp_add (&sim->bps, bp);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	sarm_clock_discontinuity (sim);

	while (1) {
		sarm_exec (sim);

		if (sarm_check_break (sim)) {
			break;
		}
	}

	pce_stop();
}

static
void do_g (cmd_t *cmd, simarm_t *sim)
{
	if (cmd_match (cmd, "b")) {
		do_g_b (cmd, sim);
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	sarm_run (sim);
}

static
void do_key (cmd_t *cmd, simarm_t *sim)
{
	unsigned short c;

	while (cmd_match_uint16 (cmd, &c)) {
		sarm_set_keycode (sim, c);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}
}

static
void do_p (cmd_t *cmd, simarm_t *sim)
{
	unsigned long cnt;
	arm_dasm_t    da;

	cnt = 1;

	cmd_match_uint32 (cmd, &cnt);

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	sarm_clock_discontinuity (sim);

	while (cnt > 0) {
		arm_dasm_mem (sim->cpu, &da, arm_get_pc (sim->cpu), ARM_XLAT_CPU);

		if (da.flags & ARM_DFLAG_CALL) {
			if (sarm_exec_to (sim, arm_get_pc (sim->cpu) + 4)) {
				break;
			}
		}
		else {
			uint32_t cpsr;

			cpsr = arm_get_cpsr (sim->cpu);

			if (sarm_exec_off (sim, arm_get_pc (sim->cpu))) {
				break;
			}

			if ((cpsr & ARM_PSR_M) == ARM_MODE_USR) {
				/* check if exception occured */
				while (arm_get_cpsr_m (sim->cpu) != ARM_MODE_USR) {
					sarm_clock (sim, 1);

					if (sim->brk) {
						break;
					}
				}
			}
		}

		cnt -= 1;
	}

	pce_stop();

	sarm_prt_state_cpu (sim->cpu, stdout);
}

static
void do_r (cmd_t *cmd, simarm_t *sim)
{
	unsigned long val;
	char          sym[256];

	if (cmd_match_eol (cmd)) {
		sarm_prt_state_cpu (sim->cpu, stdout);
		return;
	}

	if (!cmd_match_ident (cmd, sym, 256)) {
		printf ("missing register\n");
		return;
	}

	if (arm_get_reg (sim->cpu, sym, &val)) {
		printf ("bad register (%s)\n", sym);
		return;
	}

	if (cmd_match_eol (cmd)) {
		printf ("%08lX\n", val);
		return;
	}

	if (!cmd_match_uint32 (cmd, &val)) {
		printf ("missing value\n");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	arm_set_reg (sim->cpu, sym, val);

	sarm_prt_state_cpu (sim->cpu, stdout);
}

static
void do_s (cmd_t *cmd, simarm_t *sim)
{
	if (cmd_match_eol (cmd)) {
		sarm_prt_state_cpu (sim->cpu, stdout);
		return;
	}

	prt_state (sim, stdout, cmd_get_str (cmd));
}

static
void do_t (cmd_t *cmd, simarm_t *sim)
{
	unsigned long i, n;

	n = 1;

	cmd_match_uint32 (cmd, &n);

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	sarm_clock_discontinuity (sim);

	for (i = 0; i < n; i++) {
		sarm_exec (sim);
	}

	pce_stop();

	sarm_prt_state_cpu (sim->cpu, stdout);
}

static
void do_u (cmd_t *cmd, simarm_t *sim)
{
	unsigned             i;
	int                  to;
	unsigned long        addr, cnt;
	static unsigned int  first = 1;
	static unsigned long saddr = 0;
	arm_dasm_t           op;
	char                 str[256];

	if (first) {
		first = 0;
		saddr = arm_get_pc (sim->cpu);
	}

	to = 0;
	addr = saddr;
	cnt = 16;

	if (cmd_match (cmd, "-")) {
		to = 1;
	}

	if (cmd_match_uint32 (cmd, &addr)) {
		cmd_match_uint32 (cmd, &cnt);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (to) {
		addr -= 4 * (cnt - 1);
	}

	for (i = 0; i < cnt; i++) {
		arm_dasm_mem (sim->cpu, &op, addr, par_xlat);
		sarm_dasm_str (str, &op);

		fprintf (stdout, "%08lX  %s\n", addr, str);

		addr += 4;
	}

	saddr = addr;
}

static
void do_xx (cmd_t *cmd, simarm_t *sim)
{
	unsigned long addr1;
	uint32_t      addr2;
	unsigned      domn, perm;

	while (cmd_match_uint32 (cmd, &addr1)) {
		addr2 = addr1;
		if (arm_translate_extern (sim->cpu, &addr2, par_xlat, &domn, &perm)) {
			printf ("%08lX translation abort\n", addr1);
		}
		else {
			printf ("%08lX -> %08lX D=%02X P=%02X\n",
				addr1, (unsigned long) addr2, domn, perm
			);
		}
	}
}

static
void do_x (cmd_t *cmd, simarm_t *sim)
{
	if (cmd_match_eol (cmd)) {
		switch (par_xlat) {
		case ARM_XLAT_CPU:
			printf ("xlat cpu\n");
			break;

		case ARM_XLAT_REAL:
			printf ("xlat real\n");
			break;

		case ARM_XLAT_VIRTUAL:
			printf ("xlat virtual\n");
			break;

		default:
			printf ("xlat unknown\n");
			break;
		}

		return;
	}

	if (cmd_match (cmd, "c")) {
		par_xlat = ARM_XLAT_CPU;
	}
	else if (cmd_match (cmd, "r")) {
		par_xlat = ARM_XLAT_REAL;
	}
	else if (cmd_match (cmd, "v")) {
		par_xlat = ARM_XLAT_VIRTUAL;
	}
	else if (cmd_match (cmd, "x")) {
		do_xx (cmd, sim);
	}
	else {
		cmd_error (cmd, "unknown translation type");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}
}

int sarm_do_cmd (simarm_t *sim, cmd_t *cmd)
{
	if (cmd_match (cmd, "b")) {
		cmd_do_b (cmd, &sim->bps);
	}
	else if (cmd_match (cmd, "c")) {
		do_c (cmd, sim);
	}
	else if (cmd_match (cmd, "g")) {
		do_g (cmd, sim);
	}
	else if (cmd_match (cmd, "key")) {
		do_key (cmd, sim);
	}
	else if (cmd_match (cmd, "p")) {
		do_p (cmd, sim);
	}
	else if (cmd_match (cmd, "r")) {
		do_r (cmd, sim);
	}
	else if (cmd_match (cmd, "s")) {
		do_s (cmd, sim);
	}
	else if (cmd_match (cmd, "t")) {
		do_t (cmd, sim);
	}
	else if (cmd_match (cmd, "u")) {
		do_u (cmd, sim);
	}
	else if (cmd_match (cmd, "x")) {
		do_x (cmd, sim);
	}
	else {
		return (1);
	}

	return (0);
}

void sarm_cmd_init (simarm_t *sim, monitor_t *mon)
{
	mon_cmd_add (mon, par_cmd, sizeof (par_cmd) / sizeof (par_cmd[0]));
	mon_cmd_add_bp (mon);

	sim->cpu->log_ext = sim;
	sim->cpu->log_opcode = NULL;
	sim->cpu->log_undef = sarm_log_undef;
	sim->cpu->log_exception = sarm_log_trap;
}
