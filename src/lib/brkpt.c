/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/lib/brkpt.c                                            *
 * Created:       2004-05-25 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-05-25 by Hampa Hug <hampa@hampa.ch>                   *
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


#include <stdlib.h>
#include <stdio.h>

#include "brkpt.h"


breakpoint_t *bp_get (breakpoint_t *lst, unsigned long addr1, unsigned long addr2)
{
  while (lst != NULL) {
    if ((lst->addr[0] == addr1) && (lst->addr[1] == addr2)) {
      return (lst);
    }

    lst = lst->next;
  }

  return (NULL);
}

void bp_add (breakpoint_t **lst, unsigned long addr1, unsigned long addr2,
  unsigned pass, unsigned reset)
{
  breakpoint_t *bp;

  bp = bp_get (*lst, addr1, addr2);

  if (bp != NULL) {
    bp->pass = pass;
    bp->reset = reset;
    return;
  }

  bp = (breakpoint_t *) malloc (sizeof (breakpoint_t));
  if (bp == NULL) {
    return;
  }

  bp->addr[0] = addr1;
  bp->addr[1] = addr2;
  bp->pass = pass;
  bp->reset = reset;

  bp->next = *lst;
  *lst = bp;
}

int bp_clear (breakpoint_t **lst, unsigned long addr1, unsigned long addr2)
{
  breakpoint_t *bp1, *bp2;

  bp1 = *lst;
  if (bp1 == NULL) {
    return (1);
  }

  if ((bp1->addr[0] == addr1) && (bp1->addr[1] == addr2)) {
    *lst = bp1->next;
    free (bp1);
    return (0);
  }

  bp2 = bp1->next;
  while (bp2 != NULL) {
    if ((bp2->addr[0] == addr1) && (bp2->addr[1] == addr2)) {
      bp1->next = bp2->next;
      free (bp2);
      return (0);
    }

    bp1 = bp2;
    bp2 = bp2->next;
  }

  return (1);
}

void bp_clear_all (breakpoint_t **lst)
{
  breakpoint_t *bp;

  while (*lst != NULL) {
    bp = (*lst)->next;
    free (*lst);
    *lst = bp;
  }
}

void bp_print (breakpoint_t *bp, const char *str)
{
  printf ("%s%08lX  %04X  %04X\n",
    str,
    (unsigned long) bp->addr[0],
    (unsigned) bp->pass, (unsigned) bp->reset
  );
}

void bp_list (breakpoint_t *lst)
{
  if (lst == NULL) {
    printf ("No breakpoints defined\n");
  }

  while (lst != NULL) {
    bp_print (lst, "  ");

    lst = lst->next;
  }
}

int bp_check (breakpoint_t **lst, unsigned long addr1, unsigned long addr2)
{
  breakpoint_t  *bp;

  bp = bp_get (*lst, addr1, addr2);

  if (bp != NULL) {
    if (bp->pass > 0) {
      bp->pass -= 1;
    }

    if (bp->pass == 0) {
      if (bp->reset == 0) {
        bp_clear (lst, addr1, addr2);
      }
      else {
        bp->pass = bp->reset;
        bp_print (bp, "brk: ");
      }

      return (1);
    }
  }

  return (0);
}
