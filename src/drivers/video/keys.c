/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/video/keys.c                                     *
 * Created:     2007-11-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2012 Hampa Hug <hampa@hampa.ch>                     *
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

#include <drivers/video/keys.h>


typedef struct {
	pce_key_t  key;
	const char *name;
} pce_key_name_t;


/*
 * This table maps key names to key symbols and vice versa
 */
static
pce_key_name_t keyname[] = {
	{ PCE_KEY_ESC, "ESC", },

	{ PCE_KEY_F1, "F1" },
	{ PCE_KEY_F2, "F2" },
	{ PCE_KEY_F3, "F3" },
	{ PCE_KEY_F4, "F4" },
	{ PCE_KEY_F5, "F5" },
	{ PCE_KEY_F6, "F6" },
	{ PCE_KEY_F7, "F7" },
	{ PCE_KEY_F8, "F8" },
	{ PCE_KEY_F9, "F9" },
	{ PCE_KEY_F10, "F10" },
	{ PCE_KEY_F11, "F11" },
	{ PCE_KEY_F12, "F12" },

	{ PCE_KEY_PRTSCN, "PrintScreen" },
	{ PCE_KEY_PRTSCN, "PrtScn" },
	{ PCE_KEY_SCRLK, "ScrollLock" },
	{ PCE_KEY_SCRLK, "ScrLk" },
	{ PCE_KEY_PAUSE, "Pause" },

	{ PCE_KEY_BACKQUOTE, "Backquote" },
	{ PCE_KEY_BACKQUOTE, "`" },
	{ PCE_KEY_1, "1" },
	{ PCE_KEY_2, "2" },
	{ PCE_KEY_3, "3" },
	{ PCE_KEY_4, "4" },
	{ PCE_KEY_5, "5" },
	{ PCE_KEY_6, "6" },
	{ PCE_KEY_7, "7" },
	{ PCE_KEY_8, "8" },
	{ PCE_KEY_9, "9" },
	{ PCE_KEY_0, "0" },
	{ PCE_KEY_MINUS, "Minus" },
	{ PCE_KEY_MINUS, "-" },
	{ PCE_KEY_EQUAL, "Equal" },
	{ PCE_KEY_EQUAL, "=" },
	{ PCE_KEY_BACKSPACE, "Backspace" },

	{ PCE_KEY_TAB, "Tab" },
	{ PCE_KEY_Q, "q" },
	{ PCE_KEY_W, "w" },
	{ PCE_KEY_E, "e" },
	{ PCE_KEY_R, "r" },
	{ PCE_KEY_T, "t" },
	{ PCE_KEY_Y, "y" },
	{ PCE_KEY_U, "u" },
	{ PCE_KEY_I, "I" },
	{ PCE_KEY_O, "o" },
	{ PCE_KEY_P, "p" },
	{ PCE_KEY_LBRACKET, "LeftBracket" },
	{ PCE_KEY_LBRACKET, "[" },
	{ PCE_KEY_RBRACKET, "RightBracket" },
	{ PCE_KEY_RBRACKET, "]" },
	{ PCE_KEY_RETURN, "Return" },

	{ PCE_KEY_CAPSLOCK, "CapsLock" },
	{ PCE_KEY_A, "a" },
	{ PCE_KEY_S, "s" },
	{ PCE_KEY_D, "d" },
	{ PCE_KEY_F, "f" },
	{ PCE_KEY_G, "g" },
	{ PCE_KEY_H, "h" },
	{ PCE_KEY_J, "j" },
	{ PCE_KEY_K, "k" },
	{ PCE_KEY_L, "l" },
	{ PCE_KEY_SEMICOLON, "Semicolon" },
	{ PCE_KEY_SEMICOLON, ";" },
	{ PCE_KEY_QUOTE, "Quote" },
	{ PCE_KEY_QUOTE, "Apostrophe" },
	{ PCE_KEY_QUOTE, "'" },
	{ PCE_KEY_BACKSLASH, "Backslash" },
	{ PCE_KEY_BACKSLASH, "\\" },

	{ PCE_KEY_LSHIFT, "ShiftLeft" },
	{ PCE_KEY_LSHIFT, "Shift" },
	{ PCE_KEY_LESS, "Less" },
	{ PCE_KEY_LESS, "<" },
	{ PCE_KEY_Z, "z" },
	{ PCE_KEY_X, "x" },
	{ PCE_KEY_C, "c" },
	{ PCE_KEY_V, "v" },
	{ PCE_KEY_B, "b" },
	{ PCE_KEY_N, "n" },
	{ PCE_KEY_M, "m" },
	{ PCE_KEY_COMMA, "Comma" },
	{ PCE_KEY_COMMA, "," },
	{ PCE_KEY_PERIOD, "Period" },
	{ PCE_KEY_PERIOD, "." },
	{ PCE_KEY_SLASH, "Slash" },
	{ PCE_KEY_SLASH, "/" },

	{ PCE_KEY_LCTRL, "CtrlLeft" },
	{ PCE_KEY_LCTRL, "Ctrl" },
	{ PCE_KEY_LMETA, "MetaLeft" },
	{ PCE_KEY_LMETA, "Meta" },
	{ PCE_KEY_LSUPER, "SuperLeft" },
	{ PCE_KEY_LSUPER, "StartLeft" },
	{ PCE_KEY_LSUPER, "WindowsLeft" },
	{ PCE_KEY_MODE, "Mode" },
	{ PCE_KEY_LALT, "AltLeft" },
	{ PCE_KEY_LALT, "Alt" },
	{ PCE_KEY_SPACE, "Space" },
	{ PCE_KEY_RALT, "AltRight" },
	{ PCE_KEY_RMETA, "MetaRight" },
	{ PCE_KEY_RSUPER, "SuperRight" },
	{ PCE_KEY_RSUPER, "StartRight" },
	{ PCE_KEY_RSUPER, "WindowsRight" },
	{ PCE_KEY_MENU, "Menu" },
	{ PCE_KEY_RCTRL, "CtrlRight" },

	{ PCE_KEY_NUMLOCK, "NumLock" },
	{ PCE_KEY_KP_SLASH, "KP_Slash" },
	{ PCE_KEY_KP_STAR, "KP_Star" },
	{ PCE_KEY_KP_MINUS, "KP_Minus" },
	{ PCE_KEY_KP_7, "KP_7" },
	{ PCE_KEY_KP_8, "KP_8" },
	{ PCE_KEY_KP_9, "KP_9" },
	{ PCE_KEY_KP_PLUS, "KP_Plus" },
	{ PCE_KEY_KP_4, "KP_4" },
	{ PCE_KEY_KP_5, "KP_5" },
	{ PCE_KEY_KP_6, "KP_6" },
	{ PCE_KEY_KP_1, "KP_1" },
	{ PCE_KEY_KP_2, "KP_2" },
	{ PCE_KEY_KP_3, "KP_3" },
	{ PCE_KEY_KP_ENTER, "KP_Enter" },
	{ PCE_KEY_KP_0, "KP_0" },
	{ PCE_KEY_KP_PERIOD, "KP_Period" },

	{ PCE_KEY_INS, "Insert" },
	{ PCE_KEY_HOME, "Home" },
	{ PCE_KEY_PAGEUP, "PageUp" },
	{ PCE_KEY_DEL, "Delete" },
	{ PCE_KEY_END, "End" },
	{ PCE_KEY_PAGEDN, "PageDown" },
	{ PCE_KEY_UP, "Up" },
	{ PCE_KEY_LEFT, "Left" },
	{ PCE_KEY_DOWN, "Down" },
	{ PCE_KEY_RIGHT, "Right" },

	{ PCE_KEY_EXTRA1, "Extra1" },
	{ PCE_KEY_EXTRA2, "Extra2" },
	{ PCE_KEY_EXTRA3, "Extra3" },
	{ PCE_KEY_EXTRA4, "Extra4" },
	{ PCE_KEY_EXTRA5, "Extra5" },
	{ PCE_KEY_EXTRA6, "Extra6" },
	{ PCE_KEY_EXTRA7, "Extra7" },
	{ PCE_KEY_EXTRA8, "Extra8" },
	{ PCE_KEY_EXTRA9, "Extra9" },
	{ PCE_KEY_EXTRA10, "Extra10" },
	{ PCE_KEY_EXTRA11, "Extra11" },
	{ PCE_KEY_EXTRA12, "Extra12" },
	{ PCE_KEY_EXTRA13, "Extra13" },
	{ PCE_KEY_EXTRA14, "Extra14" },
	{ PCE_KEY_EXTRA15, "Extra15" },
	{ PCE_KEY_EXTRA16, "Extra16" },

	{ PCE_KEY_NONE, NULL }
};


pce_key_t pce_key_from_string (const char *str)
{
	pce_key_name_t *el;

	el = keyname;

	while (el->name != NULL) {
		if (strcasecmp (el->name, str) == 0) {
			return (el->key);
		}

		el += 1;
	}

	return (PCE_KEY_NONE);
}

const char *pce_key_to_string (pce_key_t key)
{
	pce_key_name_t *el;

	el = keyname;

	while (el->name != NULL) {
		if (el->key == key) {
			return (el->name);
		}

		el += 1;
	}

	return (NULL);
}

int pce_key_get_map (const char *map, unsigned long *src, pce_key_t *dst)
{
	unsigned i;
	char     val[32];

	while ((*map == ' ') || (*map == '\t')) {
		map += 1;
	}

	i = 0;
	while ((i < 32) && (*map != 0) && (*map != '=') && (*map != ' ')) {
		val[i++] = *(map++);
	}

	if (i >= 32) {
		return (1);
	}

	val[i] = 0;

	while (*map == ' ') {
		map += 1;
	}

	if (*map == '=') {
		map += 1;

		while (*map == ' ') {
			map += 1;
		}
	}

	*src = strtoul (val, NULL, 0);
	*dst = pce_key_from_string (map);

	return (0);
}
