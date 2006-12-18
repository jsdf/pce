/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/sim405/cmd_ppc.c                                  *
 * Created:       2004-06-01 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004-2006 Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004-2006 Lukas Ruf <ruf@lpr.ch>                       *
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

/* $Id$ */


#include "main.h"


int ppc_match_reg (cmd_t *cmd, sim405_t *sim, uint32_t **reg)
{
	p405_t *ppc;

	ppc = sim->ppc;

	cmd_match_space (cmd);

	if (cmd_match (cmd, "r")) {
		unsigned n;

		if (!cmd_match_uint (cmd, &n, 10)) {
			cmd_error (cmd, "missing register number");
			return (0);
		}

		*reg = &ppc->gpr[n & 0x3f];

		return (1);
	}
	else if (cmd_match (cmd, "cr")) {
		*reg = &ppc->cr;
		return (1);
	}
	else if (cmd_match (cmd, "ctr")) {
		*reg = &ppc->ctr;
		return (1);
	}
	else if (cmd_match (cmd, "dear")) {
		*reg = &ppc->dear;
		return (1);
	}
	else if (cmd_match (cmd, "esr")) {
		*reg = &ppc->esr;
		return (1);
	}
	else if (cmd_match (cmd, "evpr")) {
		*reg = &ppc->evpr;
		return (1);
	}
	else if (cmd_match (cmd, "lr")) {
		*reg = &ppc->lr;
		return (1);
	}
	else if (cmd_match (cmd, "msr")) {
		*reg = &ppc->msr;
		return (1);
	}
	else if (cmd_match (cmd, "pc")) {
		*reg = &ppc->pc;
		return (1);
	}
	else if (cmd_match (cmd, "pid")) {
		*reg = &ppc->pid;
		return (1);
	}
	else if (cmd_match (cmd, "pit1")) {
		*reg = &ppc->pit[1];
		return (1);
	}
	else if (cmd_match (cmd, "pit")) {
		*reg = &ppc->pit[0];
		return (1);
	}
	else if (cmd_match (cmd, "sprg0")) {
		*reg = &ppc->sprg[0];
		return (1);
	}
	else if (cmd_match (cmd, "sprg1")) {
		*reg = &ppc->sprg[1];
		return (1);
	}
	else if (cmd_match (cmd, "sprg2")) {
		*reg = &ppc->sprg[2];
		return (1);
	}
	else if (cmd_match (cmd, "sprg3")) {
		*reg = &ppc->sprg[3];
		return (1);
	}
	else if (cmd_match (cmd, "sprg4")) {
		*reg = &ppc->sprg[4];
		return (1);
	}
	else if (cmd_match (cmd, "sprg5")) {
		*reg = &ppc->sprg[5];
		return (1);
	}
	else if (cmd_match (cmd, "sprg6")) {
		*reg = &ppc->sprg[6];
		return (1);
	}
	else if (cmd_match (cmd, "sprg7")) {
		*reg = &ppc->sprg[7];
		return (1);
	}
	else if (cmd_match (cmd, "srr0")) {
		*reg = &ppc->srr[0];
		return (1);
	}
	else if (cmd_match (cmd, "srr1")) {
		*reg = &ppc->srr[1];
		return (1);
	}
	else if (cmd_match (cmd, "srr2")) {
		*reg = &ppc->srr[2];
		return (1);
	}
	else if (cmd_match (cmd, "srr3")) {
		*reg = &ppc->srr[3];
		return (1);
	}
	else if (cmd_match (cmd, "tbl")) {
		*reg = &ppc->tbl;
		return (1);
	}
	else if (cmd_match (cmd, "tbu")) {
		*reg = &ppc->tbu;
		return (1);
	}
	else if (cmd_match (cmd, "tcr")) {
		*reg = &ppc->tcr;
		return (1);
	}
	else if (cmd_match (cmd, "tsr")) {
		*reg = &ppc->tsr;
		return (1);
	}
	else if (cmd_match (cmd, "xer")) {
		*reg = &ppc->xer;
		return (1);
	}
	else if (cmd_match (cmd, "zpr")) {
		*reg = &ppc->zpr;
		return (1);
	}

	return (0);
}

void ppc_disasm_str (char *dst, p405_disasm_t *op)
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

	case 4:
		sprintf (dst, "%08lX  %-8s %s, %s, %s, %s",
			(unsigned long) op->ir, op->op,
			op->arg1, op->arg2, op->arg3, op->arg4
		);
		break;

	case 5:
		sprintf (dst, "%08lX  %-8s %s, %s, %s, %s, %s",
			(unsigned long) op->ir, op->op,
			op->arg1, op->arg2, op->arg3, op->arg4, op->arg5
		);
		break;

	default:
		strcpy (dst, "---");
		break;
	}
}

static
void prt_tlb_entry (FILE *fp, p405_tlbe_t *ent, unsigned idx)
{
	printf ("%02x: %08lx -> %08lx %06lx V%c R%c W%c X%c  TID=%02x  %08lx",
		idx,
		(unsigned long) p405_get_tlbe_epn (ent),
		(unsigned long) p405_get_tlbe_rpn (ent),
		(unsigned long) p405_get_tlbe_sizeb (ent),
		p405_get_tlbe_v (ent) ? '+' : '-',
		p405_get_tlbe_v (ent) ? '+' : '-',
		p405_get_tlbe_wr (ent) ? '+' : '-',
		p405_get_tlbe_ex (ent) ? '+' : '-',
		(unsigned) p405_get_tlbe_tid (ent),
		(unsigned long) ent->mask
	);
}

void s405_prt_state_ppc (sim405_t *sim, FILE *fp)
{
	unsigned           i;
	unsigned long long opcnt, clkcnt;
	unsigned long      delay;
	double             t, efrq;
	p405_t             *c;
	p405_disasm_t      op;
	char               str[256];

	pce_prt_sep (fp, "PPC405");

	c = sim->ppc;

	opcnt = p405_get_opcnt (c);
	clkcnt = p405_get_clkcnt (c);
	delay = p405_get_delay (c);

	t = (double) (clock() - sim->real_clk) / CLOCKS_PER_SEC;
	efrq = (t < 0.001) ? 0.0 : (clkcnt / (1000.0 * 1000.0 * t));

	fprintf (fp,
		"CLK=%llx  OP=%llx  DLY=%lx  CPI=%.4f  TIME=%.4f  EFRQ=%.6f\n",
		clkcnt, opcnt, delay,
		(opcnt > 0) ? ((double) (clkcnt + delay) / (double) opcnt) : 1.0,
		t, efrq
	);

	fprintf (fp,
		" MSR=%08lX  XER=%08lX   CR=%08lX  XER=[%c%c%c]"
		"     CR0=[%c%c%c%c]\n",
		(unsigned long) p405_get_msr (c),
		(unsigned long) p405_get_xer (c),
		(unsigned long) p405_get_cr (c),
		(p405_get_xer_so (c)) ? 'S' : '-',
		(p405_get_xer_ov (c)) ? 'O' : '-',
		(p405_get_xer_ca (c)) ? 'C' : '-',
		(p405_get_cr_lt (c, 0)) ? 'L' : '-',
		(p405_get_cr_gt (c, 0)) ? 'G' : '-',
		(p405_get_cr_eq (c, 0)) ? 'E' : '-',
		(p405_get_cr_so (c, 0)) ? 'O' : '-'
	);

	fprintf (fp,
		"  PC=%08lX  CTR=%08lX   LR=%08lX  PID=%08lX  ZPR=%08lX\n",
		(unsigned long) p405_get_pc (c),
		(unsigned long) p405_get_ctr (c),
		(unsigned long) p405_get_lr (c),
		(unsigned long) p405_get_pid (c),
		(unsigned long) p405_get_zpr (c)
	);

	fprintf (fp,
		"SRR0=%08lX SRR1=%08lX SRR2=%08lX SRR3=%08lX  ESR=%08lX\n",
		(unsigned long) p405_get_srr (c, 0),
		(unsigned long) p405_get_srr (c, 1),
		(unsigned long) p405_get_srr (c, 2),
		(unsigned long) p405_get_srr (c, 3),
		(unsigned long) p405_get_esr (c)
	);

	for (i = 0; i < 8; i++) {
		fprintf (fp,
			" R%02u=%08lX  R%02u=%08lX  R%02u=%08lX  R%02u=%08lX"
			"  SP%u=%08lX\n",
			i + 0, (unsigned long) p405_get_gpr (c, i + 0),
			i + 8, (unsigned long) p405_get_gpr (c, i + 8),
			i + 16, (unsigned long) p405_get_gpr (c, i + 16),
			i + 24, (unsigned long) p405_get_gpr (c, i + 24),
			i, (unsigned long) p405_get_sprg (c, i)
		);
	}

	p405_disasm_mem (c, &op, p405_get_pc (c), par_xlat);
	ppc_disasm_str (str, &op);

	fprintf (fp, "%08lX  %s\n", (unsigned long) p405_get_pc (c), str);
}

void s405_prt_state_spr (p405_t *c, FILE *fp)
{
	uint32_t msr;

	pce_prt_sep (fp, "PPC405 SPR");

	msr = p405_get_msr (c);

	fprintf (fp,
		" MSR=%08lX  [%s %s %s %s %s %s %s %s %s %s %s %s %s %s]\n",
		(unsigned long) msr,
		(msr & P405_MSR_AP) ? "AP" : "ap",
		(msr & P405_MSR_APE) ? "APE" : "ape",
		(msr & P405_MSR_WE) ? "WE" : "we",
		(msr & P405_MSR_CE) ? "CE" : "ce",
		(msr & P405_MSR_EE) ? "EE" : "ee",
		(msr & P405_MSR_PR) ? "PR" : "pr",
		(msr & P405_MSR_FP) ? "FP" : "fp",
		(msr & P405_MSR_ME) ? "ME" : "me",
		(msr & P405_MSR_FE0) ? "FE0" : "fe0",
		(msr & P405_MSR_DWE) ? "DWE" : "dwe",
		(msr & P405_MSR_DE) ? "DE" : "de",
		(msr & P405_MSR_FE0) ? "FE1" : "fe1",
		(msr & P405_MSR_IR) ? "IR" : "ir",
		(msr & P405_MSR_DR) ? "DR" : "dr"
	);

	fprintf (fp,
		"  PC=%08lX   XER=%08lX    CR=%08lX   PID=%08lX   ZPR=%08lX\n",
		(unsigned long) p405_get_pc (c),
		(unsigned long) p405_get_xer (c),
		(unsigned long) p405_get_cr (c),
		(unsigned long) p405_get_pid (c),
		(unsigned long) p405_get_zpr (c)
	);

	fprintf (fp,
		"SPG0=%08lX  SRR0=%08lX    LR=%08lX   CTR=%08lX   ESR=%08lX\n",
		(unsigned long) p405_get_sprg (c, 0),
		(unsigned long) p405_get_srr (c, 0),
		(unsigned long) p405_get_lr (c),
		(unsigned long) p405_get_ctr (c),
		(unsigned long) p405_get_esr (c)
	);

	fprintf (fp,
		"SPG1=%08lX  SRR1=%08lX   TBU=%08lX   TBL=%08lX  DEAR=%08lX\n",
		(unsigned long) p405_get_sprg (c, 1),
		(unsigned long) p405_get_srr (c, 1),
		(unsigned long) p405_get_tbu (c),
		(unsigned long) p405_get_tbl (c),
		(unsigned long) p405_get_dear (c)
	);

	fprintf (fp, "SPG2=%08lX  SRR2=%08lX  EVPR=%08lX\n",
		(unsigned long) p405_get_sprg (c, 2),
		(unsigned long) p405_get_srr (c, 2),
		(unsigned long) p405_get_evpr (c)
	);

	fprintf (fp,
		"SPG3=%08lX  SRR3=%08lX  DCCR=%08lX  DCWR=%08lX  ICCR=%08lX\n",
		(unsigned long) p405_get_sprg (c, 3),
		(unsigned long) p405_get_srr (c, 3),
		(unsigned long) p405_get_dccr (c),
		(unsigned long) p405_get_dcwr (c),
		(unsigned long) p405_get_iccr (c)
	);

	fprintf (fp, "SPG4=%08lX  DBC0=%08lX  PIT0=%08lX  PIT1=%08lX\n",
		(unsigned long) p405_get_sprg (c, 4),
		(unsigned long) p405_get_dbcr0 (c),
		(unsigned long) p405_get_pit (c, 0),
		(unsigned long) p405_get_pit (c, 1)
	);

	fprintf (fp, "SPG5=%08lX  DBC1=%08lX\n",
		(unsigned long) p405_get_sprg (c, 5),
		(unsigned long) p405_get_dbcr0 (c)
	);

	fprintf (fp, "SPG6=%08lX  DBSR=%08lX\n",
		(unsigned long) p405_get_sprg (c, 6),
		(unsigned long) p405_get_dbsr (c)
	);

	fprintf (fp, "SPG7=%08lX\n",
		(unsigned long) p405_get_sprg (c, 7)
	);
}

void s405_prt_state_uic (p405_uic_t *uic, FILE *fp)
{
	pce_prt_sep (fp, "UIC");

	fprintf (fp,
		"  L=%08lX  N00=%08lX  N08=%08lX  N16=%08lX  N24=%08lX\n",
		(unsigned long) p405uic_get_levels (uic),
		p405uic_get_int_cnt (uic, 0),
		p405uic_get_int_cnt (uic, 8),
		p405uic_get_int_cnt (uic, 16),
		p405uic_get_int_cnt (uic, 24)
	);
	fprintf (fp,
		" SR=%08lX  N01=%08lX  N09=%08lX  N17=%08lX  N25=%08lX\n",
		(unsigned long) p405uic_get_sr (uic),
		p405uic_get_int_cnt (uic, 1),
		p405uic_get_int_cnt (uic, 9),
		p405uic_get_int_cnt (uic, 17),
		p405uic_get_int_cnt (uic, 25)
	);
	fprintf (fp,
		" ER=%08lX  N02=%08lX  N10=%08lX  N18=%08lX  N26=%08lX\n",
		(unsigned long) p405uic_get_er (uic),
		p405uic_get_int_cnt (uic, 2),
		p405uic_get_int_cnt (uic, 10),
		p405uic_get_int_cnt (uic, 18),
		p405uic_get_int_cnt (uic, 26)
	);
	fprintf (fp,
		"MSR=%08lX  N03=%08lX  N11=%08lX  N19=%08lX  N27=%08lX\n",
		(unsigned long) p405uic_get_msr (uic),
		p405uic_get_int_cnt (uic, 3),
		p405uic_get_int_cnt (uic, 11),
		p405uic_get_int_cnt (uic, 19),
		p405uic_get_int_cnt (uic, 27)
	);
	fprintf (fp,
		" CR=%08lX  N04=%08lX  N12=%08lX  N20=%08lX  N28=%08lX\n",
		(unsigned long) p405uic_get_cr (uic),
		p405uic_get_int_cnt (uic, 4),
		p405uic_get_int_cnt (uic, 12),
		p405uic_get_int_cnt (uic, 20),
		p405uic_get_int_cnt (uic, 28)
	);
	fprintf (fp,
		" PR=%08lX  N05=%08lX  N13=%08lX  N21=%08lX  N29=%08lX\n",
		(unsigned long) p405uic_get_pr (uic),
		p405uic_get_int_cnt (uic, 5),
		p405uic_get_int_cnt (uic, 13),
		p405uic_get_int_cnt (uic, 21),
		p405uic_get_int_cnt (uic, 29)
	);
	fprintf (fp,
		" TR=%08lX  N06=%08lX  N14=%08lX  N22=%08lX  N30=%08lX\n",
		(unsigned long) p405uic_get_tr (uic),
		p405uic_get_int_cnt (uic, 6),
		p405uic_get_int_cnt (uic, 14),
		p405uic_get_int_cnt (uic, 22),
		p405uic_get_int_cnt (uic, 30)
	);
	fprintf (fp,
		"VCR=%08lX  N07=%08lX  N15=%08lX  N23=%08lX  N31=%08lX\n",
		(unsigned long) p405uic_get_vcr (uic),
		p405uic_get_int_cnt (uic, 7),
		p405uic_get_int_cnt (uic, 15),
		p405uic_get_int_cnt (uic, 23),
		p405uic_get_int_cnt (uic, 31)
	);
	fprintf (fp, " VR=%08lX\n", (unsigned long) p405uic_get_vr (uic));
}

void s405_prt_state_mem (sim405_t *sim, FILE *fp)
{
	pce_prt_sep (fp, "PPC MEM");
	mem_prt_state (sim->mem, fp);
}


static
void ppc_exec (sim405_t *sim)
{
	unsigned long long old;

	old = p405_get_opcnt (sim->ppc);

	while (p405_get_opcnt (sim->ppc) == old) {
		s405_clock (sim, 1);
	}
}

static
void ppc_run_bp (sim405_t *sim)
{
	pce_start();

	while (1) {
		ppc_exec (sim);

		if (bp_check (&sim->brkpt, p405_get_pc (sim->ppc), 0)) {
			break;
		}

		if (sim->brk) {
			break;
		}
	}

	pce_stop();
}

static
int ppc_exec_to (sim405_t *sim, unsigned long addr)
{
	while (p405_get_pc (sim->ppc) != addr) {
		s405_clock (sim, 1);

		if (sim->brk) {
			return (1);
		}
	}

	return (0);
}

static
int ppc_exec_off (sim405_t *sim, unsigned long addr)
{
	while (p405_get_pc (sim->ppc) == addr) {
		s405_clock (sim, 1);

		if (sim->brk) {
			return (1);
		}
	}

	return (0);
}


static
void ppc_hook (sim405_t *sim, unsigned long ir)
{
#if 0
	fprintf (stderr, "%08lX: hook (%lu)\n",
		(unsigned long) p405_get_pc (sim->ppc),
		(unsigned long) (val & 0x0000ffff)
	);
#endif
}

#if 0
static
void ppc_log_opcode (void *ext, unsigned long ir)
{
	sim405_t *sim;

	sim = (sim405_t *) ext;
}
#endif

static
void ppc_log_undef (void *ext, unsigned long ir)
{
	unsigned op1, op2;
	sim405_t *sim;

	sim = (sim405_t *) ext;

	op1 = (ir >> 26) & 0x3f;
	op2 = (ir >> 1) & 0x3ff;

	pce_log (MSG_DEB,
		"%08lX: undefined operation [%08lX] op1=%02X op2=%03X\n",
		(unsigned long) p405_get_pc (sim->ppc), ir, op1, op2
	);

	s405_break (sim, PCE_BRK_STOP);
}

static
void ppc_log_exception (void *ext, unsigned long ofs)
{
	sim405_t *sim;
	char     *name;

	sim = (sim405_t *) ext;

	switch (ofs) {
	case 0x0300:
		name = "data store";
		return;

	case 0x0400:
		name = "instruction store";
		return;

	case 0x0500:
		name = "external interrupt";
		return;

	case 0x0700:
		name = "program";
		return;

	case 0x0800:
		name = "fpu unavailable";
		break;

	case 0x0c00:
		name = "system call";
		return;

	case 0x1000:
		name = "PIT";
		return;

	case 0x1100:
		name = "TLB miss data";
		return;
		break;

	case 0x1200:
		name = "TLB miss instruction";
		return;

	default:
		name = "unknown";
		break;
	}

	pce_log (MSG_DEB, "%08lX: exception %x (%s)\n",
		(unsigned long) p405_get_pc (sim->ppc), ofs, name
	);
}

#if 0
static
void ppc_log_mem (void *ext, unsigned mode,
	unsigned long raddr, unsigned long vaddr, unsigned long val)
{
	sim405_t *sim;

	sim = (sim405_t *) ext;

	if ((raddr >= 0xe0000000UL) && (raddr < 0xef600000UL)) {
		pce_log (MSG_DEB, "mem: 0x%08lx 0x%08lx %02x\n", raddr, val, mode);
	}
}
#endif

static
void do_bc (cmd_t *cmd, sim405_t *sim)
{
	unsigned long addr;

	if (cmd_match_eol (cmd)) {
		bp_clear_all (&sim->brkpt);
		return;
	}

	if (!cmd_match_uint32 (cmd, &addr)) {
		cmd_error (cmd, "expecting address");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (bp_clear (&sim->brkpt, addr, 0)) {
		printf ("no breakpoint cleared at %08lX\n", addr);
	}
}

static
void do_bl (cmd_t *cmd, sim405_t *sim)
{
	if (!cmd_match_end (cmd)) {
		return;
	}

	bp_list (sim->brkpt, 0);
}

static
void do_bs (cmd_t *cmd, sim405_t *sim)
{
	unsigned long  addr;
	unsigned short pass, reset;

	addr = p405_get_pc (sim->ppc);
	pass = 1;
	reset = 0;

	if (!cmd_match_uint32 (cmd, &addr)) {
		cmd_error (cmd, "expecting address");
		return;
	}

	cmd_match_uint16 (cmd, &pass);
	cmd_match_uint16 (cmd, &reset);

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (pass > 0) {
		printf ("Breakpoint at %08lX  %04X  %04X\n",
			addr, pass, reset
		);

		bp_add (&sim->brkpt, addr, 0, pass, reset);
	}
}

static
void do_b (cmd_t *cmd, sim405_t *sim)
{
	if (cmd_match (cmd, "l")) {
		do_bl (cmd, sim);
	}
	else if (cmd_match (cmd, "s")) {
		do_bs (cmd, sim);
	}
	else if (cmd_match (cmd, "c")) {
		do_bc (cmd, sim);
	}
	else {
		cmd_error (cmd, "b: unknown command");
	}
}

static
void do_c (cmd_t *cmd, sim405_t *sim)
{
	unsigned long cnt;

	cnt = 1;

	cmd_match_uint32 (cmd, &cnt);

	if (!cmd_match_end (cmd)) {
		return;
	}

	while (cnt > 0) {
		s405_clock (sim, 1);
		cnt -= 1;
	}

	s405_prt_state_ppc (sim, stdout);
}

static
void do_d (cmd_t *cmd, sim405_t *sim)
{
	unsigned long        i, j;
	unsigned long        cnt;
	unsigned long        addr1, addr2;
	static int           first = 1;
	static unsigned long saddr = 0;
	unsigned long        p, p1, p2;
	char                 buf[256];

	if (first) {
		first = 0;
		saddr = p405_get_pc (sim->ppc);
	}

	addr1 = saddr;
	cnt = 256;

	if (cmd_match_uint32 (cmd, &addr1)) {
		cmd_match_uint32 (cmd, &cnt);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	addr2 = (addr1 + cnt - 1) & 0xffffffffUL;
	if (addr2 < addr1) {
		addr2 = 0xffffffffUL;
		cnt = addr2 - addr1 + 1;
	}

	saddr = addr1 + cnt;

	p1 = addr1 / 16;
	p2 = addr2 / 16 + 1;

	for (p = p1; p < p2; p++) {
		j = 16 * p;

		sprintf (buf,
			"%08lX  xx xx xx xx xx xx xx xx-xx xx xx xx xx xx xx xx  xxxxxxxxxxxxxxxx\n",
			j
		);

		for (i = 0; i < 16; i++) {
			if ((j >= addr1) && (j <= addr2)) {
				uint8_t  val;
				unsigned val1, val2;

				if (p405_get_xlat8 (sim->ppc, j, par_xlat, &val)) {
					val = 0xff;
				}

				val1 = (val >> 4) & 0x0f;
				val2 = val & 0x0f;

				buf[10 + 3 * i + 0] = (val1 < 10) ? ('0' + val1) : ('A' + val1 - 10);
				buf[10 + 3 * i + 1] = (val2 < 10) ? ('0' + val2) : ('A' + val2 - 10);

				if ((val >= 32) && (val <= 127)) {
					buf[59 + i] = val;
				}
				else {
					buf[59 + i] = '.';
				}
			}
			else {
				buf[10 + 3 * i] = ' ';
				buf[10 + 3 * i + 1] = ' ';
				buf[59 + i] = ' ';
			}

			j += 1;
		}

		fputs (buf, stdout);
	}
}

static
void do_e (cmd_t *cmd, sim405_t *sim)
{
	unsigned long  addr;
	unsigned short val;

	addr = 0;

	if (!cmd_match_uint32 (cmd, &addr)) {
		cmd_error (cmd, "need an address");
		return;
	}

	while (cmd_match_uint16 (cmd, &val)) {
		if (p405_set_xlat8 (sim->ppc, addr, par_xlat, val)) {
			printf ("TLB miss: %08lx <- %02x\n", addr, (unsigned) val);
		}

		addr += 1;
	}
}

static
void do_g (cmd_t *cmd, sim405_t *sim)
{
	int           run;
	unsigned long addr;

	if (cmd_match (cmd, "b")) {
		run = 0;
	}
	else {
		run = 1;
	}

	if (cmd_match_uint32 (cmd, &addr)) {
		bp_add (&sim->brkpt, addr, 0, 1, 0);
		run = 0;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (run) {
		pce_run();
	}
	else {
		ppc_run_bp (sim);

		fputs ("\n", stdout);
		s405_prt_state_ppc (sim, stdout);
	}
}

static
void do_h (cmd_t *cmd, sim405_t *sim)
{
	fputs (
		"bc [addr]                 clear a breakpoint or all\n"
		"bl                        list breakpoints\n"
		"bs addr [pass [reset]]    set a breakpoint [pass=1 reset=0]\n"
		"c [cnt]                   clock\n"
		"d [addr [cnt]]            dump memory\n"
		"e addr [val...]           enter bytes into memory\n"
		"g [b]                     run with or without breakpoints (^` to stop)\n"
		"key [val...]              send keycodes to the serial console\n"
		"m msg [val]               send a message to the core\n"
		"p [cnt]                   execute cnt instructions, skip calls [1]\n"
		"q                         quit\n"
		"rfi                       execute to next rfi or rfci\n"
		"r reg [val]               set a register\n"
		"s [what]                  print status (mem|ppc|spr)\n"
		"tlb l [first [count]]     list TLB entries\n"
		"tlb s addr                search the TLB\n"
		"t [cnt]                   execute cnt instructions [1]\n"
		"u [addr [cnt]]            disassemble\n"
		"v [expr...]               evaluate expressions\n"
		"x [c|r|v]                 set the translation mode (cpu, real, virtual)\n",
		stdout
	);
}

static
void do_key (cmd_t *cmd, sim405_t *sim)
{
	unsigned short c;

	while (cmd_match_uint16 (cmd, &c)) {
		s405_set_keycode (sim, c);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}
}

static
void do_p (cmd_t *cmd, sim405_t *sim)
{
	unsigned long cnt;
	p405_disasm_t dis;

	cnt = 1;

	cmd_match_uint32 (cmd, &cnt);

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start();

	while (cnt > 0) {
		p405_disasm_mem (sim->ppc, &dis, p405_get_pc (sim->ppc), P405_XLAT_CPU);

		if (dis.flags & P405_DFLAG_CALL) {
			if (ppc_exec_to (sim, p405_get_pc (sim->ppc) + 4)) {
				break;
			}
		}
		else {
			uint32_t msr;

			msr = p405_get_msr (sim->ppc);

			if (ppc_exec_off (sim, p405_get_pc (sim->ppc))) {
				break;
			}

			if (msr & P405_MSR_PR) {
				/* check if exception occured */
				while ((p405_get_msr (sim->ppc) & P405_MSR_PR) == 0) {
					s405_clock (sim, 1);

					if (sim->brk) {
						break;
					}
				}
			}
		}

		cnt -= 1;
	}

	pce_stop();

	s405_prt_state_ppc (sim, stdout);
}

static
void do_rfi (cmd_t *cmd, sim405_t *sim)
{
	p405_disasm_t dis;

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start();

	while (1) {
		p405_disasm_mem (sim->ppc, &dis, p405_get_pc (sim->ppc),
			P405_XLAT_CPU
		);

		if (ppc_exec_off (sim, p405_get_pc (sim->ppc))) {
			break;
		}

		if (dis.flags & P405_DFLAG_RFI) {
			break;
		}
	}

	pce_stop();

	s405_prt_state_ppc (sim, stdout);
}

static
void do_r (cmd_t *cmd, sim405_t *sim)
{
	unsigned long val;
	uint32_t      *reg;

	if (!ppc_match_reg (cmd, sim, &reg)) {
		printf ("missing register\n");
		return;
	}

	if (cmd_match_eol (cmd)) {
		printf ("%08lx\n", (unsigned long) *reg);
		return;
	}

	if (!cmd_match_uint32 (cmd, &val)) {
		printf ("missing value\n");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	*reg = val;

	s405_prt_state_ppc (sim, stdout);
}

static
void do_s (cmd_t *cmd, sim405_t *sim)
{
	if (cmd_match_eol (cmd)) {
		s405_prt_state_ppc (sim, stdout);
		return;
	}

	prt_state (sim, stdout, cmd_get_str (cmd));
}

static
void do_tlb_l (cmd_t *cmd, sim405_t *sim)
{
	unsigned long i, n, max;
	p405_tlbe_t *ent;

	i = 0;
	n = p405_get_tlb_entry_cnt (sim->ppc);

	max = n;

	if (cmd_match_uint32 (cmd, &i)) {
		cmd_match_uint32 (cmd, &n);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	i = i % max;

	while (n > 0) {
		ent = p405_get_tlb_entry_idx (sim->ppc, i);

		if (ent != NULL) {
			prt_tlb_entry (stdout, ent, i);
			fputs ("\n", stdout);
		}

		i = (i + 1) % max;
		n -= 1;
	}
}

static
void do_tlb_s (cmd_t *cmd, sim405_t *sim)
{
	unsigned long addr;
	unsigned      idx;
	p405_tlbe_t *ent;

	if (!cmd_match_uint32 (cmd, &addr)) {
		cmd_error (cmd, "expecting address");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	idx = p405_get_tlb_index (sim->ppc, addr);
	ent = p405_get_tlb_entry_idx (sim->ppc, idx);

	if (ent == NULL) {
		printf ("no match\n");
	}
	else {
		prt_tlb_entry (stdout, ent, idx);
		fputs ("\n", stdout);
	}
}

static
void do_tlb (cmd_t *cmd, sim405_t *sim)
{
	if (cmd_match (cmd, "l")) {
		do_tlb_l (cmd, sim);
	}
	else if (cmd_match (cmd, "s")) {
		do_tlb_s (cmd, sim);
	}
	else {
		cmd_error (cmd, "unknown tlb command");
	}
}

static
void do_t (cmd_t *cmd, sim405_t *sim)
{
	unsigned long i, n;

	n = 1;

	cmd_match_uint32 (cmd, &n);

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start();

	for (i = 0; i < n; i++) {
		ppc_exec (sim);
	}

	pce_stop();

	s405_prt_state_ppc (sim, stdout);
}

static
void do_u (cmd_t *cmd, sim405_t *sim)
{
	unsigned             i;
	int                  to;
	unsigned long        addr, cnt;
	static unsigned int  first = 1;
	static unsigned long saddr = 0;
	p405_disasm_t        op;
	char                 str[256];

	if (first) {
		first = 0;
		saddr = p405_get_pc (sim->ppc);
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
		p405_disasm_mem (sim->ppc, &op, addr, par_xlat);
		ppc_disasm_str (str, &op);

		fprintf (stdout, "%08lX  %s\n", addr, str);

		addr += 4;
	}

	saddr = addr;
}

static
void do_x (cmd_t *cmd, sim405_t *sim)
{
	unsigned xlat;

	if (cmd_match_eol (cmd)) {
		switch (par_xlat) {
		case P405_XLAT_CPU:
			printf ("xlat cpu\n");
			break;

		case P405_XLAT_REAL:
			printf ("xlat real\n");
			break;

		case P405_XLAT_VIRTUAL:
			printf ("xlat virtual\n");
			break;

		default:
			printf ("xlat unknown\n");
			break;
		}

		return;
	}

	if (cmd_match (cmd, "c")) {
		xlat = P405_XLAT_CPU;
	}
	else if (cmd_match (cmd, "r")) {
		xlat = P405_XLAT_REAL;
	}
	else if (cmd_match (cmd, "v")) {
		xlat = P405_XLAT_VIRTUAL;
	}
	else {
		cmd_error (cmd, "unknown translation type");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	par_xlat = xlat;
}

int ppc_do_cmd (sim405_t *sim, cmd_t *cmd)
{
	if (cmd_match (cmd, "b")) {
		do_b (cmd, sim);
	}
	else if (cmd_match (cmd, "c")) {
		do_c (cmd, sim);
	}
	else if (cmd_match (cmd, "d")) {
		do_d (cmd, sim);
	}
	else if (cmd_match (cmd, "e")) {
		do_e (cmd, sim);
	}
	else if (cmd_match (cmd, "g")) {
		do_g (cmd, sim);
	}
	else if (cmd_match (cmd, "h")) {
		do_h (cmd, sim);
	}
	else if (cmd_match (cmd, "key")) {
		do_key (cmd, sim);
	}
	else if (cmd_match (cmd, "p")) {
		do_p (cmd, sim);
	}
	else if (cmd_match (cmd, "rfi")) {
		do_rfi (cmd, sim);
	}
	else if (cmd_match (cmd, "r")) {
		do_r (cmd, sim);
	}
	else if (cmd_match (cmd, "s")) {
		do_s (cmd, sim);
	}
	else if (cmd_match (cmd, "tlb")) {
		do_tlb (cmd, sim);
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

void ppc_cmd_init (sim405_t *sim)
{
	sim->ppc->log_ext = sim;
	sim->ppc->log_opcode = NULL;
	sim->ppc->log_undef = &ppc_log_undef;
	sim->ppc->log_exception = &ppc_log_exception;
	sim->ppc->log_mem = NULL;

	p405_set_hook_fct (sim->ppc, sim, ppc_hook);
}
