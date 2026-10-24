/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/cassette.c                                       *
 * Created:     2020-04-30 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020-2021 Hampa Hug <hampa@hampa.ch>                     *
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


#include <config.h>

#include <devices/cassette.h>

#include <stdlib.h>
#include <string.h>

#include <drivers/pti/pti.h>
#include <drivers/pti/pti-io.h>

#include <lib/console.h>
#include <lib/msg.h>
#include <lib/string.h>


typedef struct {
	const char *msg;

	int (*set_msg) (cassette_t *cas, const char *msg, const char *val);
} cas_msg_list_t;


static
int cas_set_msg_commit (cassette_t *cas, const char *msg, const char *val)
{
	if (cas_commit (cas)) {
		return (1);
	}

	return (0);
}

static
int cas_set_msg_create (cassette_t *cas, const char *msg, const char *val)
{
	if (*val == 0) {
		return (1);
	}

	remove (val);

	if (cas_set_write_name (cas, val, 1)) {
		return (1);
	}

	if (cas->auto_play) {
		cas_press_keys (cas, 1, 1);
	}
	else {
		cas_press_keys (cas, 0, 0);
	}

	cas_print_state (cas);

	return (0);
}

static
int cas_set_msg_open (cassette_t *cas, const char *msg, const char *val)
{
	if (*val == 0) {
		val = NULL;
	}

	if (cas_set_read_name (cas, NULL)) {
		return (1);
	}

	if (cas_set_write_name (cas, val, 0)) {
		return (1);
	}

	cas_press_keys (cas, 0, 0);
	cas_print_state (cas);

	return (0);
}

static
int cas_set_msg_play (cassette_t *cas, const char *msg, const char *val)
{
	cas_press_keys (cas, 1, 0);
	cas_print_state (cas);

	return (0);
}

static
int cas_set_msg_load (cassette_t *cas, const char *msg, const char *val)
{
	double v;

	cas_press_keys (cas, 0, 0);

	if (*val != 0) {
		if (msg_get_double (val, &v)) {
			return (1);
		}

		cas_set_position (cas, v);
	}

	cas_print_state (cas);

	return (0);
}

static
int cas_set_msg_read (cassette_t *cas, const char *msg, const char *val)
{
	if (*val == 0) {
		val = NULL;
	}

	if (cas_set_read_name (cas, val)) {
		return (1);
	}

	cas_press_keys (cas, 0, 0);
	cas_print_state (cas);

	return (0);
}

static
int cas_set_msg_record (cassette_t *cas, const char *msg, const char *val)
{
	cas_press_keys (cas, 1, 1);
	cas_print_state (cas);

	return (0);
}

static
int cas_set_msg_space (cassette_t *cas, const char *msg, const char *val)
{
	double sec;

	if (*val != 0) {
		if (msg_get_double (val, &sec)) {
			return (1);
		}
	}
	else {
		sec = 1.0;
	}

	if (cas_space (cas, sec)) {
		return (1);
	}

	cas_print_state (cas);

	return (0);
}

static
int cas_set_msg_state (cassette_t *cas, const char *msg, const char *val)
{
	cas_print_state (cas);

	return (0);
}

static
int cas_set_msg_stop (cassette_t *cas, const char *msg, const char *val)
{
	cas_press_keys (cas, 0, 0);
	cas_print_state (cas);

	return (0);
}

static
int cas_set_msg_truncate (cassette_t *cas, const char *msg, const char *val)
{
	double v;

	if (*val == 0) {
		return (1);
	}

	if (msg_get_double (val, &v)) {
		return (1);
	}

	if (cas_truncate (cas, v)) {
		return (1);
	}

	cas_print_state (cas);

	return (0);
}

static
int cas_set_msg_write (cassette_t *cas, const char *msg, const char *val)
{
	if (*val == 0) {
		val = NULL;
	}

	if (cas_set_write_name (cas, val, 1)) {
		return (1);
	}

	if (cas->auto_play) {
		cas_press_keys (cas, 1, 1);
	}
	else {
		cas_press_keys (cas, 0, 0);
	}

	cas_print_state (cas);

	return (0);
}

static cas_msg_list_t set_msg_list[] = {
	{ "emu.cas.commit", cas_set_msg_commit },
	{ "emu.cas.create", cas_set_msg_create },
	{ "emu.cas.c", cas_set_msg_create },
	{ "emu.cas.open", cas_set_msg_open },
	{ "emu.cas.play", cas_set_msg_play },
	{ "emu.cas.load", cas_set_msg_load },
	{ "emu.cas.read", cas_set_msg_read },
	{ "emu.cas.record", cas_set_msg_record },
	{ "emu.cas.space", cas_set_msg_space },
	{ "emu.cas.state", cas_set_msg_state },
	{ "emu.cas.stop", cas_set_msg_stop },
	{ "emu.cas.truncate", cas_set_msg_truncate },
	{ "emu.cas.write", cas_set_msg_write },
	{ NULL, NULL }
};


int cas_set_msg (cassette_t *cas, const char *msg, const char *val)
{
	cas_msg_list_t *lst;

	if (val == NULL) {
		val = "";
	}

	lst = set_msg_list;

	while (lst->msg != NULL) {
		if (msg_is_message (lst->msg, msg)) {
			return (lst->set_msg (cas, msg, val));
		}

		lst += 1;
	}

	return (-1);
}


void cas_init (cassette_t *cas)
{
	cas->set_inp = NULL;
	cas->set_inp_ext = NULL;

	cas->set_play = NULL;
	cas->set_play_ext = NULL;

	cas->set_run = NULL;
	cas->set_run_ext = NULL;

	cas->read_img = NULL;
	cas->read_name = NULL;

	cas->write_img = NULL;
	cas->write_name = NULL;

	cas->modified = 0;
	cas->eof = 0;

	cas->run = 0;
	cas->motor = 0;
	cas->play = 0;
	cas->record = 0;

	cas->auto_play = 0;
	cas->auto_motor = 0;

	cas->out_val = 0;
	cas->inp_val = 0;

	cas->clock = 0;
	cas->remainder = 0;
	cas->position = 0;

	cas->motor_delay = 0;
	cas->motor_delay_count = 0;

	cas->counter = 0;
}

void cas_free (cassette_t *cas)
{
	cas_commit (cas);

	pti_img_del (cas->write_img);
	pti_img_del (cas->read_img);

	free (cas->write_name);
	free (cas->read_name);
}

cassette_t *cas_new (void)
{
	cassette_t *cas;

	if ((cas = malloc (sizeof (cassette_t))) == NULL) {
		return (NULL);
	}

	cas_init (cas);

	return (cas);
}

void cas_del (cassette_t *cas)
{
	if (cas != NULL) {
		cas_free (cas);
	}

	free (cas);
}

void cas_set_inp_fct (cassette_t *cas, void *ext, void *fct)
{
	cas->set_inp_ext = ext;
	cas->set_inp = fct;
}

void cas_set_play_fct (cassette_t *cas, void *ext, void *fct)
{
	cas->set_play_ext = ext;
	cas->set_play = fct;
}

void cas_set_run_fct (cassette_t *cas, void *ext, void *fct)
{
	cas->set_run_ext = ext;
	cas->set_run = fct;
}

void cas_set_clock (cassette_t *cas, unsigned long val)
{
	cas->clock = val;
	cas->remainder = 0;
}

void cas_set_auto_play (cassette_t *cas, int val)
{
	cas->auto_play = (val != 0);
}

void cas_set_auto_motor (cassette_t *cas, int val)
{
	cas->auto_motor = (val != 0);
}

void cas_set_motor_delay (cassette_t *cas, unsigned long val)
{
	cas->motor_delay = val;
}

static
pti_img_t *cas_get_read_img (const cassette_t *cas)
{
	if (cas->read_img != NULL) {
		return (cas->read_img);
	}
	else if (cas->write_img != NULL) {
		return (cas->write_img);
	}

	return (NULL);
}

static
int cas_file_exists (const char *name)
{
	FILE *fp;

	if ((fp = fopen (name, "rb")) == NULL) {
		return (0);
	}

	fclose (fp);

	return (1);
}

int cas_set_read_name (cassette_t *cas, const char *fname)
{
	cas_set_record (cas, 0);
	cas_set_play (cas, 0);

	pti_img_del (cas->read_img);
	cas->read_img = NULL;

	free (cas->read_name);
	cas->read_name = NULL;

	cas->position = 0;
	cas->remainder = 0;

	if (fname == NULL) {
		return (0);
	}

	if ((cas->read_name = str_copy_alloc (fname)) == NULL) {
		return (1);
	}

	if ((cas->read_img = pti_img_load (cas->read_name, PTI_FORMAT_NONE)) == NULL) {
		return (1);
	}

	cas->eof = 0;

	return (0);
}

int cas_set_write_name (cassette_t *cas, const char *fname, int create)
{
	cas_set_record (cas, 0);
	cas_set_play (cas, 0);

	if (cas_commit (cas)) {
		return (1);
	}

	pti_img_del (cas->write_img);
	cas->write_img = NULL;

	free (cas->write_name);
	cas->write_name = NULL;

	cas->modified = 0;
	cas->position = 0;
	cas->remainder = 0;

	if (fname == NULL) {
		return (0);
	}

	if ((cas->write_name = str_copy_alloc (fname)) == NULL) {
		return (1);
	}

	if (create) {
		if (cas_file_exists (fname) == 0) {
			if ((cas->write_img = pti_img_new()) == NULL) {
				return (1);
			}

			pti_img_set_clock (cas->write_img, cas->clock);

			return (0);
		}
	}

	if ((cas->write_img = pti_img_load (cas->write_name, PTI_FORMAT_NONE)) == NULL) {
		return (1);
	}

	return (0);
}

int cas_commit (cassette_t *cas)
{
	if (cas->modified == 0) {
		return (0);
	}

	if ((cas->write_img == NULL) || (cas->write_name == NULL)) {
		return (1);
	}

	pce_printf ("CASSETTE writing back %s\n", cas->write_name);

	if (pti_img_save (cas->write_name, cas->write_img, PTI_FORMAT_NONE)) {
		return (1);
	}

	cas->modified = 0;

	return (0);
}

int cas_get_position (const cassette_t *cas, double *tm)
{
	unsigned long pos, sec, clk;
	pti_img_t     *img;

	if (cas->record) {
		if ((img = cas->write_img) == NULL) {
			return (1);
		}

		pos = img->pulse_cnt;
	}
	else {
		if ((img = cas_get_read_img (cas)) == NULL) {
			return (1);
		}

		pos = cas->position;
	}

	pti_img_get_time (img, pos, &sec, &clk);
	pti_time_get (sec, clk, tm, img->clock);

	return (0);
}

int cas_set_position (cassette_t *cas, double tm)
{
	unsigned long pos, sec, clk;
	pti_img_t     *img;

	if ((img = cas_get_read_img (cas)) == NULL) {
		return (1);
	}

	pti_time_set (&sec, &clk, tm, img->clock);
	pti_img_get_index (img, &pos, sec, clk);

	cas->position = pos;

	cas->remainder = 0;
	cas->eof = 0;

	return (0);
}

int cas_truncate (cassette_t *cas, double tm)
{
	unsigned long pos, sec, clk;

	if (cas->write_img == NULL) {
		return (1);
	}

	pti_time_set (&sec, &clk, tm, cas->write_img->clock);
	pti_img_get_index (cas->write_img, &pos, sec, clk);

	if (pos < cas->write_img->pulse_cnt) {
		cas->write_img->pulse_cnt = pos;
		cas->modified = 1;
	}

	return (0);
}

int cas_space (cassette_t *cas, double tm)
{
	unsigned long sec, clk;

	if (cas->write_img == NULL) {
		return (1);
	}

	pti_time_set (&sec, &clk, tm, cas->write_img->clock);

	if (pti_img_add_pulse (cas->write_img, clk, 0)) {
		return (1);
	}

	cas->modified = 1;

	return (0);
}

static
int cas_read_pulse (cassette_t *cas, unsigned long *clk, int *level)
{
	pti_img_t *img;

	if ((img = cas->read_img) == NULL) {
		if ((img = cas->write_img) == NULL) {
			return (1);
		}
	}

	if (cas->position >= img->pulse_cnt) {
		return (1);
	}

	pti_pulse_get (img->pulse[cas->position], clk, level);

	*clk = pti_pulse_convert (*clk, cas->clock, img->clock, &cas->remainder);

	cas->position += 1;

	return (0);
}

static
int cas_write_pulse (cassette_t *cas, unsigned long clk, int level)
{
	pti_img_t *img;

	if ((img = cas->write_img) == NULL) {
		return (1);
	}

	clk = pti_pulse_convert (clk, img->clock, cas->clock, &cas->remainder);

	if (pti_img_add_pulse (img, clk, level)) {
		return (1);
	}

	cas->modified = 1;

	return (0);
}

static
void cas_write_pulse_flush (cassette_t *cas)
{
	if ((cas->run == 0) || (cas->record == 0)) {
		return;
	}

	if (cas->counter > 0) {
		cas_write_pulse (cas, cas->counter, cas->out_val ? 1 : -1);
		cas->counter = 0;
	}
}

static
void cas_run_stop (cassette_t *cas)
{
	int run;

	run = (cas->motor && cas->play) || (cas->motor_delay_count > 0);

	if (cas->run == run) {
		return;
	}

	if (cas->run && cas->record) {
		cas_write_pulse_flush (cas);
	}

	cas->run = run;

	if (cas->set_run != NULL) {
		cas->set_run (cas->set_run_ext, run);
	}

	if (cas->record) {
		cas->eof = 0;
	}

	cas_print_state (cas);

	if (cas->run) {
		cas->counter = 0;
		cas->remainder = 0;
	}
}

void cas_set_play (cassette_t *cas, int val)
{
	val = (val != 0) || cas->auto_play;

	if (cas->play == val) {
		return;
	}

	cas->play = val;
	cas->motor_delay_count = 0;

	if (cas->auto_motor) {
		cas->motor = cas->play || cas->record;
	}

	if (cas->set_play != NULL) {
		cas->set_play (cas->set_play_ext, cas->play);
	}

	cas_run_stop (cas);
}

void cas_set_record (cassette_t *cas, int val)
{
	val = (val != 0);

	if (cas->record == val) {
		return;
	}

	if (cas->run && cas->record) {
		cas_write_pulse_flush (cas);
	}

	cas->record = val;
	cas->motor_delay_count = 0;

	if (cas->auto_motor) {
		cas->motor = cas->play || cas->record;
	}

	cas_run_stop (cas);
}

void cas_press_keys (cassette_t *cas, int play, int record)
{
	cas_set_record (cas, record);
	cas_set_play (cas, play);
}

void cas_print_state (cassette_t *cas)
{
	pti_img_t     *img1, *img2;
	const char    *state;
	unsigned long sec, clk;
	double        tm1, tm2;

	img1 = cas_get_read_img (cas);
	img2 = cas->write_img;

	tm1 = 0.0;
	tm2 = 0.0;

	if (img1 != NULL) {
		pti_img_get_time (img1, cas->position, &sec, &clk);
		pti_time_get (sec, clk, &tm1, img1->clock);
	}

	if (img2 != NULL) {
		pti_img_get_length (img2, &sec, &clk);
		pti_time_get (sec, clk, &tm2, img2->clock);
	}

	if (cas->eof) {
		state = "EOF";
	}
	else if (cas->run) {
		state = cas->record ? "SAVE" : "LOAD";
	}
	else {
		state = "";
	}

	pce_printf ("CASSETTE [%c%c%c] [%05.1f] [%05.1f] %s\n",
		cas->play ? 'P' : '-',
		cas->record ? 'R' : '-',
		cas->motor ? 'M' : '-',
		tm1, tm2,
		state
	);
}

void cas_set_motor (cassette_t *cas, int val)
{
	val = (val != 0);

	if (cas->auto_motor) {
		val |= cas->play || cas->record;
	}

	if (cas->motor == val) {
		return;
	}

	cas->motor = val;

	if (cas->run && (cas->motor == 0)) {
		cas->motor_delay_count = cas->motor_delay;
	}
	else {
		cas->motor_delay_count = 0;
	}

	cas_run_stop (cas);
}

static
void cas_set_inp (cassette_t *cas, int val)
{
	val = (val != 0);

	if (cas->inp_val == val) {
		return;
	}

	if (cas->set_inp != NULL) {
		cas->set_inp (cas->set_inp_ext, val);
	}

	cas->inp_val = val;
}

int cas_get_inp (const cassette_t *cas)
{
	return (cas->inp_val);
}

void cas_set_out (cassette_t *cas, int val)
{
	val = (val != 0);

	if (cas->out_val == val) {
		return;
	}

	if ((cas->run == 0) || (cas->record == 0)) {
		cas->out_val = val;
		cas->inp_val = val;
		return;
	}

	cas_write_pulse (cas, cas->counter, cas->out_val ? 1 : -1);

	cas->counter = 0;

	cas->out_val = val;
}

void cas_clock (cassette_t *cas)
{
	int v;

	if (cas->run == 0) {
		return;
	}

	if (cas->motor_delay_count > 0) {
		cas->motor_delay_count -= 1;

		if (cas->motor_delay_count == 0) {
			cas_run_stop (cas);
		}
	}

	if (cas->record) {
		cas->counter += 1;
		return;
	}

	if (cas->counter > 1) {
		cas->counter -= 1;
		return;
	}

	if (cas_read_pulse (cas, &cas->counter, &v)) {
		cas->eof = 1;
		cas_press_keys (cas, 0, 0);
		return;
	}

	cas_set_inp (cas, v >= 0);
}
