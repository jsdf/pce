/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     pce.c                                                      *
 * Created:       1999-04-16 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: pce.c,v 1.1 2003/04/15 04:03:56 hampa Exp $ */


#include <stdio.h>

#include "pce.h"
#include "ibmpc.h"


typedef struct {
  unsigned i;
  unsigned n;
  char     str[256];
  char     cur[256];
  short    err;
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


char *str_ltrim (char *str)
{
  unsigned i, j;

  i = 0;
  j = 0;

  while (str[i] != 0) {
    if ((str[i] == ' ') || (str[i] == '\t') || (str[i] == '\n')) {
      i++;
    }
    else {
      strcpy (str, str + i);
      return (str);
    }
  }

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

int cmd_is_space (char c)
{
  if (c == ' ') {
    return (1);
  }

  if (c == '\n') {
    return (1);
  }

  if (c == '\t') {
    return (1);
  }

  return (0);
}

void cmd_get (cmd_t *cmd)
{
  unsigned i, j, k;
  short    f;

  fgets (cmd->str, 256, stdin);

  i = 0;
  j = 0;
  k = 0;
  f = 0;

  while (cmd->str[i] != 0) {
    if (cmd_is_space (cmd->str[i])) {
      if (f == 0) {
        cmd->str[j] = ' ';
        j++;
      }
      f = 1;
    }
    else {
      cmd->str[j] = cmd->str[i];
      j++;
      k = j;
      f = 0;
    }
    i++;
  }

  cmd->i = 0;
  cmd->n = k;
  cmd->str[k] = 0;
  cmd->cur[0] = 0;
}

void cmd_get_str (cmd_t *cmd)
{
  unsigned i, j;

  i = cmd->i;
  j = 0;

  while ((i < cmd->n) && cmd_is_space (cmd->str[i])) {
    i++;
  }

  while ((i < cmd->n) && !cmd_is_space (cmd->str[i])) {
    cmd->cur[j] = cmd->str[i];
    i++;
    j++;
  }

  cmd->i = i;
  cmd->cur[j] = 0;
}

unsigned cmd_get_uint16 (char *str)
{
  unsigned i, r;

  i = 0;
  r = 0;

  while (str[i] != 0) {
    if ((str[i] >= '0') && (str[i] <= '9')) {
      r = 16 * r + (unsigned)(str[i] - '0');
    }
    else if ((str[i] >= 'a') && (str[i] <= 'f')) {
      r = 16 * r + (unsigned)(str[i] - 'a' + 10);
    }
    else if ((str[i] >= 'A') && (str[i] <= 'F')) {
      r = 16 * r + (unsigned)(str[i] - 'A' + 10);
    }
    else {
      return (r);
    }

    i++;
  }

  return (r);
}

void cmd_get_addr (char *str, unsigned short *seg, unsigned short *ofs)
{
  unsigned i, s, o;

  i = 0;
  s = *seg;
  o = 0;

  while (str[i] != 0) {
    if ((str[i] >= '0') && (str[i] <= '9')) {
      o = 16 * o + (unsigned)(str[i] - '0');
    }
    else if ((str[i] >= 'a') && (str[i] <= 'f')) {
      o = 16 * o + (unsigned)(str[i] - 'a' + 10);
    }
    else if ((str[i] >= 'A') && (str[i] <= 'F')) {
      o = 16 * o + (unsigned)(str[i] - 'A' + 10);
    }
    else if (str[i] == ':') {
      s = o;
      o = 0;
    }
    else {
      *seg = s;
      *ofs = o;
      return;
    }

    i++;
  }

  *seg = s;
  *ofs = o;
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

void prt_prompt (FILE *fp)
{
  e86_disasm_t op;
  char         str[256];

  e86_disasm_cur (pc->cpu, &op);
  disasm_str (str, &op);

  fprintf (fp, "%04X:%04X(%lu) %s\n-",
    (unsigned) pc->cpu->sreg[E86_REG_CS],
    (unsigned) pc->cpu->ip,
    pc->clk_cnt,
    str
  );
}

unsigned parse_uint16 (char *str, unsigned short *val)
{
  unsigned i;

  i = 0;
  *val = 0;

  while (str[i] != 0) {
    if ((str[i] >= '0') && (str[i] <= '9')) {
      *val = 16 * *val + (unsigned short)(str[i] - '0');
    }
    else if ((str[i] >= 'a') && (str[i] <= 'f')) {
      *val = 16 * *val + (unsigned short)(str[i] - 'a' + 10);
    }
    else if ((str[i] >= 'A') && (str[i] <= 'F')) {
      *val = 16 * *val + (unsigned short)(str[i] - 'A' + 10);
    }
    else {
      return (i);
    }

    i++;
  }

  return (i);
}

unsigned parse_addr (char *str, unsigned short *seg, unsigned short *ofs)
{
  unsigned i, j;

  i = parse_uint16 (str, ofs);
  if (i == 0) {
    printf ("Syntax error\n");
    return (0);
  }

  if (str[i] == ':') {
    i++;
    *seg = *ofs;
    j = parse_uint16 (str + i, ofs);
    if (j == 0) {
      printf ("Syntax error\n");
      return (0);
    }
    i += j;
  }

  return (i);
}

void do_dump (cmd_t *cmd)
{
  FILE *fp;

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

  pc_prt_state (pc, stdout);
}

void do_bl (cmd_t *cmd)
{
  bp_list ();
}

void do_bs (cmd_t *cmd)
{
  unsigned short seg, ofs, pass;

  cmd_get_str (cmd);

  seg = pc->cpu->sreg[E86_REG_CS];
  ofs = 0;
  cmd_get_addr (cmd->cur, &seg, &ofs);

  cmd_get_str (cmd);

  if (cmd->cur[0] != 0) {
    pass = (unsigned short) cmd_get_uint16 (cmd->cur);
  }
  else {
    pass = 1;
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
  cmd_get_str (cmd);

  if (strcmp (cmd->cur, "l") == 0) {
    do_bl (cmd);
  }
  else if (strcmp (cmd->cur, "g") == 0) {
    do_bg (cmd);
  }
  else if (strcmp (cmd->cur, "s") == 0) {
    do_bs (cmd);
  }
  else {
    printf ("b: unknown command (%s)\n", cmd->cur);
  }
}

void do_g (cmd_t *cmd)
{
  unsigned long cnt;

  cmd_get_str (cmd);

  if (cmd->cur[0] == 0) {
    cnt = 1;
  }
  else {
    cnt = cmd_get_uint16 (cmd->cur);
  }

  cnt += pc->cpu->instructions;

  while (pc->cpu->instructions < cnt) {
    pc_clock (pc);
  }

  pc_prt_state (pc, stdout);
}

void do_c (cmd_t *cmd)
{
  pc_clock (pc);
  pc_prt_state (pc, stdout);
}

void do_s (cmd_t *cmd)
{
  cmd_get_str (cmd);

  if (cmd->cur[0] == 0) {
    pc_prt_state (pc, stdout);
    return;
  }

  while (cmd->cur[0] != 0) {
    if (strcmp (cmd->cur, "pc") == 0) {
      pc_prt_state (pc, stdout);
    }
    else if (strcmp (cmd->cur, "cpu") == 0) {
      e86_prt_state (pc->cpu, stdout);
    }
    else {
      printf ("unknown component (%s)\n", cmd->cur);
    }

    cmd_get_str (cmd);
  }
}

void do_u (cmd_t *cmd)
{
  unsigned       i, n;
  unsigned short seg, ofs;
  e86_disasm_t   op;
  char           str[256];

  cmd_get_str (cmd);

  seg = pc->cpu->sreg[E86_REG_CS];
  ofs = pc->cpu->ip;
  n = 16;

  if (cmd->cur[0] != 0) {
    cmd_get_addr (cmd->cur, &seg, &ofs);

    cmd_get_str (cmd);
    if (cmd->cur[0] != 0) {
      n = cmd_get_uint16 (cmd->cur);
    }
  }

  for (i = 0; i < n; i++) {
    e86_disasm_mem (pc->cpu, &op, seg, ofs);
    disasm_str (str, &op);

    fprintf (stdout, "%04X:%04X  %s\n", seg, ofs, str);

    ofs = (ofs + op.dat_n) & 0xffff;
  }
}

void do_d (cmd_t *cmd)
{
  unsigned       i, n;
  unsigned short seg, ofs;
  unsigned char  val;

  cmd_get_str (cmd);

  seg = pc->cpu->sreg[E86_REG_DS];
  ofs = 0;
  n = 256;

  if (cmd->cur[0] != 0) {
    cmd_get_addr (cmd->cur, &seg, &ofs);

    cmd_get_str (cmd);
    if (cmd->cur[0] != 0) {
      n = cmd_get_uint16 (cmd->cur);
    }
  }

  for (i = 0; i < n; i++) {
    if ((i & 15) == 0) {
      if (i > 0) {
        fputs ("\n", stdout);
      }
      fprintf (stdout, "%04X:%04X  ", seg, ofs);
    }

    val = mem_get_uint8 (pc->mem, (seg << 4) + ofs);

    fprintf (stdout, " %02X", val);

    ofs = (ofs + 1) & 0xffff;
  }

  fputs ("\n", stdout);
}

unsigned short *pc_get_reg (const char *reg)
{
  unsigned i;

  static char *dreg[8] = {
    "ax", "cx", "dx", "bx", "sp", "bp", "si", "di"
  };

  static char *sreg[4] = {
    "es", "cs", "ss", "ds"
  };

  for (i = 0; i < 8; i++) {
    if (strcmp (dreg[i], reg) == 0) {
      return (&pc->cpu->dreg[i]);
    }
  }

  for (i = 0; i < 4; i++) {
    if (strcmp (sreg[i], reg) == 0) {
      return (&pc->cpu->sreg[i]);
    }
  }

  if (strcmp (reg, "ip") == 0) {
    return (&pc->cpu->ip);
  }

  if (strcmp (reg, "flags") == 0) {
    return (&pc->cpu->flg);
  }

  return (0);
}

void do_r (cmd_t *cmd)
{
  unsigned       val;
  unsigned short *reg;

  cmd_get_str (cmd);

  reg = pc_get_reg (cmd->cur);

  if (reg == NULL) {
    printf ("unknown register (%s)\n", cmd->cur);
    return;
  }

  cmd_get_str (cmd);
  if (cmd->cur[0] == 0) {
    printf ("missing value\n");
    return;
  }

  val = cmd_get_uint16 (cmd->cur);

  *reg = val;
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
    cmd_get_str (&cmd);

    if (strcmp (cmd.cur, "b") == 0) {
      do_b (&cmd);
    }
    else if (strcmp (cmd.cur, "c") == 0) {
      do_c (&cmd);
    }
    else if (strcmp (cmd.cur, "g") == 0) {
      do_g (&cmd);
    }
    else if (strcmp (cmd.str, "q") == 0) {
      break;
    }
    else if (strcmp (cmd.cur, "s") == 0) {
      do_s (&cmd);
    }
    else if (strcmp (cmd.cur, "u") == 0) {
      do_u (&cmd);
    }
    else if (strcmp (cmd.cur, "d") == 0) {
      do_d (&cmd);
    }
    else if (strcmp (cmd.cur, "r") == 0) {
      do_r (&cmd);
    }
    else if (strcmp (cmd.cur, "dump") == 0) {
      do_dump (&cmd);
    }
    else if (cmd.cur[0] == 0) {
      ;
    }
    else {
      printf ("unknown command (%s)\n", cmd.str);
    }
  };

  return (0);
}
