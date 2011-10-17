/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/options.h                                        *
 * Created:     2009-10-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DRIVERS_OPTIONS_H
#define PCE_DRIVERS_OPTIONS_H 1


/*!***************************************************************************
 * @short  Get a named driver option
 * @return The option value or NULL if the option is not defined in str
 *
 * The returned string is owned by the caller.
 *****************************************************************************/
char *drv_get_option (const char *str, const char *name);

/*!***************************************************************************
 * @short  Get a named boolean driver option
 @ @return The option value
 *****************************************************************************/
int drv_get_option_bool (const char *str, const char *name, int def);

/*!***************************************************************************
 * @short  Get a named unsigned integer driver option
 @ @return The option value
 *****************************************************************************/
unsigned long drv_get_option_uint (const char *str, const char *name, unsigned long def);

/*!***************************************************************************
 * @short  Get a named integer driver option
 @ @return The option value
 *****************************************************************************/
long drv_get_option_sint (const char *str, const char *name, long def);


#endif
