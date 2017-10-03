;-----------------------------------------------------------------------------
; pce
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; File name:    src/arch/ibmpc/pce.inc
; Created:      2003-09-02 by Hampa Hug <hampa@hampa.ch>
; Copyright:    (C) 2003-2017 Hampa Hug <hampa@hampa.ch>
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


%define PCEH_STOP      0x0000
%define PCEH_ABORT     0x0100

%define PCEH_SET_BOOT  0x0001
%define PCEH_SET_INT28 0x0401
%define PCEH_SET_CPU   0x0501
%define PCEH_SET_AMSK  0x0601

%define PCEH_GET_BOOT  0x0002
%define PCEH_GET_COM   0x0102
%define PCEH_GET_LPT   0x0202
%define PCEH_GET_INT28 0x0402
%define PCEH_GET_CPU   0x0502
%define PCEH_GET_AMSK  0x0602
%define PCEH_GET_VERS  0x0702
%define PCEH_GET_FDCNT 0x0802
%define PCEH_GET_HDCNT 0x0902

%define PCEH_XMS       0x0003
%define PCEH_XMS_INFO  0x0103

%define PCEH_EMS       0x0004
%define PCEH_EMS_INFO  0x0104

%define PCEH_MSG       0x0005

%define PCEH_CHECK_INT 0x0006

%define PCEH_INT       0x00cd


%macro pceh 1
	db	0x66, 0x66, (%1 & 0xff), (%1 >> 8) & 0xff
%endmacro

%macro pceh 2
	db	0x66, 0x66, %1, %2
%endmacro


%macro set_pos 1
	times %1 - ($ - $$) db 0
%endmacro


%define PCE_HOOK_CHECK           0x0000
%define PCE_HOOK_GET_VERSION     0x0001
%define PCE_HOOK_GET_VERSION_STR 0x0002
%define PCE_HOOK_STOP            0x0100
%define PCE_HOOK_ABORT           0x0101
%define PCE_HOOK_SET_MSG         0x0200
%define PCE_HOOK_GET_TIME_UNIX   0x0300
%define PCE_HOOK_GET_TIME_UTC    0x0301
%define PCE_HOOK_GET_TIME_LOCAL  0x0302
%define PCE_HOOK_XMS             0x0400
%define PCE_HOOK_XMS_INFO        0x0401
%define PCE_HOOK_EMS             0x0500
%define PCE_HOOK_EMS_INFO        0x0501


%endif
