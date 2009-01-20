/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/iniram.h                                             *
 * Created:     2005-07-24 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2005-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_LIB_INIRAM_H
#define PCE_LIB_INIRAM_H 1


#include <libini/libini.h>

#include <devices/memory.h>


/*!***************************************************************************
 * @short  Initialize memory blocks from ini file
 * @param  mem   The address space
 * @param  ini   The ini section containing the "ram" sections
 * @retval addr0 Returns the memory block that starts at address 0, or NULL.
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int ini_get_ram (memory_t *mem, ini_sct_t *ini, mem_blk_t **addr0);

/*!***************************************************************************
 * @short  Initialize read-only memory blocks from ini file
 * @param  mem The address space
 * @param  ini The ini section containing the "rom" sections
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int ini_get_rom (memory_t *mem, ini_sct_t *ini);


#endif
