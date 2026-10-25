/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/sysdep.h                                             *
 * Created:     2006-06-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2006-2015 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_LIB_SYSDEP_H
#define PCE_LIB_SYSDEP_H 1


#define PCE_BRK_STOP  1
#define PCE_BRK_ABORT 2


int pce_usleep (unsigned long usec);

/*!***************************************************************************
 * @short Get an interval in microseconds
 *
 * Returns the number of microseconds that passed since this function
 * was called the last time with the same parameter.
 *****************************************************************************/
unsigned long pce_get_interval_us (unsigned long *val);

void pce_srand (unsigned val);

#ifdef EMSCRIPTEN
/*!***************************************************************************
 * @short Run fct as the browser main loop, does not return
 *
 * fct is called once per animation frame while the page is visible and
 * from a timer while it is hidden, because browsers don't run animation
 * frames for hidden pages.
 *****************************************************************************/
void pce_em_run (void (*fct) (void));

/*!***************************************************************************
 * @short Get the real time to emulate in a browser main loop iteration
 *
 * Returns the number of milliseconds that passed since the last call,
 * limited to 100 ms (1000 ms while the page is hidden). If more time has
 * passed the emulation falls behind real time instead of catching up.
 *****************************************************************************/
double pce_em_get_frame_ms (void);

/*!***************************************************************************
 * @short Adjust the extra CPU clocks used at maximum speed
 * @param extra The extra clocks that are added to each CPU step
 * @param t0    The value of emscripten_get_now() before emulating ms
 * @param ms    The real time that was emulated
 *
 * This replaces the adjustment in the realtime sync functions, which
 * relies on sleeping.
 *****************************************************************************/
void pce_em_adjust_speed (unsigned long *extra, double t0, double ms);

/*!***************************************************************************
 * @short Count real time that was emulated
 *
 * The total is returned by pce_em_get_emulated_ms(), which is exported to
 * JavaScript (Module._pce_em_get_emulated_ms) for tests.
 *****************************************************************************/
void pce_em_add_emulated_ms (double ms);

double pce_em_get_emulated_ms (void);
#endif

void pce_set_fd_interactive (int fd, int interactive);

/*!***************************************************************************
 * @short Check if a file descriptor is readable without blocking
 * @param fd The file descriptor
 * @param t  Wait at most t milliseconds for fd to become readable
 *****************************************************************************/
int pce_fd_readable (int fd, int t);

/*!***************************************************************************
 * @short Check if a file descriptor is writeable without blocking
 * @param fd The file descriptor
 * @param t  Wait at most t milliseconds for fd to become writeable
 *****************************************************************************/
int pce_fd_writeable (int fd, int t);


void pce_start (unsigned *brk);

void pce_stop (void);


#endif
