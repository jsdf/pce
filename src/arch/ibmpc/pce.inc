;-----------------------------------------------------------------------------
; pce
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; File name:    src/arch/ibmpc/pce.inc
; Created:      2003-09-02 by Hampa Hug <hampa@hampa.ch>
; Copyright:    (C) 2003-2022 Hampa Hug <hampa@hampa.ch>
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; This program is free software. You can redistribute it and / or modify it
; under the terms of the GNU General Public License version 2 as  published
; by the Free Software Foundation.
;
; This program is distributed in the hope  that  it  will  be  useful,  but
; WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General
; Public License for more details.
;-----------------------------------------------------------------------------


%ifndef PCE_INC
%define PCE_INC 1


%include "config.inc"


%define PCE_BRK_STOP  1
%define PCE_BRK_ABORT 2


%define PCE_HOOK_CHECK           0x0000
%define PCE_HOOK_GET_VERSION     0x0001
%define PCE_HOOK_GET_VERSION_STR 0x0002
%define PCE_HOOK_STOP            0x0100
%define PCE_HOOK_ABORT           0x0101
%define PCE_HOOK_SET_MSG         0x0200
%define PCE_HOOK_GET_TIME_UNIX   0x0300
%define PCE_HOOK_GET_TIME_UTC    0x0301
%define PCE_HOOK_GET_TIME_LOCAL  0x0302
%define PCE_HOOK_GET_TIME_BIOS   0x0303
%define PCE_HOOK_XMS             0x0400
%define PCE_HOOK_XMS_INFO        0x0401
%define PCE_HOOK_EMS             0x0500
%define PCE_HOOK_EMS_INFO        0x0501
%define PCE_HOOK_INT             0x0600
%define PCE_HOOK_GET_MODEL       0x0700
%define PCE_HOOK_GET_COM         0x0701
%define PCE_HOOK_GET_LPT         0x0702
%define PCE_HOOK_GET_BOOT        0x0703
%define PCE_HOOK_GET_DRVCNT      0x0704


%macro pce 0
	db	0xd5, 0x00
%endmacro

%macro pce 1
	mov	ax, %1
	db	0xd5, 0x00
%endmacro

%macro pceint 2
	push	ax
	mov	ax, PCE_HOOK_INT | (%1)
	db	0xd5, 0x00
	jmp	short %2
%endmacro

%macro set_pos 1
	times %1 - ($ - $$) db 0
%endmacro


%endif
