/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/ibmpc/cassette.h                                         *
 * Created:     2008-11-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2008-2010 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_IBMPC_CASSETTE_H
#define PCE_IBMPC_CASSETTE_H 1


#include <stdio.h>


typedef struct {
	long a[3];
	long b[3];
	long x[3];
	long y[3];
} pc_cas_iir2_t;


typedef struct {
	char          save;
	char          pcm;
	char          filter;

	unsigned char motor;

	unsigned long position;

	unsigned long position_save;
	unsigned long position_load;

	unsigned char data_out;
	unsigned char data_inp;

	int           pcm_out_vol;
	long          pcm_out_val;

	pc_cas_iir2_t pcm_out_iir;
	pc_cas_iir2_t pcm_inp_iir;

	unsigned      cas_out_cnt;
	unsigned char cas_out_buf;

	unsigned      cas_inp_cnt;
	unsigned char cas_inp_buf;
	unsigned char cas_inp_bit;

	unsigned long clk;

	unsigned long clk_pcm;

	unsigned long clk_out;
	unsigned long clk_inp;

	char          close;
	char          *fname;
	FILE          *fp;
} pc_cassette_t;


void pc_cas_init (pc_cassette_t *cas);
void pc_cas_free (pc_cassette_t *cas);

pc_cassette_t *pc_cas_new (void);
void pc_cas_del (pc_cassette_t *cas);

/*!***************************************************************************
 * @short  Set the cassette file
 * @return True on error, false otherwise
 *****************************************************************************/
int pc_cas_set_fname (pc_cassette_t *cas, const char *fname);

/*!***************************************************************************
 * @short  Get the cassette mode
 * @return True if in save mode, false if in load mode
 *****************************************************************************/
int pc_cas_get_mode (pc_cassette_t *cas);

/*!***************************************************************************
 * @short Set the cassette mode
 * @param save If true set save mode, otherwise set load mode
 *****************************************************************************/
void pc_cas_set_mode (pc_cassette_t *cas, int save);

/*!***************************************************************************
 * @short  Get the cassette pcm mode
 * @return True if in pcm mode, false if in binary mode
 *****************************************************************************/
int pc_cas_get_pcm (pc_cassette_t *cas);

/*!***************************************************************************
 * @short Set the cassette pcm mode
 * @param pcm If true set pcm mode, otherwise set binary mode
 *****************************************************************************/
void pc_cas_set_pcm (pc_cassette_t *cas, int pcm);

/*!***************************************************************************
 * @short  Get the cassette pcm filter mode
 * @return True if in filtered pcm mode, false if in raw pcm mode
 *****************************************************************************/
int pc_cas_get_filter (pc_cassette_t *cas);

/*!***************************************************************************
 * @short Set the cassette pcm filter mode
 * @param pcm If true set filtered pcm mode, otherwise set raw pcm mode
 *****************************************************************************/
void pc_cas_set_filter (pc_cassette_t *cas, int filter);

/*!***************************************************************************
 * @short Rewind the cassette
 *****************************************************************************/
void pc_cas_rewind (pc_cassette_t *cas);

/*!***************************************************************************
 * @short Fast forward to the end of the cassette
 *****************************************************************************/
void pc_cas_append (pc_cassette_t *cas);

/*!***************************************************************************
 * @short Get the current load/save position
 *****************************************************************************/
unsigned long pc_cas_get_position (pc_cassette_t *cas);

/*!***************************************************************************
 * @short Set the current load/save position
 *****************************************************************************/
int pc_cas_set_position (pc_cassette_t *cas, unsigned long pos);

void pc_cas_set_motor (pc_cassette_t *cas, unsigned char val);

unsigned char pc_cas_get_inp (pc_cassette_t *cas);

void pc_cas_set_out (pc_cassette_t *cas, unsigned char val);

void pc_cas_print_state (pc_cassette_t *cas);

void pc_cas_clock (pc_cassette_t *cas, unsigned long cnt);


#endif
