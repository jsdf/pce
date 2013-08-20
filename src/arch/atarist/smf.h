/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/smf.h                                       *
 * Created:     2013-06-23 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_ATARIST_SMF_H
#define PCE_ATARIST_SMF_H 1


#include <stdio.h>


typedef struct {
	FILE          *fp;
	char          *auto_name;
	unsigned      auto_index;

	unsigned long mthd_ofs;
	unsigned long mtrk_ofs;
	unsigned long mtrk_size;

	char          clock_inited;
	unsigned long clock;

	unsigned char status;
	unsigned long evt_clk;
	unsigned      buf_idx;
	unsigned      buf_cnt;
	unsigned char buf[4096];
} st_smf_t;


void st_smf_init (st_smf_t *smf);
void st_smf_free (st_smf_t *smf);

int st_smf_set_file (st_smf_t *smf, const char *fname);
int st_smf_set_auto (st_smf_t *smf, const char *fname);

void st_smf_set_uint8 (st_smf_t *smf, unsigned char val, unsigned long clk);


#endif
