;*****************************************************************************
;* pce                                                                       *
;*****************************************************************************

;*****************************************************************************
;* File name:     pceint28.asm                                               *
;* Created:       2003-09-18 by Hampa Hug <hampa@hampa.ch>                   *
;* Last modified: 2003-09-18 by Hampa Hug <hampa@hampa.ch>                   *
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

; $Id: pceint28.asm,v 1.1 2003/12/20 01:01:36 hampa Exp $

; pceint28 [on|off]


%include "pce.inc"


section .text

  org     0x100

  jmp     start


msg_info       db "INT 28 sleeping is", 0
msg_on         db " ON", 13, 10, 0
msg_off        db " OFF", 13, 10, 0
msg_help       db "usage: pceint28 [on|off]", 13, 10, 0

str_empty      db 0
str_on         db "on", 0
str_off        db "off", 0


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

  mov     di, str_on
  call    is_param
  or      ax, ax
  jnz     do_on

  mov     di, str_off
  call    is_param
  or      ax, ax
  jnz     do_off


do_help:
  mov     si, msg_help
  call    prt_string
  jmp     done


do_on:
  mov     ax, 10
  pceh    PCEH_SET_INT28
  jmp     done


do_off:
  xor     ax, ax
  pceh    PCEH_SET_INT28
  jmp     done


do_info:
  mov     si, msg_info
  call    prt_string

  pceh    PCEH_GET_INT28

  mov     si, msg_on
  or      ax, ax
  jnz     .on

  mov     si, msg_off

.on:
  call    prt_string


done:
  mov     ax, 0x4c00
  int     0x21
