/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/pce.c                                            *
 * Created:       1999-04-16 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-09-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 1996-2003 by Hampa Hug <hampa@hampa.ch>                *
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

/* $Id: pce.c,v 1.24 2003/09/18 19:07:14 hampa Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include <unistd.h>
#include <signal.h>
#include <termios.h>

#include "pce.h"


typedef struct {
  unsigned i;
  char     str[256];
} cmd_t;


typedef struct breakpoint_t {
  struct breakpoint_t *next;
  unsigned short      seg;
  unsigned short      ofs;
  unsigned            pass;
  unsigned            reset;
} breakpoint_t;


int                       par_verbose = 0;
char                      *par_terminal = NULL;
char                      *par_video = NULL;
unsigned                  par_boot = 128;
char                      *par_cpu = NULL;
int                       pce_int28 = 1;


static unsigned           bp_cnt = 0;
static breakpoint_t       *breakpoint = NULL;

static unsigned long long pce_eops = 0;
static unsigned long long pce_eops_last = 0;
static unsigned long long pce_eclk = 0;
static unsigned long long pce_eclk_base = PCE_EMU_FOSC;
static unsigned long long pce_eclk_last = 0;
static unsigned long long pce_hclk = 0;
static unsigned long long pce_hclk_base = PCE_HOST_FOSC;
static unsigned long long pce_hclk_last = 0;

#define PCE_LAST_MAX 1024
static unsigned short     pce_last_i = 0;
static unsigned short     pce_last[PCE_LAST_MAX][2];

static ibmpc_t            *pc;


static
void prt_help (void)
{
  fputs (
    "usage: pce [options]\n"
    "  --help                 Print usage information\n"
    "  --version              Print version information\n"
    "  -v, --verbose          Verbose operation\n"
    "  -c, --config string    Set the config file\n"
    "  -l, --log string       Set the log file\n"
    "  -t, --terminal string  Set terminal\n"
    "  -g, --video string     Set video device\n"
    "  -p, --cpu string       Set the cpu model\n"
    "  -b, --boot int         Set boot drive [128]\n"
    "  -r, --run              Start running immediately\n",
    stdout
  );

  fflush (stdout);
}

static
void prt_version (void)
{
  fputs (
    "pce version " PCE_VERSION_STR
    " (compiled " PCE_CFG_DATE " " PCE_CFG_TIME ")\n"
    "Copyright (C) 1995-2003 Hampa Hug <hampa@hampa.ch>\n",
    stdout
  );

  fflush (stdout);
}

void sig_int (int s)
{
  /* hmm... */
}

#ifdef PCE_HAVE_TSC
static inline
unsigned long long read_tsc (void)
{
  unsigned long long ret;

  __asm __volatile (
    "rdtsc" : "=A" (ret) :: "memory"
  );

  return (ret);
}
#else
unsigned long long read_tsc (void)
{
  double sec;

  sec = (double) clock() / (double) CLOCKS_PER_SEC;

  return ((unsigned long long) sec * pce_hclk_base);
}
#endif

int str_is_space (char c)
{
  if (c == ' ') {
    return (1);
  }

  if (c == '\n') {
    return (1);
  }

  if (c == '\r') {
    return (1);
  }

  if (c == '\t') {
    return (1);
  }

  return (0);
}

char *str_ltrim (char *str)
{
  unsigned i, j;

  i = 0;
  j = 0;

  while ((str[i] != 0) && str_is_space (str[i])) {
    i += 1;
  }

  if (i == 0) {
    return (str);
  }

  while (str[i] != 0) {
    str[j] = str[i];
    i += 1;
    j += 1;
  }

  str[j] = 0;

  return (str);
}

char *str_rtrim (char *str)
{
  unsigned i, j;

  i = 0; j = 0;

  while (str[i] != 0) {
    if ((str[i] != ' ') && (str[i] != '\t') && (str[i] != '\n')) {
      j = i + 1;
    }
    i++;
  }

  str[j] = 0;

  return (str);
}

void cmd_get (cmd_t *cmd)
{
  fgets (cmd->str, 256, stdin);

  str_ltrim (cmd->str);
  str_rtrim (cmd->str);

  cmd->i = 0;

  trm_check (pc->trm);
}

int cmd_match_space (cmd_t *cmd)
{
  int      r;
  unsigned i;

  i = cmd->i;

  while ((cmd->str[i] != 0) && str_is_space (cmd->str[i])) {
    i++;
  }

  r = (i > cmd->i);

  cmd->i = i;

  return (r);
}

void cmd_error (cmd_t *cmd, const char *str)
{
  printf ("** %s [%s]\n", str, cmd->str + cmd->i);
  fflush (stdout);
}

int cmd_match_str (cmd_t *cmd, char *str)
{
  unsigned i, n;

  cmd_match_space (cmd);

  i = cmd->i;
  n = 0;

  while ((cmd->str[i] != 0) && !str_is_space (cmd->str[i])) {
    *(str++) = cmd->str[i];
    i += 1;
    n += 1;
  }

  *str = 0;

  cmd->i = i;

  return (n > 0);
}

int cmd_match_eol (cmd_t *cmd)
{
  cmd_match_space (cmd);

  if (cmd->str[cmd->i] == 0) {
    return (1);
  }

  return (0);
}

int cmd_match_end (cmd_t *cmd)
{
  if (cmd_match_eol (cmd)) {
    return (1);
  }

  cmd_error (cmd, "syntax error");

  return (0);
}

int cmd_match (cmd_t *cmd, const char *str)
{
  unsigned i;

  cmd_match_space (cmd);

  i = cmd->i;

  while ((*str != 0) && (cmd->str[i] == *str)) {
    i += 1;
    str += 1;
  }

  if (*str != 0) {
    return (0);
  }

  cmd->i = i;

  return (1);
}

int cmd_match_reg (cmd_t *cmd, unsigned short **reg)
{
  unsigned i;

  static char *dreg[8] = {
    "ax", "cx", "dx", "bx", "sp", "bp", "si", "di"
  };

  static char *sreg[4] = {
    "es", "cs", "ss", "ds"
  };

  for (i = 0; i < 8; i++) {
    if (cmd_match (cmd, dreg[i])) {
      *reg = &pc->cpu->dreg[i];
      return (1);
    }
  }

  for (i = 0; i < 4; i++) {
    if (cmd_match (cmd, sreg[i])) {
      *reg = &pc->cpu->sreg[i];
      return (1);
    }
  }

  if (cmd_match (cmd, "ip")) {
    *reg = &pc->cpu->ip;
    return (1);
  }

  if (cmd_match (cmd, "flags")) {
    *reg = &pc->cpu->flg;
    return (1);
  }

  *reg = NULL;

  return (0);
}

int cmd_match_uint16 (cmd_t *cmd, unsigned short *val)
{
  unsigned       i;
  unsigned       cnt;
  unsigned short ret;
  unsigned short *reg;

  if (cmd_match_reg (cmd, &reg)) {
    *val = *reg;
    return (1);
  }

  cmd_match_space (cmd);

  i = cmd->i;

  ret = 0;
  cnt = 0;

  while (cmd->str[i] != 0) {
    if ((cmd->str[i] >= '0') && (cmd->str[i] <= '9')) {
      ret = 16 * ret + (unsigned short) (cmd->str[i] - '0');
    }
    else if ((cmd->str[i] >= 'a') && (cmd->str[i] <= 'f')) {
      ret = 16 * ret + (unsigned short) (cmd->str[i] - 'a' + 10);
    }
    else if ((cmd->str[i] >= 'A') && (cmd->str[i] <= 'F')) {
      ret = 16 * ret + (unsigned short) (cmd->str[i] - 'A' + 10);
    }
    else {
      break;
    }

    cnt += 1;
    i += 1;
  }

  if (cnt == 0) {
    return (0);
  }

  cmd->i = i;
  *val = ret;

  return (1);
}

int cmd_match_uint32 (cmd_t *cmd, unsigned long *val)
{
  unsigned       i;
  unsigned       cnt;
  unsigned long  ret;
  unsigned short *reg;

  if (cmd_match_reg (cmd, &reg)) {
    *val = *reg;
    return (1);
  }

  cmd_match_space (cmd);

  i = cmd->i;

  ret = 0;
  cnt = 0;

  while (cmd->str[i] != 0) {
    if ((cmd->str[i] >= '0') && (cmd->str[i] <= '9')) {
      ret = 16 * ret + (unsigned short) (cmd->str[i] - '0');
    }
    else if ((cmd->str[i] >= 'a') && (cmd->str[i] <= 'f')) {
      ret = 16 * ret + (unsigned short) (cmd->str[i] - 'a' + 10);
    }
    else if ((cmd->str[i] >= 'A') && (cmd->str[i] <= 'F')) {
      ret = 16 * ret + (unsigned short) (cmd->str[i] - 'A' + 10);
    }
    else {
      break;
    }

    cnt += 1;
    i += 1;
  }

  if (cnt == 0) {
    return (0);
  }

  cmd->i = i;
  *val = ret;

  return (1);
}

int cmd_match_addr (cmd_t *cmd, unsigned short *seg, unsigned short *ofs)
{
  unsigned short val;

  if (!cmd_match_uint16 (cmd, &val)) {
    return (0);
  }

  if (!cmd_match (cmd, ":")) {
    *ofs = val;
    return (1);
  }

  *seg = val;

  cmd_match_uint16 (cmd, ofs);

  return (1);
}

breakpoint_t *bp_get (unsigned short seg, unsigned short ofs)
{
  breakpoint_t *bp;

  bp = breakpoint;
  while (bp != NULL) {
    if ((bp->seg == seg) && (bp->ofs == ofs)) {
      return (bp);
    }
    bp = bp->next;
  }

  return (NULL);
}

void bp_add (unsigned short seg, unsigned short ofs, unsigned pass, unsigned reset)
{
  breakpoint_t *bp;

  bp = bp_get (seg, ofs);

  if (bp != NULL) {
    bp->pass = pass;
    bp->reset = reset;
    return;
  }

  bp = (breakpoint_t *) malloc (sizeof (breakpoint_t));
  if (bp == NULL) {
    return;
  }

  bp->seg = seg;
  bp->ofs = ofs;
  bp->pass = pass;
  bp->reset = reset;

  bp->next = breakpoint;
  breakpoint = bp;

  bp_cnt += 1;
}

int bp_clear (unsigned short seg, unsigned short ofs)
{
  breakpoint_t *bp1, *bp2;

  bp1 = breakpoint;
  if (bp1 == NULL) {
    return (1);
  }

  if ((bp1->seg == seg) && (bp1->ofs == ofs)) {
    breakpoint = bp1->next;
    free (bp1);
    bp_cnt--;
    return (0);
  }

  bp2 = bp1->next;
  while (bp2 != NULL) {
    if ((bp2->seg == seg) && (bp2->ofs == ofs)) {
      bp1->next = bp2->next;
      free (bp2);
      bp_cnt--;
      return (0);
    }

    bp1 = bp2;
    bp2 = bp2->next;
  }

  return (1);
}

void bp_clear_all (void)
{
  breakpoint_t *bp;

  while (breakpoint != NULL) {
    bp = breakpoint->next;
    free (breakpoint);
    breakpoint = bp;
  }

  bp_cnt--;
}

void bp_print (breakpoint_t *bp, char *str)
{
  printf ("%s%04X:%04X  %04X  %04X\n",
    str,
    (unsigned) bp->seg, (unsigned) bp->ofs,
    (unsigned) bp->pass, (unsigned) bp->reset
  );
}

void bp_list (void)
{
  breakpoint_t *bp;

  bp = breakpoint;

  if (bp == NULL) {
    printf ("No breakpoints defined\n");
  }

  while (bp != NULL) {
    bp_print (bp, "  ");

    bp = bp->next;
  }
}

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

  strcpy (dst + dst_i, op->op);
  while (dst[dst_i] != 0) {
    dst_i += 1;
  }

  if (op->arg_n == 0) {
    dst[dst_i] = 0;
  }
  else if (op->arg_n == 1) {
    dst[dst_i++] = ' ';
    while (dst_i < 26) {
      dst[dst_i++] = ' ';
    }
    sprintf (dst + dst_i, op->arg1);
  }
  else if (op->arg_n == 2) {
    dst[dst_i++] = ' ';
    while (dst_i < 26) {
      dst[dst_i++] = ' ';
    }
    sprintf (dst + dst_i, "%s, %s", op->arg1, op->arg2);
  }
  else {
    dst[dst_i] = 0;
  }
}

void prt_uint8_bin (FILE *fp, unsigned char val)
{
  unsigned      i;
  unsigned char m;

  m = 0x80;

  for (i = 0; i < 8; i++) {
    if (val & m) {
      fputc ('1', fp);
    }
    else {
      fputc ('0', fp);
    }
    m = m >> 1;
  }
}

void prt_state_time (FILE *fp)
{
  double ratio;

  fputs ("-time------------------------------------------------------------------------\n", fp);

  if (pce_hclk > 0) {
    ratio = 1.0E-6 * ((double) pce_eclk / (double) pce_hclk * pce_hclk_base);
  }
  else {
    ratio = 0.0;
  }

  fprintf (fp,
    "HCLK=%016llX @ %8.4fMHz [%.4fs]\n"
    "ECLK=%016llX @ %8.4fMHz [%.4fs]\n"
    "EOPS=%016llX @ %8.4fMHz\n",
    pce_hclk, 1.0E-6 * pce_hclk_base, (double) pce_hclk / (double) pce_hclk_base,
    pce_eclk, 1.0E-6 * pce_eclk_base, (double) pce_eclk / (double) pce_eclk_base,
    pce_eops, ratio
  );
}

void prt_state_video (video_t *vid, FILE *fp)
{
  fputs ("-video-----------------------------------------------------------------------\n", fp);
  pce_video_prt_state (vid, fp);
}

void prt_state_pit (e8253_t *pit, FILE *fp)
{
  unsigned        i;
  e8253_counter_t *cnt;

  fprintf (fp, "-8253-PIT--------------------------------------------------------------------\n");

  for (i = 0; i < 3; i++) {
    cnt = &pit->counter[i];

    fprintf (fp,
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

void prt_state_ppi (e8255_t *ppi, FILE *fp)
{
  fputs ("-8255-PPI--------------------------------------------------------------------\n", fp);

  fprintf (fp,
    "MOD=%02X  MODA=%u  MODB=%u",
    ppi->mode, ppi->group_a_mode, ppi->group_b_mode
  );

  if (ppi->port[0].inp != 0) {
    fprintf (fp, "  A=I[%02X]", e8255_get_inp (ppi, 0));
  }
  else {
    fprintf (fp, "  A=O[%02X]", e8255_get_out (ppi, 0));
  }

  if (ppi->port[1].inp != 0) {
    fprintf (fp, "  B=I[%02X]", e8255_get_inp (ppi, 1));
  }
  else {
    fprintf (fp, "  B=O[%02X]", e8255_get_out (ppi, 1));
  }

  switch (ppi->port[2].inp) {
    case 0xff:
      fprintf (fp, "  C=I[%02X]", e8255_get_inp (ppi, 2));
      break;

    case 0x00:
      fprintf (fp, "  C=O[%02X]", e8255_get_out (ppi, 2));
      break;

    case 0x0f:
      fprintf (fp, "  CH=O[%X]  CL=I[%X]",
        (e8255_get_out (ppi, 2) >> 4) & 0x0f, e8255_get_inp (ppi, 2) & 0x0f
      );
      break;

    case 0xf0:
      fprintf (fp, "  CH=I[%X]  CL=O[%X]",
        (e8255_get_inp (ppi, 2) >> 4) & 0x0f, e8255_get_out (ppi, 2) & 0x0f
      );
      break;
  }

  fputs ("\n", fp);
  fflush (fp);
}

void prt_state_pic (e8259_t *pic, FILE *fp)
{
  unsigned i;

  fputs ("-8259A-PIC-------------------------------------------------------------------\n", fp);
  fputs ("IRR=", fp); prt_uint8_bin (fp, pic->irr);
  fputs ("  IMR=", fp); prt_uint8_bin (fp, pic->imr);
  fputs ("  ISR=", fp); prt_uint8_bin (fp, pic->isr);
  fputs ("\n", fp);

  fprintf (fp, "ICW=[%02X %02X %02X %02X]  OCW=[%02X %02X %02X]\n",
    pic->icw[0], pic->icw[1], pic->icw[2], pic->icw[3],
    pic->ocw[0], pic->ocw[1], pic->ocw[2]
  );

  fprintf (fp, "N0=%04lX", pic->irq_cnt[0]);
  for (i = 1; i < 8; i++) {
    fprintf (fp, "  N%u=%04lX", i, pic->irq_cnt[i]);
  }
  fputs ("\n", fp);

  fflush (fp);
}

void prt_state_uart (e8250_t *uart, unsigned base, FILE *fp)
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

  fputs ("-8250-UART-------------------------------------------------------------------\n", fp);
  fprintf (stderr,
    "IO=%04X  %lu %u%c%u  DTR=%d  RTS=%d\n"
    "TxD=%02X%c RxD=%02X%c SCR=%02X  DIV=%04X\n"
    "IER=%02X  IIR=%02X  LCR=%02X  LSR=%02X  MCR=%02X  MSR=%02X\n",
    base,
    e8250_get_bps (uart), e8250_get_databits (uart), p, e8250_get_stopbits (uart),
    e8250_get_dtr (uart), e8250_get_rts (uart),
    uart->txd[0], uart->txd[1] ? '*' : ' ',
    uart->rxd[0], uart->rxd[1] ? '*' : ' ',
    uart->scratch, uart->divisor,
    uart->ier, uart->iir, uart->lcr, uart->lsr, uart->mcr, uart->msr
  );
}

void prt_state_cpu (e8086_t *c, FILE *fp)
{
  double      cpi, mips;
  static char ft[2] = { '-', '+' };

  fputs ("-8086------------------------------------------------------------------------\n", fp);

  cpi = (c->instructions > 0) ? ((double) c->clocks / (double) c->instructions) : 1.0;
  mips = (c->clocks > 0) ? (4.77 * (double) c->instructions / (double) c->clocks) : 0.0;
  fprintf (fp, "CLK=%llu  OP=%llu  DLY=%lu  CPI=%.4f  MIPS=%.4f\n",
    c->clocks, c->instructions,
    c->delay,
    cpi, mips
  );

  fprintf (fp,
    "AX=%04X  BX=%04X  CX=%04X  DX=%04X  SP=%04X  BP=%04X  SI=%04X  DI=%04X INT=%02X\n",
    e86_get_ax (c), e86_get_bx (c), e86_get_cx (c), e86_get_dx (c),
    e86_get_sp (c), e86_get_bp (c), e86_get_si (c), e86_get_di (c),
    e86_get_last_int (c)
  );

  fprintf (fp, "CS=%04X  DS=%04X  ES=%04X  SS=%04X  IP=%04X  F =%04X",
    e86_get_cs (c), e86_get_ds (c), e86_get_es (c), e86_get_ss (c),
    e86_get_ip (c), c->flg
  );

  fprintf (fp, "  I%c D%c O%c S%c Z%c A%c P%c C%c\n",
    ft[e86_get_if (c)], ft[e86_get_df (c)],
    ft[e86_get_of (c)], ft[e86_get_sf (c)],
    ft[e86_get_zf (c)], ft[e86_get_af (c)],
    ft[e86_get_pf (c)], ft[e86_get_cf (c)]
  );
}

void prt_state_pc (ibmpc_t *pc, FILE *fp)
{
  prt_state_video (pc->video, fp);
  prt_state_ppi (pc->ppi, fp);
  prt_state_pit (pc->pit, fp);
  prt_state_pic (pc->pic, fp);
  prt_state_time (fp);
  prt_state_cpu (pc->cpu, fp);
}

void prt_state (ibmpc_t *pc, FILE *fp)
{
  e86_disasm_t op;
  char         str[256];

  e86_disasm_cur (pc->cpu, &op);
  disasm_str (str, &op);

  prt_state_cpu (pc->cpu, fp);

  fprintf (fp, "%04X:%04X  %s\n",
    (unsigned) pc->cpu->sreg[E86_REG_CS],
    (unsigned) pc->cpu->ip,
    str
  );
}

void prt_prompt (FILE *fp)
{
  fputs ("\x1b[0;37;40m-", fp);
  fflush (fp);
}

void prt_error (const char *str, ...)
{
  va_list va;

  va_start (va, str);
  vfprintf (stderr, str, va);
  va_end (va);
}

void pce_set_fd (int fd, int interactive)
{
  static int            sios_ok = 0;
  static struct termios sios;
  struct termios        tios;

  if (sios_ok == 0) {
    tcgetattr (fd, &sios);
    sios_ok = 1;
  }

  if (interactive) {
    tcsetattr (fd, TCSANOW, &sios);
  }
  else {
    tios = sios;

    tios.c_lflag &= ~(ICANON | ECHO);
    tios.c_cc[VMIN] = 1;
    tios.c_cc[VTIME] = 0;

    tcsetattr (fd, TCSANOW, &tios);
  }
}

void cpu_start()
{
  pce_eclk_last = e86_get_clock (pc->cpu);
  pce_eops_last = e86_get_opcnt (pc->cpu);
  pce_hclk_last = read_tsc();
}

void cpu_end()
{
  pce_hclk_last = read_tsc() - pce_hclk_last;
  pce_eclk_last = e86_get_clock (pc->cpu) - pce_eclk_last;
  pce_eops_last = e86_get_opcnt (pc->cpu) - pce_eops_last;

  pce_eclk += pce_eclk_last;
  pce_hclk += pce_hclk_last;
  pce_eops += pce_eops_last;
}

void cpu_exec (void)
{
  unsigned long long old;

  old = e86_get_opcnt (pc->cpu);

  while (e86_get_opcnt (pc->cpu) == old) {
    pc_clock (pc);
  }
}

void pce_start (void)
{
  pce_set_fd (0, 0);
  pc->brk = 0;
  cpu_start();
}

void pce_stop (void)
{
  cpu_end();
  pce_set_fd (0, 1);
}

void pce_run (void)
{
  pce_start();

  while (1) {
    cpu_exec();

    if (pc->brk) {
      break;
    }
  }

  pce_stop();
}

void pce_op_stat (void *ext, unsigned char op1, unsigned char op2)
{
  ibmpc_t *pc;

  pc = (ibmpc_t *) ext;

  pce_last_i = (pce_last_i + 1) % PCE_LAST_MAX;
  pce_last[pce_last_i][0] = e86_get_cs (pc->cpu);
  pce_last[pce_last_i][1] = e86_get_ip (pc->cpu);

//  pce_log (MSG_DEB, "%04X:%04X\n", e86_get_cs (pc->cpu), e86_get_ip (pc->cpu));

  if (pce_int28) {
    if ((op1 == 0xcd) && (op2 == 0x28) && (pc->key_clk == 0)) {
      cpu_end();
      usleep (10000);
      cpu_start();
    }
  }
}

void pce_op_undef (void *ext, unsigned char op1, unsigned char op2)
{
  ibmpc_t *pc;

  pc = (ibmpc_t *) ext;

  pce_log (MSG_DEB, "%04X:%04X: undefined operation [%02X %02x]\n",
    e86_get_cs (pc->cpu), e86_get_ip (pc->cpu), op1, op2
  );
}


void do_bc (cmd_t *cmd)
{
  unsigned short seg, ofs;

  seg = e86_get_cs (pc->cpu);
  ofs = e86_get_ip (pc->cpu);

  if (!cmd_match_addr (cmd, &seg, &ofs)) {
    cmd_error (cmd, "expecting address");
    return;
  }

  if (!cmd_match_end (cmd)) {
    return;
  }

  if (bp_clear (seg, ofs)) {
    printf ("no breakpoint cleared at %04X:%04X\n", seg, ofs);
  }
}

void do_bl (cmd_t *cmd)
{
  if (!cmd_match_end (cmd)) {
    return;
  }

  bp_list ();
}

void do_bs (cmd_t *cmd)
{
  unsigned short seg, ofs, pass, reset;

  seg = pc->cpu->sreg[E86_REG_CS];
  ofs = 0;
  pass = 1;
  reset = 0;

  if (!cmd_match_addr (cmd, &seg, &ofs)) {
    cmd_error (cmd, "expecting address");
    return;
  }

  cmd_match_uint16 (cmd, &pass);
  cmd_match_uint16 (cmd, &reset);

  if (!cmd_match_end (cmd)) {
    return;
  }

  if (pass > 0) {
    printf ("Breakpoint at %04X:%04X  %04X  %04X\n",
      (unsigned)seg,
      (unsigned)ofs,
      pass, reset
    );

    bp_add (seg, ofs, pass, reset);
  }
}

void do_b (cmd_t *cmd)
{
  if (cmd_match (cmd, "l")) {
    do_bl (cmd);
  }
  else if (cmd_match (cmd, "s")) {
    do_bs (cmd);
  }
  else if (cmd_match (cmd, "c")) {
    do_bc (cmd);
  }
  else {
    prt_error ("b: unknown command (%s)\n", cmd->str + cmd->i);
  }
}

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

  prt_state (pc, stdout);
}

void do_dump (cmd_t *cmd)
{
  FILE *fp;

  if (!cmd_match_end (cmd)) {
    return;
  }

  fp = fopen ("memory.dat", "wb");
  if (fp == NULL) {
    return;
  }

  fwrite (pc->ram->data, 1, pc->ram->size, fp);

  fclose (fp);
}

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

  if (cmd_match_addr (cmd, &seg, &ofs1)) {
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

    fputs (buf, stdout);
  }
}

void do_e (cmd_t *cmd)
{
  unsigned short seg, ofs;
  unsigned short val;

  seg = 0;
  ofs = 0;

  if (!cmd_match_addr (cmd, &seg, &ofs)) {
    cmd_error (cmd, "need an address");
  }

  while (cmd_match_uint16 (cmd, &val)) {
    mem_set_uint8 (pc->mem, (seg << 4) + ofs, val);
    ofs = (ofs + 1) & 0xffff;
  }
}

void do_g (cmd_t *cmd)
{
  breakpoint_t   *bp;
  unsigned short seg, ofs;

  if (!cmd_match_end (cmd)) {
    return;
  }

  pce_start();

  while (1) {
    cpu_exec();

    seg = e86_get_cs (pc->cpu);
    ofs = e86_get_ip (pc->cpu);

    bp = bp_get (seg, ofs);

    if (bp != NULL) {
      if (bp->pass > 0) {
        bp->pass--;
      }

      if (bp->pass == 0) {
        if (bp->reset == 0) {
          bp_clear (seg, ofs);
        }
        else {
          bp->pass = bp->reset;
          bp_print (bp, "brk: ");
        }

        break;
      }
    }

    if (pc->brk) {
      break;
    }
  }

  pce_stop();

  fputs ("\n", stdout);
  prt_state (pc, stdout);
}

void do_h (cmd_t *cmd)
{
  fputs (
    "bc addr                 clear a breakpoint\n"
    "bl                      list breakpoints\n"
    "bs addr [pass [reset]]  set a breakpoint\n",
    stdout
  );
}

void do_int28 (cmd_t *cmd)
{
  int c;

  c = 0;

  if (cmd_match (cmd, "on")) {
    c = 1;
  }
  else if (cmd_match (cmd, "off")) {
    c = 2;
  }

  if (!cmd_match_end (cmd)) {
    return;
  }

  switch (c) {
    case 1:
      pce_int28 = 1;
      break;

    case 2:
      pce_int28 = 0;
      break;
  }

  printf ("int 28h sleeping is %s\n", pce_int28 ? "on" : "off");
}

void do_last (cmd_t *cmd)
{
  unsigned short i, j, n;
  unsigned       idx;

  i = 0;
  n = 16;

  cmd_match_uint16 (cmd, &n);
  cmd_match_uint16 (cmd, &i);

  idx = (pce_last_i + PCE_LAST_MAX - i - n + 1) % PCE_LAST_MAX;

  for (j = 0; j < n; j++) {
    printf ("%d: %04X:%04X\n",
      (int) j - (int) n - (int) i,
      pce_last[idx][0], pce_last[idx][1]
    );
    idx = (idx + 1) % PCE_LAST_MAX;
  }
}

void do_parport (cmd_t *cmd)
{
  unsigned short port;
  char           fname[256];

  if (!cmd_match_uint16 (cmd, &port)) {
    cmd_error (cmd, "need a port number");
    return;
  }

  if (!cmd_match_str (cmd, fname)) {
    cmd_error (cmd, "need a file name");
    return;
  }

  if (!cmd_match_end (cmd)) {
    return;
  }

  if ((port >= 4) || (pc->parport[port] == NULL)) {
    prt_error ("no parallel port %u\n", (unsigned) port);
    return;
  }

  if (parport_set_fname (pc->parport[port], fname)) {
    prt_error ("setting new file failed\n");
    return;
  }
}

void do_p (cmd_t *cmd)
{
  unsigned short seg, ofs;
  e86_disasm_t   op;

  if (!cmd_match_end (cmd)) {
    return;
  }

  e86_disasm_cur (pc->cpu, &op);

  seg = e86_get_cs (pc->cpu);
  ofs = e86_get_ip (pc->cpu) + op.dat_n;

  pce_start();

  while ((e86_get_cs (pc->cpu) == seg) && (e86_get_ip (pc->cpu) == ofs)) {
    pc_clock (pc);

    if (pc->brk) {
      break;
    }
  }

  while ((e86_get_cs (pc->cpu) != seg) || (e86_get_ip (pc->cpu) != ofs)) {
    pc_clock (pc);

    if (pc->brk) {
      break;
    }
  }

  pce_stop();

  prt_state (pc, stdout);
}

void do_r (cmd_t *cmd)
{
  unsigned short val;
  unsigned short *reg;

  if (!cmd_match_reg (cmd, &reg)) {
    prt_error ("missing register\n");
    return;
  }

  if (!cmd_match_uint16 (cmd, &val)) {
    prt_error ("missing value\n");
    return;
  }

  if (!cmd_match_end (cmd)) {
    return;
  }

  *reg = val;

  prt_state (pc, stdout);
}

void do_s (cmd_t *cmd)
{
  if (cmd_match_eol (cmd)) {
    prt_state (pc, stdout);
    return;
  }

  while (!cmd_match_eol (cmd)) {
    if (cmd_match (cmd, "pc")) {
      prt_state_pc (pc, stdout);
    }
    else if (cmd_match (cmd, "cpu")) {
      prt_state_cpu (pc->cpu, stdout);
    }
    else if (cmd_match (cmd, "pit")) {
      prt_state_pit (pc->pit, stdout);
    }
    else if (cmd_match (cmd, "ppi")) {
      prt_state_ppi (pc->ppi, stdout);
    }
    else if (cmd_match (cmd, "pic")) {
      prt_state_pic (pc->pic, stdout);
    }
    else if (cmd_match (cmd, "uart")) {
      unsigned short i;
      if (!cmd_match_uint16 (cmd, &i)) {
        i = 0;
      }
      if ((i < 4) && (pc->serport[i] != NULL)) {
        prt_state_uart (&pc->serport[i]->uart, pc->serport[i]->io, stdout);
      }
    }
    else if (cmd_match (cmd, "video")) {
      prt_state_video (pc->video, stdout);
    }
    else if (cmd_match (cmd, "time")) {
      prt_state_time (stdout);
    }
    else {
      prt_error ("unknown component (%s)\n", cmd->str + cmd->i);
      return;
    }
  }
}

void do_screenshot (cmd_t *cmd)
{
  char     fname[256];
  unsigned mode;
  FILE     *fp;

  if (!cmd_match_str (cmd, fname)) {
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
    prt_error ("can't open file (%s)\n", fname);
    return;
  }

  if (pce_video_screenshot (pc->video, fp, mode)) {
    fclose (fp);
    prt_error ("screenshot failed\n");
    return;
  }

  fclose (fp);
}

void do_time (cmd_t *cmd)
{
  if (cmd_match (cmd, "c")) {
    pce_eops = 0;
    pce_eclk = 0;
    pce_hclk = 0;
  }
  else if (cmd_match_eol (cmd)) {
    prt_state_time (stdout);
  }
}

void do_t (cmd_t *cmd)
{
  unsigned long i, n;

  n = 1;

  cmd_match_uint32 (cmd, &n);

  if (!cmd_match_end (cmd)) {
    return;
  }

  pce_start();

  for (i = 0; i < n; i++) {
    cpu_exec();
  }

  pce_stop();

  prt_state (pc, stdout);
}

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

  if (cmd_match_addr (cmd, &seg, &ofs)) {
    cmd_match_uint16 (cmd, &cnt);
  }

  if (!cmd_match_end (cmd)) {
    return;
  }

  for (i = 0; i < cnt; i++) {
    e86_disasm_mem (pc->cpu, &op, seg, ofs);
    disasm_str (str, &op);

    fprintf (stdout, "%04X:%04X  %s\n", seg, ofs, str);

    ofs = (ofs + op.dat_n) & 0xffff;
  }

  sseg = seg;
  sofs = ofs;
}

int do_cmd (void)
{
  cmd_t  cmd;

  while (1) {
    prt_prompt (stdout);
    fflush (stdout);

    cmd_get (&cmd);

    if (cmd_match (&cmd, "b")) {
      do_b (&cmd);
    }
    else if (cmd_match (&cmd, "c")) {
      do_c (&cmd);
    }
    else if (cmd_match (&cmd, "dump")) {
      do_dump (&cmd);
    }
    else if (cmd_match (&cmd, "d")) {
      do_d (&cmd);
    }
    else if (cmd_match (&cmd, "e")) {
      do_e (&cmd);
    }
    else if (cmd_match (&cmd, "g")) {
      do_g (&cmd);
    }
    else if (cmd_match (&cmd, "h")) {
      do_h (&cmd);
    }
    else if (cmd_match (&cmd, "int28")) {
      do_int28 (&cmd);
    }
    else if (cmd_match (&cmd, "last")) {
      do_last (&cmd);
    }
    else if (cmd_match (&cmd, "parport")) {
      do_parport (&cmd);
    }
    else if (cmd_match (&cmd, "p")) {
      do_p (&cmd);
    }
    else if (cmd_match (&cmd, "q")) {
      break;
    }
    else if (cmd_match (&cmd, "r")) {
      do_r (&cmd);
    }
    else if (cmd_match (&cmd, "screenshot")) {
      do_screenshot (&cmd);
    }
    else if (cmd_match (&cmd, "s")) {
      do_s (&cmd);
    }
    else if (cmd_match (&cmd, "time")) {
      do_time (&cmd);
    }
    else if (cmd_match (&cmd, "t")) {
      do_t (&cmd);
    }
    else if (cmd_match (&cmd, "u")) {
      do_u (&cmd);
    }
    else if (cmd.str[cmd.i] == 0) {
      ;
    }
    else {
      printf ("unknown command (%s)\n", cmd.str);
    }

    if (pc->brk == PCE_BRK_ABORT) {
      break;
    }
  };

  return (0);
}

ini_sct_t *pce_load_config (const char *fname)
{
  ini_sct_t *ini;
  char      *home;
  char      buf[1024];

  if (fname != NULL) {
    ini = ini_read (fname);
    if (ini != NULL) {
      pce_log (MSG_INF, "pce:\tusing config file '%s'\n", fname);
      return (ini);
    }
  }

  home = getenv ("HOME");
  if (home != NULL) {
    sprintf (buf, "%s/.pce.cfg", home);
    ini = ini_read (buf);
    if (ini != NULL) {
      pce_log (MSG_INF, "pce:\tusing config file '%s'\n", buf);
      return (ini);
    }
  }

  ini = ini_read (PCE_DIR_ETC "/pce.cfg");
  if (ini != NULL) {
    pce_log (MSG_INF, "pce:\tusing config file '" PCE_DIR_ETC "/pce.cfg'\n");
    return (ini);
  }

  return (NULL);
}

int str_isarg1 (const char *str, const char *arg)
{
  if (strcmp (str, arg) == 0) {
    return (1);
  }

  return (0);
}

int str_isarg2 (const char *str, const char *arg1, const char *arg2)
{
  if (strcmp (str, arg1) == 0) {
    return (1);
  }

  if (strcmp (str, arg2) == 0) {
    return (1);
  }

  return (0);
}

int main (int argc, char *argv[])
{
  int       i;
  int       run;
  char      *cfg;
  ini_sct_t *ini, *sct;

  if (argc == 2) {
    if (str_isarg1 (argv[1], "--help")) {
      prt_help();
      return (0);
    }
    else if (str_isarg1 (argv[1], "--version")) {
      prt_version();
      return (0);
    }
  }

  cfg = NULL;
  run = 0;

  pce_log_set_fp (NULL, 0);
  pce_log_set_stderr (0);

  i = 1;
  while (i < argc) {
    if (str_isarg2 (argv[i], "-v", "--verbose")) {
      par_verbose = 1;
    }
    else if (str_isarg2 (argv[i], "-c", "--config")) {
      i += 1;
      if (i >= argc) {
        return (1);
      }
      cfg = argv[i];
    }
    else if (str_isarg2 (argv[i], "-l", "--log")) {
      i += 1;
      if (i >= argc) {
        return (1);
      }
      pce_log_set_fname (argv[i]);
    }
    else if (str_isarg2 (argv[i], "-t", "--terminal")) {
      i += 1;
      if (i >= argc) {
        return (1);
      }

      par_terminal = argv[i];
    }
    else if (str_isarg2 (argv[i], "-g", "--video")) {
      i += 1;
      if (i >= argc) {
        return (1);
      }

      par_video = argv[i];
    }
    else if (str_isarg2 (argv[i], "-p", "--cpu")) {
      i += 1;
      if (i >= argc) {
        return (1);
      }

      par_cpu = argv[i];
    }
    else if (str_isarg2 (argv[i], "-b", "--boot")) {
      i += 1;
      if (i >= argc) {
        return (1);
      }

      par_boot = (unsigned) strtoul (argv[i], NULL, 0);
    }
    else if (str_isarg2 (argv[i], "-r", "--run")) {
      run = 1;
    }
    else {
      printf ("%s: unknown option (%s)\n", argv[0], argv[i]);
      return (1);
    }

    i += 1;
  }

  if (par_verbose) {
    pce_log_set_stderr (1);
  }

  pce_log (MSG_INF,
    "pce version " PCE_VERSION_STR
    " (compiled " PCE_CFG_DATE " " PCE_CFG_TIME ")\n"
    "Copyright (C) 1995-2003 Hampa Hug <hampa@hampa.ch>\n"
  );

  ini = pce_load_config (cfg);
  if (ini == NULL) {
    pce_log (MSG_ERR, "loading config file failed (%s)\n", cfg);
    return (1);
  }

  sct = ini_sct_find_sct (ini, "pc");
  if (sct == NULL) {
    pce_log (MSG_ERR, "section 'pc' not found\n");
    return (1);
  }

  pc = pc_new (sct);

  pc->cpu->op_stat = &pce_op_stat;
  pc->cpu->op_undef = &pce_op_undef;

  e86_reset (pc->cpu);

  signal (SIGINT, &sig_int);

  if (run) {
    pce_run();
    if (pc->brk != 2) {
      fputs ("\n", stdout);
      do_cmd();
    }
  }
  else {
    do_cmd();
  }

  pc_del (pc);

  return (0);
}
