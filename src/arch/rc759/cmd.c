/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/cmd.c                                         *
 * Created:     2012-06-29 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2013 Hampa Hug <hampa@hampa.ch>                     *
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
#include "rc759.h"

#include <stdio.h>
#include <string.h>

#include <lib/brkpt.h>
#include <lib/cmd.h>
#include <lib/console.h>
#include <lib/log.h>
#include <lib/monitor.h>
#include <lib/sysdep.h>


static mon_cmd_t par_cmd[] = {
	{ "c", "[cnt]", "clock the simulation [1]" },
	{ "gb", "[addr...]", "run with breakpoints" },
	{ "g", "far", "run until CS changes" },
	{ "g", "", "run" },
	{ "hm", "", "print help on messages" },
	{ "i", "[b|w] port", "input a byte or word from a port" },
	{ "key", "[[+|-]key...]", "simulate pressing or releasing keys" },
	{ "log", "int l", "list interrupt log expressions" },
	{ "log", "int n [expr]", "set interrupt n log expression to expr" },
	{ "o", "[b|w] port val", "output a byte or word to a port" },
	{ "pq", "[c|f|s]", "prefetch queue clear/fill/status" },
	{ "p", "[cnt]", "execute cnt instructions, without trace in calls [1]" },
	{ "r", "[reg val]", "set a register" },
	{ "s", "[what]", "print status (cpu|icu|mem||ppi|pic|rc759|tcu|time)" },
	{ "t", "[cnt]", "execute cnt instructions [1]" },
	{ "u", "[addr [cnt [mode]]]", "disassemble" }
};


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
void print_state_icu (e80186_icu_t *icu)
{
	unsigned i;
	unsigned char inp, irr, imr, isr;

	const char *names[] = {
		"TMR", NULL, "DMA0", "DMA1", "INT0", "INT1", "INT2", "INT3"
	};

	inp = icu->inp;
	irr = e80186_icu_get_irr (icu);
	imr = e80186_icu_get_imr (icu);
	isr = e80186_icu_get_isr (icu);

	pce_prt_sep ("80186-ICU");

	pce_puts ("INP=");
	prt_uint8_bin (inp);
	pce_puts (" IRR=");
	prt_uint8_bin (irr);
	pce_puts (" IMR=");
	prt_uint8_bin (imr);
	pce_puts (" ISR=");
	prt_uint8_bin (isr);
	pce_puts ("\n");

	pce_printf ("PMR=%04X  PST=%04X  INT=%d\n",
		e80186_icu_get_pmr (icu),
		e80186_icu_get_pollst (icu),
		icu->intr_val
	);

	for (i = 0; i < 8; i++) {
		pce_printf ("%4s[%u]: CTL=%04X  CNT=%04lX  I=%u R=%u M=%u S=%u\n",
			(names[i] != NULL) ? names[i] : "-", i,
			e80186_icu_get_icon (icu, i),
			icu->int_cnt[i],
			inp & 1, irr & 1, imr & 1, isr & 1
		);

		inp >>= 1;
		irr >>= 1;
		imr >>= 1;
		isr >>= 1;
	}
}

static
void print_state_tcu (e80186_tcu_t *tcu)
{
	unsigned       i;
	unsigned short ctl;

	pce_prt_sep ("80186-TCU");

	for (i = 0; i < 3; i++) {
		ctl = e80186_tcu_get_control (tcu, i);

		pce_printf (
			"%cC%d: CTL=%04X CNT=%04X MAXA=%04X MAXB=%04X\n",
			(ctl & 0x8000) ? '*' : ' ',
			i,
			ctl,
			e80186_tcu_get_count (tcu, i),
			e80186_tcu_get_max_count_a (tcu, i),
			e80186_tcu_get_max_count_b (tcu, i)
		);
	}
}

static
void print_state_ppi (e8255_t *ppi)
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
void print_state_dma (e80186_dma_t *dma)
{
	unsigned i;

	pce_prt_sep ("80186-DMA");

	for (i = 0; i < 2; i++) {
		pce_printf ("%u: CTL=%04X CNT=%04X SRC=%05lX DST=%05lX\n",
			i,
			e80186_dma_get_control (dma, i),
			e80186_dma_get_count (dma, i),
			e80186_dma_get_src (dma, i),
			e80186_dma_get_dst (dma, i)
		);
	}
}

static
void print_state_pic (e8259_t *pic)
{
	unsigned i;

	pce_prt_sep ("8259A-PIC");

	pce_puts ("INP=");
	prt_uint8_bin (pic->irq_inp);
	pce_puts ("\n");

	pce_puts ("IRR=");
	prt_uint8_bin (e8259_get_irr (pic));
	pce_printf ("  PRIO=%u\n", pic->priority);

	pce_puts ("IMR=");
	prt_uint8_bin (e8259_get_imr (pic));
	pce_printf ("  INTR=%d\n", pic->intr_val != 0);

	pce_puts ("ISR=");
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
void print_state_fdc (rc759_fdc_t *fdc)
{
	unsigned       i;
	wd179x_drive_t *drv;

	pce_prt_sep ("179X-FDC");

	pce_printf ("FCR=%02X  RSV=%02X\n",
		rc759_fdc_get_fcr (fdc), rc759_fdc_get_reserve (fdc)
	);

	pce_printf ("STATUS=%02X  CMD=%02X\n",
		fdc->wd179x.status, fdc->wd179x.cmd
	);

	for (i = 0; i < 2; i++) {
		drv = &fdc->wd179x.drive[i];

		pce_printf ("DRIVE %u: SEL=%d RDY=%d M=%d  C=%02X H=%X  POS=%lu CNT=%lu\n",
			i,
			fdc->wd179x.drv->d == i, drv->ready != 0, drv->motor != 0,
			(unsigned) drv->c, (unsigned) drv->h,
			(unsigned long) drv->trkbuf_idx, (unsigned long) drv->trkbuf_cnt
		);
	}
}

static
void print_state_video (e82730_t *crt)
{
	unsigned i;

	pce_prt_sep ("82730-CRTC");

	pce_printf ("CBP:   %08lX\n", crt->cbp);
	pce_printf ("GRAPH: %-4d  MONO: %d  SINT: %-4d\n", crt->graphic != 0, crt->monochrome != 0, crt->sint_val != 0);
	pce_printf ("STAT:  %04X  IMSK: %04X\n", crt->status, crt->intmask);
	pce_printf ("ALF:   %-4d  LSSW: %-4d\n", crt->auto_lf != 0, crt->list_switch != 0);
	pce_printf ("FATTR: %04X\n", crt->field_attrib);


	pce_printf ("Mode:\n");
	pce_printf ("\tline length:     %u\n", crt->mode_line_length);
	pce_printf ("\th field start:   %u\n", crt->mode_hfldstrt);
	pce_printf ("\th field stop:    %u\n", crt->mode_hfldstp);
	pce_printf ("\tframe length:    %u\n", crt->mode_frame_length);
	pce_printf ("\tv field start:   %u\n", crt->mode_vfldstrt);
	pce_printf ("\tv field stop:    %u\n", crt->mode_vfldstp);
	pce_printf ("\tlines per row:   %u\n", crt->mode_lines_per_row);
	pce_printf ("\tframe int count: %u\n", crt->frame_int_count[0]);

	for (i = 0; i < 2; i++) {
		pce_printf ("CURSOR %u: X=%02X Y=%02X OE=%d RVV=%d BE=%d FREQ=%u DUTY=%u START=%02X STOP=%02X\n",
			i + 1,
			crt->cursor_x[i],
			crt->cursor_y[i],
			crt->cursor_enable[i],
			crt->cursor_reverse[i],
			crt->cursor_blink[i],
			crt->cursor_max,
			crt->cursor_duty,
			crt->cursor_start[i],
			crt->cursor_stop[i]
		);
	}
}

static
void print_state_time (e8086_t *c)
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

void print_state_cpu (e8086_t *c)
{
	static char ft[2] = { '-', '+' };

	pce_prt_sep ("80186");

	pce_printf (
		"AX=%04X  BX=%04X  CX=%04X  DX=%04X  "
		"SP=%04X  BP=%04X  SI=%04X  DI=%04X INT=%02X%c\n",
		e86_get_ax (c), e86_get_bx (c), e86_get_cx (c), e86_get_dx (c),
		e86_get_sp (c), e86_get_bp (c), e86_get_si (c), e86_get_di (c),
		par_sim->current_int & 0xff,
		(par_sim->current_int & 0x100) ? '*' : ' '
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
void print_state_mem (rc759_t *sim)
{
	pce_prt_sep ("RC759 MEM");
	mem_prt_state (sim->mem, stdout);
}

static
void print_state_ports (rc759_t *sim)
{
	pce_prt_sep ("RC759 PORTS");
	mem_prt_state (sim->iop, stdout);
}

static
void print_state_rc759 (rc759_t *sim)
{
	print_state_ppi (&sim->ppi);
	print_state_tcu (&sim->tcu);
	print_state_pic (&sim->pic);
	print_state_dma (&sim->dma);
	print_state_time (sim->cpu);
	print_state_cpu (sim->cpu);
}

static
void print_state (rc759_t *sim)
{
	e86_disasm_t op;
	char         str[256];

	e86_disasm_cur (sim->cpu, &op);
	disasm_str (str, &op);

	print_state_cpu (sim->cpu);

	pce_printf ("%04X:%04X  %s\n",
		(unsigned) e86_get_cs (sim->cpu),
		(unsigned) e86_get_ip (sim->cpu),
		str
	);
}

static
int rc759_check_break (rc759_t *sim)
{
	unsigned short seg, ofs;

	seg = e86_get_cs (sim->cpu);
	ofs = e86_get_ip (sim->cpu);

	if (bps_check (&sim->bps, seg, ofs, stdout)) {
		return (1);
	}

	if (sim->brk) {
		return (1);
	}

	return (0);
}

static
void rc759_exec (rc759_t *sim)
{
	unsigned long long old;

	sim->current_int &= 0xff;

	old = e86_get_opcnt (sim->cpu);

	while (e86_get_opcnt (sim->cpu) == old) {
		rc759_clock (sim, 1);

		if (sim->brk) {
			break;
		}
	}
}

void rc759_run (rc759_t *sim)
{
	pce_start (&sim->brk);

	rc759_clock_discontinuity (sim);

	while (sim->brk == 0) {
		if (sim->pause == 0) {
			rc759_clock (sim, 8);
		}
		else {
			pce_usleep (100000);
			trm_check (sim->trm);
		}
	}

	sim->current_int &= 0xff;

	pce_stop();
}

static
void pce_op_int (void *ext, unsigned char n)
{
	rc759_t *sim;

	sim = ext;

	sim->current_int = n | 0x100;

	if (rc759_intlog_check (sim, n)) {
		pce_printf ("%04X:%04X: int %02X"
			" [AX=%04X BX=%04X CX=%04X DX=%04X DS=%04X ES=%04X]\n",
			e86_get_cs (sim->cpu), e86_get_cur_ip (sim->cpu),
			n,
			e86_get_ax (sim->cpu), e86_get_bx (sim->cpu),
			e86_get_cx (sim->cpu), e86_get_dx (sim->cpu),
			e86_get_ds (sim->cpu), e86_get_es (sim->cpu)
		);
	}
}

static
void pce_op_undef (void *ext, unsigned char op1, unsigned char op2)
{
	rc759_t *sim;

	sim = ext;

	pce_log (MSG_DEB, "%04X:%04X: undefined operation [%02X %02x]\n",
		e86_get_cs (sim->cpu), e86_get_ip (sim->cpu), op1, op2
	);

	pce_usleep (100000UL);

	trm_check (sim->trm);
}


static
void rc759_cmd_c (cmd_t *cmd, rc759_t *sim)
{
	unsigned long cnt;

	cnt = 1;

	cmd_match_uint32 (cmd, &cnt);

	if (!cmd_match_end (cmd)) {
		return;
	}

	rc759_clock_discontinuity (sim);

	while (cnt > 0) {
		rc759_clock (sim, 1);
		cnt -= 1;
	}

	print_state (sim);
}

static
void rc759_cmd_g_b (cmd_t *cmd, rc759_t *sim)
{
	unsigned long seg, ofs;
	breakpoint_t  *bp;

	while (cmd_match_uint32 (cmd, &seg)) {
		if (cmd_match (cmd, ":")) {
			if (!cmd_match_uint32 (cmd, &ofs)) {
				cmd_error (cmd, "expecting offset");
				return;
			}

			bp = bp_segofs_new (seg, ofs);
		}
		else {
			bp = bp_addr_new (seg);
		}

		bps_bp_add (&sim->bps, bp);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	rc759_clock_discontinuity (sim);

	while (1) {
		rc759_exec (sim);

		if (rc759_check_break (sim)) {
			break;
		}
	}

	pce_stop();
}

static
void rc759_cmd_g_far (cmd_t *cmd, rc759_t *sim)
{
	unsigned short seg;

	if (!cmd_match_end (cmd)) {
		return;
	}

	seg = e86_get_cs (sim->cpu);

	pce_start (&sim->brk);

	rc759_clock_discontinuity (sim);

	while (1) {
		rc759_exec (sim);

		if (e86_get_cs (sim->cpu) != seg) {
			print_state (sim);
			break;
		}

		if (rc759_check_break (sim)) {
			break;
		}
	}

	pce_stop();
}

static
void rc759_cmd_g (cmd_t *cmd, rc759_t *sim)
{
	if (cmd_match (cmd, "b")) {
		rc759_cmd_g_b (cmd, sim);
	}
	else if (cmd_match (cmd, "far")) {
		rc759_cmd_g_far (cmd, sim);
	}
	else {
		if (!cmd_match_end (cmd)) {
			return;
		}

		rc759_run (sim);
	}
}

static
void rc759_cmd_hm (cmd_t *cmd)
{
	pce_puts (
		"emu.config.save      <filename>\n"
		"emu.exit\n"
		"emu.stop\n"
		"emu.pause            \"0\" | \"1\"\n"
		"emu.pause.toggle\n"
		"emu.reset\n"
		"\n"
		"emu.cpu.speed        <factor>\n"
		"emu.cpu.speed.step   <adjustment>\n"
		"\n"
		"emu.disk.commit      [<drive>]\n"
		"emu.disk.eject       <drive>\n"
		"emu.disk.insert      <drive>:<fname>\n"
		"\n"
		"emu.parport1.driver  <driver>\n"
		"emu.parport1.file    <filename>\n"
		"emu.parport2.driver  <driver>\n"
		"emu.parport2.file    <filename>\n"
		"\n"
		"emu.term.fullscreen  \"0\" | \"1\"\n"
		"emu.term.fullscreen.toggle\n"
		"emu.term.grab\n"
		"emu.term.release\n"
		"emu.term.screenshot  [<filename>]\n"
		"emu.term.title       <title>\n"
	);
}

static
void rc759_cmd_i (cmd_t *cmd, rc759_t *sim)
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
		pce_printf ("%04X: %04X\n", port, e86_get_prt16 (sim->cpu, port));
	}
	else {
		pce_printf ("%04X: %02X\n", port, e86_get_prt8 (sim->cpu, port));
	}
}

static
void rc759_cmd_key (cmd_t *cmd, rc759_t *sim)
{
	unsigned       i;
	unsigned       event;
	pce_key_t      key;
	char           str[256];

	while (cmd_match_str (cmd, str, 256)) {
		i = 0;

		event = PCE_KEY_EVENT_DOWN;

		if (str[0] == '+') {
			i += 1;
		}
		else if (str[0] == '-') {
			i += 1;
			event = PCE_KEY_EVENT_UP;
		}

		key = pce_key_from_string (str + i);

		if (key == PCE_KEY_NONE) {
			pce_printf ("unknown key: %s\n", str);
		}
		else {
			pce_printf ("key: %s%s\n",
				(event == PCE_KEY_EVENT_DOWN) ? "+" : "-",
				str + i
			);

			rc759_kbd_set_key (&sim->kbd, event, key);
		}
	}

	if (!cmd_match_end (cmd)) {
		return;
	}
}

static
void rc759_cmd_log_int_l (cmd_t *cmd, rc759_t *sim)
{
	unsigned   i;
	const char *str;

	for (i = 0; i < 256; i++) {
		str = rc759_intlog_get (sim, i);

		if (str != NULL) {
			pce_printf ("%02X: %s\n", i, str);
		}
	}
}

static
void rc759_cmd_log_int (cmd_t *cmd, rc759_t *sim)
{
	unsigned short n;
	char           buf[256];

	if (cmd_match_eol (cmd)) {
		rc759_cmd_log_int_l (cmd, sim);
		return;
	}

	if (cmd_match (cmd, "l")) {
		rc759_cmd_log_int_l (cmd, sim);
		return;
	}

	if (!cmd_match_uint16 (cmd, &n)) {
		cmd_error (cmd, "need an interrupt number");
		return;
	}

	if (cmd_match_eol (cmd)) {
		rc759_intlog_set (sim, n, NULL);
		pce_printf ("%02X: <deleted>\n", n);
		return;
	}

	if (!cmd_match_str (cmd, buf, 256)) {
		cmd_error (cmd, "need an expression");
		return;
	}

	pce_printf ("%02X: %s\n", n, buf);

	rc759_intlog_set (sim, n, buf);
}

static
void rc759_cmd_log (cmd_t *cmd, rc759_t *sim)
{
	if (cmd_match (cmd, "int")) {
		rc759_cmd_log_int (cmd, sim);
	}
	else {
		cmd_error (cmd, "log what?");
	}
}

static
void rc759_cmd_o (cmd_t *cmd, rc759_t *sim)
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
		e86_set_prt16 (sim->cpu, port, val);
	}
	else {
		e86_set_prt8 (sim->cpu, port, val);
	}
}

static
void rc759_cmd_pqc (cmd_t *cmd, rc759_t *sim)
{
	if (!cmd_match_end (cmd)) {
		return;
	}

	e86_pq_init (sim->cpu);
}

static
void rc759_cmd_pqs (cmd_t *cmd, rc759_t *sim)
{
	unsigned i;

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_puts ("PQ:");

	for (i = 0; i < sim->cpu->pq_cnt; i++) {
		pce_printf (" %02X", sim->cpu->pq[i]);
	}

	pce_puts ("\n");
}

static
void rc759_cmd_pqf (cmd_t *cmd, rc759_t *sim)
{
	if (!cmd_match_end (cmd)) {
		return;
	}

	e86_pq_fill (sim->cpu);
}

static
void rc759_cmd_pq (cmd_t *cmd, rc759_t *sim)
{
	if (cmd_match (cmd, "c")) {
		rc759_cmd_pqc (cmd, sim);
	}
	else if (cmd_match (cmd, "f")) {
		rc759_cmd_pqf (cmd, sim);
	}
	else if (cmd_match (cmd, "s")) {
		rc759_cmd_pqs (cmd, sim);
	}
	else if (cmd_match_eol (cmd)) {
		rc759_cmd_pqs (cmd, sim);
	}
	else {
		cmd_error (cmd, "pq: unknown command (%s)\n");
	}
}

static
void rc759_cmd_p (cmd_t *cmd, rc759_t *sim)
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

	pce_start (&sim->brk);

	sim->current_int &= 0xff;

	rc759_clock_discontinuity (sim);

	for (i = 0; i < n; i++) {
		e86_disasm_cur (sim->cpu, &op);

		seg = e86_get_cs (sim->cpu);
		ofs = e86_get_ip (sim->cpu);

		while ((e86_get_cs (sim->cpu) == seg) && (e86_get_ip (sim->cpu) == ofs)) {
			rc759_clock (sim, 1);

			if (rc759_check_break (sim)) {
				brk = 1;
				break;
			}
		}

		if (brk) {
			break;
		}

		if (op.flags & (E86_DFLAGS_CALL | E86_DFLAGS_LOOP)) {
			unsigned short ofs2 = ofs + op.dat_n;

			while ((e86_get_cs (sim->cpu) != seg) || (e86_get_ip (sim->cpu) != ofs2)) {
				rc759_clock (sim, 1);

				if (rc759_check_break (sim)) {
					brk = 1;
					break;
				}
			}
		}

		if (brk) {
			break;
		}

		if (rc759_check_break (sim)) {
			break;
		}
	}

	pce_stop();

	print_state (sim);
}

static
void rc759_cmd_r (cmd_t *cmd, rc759_t *sim)
{
	unsigned long val;
	char          sym[256];

	if (cmd_match_eol (cmd)) {
		print_state_cpu (sim->cpu);
		return;
	}

	if (!cmd_match_ident (cmd, sym, 256)) {
		pce_printf ("missing register\n");
		return;
	}

	if (e86_get_reg (sim->cpu, sym, &val)) {
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

	e86_set_reg (sim->cpu, sym, val);

	print_state (sim);
}

static
void rc759_cmd_set (cmd_t *cmd, rc759_t *sim)
{
	unsigned short val;
	char           str[256];

	if (!cmd_match_str (cmd, str, 256)) {
		cmd_error (cmd, "missing signal\n");
		return;
	}

	if (!cmd_match_uint16 (cmd, &val)) {
		cmd_error (cmd, "missing value\n");
		return;
	}

	if (strcmp (str, "pic.irq0") == 0) {
		e8259_set_irq0 (&sim->pic, val);
	}
	else if (strcmp (str, "pic.irq1") == 0) {
		e8259_set_irq1 (&sim->pic, val);
	}
	else if (strcmp (str, "pic.irq2") == 0) {
		e8259_set_irq2 (&sim->pic, val);
	}
	else if (strcmp (str, "pic.irq3") == 0) {
		e8259_set_irq3 (&sim->pic, val);
	}
	else if (strcmp (str, "pic.irq4") == 0) {
		e8259_set_irq4 (&sim->pic, val);
	}
	else if (strcmp (str, "pic.irq5") == 0) {
		e8259_set_irq5 (&sim->pic, val);
	}
	else if (strcmp (str, "pic.irq6") == 0) {
		e8259_set_irq6 (&sim->pic, val);
	}
	else if (strcmp (str, "pic.irq7") == 0) {
		e8259_set_irq7 (&sim->pic, val);
	}
	else {
		pce_printf ("unknown signal (%s)\n", str);
	}

	cmd_match_end (cmd);
}

static
void rc759_cmd_s (cmd_t *cmd, rc759_t *sim)
{
	if (cmd_match_eol (cmd)) {
		print_state (sim);
		return;
	}

	while (!cmd_match_eol (cmd)) {
		if (cmd_match (cmd, "rc759")) {
			print_state_rc759 (sim);
		}
		else if (cmd_match (cmd, "cpu")) {
			print_state_cpu (sim->cpu);
		}
		else if (cmd_match (cmd, "dma")) {
			print_state_dma (&sim->dma);
		}
		else if (cmd_match (cmd, "fdc")) {
			print_state_fdc (&sim->fdc);
		}
		else if (cmd_match (cmd, "icu")) {
			print_state_icu (&sim->icu);
		}
		else if (cmd_match (cmd, "mem")) {
			print_state_mem (sim);
		}
		else if (cmd_match (cmd, "ports")) {
			print_state_ports (sim);
		}
		else if (cmd_match (cmd, "pic")) {
			print_state_pic (&sim->pic);
		}
		else if (cmd_match (cmd, "ppi")) {
			print_state_ppi (&sim->ppi);
		}
		else if (cmd_match (cmd, "tcu")) {
			print_state_tcu (&sim->tcu);
		}
		else if (cmd_match (cmd, "time")) {
			print_state_time (sim->cpu);
		}
		else if (cmd_match (cmd, "video")) {
			print_state_video (&sim->crt);
		}
		else {
			cmd_error (cmd, "unknown component (%s)\n");
			return;
		}
	}
}

static
void rc759_cmd_t (cmd_t *cmd, rc759_t *sim)
{
	unsigned long i, n;

	n = 1;

	cmd_match_uint32 (cmd, &n);

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	rc759_clock_discontinuity (sim);

	for (i = 0; i < n; i++) {
		rc759_exec (sim);

		if (rc759_check_break (sim)) {
			break;
		}
	}

	pce_stop();

	print_state (sim);
}

static
void rc759_cmd_u (cmd_t *cmd, rc759_t *sim)
{
	unsigned short        seg, ofs, cnt, mode;
	static unsigned int   first = 1;
	static unsigned short sseg = 0;
	static unsigned short sofs = 0;
	e86_disasm_t          op;
	char                  str[256];

	if (first) {
		first = 0;
		sseg = e86_get_cs (sim->cpu);
		sofs = e86_get_ip (sim->cpu);
	}

	seg = sseg;
	ofs = sofs;
	cnt = 16;
	mode = 0;

	if (cmd_match_uint16_16 (cmd, &seg, &ofs)) {
		cmd_match_uint16 (cmd, &cnt);
	}

	cmd_match_uint16 (cmd, &mode);

	if (!cmd_match_end (cmd)) {
		return;
	}

	while (cnt > 0) {
		e86_disasm_mem (sim->cpu, &op, seg, ofs);
		disasm_str (str, &op);

		pce_printf ("%04X:%04X  %s\n", seg, ofs, str);

		ofs = (ofs + op.dat_n) & 0xffff;

		if (mode == 0) {
			cnt -= 1;
		}
		else {
			cnt = (cnt < op.dat_n) ? 0 : (cnt - op.dat_n);
		}
	}

	sseg = seg;
	sofs = ofs;
}

int rc759_cmd (rc759_t *sim, cmd_t *cmd)
{
	if (sim->trm != NULL) {
		trm_check (sim->trm);
	}

	if (cmd_match (cmd, "b")) {
		cmd_do_b (cmd, &sim->bps);
	}
	else if (cmd_match (cmd, "c")) {
		rc759_cmd_c (cmd, sim);
	}
	else if (cmd_match (cmd, "g")) {
		rc759_cmd_g (cmd, sim);
	}
	else if (cmd_match (cmd, "hm")) {
		rc759_cmd_hm (cmd);
	}
	else if (cmd_match (cmd, "i")) {
		rc759_cmd_i (cmd, sim);
	}
	else if (cmd_match (cmd, "key")) {
		rc759_cmd_key (cmd, sim);
	}
	else if (cmd_match (cmd, "log")) {
		rc759_cmd_log (cmd, sim);
	}
	else if (cmd_match (cmd, "o")) {
		rc759_cmd_o (cmd, sim);
	}
	else if (cmd_match (cmd, "pq")) {
		rc759_cmd_pq (cmd, sim);
	}
	else if (cmd_match (cmd, "p")) {
		rc759_cmd_p (cmd, sim);
	}
	else if (cmd_match (cmd, "r")) {
		rc759_cmd_r (cmd, sim);
	}
	else if (cmd_match (cmd, "set")) {
		rc759_cmd_set (cmd, sim);
	}
	else if (cmd_match (cmd, "s")) {
		rc759_cmd_s (cmd, sim);
	}
	else if (cmd_match (cmd, "t")) {
		rc759_cmd_t (cmd, sim);
	}
	else if (cmd_match (cmd, "u")) {
		rc759_cmd_u (cmd, sim);
	}
	else {
		return (1);
	}

	return (0);
}

void rc759_cmd_init (rc759_t *sim, monitor_t *mon)
{
	mon_cmd_add (mon, par_cmd, sizeof (par_cmd) / sizeof (par_cmd[0]));
	mon_cmd_add_bp (mon);

	sim->cpu->op_ext = sim;

	sim->cpu->op_int = pce_op_int;
	sim->cpu->op_undef = pce_op_undef;
}
