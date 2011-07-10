/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/video/terminal.h                                 *
 * Created:     2003-04-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_VIDEO_TERMINAL_H
#define PCE_VIDEO_TERMINAL_H 1


#include <stdio.h>

#include <drivers/video/keys.h>

#include <libini/libini.h>


/*!***************************************************************************
 * @short The terminal structure
 *****************************************************************************/
typedef struct {
	void *ext;

	void *set_msg_emu_ext;
	int  (*set_msg_emu) (void *ext, const char *msg, const char *val);

	void *set_key_ext;
	void (*set_key) (void *ext, unsigned event, pce_key_t key);

	void *set_mouse_ext;
	void (*set_mouse) (void *ext, int dx, int dy, unsigned but);

	void (*del) (void *ext);

	int  (*open) (void *ext, unsigned w, unsigned h);

	int  (*close) (void *ext);

	int  (*set_msg_trm) (void *ext, const char *msg, const char *val);

	void (*update) (void *ext);

	void (*check) (void *ext);

	int           is_open;

	pce_key_t     escape_key;
	unsigned      escape;

	/* terminal buffer size */
	unsigned      w;
	unsigned      h;

	unsigned long buf_cnt;
	unsigned char *buf;

	unsigned      scale_x;
	unsigned      scale_y;

	/* mul, div, overflow */
	int           mouse_scale_x[3];
	int           mouse_scale_y[3];

	unsigned long scale_buf_cnt;
	unsigned char *scale_buf;

	/* update rectangle */
	unsigned      update_x;
	unsigned      update_y;
	unsigned      update_w;
	unsigned      update_h;

	/* picture index for screenshots */
	unsigned      pict_index;
} terminal_t;


terminal_t *null_new (ini_sct_t *ini);
terminal_t *xt_new (ini_sct_t *ini);
terminal_t *sdl_new (ini_sct_t *ini);


/*!***************************************************************************
 * @short Initialize a terminal structure
 *****************************************************************************/
void trm_init (terminal_t *trm, void *ext);

/*!***************************************************************************
 * @short Free the resources used by a terminal structure
 *****************************************************************************/
void trm_free (terminal_t *trm);

/*!***************************************************************************
 * @short Delete a terminal
 *****************************************************************************/
void trm_del (terminal_t *trm);

/*!***************************************************************************
 * @short Open a terminal
 *
 * This function must be called after trm_init() and before the first
 * output is sent to the terminal. The terminal size w and h is considered
 * a hint only.
 *****************************************************************************/
int trm_open (terminal_t *trm, unsigned w, unsigned h);

/*!***************************************************************************
 * @short Close a terminal
 *
 * This function is called automatically by trm_del().
 *****************************************************************************/
int trm_close (terminal_t *trm);

/*!***************************************************************************
 * @short Set the terminal message function
 *
 * The terminal calls the message function when it wants to send a message
 * to the core.
 *****************************************************************************/
void trm_set_msg_fct (terminal_t *trm, void *ext, void *fct);

/*!***************************************************************************
 * @short Set the terminal key function
 *
 * The terminal calls the key function to send key events to the core.
 *****************************************************************************/
void trm_set_key_fct (terminal_t *trm, void *ext, void *fct);

/*!***************************************************************************
 * @short Set the terminal mouse function
 *
 * The terminal calls the mouse function to send key events to the core.
 *****************************************************************************/
void trm_set_mouse_fct (terminal_t *trm, void *ext, void *fct);

/*!***************************************************************************
 * @short Save the terminal buffer to an image file
 *
 * If the file name is NULL, a default name is generated automatically.
 *****************************************************************************/
int trm_screenshot (terminal_t *trm, const char *fname);

/*!***************************************************************************
 * @short Send a message to the terminal
 *****************************************************************************/
int trm_set_msg_trm (terminal_t *trm, const char *msg, const char *val);

/*!***************************************************************************
 * @short Set the mouse scale factors
 *
 * Terminal mouse coordinates are multiplied by (mul_x / div_x) and
 * (mul_y / div_y) before they are sent to the emulator core.
 *****************************************************************************/
void trm_set_mouse_scale (terminal_t *trm, int mul_x, int div_x, int mul_y, int div_y);

/*!***************************************************************************
 * @short Set the terminal window size
 *****************************************************************************/
void trm_set_size (terminal_t *trm, unsigned w, unsigned h);

/*!***************************************************************************
 * @short Set the terminal scale factors
 *****************************************************************************/
void trm_set_scale (terminal_t *trm, unsigned fx, unsigned fy);

/*!***************************************************************************
 * @short Set a pixel in the terminal buffer
 * @param col The pixel color. This is an array of three RGB values.
 *****************************************************************************/
void trm_set_pixel (terminal_t *trm, unsigned x, unsigned y, const unsigned char *col);

/*!***************************************************************************
 * @short Set lines in the terminal buffer
 * @param buf The RGB source buffer, 3 bytes per pixel
 * @param y   The first line in the terminal buffer
 * @param cnt The number of lines
 *
 * The buffer width is implicit, as set by trm_set_size().
 *****************************************************************************/
void trm_set_lines (terminal_t *trm, const void *buf, unsigned y, unsigned cnt);

/*!***************************************************************************
 * @short Update the screen from the terminal buffer
 *****************************************************************************/
void trm_update (terminal_t *trm);

/*!***************************************************************************
 * @short Check for terminal events
 *
 * This function must be called periodically.
 *****************************************************************************/
void trm_check (terminal_t *trm);


/*!***************************************************************************
 * @short Send a message to the emulator core
 *****************************************************************************/
int trm_set_msg_emu (terminal_t *trm, const char *msg, const char *val);

/*!***************************************************************************
 * @short Set the terminal escape key
 *****************************************************************************/
int trm_set_escape_str (terminal_t *trm, const char *str);

/*!***************************************************************************
 * @short Set the terminal escape key
 *****************************************************************************/
void trm_set_escape_key (terminal_t *trm, pce_key_t key);

/*!***************************************************************************
 * @short Send a key event to the emulator core
 *****************************************************************************/
void trm_set_key (terminal_t *trm, unsigned event, pce_key_t key);

/*!***************************************************************************
 * @short Send a mouse event to the emulator core
 *****************************************************************************/
void trm_set_mouse (terminal_t *trm, int dx, int dy, unsigned but);

/*!***************************************************************************
 * @short Get the scale factors, given a specific terminal size
 *****************************************************************************/
void trm_get_scale (terminal_t *trm, unsigned w, unsigned h, unsigned *fx, unsigned *fy);

/*!***************************************************************************
 * @short Scale the terminal buffer
 * @param src The source buffer
 * @param w   The source buffer width
 * @param h   The source buffer height
 * @param fx  The scale factor in x direction
 * @param fy  The scale factor in y direction
 *
 * The returned buffer is owned by the terminal and will be overwritten by
 * subsequent calls to this function.
 *****************************************************************************/
const unsigned char *trm_scale (terminal_t *trm,
	const unsigned char *src, unsigned w, unsigned h,
	unsigned fx, unsigned fy
);


#endif
