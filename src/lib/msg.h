/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/msg.h                                                *
 * Created:     2005-12-08 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2005-2020 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_LIB_MSG_H
#define PCE_LIB_MSG_H 1


/*!***************************************************************************
 * @short  Check a message
 * @return True if val is of the form "[rest.]msg"
 *****************************************************************************/
int msg_is_message (const char *msg, const char *val);

/*!***************************************************************************
 * @short  Check a message for a prefix
 * @return True if val is of the form "pre.rest".
 *****************************************************************************/
int msg_is_prefix (const char *pre, const char *val);

int msg_get_ulng (const char *str, unsigned long *val);
int msg_get_slng (const char *str, long *val);
int msg_get_uint (const char *str, unsigned *val);
int msg_get_sint (const char *str, int *val);
int msg_get_double (const char *str, double *val);
int msg_get_bool (const char *str, int *val);

int msg_get_prefix_ulng (const char **str, unsigned long *val, const char *sep, const char *trim);
int msg_get_prefix_uint (const char **str, unsigned *val, const char *sep, const char *trim);
int msg_get_prefix_slng (const char **str, long *val, const char *sep, const char *trim);
int msg_get_prefix_sint (const char **str, int *val, const char *sep, const char *trim);
int msg_get_prefix_bool (const char **str, int *val, const char *sep, const char *trim);


#endif
