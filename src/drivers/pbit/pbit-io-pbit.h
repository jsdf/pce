/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pbit/pbit-io-pbit.h                              *
 * Created:     2012-01-31 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_PBIT_IO_PBIT_H
#define PCE_PBIT_IO_PBIT_H 1


#include <drivers/pbit/pbit.h>


pbit_img_t *pbit_load_pbit (FILE *fp);

int pbit_save_pbit (FILE *fp, const pbit_img_t *img);

int pbit_probe_pbit_fp (FILE *fp);
int pbit_probe_pfdc (const char *fname);


#endif
