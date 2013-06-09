/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi-io.h                                     *
 * Created:     2012-02-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PSI_PSI_IO_H
#define PSI_PSI_IO_H 1


unsigned long psi_get_uint32_be (const void *buf, unsigned idx);
unsigned psi_get_uint16_be (const void *buf, unsigned idx);
void psi_set_uint16_be (void *buf, unsigned idx, unsigned val);
void psi_set_uint32_be (void *buf, unsigned idx, unsigned long val);

unsigned psi_get_uint16_le (const void *buf, unsigned idx);
unsigned long psi_get_uint32_le (const void *buf, unsigned idx);
void psi_set_uint16_le (void *buf, unsigned idx, unsigned val);

int psi_read (FILE *fp, void *buf, unsigned long cnt);
int psi_read_ofs (FILE *fp, unsigned long ofs, void *buf, unsigned long cnt);
int psi_write (FILE *fp, const void *buf, unsigned long cnt);
int psi_write_ofs (FILE *fp, unsigned long ofs, const void *buf, unsigned long cnt);
int psi_skip (FILE *fp, unsigned long cnt);


#endif
