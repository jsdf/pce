/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/pfi-scp.h                                    *
 * Created:     2014-01-30 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2014-2017 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PFI_IMG_SCP_H
#define PFI_IMG_SCP_H 1


#include <drivers/pfi/pfi.h>


pfi_img_t *pfi_load_scp (FILE *fp);

int pfi_save_scp (FILE *fp, pfi_img_t *img);

int pfi_probe_scp_fp (FILE *fp);
int pfi_probe_scp (const char *fname);


#endif
