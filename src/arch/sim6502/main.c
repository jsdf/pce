/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/sim6502/main.c                                    *
 * Created:       2004-05-25 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-06-23 by Hampa Hug <hampa@hampa.ch>                   *
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


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>

#include <unistd.h>
#include <signal.h>
#include <termios.h>

#include "main.h"


int       par_verbose = 0;

sim6502_t *par_sim = NULL;
ini_sct_t *par_cfg = NULL;
unsigned  par_sig_int = 0;


static breakpoint_t *breakpoint = NULL;


static
void prt_help (void)
{
  fputs (
    "usage: sim6502 [options]\n"
    "  --help                 Print usage information\n"
    "  --version              Print version information\n"
    "  -v, --verbose          Verbose operation\n"
    "  -c, --config string    Set the config file\n"
    "  -l, --log string       Set the log file\n"
    "  -r, --run              Start running immediately\n",
    stdout
  );

  fflush (stdout);
}

static
void prt_version (void)
{
  fputs (
    "pce sim6502 version " PCE_VERSION_STR
    " (" PCE_CFG_DATE " " PCE_CFG_TIME ")\n"
    "Copyright (C) 1995-2004 Hampa Hug <hampa@hampa.ch>\n",
    stdout
  );

  fflush (stdout);
}

void sig_int (int s)
{
  par_sig_int = 1;
}

void sig_segv (int s)
{
  fprintf (stderr, "pce-sim6502: segmentation fault\n");
  fflush (stderr);

  exit (1);
}

int s6502_match_reg8 (cmd_t *cmd, sim6502_t *sim, unsigned char **reg)
{
  cmd_match_space (cmd);

  if (cmd_match (cmd, "ra")) {
    *reg = &sim->cpu->a;
    return (1);
  }
  else if (cmd_match (cmd, "x")) {
    *reg = &sim->cpu->x;
    return (1);
  }
  else if (cmd_match (cmd, "y")) {
    *reg = &sim->cpu->y;
    return (1);
  }
  else if (cmd_match (cmd, "p")) {
    *reg = &sim->cpu->p;
    return (1);
  }
  else if (cmd_match (cmd, "s")) {
    *reg = &sim->cpu->s;
    return (1);
  }

  return (0);
}

int s6502_match_reg16 (cmd_t *cmd, sim6502_t *sim, unsigned short **reg)
{
  cmd_match_space (cmd);

  if (cmd_match (cmd, "pc") || cmd_match (cmd, "PC")) {
    *reg = &sim->cpu->pc;
    return (1);
  }

  return (0);
}

static
int cmd_match_sym (cmd_t *cmd, unsigned long *val)
{
  unsigned char  *reg8;
  unsigned short *reg16;

  if (s6502_match_reg16 (cmd, par_sim, &reg16)) {
    *val = *reg16;
    return (1);
  }

  if (s6502_match_reg8 (cmd, par_sim, &reg8)) {
    *val = *reg8;
    return (1);
  }

  return (0);
}

void s6502_disasm_str (char *dst, e6502_disasm_t *op)
{
  unsigned i, n;

  n = sprintf (dst, "%04X ", (unsigned) op->pc);

  for (i = 0; i < op->dat_n; i++) {
    n += sprintf (dst + n, " %02X", (unsigned) op->dat[i]);
  }

  while (n < 16) {
    dst[n++] = ' ';
  }

  n += sprintf (dst + n, "%s", op->op);

  if (op->arg_n > 0) {
    while (n < 21) {
      dst[n++] = ' ';
    }

    n += sprintf (dst + n, "%s", op->arg1);
  }

  dst[n] = 0;
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

void prt_error (const char *str, ...)
{
  va_list va;

  va_start (va, str);
  vfprintf (stderr, str, va);
  va_end (va);
}

void prt_sep (FILE *fp, const char *str, ...)
{
  unsigned i;
  va_list  va;

  fputs ("-", fp);
  i = 1;

  va_start (va, str);
  i += vfprintf (fp, str, va);
  va_end (va);

  while (i < 78) {
    fputc ('-', fp);
    i += 1;
  }

  fputs ("\n", fp);
}

void prt_state_cpu (e6502_t *c, FILE *fp)
{
  unsigned long long opcnt, clkcnt;
  unsigned long      delay;
  e6502_disasm_t     op;
  char               str[256];

  prt_sep (fp, "6502");

  opcnt = e6502_get_opcnt (c);
  clkcnt = e6502_get_clock (c);
  delay = e6502_get_delay (c);

  fprintf (fp, "CLK=%llx  OP=%llx  DLY=%lx  CPI=%.4f\n",
    clkcnt, opcnt, delay,
    (opcnt > 0) ? ((double) (clkcnt + delay) / (double) opcnt) : 1.0
  );

  fprintf (fp, "A=%02X  X=%02X  Y=%02X  S=%02X  P=%02X [%c%c-%c%c%c%c%c]  PC=%04X\n",
    (unsigned) e6502_get_a (c),
    (unsigned) e6502_get_x (c),
    (unsigned) e6502_get_y (c),
    (unsigned) e6502_get_s (c),
    (unsigned) e6502_get_p (c),
    (e6502_get_nf (c)) ? 'N' : '-',
    (e6502_get_vf (c)) ? 'V' : '-',
    (e6502_get_bf (c)) ? 'B' : '-',
    (e6502_get_df (c)) ? 'D' : '-',
    (e6502_get_if (c)) ? 'I' : '-',
    (e6502_get_cf (c)) ? 'C' : '-',
    (e6502_get_zf (c)) ? 'Z' : '-',
    (unsigned) e6502_get_pc (c)
  );

  e6502_disasm_cur (c, &op);
  s6502_disasm_str (str, &op);

  fprintf (fp, "%s\n", str);
}

static
void prt_state_default (sim6502_t *sim, FILE *fp)
{
  prt_state_cpu (sim->cpu, fp);
}

static
void prt_state (sim6502_t *sim, FILE *fp, const char *str)
{
}

static
void prt_prompt (sim6502_t *sim, FILE *fp)
{
  fputs ("\x1b[0;37;40m-", fp);

  fflush (fp);
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

void pce_start (void)
{
  pce_set_fd (0, 0);
  par_sim->brk = 0;
}

void pce_stop (void)
{
  pce_set_fd (0, 1);
}

void pce_run (sim6502_t *sim)
{
  pce_start();

  while (1) {
    s6502_clock (sim, 1);

    if (sim->brk) {
      break;
    }
  }

  pce_stop();
}


static
void s6502_exec (sim6502_t *sim)
{
  unsigned long long old;

  old = e6502_get_opcnt (sim->cpu);

  while (e6502_get_opcnt (sim->cpu) == old) {
    s6502_clock (sim, 1);
  }
}

static
int s6502_exec_to (sim6502_t *sim, unsigned short addr)
{
  while (e6502_get_pc (sim->cpu) != addr) {
    s6502_clock (sim, 1);

    if (sim->brk) {
      return (1);
    }
  }

  return (0);
}

static
int s6502_exec_off (sim6502_t *sim, unsigned short addr)
{
  while (e6502_get_pc (sim->cpu) == addr) {
    s6502_clock (sim, 1);

    if (sim->brk) {
      return (1);
    }
  }

  return (0);
}

static
void s6502_run_bp (sim6502_t *sim)
{
  pce_start();

  while (1) {
    s6502_exec_off (sim, e6502_get_pc (sim->cpu));

    if (bp_check (&breakpoint, e6502_get_pc (sim->cpu), 0)) {
      break;
    }

    if (sim->brk) {
      break;
    }
  }

  pce_stop();
}


static
void s6502_log_undef (void *ext, unsigned char op)
{
  sim6502_t *sim;

  sim = (sim6502_t *) ext;

  pce_log (MSG_DEB,
    "%04X: undefined operation [%02X]\n",
    (unsigned) e6502_get_pc (sim->cpu), (unsigned) op
  );

  s6502_break (sim, PCE_BRK_STOP);
}


static
void do_bc (cmd_t *cmd, sim6502_t *sim)
{
  unsigned short addr;

  if (cmd_match_eol (cmd)) {
    bp_clear_all (&breakpoint);
    return;
  }

  if (!cmd_match_uint16 (cmd, &addr)) {
    cmd_error (cmd, "expecting address");
    return;
  }

  if (!cmd_match_end (cmd)) {
    return;
  }

  if (bp_clear (&breakpoint, addr, 0)) {
    printf ("no breakpoint cleared at %04X\n", (unsigned) addr);
  }
}

static
void do_bl (cmd_t *cmd, sim6502_t *sim)
{
  if (!cmd_match_end (cmd)) {
    return;
  }

  bp_list (breakpoint, 0);
}

static
void do_bs (cmd_t *cmd, sim6502_t *sim)
{
  unsigned short addr, pass, reset;

  addr = e6502_get_pc (sim->cpu);
  pass = 1;
  reset = 0;

  if (!cmd_match_uint16 (cmd, &addr)) {
    cmd_error (cmd, "expecting address");
    return;
  }

  cmd_match_uint16 (cmd, &pass);
  cmd_match_uint16 (cmd, &reset);

  if (!cmd_match_end (cmd)) {
    return;
  }

  if (pass > 0) {
    printf ("Breakpoint at %04X  p=%04X  r=%04X\n",
      addr, pass, reset
    );

    bp_add (&breakpoint, addr, 0, pass, reset);
  }
}

static
void do_b (cmd_t *cmd, sim6502_t *sim)
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
    cmd_error (cmd, "b: unknown command (%s)");
  }
}

static
void do_c (cmd_t *cmd, sim6502_t *sim)
{
  unsigned short cnt;

  cnt = 1;

  cmd_match_uint16 (cmd, &cnt);

  if (!cmd_match_end (cmd)) {
    return;
  }

  while (cnt > 0) {
    s6502_clock (sim, 1);
    cnt -= 1;
  }

  prt_state_default (sim, stdout);
}

static
void do_dump (cmd_t *cmd, sim6502_t *sim)
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
    prt_error ("dump: can't open file (%s)\n", fname);
    return;
  }

  if (strcmp (what, "ram") == 0) {
    fprintf (fp, "# RAM dump\n\n");
    pce_dump_hex (fp, sim->ram->data, sim->ram->size, 0, 16, "", 1);
  }
  else if (strcmp (what, "config") == 0) {
    if (ini_write_fp (par_cfg, fp)) {
      prt_error ("dumping configuration failed\n");
    }
  }
  else {
    prt_error ("dump: don't know what to dump (%s)\n", what);
  }

  fclose (fp);
}

static
void do_d (cmd_t *cmd, sim6502_t *sim)
{
  unsigned short        i, j;
  unsigned short        cnt;
  unsigned short        addr1, addr2;
  static int            first = 1;
  static unsigned short saddr = 0;
  unsigned short        p, p1, p2;
  char                  buf[256];

  if (first) {
    first = 0;
    saddr = e6502_get_pc (sim->cpu);
  }

  addr1 = saddr;
  cnt = 256;

  if (cmd_match_uint16 (cmd, &addr1)) {
    cmd_match_uint16 (cmd, &cnt);
  }

  if (!cmd_match_end (cmd)) {
    return;
  }

  addr2 = (addr1 + cnt - 1) & 0xffffU;
  if (addr2 < addr1) {
    addr2 = 0xffffU;
    cnt = addr2 - addr1 + 1;
  }

  saddr = addr1 + cnt;

  p1 = addr1 / 16;
  p2 = addr2 / 16 + 1;

  for (p = p1; p < p2; p++) {
    j = 16 * p;

    sprintf (buf,
      "%04X  xx xx xx xx xx xx xx xx-xx xx xx xx xx xx xx xx  xxxxxxxxxxxxxxxx\n",
      j
    );

    for (i = 0; i < 16; i++) {
      if ((j >= addr1) && (j <= addr2)) {
        unsigned char val;
        unsigned      val1, val2;

        val = e6502_get_mem8 (sim->cpu, j);
        val1 = (val >> 4) & 0x0f;
        val2 = val & 0x0f;

        buf[6 + 3 * i + 0] = (val1 < 10) ? ('0' + val1) : ('A' + val1 - 10);
        buf[6 + 3 * i + 1] = (val2 < 10) ? ('0' + val2) : ('A' + val2 - 10);

        if ((val >= 32) && (val <= 127)) {
          buf[55 + i] = val;
        }
        else {
          buf[55 + i] = '.';
        }
      }
      else {
        buf[6 + 3 * i] = ' ';
        buf[6 + 3 * i + 1] = ' ';
        buf[55 + i] = ' ';
      }

      j += 1;
    }

    fputs (buf, stdout);
  }
}

static
void do_e (cmd_t *cmd, sim6502_t *sim)
{
  unsigned short addr, val;

  addr = 0;

  if (!cmd_match_uint16 (cmd, &addr)) {
    cmd_error (cmd, "need an address");
    return;
  }

  while (cmd_match_uint16 (cmd, &val)) {
    e6502_set_mem8 (sim->cpu, addr, val);

    addr += 1;
  }
}

static
void do_g (cmd_t *cmd, sim6502_t *sim)
{
  int            run;
  unsigned short addr;

  if (cmd_match (cmd, "b")) {
    run = 0;
  }
  else {
    run = 1;
  }

  if (cmd_match_uint16 (cmd, &addr)) {
    bp_add (&breakpoint, addr, 0, 1, 0);
    run = 0;
  }

  if (!cmd_match_end (cmd)) {
    return;
  }

  if (run) {
    pce_run (sim);
    return;
  }

  s6502_run_bp (sim);

  fputs ("\n", stdout);
  prt_state_default (sim, stdout);
}

static
void do_h (cmd_t *cmd, sim6502_t *sim)
{
  fputs (
    "bc [addr]                 clear a breakpoint or all\n"
    "bl                        list breakpoints\n"
    "bs addr [pass [reset]]    set a breakpoint [pass=1 reset=0]\n"
    "c [cnt]                   clock\n"
    "d [addr [cnt]]            dump memory\n"
    "e addr [val...]           enter bytes into memory\n"
    "g [b]                     run with or without breakpoints (ESC to stop)\n"
    "p [cnt]                   execute cnt instructions, skip calls [1]\n"
    "q                         quit\n"
    "r reg [val]               set a register\n"
    "s [what]                  print status (ppc|cth|clp|dpp|epc)\n"
    "t [cnt]                   execute cnt instructions [1]\n"
    "u [addr [cnt]]            disassemble\n",
    stdout
  );
}

static
void do_p (cmd_t *cmd, sim6502_t *sim)
{
  unsigned short cnt;
  e6502_disasm_t dis;

  cnt = 1;

  cmd_match_uint16 (cmd, &cnt);

  if (!cmd_match_end (cmd)) {
    return;
  }

  pce_start();

  while (cnt > 0) {
    e6502_disasm_cur (sim->cpu, &dis);

    if (dis.flags & E6502_OPF_JSR) {
      if (s6502_exec_to (sim, dis.pc + dis.dat_n)) {
        break;
      }
    }
    else {
      if (s6502_exec_off (sim, dis.pc)) {
        break;
      }
    }

    cnt -= 1;
  }

  pce_stop();

  prt_state_default (sim, stdout);
}

static
void do_r (cmd_t *cmd, sim6502_t *sim)
{
  unsigned char  *reg8;
  unsigned short *reg16;
  unsigned short val;

  if (s6502_match_reg16 (cmd, sim, &reg16)) {
    if (cmd_match_eol (cmd)) {
      printf ("%04x\n", *reg16);
      return;
    }

    if (!cmd_match_uint16 (cmd, &val)) {
      prt_error ("missing value\n");
      return;
    }

    *reg16 = val;
  }
  else if (s6502_match_reg8 (cmd, sim, &reg8)) {
    if (cmd_match_eol (cmd)) {
      printf ("%02x\n", *reg8);
      return;
    }

    if (!cmd_match_uint16 (cmd, &val)) {
      prt_error ("missing value\n");
      return;
    }

    *reg8 = (val & 0xff);
  }

  if (!cmd_match_end (cmd)) {
    return;
  }

  prt_state_default (sim, stdout);
}

static
void do_s (cmd_t *cmd, sim6502_t *sim)
{
  if (cmd_match_eol (cmd)) {
    prt_state_default (sim, stdout);
    return;
  }

  prt_state (sim, stdout, cmd_get_str (cmd));
}

static
void do_t (cmd_t *cmd, sim6502_t *sim)
{
  unsigned short i, n;

  n = 1;

  cmd_match_uint16 (cmd, &n);

  if (!cmd_match_end (cmd)) {
    return;
  }

  pce_start();

  for (i = 0; i < n; i++) {
    s6502_exec (sim);
  }

  pce_stop();

  prt_state_default (sim, stdout);
}

static
void do_u (cmd_t *cmd, sim6502_t *sim)
{
  unsigned              i;
  unsigned short        addr, cnt;
  static unsigned int   first = 1;
  static unsigned short saddr = 0;
  e6502_disasm_t        op;
  char                  str[256];

  if (first) {
    first = 0;
    saddr = e6502_get_pc (sim->cpu);
  }

  addr = saddr;
  cnt = 16;

  if (cmd_match_uint16 (cmd, &addr)) {
    cmd_match_uint16 (cmd, &cnt);
  }

  if (!cmd_match_end (cmd)) {
    return;
  }

  for (i = 0; i < cnt; i++) {
    e6502_disasm_mem (sim->cpu, &op, addr);
    s6502_disasm_str (str, &op);

    fprintf (stdout, "%s\n", str);

    addr += op.dat_n;
  }

  saddr = addr;
}

int do_cmd (sim6502_t *sim)
{
  cmd_t cmd;

  while (1) {
    prt_prompt (sim, stdout);
    fflush (stdout);

    cmd_get (&cmd);

    if (cmd_match_eol (&cmd)) {
      ;
    }
    else if (cmd_match (&cmd, "q")) {
      break;
    }
    else if (cmd_match (&cmd, "b")) {
      do_b (&cmd, sim);
    }
    else if (cmd_match (&cmd, "c")) {
      do_c (&cmd, sim);
    }
    else if (cmd_match (&cmd, "dump")) {
      do_dump (&cmd, sim);
    }
    else if (cmd_match (&cmd, "d")) {
      do_d (&cmd, sim);
    }
    else if (cmd_match (&cmd, "e")) {
      do_e (&cmd, sim);
    }
    else if (cmd_match (&cmd, "g")) {
      do_g (&cmd, sim);
    }
    else if (cmd_match (&cmd, "h")) {
      do_h (&cmd, sim);
    }
    else if (cmd_match (&cmd, "p")) {
      do_p (&cmd, sim);
    }
    else if (cmd_match (&cmd, "r")) {
      do_r (&cmd, sim);
    }
    else if (cmd_match (&cmd, "s")) {
      do_s (&cmd, sim);
    }
    else if (cmd_match (&cmd, "t")) {
      do_t (&cmd, sim);
    }
    else if (cmd_match (&cmd, "u")) {
      do_u (&cmd, sim);
    }
    else {
      printf ("unknown command (%s)\n", cmd_get_str (&cmd));
    }

    if (sim->brk == PCE_BRK_ABORT) {
      break;
    }
  };

  return (0);
}

static
ini_sct_t *pce_load_config (const char *fname)
{
  ini_sct_t *ini;

  if (fname != NULL) {
    ini = ini_read (fname);
    if (ini != NULL) {
      pce_log (MSG_INF, "pce:\tusing config file '%s'\n", fname);
      return (ini);
    }
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
    "pce sim6502 version " PCE_VERSION_STR
    " (compiled " PCE_CFG_DATE " " PCE_CFG_TIME ")\n"
    "Copyright (C) 1995-2004 Hampa Hug <hampa@hampa.ch>\n"
  );

  ini = pce_load_config (cfg);
  if (ini == NULL) {
    pce_log (MSG_ERR, "loading config file failed\n");
    return (1);
  }

  par_cfg = ini;

  sct = ini_sct_find_sct (ini, "sim6502");
  if (sct == NULL) {
    pce_log (MSG_ERR, "section 'sim6502' not found in config file\n");
    return (1);
  }

  par_sim = s6502_new (sct);

  par_sim->cpu->op_ext = par_sim;
  par_sim->cpu->op_undef = &s6502_log_undef;

  signal (SIGINT, &sig_int);
  signal (SIGSEGV, &sig_segv);

  cmd_init (stdin, stdout, &cmd_match_sym);

  s6502_reset (par_sim);

  if (run) {
    pce_run (par_sim);
    if (par_sim->brk != 2) {
      fputs ("\n", stdout);
      do_cmd (par_sim);
    }
  }
  else {
    pce_log (MSG_INF, "type 'h' for help\n");
    do_cmd (par_sim);
  }

  s6502_del (par_sim);

  return (0);
}
