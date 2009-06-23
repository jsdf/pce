/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/ibmpc/cassette.h                                         *
 * Created:     2008-11-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2008-2009 Hampa Hug <hampa@hampa.ch>                     *
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


typedef struct {
	char          save;

	unsigned char motor;

	unsigned long position;

	unsigned long position_save;
	unsigned long position_load;

	unsigned char data_out;
	unsigned char data_inp;

	unsigned long clk;

	unsigned long clk_out;
	unsigned long clk_inp;

	unsigned      out_cnt;
	unsigned char out_buf;

	unsigned      inp_cnt;
	unsigned char inp_buf;
	unsigned char inp_bit;

	char          close;
	FILE          *fp;
} pc_cassette_t;


void pc_cas_init (pc_cassette_t *cas);
void pc_cas_free (pc_cassette_t *cas);

pc_cassette_t *pc_cas_new (void);
void pc_cas_del (pc_cassette_t *cas);

/*!***************************************************************************
 * @short Get the cassette mode
 * @return True if in save mode, false if in load mode
 *****************************************************************************/
int pc_cas_get_mode (pc_cassette_t *cas);

/*!***************************************************************************
 * @short Set the cassette mode
 * @param save If true set save mode, otherwise set load mode
 *****************************************************************************/
void pc_cas_set_mode (pc_cassette_t *cas, int save);

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

int pc_cas_set_fname (pc_cassette_t *cas, const char *fname);

void pc_cas_set_motor (pc_cassette_t *cas, unsigned char val);

unsigned char pc_cas_get_inp (pc_cassette_t *cas);

void pc_cas_set_out (pc_cassette_t *cas, unsigned char val);

void pc_cas_print_state (pc_cassette_t *cas);

void pc_cas_clock (pc_cassette_t *cas, unsigned long cnt);


#endif
