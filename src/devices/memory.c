/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/memory.c                                         *
 * Created:     2000-04-23 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2000-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdlib.h>
#include <string.h>

#include "memory.h"


int mem_blk_init (mem_blk_t *blk, unsigned long base, unsigned long size, int alloc)
{
	if (alloc) {
		blk->data = malloc (size + 16);
		if (blk->data == NULL) {
			return (1);
		}
	}
	else {
		blk->data = NULL;
	}

	blk->get_uint8 = NULL;
	blk->get_uint16 = NULL;
	blk->get_uint32 = NULL;
	blk->set_uint8 = NULL;
	blk->set_uint16 = NULL;
	blk->set_uint32 = NULL;

	blk->ext = blk;

	blk->active = 1;
	blk->readonly = 0;
	blk->data_del = (blk->data != NULL);
	blk->addr1 = base;
	blk->addr2 = base + size - 1;
	blk->size = size;

	return (0);
}

mem_blk_t *mem_blk_new (unsigned long base, unsigned long size, int alloc)
{
	mem_blk_t *blk;

	blk = (mem_blk_t *) malloc (sizeof (mem_blk_t));
	if (blk == NULL) {
		return (NULL);
	}

	if (mem_blk_init (blk, base, size, alloc)) {
		free (blk);
		return (NULL);
	}

	return (blk);
}

void mem_blk_free (mem_blk_t *blk)
{
	if (blk != NULL) {
		if (blk->data_del) {
			free (blk->data);
		}
	}
}

void mem_blk_del (mem_blk_t *blk)
{
	if (blk != NULL) {
		mem_blk_free (blk);
		free (blk);
	}
}

mem_blk_t *mem_blk_clone (const mem_blk_t *blk)
{
	mem_blk_t *ret;

	ret = malloc (sizeof (mem_blk_t));
	if (ret == NULL) {
		return (NULL);
	}

	*ret = *blk;

	ret->data_del = 0;
	ret->active = 1;

	return (ret);
}

void mem_blk_fix_fct (mem_blk_t *blk)
{
	if (blk->data != NULL) {
		return;
	}

	if (blk->get_uint8 == NULL) {
		blk->get_uint8 = (void *) mem_blk_get_uint8_null;
	}

	if (blk->get_uint16 == NULL) {
		blk->get_uint16 = (void *) mem_blk_get_uint16_null;
	}

	if (blk->get_uint32 == NULL) {
		blk->get_uint32 = (void *) mem_blk_get_uint32_null;
	}

	if (blk->set_uint8 == NULL) {
		blk->set_uint8 = (void *) mem_blk_set_uint8_null;
	}

	if (blk->set_uint16 == NULL) {
		blk->set_uint16 = (void *) mem_blk_set_uint16_null;
	}

	if (blk->set_uint32 == NULL) {
		blk->set_uint32 = (void *) mem_blk_set_uint32_null;
	}
}

void mem_blk_set_fget (mem_blk_t *blk, void *ext, void *g8, void *g16, void *g32)
{
	blk->ext = ext;
	blk->get_uint8 = g8;
	blk->get_uint16 = g16;
	blk->get_uint32 = g32;

	mem_blk_fix_fct (blk);
}

void mem_blk_set_fset (mem_blk_t *blk, void *ext, void *s8, void *s16, void *s32)
{
	blk->ext = ext;
	blk->set_uint8 = s8;
	blk->set_uint16 = s16;
	blk->set_uint32 = s32;

	mem_blk_fix_fct (blk);
}

void mem_blk_set_fct (mem_blk_t *blk, void *ext,
	void *g8, void *g16, void *g32, void *s8, void *s16, void *s32)
{
	blk->ext = ext;

	blk->get_uint8 = g8;
	blk->get_uint16 = g16;
	blk->get_uint32 = g32;

	blk->set_uint8 = s8;
	blk->set_uint16 = s16;
	blk->set_uint32 = s32;

	mem_blk_fix_fct (blk);
}

void mem_blk_set_ext (mem_blk_t *blk, void *ext)
{
	blk->ext = ext;
}

void mem_blk_clear (mem_blk_t *blk, unsigned char val)
{
	if (blk->data != NULL) {
		memset (blk->data, val, blk->size);
	}
}

unsigned char *mem_blk_get_data (mem_blk_t *blk)
{
	return (blk->data);
}

void mem_blk_set_data (mem_blk_t *blk, void *data, int del)
{
	if (blk->data_del) {
		free (blk->data);
	}

	blk->data = data;
	blk->data_del = (data != NULL) && del;
}

int mem_blk_get_active (mem_blk_t *blk)
{
	return (blk->active);
}

void mem_blk_set_active (mem_blk_t *blk, int val)
{
	blk->active = (val != 0);
}

int mem_blk_get_readonly (mem_blk_t *blk)
{
	return (blk->readonly);
}

void mem_blk_set_readonly (mem_blk_t *blk, int val)
{
	blk->readonly = (val != 0);
}

unsigned long mem_blk_get_addr (const mem_blk_t *blk)
{
	return (blk->addr1);
}

void mem_blk_set_addr (mem_blk_t *blk, unsigned long addr)
{
	blk->addr1 = addr;
	blk->addr2 = addr + blk->size - 1;
}

unsigned long mem_blk_get_size (const mem_blk_t *blk)
{
	return (blk->size);
}

void mem_blk_set_size (mem_blk_t *blk, unsigned long size)
{
	blk->size = size;
	blk->addr2 = blk->addr1 + size - 1;
}


void buf_set_uint8 (void *buf, unsigned long addr, unsigned char val)
{
	unsigned char *tmp = (unsigned char *) buf + addr;

	tmp[0] = val;
}

void buf_set_uint16_be (void *buf, unsigned long addr, unsigned short val)
{
	unsigned char *tmp = (unsigned char *) buf + addr;

	tmp[0] = (val >> 8) & 0xff;
	tmp[1] = val & 0xff;
}

void buf_set_uint16_le (void *buf, unsigned long addr, unsigned short val)
{
	unsigned char *tmp = (unsigned char *) buf + addr;

	tmp[0] = val & 0xff;
	tmp[1] = (val >> 8) & 0xff;
}

void buf_set_uint32_be (void *buf, unsigned long addr, unsigned long val)
{
	unsigned char *tmp = (unsigned char *) buf + addr;

	tmp[0] = (val >> 24) & 0xff;
	tmp[1] = (val >> 16) & 0xff;
	tmp[2] = (val >> 8) & 0xff;
	tmp[3] = val & 0xff;
}

void buf_set_uint32_le (void *buf, unsigned long addr, unsigned long val)
{
	unsigned char *tmp = (unsigned char *) buf + addr;

	tmp[0] = val & 0xff;
	tmp[1] = (val >> 8) & 0xff;
	tmp[2] = (val >> 16) & 0xff;
	tmp[3] = (val >> 24) & 0xff;
}

unsigned char buf_get_uint8 (const void *buf, unsigned long addr)
{
	const unsigned char *tmp = (const unsigned char *) buf + addr;

	return (tmp[0]);
}

unsigned short buf_get_uint16_be (const void *buf, unsigned long addr)
{
	const unsigned char *tmp = (const unsigned char *) buf + addr;
	unsigned short      ret;

	ret = tmp[0];
	ret = (ret << 8) | tmp[1];

	return (ret);
}

unsigned short buf_get_uint16_le (const void *buf, unsigned long addr)
{
	const unsigned char *tmp = (const unsigned char *) buf + addr;
	unsigned short      ret;

	ret = tmp[1];
	ret = (ret << 8) | tmp[0];

	return (ret);
}

unsigned long buf_get_uint32_be (const void *buf, unsigned long addr)
{
	const unsigned char *tmp = (const unsigned char *) buf + addr;
	unsigned long       ret;

	ret = tmp[0];
	ret = (ret << 8) | tmp[1];
	ret = (ret << 8) | tmp[2];
	ret = (ret << 8) | tmp[3];

	return (ret);
}

unsigned long buf_get_uint32_le (const void *buf, unsigned long addr)
{
	const unsigned char *tmp = (const unsigned char *) buf + addr;
	unsigned long       ret;

	ret = tmp[3];
	ret = (ret << 8) | tmp[2];
	ret = (ret << 8) | tmp[1];
	ret = (ret << 8) | tmp[0];

	return (ret);
}


void mem_blk_set_uint8 (mem_blk_t *blk, unsigned long addr, unsigned char val)
{
	blk->data[addr] = val;
}

void mem_blk_set_uint8_null (void *ext, unsigned long addr, unsigned char val)
{
}

void mem_blk_set_uint16_be (mem_blk_t *blk, unsigned long addr, unsigned short val)
{
	blk->data[addr] = (val >> 8) & 0xff;
	blk->data[addr + 1] = val & 0xff;
}

void mem_blk_set_uint16_le (mem_blk_t *blk, unsigned long addr, unsigned short val)
{
	blk->data[addr] = val & 0xff;
	blk->data[addr + 1] = (val >> 8) & 0xff;
}

void mem_blk_set_uint16_null (void *ext, unsigned long addr, unsigned short val)
{
}

void mem_blk_set_uint32_be (mem_blk_t *blk, unsigned long addr, unsigned long val)
{
	blk->data[addr] = (val >> 24) & 0xff;
	blk->data[addr + 1] = (val >> 16) & 0xff;
	blk->data[addr + 2] = (val >> 8) & 0xff;
	blk->data[addr + 3] = val & 0xff;
}

void mem_blk_set_uint32_le (mem_blk_t *blk, unsigned long addr, unsigned long val)
{
	blk->data[addr] = val & 0xff;
	blk->data[addr + 1] = (val >> 8) & 0xff;
	blk->data[addr + 2] = (val >> 16) & 0xff;
	blk->data[addr + 3] = (val >> 24) & 0xff;
}

void mem_blk_set_uint32_null (void *ext, unsigned long addr, unsigned long val)
{
}

unsigned char mem_blk_get_uint8 (const mem_blk_t *blk, unsigned long addr)
{
	return (blk->data[addr]);
}

unsigned char mem_blk_get_uint8_null (const void *ext, unsigned long addr)
{
	return (0);
}

unsigned short mem_blk_get_uint16_be (const mem_blk_t *blk, unsigned long addr)
{
	unsigned short ret;

	ret = blk->data[addr];
	ret = (ret << 8) | blk->data[addr + 1];

	return (ret);
}

unsigned short mem_blk_get_uint16_le (const mem_blk_t *blk, unsigned long addr)
{
	unsigned short ret;

	ret = blk->data[addr + 1];
	ret = (ret << 8) | blk->data[addr];

	return (ret);
}

unsigned short mem_blk_get_uint16_null (const void *ext, unsigned long addr)
{
	return (0);
}

unsigned long mem_blk_get_uint32_be (const mem_blk_t *blk, unsigned long addr)
{
	unsigned long ret;

	ret = blk->data[addr];
	ret = (ret << 8) | blk->data[addr + 1];
	ret = (ret << 8) | blk->data[addr + 2];
	ret = (ret << 8) | blk->data[addr + 3];

	return (ret);
}

unsigned long mem_blk_get_uint32_le (const mem_blk_t *blk, unsigned long addr)
{
	unsigned long ret;

	ret = blk->data[addr + 3];
	ret = (ret << 8) | blk->data[addr + 2];
	ret = (ret << 8) | blk->data[addr + 1];
	ret = (ret << 8) | blk->data[addr];

	return (ret);
}

unsigned long mem_blk_get_uint32_null (const void *ext, unsigned long addr)
{
	return (0);
}


static
void mem_init_last (memory_t *mem)
{
	unsigned i;

	for (i = 0; i < MEM_LAST_CNT; i++) {
		mem->last[i] = NULL;
	}
}

void mem_init (memory_t *mem)
{
	mem->cnt = 0;
	mem->lst = NULL;

	mem_init_last (mem);

	mem->ext = NULL;
	mem->get_uint8 = NULL;
	mem->get_uint16 = NULL;
	mem->get_uint32 = NULL;
	mem->set_uint8 = NULL;
	mem->set_uint16 = NULL;
	mem->set_uint32 = NULL;

	mem->defval = 0xffffffff;
}

memory_t *mem_new (void)
{
	memory_t *mem;

	mem = malloc (sizeof (memory_t));
	if (mem == NULL) {
		return (NULL);
	}

	mem_init (mem);

	return (mem);
}

void mem_free (memory_t *mem)
{
	unsigned i;

	if (mem != NULL) {
		for (i = 0; i < mem->cnt; i++) {
			if (mem->lst[i].del) {
				mem_blk_del (mem->lst[i].blk);
			}
		}

		free (mem->lst);
	}
}

void mem_del (memory_t *mem)
{
	if (mem != NULL) {
		mem_free (mem);
		free (mem);
	}
}

void mem_set_fct (memory_t *mem, void *ext,
	void *g8, void *g16, void *g32, void *s8, void *s16, void *s32)
{
	mem->ext = ext;

	mem->get_uint8 = g8;
	mem->get_uint16 = g16;
	mem->get_uint32 = g32;

	mem->set_uint8 = s8;
	mem->set_uint16 = s16;
	mem->set_uint32 = s32;
}

void mem_set_default (memory_t *mem, unsigned char val)
{
	unsigned long tmp;

	tmp = val & 0xff;
	tmp = (tmp << 8) | tmp;
	tmp = (tmp << 16) | tmp;

	mem->defval = tmp;
}

void mem_prt_state (memory_t *mem, FILE *fp)
{
	unsigned  i;
	mem_lst_t *lst;

	for (i = 0; i < mem->cnt; i++) {
		lst = &mem->lst[i];

		fprintf (fp, "BLK %04X: A1=%08lX A2=%08lX S=%08lX RO=%d\n",
			i, lst->blk->addr1, lst->blk->addr2, lst->blk->size,
			(lst->blk->readonly != 0)
		);
	}
}

void mem_add_blk (memory_t *mem, mem_blk_t *blk, int del)
{
	mem_lst_t *lst;

	if (blk == NULL) {
		return;
	}

	lst = realloc (mem->lst, (mem->cnt + 1) * sizeof (mem_lst_t));
	if (lst == NULL) {
		return;
	}

	mem->lst = lst;

	lst += mem->cnt;
	mem->cnt += 1;

	lst->blk = blk;
	lst->del = (del != 0);

	mem_init_last (mem);
}

void mem_rmv_blk (memory_t *mem, const mem_blk_t *blk)
{
	unsigned  i, j;
	mem_lst_t *lst;

	i = 0;
	j = 0;
	lst = mem->lst;

	while (i < mem->cnt) {
		if (lst[i].blk != blk) {
			lst[j++] = lst[i];
		}

		i += 1;
	}

	mem->cnt = j;

	mem_init_last (mem);
}

void mem_rmv_all (memory_t *mem)
{
	unsigned i;

	for (i = 0; i < mem->cnt; i++) {
		if (mem->lst[i].del) {
			mem_blk_del (mem->lst[i].blk);
		}
	}

	mem->cnt = 0;

	mem_init_last (mem);
}

static inline
mem_blk_t *mem_get_blk_inline (memory_t *mem, unsigned long addr, unsigned last)
{
	unsigned  i;
	mem_blk_t *blk;
	mem_lst_t *lst;

	last &= (MEM_LAST_CNT - 1);

	if (mem->last[last] != NULL) {
		blk = mem->last[last]->blk;
		if (blk->active && (addr >= blk->addr1) && (addr <= blk->addr2)) {
			return (blk);
		}
	}

	lst = mem->lst;

	for (i = 0; i < mem->cnt; i++) {
		blk = lst->blk;
		if (blk->active && (addr >= blk->addr1) && (addr <= blk->addr2)) {
			mem->last[last] = lst;
			return (blk);
		}

		lst += 1;
	}

	return (NULL);
}

mem_blk_t *mem_get_blk (memory_t *mem, unsigned long addr)
{
	return (mem_get_blk_inline (mem, addr, 0));
}

unsigned char mem_get_uint8 (memory_t *mem, unsigned long addr)
{
	mem_blk_t *blk;

	blk = mem_get_blk_inline (mem, addr, 1);

	if (blk != NULL) {
		addr -= blk->addr1;

		if (blk->get_uint8 != NULL) {
			return (blk->get_uint8 (blk->ext, addr));
		}
		else {
			return (blk->data[addr]);
		}
	}

	if (mem->get_uint8 != NULL) {
		return (mem->get_uint8 (mem->ext, addr));
	}

	return (mem->defval & 0xff);
}

unsigned short mem_get_uint16_be (memory_t *mem, unsigned long addr)
{
	unsigned short val;
	mem_blk_t      *blk;

	blk = mem_get_blk_inline (mem, addr, 1);

	if (blk != NULL) {
		if ((addr + 1) > blk->addr2) {
			val = (unsigned short) mem_get_uint8 (mem, addr) << 8;
			val |= mem_get_uint8 (mem, addr + 1);
			return (val);
		}

		addr -= blk->addr1;

		if (blk->get_uint16 != NULL) {
			return (blk->get_uint16 (blk->ext, addr));
		}
		else {
			val = (unsigned short) blk->data[addr] << 8;
			val |= blk->data[addr + 1];
			return (val);
		}
	}

	if (mem->get_uint16 != NULL) {
		return (mem->get_uint16 (mem->ext, addr));
	}

	return (mem->defval & 0xffff);
}

unsigned short mem_get_uint16_le (memory_t *mem, unsigned long addr)
{
	unsigned short val;
	mem_blk_t      *blk;

	blk = mem_get_blk_inline (mem, addr, 1);

	if (blk != NULL) {
		if ((addr + 1) > blk->addr2) {
			val = mem_get_uint8 (mem, addr);
			val |= (unsigned short) mem_get_uint8 (mem, addr + 1) << 8;
			return (val);
		}

		addr -= blk->addr1;

		if (blk->get_uint16 != NULL) {
			return (blk->get_uint16 (blk->ext, addr));
		}
		else {
			val = blk->data[addr];
			val |= (unsigned short) blk->data[addr + 1] << 8;
			return (val);
		}
	}

	if (mem->get_uint16 != NULL) {
		return (mem->get_uint16 (mem->ext, addr));
	}

	return (mem->defval & 0xffff);
}

unsigned long mem_get_uint32_be (memory_t *mem, unsigned long addr)
{
	unsigned long val;
	mem_blk_t     *blk;

	blk = mem_get_blk_inline (mem, addr, 1);

	if (blk != NULL) {
		if ((addr + 3) > blk->addr2) {
			val = (unsigned long) mem_get_uint8 (mem, addr) << 24;
			val |= (unsigned long) mem_get_uint8 (mem, addr + 1) << 16;
			val |= (unsigned long) mem_get_uint8 (mem, addr + 2) << 8;
			val |= mem_get_uint8 (mem, addr + 3);
			return (val);
		}

		addr -= blk->addr1;

		if (blk->get_uint32 != NULL) {
			return (blk->get_uint32 (blk->ext, addr));
		}
		else {
			val = (unsigned long) blk->data[addr] << 24;
			val |= (unsigned long) blk->data[addr + 1] << 16;
			val |= (unsigned long) blk->data[addr + 2] << 8;
			val |= blk->data[addr + 3];
			return (val);
		}
	}

	if (mem->get_uint32 != NULL) {
		return (mem->get_uint32 (mem->ext, addr));
	}

	return (mem->defval);
}

unsigned long mem_get_uint32_le (memory_t *mem, unsigned long addr)
{
	unsigned long val;
	mem_blk_t     *blk;

	blk = mem_get_blk_inline (mem, addr, 1);

	if (blk != NULL) {
		if ((addr + 3) > blk->addr2) {
			val = (unsigned long) mem_get_uint8 (mem, addr);
			val |= (unsigned long) mem_get_uint8 (mem, addr + 1) << 8;
			val |= (unsigned long) mem_get_uint8 (mem, addr + 2) << 16;
			val |= (unsigned long) mem_get_uint8 (mem, addr + 3) << 24;
			return (val);
		}

		addr -= blk->addr1;

		if (blk->get_uint32 != NULL) {
			return (blk->get_uint32 (blk->ext, addr));
		}
		else {
			val = (unsigned long) blk->data[addr];
			val |= (unsigned long) blk->data[addr + 1] << 8;
			val |= (unsigned long) blk->data[addr + 2] << 16;
			val |= (unsigned long) blk->data[addr + 3] << 24;
			return (val);
		}
	}

	if (mem->get_uint32 != NULL) {
		return (mem->get_uint32 (mem->ext, addr));
	}

	return (mem->defval);
}

void mem_set_uint8_rw (memory_t *mem, unsigned long addr, unsigned char val)
{
	mem_blk_t *blk;

	blk = mem_get_blk (mem, addr);

	if (blk != NULL) {
		addr -= blk->addr1;

		if (blk->set_uint8 != NULL) {
			blk->set_uint8 (blk->ext, addr, val);
		}
		else {
			blk->data[addr] = val;
		}
	}
	else if (mem->set_uint8 != NULL) {
		mem->set_uint8 (mem->ext, addr, val);
	}
}

void mem_set_uint8 (memory_t *mem, unsigned long addr, unsigned char val)
{
	mem_blk_t *blk;

	blk = mem_get_blk_inline (mem, addr, 2);

	if (blk != NULL) {
		if (blk->readonly) {
			return;
		}

		addr -= blk->addr1;

		if (blk->set_uint8 != NULL) {
			blk->set_uint8 (blk->ext, addr, val);
		}
		else {
			blk->data[addr] = val;
		}
	}
	else if (mem->set_uint8 != NULL) {
		mem->set_uint8 (mem->ext, addr, val);
	}
}

void mem_set_uint16_be (memory_t *mem, unsigned long addr, unsigned short val)
{
	mem_blk_t *blk;

	blk = mem_get_blk_inline (mem, addr, 2);

	if (blk != NULL) {
		if ((addr + 1) > blk->addr2) {
			mem_set_uint8 (mem, addr, (val >> 8) & 0xff);
			mem_set_uint8 (mem, addr + 1, val & 0xff);
			return;
		}

		if (blk->readonly) {
			return;
		}

		addr -= blk->addr1;

		if (blk->set_uint16 != NULL) {
			blk->set_uint16 (blk->ext, addr, val);
		}
		else {
			blk->data[addr] = (val >> 8) & 0xff;
			blk->data[addr + 1] = val & 0xff;
		}
	}
	else if (mem->set_uint16 != NULL) {
		mem->set_uint16 (mem->ext, addr, val);
	}
}

void mem_set_uint16_le (memory_t *mem, unsigned long addr, unsigned short val)
{
	mem_blk_t *blk;

	blk = mem_get_blk_inline (mem, addr, 2);

	if (blk != NULL) {
		if ((addr + 1) > blk->addr2) {
			mem_set_uint8 (mem, addr, val & 0xff);
			mem_set_uint8 (mem, addr + 1, (val >> 8) & 0xff);
			return;
		}

		if (blk->readonly) {
			return;
		}

		addr -= blk->addr1;

		if (blk->set_uint16 != NULL) {
			blk->set_uint16 (blk->ext, addr, val);
		}
		else {
			blk->data[addr] = val & 0xff;
			blk->data[addr + 1] = (val >> 8) & 0xff;
		}
	}
	else if (mem->set_uint16 != NULL) {
		mem->set_uint16 (mem->ext, addr, val);
	}
}

void mem_set_uint32_be (memory_t *mem, unsigned long addr, unsigned long val)
{
	mem_blk_t *blk;

	blk = mem_get_blk_inline (mem, addr, 2);

	if (blk != NULL) {
		if ((addr + 3) > blk->addr2) {
			mem_set_uint8 (mem, addr, (val >> 24) & 0xff);
			mem_set_uint8 (mem, addr + 1, (val >> 16) & 0xff);
			mem_set_uint8 (mem, addr + 2, (val >> 8) & 0xff);
			mem_set_uint8 (mem, addr + 3, val & 0xff);
			return;
		}

		if (blk->readonly) {
			return;
		}

		addr -= blk->addr1;

		if (blk->set_uint32 != NULL) {
			blk->set_uint32 (blk->ext, addr, val);
		}
		else {
			blk->data[addr] = (val >> 24) & 0xff;
			blk->data[addr + 1] = (val >> 16) & 0xff;
			blk->data[addr + 2] = (val >> 8) & 0xff;
			blk->data[addr + 3] = val & 0xff;
		}
	}
	else if (mem->set_uint32 != NULL) {
		mem->set_uint32 (mem->ext, addr, val);
	}
}

void mem_set_uint32_le (memory_t *mem, unsigned long addr, unsigned long val)
{
	mem_blk_t *blk;

	blk = mem_get_blk_inline (mem, addr, 2);

	if (blk != NULL) {
		if ((addr + 3) > blk->addr2) {
			mem_set_uint8 (mem, addr, val & 0xff);
			mem_set_uint8 (mem, addr + 1, (val >> 8) & 0xff);
			mem_set_uint8 (mem, addr + 2, (val >> 16) & 0xff);
			mem_set_uint8 (mem, addr + 3, (val >> 24) & 0xff);
			return;
		}

		if (blk->readonly) {
			return;
		}

		addr -= blk->addr1;

		if (blk->set_uint32 != NULL) {
			blk->set_uint32 (blk->ext, addr, val);
		}
		else {
			blk->data[addr] = val & 0xff;
			blk->data[addr + 1] = (val >> 8) & 0xff;
			blk->data[addr + 2] = (val >> 16) & 0xff;
			blk->data[addr + 3] = (val >> 24) & 0xff;
		}
	}
	else if (mem->set_uint32 != NULL) {
		mem->set_uint32 (mem->ext, addr, val);
	}
}
