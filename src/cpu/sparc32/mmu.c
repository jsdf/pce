/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/sparc32/mmu.c                                        *
 * Created:     2004-09-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdio.h>
#include <stdlib.h>

#include "sparc32.h"
#include "internal.h"


int s32_ifetch (sparc32_t *c, uint32_t addr, uint8_t asi, uint32_t *val)
{
	addr &= ~0x03UL;

	s32_set_asi (c, asi);

	if (c->get_uint32 != NULL) {
		*val = c->get_uint32 (c->mem_ext, addr);
	}
	else {
		*val = 0xffffffffUL;
	}

	return (0);
}

int s32_dload8 (sparc32_t *c, uint32_t addr, uint8_t asi, uint8_t *val)
{
	s32_set_asi (c, asi);

	if (c->get_uint8 != NULL) {
		*val = c->get_uint8 (c->mem_ext, addr);
	}
	else {
		*val = 0xff;
	}

	return (0);
}

int s32_dload16 (sparc32_t *c, uint32_t addr, uint8_t asi, uint16_t *val)
{
	s32_set_asi (c, asi);

	if (c->get_uint16 != NULL) {
		*val = c->get_uint16 (c->mem_ext, addr);
	}
	else {
		*val = 0xffffU;
	}

	return (0);
}

int s32_dload32 (sparc32_t *c, uint32_t addr, uint8_t asi, uint32_t *val)
{
	s32_set_asi (c, asi);

	if (c->get_uint32 != NULL) {
		*val = c->get_uint32 (c->mem_ext, addr);
	}
	else {
		*val = 0xffffffffUL;
	}

	return (0);
}

int s32_dstore8 (sparc32_t *c, uint32_t addr, uint8_t asi, uint8_t val)
{
	s32_set_asi (c, asi);

	if (c->set_uint8 != NULL) {
		c->set_uint8 (c->mem_ext, addr, val);
	}

	return (0);
}

int s32_dstore16 (sparc32_t *c, uint32_t addr, uint8_t asi, uint16_t val)
{
	s32_set_asi (c, asi);

	if (c->set_uint16 != NULL) {
		c->set_uint16 (c->mem_ext, addr, val);
	}

	return (0);
}

int s32_dstore32 (sparc32_t *c, uint32_t addr, uint8_t asi, uint32_t val)
{
	s32_set_asi (c, asi);

	if (c->set_uint32 != NULL) {
		c->set_uint32 (c->mem_ext, addr, val);
	}

	return (0);
}


int s32_get_mem8 (sparc32_t *c, uint32_t addr, uint8_t asi, unsigned xlat, uint8_t *val)
{
	s32_set_asi (c, asi);

	if (c->get_uint8 != NULL) {
		*val = c->get_uint8 (c->mem_ext, addr);
	}
	else {
		*val = 0xff;
	}

	return (0);
}

int s32_get_mem16 (sparc32_t *c, uint32_t addr, uint8_t asi, unsigned xlat, uint16_t *val)
{
	s32_set_asi (c, asi);

	if (c->get_uint16 != NULL) {
		*val = c->get_uint16 (c->mem_ext, addr);
	}
	else {
		*val = 0xffffU;
	}

	return (0);
}

int s32_get_mem32 (sparc32_t *c, uint32_t addr, uint8_t asi, unsigned xlat, uint32_t *val)
{
	s32_set_asi (c, asi);

	if (c->get_uint32 != NULL) {
		*val = c->get_uint32 (c->mem_ext, addr);
	}
	else {
		*val = 0xffffffffUL;
	}

	return (0);
}

int s32_set_mem8 (sparc32_t *c, uint32_t addr, uint8_t asi, unsigned xlat, uint8_t val)
{
	s32_set_asi (c, asi);

	if (c->set_uint8 != NULL) {
		c->set_uint8 (c->mem_ext, addr, val);
	}

	return (0);
}

int s32_set_mem16 (sparc32_t *c, uint32_t addr, uint8_t asi, unsigned xlat, uint16_t val)
{
	s32_set_asi (c, asi);

	if (c->set_uint16 != NULL) {
		c->set_uint16 (c->mem_ext, addr, val);
	}

	return (0);
}

int s32_set_mem32 (sparc32_t *c, uint32_t addr, uint8_t asi, unsigned xlat, uint32_t val)
{
	s32_set_asi (c, asi);

	if (c->set_uint32 != NULL) {
		c->set_uint32 (c->mem_ext, addr, val);
	}

	return (0);
}
