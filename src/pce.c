/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     pce.c                                                      *
 * Created:       1999-04-16 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-15 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: pce.c,v 1.2 2003/04/16 02:26:39 hampa Exp $ */


#include <stdio.h>
#include <stdarg.h>

#include "pce.h"
#include "ibmpc.h"


typedef struct {
  unsigned i;
  char     str[256];
} cmd_t;


typedef struct breakpoint_t {
  struct breakpoint_t *next;
  unsigned short      seg;
  unsigned short      ofs;
  unsigned            pass;
} breakpoint_t;


static unsigned     bp_cnt = 0;
static breakpoint_t *breakpoint = NULL;
static ibmpc_t      *pc;


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
}

void cmd_match_space (cmd_t *cmd)
{
  unsigned i;

  i = cmd->i;

  while ((cmd->str[i] != 0) && str_is_space (cmd->str[i])) {
    i++;
  }

  cmd->i = i;
}

void cmd_error (cmd_t *cmd, const char *str)
{
  printf ("** %s [%s]\n", str, cmd->str + cmd->i);
  fflush (stdout);
}

void cmd_match_str (cmd_t *cmd, char *str)
{
  unsigned i;

  cmd_match_space (cmd);

  i = cmd->i;

  while ((cmd->str[i] != 0) && !str_is_space (cmd->str[i])) {
    *(str++) = cmd->str[i];
    i++;
  }

  *str = 0;

  cmd->i = i;
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

void bp_add (unsigned short seg, unsigned short ofs, unsigned pass)
{
  breakpoint_t *bp;

  bp = bp_get (seg, ofs);

  if (bp != NULL) {
    bp->pass = pass;
    return;
  }

  bp = (breakpoint_t *) malloc (sizeof (breakpoint_t));
  if (bp == NULL) return;

  bp->next = breakpoint;
  bp->seg = seg;
  bp->ofs = ofs;
  bp->pass = pass;
  breakpoint = bp;

  bp_cnt++;
}

void bp_clear (unsigned short seg, unsigned short ofs)
{
  breakpoint_t *bp1, *bp2;

  bp1 = breakpoint;
  if (bp1 == NULL) return;

  if ((bp1->seg == seg) && (bp1->ofs == ofs)) {
    breakpoint = bp1->next;
    free (bp1);
    bp_cnt--;
    return;
  }

  bp2 = bp1->next;
  while (bp2 != NULL) {
    if ((bp2->seg == seg) && (bp2->ofs == ofs)) {
      bp1->next = bp2->next;
      free (bp2);
      bp_cnt--;
      return;
    }

    bp1 = bp2;
    bp2 = bp2->next;
  }
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

void bp_list (void)
{
  breakpoint_t *bp;

  bp = breakpoint;

  if (bp == NULL) {
    printf ("No breakpoints defined\n");
  }
  else {
    printf ("%u breakpoints:\n", bp_cnt);
  }

  while (bp != NULL) {
    printf ("  %04X:%04X (%u)\n",
      (unsigned)bp->seg,
      (unsigned)bp->ofs,
      bp->pass
    );
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

void prt_state (ibmpc_t *pc, FILE *fp)
{
  e86_disasm_t op;
  char         str[256];

  e86_disasm_cur (pc->cpu, &op);
  disasm_str (str, &op);

  fprintf (fp, "-8086-state------------------------------------------------------------------\n");

  e86_prt_state (pc->cpu, fp);

  fprintf (fp, "%04X:%04X  %s\n",
    (unsigned) pc->cpu->sreg[E86_REG_CS],
    (unsigned) pc->cpu->ip,
    str
  );
}

void prt_prompt (FILE *fp)
{
  fputs ("-", fp);
  fflush (fp);
}

void prt_error (const char *str, ...)
{
  va_list va;

  va_start (va, str);
  vfprintf (stderr, str, va);
  va_end (va);
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

void do_bg (cmd_t *cmd)
{
  breakpoint_t   *bp;
  unsigned short seg, ofs;
  unsigned long  old;

  if (!cmd_match_end (cmd)) {
    return;
  }

  key_set_fd (pc->key, 0);

  do {
    old = pc->cpu->instructions;
    while (pc->cpu->instructions == old) {
      pc_clock (pc);
    }

    seg = pc->cpu->sreg[E86_REG_CS];
    ofs = pc->cpu->ip;

    bp = bp_get (seg, ofs);

    if (bp != NULL) {
      if (bp->pass > 0) {
        bp->pass--;
      }
      if (bp->pass == 0) {
        bp_clear (seg, ofs);
      }
      else {
        bp = NULL;
      }
    }
  } while (bp == NULL);

  key_set_fd (pc->key, -1);

  prt_state (pc, stdout);
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
  unsigned short seg, ofs, pass;

  seg = pc->cpu->sreg[E86_REG_CS];
  ofs = 0;
  pass = 1;

  if (!cmd_match_addr (cmd, &seg, &ofs)) {
    cmd_error (cmd, "expecting address");
    return;
  }

  cmd_match_uint16 (cmd, &pass);

  if (!cmd_match_end (cmd)) {
    return;
  }

  if (pass > 0) {
    printf ("Breakpoint at %04X:%04X (%u)\n",
      (unsigned)seg,
      (unsigned)ofs,
      pass
    );

    bp_add (seg, ofs, pass);
  }
}

void do_b (cmd_t *cmd)
{
  if (cmd_match (cmd, "l")) {
    do_bl (cmd);
  }
  else if (cmd_match (cmd, "g")) {
    do_bg (cmd);
  }
  else if (cmd_match (cmd, "s")) {
    do_bs (cmd);
  }
  else {
    prt_error ("b: unknown command (%s)\n", cmd->str + cmd->i);
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

  key_set_fd (pc->key, 0);

  while ((e86_get_cs (pc->cpu) == seg) && (e86_get_ip (pc->cpu) == ofs)) {
    pc_clock (pc);
  }

  while ((e86_get_cs (pc->cpu) != seg) || (e86_get_ip (pc->cpu) != ofs)) {
    pc_clock (pc);
  }

  key_set_fd (pc->key, -1);

  prt_state (pc, stdout);
}

void do_g (cmd_t *cmd)
{
  unsigned short cnt;
  unsigned long  inst;

  cnt = 1;

  cmd_match_uint16 (cmd, &cnt);

  if (!cmd_match_end (cmd)) {
    return;
  }

  inst = pc->cpu->instructions + cnt;

  while (pc->cpu->instructions < inst) {
    pc_clock (pc);
  }

  prt_state (pc, stdout);
}

void do_c (cmd_t *cmd)
{
  unsigned short cnt;

  cnt = 1;

  cmd_match_uint16 (cmd, &cnt);

  if (!cmd_match_end (cmd)) {
    return;
  }

  while (cnt > 0) {
    pc_clock (pc);
    cnt -= 1;
  }

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
      pc_prt_state (pc, stdout);
    }
    else if (cmd_match (cmd, "cpu")) {
      e86_prt_state (pc->cpu, stdout);
    }
    else {
      prt_error ("unknown component (%s)\n", cmd->str + cmd->i);
      return;
    }
  }
}

void do_u (cmd_t *cmd)
{
  unsigned       i;
  unsigned short seg, ofs, cnt;
  e86_disasm_t   op;
  char           str[256];

  seg = pc->cpu->sreg[E86_REG_CS];
  ofs = pc->cpu->ip;
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
}

void do_d (cmd_t *cmd)
{
  unsigned              i, j;
  unsigned short        cnt;
  unsigned short        seg, ofs1, ofs2;
  static unsigned short sseg = 0;
  static unsigned short sofs = 0;
  unsigned short        p, p1, p2;
  char                  buf[256];

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

int main (int argc, char *argv[])
{
  cmd_t cmd;

  printf ("8086 CPU emulator by Hampa '99\n\n");

  pc = pc_new ();
  e86_reset (pc->cpu);

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
    else if (cmd_match (&cmd, "g")) {
      do_g (&cmd);
    }
    else if (cmd_match (&cmd, "p")) {
      do_p (&cmd);
    }
    else if (cmd_match (&cmd, "q")) {
      break;
    }
    else if (cmd_match (&cmd, "s")) {
      do_s (&cmd);
    }
    else if (cmd_match (&cmd, "u")) {
      do_u (&cmd);
    }
    else if (cmd_match (&cmd, "d")) {
      do_d (&cmd);
    }
    else if (cmd_match (&cmd, "r")) {
      do_r (&cmd);
    }
    else if (cmd_match (&cmd, "dump")) {
      do_dump (&cmd);
    }
    else if (cmd.str[cmd.i] == 0) {
      ;
    }
    else {
      printf ("unknown command (%s)\n", cmd.str);
    }
  };

  return (0);
}
