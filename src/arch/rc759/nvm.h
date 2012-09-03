/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/nvm.h                                         *
 * Created:     2012-07-02 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_RC759_NVM_H
#define PCE_RC759_NVM_H 1


#define RC759_NVM_SIZE 128


typedef struct {
	char          *fname;
	char          modified;
	unsigned char data[RC759_NVM_SIZE];
} rc759_nvm_t;


void rc759_nvm_init (rc759_nvm_t *nvm);
void rc759_nvm_free (rc759_nvm_t *nvm);

int rc759_nvm_set_fname (rc759_nvm_t *nvm, const char *fname);

void rc759_nvm_reset (rc759_nvm_t *nvm);

int rc759_nvm_load (rc759_nvm_t *nvm);
int rc759_nvm_save (rc759_nvm_t *nvm);

void rc759_nvm_fix_checksum (rc759_nvm_t *nvm);
void rc759_nvm_sanitize (rc759_nvm_t *nvm);

unsigned char rc759_nvm_get_uint4 (const rc759_nvm_t *nvm, unsigned idx);
void rc759_nvm_set_uint4 (rc759_nvm_t *nvm, unsigned idx, unsigned char val);

unsigned char rc759_nvm_get_uint8 (const rc759_nvm_t *nvm, unsigned idx);
void rc759_nvm_set_uint8 (rc759_nvm_t *nvm, unsigned idx, unsigned char val);


#endif
