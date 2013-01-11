/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/sims32/cmd_s32.c                                    *
 * Created:     2004-09-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"


static mon_cmd_t par_cmd[] = {
	{ "c", "[cnt]", "clock" },
	{ "gb", "[addr...]", "run with breakpoints" },
	{ "g", "", "run" },
	{ "key", "[val...]", "send keycodes to the serial console" },
	{ "p", "[cnt]", "execute cnt instructions, skip calls [1]" },
	{ "rett", "", "execute to next rett" },
	{ "r", "reg [val]", "get or set a register" },
	{ "s", "[what]", "print status (cpu|mem)" },
	{ "t", "[cnt]", "execute cnt instructions [1]" },
	{ "u", "[addr [cnt]]", "disassemble" }
};


void ss32_dasm_str (char *dst, s32_dasm_t *op)
{
	switch (op->argn) {
	case 0:
		sprintf (dst, "%08lX  %s", (unsigned long) op->ir, op->op);
		break;

	case 1:
		sprintf (dst, "%08lX  %-8s %s",
			(unsigned long) op->ir, op->op, op->arg1
		);
		break;

	case 2:
		sprintf (dst, "%08lX  %-8s %s, %s",
			(unsigned long) op->ir, op->op, op->arg1, op->arg2
		);
		break;

	case 3:
		sprintf (dst, "%08lX  %-8s %s, %s, %s",
			(unsigned long) op->ir, op->op,
			op->arg1, op->arg2, op->arg3
		);
		break;

	default:
		strcpy (dst, "---");
		break;
	}
}

void ss32_prt_state_cpu (sparc32_t *c, FILE *fp)
{
	unsigned           i;
	unsigned long long opcnt, clkcnt;
	unsigned long      delay;
	s32_dasm_t         op;
	char               str[256];

	pce_prt_sep ("SPARC32");

	opcnt = s32_get_opcnt (c);
	clkcnt = s32_get_clkcnt (c);
	delay = s32_get_delay (c);

	fprintf (fp, "CLK=%llx  OP=%llx  DLY=%lx  CPI=%.4f\n",
		clkcnt, opcnt, delay,
		(opcnt > 0) ? ((double) (clkcnt + delay) / (double) opcnt) : 1.0
	);

	fprintf (fp, " PSR=%08lX  ICC=[%c%c%c%c]\n",
		(unsigned long) s32_get_psr (c),
		(s32_get_psr_c (c)) ? 'C' : '-',
		(s32_get_psr_v (c)) ? 'V' : '-',
		(s32_get_psr_z (c)) ? 'Z' : '-',
		(s32_get_psr_n (c)) ? 'N' : '-'
	);

	fprintf (fp, " r00=%08lX  r08=%08lX  r16=%08lX  r24=%08lX    PC=%08lX\n",
		(unsigned long) s32_get_gpr (c, 0),
		(unsigned long) s32_get_gpr (c, 8),
		(unsigned long) s32_get_gpr (c, 16),
		(unsigned long) s32_get_gpr (c, 24),
		(unsigned long) s32_get_pc (c)
	);

	fprintf (fp, " r01=%08lX  r09=%08lX  r17=%08lX  r25=%08lX   nPC=%08lX\n",
		(unsigned long) s32_get_gpr (c, 1),
		(unsigned long) s32_get_gpr (c, 9),
		(unsigned long) s32_get_gpr (c, 17),
		(unsigned long) s32_get_gpr (c, 25),
		(unsigned long) s32_get_npc (c)
	);

	fprintf (fp, " r02=%08lX  r10=%08lX  r18=%08lX  r26=%08lX   WIM=%08lX\n",
		(unsigned long) s32_get_gpr (c, 2),
		(unsigned long) s32_get_gpr (c, 10),
		(unsigned long) s32_get_gpr (c, 18),
		(unsigned long) s32_get_gpr (c, 26),
		(unsigned long) s32_get_wim (c)
	);

	fprintf (fp, " r03=%08lX  r11=%08lX  r19=%08lX  r27=%08lX   TBR=%08lX\n",
		(unsigned long) s32_get_gpr (c, 3),
		(unsigned long) s32_get_gpr (c, 11),
		(unsigned long) s32_get_gpr (c, 19),
		(unsigned long) s32_get_gpr (c, 27),
		(unsigned long) s32_get_tbr (c)
	);

	fprintf (fp, " r04=%08lX  r12=%08lX  r20=%08lX  r28=%08lX     Y=%08lX\n",
		(unsigned long) s32_get_gpr (c, 4),
		(unsigned long) s32_get_gpr (c, 12),
		(unsigned long) s32_get_gpr (c, 20),
		(unsigned long) s32_get_gpr (c, 28),
		(unsigned long) s32_get_y (c)
	);

	for (i = 5; i < 8; i++) {
		fprintf (fp, " r%02u=%08lX  r%02u=%08lX  r%02u=%08lX  r%02u=%08lX\n",
			i + 0, (unsigned long) s32_get_gpr (c, i + 0),
			i + 8, (unsigned long) s32_get_gpr (c, i + 8),
			i + 16, (unsigned long) s32_get_gpr (c, i + 16),
			i + 24, (unsigned long) s32_get_gpr (c, i + 24)
		);
	}

	s32_dasm_mem (c, &op, s32_get_pc (c), c->asi_text, par_xlat);
	ss32_dasm_str (str, &op);

	fprintf (fp, "%08lX  %s\n", (unsigned long) s32_get_pc (c), str);
}

void ss32_prt_state_mem (sims32_t *sim, FILE *fp)
{
	pce_prt_sep ("SPARC MEM");
	mem_prt_state (sim->mem, fp);
}


static
int ss32_check_break (sims32_t *sim)
{
	if (bps_check (&sim->bps, 0, s32_get_pc (sim->cpu), stdout)) {
		return (1);
	}

	if (sim->brk) {
		return (1);
	}

	return (0);
}

static
void ss32_exec (sims32_t *sim)
{
	unsigned long long old;

	old = s32_get_opcnt (sim->cpu);

	while (s32_get_opcnt (sim->cpu) == old) {
		ss32_clock (sim, 1);
	}
}

static
int ss32_exec_to (sims32_t *sim, unsigned long addr)
{
	while (s32_get_pc (sim->cpu) != addr) {
		ss32_clock (sim, 1);

		if (sim->brk) {
			return (1);
		}
	}

	return (0);
}

static
int ss32_exec_off (sims32_t *sim, unsigned long addr)
{
	while (s32_get_pc (sim->cpu) == addr) {
		ss32_clock (sim, 1);

		if (sim->brk) {
			return (1);
		}
	}

	return (0);
}

void ss32_run (sims32_t *sim)
{
	pce_start (&sim->brk);

	while (1) {
		ss32_clock (sim, 1);

		if (sim->brk) {
			break;
		}
	}

	pce_stop();
}


#if 0
static
void ss32_log_opcode (void *ext, unsigned long ir)
{
	sims32_t *sim = ext;
}
#endif

static
void ss32_log_undef (void *ext, unsigned long ir)
{
	sims32_t *sim = ext;

	pce_log (MSG_DEB,
		"%08lX: undefined operation [%08lX]\n",
		(unsigned long) s32_get_pc (sim->cpu), ir
	);

	ss32_break (sim, PCE_BRK_STOP);
}

static
void ss32_log_trap (void *ext, unsigned tn)
{
	sims32_t *sim = ext;
	char     *name;

	switch (tn) {
		case 0x0300:
			name = "data store";
			return;

		default:
			name = "unknown";
			break;
	}

	pce_log (MSG_DEB, "%08lX: trap %x (%s)\n",
		(unsigned long) s32_get_pc (sim->cpu), tn, name
	);
}

static
void do_c (cmd_t *cmd, sims32_t *sim)
{
	unsigned long cnt;

	cnt = 1;

	cmd_match_uint32 (cmd, &cnt);

	if (!cmd_match_end (cmd)) {
		return;
	}

	while (cnt > 0) {
		ss32_clock (sim, 1);
		cnt -= 1;
	}

	ss32_prt_state_cpu (sim->cpu, stdout);
}

static
void do_g_b (cmd_t *cmd, sims32_t *sim)
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

	while (1) {
		ss32_exec (sim);

		if (ss32_check_break (sim)) {
			break;
		}
	}

	pce_stop();
}

static
void do_g (cmd_t *cmd, sims32_t *sim)
{
	if (cmd_match (cmd, "b")) {
		do_g_b (cmd, sim);
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	ss32_run (sim);
}

static
void do_key (cmd_t *cmd, sims32_t *sim)
{
	unsigned short c;

	while (cmd_match_uint16 (cmd, &c)) {
		ss32_set_keycode (sim, c);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}
}

static
void do_p (cmd_t *cmd, sims32_t *sim)
{
	unsigned long cnt;
	s32_dasm_t    da;

	cnt = 1;

	cmd_match_uint32 (cmd, &cnt);

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	while (cnt > 0) {
		s32_dasm_mem (sim->cpu, &da, s32_get_pc (sim->cpu), sim->cpu->asi_text, S32_XLAT_CPU);

		if (da.flags & S32_DFLAG_CALL) {
			if (ss32_exec_to (sim, s32_get_pc (sim->cpu) + 8)) {
				break;
			}
		}
		else {
			uint32_t psr;

			psr = s32_get_psr (sim->cpu);

			if (ss32_exec_off (sim, s32_get_pc (sim->cpu))) {
				break;
			}

			if ((psr & S32_PSR_S) == 0) {
				/* check if exception occured */
				while (s32_get_psr_s (sim->cpu)) {
					ss32_clock (sim, 1);

					if (sim->brk) {
						break;
					}
				}
			}
		}

		cnt -= 1;
	}

	pce_stop();

	ss32_prt_state_cpu (sim->cpu, stdout);
}

static
void do_rett (cmd_t *cmd, sims32_t *sim)
{
	s32_dasm_t da;

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	while (1) {
		s32_dasm_mem (sim->cpu, &da, s32_get_pc (sim->cpu), sim->cpu->asi_text, S32_XLAT_CPU);

		if (ss32_exec_off (sim, s32_get_pc (sim->cpu))) {
			break;
		}

		if (da.flags & S32_DFLAG_RETT) {
			break;
		}
	}

	pce_stop();

	ss32_prt_state_cpu (sim->cpu, stdout);
}

static
void do_r (cmd_t *cmd, sims32_t *sim)
{
	unsigned long val;
	char          sym[256];

	if (cmd_match_eol (cmd)) {
		ss32_prt_state_cpu (sim->cpu, stdout);
		return;
	}

	if (!cmd_match_ident (cmd, sym, 256)) {
		printf ("missing register\n");
		return;
	}

	if (s32_get_reg (sim->cpu, sym, &val)) {
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

	s32_set_reg (sim->cpu, sym, val);

	ss32_prt_state_cpu (sim->cpu, stdout);
}

static
void do_s (cmd_t *cmd, sims32_t *sim)
{
	if (cmd_match_eol (cmd)) {
		ss32_prt_state_cpu (sim->cpu, stdout);
		return;
	}

	prt_state (sim, stdout, cmd_get_str (cmd));
}

static
void do_t (cmd_t *cmd, sims32_t *sim)
{
	unsigned long i, n;

	n = 1;

	cmd_match_uint32 (cmd, &n);

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	for (i = 0; i < n; i++) {
		ss32_exec (sim);
	}

	pce_stop();

	ss32_prt_state_cpu (sim->cpu, stdout);
}

static
void do_u (cmd_t *cmd, sims32_t *sim)
{
	unsigned             i;
	int                  to;
	unsigned long        addr, cnt;
	static unsigned int  first = 1;
	static unsigned long saddr = 0;
	s32_dasm_t           op;
	char                 str[256];

	if (first) {
		first = 0;
		saddr = s32_get_pc (sim->cpu);
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
		s32_dasm_mem (sim->cpu, &op, addr, sim->cpu->asi_text, par_xlat);
		ss32_dasm_str (str, &op);

		fprintf (stdout, "%08lX  %s\n", addr, str);

		addr += 4;
	}

	saddr = addr;
}

int ss32_do_cmd (sims32_t *sim, cmd_t *cmd)
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
	else if (cmd_match (cmd, "rett")) {
		do_rett (cmd, sim);
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
	else {
		return (1);
	}

	return (0);
}

void ss32_cmd_init (sims32_t *sim, monitor_t *mon)
{
	mon_cmd_add (mon, par_cmd, sizeof (par_cmd) / sizeof (par_cmd[0]));
	mon_cmd_add_bp (mon);

	sim->cpu->log_ext = sim;
	sim->cpu->log_opcode = NULL;
	sim->cpu->log_undef = &ss32_log_undef;
	sim->cpu->log_exception = &ss32_log_trap;
}
