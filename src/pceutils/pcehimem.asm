;*****************************************************************************
;* pce                                                                       *
;*****************************************************************************

;*****************************************************************************
;* File name:     pcehimem.asm                                               *
;* Created:       2003-09-02 by Hampa Hug <hampa@hampa.ch>                   *
;* Last modified: 2003-09-02 by Hampa Hug <hampa@hampa.ch>                   *
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

; $Id: pcehimem.asm,v 1.1 2003/09/02 11:44:47 hampa Exp $


%include "config.inc"


section text

  dd      -1                            ; link to next driver
  dw      0x8000                        ; flags
  dw      strategy                      ; strategy offset
  dw      interrupt                     ; interrupt offset
  db      "XMSXXXX0"                    ; driver name

reqadr         dd 0

saveint2f      dd 0


;*****************************************************************************
;* int 2f handler
;*****************************************************************************

int_2f:
  cmp     ax, 0x4300
  jne     .not4300

  ; check if XMS driver is installed
  mov     al, 0x80
  iret

.not4300:
  cmp     ax, 0x4310
  jne     .done

  ; get XMS driver entry point
  mov     bx, cs
  mov     es, bx
  mov     bx, xms_handler
  iret

.done
  jmp     far [cs:saveint2f]


;*****************************************************************************
;* XMS handler
;*****************************************************************************

xms_handler:
  jmp     short .handler
  nop
  nop
  nop

.handler:
  ; xms hook
  db      0x66, 0x66, 0x03, 0x00
  retf


;*****************************************************************************
;* driver functions
;*****************************************************************************

strategy:
  mov     [cs:reqadr], bx
  mov     [cs:reqadr + 2], es
  retf


interrupt:
  pushf
  push    ax
  push    cx
  push    dx
  push    bx
  push    ds

  lds     bx, [cs:reqadr]

  mov     ah, [bx + 2]

  or      ah, ah
  jne     .not00

  call    drv_init
  jmp     .done

.not00:

  ; unknown command
  mov     word [bx + 3], 0x8103

.done:
  pop     ds
  pop     bx
  pop     dx
  pop     cx
  pop     ax
  popf
  retf


drv_end:


;*****************************************************************************
;* func 00: init driver
;*****************************************************************************

drv_init:
  push    si

  mov     si, msg_init
  call    prt_string

  ; xms info
  db      0x66, 0x66, 0x03, 0x01

  or      ax, ax
  jnz     .xmsok

  mov     si, msg_noxms
  call    prt_string

  jmp     .doneerr

.xmsok:
  mov     ax, dx
  mov     si, msg_himem
  call    prt_string
  call    prt_uint16
  mov     si, msg_avail
  call    prt_string

  push    ds
  xor     ax, ax
  mov     ds, ax

  mov     ax, [4 * 0x002f]
  mov     [cs:saveint2f], ax
  mov     ax, [4 * 0x002f + 2]
  mov     [cs:saveint2f + 2], ax

  mov     word [4 * 0x002f], int_2f
  mov     word [4 * 0x002f + 2], cs
  pop     ds

  jmp     .doneok

.doneerr:
  ; end address
  mov     word [bx + 14], 0x0000
  mov     word [bx + 16], cs

  ; status
  mov     word [bx + 3], 0x8100
  jmp     .done

.doneok:
  ; end address
  mov     word [bx + 14], drv_end
  mov     word [bx + 16], cs

  ; status
  mov     word [bx + 3], 0x0100

.done:
  ; param address
  mov     word [bx + 18], 0
  mov     word [bx + 20], 0

  ; message flag
  mov     word [bx + 23], 0x0000

  pop     si
  ret


prt_string:
  push    ax
  push    bx
  push    si

.next:
  mov     ah, 0x0e
  mov     bx, 0x0007
  mov     al, [cs:si]
  or      al, al
  jz      .done
  int     0x10
  inc     si
  jmp     .next

.done:
  pop     si
  pop     bx
  pop     ax
  ret


prt_uint16:
  push    ax
  push    cx
  push    dx
  push    bx

  xor     cx, cx
  mov     bx, 10

.next1:
  xor     dx, dx
  div     bx
  push    dx
  inc     cx
  or      ax, ax
  jnz     .next1

.next2:
  pop     ax
  mov     ah, 0x0e
  add     al, '0'
  mov     bx, 0x0007
  int     0x10
  loop    .next2

  pop     bx
  pop     dx
  pop     cx
  pop     ax
  ret


msg_init       db "HIMEM: PCE XMS Driver version ", PCE_VERSION_STR
               db " (", PCE_CFG_DATE, " ", PCE_CFG_TIME, ")"
               db 0x0d, 0x0a, 0x00

msg_noxms      db "HIMEM: Error: no XMS available", 0x0d, 0x0a, 0x00

msg_himem      db "HIMEM: ", 0x00

msg_avail     db "K XMS available", 0x0d, 0x0a, 0x00
