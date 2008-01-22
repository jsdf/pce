/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/ibmpc/main.c                                      *
 * Created:       1999-04-16 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 1996-2007 Hampa Hug <hampa@hampa.ch>                   *
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


#include "main.h"

#include <signal.h>


static void prt_state_cpu (e8086_t *c);


char                      *par_terminal = NULL;
char                      *par_video = NULL;
char                      *par_cpu = NULL;
unsigned long             par_int28 = 10000;

monitor_t                 par_mon;

ibmpc_t                   *par_pc = NULL;

ini_sct_t                 *par_cfg = NULL;

static unsigned           par_boot = 128;

static unsigned           pce_last_int = 0;

static ibmpc_t            *pc;



static
void prt_help (void)
{
	fputs (
		"usage: ibmpc [options]\n"
		"  --help                 Print usage information\n"
		"  --version              Print version information\n"
		"  -b, --boot int         Set boot drive [128]\n"
		"  -c, --config string    Set the config file\n"
		"  -g, --video string     Set video device\n"
		"  -l, --log string       Set the log file\n"
		"  -p, --cpu string       Set the cpu model\n"
		"  -q, --quiet            Quiet operation [no]\n"
		"  -r, --run              Start running immediately\n"
		"  -R, --no-monitor       Never stop running\n"
		"  -t, --terminal string  Set terminal\n"
		"  -v, --verbose          Verbose operation [no]\n",
		stdout
	);

	fflush (stdout);
}

static
void prt_version (void)
{
	fputs (
		"pce ibmpc version " PCE_VERSION_STR
		" (" PCE_CFG_DATE " " PCE_CFG_TIME ")\n"
		"Copyright (C) 1995-2007 Hampa Hug <hampa@hampa.ch>\n",
		stdout
	);

	fflush (stdout);
}

void sig_int (int s)
{
	/* hmm... */
}

void sig_segv (int s)
{
	fprintf (stderr, "pce: segmentation fault\n");
	fflush (stderr);

	if ((pc != NULL) && (pc->cpu != NULL)) {
		prt_state_cpu (pc->cpu);
	}

	pce_set_fd_interactive (0, 1);

	exit (1);
}

static
int cmd_get_sym (ibmpc_t *pc, const char *sym, unsigned long *val)
{
	if (e86_get_reg (pc->cpu, sym, val) == 0) {
		return (0);
	}

	return (1);
}

static
int cmd_set_sym (ibmpc_t *pc, const char *sym, unsigned long val)
{
	if (e86_set_reg (pc->cpu, sym, val) == 0) {
		return (0);
	}

	return (1);
}

static
void disasm_str (char *dst, e86_disasm_t *op)
{
	unsigned     i;
	unsigned     dst_i;

	dst_i = 2;
	sprintf (dst, "%02X", op->dat[0]);

	for (i = 1; i < op->dat_n; i++) {
		sprintf (dst + dst_i, " %02X", op->dat[i]);
		dst_i += 3;
	}

	dst[dst_i++] = ' ';
	while (dst_i < 20) {
		dst[dst_i++] = ' ';
	}

	if ((op->flags & ~(E86_DFLAGS_CALL | E86_DFLAGS_LOOP)) != 0) {
		unsigned flg;

		flg = op->flags;

		dst[dst_i++] = '[';

		if (flg & E86_DFLAGS_186) {
			dst_i += sprintf (dst + dst_i, "186");
			flg &= ~E86_DFLAGS_186;
		}

		if (flg != 0) {
			if (flg != op->flags) {
				dst[dst_i++] = ' ';
			}
			dst_i += sprintf (dst + dst_i, " %04X", flg);
		}
		dst[dst_i++] = ']';
		dst[dst_i++] = ' ';
	}

	strcpy (dst + dst_i, op->op);
	while (dst[dst_i] != 0) {
		dst_i += 1;
	}

	if (op->arg_n > 0) {
		dst[dst_i++] = ' ';
		while (dst_i < 26) {
			dst[dst_i++] = ' ';
		}
	}

	if (op->arg_n == 1) {
		dst_i += sprintf (dst + dst_i, op->arg1);
	}
	else if (op->arg_n == 2) {
		dst_i += sprintf (dst + dst_i, "%s, %s", op->arg1, op->arg2);
	}

	dst[dst_i] = 0;
}

static
void prt_uint8_bin (unsigned char val)
{
	unsigned      i;
	unsigned char m;
	char          str[16];

	m = 0x80;

	for (i = 0; i < 8; i++) {
		str[i] = (val & m) ? '1' : '0';
		m = m >> 1;
	}

	str[8] = 0;

	pce_puts (str);
}

static
void prt_state_video (video_t *vid)
{
	FILE *fp;

	pce_prt_sep ("video");

	pce_video_prt_state (vid, pce_get_fp_out());

	fp = pce_get_redir_out();
	if (fp != NULL) {
		pce_video_prt_state (vid, fp);
	}
}

static
void prt_state_ems (ems_t *ems)
{
	pce_prt_sep ("EMS");
	ems_prt_state (ems);
}

static
void prt_state_xms (xms_t *xms)
{
	pce_prt_sep ("XMS");
	xms_prt_state (xms);
}

static
void prt_state_pit (e8253_t *pit)
{
	unsigned        i;
	e8253_counter_t *cnt;

	pce_prt_sep ("8253-PIT");

	for (i = 0; i < 3; i++) {
		cnt = &pit->counter[i];

		pce_printf (
			"C%d: SR=%02X M=%u RW=%d  CE=%04X  %s=%02X %s=%02X  %s=%02X %s=%02X  "
			"G=%u O=%u R=%d\n",
			i,
			cnt->sr, cnt->mode, cnt->rw,
			cnt->val,
			(cnt->cr_wr & 2) ? "cr1" : "CR1", cnt->cr[1],
			(cnt->cr_wr & 1) ? "cr0" : "CR0", cnt->cr[0],
			(cnt->ol_rd & 2) ? "ol1" : "OL1", cnt->ol[1],
			(cnt->ol_rd & 1) ? "ol0" : "OL0", cnt->ol[0],
			(unsigned) cnt->gate,
			(unsigned) cnt->out_val,
			cnt->counting
		);
	}
}

static
void prt_state_ppi (e8255_t *ppi)
{
	pce_prt_sep ("8255-PPI");

	pce_printf (
		"MOD=%02X  MODA=%u  MODB=%u",
		ppi->mode, ppi->group_a_mode, ppi->group_b_mode
	);

	if (ppi->port[0].inp != 0) {
		pce_printf ("  A=I[%02X]", e8255_get_inp (ppi, 0));
	}
	else {
		pce_printf ("  A=O[%02X]", e8255_get_out (ppi, 0));
	}

	if (ppi->port[1].inp != 0) {
		pce_printf ("  B=I[%02X]", e8255_get_inp (ppi, 1));
	}
	else {
		pce_printf ("  B=O[%02X]", e8255_get_out (ppi, 1));
	}

	switch (ppi->port[2].inp) {
		case 0xff:
			pce_printf ("  C=I[%02X]", e8255_get_inp (ppi, 2));
			break;

		case 0x00:
			pce_printf ("  C=O[%02X]", e8255_get_out (ppi, 2));
			break;

		case 0x0f:
			pce_printf ("  CH=O[%X]  CL=I[%X]",
				(e8255_get_out (ppi, 2) >> 4) & 0x0f,
				e8255_get_inp (ppi, 2) & 0x0f
			);
			break;

		case 0xf0:
			pce_printf ("  CH=I[%X]  CL=O[%X]",
				(e8255_get_inp (ppi, 2) >> 4) & 0x0f,
				e8255_get_out (ppi, 2) & 0x0f
			);
			break;
	}

	pce_puts ("\n");
}

static
void prt_state_dma (e8237_t *dma)
{
	unsigned i;

	pce_prt_sep ("8237-DMAC");

	pce_printf ("CMD=%02X  PRI=%02X  CHK=%d\n",
		e8237_get_command (dma),
		e8237_get_priority (dma),
		dma->check != 0
	);

	for (i = 0; i < 4; i++) {
		unsigned short state;

		state = e8237_get_state (dma, i);

		pce_printf (
			"CHN %u: MODE=%02X ADDR=%04X[%04X] CNT=%04X[%04X] DREQ=%d SREQ=%d MASK=%d\n",
			i,
			e8237_get_mode (dma, i) & 0xfcU,
			e8237_get_addr (dma, i),
			e8237_get_addr_base (dma, i),
			e8237_get_cnt (dma, i),
			e8237_get_cnt_base (dma, i),
			(state & E8237_STATE_DREQ) != 0,
			(state & E8237_STATE_SREQ) != 0,
			(state & E8237_STATE_MASK) != 0
		);
	}
}

static
void prt_state_pic (e8259_t *pic)
{
	unsigned i;

	pce_prt_sep ("8259A-PIC");

	pce_puts ("IRR=");
	prt_uint8_bin (e8259_get_irr (pic));

	pce_puts ("  IMR=");
	prt_uint8_bin (e8259_get_imr (pic));

	pce_puts ("  ISR=");
	prt_uint8_bin (e8259_get_isr (pic));

	pce_puts ("\n");

	pce_printf ("ICW=[%02X %02X %02X %02X]  OCW=[%02X %02X %02X]\n",
		e8259_get_icw (pic, 0), e8259_get_icw (pic, 1), e8259_get_icw (pic, 2),
		e8259_get_icw (pic, 3),
		e8259_get_ocw (pic, 0), e8259_get_ocw (pic, 1), e8259_get_ocw (pic, 2)
	);

	pce_printf ("N0=%04lX", pic->irq_cnt[0]);
	for (i = 1; i < 8; i++) {
		pce_printf ("  N%u=%04lX", i, pic->irq_cnt[i]);
	}

	pce_puts ("\n");
}

static
void prt_state_uart (e8250_t *uart, unsigned base)
{
	char p;

	switch (e8250_get_parity (uart)) {
	case E8250_PARITY_N:
		p = 'N';
		break;

	case E8250_PARITY_E:
		p = 'E';
		break;

	case E8250_PARITY_O:
		p = 'O';
		break;

	case E8250_PARITY_M:
		p = 'M';
		break;

	case E8250_PARITY_S:
		p = 'S';
		break;

	default:
		p = '?';
		break;
	}

	pce_prt_sep ("8250-UART");

	pce_printf (
		"IO=%04X  %lu %u%c%u  DTR=%d  RTS=%d\n"
		"TxD=%02X%c RxD=%02X%c SCR=%02X  DIV=%04X\n"
		"IER=%02X  IIR=%02X  LCR=%02X  LSR=%02X  MCR=%02X  MSR=%02X\n",
		base,
		e8250_get_bps (uart), e8250_get_databits (uart), p,
		e8250_get_stopbits (uart),
		e8250_get_dtr (uart), e8250_get_rts (uart),
		uart->txd[0], uart->txd[1] ? '*' : ' ',
		uart->rxd[0], uart->rxd[1] ? '*' : ' ',
		uart->scratch, uart->divisor,
		uart->ier, uart->iir, uart->lcr, uart->lsr, uart->mcr, uart->msr
	);
}

static
void prt_state_cpu (e8086_t *c)
{
	double      cpi, mips;
	static char ft[2] = { '-', '+' };

	pce_prt_sep ("8086");

	cpi = (c->instructions > 0) ? ((double) c->clocks / (double) c->instructions) : 1.0;
	mips = (c->clocks > 0) ? (4.77 * (double) c->instructions / (double) c->clocks) : 0.0;

	pce_printf ("CLK=%llu  OP=%llu  DLY=%lu  CPI=%.4f  MIPS=%.4f\n",
		c->clocks, c->instructions,
		c->delay,
		cpi, mips
	);

	pce_printf (
		"AX=%04X  BX=%04X  CX=%04X  DX=%04X  SP=%04X  BP=%04X  SI=%04X  DI=%04X INT=%02X\n",
		e86_get_ax (c), e86_get_bx (c), e86_get_cx (c), e86_get_dx (c),
		e86_get_sp (c), e86_get_bp (c), e86_get_si (c), e86_get_di (c),
		pce_last_int
	);

	pce_printf ("CS=%04X  DS=%04X  ES=%04X  SS=%04X  IP=%04X  F =%04X",
		e86_get_cs (c), e86_get_ds (c), e86_get_es (c), e86_get_ss (c),
		e86_get_ip (c), c->flg
	);

	pce_printf ("  I%c D%c O%c S%c Z%c A%c P%c C%c\n",
		ft[e86_get_if (c)], ft[e86_get_df (c)],
		ft[e86_get_of (c)], ft[e86_get_sf (c)],
		ft[e86_get_zf (c)], ft[e86_get_af (c)],
		ft[e86_get_pf (c)], ft[e86_get_cf (c)]
	);
}

static
void prt_state_mem (ibmpc_t *pc)
{
	pce_prt_sep ("PC MEM");
	mem_prt_state (pc->mem, stdout);
}

static
void prt_state_pc (ibmpc_t *pc)
{
	prt_state_video (pc->video);
	prt_state_ppi (&pc->ppi);
	prt_state_pit (&pc->pit);
	prt_state_pic (&pc->pic);
	prt_state_dma (&pc->dma);
	prt_state_cpu (pc->cpu);
}

static
void prt_state (ibmpc_t *pc)
{
	e86_disasm_t op;
	char         str[256];

	e86_disasm_cur (pc->cpu, &op);
	disasm_str (str, &op);

	prt_state_cpu (pc->cpu);

	pce_printf ("%04X:%04X  %s\n",
		(unsigned) e86_get_cs (pc->cpu),
		(unsigned) e86_get_ip (pc->cpu),
		str
	);
}

static
int pc_check_break (ibmpc_t *pc)
{
	unsigned short seg, ofs;
	unsigned long  addr;

	seg = e86_get_cs (pc->cpu);
	ofs = e86_get_ip (pc->cpu);
	addr = (seg << 16) | ofs;

	if (bps_check (&pc->bps, addr, stdout)) {
		return (1);
	}

	if (pc->brk) {
		return (1);
	}

	return (0);
}

static
void pc_exec (ibmpc_t *pc)
{
	unsigned long long old;

	old = e86_get_opcnt (pc->cpu);

	while (e86_get_opcnt (pc->cpu) == old) {
		pc_clock (pc);
	}
}

static
void pc_run (ibmpc_t *pc)
{
	pce_start (&pc->brk);

	while (pc->brk == 0) {
		pc_clock (pc);
		pc_clock (pc);

		while (pc->pause) {
			pce_usleep (250000UL);
			trm_check (pc->trm);
		}
	}

	pce_stop();
}

#if 0
static
void pce_op_stat (void *ext, unsigned char op1, unsigned char op2)
{
	ibmpc_t *pc;

	pc = (ibmpc_t *) ext;

}
#endif

static
void pce_op_int (void *ext, unsigned char n)
{
	pce_last_int = n;

	if (n == 0x28) {
		if (par_int28 > 0) {
			pce_usleep (par_int28);
		}
	}
}

static
void pce_op_undef (void *ext, unsigned char op1, unsigned char op2)
{
	ibmpc_t *pc;

	pc = (ibmpc_t *) ext;

	pce_log (MSG_DEB, "%04X:%04X: undefined operation [%02X %02x]\n",
		e86_get_cs (pc->cpu), e86_get_ip (pc->cpu), op1, op2
	);

	pce_usleep (100000UL);

	trm_check (pc->trm);
}


static
void do_boot (cmd_t *cmd)
{
	unsigned short val;

	if (cmd_match_eol (cmd)) {
		pce_printf ("boot drive is 0x%02x\n", pc_get_bootdrive (pc));
		return;
	}

	if (!cmd_match_uint16 (cmd, &val)) {
		cmd_error (cmd, "expecting boot drive");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	pc_set_bootdrive (pc, val);
}

static
void do_c (cmd_t *cmd)
{
	unsigned long cnt;

	cnt = 1;

	cmd_match_uint32 (cmd, &cnt);

	if (!cmd_match_end (cmd)) {
		return;
	}

	while (cnt > 0) {
		pc_clock (pc);
		cnt -= 1;
	}

	prt_state (pc);
}

static
void do_dump (cmd_t *cmd)
{
	FILE *fp;
	char what[256];
	char fname[256];

	if (!cmd_match_str (cmd, what, 256)) {
		cmd_error (cmd, "don't know what to dump");
		return;
	}

	if (!cmd_match_str (cmd, fname, 256)) {
		cmd_error (cmd, "need a file name");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	fp = fopen (fname, "wb");
	if (fp == NULL) {
		pce_printf ("dump: can't open file (%s)\n", fname);
		return;
	}

	if (strcmp (what, "ram") == 0) {
		fprintf (fp, "# RAM dump\n\n");
		pce_dump_hex (fp, pc->ram->data, pc->ram->size, 0, 16, "", 1);
	}
	else if (strcmp (what, "video") == 0) {
		if (pce_video_dump (pc->video, fp)) {
			pce_printf ("dumping video failed\n");
		}
	}
	else if (strcmp (what, "config") == 0) {
		if (ini_write_fp (par_cfg, fp)) {
			pce_printf ("dumping configuration failed\n");
		}
	}
	else {
		pce_printf ("dump: don't know what to dump (%s)\n", what);
	}

	fclose (fp);
}

static
void do_d (cmd_t *cmd)
{
	unsigned              i, j;
	unsigned short        cnt;
	unsigned short        seg, ofs1, ofs2;
	static int            first = 1;
	static unsigned short sseg = 0;
	static unsigned short sofs = 0;
	unsigned short        p, p1, p2;
	char                  buf[256];

	if (first) {
		first = 0;
		sseg = e86_get_ds (pc->cpu);
		sofs = 0;
	}

	seg = sseg;
	ofs1 = sofs;
	cnt = 256;

	if (cmd_match_uint16_16 (cmd, &seg, &ofs1)) {
		cmd_match_uint16 (cmd, &cnt);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	ofs2 = (ofs1 + cnt - 1) & 0xffff;
	if (ofs2 < ofs1) {
		ofs2 = 0xffff;
		cnt = ofs2 - ofs1 + 1;
	}

	sseg = seg;
	sofs = ofs1 + cnt;

	p1 = ofs1 / 16;
	p2 = ofs2 / 16 + 1;

	for (p = p1; p < p2; p++) {
		j = 16 * p;

		sprintf (buf,
			"%04X:%04X  xx xx xx xx xx xx xx xx-xx xx xx xx xx xx xx xx  xxxxxxxxxxxxxxxx\n",
			seg, j
		);

		for (i = 0; i < 16; i++) {
			if ((j >= ofs1) && (j <= ofs2)) {
				unsigned val, val1, val2;

				val = e86_get_mem8 (pc->cpu, seg, j);
				val1 = (val >> 4) & 0x0f;
				val2 = val & 0x0f;

				buf[11 + 3 * i + 0] = (val1 < 10) ? ('0' + val1) : ('A' + val1 - 10);
				buf[11 + 3 * i + 1] = (val2 < 10) ? ('0' + val2) : ('A' + val2 - 10);

				if ((val >= 32) && (val <= 127)) {
					buf[60 + i] = val;
				}
				else {
					buf[60 + i] = '.';
				}
			}
			else {
				buf[11 + 3 * i] = ' ';
				buf[11 + 3 * i + 1] = ' ';
				buf[60 + i] = ' ';
			}

			j += 1;
		}

		pce_puts (buf);
	}
}

static
void do_e (cmd_t *cmd)
{
	unsigned       i;
	unsigned short seg, ofs;
	unsigned long  addr;
	unsigned short val;
	char           buf[256];

	seg = 0;
	ofs = 0;

	if (!cmd_match_uint16_16 (cmd, &seg, &ofs)) {
		cmd_error (cmd, "need an address");
	}

	addr = (seg << 4) + ofs;

	while (1) {
		if (cmd_match_uint16 (cmd, &val)) {
			mem_set_uint8 (pc->mem, addr, val);
			addr += 1;
		}
		else if (cmd_match_str (cmd, buf, 256)) {
			i = 0;
			while (buf[i] != 0) {
				mem_set_uint8 (pc->mem, addr, buf[i]);
				addr += 1;
				i += 1;
			}
		}
		else {
			break;
		}
	}

	if (!cmd_match_end (cmd)) {
		return;
	}
}

static
void do_g_b (cmd_t *cmd, ibmpc_t *pc)
{
	unsigned short seg, ofs;
	breakpoint_t  *bp;

	while (cmd_match_uint16_16 (cmd, &seg, &ofs)) {
		bp = bp_segofs_new (seg, ofs);
		bps_bp_add (&pc->bps, bp);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&pc->brk);

	while (1) {
		pc_exec (pc);

		if (pc_check_break (pc)) {
			break;
		}
	}

	pce_stop();
}

static
void do_g_far (cmd_t *cmd, ibmpc_t *pc)
{
	unsigned short seg;

	if (!cmd_match_end (cmd)) {
		return;
	}

	seg = e86_get_cs (pc->cpu);

	pce_start (&pc->brk);

	while (1) {
		pc_exec (pc);

		if (e86_get_cs (pc->cpu) != seg) {
			prt_state (pc);
			break;
		}

		if (pc_check_break (pc)) {
			break;
		}
	}

	pce_stop();
}

static
void do_g (cmd_t *cmd, ibmpc_t *pc)
{
	if (cmd_match (cmd, "b")) {
		do_g_b (cmd, pc);
	}
	else if (cmd_match (cmd, "far")) {
		do_g_far (cmd, pc);
	}
	else {
		if (!cmd_match_end (cmd)) {
			return;
		}

		pc_run (pc);
	}
}

static
void do_h (cmd_t *cmd)
{
	if (cmd_match (cmd, "m")) {
		pce_puts (
			"emu.stop\n"
			"emu.exit\n"
			"emu.pause            <0|1>\n"
			"emu.pause.toggle\n"
			"emu.idle             <ms>\n"
			"emu.idle.toggle\n"
			"emu.config.save      <fname>\n"
			"emu.realtime         <0|1>\n"
			"emu.realtime.toggle\n"
			"emu.cpu.model        <8086|8088|v20|v30|80186|80188>\n"
			"emu.video.redraw\n"
			"emu.video.screenshot [fname]\n"
			"emu.video.size       <w> <h>\n"
			"emu.disk.boot        <bootdrive>\n"
			"emu.disk.commit      [drive]\n"
			"emu.disk.eject       <drive>\n"
			"emu.disk.insert      <drive>:<fname>\n"
		);

		return;
	}

	pce_puts (
		"boot [drive]              Set the boot drive\n"
		"bc [index]                clear a breakpoint or all\n"
		"bl                        list breakpoints\n"
		"bss addr [pass [reset]]   set an address breakpoint [pass=1 reset=0]\n"
		"bsx expr [pass [reset]]   set an expression breakpoint [pass=1 reset=0]\n"
		"c [cnt]                   clock [1]\n"
		"d [addr [cnt]]            dump memory\n"
		"e addr [val|string...]    enter bytes into memory\n"
		"dump what fname           dump to file (ram|video)\n"
		"gb [addr...]              run with breakpoints\n"
		"g far                     run until CS changes\n"
		"g                         run\n"
		"int28 [on|off|val]        turn int28 sleeping on/off\n"
		"i [b|w] port              input a byte or word from a port\n"
		"m[g|s] [msg [val]]        send a message\n"
		"o [b|w] port val          output a byte or word to a port\n"
		"par i fname               set parport output file\n"
		"pq [c|f|s]                prefetch queue clear/fill/status\n"
		"p [cnt]                   execute cnt instructions, without trace in calls [1]\n"
		"q                         quit\n"
		"r [reg val]               set a register\n"
		"s [what]                  print status (pc|cpu|mem|pit|ppi|pic|uart|video|xms)\n"
		"t [cnt]                   execute cnt instructions [1]\n"
		"u [addr [cnt]]            disassemble\n"
		"v [expr...]               evaluate expressions\n"
		"w name addr cnt           save memory to file\n"
	);
}

static
void do_int28 (cmd_t *cmd)
{
	int            set;
	unsigned short val;

	if (cmd_match (cmd, "on")) {
		set = 1;
		val = 10;
	}
	else if (cmd_match (cmd, "off")) {
		set = 1;
		val = 0;
	}
	else if (cmd_match_uint16 (cmd, &val)) {
		set = 1;
	}
	else {
		set = 0;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (set) {
		par_int28 = 1000UL * val;
	}

	pce_printf ("int 28h sleeping is %s (%lums)\n",
		(par_int28 > 0) ? "on" : "off",
		par_int28 / 1000UL
	);
}

static
void do_i (cmd_t *cmd)
{
	int            word;
	unsigned short port;

	if (cmd_match (cmd, "w")) {
		word = 1;
	}
	else if (cmd_match (cmd, "b")) {
		word = 0;
	}
	else {
		word = 0;
	}

	if (!cmd_match_uint16 (cmd, &port)) {
		cmd_error (cmd, "need a port address");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (word) {
		pce_printf ("%04X: %04X\n", port, e86_get_prt16 (pc->cpu, port));
	}
	else {
		pce_printf ("%04X: %02X\n", port, e86_get_prt8 (pc->cpu, port));
	}
}

static
void do_key (cmd_t *cmd)
{
	unsigned short c;

	while (1) {
		if (cmd_match (cmd, "cad")) {
			pc_set_keycode (pc, 0x38);
			pc_set_keycode (pc, 0x1d);
			pc_set_keycode (pc, 0x53);
			pc_set_keycode (pc, 0xd3);
			pc_set_keycode (pc, 0x9d);
			pc_set_keycode (pc, 0xb8);
		}
		else if (cmd_match_uint16 (cmd, &c)) {
			pc_set_keycode (pc, c);
		}
		else {
			break;
		}
	}

	if (!cmd_match_end (cmd)) {
		return;
	}
}

static
void do_o (cmd_t *cmd)
{
	int            word;
	unsigned short port, val;

	if (cmd_match (cmd, "w")) {
		word = 1;
	}
	else if (cmd_match (cmd, "b")) {
		word = 0;
	}
	else {
		word = 0;
	}

	if (!cmd_match_uint16 (cmd, &port)) {
		cmd_error (cmd, "need a port address");
		return;
	}

	if (!cmd_match_uint16 (cmd, &val)) {
		cmd_error (cmd, "need a value");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (word) {
		e86_set_prt16 (pc->cpu, port, val);
	}
	else {
		e86_set_prt8 (pc->cpu, port, val);
	}
}

static
void do_par (cmd_t *cmd)
{
	unsigned short port;
	char           fname[256];

	if (!cmd_match_uint16 (cmd, &port)) {
		cmd_error (cmd, "need a port number");
		return;
	}

	if (!cmd_match_str (cmd, fname, 256)) {
		cmd_error (cmd, "need a file name");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	if ((port >= 4) || (pc->parport[port] == NULL)) {
		pce_printf ("no parallel port %u\n", (unsigned) port);
		return;
	}

	if (parport_set_fname (pc->parport[port], fname)) {
		pce_printf ("setting new file failed\n");
		return;
	}
}

static
void do_pqc (cmd_t *cmd)
{
	if (!cmd_match_end (cmd)) {
		return;
	}

	e86_pq_init (pc->cpu);
}

static
void do_pqs (cmd_t *cmd)
{
	unsigned i;

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_puts ("PQ:");

	for (i = 0; i < pc->cpu->pq_cnt; i++) {
		pce_printf (" %02X", pc->cpu->pq[i]);
	}

	pce_puts ("\n");
}

static
void do_pqf (cmd_t *cmd)
{
	if (!cmd_match_end (cmd)) {
		return;
	}

	e86_pq_fill (pc->cpu);
}

static
void do_pq (cmd_t *cmd)
{
	if (cmd_match (cmd, "c")) {
		do_pqc (cmd);
	}
	else if (cmd_match (cmd, "f")) {
		do_pqf (cmd);
	}
	else if (cmd_match (cmd, "s")) {
		do_pqs (cmd);
	}
	else if (cmd_match_eol (cmd)) {
		do_pqs (cmd);
	}
	else {
		cmd_error (cmd, "pq: unknown command (%s)\n");
	}
}

static
void do_p (cmd_t *cmd)
{
	unsigned short seg, ofs;
	unsigned long  i, n;
	int            brk;
	e86_disasm_t   op;

	n = 1;

	cmd_match_uint32 (cmd, &n);

	if (!cmd_match_end (cmd)) {
		return;
	}

	brk = 0;

	pce_start (&pc->brk);

	for (i = 0; i < n; i++) {
		e86_disasm_cur (pc->cpu, &op);

		seg = e86_get_cs (pc->cpu);
		ofs = e86_get_ip (pc->cpu);

		while ((e86_get_cs (pc->cpu) == seg) && (e86_get_ip (pc->cpu) == ofs)) {
			pc_clock (pc);

			if (pc_check_break (pc)) {
				brk = 1;
				break;
			}
		}

		if (brk) {
			break;
		}

		if (op.flags & (E86_DFLAGS_CALL | E86_DFLAGS_LOOP)) {
			unsigned short ofs2 = ofs + op.dat_n;

			while ((e86_get_cs (pc->cpu) != seg) || (e86_get_ip (pc->cpu) != ofs2)) {
				pc_clock (pc);

				if (pc_check_break (pc)) {
					brk = 1;
					break;
				}
			}
		}

		if (brk) {
			break;
		}

		if (pc_check_break (pc)) {
			break;
		}
	}

	pce_stop();

	prt_state (pc);
}

static
void do_r (cmd_t *cmd, ibmpc_t *pc)
{
	unsigned long val;
	char          sym[256];

	if (cmd_match_eol (cmd)) {
		prt_state_cpu (pc->cpu);
		return;
	}

	if (!cmd_match_ident (cmd, sym, 256)) {
		pce_printf ("missing register\n");
		return;
	}

	if (e86_get_reg (pc->cpu, sym, &val)) {
		pce_printf ("bad register (%s)\n", sym);
		return;
	}

	if (cmd_match_eol (cmd)) {
		pce_printf ("%04lX\n", val);
		return;
	}

	if (!cmd_match_uint32 (cmd, &val)) {
		pce_printf ("missing value\n");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	e86_set_reg (pc->cpu, sym, val);

	prt_state (pc);
}

static
void do_s (cmd_t *cmd)
{
	if (cmd_match_eol (cmd)) {
		prt_state (pc);
		return;
	}

	while (!cmd_match_eol (cmd)) {
		if (cmd_match (cmd, "pc")) {
			prt_state_pc (pc);
		}
		else if (cmd_match (cmd, "cpu")) {
			prt_state_cpu (pc->cpu);
		}
		else if (cmd_match (cmd, "pit")) {
			prt_state_pit (&pc->pit);
		}
		else if (cmd_match (cmd, "ppi")) {
			prt_state_ppi (&pc->ppi);
		}
		else if (cmd_match (cmd, "pic")) {
			prt_state_pic (&pc->pic);
		}
		else if (cmd_match (cmd, "dma")) {
			prt_state_dma (&pc->dma);
		}
		else if (cmd_match (cmd, "mem")) {
			prt_state_mem (pc);
		}
		else if (cmd_match (cmd, "uart")) {
			unsigned short i;
			if (!cmd_match_uint16 (cmd, &i)) {
				i = 0;
			}
			if ((i < 4) && (pc->serport[i] != NULL)) {
				prt_state_uart (
					&pc->serport[i]->uart,
					pc->serport[i]->io
				);
			}
		}
		else if (cmd_match (cmd, "video")) {
			prt_state_video (pc->video);
		}
		else if (cmd_match (cmd, "ems")) {
			prt_state_ems (pc->ems);
		}
		else if (cmd_match (cmd, "xms")) {
			prt_state_xms (pc->xms);
		}
		else {
			cmd_error (cmd, "unknown component (%s)\n");
			return;
		}
	}
}

static
void do_screenshot (cmd_t *cmd)
{
	char     fname[256];
	unsigned mode;
	FILE     *fp;

	if (!cmd_match_str (cmd, fname, 256)) {
		cmd_error (cmd, "need a file name");
		return;
	}

	if (cmd_match (cmd, "t")) {
		mode = 1;
	}
	else if (cmd_match (cmd, "g")) {
		mode = 2;
	}
	else {
		if (!cmd_match_eol (cmd)) {
			return;
		}

		mode = 0;
	}

	fp = fopen (fname, "wb");
	if (fp == NULL) {
		pce_printf ("can't open file (%s)\n", fname);
		return;
	}

	if (pce_video_screenshot (pc->video, fp, mode)) {
		fclose (fp);
		pce_printf ("screenshot failed\n");
		return;
	}

	fclose (fp);
}

static
void do_t (cmd_t *cmd)
{
	unsigned long i, n;

	n = 1;

	cmd_match_uint32 (cmd, &n);

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&pc->brk);

	for (i = 0; i < n; i++) {
		pc_exec (pc);

		if (pc_check_break (pc)) {
			break;
		}
	}

	pce_stop();

	prt_state (pc);
}

static
void do_update (cmd_t *cmd)
{
	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_video_update (pc->video);
}

static
void do_u (cmd_t *cmd)
{
	unsigned              i;
	unsigned short        seg, ofs, cnt;
	static unsigned int   first = 1;
	static unsigned short sseg = 0;
	static unsigned short sofs = 0;
	e86_disasm_t          op;
	char                  str[256];

	if (first) {
		first = 0;
		sseg = e86_get_cs (pc->cpu);
		sofs = e86_get_ip (pc->cpu);
	}

	seg = sseg;
	ofs = sofs;
	cnt = 16;

	if (cmd_match_uint16_16 (cmd, &seg, &ofs)) {
		cmd_match_uint16 (cmd, &cnt);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	for (i = 0; i < cnt; i++) {
		e86_disasm_mem (pc->cpu, &op, seg, ofs);
		disasm_str (str, &op);

		pce_printf ("%04X:%04X  %s\n", seg, ofs, str);

		ofs = (ofs + op.dat_n) & 0xffff;
	}

	sseg = seg;
	sofs = ofs;
}

static
void do_w (cmd_t *cmd)
{
	unsigned short seg, ofs;
	unsigned long  cnt;
	char           fname[256];
	unsigned char  v;
	FILE           *fp;

	if (!cmd_match_str (cmd, fname, 256)) {
		cmd_error (cmd, "need a file name");
		return;
	}

	if (!cmd_match_uint16_16 (cmd, &seg, &ofs)) {
		cmd_error (cmd, "address expected");
		return;
	}

	if (!cmd_match_uint32 (cmd, &cnt)) {
		cmd_error (cmd, "byte count expected");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	fp = fopen (fname, "wb");
	if (fp == NULL) {
		pce_printf ("can't open file (%s)\n", fname);
		return;
	}

	while (cnt > 0) {
		v = e86_get_mem8 (pc->cpu, seg, ofs);
		fputc (v, fp);

		ofs += 1;
		seg += ofs >> 4;
		ofs &= 0x000f;

		cnt -= 1;
	}

	fclose (fp);
}

static
int pc_do_cmd (ibmpc_t *pc, cmd_t *cmd)
{
	if (pc->trm != NULL) {
		trm_check (pc->trm);
	}

	if (cmd_match (cmd, "boot")) {
		do_boot (cmd);
	}
	else if (cmd_match (cmd, "b")) {
		cmd_do_b (cmd, &pc->bps, 1);
	}
	else if (cmd_match (cmd, "c")) {
		do_c (cmd);
	}
	else if (cmd_match (cmd, "dump")) {
		do_dump (cmd);
	}
	else if (cmd_match (cmd, "d")) {
		do_d (cmd);
	}
	else if (cmd_match (cmd, "e")) {
		do_e (cmd);
	}
	else if (cmd_match (cmd, "g")) {
		do_g (cmd, pc);
	}
	else if (cmd_match (cmd, "h")) {
		do_h (cmd);
	}
	else if (cmd_match (cmd, "int28")) {
		do_int28 (cmd);
	}
	else if (cmd_match (cmd, "i")) {
		do_i (cmd);
	}
	else if (cmd_match (cmd, "key")) {
		do_key (cmd);
	}
	else if (cmd_match (cmd, "par")) {
		do_par (cmd);
	}
	else if (cmd_match (cmd, "o")) {
		do_o (cmd);
	}
	else if (cmd_match (cmd, "pq")) {
		do_pq (cmd);
	}
	else if (cmd_match (cmd, "p")) {
		do_p (cmd);
	}
	else if (cmd_match (cmd, "r")) {
		do_r (cmd, pc);
	}
	else if (cmd_match (cmd, "screenshot")) {
		do_screenshot (cmd);
	}
	else if (cmd_match (cmd, "s")) {
		do_s (cmd);
	}
	else if (cmd_match (cmd, "t")) {
		do_t (cmd);
	}
	else if (cmd_match (cmd, "update")) {
		do_update (cmd);
	}
	else if (cmd_match (cmd, "u")) {
		do_u (cmd);
	}
	else if (cmd_match (cmd, "w")) {
		do_w (cmd);
	}
	else {
		return (1);
	}

	return (0);
}

static
int pce_load_default_config (const char *dir)
{
	unsigned i, j;
	char     str[1024];

	if (dir == NULL) {
		dir = "";
	}

	if (strlen (dir) > (1024 - 16)) {
		return (1);
	}

	i = 0;
	j = 0;
	while (dir[i] != 0) {
		str[i] = dir[i];
		if (str[i] == '/') {
			j = i + 1;
		}
		i += 1;
	}

	if (j > 0) {
		str[j] = 0;
		if (chdir (str) != 0) {
			return (1);
		}
	}

	strcpy (str + j, "ibmpc.cfg");
	par_cfg = pce_load_config (str);
	if (par_cfg != NULL) {
		return (0);
	}

	strcpy (str + j, "pce.cfg");
	par_cfg = pce_load_config (str);
	if (par_cfg != NULL) {
		return (0);
	}

	return (1);
}

static
int pce_default_config (const char *argv0)
{
	if (pce_load_default_config (NULL) == 0) {
		return (0);
	}

	if (pce_load_default_config (argv0) == 0) {
		return (0);
	}

	return (1);
}

int main (int argc, char *argv[])
{
	int       i;
	int       run, nomon;
	char      *cfg;
	ini_sct_t *sct;

	if (argc == 2) {
		if (str_isarg (argv[1], "?", "help")) {
			prt_help();
			return (0);
		}
		else if (str_isarg (argv[1], NULL, "version")) {
			prt_version();
			return (0);
		}
	}

	cfg = NULL;
	run = 0;
	nomon = 0;

	pce_log_init();
	pce_log_add_fp (stderr, 0, MSG_INF);

	i = 1;
	while (i < argc) {
		if (str_isarg (argv[i], "v", "verbose")) {
			pce_log_set_level (stderr, MSG_DEB);
		}
		else if (str_isarg (argv[i], "q", "quiet")) {
			pce_log_set_level (stderr, MSG_ERR);
		}
		else if (str_isarg (argv[i], "c", "config")) {
			i += 1;
			if (i >= argc) {
				return (1);
			}
			cfg = argv[i];
		}
		else if (str_isarg (argv[i], "l", "log")) {
			i += 1;
			if (i >= argc) {
				return (1);
			}
			pce_log_add_fname (argv[i], MSG_DEB);
		}
		else if (str_isarg (argv[i], "t", "terminal")) {
			i += 1;
			if (i >= argc) {
				return (1);
			}

			par_terminal = argv[i];
		}
		else if (str_isarg (argv[i], "g", "video")) {
			i += 1;
			if (i >= argc) {
				return (1);
			}

			par_video = argv[i];
		}
		else if (str_isarg (argv[i], "p", "cpu")) {
			i += 1;
			if (i >= argc) {
				return (1);
			}

			par_cpu = argv[i];
		}
		else if (str_isarg (argv[i], "b", "boot")) {
			i += 1;
			if (i >= argc) {
				return (1);
			}

			par_boot = (unsigned) strtoul (argv[i], NULL, 0);
		}
		else if (str_isarg (argv[i], "r", "run")) {
			run = 1;
		}
		else if (str_isarg (argv[i], "R", "no-monitor")) {
			nomon = 1;
		}
		else {
			printf ("%s: unknown option (%s)\n", argv[0], argv[i]);
			return (1);
		}

		i += 1;
	}

	pce_log (MSG_MSG,
		"pce ibmpc version " PCE_VERSION_STR
		" (compiled " PCE_CFG_DATE " " PCE_CFG_TIME ")\n"
		"Copyright (C) 1995-2007 Hampa Hug <hampa@hampa.ch>\n"
	);

	if (argc < 2) {
		/* no arguments, use defaults */
		pce_log_set_level (stderr, MSG_ERR);
		par_terminal = "sdl";
		par_video = "ega";
		nomon = 1;

		if (pce_default_config (argv[0])) {
			pce_log (MSG_ERR, "loading config file failed\n");
			return (1);
		}
	}
	else {
		par_cfg = pce_load_config (cfg);
		if (par_cfg == NULL) {
			pce_log (MSG_ERR, "loading config file failed (%s)\n", cfg);
			return (1);
		}
	}

	sct = ini_next_sct (par_cfg, NULL, "pc");
	if (sct == NULL) {
		sct = par_cfg;
	}

	pc = pc_new (sct);
	par_pc = pc;

	pc->cpu->op_int = &pce_op_int;
	pc->cpu->op_undef = &pce_op_undef;
#if 0
	pc->cpu->op_stat = pce_op_stat;
#endif

	pc_set_bootdrive (pc, par_boot);

	e86_reset (pc->cpu);

	signal (SIGINT, &sig_int);
	signal (SIGSEGV, &sig_segv);

	pce_console_init (stdin, stdout);
	cmd_init (pc, cmd_get_sym, cmd_set_sym);

	mon_init (&par_mon);
	mon_set_cmd_fct (&par_mon, pc_do_cmd, par_pc);
	mon_set_msg_fct (&par_mon, pc_set_msg, pc_get_msg, par_pc);

	if (nomon) {
		while (pc->brk != PCE_BRK_ABORT) {
			pc_run (pc);
		}
	}
	else if (run) {
		pc_run (pc);
		if (pc->brk != PCE_BRK_ABORT) {
			pce_puts ("\n");
		}
	}
	else {
		pce_puts ("type 'h' for help\n");
	}

	if (pc->brk != PCE_BRK_ABORT) {
		mon_run (&par_mon);
	}

	pc_del (pc);

	mon_free (&par_mon);
	pce_console_done();
	pce_log_done();

	return (0);
}
