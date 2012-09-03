/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/nvm.c                                         *
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


#include "main.h"
#include "nvm.h"

#include <stdio.h>
#include <stdlib.h>

#include <lib/string.h>


void rc759_nvm_init (rc759_nvm_t *nvm)
{
	nvm->fname = NULL;
	nvm->modified = 0;

	rc759_nvm_reset (nvm);
}

void rc759_nvm_free (rc759_nvm_t *nvm)
{
	if (nvm->modified) {
		rc759_nvm_save (nvm);
	}

	if (nvm->fname != NULL) {
		free (nvm->fname);
	}
}

int rc759_nvm_set_fname (rc759_nvm_t *nvm, const char *fname)
{
	if (nvm->fname != NULL) {
		free (nvm->fname);
		nvm->fname = NULL;
	}

	if (fname == NULL) {
		return (0);
	}

	nvm->fname = str_copy_alloc (fname);

	if (nvm->fname == NULL) {
		return (1);
	}

	return (0);
}

void rc759_nvm_reset (rc759_nvm_t *nvm)
{
	unsigned i;

	nvm->modified = 1;
	nvm->data[0] = 0xaa;

	for (i = 1; i < RC759_NVM_SIZE; i++) {
		nvm->data[i] = 0;
	}
}

int rc759_nvm_load (rc759_nvm_t *nvm)
{
	unsigned n;
	FILE     *fp;

	if (nvm->fname == NULL) {
		return (1);
	}

	fp = fopen (nvm->fname, "rb");

	if (fp == NULL) {
		return (1);
	}

	n = fread (nvm->data, 1, RC759_NVM_SIZE, fp);

	fclose (fp);

	if (n < RC759_NVM_SIZE) {
		nvm->modified = 1;

		while (n < RC759_NVM_SIZE) {
			nvm->data[n++] = 0;
		}
	}
	else {
		nvm->modified = 0;
	}

	return (0);
}

int rc759_nvm_save (rc759_nvm_t *nvm)
{
	FILE *fp;

	if (nvm->fname == NULL) {
		return (1);
	}

	fp = fopen (nvm->fname, "wb");

	if (fp == NULL) {
		return (1);
	}

	if (fwrite (nvm->data, 1, RC759_NVM_SIZE, fp) != RC759_NVM_SIZE) {
		fclose (fp);
		return (1);
	}

	fclose (fp);

	nvm->modified = 0;

	return (0);
}

void rc759_nvm_fix_checksum (rc759_nvm_t *nvm)
{
	unsigned i, n, v;

	v = 0;
	n = (RC759_NVM_SIZE < 96) ? RC759_NVM_SIZE : 96;

	for (i = 0; i < n; i++) {
		v += nvm->data[i];
	}

	nvm->data[0] = (nvm->data[0] + 0xaa - v) & 0xff;
}

void rc759_nvm_sanitize (rc759_nvm_t *nvm)
{
	if (nvm->data[0x1a] < 2) {
		/* data buffers */
		nvm->data[0x1a] = 2;
	}
}

unsigned char rc759_nvm_get_uint4 (const rc759_nvm_t *nvm, unsigned idx)
{
	unsigned char val;

	if (idx < (2 * RC759_NVM_SIZE)) {
		val = nvm->data[idx >> 1];
	}
	else {
		val = 0;
	}

	if ((idx & 1) == 0) {
		val >>= 4;
	}

	return (val & 0x0f);
}

void rc759_nvm_set_uint4 (rc759_nvm_t *nvm, unsigned idx, unsigned char val)
{
	unsigned char *p;

	if (idx < (2 * RC759_NVM_SIZE)) {
		p = &nvm->data[idx >> 1];
	}
	else {
		return;
	}

	if (idx & 1) {
		*p = (*p & 0xf0) | (val & 0x0f);
	}
	else {
		*p = (*p & 0x0f) | ((val << 4) & 0xf0);
	}

	nvm->modified = 1;
}

unsigned char rc759_nvm_get_uint8 (const rc759_nvm_t *nvm, unsigned idx)
{
	if (idx < RC759_NVM_SIZE) {
		return (nvm->data[idx]);
	}

	return (0);
}

void rc759_nvm_set_uint8 (rc759_nvm_t *nvm, unsigned idx, unsigned char val)
{
	if (idx < RC759_NVM_SIZE) {
		nvm->data[idx] = val;
		nvm->modified = 1;
	}
}
