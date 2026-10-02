/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/monitor.h                                            *
 * Created:     2006-12-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2006-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_LIB_MONITOR_H
#define PCE_LIB_MONITOR_H 1


#include <lib/cmd.h>


typedef struct {
	const char *cmd;
	const char *par;
	const char *text;
} mon_cmd_t;


typedef struct {
	void *cmdext;
	int  (*docmd) (void *ext, cmd_t *cmd);

	void *msgext;
	int  (*setmsg) (void *ext, const char *msg, const char *val);

	void          *get_mem8_ext;
	unsigned char (*get_mem8) (void *ext, unsigned long addr);

	void *set_mem8_ext;
	void (*set_mem8) (void *ext, unsigned long addr, unsigned char val);

	void *set_mem8rw_ext;
	void (*set_mem8rw) (void *ext, unsigned long addr, unsigned char val);

	unsigned       memory_mode;

	unsigned short default_seg;

	unsigned long  last_addr;
	unsigned short last_ofs;

	unsigned       cmd_cnt;
	mon_cmd_t      *cmd;

	char           rw;
	char           terminate;

	const char     *prompt;
} monitor_t;


void mon_init (monitor_t *mon);
void mon_free (monitor_t *mon);

monitor_t *mon_new (void);
void mon_del (monitor_t *mon);

void mon_set_cmd_fct (monitor_t *mon, void *fct, void *ext);
void mon_set_msg_fct (monitor_t *mon, void *fct, void *ext);

void mon_set_get_mem_fct (monitor_t *mon, void *ext, void *fct);
void mon_set_set_mem_fct (monitor_t *mon, void *ext, void *fct);
void mon_set_set_memrw_fct (monitor_t *mon, void *ext, void *fct);

void mon_set_memory_mode (monitor_t *mon, unsigned mode);

void mon_set_terminate (monitor_t *mon, int val);

void mon_set_prompt (monitor_t *mon, const char *str);

int mon_cmd_add (monitor_t *mon, const mon_cmd_t *cmd, unsigned cnt);
int mon_cmd_add_bp (monitor_t *mon);

int mon_run (monitor_t *mon);


#endif
