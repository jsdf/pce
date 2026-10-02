/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/char/char-wincom.c                               *
 * Created:     2009-03-07 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009-2015 Hampa Hug <hampa@hampa.ch>                     *
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

#include <drivers/options.h>
#include <drivers/char/char.h>
#include <drivers/char/char-wincom.h>

#include <windows.h>


static
void chr_wincom_close (char_drv_t *cdrv)
{
	char_wincom_t *drv;

	drv = cdrv->ext;

	if (drv->fname != NULL) {
		free (drv->fname);
	}

	if (drv->h != INVALID_HANDLE_VALUE) {
		CloseHandle (drv->h);
	}

	free (drv);
}

static
unsigned chr_wincom_read (char_drv_t *cdrv, void *buf, unsigned cnt)
{
	char_wincom_t *drv;
	DWORD         r;

	drv = cdrv->ext;

	if (drv->h == INVALID_HANDLE_VALUE) {
		return (0);
	}

	if (ReadFile (drv->h, buf, cnt, &r, NULL) == 0) {
		return (0);
	}

	return (r);
}

static
unsigned chr_wincom_write (char_drv_t *cdrv, const void *buf, unsigned cnt)
{
	char_wincom_t *drv;
	DWORD         r;

	drv = cdrv->ext;

	if (drv->h == INVALID_HANDLE_VALUE) {
		return (cnt);
	}

	if (WriteFile (drv->h, buf, cnt, &r, NULL) == 0) {
		return (0);
	}

	return (r);
}

static
int chr_wincom_set_params (char_drv_t *cdrv, unsigned long bps, unsigned bpc, unsigned parity, unsigned stop)
{
	char_wincom_t *drv;
	DCB           dcb;

	drv = cdrv->ext;

	if (drv->h == INVALID_HANDLE_VALUE) {
		return (0);
	}

	memset (&dcb, 0, sizeof (dcb));
	dcb.DCBlength = sizeof (dcb);

	if (GetCommState (drv->h, &dcb) == 0) {
		return (1);
	}

	dcb.BaudRate = bps;
	dcb.ByteSize = bpc;

	switch (stop) {
	case 1:
		dcb.StopBits = ONESTOPBIT;
		break;

	case 2:
		dcb.StopBits = TWOSTOPBITS;
		break;

	default:
		dcb.StopBits = ONESTOPBIT;
		break;
	}

	switch (parity) {
	case 0:
		dcb.Parity = NOPARITY;
		break;

	case 1:
		dcb.Parity = ODDPARITY;
		break;

	case 2:
		dcb.Parity = EVENPARITY;
		break;

	default:
		dcb.Parity = NOPARITY;
		break;
	}

	if (SetCommState (drv->h, &dcb) == 0) {
		return (1);
	}

	return (0);
}

static
int chr_wincom_get_ctl (char_drv_t *cdrv, unsigned *ctl)
{
	char_wincom_t *drv;
	DWORD         val;

	drv = cdrv->ext;

	if (drv->h != INVALID_HANDLE_VALUE) {
		if (GetCommModemStatus (drv->h, &val) == 0) {
			return (1);
		}
	}
	else {
		val = MS_DSR_ON | MS_CTS_ON | MS_RLSD_ON;
	}

	*ctl = 0;
	*ctl |= (val & MS_DSR_ON) ? PCE_CHAR_DSR : 0;
	*ctl |= (val & MS_CTS_ON) ? PCE_CHAR_CTS : 0;
	*ctl |= (val & MS_RLSD_ON) ? PCE_CHAR_CD : 0;
	*ctl |= (val & MS_RING_ON) ? PCE_CHAR_RI : 0;

	return (0);
}

static
int chr_wincom_set_ctl (char_drv_t *cdrv, unsigned ctl)
{
	char_wincom_t *drv;

	drv = cdrv->ext;

	if (drv->h == INVALID_HANDLE_VALUE) {
		return (0);
	}

	EscapeCommFunction (drv->h, (ctl & PCE_CHAR_DTR) ? SETDTR : CLRDTR);
	EscapeCommFunction (drv->h, (ctl & PCE_CHAR_RTS) ? SETRTS : CLRRTS);

	return (0);
}

static
int chr_wincom_set_device (char_wincom_t *drv, const char *name)
{
	COMMTIMEOUTS to;

	if (drv->h != INVALID_HANDLE_VALUE) {
		CloseHandle (drv->h);
	}

	drv->h = CreateFile (name,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);

	if (drv->h == INVALID_HANDLE_VALUE) {
		return (1);
	}

	GetCommTimeouts (drv->h, &to);

	to.ReadIntervalTimeout = MAXDWORD;
	to.ReadTotalTimeoutConstant = 0;
	to.ReadTotalTimeoutMultiplier = 0;

	to.WriteTotalTimeoutConstant = 1;
	to.WriteTotalTimeoutMultiplier = 0;

	SetCommTimeouts (drv->h, &to);

	return (0);
}

static
int chr_wincom_init (char_wincom_t *drv, const char *name)
{
	chr_init (&drv->cdrv, drv);

	drv->cdrv.close = chr_wincom_close;
	drv->cdrv.read = chr_wincom_read;
	drv->cdrv.write = chr_wincom_write;
	drv->cdrv.set_params = chr_wincom_set_params;
	drv->cdrv.get_ctl = chr_wincom_get_ctl;
	drv->cdrv.set_ctl = chr_wincom_set_ctl;

	drv->fname = drv_get_option (name, "port");

	drv->h = INVALID_HANDLE_VALUE;

	if (drv->fname != NULL) {
		if (chr_wincom_set_device (drv, drv->fname)) {
			return (1);
		}
	}

	return (0);
}

char_drv_t *chr_wincom_open (const char *name)
{
	char_wincom_t *drv;

	drv = malloc (sizeof (char_wincom_t));

	if (drv == NULL) {
		return (NULL);
	}

	if (chr_wincom_init (drv, name)) {
		free (drv);
		return (NULL);
	}

	return (&drv->cdrv);
}
