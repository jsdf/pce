/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/cassette.c                                    *
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


#include "main.h"


#define PC_CAS_CLK 1193				/* 1 ms */


void pc_cas_init (pc_cassette_t *cas)
{
	cas->save = 0;

	cas->motor = 0;

	cas->position = 0;

	cas->position_save = 0;
	cas->position_load = 0;

	cas->data_out = 0;
	cas->data_inp = 0;

	cas->clk = 0;

	cas->clk_out = 0;
	cas->clk_inp = 0;

	cas->out_cnt = 0;
	cas->out_buf = 0;

	cas->inp_cnt = 0;
	cas->inp_buf = 0;
	cas->inp_bit = 0;

	cas->close = 0;
	cas->fp = NULL;
}

void pc_cas_free (pc_cassette_t *cas)
{
	if (cas->close) {
		fclose (cas->fp);
	}
}

int pc_cas_get_mode (pc_cassette_t *cas)
{
	return (cas->save);
}

void pc_cas_set_mode (pc_cassette_t *cas, int save)
{
	save = (save != 0);

	if (cas->save == save) {
		return;
	}

	if (cas->save) {
		cas->position_save = cas->position;
		cas->position = cas->position_load;
	}
	else {
		cas->position_load = cas->position;
		cas->position = cas->position_save;
	}

	cas->save = save;

	if (cas->fp != NULL) {
		fflush (cas->fp);

		pc_cas_set_position (cas, cas->position);
	}
}

void pc_cas_rewind (pc_cassette_t *cas)
{
	if (cas->fp != NULL) {
		rewind (cas->fp);
		cas->position = 0;
	}
}

void pc_cas_append (pc_cassette_t *cas)
{
	if (cas->fp != NULL) {
		fseek (cas->fp, 0, SEEK_END);
		cas->position = ftell (cas->fp);
	}
}

unsigned long pc_cas_get_position (pc_cassette_t *cas)
{
	return (cas->position);
}

int pc_cas_set_position (pc_cassette_t *cas, unsigned long pos)
{
	if (cas->fp == NULL) {
		return (1);
	}

	if (fseek (cas->fp, pos, SEEK_SET) != 0) {
		return (1);
	}

	cas->position = pos;

	return (0);
}

int pc_cas_set_fname (pc_cassette_t *cas, const char *fname)
{
	if (cas->close) {
		fclose (cas->fp);
	}

	cas->close = 0;
	cas->fp = NULL;

	cas->position = 0;

	cas->position_save = 0;
	cas->position_load = 0;

	if (fname == NULL) {
		return (0);
	}

	cas->fp = fopen (fname, "r+b");

	if (cas->fp == NULL) {
		cas->fp = fopen (fname, "w+b");
	}

	if (cas->fp == NULL) {
		return (1);
	}

	cas->close = 1;

	pc_cas_append (cas);

	cas->position_save = cas->position;

	if (cas->save == 0) {
		pc_cas_set_position (cas, 0);
	}

	return (0);
}

void pc_cas_set_motor (pc_cassette_t *cas, unsigned char val)
{
	val = (val != 0);

	if (val == cas->motor) {
		return;
	}

	pce_printf ("cassette at %lu motor %s\n",
		cas->position,
		val ? "on " : "off"
	);

	cas->motor = val;

	cas->clk_out = cas->clk;
	cas->clk_inp = 0;

	cas->out_cnt = 0;
	cas->out_buf = 0;

	cas->inp_cnt = 0;
	cas->inp_buf = 0;
	cas->inp_bit = 0;

	if (cas->fp != NULL) {
		fflush (cas->fp);

		pc_cas_set_position (cas, cas->position);
	}
}

static
void pc_cas_read_bit (pc_cassette_t *cas)
{
	int val;
	int eof;

	if (cas->inp_cnt == 0) {
		if (cas->fp == NULL) {
			return;
		}

		eof = feof (cas->fp);

		val = fgetc (cas->fp);
		if (val == EOF) {
			if (feof (cas->fp) && !eof) {
				pce_printf ("cassette at %lu EOF\n",
					cas->position
				);
			}

			return;
		}

		cas->position += 1;

		cas->inp_cnt = 8;
		cas->inp_buf = val;
	}

	cas->inp_bit = ((cas->inp_buf & 0x80) != 0);

	cas->inp_buf = (cas->inp_buf << 1) & 0xff;
	cas->inp_cnt -= 1;
}

static
void pc_cas_write_bit (pc_cassette_t *cas, unsigned char val)
{
	if (val) {
		cas->out_buf |= (0x80 >> cas->out_cnt);
	}

	cas->out_cnt += 1;

	if (cas->out_cnt >= 8) {
		if (cas->fp != NULL) {
			fputc (cas->out_buf, cas->fp);
			cas->position += 1;
		}

		cas->out_buf = 0;
		cas->out_cnt = 0;
	}
}

unsigned char pc_cas_get_inp (pc_cassette_t *cas)
{
	return (cas->data_inp);
}

void pc_cas_set_out (pc_cassette_t *cas, unsigned char val)
{
	unsigned long clk;

	val = (val != 0);

	if (cas->motor == 0) {
		cas->data_inp = val;
		return;
	}

	if (cas->save == 0) {
		return;
	}

	if (cas->data_out == val) {
		return;
	}

	cas->data_out = val;

	if (val == 0) {
		return;
	}

	clk = cas->clk - cas->clk_out;
	cas->clk_out = cas->clk;

	if (clk < (PC_CAS_CLK / 4)) {
		;
	}
	else if (clk < ((3 * PC_CAS_CLK) / 4)) {
		pc_cas_write_bit (cas, 0);
	}
	else if (clk < ((5 * PC_CAS_CLK) / 4)) {
		pc_cas_write_bit (cas, 1);
	}
}

void pc_cas_print_state (pc_cassette_t *cas)
{
	pce_printf ("cassette at %lu %s\n",
		cas->position,
		cas->save ? "saving" : "loading"
	);
}

void pc_cas_clock (pc_cassette_t *cas, unsigned long cnt)
{
	cas->clk += cnt;

	if (cas->motor == 0) {
		return;
	}

	if (cas->save) {
		return;
	}

	if (cas->clk_inp > cnt) {
		cas->clk_inp -= cnt;
		return;
	}

	cnt -= cas->clk_inp;

	cas->data_inp = !cas->data_inp;

	if (cas->data_inp) {
		pc_cas_read_bit (cas);
	}

	cas->clk_inp = cas->inp_bit ? (PC_CAS_CLK / 2) : (PC_CAS_CLK / 4);

	if (cas->clk_inp > cnt) {
		cas->clk_inp -= cnt;
	}
}
