/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/cmd.c                                         *
 * Created:     2010-09-21 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2011 Hampa Hug <hampa@hampa.ch>                     *
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
#include "ibmpc.h"

#include <stdio.h>
#include <string.h>

#include <lib/brkpt.h>
#include <lib/cmd.h>
#include <lib/console.h>
#include <lib/log.h>
#include <lib/sysdep.h>


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
		dst_i += sprintf (dst + dst_i, "%s", op->arg1);
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

	pce_video_print_info (vid, pce_get_fp_out());

	fp = pce_get_redir_out();
	if (fp != NULL) {
		pce_video_print_info (vid, fp);
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
	char          p;
	unsigned char lsr, msr;

	lsr = e8250_get_lsr (uart);
	msr = e8250_get_msr (uart);

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
		"IO=%04X  %lu %u%c%u  DTR=%d  RTS=%d   DSR=%d  CTS=%d  DCD=%d  RI=%d\n"
		"TxD=%02X%c RxD=%02X%c SCR=%02X  DIV=%04X\n"
		"IER=%02X  IIR=%02X  LCR=%02X  LSR=%02X  MCR=%02X  MSR=%02X\n",
		base,
		e8250_get_bps (uart), e8250_get_databits (uart), p,
		e8250_get_stopbits (uart),
		e8250_get_dtr (uart), e8250_get_rts (uart),
		(msr & E8250_MSR_DSR) != 0,
		(msr & E8250_MSR_CTS) != 0,
		(msr & E8250_MSR_DCD) != 0,
		(msr & E8250_MSR_RI) != 0,
		uart->txd, (lsr & E8250_LSR_TBE) ? ' ' : '*',
		uart->rxd, (lsr & E8250_LSR_RRD) ? '*' : ' ',
		uart->scratch, uart->divisor,
		uart->ier, uart->iir, uart->lcr, lsr, uart->mcr, msr
	);
}

static
void prt_state_time (e8086_t *c)
{
	double cpi;

	pce_prt_sep ("TIME");

	if (c->instructions > 0) {
		cpi = (double) c->clocks / (double) c->instructions;
	}
	else {
		cpi = 0.0;
	}

	pce_printf ("CLK=%llu + %lu\n", c->clocks, c->delay);
	pce_printf ("OPS=%llu\n", c->instructions);
	pce_printf ("CPI=%.4f\n", cpi);
}

void prt_state_cpu (e8086_t *c)
{
	static char ft[2] = { '-', '+' };

	pce_prt_sep ("8086");

	pce_printf (
		"AX=%04X  BX=%04X  CX=%04X  DX=%04X  "
		"SP=%04X  BP=%04X  SI=%04X  DI=%04X INT=%02X%c\n",
		e86_get_ax (c), e86_get_bx (c), e86_get_cx (c), e86_get_dx (c),
		e86_get_sp (c), e86_get_bp (c), e86_get_si (c), e86_get_di (c),
		par_pc->current_int & 0xff,
		(par_pc->current_int & 0x100) ? '*' : ' '
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

	if (c->halt) {
		pce_printf ("HALT=1\n");
	}
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
	prt_state_time (pc->cpu);
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

	pc->current_int &= 0xff;

	old = e86_get_opcnt (pc->cpu);

	while (e86_get_opcnt (pc->cpu) == old) {
		pc_clock (pc, 1);

		if (pc->brk) {
			break;
		}
	}
}

void pc_run (ibmpc_t *pc)
{
	pce_start (&pc->brk);

	pc_clock_discontinuity (pc);

	while (pc->brk == 0) {
		if (pc->pause == 0) {
			pc_clock (pc, 4 * pc->speed_current);
		}
		else {
			pce_usleep (100000);
			trm_check (pc->trm);
		}
	}

	pc->current_int &= 0xff;

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
	unsigned seg;
	ibmpc_t  *pc;

	pc = ext;

	pc->current_int = n | 0x100;

	if (pc_intlog_check (pc, n)) {
		pce_log (MSG_DEB, "%04X:%04X: int %02X"
			" [AX=%04X BX=%04X CX=%04X DX=%04X]\n",
			e86_get_cs (pc->cpu), e86_get_ip (pc->cpu),
			n,
			e86_get_ax (pc->cpu), e86_get_bx (pc->cpu),
			e86_get_cx (pc->cpu), e86_get_dx (pc->cpu)
		);
	}

	if (n == 0x19) {
		if (pc->patch_bios_int19 == 0) {
			return;
		}

		if (pc->patch_bios_init) {
			return;
		}

		if (e86_get_mem16 (pc->cpu, 0, 4 * 0x19 + 2) < 0xf000) {
			return;
		}

		seg = pc_get_pcex_seg (pc);

		if (seg == 0) {
			return;
		}

		pc_log_deb ("patching int 19 (0x%04x)\n", seg);

		e86_set_mem16 (pc->cpu, 0, 4 * 0x19 + 0, 0x0010);
		e86_set_mem16 (pc->cpu, 0, 4 * 0x19 + 2, seg);
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
void pc_cmd_boot (cmd_t *cmd, ibmpc_t *pc)
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
void pc_cmd_c (cmd_t *cmd, ibmpc_t *pc)
{
	unsigned long cnt;

	cnt = 1;

	cmd_match_uint32 (cmd, &cnt);

	if (!cmd_match_end (cmd)) {
		return;
	}

	pc_clock_discontinuity (pc);

	while (cnt > 0) {
		pc_clock (pc, 1);
		cnt -= 1;
	}

	prt_state (pc);
}

static
void pc_cmd_d (cmd_t *cmd, ibmpc_t *pc)
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
void pc_cmd_e (cmd_t *cmd, ibmpc_t *pc)
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
void pc_cmd_g_b (cmd_t *cmd, ibmpc_t *pc)
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

	pc_clock_discontinuity (pc);

	while (1) {
		pc_exec (pc);

		if (pc_check_break (pc)) {
			break;
		}
	}

	pce_stop();
}

static
void pc_cmd_g_far (cmd_t *cmd, ibmpc_t *pc)
{
	unsigned short seg;

	if (!cmd_match_end (cmd)) {
		return;
	}

	seg = e86_get_cs (pc->cpu);

	pce_start (&pc->brk);

	pc_clock_discontinuity (pc);

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
void pc_cmd_g (cmd_t *cmd, ibmpc_t *pc)
{
	if (cmd_match (cmd, "b")) {
		pc_cmd_g_b (cmd, pc);
	}
	else if (cmd_match (cmd, "far")) {
		pc_cmd_g_far (cmd, pc);
	}
	else {
		if (!cmd_match_end (cmd)) {
			return;
		}

		pc_run (pc);
	}
}

static
void pc_cmd_hm (cmd_t *cmd)
{
		pce_puts (
			"emu.config.save      <filename>\n"
			"emu.exit\n"
			"emu.stop\n"
			"emu.pause            \"0\" | \"1\"\n"
			"emu.pause.toggle\n"
			"emu.reset\n"
			"\n"
			"emu.cpu.model        \"8086\" | \"8088\" | \"80186\" | \"80188\"\n"
			"emu.cpu.speed        <factor>\n"
			"emu.cpu.speed.step   <adjustment>\n"
			"\n"
			"emu.disk.boot        <bootdrive>\n"
			"emu.disk.commit      [<drive>]\n"
			"emu.disk.eject       <drive>\n"
			"emu.disk.insert      <drive>:<fname>\n"
			"\n"
			"emu.parport.driver   <driver>\n"
			"emu.parport.file     <filename>\n"
			"\n"
			"emu.serport.driver   <driver>\n"
			"emu.serport.file     <filename>\n"
			"\n"
			"emu.tape.append\n"
			"emu.tape.file        <filename>\n"
			"emu.tape.load        [<position> | \"end\"]\n"
			"emu.tape.rewind\n"
			"emu.tape.save        [<position> | \"end\"]\n"
			"emu.tape.state\n"
			"\n"
			"emu.term.fullscreen  \"0\" | \"1\"\n"
			"emu.term.fullscreen.toggle\n"
			"emu.term.grab\n"
			"emu.term.release\n"
			"emu.term.screenshot  [<filename>]\n"
			"emu.term.title       <title>\n"
			"\n"
			"emu.video.blink      <blink-rate>\n"
			"emu.video.redraw     [\"now\"]\n"
		);
}

static
void pc_cmd_h (cmd_t *cmd)
{
	if (cmd_match (cmd, "m")) {
		pc_cmd_hm (cmd);
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
		"gb [addr...]              run with breakpoints\n"
		"g far                     run until CS changes\n"
		"g                         run\n"
		"h                         print help summary\n"
		"hm                        print help on messages\n"
		"i [b|w] port              input a byte or word from a port\n"
		"log int l                 list interrupt log expressions\n"
		"log int n [expr]          set interrupt n log expression to expr\n"
		"m msg [val]               send a message\n"
		"o [b|w] port val          output a byte or word to a port\n"
		"pq [c|f|s]                prefetch queue clear/fill/status\n"
		"p [cnt]                   execute cnt instructions, without trace in calls [1]\n"
		"q                         quit\n"
		"r [reg val]               set a register\n"
		"s [what]                  print status (pc|cpu|mem|pit|ppi|pic|time|uart|video|xms)\n"
		"t [cnt]                   execute cnt instructions [1]\n"
		"u [addr [cnt]]            disassemble\n"
		"v [expr...]               evaluate expressions\n"
		"w name addr cnt           save memory to file\n"
	);
}

static
void pc_cmd_i (cmd_t *cmd, ibmpc_t *pc)
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
void pc_cmd_key (cmd_t *cmd, ibmpc_t *pc)
{
	unsigned short c;

	while (1) {
		if (cmd_match (cmd, "cad")) {
			pc_kbd_set_keycode (&pc->kbd, 0x38);
			pc_kbd_set_keycode (&pc->kbd, 0x1d);
			pc_kbd_set_keycode (&pc->kbd, 0x53);
			pc_kbd_set_keycode (&pc->kbd, 0xd3);
			pc_kbd_set_keycode (&pc->kbd, 0x9d);
			pc_kbd_set_keycode (&pc->kbd, 0xb8);
		}
		else if (cmd_match_uint16 (cmd, &c)) {
			pc_kbd_set_keycode (&pc->kbd, c);
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
void pc_cmd_log_int_l (cmd_t *cmd, ibmpc_t *pc)
{
	unsigned   i;
	const char *str;

	for (i = 0; i < 256; i++) {
		str = pc_intlog_get (pc, i);

		if (str != NULL) {
			pce_printf ("%02X: %s\n", i, str);
		}
	}
}

static
void pc_cmd_log_int (cmd_t *cmd, ibmpc_t *pc)
{
	unsigned short n;
	char           buf[256];

	if (cmd_match_eol (cmd)) {
		pc_cmd_log_int_l (cmd, pc);
		return;
	}

	if (cmd_match (cmd, "l")) {
		pc_cmd_log_int_l (cmd, pc);
		return;
	}

	if (!cmd_match_uint16 (cmd, &n)) {
		cmd_error (cmd, "need an interrupt number");
		return;
	}

	if (cmd_match_eol (cmd)) {
		pc_intlog_set (pc, n, NULL);
		pce_printf ("%02X: <deleted>\n", n);
		return;
	}

	if (!cmd_match_str (cmd, buf, 256)) {
		cmd_error (cmd, "need an expression");
		return;
	}

	pce_printf ("%02X: %s\n", n, buf);

	pc_intlog_set (pc, n, buf);
}

static
void pc_cmd_log (cmd_t *cmd, ibmpc_t *pc)
{
	if (cmd_match (cmd, "int")) {
		pc_cmd_log_int (cmd, pc);
	}
	else {
		cmd_error (cmd, "log what?");
	}
}

static
void pc_cmd_o (cmd_t *cmd, ibmpc_t *pc)
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
void pc_cmd_pqc (cmd_t *cmd, ibmpc_t *pc)
{
	if (!cmd_match_end (cmd)) {
		return;
	}

	e86_pq_init (pc->cpu);
}

static
void pc_cmd_pqs (cmd_t *cmd, ibmpc_t *pc)
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
void pc_cmd_pqf (cmd_t *cmd, ibmpc_t *pc)
{
	if (!cmd_match_end (cmd)) {
		return;
	}

	e86_pq_fill (pc->cpu);
}

static
void pc_cmd_pq (cmd_t *cmd, ibmpc_t *pc)
{
	if (cmd_match (cmd, "c")) {
		pc_cmd_pqc (cmd, pc);
	}
	else if (cmd_match (cmd, "f")) {
		pc_cmd_pqf (cmd, pc);
	}
	else if (cmd_match (cmd, "s")) {
		pc_cmd_pqs (cmd, pc);
	}
	else if (cmd_match_eol (cmd)) {
		pc_cmd_pqs (cmd, pc);
	}
	else {
		cmd_error (cmd, "pq: unknown command (%s)\n");
	}
}

static
void pc_cmd_p (cmd_t *cmd, ibmpc_t *pc)
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

	pc->current_int &= 0xff;

	pc_clock_discontinuity (pc);

	for (i = 0; i < n; i++) {
		e86_disasm_cur (pc->cpu, &op);

		seg = e86_get_cs (pc->cpu);
		ofs = e86_get_ip (pc->cpu);

		while ((e86_get_cs (pc->cpu) == seg) && (e86_get_ip (pc->cpu) == ofs)) {
			pc_clock (pc, 1);

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
				pc_clock (pc, 1);

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
void pc_cmd_r (cmd_t *cmd, ibmpc_t *pc)
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
void pc_cmd_s (cmd_t *cmd, ibmpc_t *pc)
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
		else if (cmd_match (cmd, "time")) {
			prt_state_time (pc->cpu);
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
void pc_cmd_t (cmd_t *cmd, ibmpc_t *pc)
{
	unsigned long i, n;

	n = 1;

	cmd_match_uint32 (cmd, &n);

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&pc->brk);

	pc_clock_discontinuity (pc);

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
void pc_cmd_u (cmd_t *cmd, ibmpc_t *pc)
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
void pc_cmd_w (cmd_t *cmd, ibmpc_t *pc)
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

int pc_cmd (ibmpc_t *pc, cmd_t *cmd)
{
	if (pc->trm != NULL) {
		trm_check (pc->trm);
	}

	if (cmd_match (cmd, "boot")) {
		pc_cmd_boot (cmd, pc);
	}
	else if (cmd_match (cmd, "b")) {
		cmd_do_b (cmd, &pc->bps, 1);
	}
	else if (cmd_match (cmd, "c")) {
		pc_cmd_c (cmd, pc);
	}
	else if (cmd_match (cmd, "d")) {
		pc_cmd_d (cmd, pc);
	}
	else if (cmd_match (cmd, "e")) {
		pc_cmd_e (cmd, pc);
	}
	else if (cmd_match (cmd, "g")) {
		pc_cmd_g (cmd, pc);
	}
	else if (cmd_match (cmd, "h")) {
		pc_cmd_h (cmd);
	}
	else if (cmd_match (cmd, "i")) {
		pc_cmd_i (cmd, pc);
	}
	else if (cmd_match (cmd, "key")) {
		pc_cmd_key (cmd, pc);
	}
	else if (cmd_match (cmd, "log")) {
		pc_cmd_log (cmd, pc);
	}
	else if (cmd_match (cmd, "o")) {
		pc_cmd_o (cmd, pc);
	}
	else if (cmd_match (cmd, "pq")) {
		pc_cmd_pq (cmd, pc);
	}
	else if (cmd_match (cmd, "p")) {
		pc_cmd_p (cmd, pc);
	}
	else if (cmd_match (cmd, "r")) {
		pc_cmd_r (cmd, pc);
	}
	else if (cmd_match (cmd, "s")) {
		pc_cmd_s (cmd, pc);
	}
	else if (cmd_match (cmd, "t")) {
		pc_cmd_t (cmd, pc);
	}
	else if (cmd_match (cmd, "u")) {
		pc_cmd_u (cmd, pc);
	}
	else if (cmd_match (cmd, "w")) {
		pc_cmd_w (cmd, pc);
	}
	else {
		return (1);
	}

	return (0);
}

void pc_cmd_init (ibmpc_t *pc)
{
	pc->cpu->op_int = &pce_op_int;
	pc->cpu->op_undef = &pce_op_undef;
#if 0
	pc->cpu->op_stat = pce_op_stat;
#endif

}
