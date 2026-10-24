/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/cassette.h                                       *
 * Created:     2020-04-30 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_DEVICES_CASSETTE_H
#define PCE_DEVICES_CASSETTE_H 1


#include <drivers/pti/pti.h>


typedef struct {
	void          *set_inp_ext;
	void          (*set_inp) (void *ext, unsigned char val);

	void          *set_play_ext;
	void          (*set_play) (void *ext, unsigned char val);

	void          *set_run_ext;
	void          (*set_run) (void *ext, unsigned char val);

	pti_img_t     *read_img;
	char          *read_name;

	pti_img_t     *write_img;
	char          *write_name;

	char          modified;
	char          eof;

	char          run;
	char          motor;
	char          play;
	char          record;

	char          auto_play;
	char          auto_motor;

	char          out_val;
	char          inp_val;

	unsigned long clock;
	unsigned long remainder;
	unsigned long position;

	unsigned long motor_delay;
	unsigned long motor_delay_count;

	unsigned long counter;
} cassette_t;


int cas_set_msg (cassette_t *cas, const char *msg, const char *val);

void cas_init (cassette_t *cas);
void cas_free (cassette_t *cas);

cassette_t *cas_new (void);
void cas_del (cassette_t *cas);

void cas_set_inp_fct (cassette_t *cas, void *ext, void *fct);
void cas_set_play_fct (cassette_t *cas, void *ext, void *fct);
void cas_set_run_fct (cassette_t *cas, void *ext, void *fct);

void cas_set_clock (cassette_t *cas, unsigned long val);

void cas_set_auto_play (cassette_t *cas, int val);
void cas_set_auto_motor (cassette_t *cas, int val);
void cas_set_motor_delay (cassette_t *cas, unsigned long val);

int cas_set_read_name (cassette_t *cas, const char *fname);
int cas_set_write_name (cassette_t *cas, const char *fname, int create);

int cas_commit (cassette_t *cas);

int cas_get_position (const cassette_t *cas, double *tm);
int cas_set_position (cassette_t *cas, double tm);

int cas_truncate (cassette_t *cas, double tm);
int cas_space (cassette_t *cas, double tm);

void cas_set_play (cassette_t *cas, int val);
void cas_set_record (cassette_t *cas, int val);
void cas_press_keys (cassette_t *cas, int play, int record);

void cas_print_state (cassette_t *cas);

void cas_set_motor (cassette_t *cas, int val);

int cas_get_inp (const cassette_t *cas);

void cas_set_out (cassette_t *cas, int val);

void cas_clock (cassette_t *cas);


#endif
