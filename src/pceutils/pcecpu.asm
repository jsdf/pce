;*****************************************************************************
;* pce                                                                       *
;*****************************************************************************

;*****************************************************************************
;* File name:     pcecpu.asm                                                 *
;* Created:       2003-09-22 by Hampa Hug <hampa@hampa.ch>                   *
;* Last modified: 2003-09-22 by Hampa Hug <hampa@hampa.ch>                   *
;* Copyright:     (C) 2003 by Hampa Hug <hampa@hampa.ch>                     *
;*****************************************************************************

;*****************************************************************************
;* This program is free software. You can redistribute it and / or modify it *
;* under the terms of the GNU General Public License version 2 as  published *
;* by the Free Software Foundation.                                          *
;*                                                                           *
;* This program is distributed in the hope  that  it  will  be  useful,  but *
;* WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
;* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General *
;* Public License for more details.                                          *
;*****************************************************************************

; $Id: pcecpu.asm,v 1.2 2003/10/07 16:05:25 hampa Exp $

; pcecpu [cpu]


%include "pce.inc"


section .text

  org     0x100

  jmp     start


msg_info1      db "current cpu type is ", 0
msg_info2      db 13, 10, 0

msg_help       db "usage: pcecpu [cpu]", 13, 10, 0

str_empty      db 0
str_8086       db "8086", 0
str_8088       db "8088", 0
str_v20        db "v20", 0
str_v30        db "v30", 0
str_80186      db "80186", 0
str_80188      db "80188", 0
str_80286      db "80286", 0
str_unknown    db "unknown", 0


prt_string:
  push    ax
  push    si

.next:
  mov     ah, 0x0e
  lodsb
  or      al, al
  jz      .done
  int     0x10
  jmp     .next

.done:
  pop     si
  pop     ax
  ret


is_param:
  push    cx
  push    si
  push    di

  jcxz    .no

.skip:
  mov     al, [si]
  cmp     al, ' '
  jne     .next
  inc     si
  loop    .skip

  jmp     .no

.next:
  mov     ah, [di]
  or      ah, ah
  jz      .yes

  mov     al, [si]
  cmp     al, ah
  jne     .no

  inc     si
  inc     di
  loop    .next

  cmp     byte [di], 0x00
  je      .yes

.no:
  pop     di
  pop     si
  pop     cx
  xor     ax, ax
  ret

.yes:
  pop     di
  pop     ax
  pop     ax
  mov     ax, 1
  ret


start:
  mov     ax, cs
  mov     ds, ax

  mov     si, 0x0080                    ; SI points to parameters

  lodsb
  mov     ah, 0
  mov     cx, ax                        ; parameter size in CX

  mov     di, str_empty
  call    is_param
  jcxz    do_info

  mov     di, str_8086
  call    is_param
  or      ax, ax
  jnz     set_8086

  mov     di, str_8088
  call    is_param
  or      ax, ax
  jnz     set_8088

  mov     di, str_v20
  call    is_param
  or      ax, ax
  jnz     set_v20

  mov     di, str_v30
  call    is_param
  or      ax, ax
  jnz     set_v30

  mov     di, str_80186
  call    is_param
  or      ax, ax
  jnz     set_80186

  mov     di, str_80188
  call    is_param
  or      ax, ax
  jnz     set_80188

  mov     di, str_80286
  call    is_param
  or      ax, ax
  jnz     set_80286

  jmp     do_help

set_8086:
  mov     ax, PCE_CPU_8086
  jmp     setok

set_8088:
  mov     ax, PCE_CPU_8088
  jmp     setok

set_v20:
  mov     ax, PCE_CPU_V20
  jmp     setok

set_v30:
  mov     ax, PCE_CPU_V30
  jmp     setok

set_80186:
  mov     ax, PCE_CPU_80186
  jmp     setok

set_80188:
  mov     ax, PCE_CPU_80188
  jmp     setok

set_80286:
  mov     ax, PCE_CPU_80286
  jmp     setok

setok:
  pceh    PCEH_SET_CPU
  jmp     done


do_info:
  mov     si, msg_info1
  call    prt_string

  pceh    PCEH_GET_CPU

  mov     si, str_8086
  cmp     ax, PCE_CPU_8086
  je      .ok

  mov     si, str_8088
  cmp     ax, PCE_CPU_8088
  je      .ok

  mov     si, str_v20
  cmp     ax, PCE_CPU_V20
  je      .ok

  mov     si, str_v30
  cmp     ax, PCE_CPU_V30
  je      .ok

  mov     si, str_80186
  cmp     ax, PCE_CPU_80186
  je      .ok

  mov     si, str_80188
  cmp     ax, PCE_CPU_80188
  je      .ok

  mov     si, str_80286
  cmp     ax, PCE_CPU_80286
  je      .ok

  mov     si, str_unknown

.ok:
  call    prt_string

  mov     si, msg_info2
  call    prt_string

  jmp     done


do_help:
  mov     si, msg_help
  call    prt_string


done:
  mov     ax, 0x4c00
  int     0x21
