;*****************************************************************************
;* pce                                                                       *
;*****************************************************************************

;*****************************************************************************
;* File name:     basic.asm                                                  *
;* Created:       2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
;* Last modified: 2003-04-15 by Hampa Hug <hampa@hampa.ch>                   *
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

; $Id: basic.asm,v 1.2 2003/04/15 04:14:00 hampa Exp $


section .text

  jmp     L_7E92

db 0xE8, 0xA7, 0x6B                     ; 0003 call 0x6bad
db 0xCB                                 ; 0006 retf
db 0xE8, 0x02, 0x65                     ; 0007 call 0x650c
db 0xCB                                 ; 000A retf
db 0x5D                                 ; 000B pop bp
db 0xE8, 0xC7, 0x2F                     ; 000C call 0x2fd6
db 0x74, 0x0D                           ; 000F jz 0x1e
db 0x8B, 0x36, 0xE9, 0x04               ; 0011 mov si,[0x4e9]
db 0x8A, 0x44, 0x2E                     ; 0015 mov al,[si+0x2e]
db 0x3C, 0xFE                           ; 0018 cmp al,0xfe
db 0x74, 0x02                           ; 001A jz 0x1e
db 0x3C, 0xFD                           ; 001C cmp al,0xfd
db 0xC3                                 ; 001E ret
db 0x00, 0x00                           ; 001F add [bx+si],al
db 0x00, 0x00                           ; 0021 add [bx+si],al
db 0x00, 0x00                           ; 0023 add [bx+si],al
db 0x33, 0x2E, 0x94, 0x0D               ; 0025 xor bp,[0xd94]
db 0x68, 0x73, 0x5B                     ; 0029 push word 0x5b73
db 0x11, 0x59, 0x15                     ; 002C adc [bx+di+0x15],bx
db 0x66, 0x37                           ; 002F o32 aaa
db 0x3E, 0x16                           ; 0031 ds push ss
db 0x8F                                 ; 0033 db 0x8F
db 0x11, 0xED                           ; 0034 adc bp,bp
db 0x10, 0xB6, 0x10, 0xF8               ; 0036 adc [bp+0xf810],dh
db 0x12, 0x0C                           ; 003A adc cl,[si]
db 0x2E, 0xCE                           ; 003C cs into
db 0x10, 0x35                           ; 003E adc [di],dh
db 0x11, 0x5F, 0x11                     ; 0040 adc [bx+0x11],bx
db 0x2D, 0x2E, 0x48                     ; 0043 sub ax,0x482e
db 0x13, 0x50, 0x2F                     ; 0046 adc dx,[bx+si+0x2f]
db 0x35, 0x1F, 0x1B                     ; 0049 xor ax,0x1b1f
db 0x2D, 0xFE, 0x11                     ; 004C sub ax,0x11fe
db 0x58                                 ; 004F pop ax
db 0x1E                                 ; 0050 push ds
db 0xF0, 0x1B, 0x91, 0x22, 0x85         ; 0051 lock sbb dx,[bx+di+0x8522]
db 0x2E, 0xBE, 0x07, 0xBE               ; 0056 cs mov si,0xbe07
db 0x07                                 ; 005A pop es
db 0x53                                 ; 005B push bx
db 0x1E                                 ; 005C push ds
db 0x43                                 ; 005D inc bx
db 0x13, 0x2E, 0x1F, 0x00               ; 005E adc bp,[0x1f]
db 0x00, 0x8A, 0x1E, 0x5F               ; 0062 add [bp+si+0x5f1e],cl
db 0x11, 0x9C, 0x2E, 0x9D               ; 0066 adc [si+0x9d2e],bx
db 0x2E, 0xA3, 0x2E, 0xF3               ; 006A mov [cs:0xf32e],ax
db 0x2E, 0xDF, 0x36, 0xA9, 0x12         ; 006E fbstp tword [cs:0x12a9]
db 0x58                                 ; 0073 pop ax
db 0x12, 0x39                           ; 0074 adc bh,[bx+di]
db 0x22, 0xB8, 0x12, 0xD8               ; 0076 and bh,[bx+si+0xd812]
db 0x22, 0xFB                           ; 007A and bh,bl
db 0x0F, 0xFE, 0x0F                     ; 007C paddd mm1,[bx]
db 0x01, 0x10                           ; 007F add [bx+si],dx
db 0x04, 0x10                           ; 0081 add al,0x10
db 0xD9, 0x14                           ; 0083 fst dword [si]
db 0x04, 0x3D                           ; 0085 add al,0x3d
db 0x30, 0x3D                           ; 0087 xor [di],bh
db 0xA0, 0x5D, 0x00                     ; 0089 mov al,[0x5d]
db 0x00, 0x00                           ; 008C add [bx+si],al
db 0x00, 0x00                           ; 008E add [bx+si],al
db 0x00, 0xA9, 0x3D, 0x3F               ; 0090 add [bx+di+0x3f3d],ch
db 0x24, 0x9E                           ; 0094 and al,0x9e
db 0x24, 0x04                           ; 0096 and al,0x4
db 0x40                                 ; 0098 inc ax
db 0x52                                 ; 0099 push dx
db 0x43                                 ; 009A inc bx
db 0x6C                                 ; 009B insb
db 0x41                                 ; 009C inc cx
db 0x6D                                 ; 009D insw
db 0x41                                 ; 009E inc cx
db 0xCE                                 ; 009F into
db 0x41                                 ; 00A0 inc cx
db 0x2F                                 ; 00A1 das
db 0x53                                 ; 00A2 push bx
db 0xE7, 0x52                           ; 00A3 out 0x52,ax
db 0x26, 0x5D                           ; 00A5 es pop bp
db 0x13, 0x46, 0x33                     ; 00A7 adc ax,[bp+0x33]
db 0x46                                 ; 00AA inc si
db 0x29, 0x58, 0x0D                     ; 00AB sub [bx+si+0xd],bx
db 0x58                                 ; 00AE pop ax
db 0xD1, 0x47, 0xCD                     ; 00AF rol word [bx-0x33],1
db 0x47                                 ; 00B2 inc di
db 0x93                                 ; 00B3 xchg ax,bx
db 0x51                                 ; 00B4 push cx
db 0x2A, 0x50, 0x17                     ; 00B5 sub dl,[bx+si+0x17]
db 0x54                                 ; 00B8 push sp
db 0x68, 0x29, 0xA5                     ; 00B9 push word 0xa529
db 0x29, 0xB1, 0x29, 0x1C               ; 00BC sub [bx+di+0x1c29],si
db 0x65, 0x80, 0x7E, 0x96, 0x7D         ; 00C0 cmp byte [gs:bp-0x6a],0x7d
db 0xF1                                 ; 00C5 int1
db 0x70, 0x87                           ; 00C6 jo 0x4f
db 0x78, 0x0E                           ; 00C8 js 0xd8
db 0x7A, 0x0C                           ; 00CA jpe 0xd8
db 0x73, 0x84                           ; 00CC jnc 0x52
db 0x62                                 ; 00CE db 0x62
db 0xF4                                 ; 00CF hlt
db 0x79, 0xAD                           ; 00D0 jns 0x7f
db 0x7A, 0xCD                           ; 00D2 jpe 0xa1
db 0x7A, 0x8C                           ; 00D4 jpe 0x62
db 0x2B, 0x3D                           ; 00D6 sub di,[di]
db 0x1E                                 ; 00D8 push ds
db 0x7A, 0x1B                           ; 00D9 jpe 0xf6
db 0xE8, 0x28, 0x17                     ; 00DB call 0x1806
db 0x26, 0xE9, 0x29, 0xF8               ; 00DE es jmp 0xf90b
db 0x28, 0x0B                           ; 00E2 sub [bp+di],cl
db 0x29, 0x80, 0x22, 0x47               ; 00E4 sub [bx+si+0x4722],ax
db 0x29, 0x0D                           ; 00E8 sub [di],cx
db 0x26, 0x12, 0x26, 0x75, 0x1B         ; 00EA adc ah,[es:0x1b75]
db 0xAD                                 ; 00EF lodsw
db 0x6B, 0x51, 0x6B, 0x82               ; 00F0 imul dx,[bx+di+0x6b],byte -0x7e
db 0x6B, 0x9C, 0x65, 0xFA, 0x55         ; 00F4 imul bx,[si+0xfa65],byte +0x55
db 0x98                                 ; 00F9 cbw
db 0x56                                 ; 00FA push si
db 0x12, 0x57, 0xC0                     ; 00FB adc dl,[bx-0x40]
db 0x44                                 ; 00FE inc sp
db 0x98                                 ; 00FF cbw
db 0x44                                 ; 0100 inc sp
db 0xAC                                 ; 0101 lodsb
db 0x44                                 ; 0102 inc sp
db 0x37                                 ; 0103 aaa
db 0x01, 0x48, 0x01                     ; 0104 add [bx+si+0x1],cx
db 0x57                                 ; 0107 push di
db 0x01, 0x8B, 0x01, 0xB4               ; 0108 add [bp+di+0xb401],cx
db 0x01, 0xD9                           ; 010C add cx,bx
db 0x01, 0xE5                           ; 010E add bp,sp
db 0x01, 0xF4                           ; 0110 add sp,si
db 0x01, 0xF9                           ; 0112 add cx,di
db 0x01, 0x15                           ; 0114 add [di],dx
db 0x02, 0x16, 0x02, 0x1A               ; 0116 add dl,[0x1a02]
db 0x02, 0x50, 0x02                     ; 011A add dl,[bx+si+0x2]
db 0x62, 0x02                           ; 011D bound ax,[bp+si]
db 0x6D                                 ; 011F insw
db 0x02, 0x86, 0x02, 0xA9               ; 0120 add al,[bp+0xa902]
db 0x02, 0xAA, 0x02, 0xDF               ; 0124 add ch,[bp+si+0xdf02]
db 0x02, 0x23                           ; 0128 add ah,[bp+di]
db 0x03, 0x3A                           ; 012A add di,[bp+si]
db 0x03, 0x43, 0x03                     ; 012C add ax,[bp+di+0x3]
db 0x4D                                 ; 012F dec bp
db 0x03, 0x65, 0x03                     ; 0130 add sp,[di+0x3]
db 0x69, 0x03, 0x6A, 0x03               ; 0133 imul ax,[bp+di],word 0x36a
db 0x55                                 ; 0137 push bp
db 0x54                                 ; 0138 push sp
db 0xCF                                 ; 0139 iret
db 0xAA                                 ; 013A stosb
db 0x4E                                 ; 013B dec si
db 0xC4                                 ; 013C db 0xC4
db 0xEE                                 ; 013D out dx,al
db 0x42                                 ; 013E inc dx
db 0xD3, 0x06, 0x54, 0xCE               ; 013F rol word [0xce54],cl
db 0x0E                                 ; 0143 push cs
db 0x53                                 ; 0144 push bx
db 0xC3                                 ; 0145 ret
db 0x15, 0x00, 0x53                     ; 0146 adc ax,0x5300
db 0x41                                 ; 0149 inc cx
db 0x56                                 ; 014A push si
db 0xC5                                 ; 014B db 0xC5
db 0xC2, 0x4C, 0x4F                     ; 014C ret 0x4f4c
db 0x41                                 ; 014F inc cx
db 0xC4                                 ; 0150 db 0xC4
db 0xC3                                 ; 0151 ret
db 0x45                                 ; 0152 inc bp
db 0x45                                 ; 0153 inc bp
db 0xD0, 0xC5                           ; 0154 rol ch,1
db 0x00, 0x4F, 0x4C                     ; 0156 add [bx+0x4c],cl
db 0x4F                                 ; 0159 dec di
db 0xD2, 0xBF, 0x4C, 0x4F               ; 015A sar byte [bx+0x4f4c],cl
db 0x53                                 ; 015E push bx
db 0xC5, 0xBB, 0x4F, 0x4E               ; 015F lds di,[bp+di+0x4e4f]
db 0xD4, 0x99                           ; 0163 aam 0x99
db 0x4C                                 ; 0165 dec sp
db 0x45                                 ; 0166 inc bp
db 0x41                                 ; 0167 inc cx
db 0xD2, 0x92, 0x53, 0x52               ; 0168 rcl byte [bp+si+0x5253],cl
db 0x4C                                 ; 016C dec sp
db 0x49                                 ; 016D dec cx
db 0xCE                                 ; 016E into
db 0xDB                                 ; 016F db 0xDB
db 0x49                                 ; 0170 dec cx
db 0x4E                                 ; 0171 dec si
db 0xD4, 0x1C                           ; 0172 aam 0x1c
db 0x53                                 ; 0174 push bx
db 0x4E                                 ; 0175 dec si
db 0xC7                                 ; 0176 db 0xC7
db 0x1D, 0x44, 0x42                     ; 0177 sbb ax,0x4244
db 0xCC                                 ; 017A int3
db 0x1E                                 ; 017B push ds
db 0x4F                                 ; 017C dec di
db 0xD3, 0x0C                           ; 017D ror word [si],cl
db 0x48                                 ; 017F dec ax
db 0x52                                 ; 0180 push dx
db 0xA4                                 ; 0181 movsb
db 0x16                                 ; 0182 push ss
db 0x41                                 ; 0183 inc cx
db 0x4C                                 ; 0184 dec sp
db 0xCC                                 ; 0185 int3
db 0xB3, 0x4C                           ; 0186 mov bl,0x4c
db 0xD3, 0xC0                           ; 0188 rol ax,cl
db 0x00, 0x45, 0x4C                     ; 018A add [di+0x4c],al
db 0x45                                 ; 018D inc bp
db 0x54                                 ; 018E push sp
db 0xC5, 0xA9, 0x41, 0x54               ; 018F lds bp,[bx+di+0x5441]
db 0xC1, 0x84, 0x49, 0xCD, 0x86         ; 0193 rol word [si+0xcd49],0x86
db 0x45                                 ; 0198 inc bp
db 0x46                                 ; 0199 inc si
db 0x53                                 ; 019A push bx
db 0x54                                 ; 019B push sp
db 0xD2, 0xAC, 0x45, 0x46               ; 019C shr byte [si+0x4645],cl
db 0x49                                 ; 01A0 dec cx
db 0x4E                                 ; 01A1 dec si
db 0xD4, 0xAD                           ; 01A2 aam 0xad
db 0x45                                 ; 01A4 inc bp
db 0x46                                 ; 01A5 inc si
db 0x53                                 ; 01A6 push bx
db 0x4E                                 ; 01A7 dec si
db 0xC7                                 ; 01A8 db 0xC7
db 0xAE                                 ; 01A9 scasb
db 0x45                                 ; 01AA inc bp
db 0x46                                 ; 01AB inc si
db 0x44                                 ; 01AC inc sp
db 0x42                                 ; 01AD inc dx
db 0xCC                                 ; 01AE int3
db 0xAF                                 ; 01AF scasw
db 0x45                                 ; 01B0 inc bp
db 0xC6                                 ; 01B1 db 0xC6
db 0x97                                 ; 01B2 xchg ax,di
db 0x00, 0x4C, 0x53                     ; 01B3 add [si+0x53],cl
db 0xC5, 0xA1, 0x4E, 0xC4               ; 01B6 lds sp,[bx+di+0xc44e]
db 0x81, 0x52, 0x41, 0x53, 0xC5         ; 01BA adc word [bp+si+0x41],0xc553
db 0xA5                                 ; 01BF movsw
db 0x44                                 ; 01C0 inc sp
db 0x49                                 ; 01C1 dec cx
db 0xD4, 0xA6                           ; 01C2 aam 0xa6
db 0x52                                 ; 01C4 push dx
db 0x52                                 ; 01C5 push dx
db 0x4F                                 ; 01C6 dec di
db 0xD2, 0xA7, 0x52, 0xCC               ; 01C7 shl byte [bx+0xcc52],cl
db 0xD4, 0x52                           ; 01CB aam 0x52
db 0xD2, 0xD5                           ; 01CD rcl ch,cl
db 0x58                                 ; 01CF pop ax
db 0xD0, 0x0B                           ; 01D0 ror byte [bp+di],1
db 0x4F                                 ; 01D2 dec di
db 0xC6                                 ; 01D3 db 0xC6
db 0x23, 0x51, 0xD6                     ; 01D4 and dx,[bx+di-0x2a]
db 0xF1                                 ; 01D7 int1
db 0x00, 0x4F, 0xD2                     ; 01D8 add [bx-0x2e],cl
db 0x82                                 ; 01DB db 0x82
db 0xCE                                 ; 01DC into
db 0xD1, 0x52, 0xC5                     ; 01DD rcl word [bp+si-0x3b],1
db 0x0F, 0x49, 0xD8                     ; 01E0 cmovns bx,ax
db 0x1F                                 ; 01E3 pop ds
db 0x00, 0x4F, 0x54                     ; 01E4 add [bx+0x54],cl
db 0xCF                                 ; 01E7 iret
db 0x89, 0x4F, 0x20                     ; 01E8 mov [bx+0x20],cx
db 0x54                                 ; 01EB push sp
db 0xCF                                 ; 01EC iret
db 0x89, 0x4F, 0x53                     ; 01ED mov [bx+0x53],cx
db 0x55                                 ; 01F0 push bp
db 0xC2, 0x8D, 0x00                     ; 01F1 ret 0x8d
db 0x45                                 ; 01F4 inc bp
db 0x58                                 ; 01F5 pop ax
db 0xA4                                 ; 01F6 movsb
db 0x1A, 0x00                           ; 01F7 sbb al,[bx+si]
db 0x4E                                 ; 01F9 dec si
db 0x50                                 ; 01FA push ax
db 0x55                                 ; 01FB push bp
db 0xD4, 0x85                           ; 01FC aam 0x85
db 0xC6                                 ; 01FE db 0xC6
db 0x8B, 0x4E, 0x53                     ; 01FF mov cx,[bp+0x53]
db 0x54                                 ; 0202 push sp
db 0xD2, 0xD8                           ; 0203 rcr al,cl
db 0x4E                                 ; 0205 dec si
db 0xD4, 0x05                           ; 0206 aam 0x5
db 0x4E                                 ; 0208 dec si
db 0xD0, 0x10                           ; 0209 rcl byte [bx+si],1
db 0x4D                                 ; 020B dec bp
db 0xD0                                 ; 020C db 0xD0
db 0xF2, 0x4E                           ; 020D repne dec si
db 0x4B                                 ; 020F dec bx
db 0x45                                 ; 0210 inc bp
db 0x59                                 ; 0211 pop cx
db 0xA4                                 ; 0212 movsb
db 0xDE, 0x00                           ; 0213 fiadd word [bx+si]
db 0x00, 0x45, 0xD9                     ; 0215 add [di-0x27],al
db 0xC9                                 ; 0218 leave
db 0x00, 0x4F, 0x43                     ; 0219 add [bx+0x43],cl
db 0x41                                 ; 021C inc cx
db 0x54                                 ; 021D push sp
db 0xC5                                 ; 021E db 0xC5
db 0xCA, 0x50, 0x52                     ; 021F retf 0x5250
db 0x49                                 ; 0222 dec cx
db 0x4E                                 ; 0223 dec si
db 0xD4, 0x9D                           ; 0224 aam 0x9d
db 0x4C                                 ; 0226 dec sp
db 0x49                                 ; 0227 dec cx
db 0x53                                 ; 0228 push bx
db 0xD4, 0x9E                           ; 0229 aam 0x9e
db 0x50                                 ; 022B push ax
db 0x4F                                 ; 022C dec di
db 0xD3, 0x1B                           ; 022D rcr word [bp+di],cl
db 0x45                                 ; 022F inc bp
db 0xD4, 0x88                           ; 0230 aam 0x88
db 0x49                                 ; 0232 dec cx
db 0x4E                                 ; 0233 dec si
db 0xC5, 0xB0, 0x4F, 0x41               ; 0234 lds si,[bx+si+0x414f]
db 0xC4, 0xBC, 0x49, 0x53               ; 0238 les di,[si+0x5349]
db 0xD4, 0x93                           ; 023C aam 0x93
db 0x4F                                 ; 023E dec di
db 0xC7                                 ; 023F db 0xC7
db 0x0A, 0x4F, 0xC3                     ; 0240 or cl,[bx-0x3d]
db 0x24, 0x45                           ; 0243 and al,0x45
db 0xCE                                 ; 0245 into
db 0x12, 0x45, 0x46                     ; 0246 adc al,[di+0x46]
db 0x54                                 ; 0249 push sp
db 0xA4                                 ; 024A movsb
db 0x01, 0x4F, 0xC6                     ; 024B add [bx-0x3a],cx
db 0x25, 0x00, 0x4F                     ; 024E and ax,0x4f00
db 0x54                                 ; 0251 push sp
db 0x4F                                 ; 0252 dec di
db 0xD2, 0xC1                           ; 0253 rol cl,cl
db 0x45                                 ; 0255 inc bp
db 0x52                                 ; 0256 push dx
db 0x47                                 ; 0257 inc di
db 0xC5, 0xBD, 0x4F, 0xC4               ; 0258 lds di,[di+0xc44f]
db 0xF3, 0x49                           ; 025C rep dec cx
db 0x44                                 ; 025E inc sp
db 0xA4                                 ; 025F movsb
db 0x03, 0x00                           ; 0260 add ax,[bx+si]
db 0x45                                 ; 0262 inc bp
db 0x58                                 ; 0263 pop ax
db 0xD4, 0x83                           ; 0264 aam 0x83
db 0x45                                 ; 0266 inc bp
db 0xD7                                 ; 0267 xlatb
db 0x94                                 ; 0268 xchg ax,sp
db 0x4F                                 ; 0269 dec di
db 0xD4, 0xD3                           ; 026A aam 0xd3
db 0x00, 0x50, 0x45                     ; 026C add [bx+si+0x45],dl
db 0xCE                                 ; 026F into
db 0xBA, 0x55, 0xD4                     ; 0270 mov dx,0xd455
db 0x9C                                 ; 0273 pushf
db 0xCE                                 ; 0274 into
db 0x95                                 ; 0275 xchg ax,bp
db 0xD2, 0xEF                           ; 0276 shr bh,cl
db 0x43                                 ; 0278 inc bx
db 0x54                                 ; 0279 push sp
db 0xA4                                 ; 027A movsb
db 0x19, 0x50, 0x54                     ; 027B sbb [bx+si+0x54],dx
db 0x49                                 ; 027E dec cx
db 0x4F                                 ; 027F dec di
db 0xCE                                 ; 0280 into
db 0xB8, 0x46, 0xC6                     ; 0281 mov ax,0xc646
db 0xDD, 0x00                           ; 0284 fld qword [bx+si]
db 0x52                                 ; 0286 push dx
db 0x49                                 ; 0287 dec cx
db 0x4E                                 ; 0288 dec si
db 0xD4, 0x91                           ; 0289 aam 0x91
db 0x4F                                 ; 028B dec di
db 0x4B                                 ; 028C dec bx
db 0xC5, 0x98, 0x4F, 0xD3               ; 028D lds bx,[bx+si+0xd34f]
db 0x11, 0x45, 0x45                     ; 0291 adc [di+0x45],ax
db 0xCB                                 ; 0294 retf
db 0x17                                 ; 0295 pop ss
db 0x53                                 ; 0296 push bx
db 0x45                                 ; 0297 inc bp
db 0xD4, 0xC6                           ; 0298 aam 0xc6
db 0x52                                 ; 029A push dx
db 0x45                                 ; 029B inc bp
db 0x53                                 ; 029C push bx
db 0x45                                 ; 029D inc bp
db 0xD4, 0xC7                           ; 029E aam 0xc7
db 0x4F                                 ; 02A0 dec di
db 0x49                                 ; 02A1 dec cx
db 0x4E                                 ; 02A2 dec si
db 0xD4, 0xDC                           ; 02A3 aam 0xdc
db 0x45                                 ; 02A5 inc bp
db 0xCE                                 ; 02A6 into
db 0x20, 0x00                           ; 02A7 and [bx+si],al
db 0x00, 0x55, 0xCE                     ; 02A9 add [di-0x32],dl
db 0x8A, 0x45, 0x54                     ; 02AC mov al,[di+0x54]
db 0x55                                 ; 02AF push bp
db 0x52                                 ; 02B0 push dx
db 0xCE                                 ; 02B1 into
db 0x8E, 0x45, 0x41                     ; 02B2 mov es,[di+0x41]
db 0xC4, 0x87, 0x45, 0x53               ; 02B5 les ax,[bx+0x5345]
db 0x54                                 ; 02B9 push sp
db 0x4F                                 ; 02BA dec di
db 0x52                                 ; 02BB push dx
db 0xC5, 0x8C, 0x45, 0xCD               ; 02BC lds cx,[si+0xcd45]
db 0x8F, 0x45, 0x53                     ; 02C0 pop word [di+0x53]
db 0x55                                 ; 02C3 push bp
db 0x4D                                 ; 02C4 dec bp
db 0xC5, 0xA8, 0x49, 0x47               ; 02C5 lds bp,[bx+si+0x4749]
db 0x48                                 ; 02C9 dec ax
db 0x54                                 ; 02CA push sp
db 0xA4                                 ; 02CB movsb
db 0x02, 0x4E, 0xC4                     ; 02CC add cl,[bp-0x3c]
db 0x08, 0x45, 0x4E                     ; 02CF or [di+0x4e],al
db 0x55                                 ; 02D2 push bp
db 0xCD, 0xAB                           ; 02D3 int 0xab
db 0x41                                 ; 02D5 inc cx
db 0x4E                                 ; 02D6 dec si
db 0x44                                 ; 02D7 inc sp
db 0x4F                                 ; 02D8 dec di
db 0x4D                                 ; 02D9 dec bp
db 0x49                                 ; 02DA dec cx
db 0x5A                                 ; 02DB pop dx
db 0xC5, 0xB9, 0x00, 0x43               ; 02DC lds di,[bx+di+0x4300]
db 0x52                                 ; 02E0 push dx
db 0x45                                 ; 02E1 inc bp
db 0x45                                 ; 02E2 inc bp
db 0xCE                                 ; 02E3 into
db 0xC8, 0x54, 0x4F, 0xD0               ; 02E4 enter 0x4f54,0xd0
db 0x90                                 ; 02E8 nop
db 0x57                                 ; 02E9 push di
db 0x41                                 ; 02EA inc cx
db 0xD0, 0xA4, 0x41, 0x56               ; 02EB shl byte [si+0x5641],1
db 0xC5, 0xBE, 0x50, 0x43               ; 02EF lds di,[bp+0x4350]
db 0xA8, 0xD2                           ; 02F3 test al,0xd2
db 0x54                                 ; 02F5 push sp
db 0x45                                 ; 02F6 inc bp
db 0xD0, 0xCF                           ; 02F7 ror bh,1
db 0x47                                 ; 02F9 inc di
db 0xCE                                 ; 02FA into
db 0x04, 0x51                           ; 02FB add al,0x51
db 0xD2, 0x07                           ; 02FD rol byte [bx],cl
db 0x49                                 ; 02FF dec cx
db 0xCE                                 ; 0300 into
db 0x09, 0x54, 0x52                     ; 0301 or [si+0x52],dx
db 0xA4                                 ; 0304 movsb
db 0x13, 0x54, 0x52                     ; 0305 adc dx,[si+0x52]
db 0x49                                 ; 0308 dec cx
db 0x4E                                 ; 0309 dec si
db 0x47                                 ; 030A inc di
db 0xA4                                 ; 030B movsb
db 0xD6                                 ; 030C salc
db 0x50                                 ; 030D push ax
db 0x41                                 ; 030E inc cx
db 0x43                                 ; 030F inc bx
db 0x45                                 ; 0310 inc bp
db 0xA4                                 ; 0311 movsb
db 0x18, 0x4F, 0x55                     ; 0312 sbb [bx+0x55],cl
db 0x4E                                 ; 0315 dec si
db 0xC4                                 ; 0316 db 0xC4
db 0xC4, 0x54, 0x49                     ; 0317 les dx,[si+0x49]
db 0x43                                 ; 031A inc bx
db 0xCB                                 ; 031B retf
db 0x21, 0x54, 0x52                     ; 031C and [si+0x52],dx
db 0x49                                 ; 031F dec cx
db 0xC7                                 ; 0320 db 0xC7
db 0x22, 0x00                           ; 0321 and al,[bx+si]
db 0x48                                 ; 0323 dec ax
db 0x45                                 ; 0324 inc bp
db 0xCE                                 ; 0325 into
db 0xCD, 0x52                           ; 0326 int 0x52
db 0x4F                                 ; 0328 dec di
db 0xCE                                 ; 0329 into
db 0xA2, 0x52, 0x4F                     ; 032A mov [0x4f52],al
db 0x46                                 ; 032D inc si
db 0xC6                                 ; 032E db 0xC6
db 0xA3, 0x41, 0x42                     ; 032F mov [0x4241],ax
db 0xA8, 0xCE                           ; 0332 test al,0xce
db 0xCF                                 ; 0334 iret
db 0xCC                                 ; 0335 int3
db 0x41                                 ; 0336 inc cx
db 0xCE                                 ; 0337 into
db 0x0D, 0x00, 0x53                     ; 0338 or ax,0x5300
db 0x49                                 ; 033B dec cx
db 0x4E                                 ; 033C dec si
db 0xC7                                 ; 033D db 0xC7
db 0xD7                                 ; 033E xlatb
db 0x53                                 ; 033F push bx
db 0xD2, 0xD0                           ; 0340 rcl al,cl
db 0x00, 0x41, 0xCC                     ; 0342 add [bx+di-0x34],al
db 0x14, 0x41                           ; 0345 adc al,0x41
db 0x52                                 ; 0347 push dx
db 0x50                                 ; 0348 push ax
db 0x54                                 ; 0349 push sp
db 0xD2, 0xDA                           ; 034A rcr dl,cl
db 0x00, 0x49, 0x44                     ; 034C add [bx+di+0x44],cl
db 0x54                                 ; 034F push sp
db 0xC8, 0xA0, 0x41, 0x49               ; 0350 enter 0x41a0,0x49
db 0xD4, 0x96                           ; 0354 aam 0x96
db 0x48                                 ; 0356 dec ax
db 0x49                                 ; 0357 dec cx
db 0x4C                                 ; 0358 dec sp
db 0xC5, 0xB1, 0x45, 0x4E               ; 0359 lds si,[bx+di+0x4e45]
db 0xC4, 0xB2, 0x52, 0x49               ; 035D les si,[bp+si+0x4952]
db 0x54                                 ; 0361 push sp
db 0xC5, 0xB7, 0x00, 0x4F               ; 0362 lds si,[bx+0x4f00]
db 0xD2                                 ; 0366 db 0xD2
db 0xF0, 0x00, 0x00                     ; 0367 lock add [bx+si],al
db 0x00, 0xAB, 0xE9, 0xAD               ; 036A add [bp+di+0xade9],ch
db 0xEA, 0xAA, 0xEB, 0xAF, 0xEC         ; 036E jmp 0xecaf:0xebaa
db 0xDE, 0xED                           ; 0373 fsubp st5
db 0xDC, 0xF4                           ; 0375 fdivr to st4
db 0xA7                                 ; 0377 cmpsw
db 0xD9, 0xBE, 0xE6, 0xBD               ; 0378 fnstcw [bp+0xbde6]
db 0xE7, 0xBC                           ; 037C out 0xbc,ax
db 0xE8, 0x00, 0x79                     ; 037E call 0x7c81
db 0x79, 0x7C                           ; 0381 jns 0x3ff
db 0x7C, 0x7F                           ; 0383 jl 0x404
db 0x50                                 ; 0385 push ax
db 0x46                                 ; 0386 inc si
db 0x3C, 0x32                           ; 0387 cmp al,0x32
db 0x28, 0x7A, 0x7B                     ; 0389 sub [bp+si+0x7b],bh
db 0x82                                 ; 038C db 0x82
db 0x6B, 0x00, 0x00                     ; 038D imul ax,[bx+si],byte +0x0
db 0xAD                                 ; 0390 lodsw
db 0x6B, 0x3B, 0x64                     ; 0391 imul di,[bp+di],byte +0x64
db 0x51                                 ; 0394 push cx
db 0x6B, 0xA8, 0x66, 0x03, 0x63         ; 0395 imul bp,[bx+si+0x366],byte +0x63
db 0x53                                 ; 039A push bx
db 0x6C                                 ; 039B insb
db 0x20, 0x63, 0x74                     ; 039C and [bp+di+0x74],ah
db 0x65, 0x12, 0x63, 0x19               ; 039F adc ah,[gs:bp+di+0x19]
db 0x63, 0x41, 0x63                     ; 03A3 arpl [bx+di+0x63],ax
db 0x28, 0x63, 0x31                     ; 03A6 sub [bp+di+0x31],ah
db 0x64, 0x6A, 0x63                     ; 03A9 fs push byte +0x63
db 0x4F                                 ; 03AC dec di
db 0x63, 0x89, 0x63, 0xD7               ; 03AD arpl [bx+di+0xd763],cx
db 0x18, 0xB4, 0x65, 0x00               ; 03B1 sbb [si+0x65],dh
db 0x4E                                 ; 03B5 dec si
db 0x45                                 ; 03B6 inc bp
db 0x58                                 ; 03B7 pop ax
db 0x54                                 ; 03B8 push sp
db 0x20, 0x77, 0x69                     ; 03B9 and [bx+0x69],dh
db 0x74, 0x68                           ; 03BC jz 0x426
db 0x6F                                 ; 03BE outsw
db 0x75, 0x74                           ; 03BF jnz 0x435
db 0x20, 0x46, 0x4F                     ; 03C1 and [bp+0x4f],al
db 0x52                                 ; 03C4 push dx
db 0x00

err_syntax:                             ; 03C6
  db      "Syntax error", 0

err_return:                             ; 03D3
  db      "RETURN without GOSUB", 0

err_data:                               ; 03E8
  db      "Out of DATA", 0

err_function:                           ; 03F4
  db      "Illegal function call", 0

err_overflow:                           ; 040A
  db      "Overflow", 0

err_memory:                             ; 0413
  db      "Out of memory", 0

db 0x55, 0x6E                     ; 0421 add [di+0x6e],dl
db 0x64, 0x65, 0x66, 0x69, 0x6E, 0x65, 0x64
db 0x20, 0x6C, 0x69                     ; 042A
db 0x6E                                 ; 042D outsb
db 0x65, 0x20, 0x6E, 0x75               ; 042E and [gs:bp+0x75],ch
db 0x6D                                 ; 0432 insw
db 0x62, 0x65, 0x72                     ; 0433 bound sp,[di+0x72]
db 0x00, 0x53, 0x75                     ; 0436 add [bp+di+0x75],dl
db 0x62, 0x73, 0x63                     ; 0439 bound si,[bp+di+0x63]
db 0x72, 0x69                           ; 043C jc 0x4a7
db 0x70, 0x74                           ; 043E jo 0x4b4
db 0x20, 0x6F, 0x75                     ; 0440 and [bx+0x75],ch
db 0x74, 0x20                           ; 0443 jz 0x465
db 0x6F                                 ; 0445 outsw
db 0x66, 0x20, 0x72, 0x61               ; 0446 o32 and [bp+si+0x61],dh
db 0x6E                                 ; 044A outsb
db 0x67, 0x65, 0x00, 0x44, 0x75, 0x70   ; 044B add [gs:ebp+esi*2+0x70],al
db 0x6C                                 ; 0451 insb
db 0x69, 0x63, 0x61, 0x74, 0x65         ; 0452 imul sp,[bp+di+0x61],word 0x6574
db 0x20, 0x44, 0x65                     ; 0457 and [si+0x65],al
db 0x66, 0x69, 0x6E, 0x69, 0x74, 0x69, 0x6F, 0x6E       ; 045A imul ebp,[bp+0x69],dword 0x6e6f6974
db 0x00, 0x44, 0x69                     ; 0462 add [si+0x69],al
db 0x76, 0x69                           ; 0465 jna 0x4d0
db 0x73, 0x69                           ; 0467 jnc 0x4d2
db 0x6F                                 ; 0469 outsw
db 0x6E                                 ; 046A outsb
db 0x20, 0x62, 0x79                     ; 046B and [bp+si+0x79],ah
db 0x20, 0x7A, 0x65                     ; 046E and [bp+si+0x65],bh
db 0x72, 0x6F                           ; 0471 jc 0x4e2
db 0x00, 0x49, 0x6C                     ; 0473 add [bx+di+0x6c],cl
db 0x6C                                 ; 0476 insb
db 0x65, 0x67, 0x61                     ; 0477 gs a32 popa
db 0x6C                                 ; 047A insb
db 0x20, 0x64, 0x69                     ; 047B and [si+0x69],ah
db 0x72, 0x65                           ; 047E jc 0x4e5
db 0x63, 0x74, 0x00                     ; 0480 arpl [si+0x0],si
db 0x54                                 ; 0483 push sp
db 0x79, 0x70                           ; 0484 jns 0x4f6
db 0x65, 0x20, 0x6D, 0x69               ; 0486 and [gs:di+0x69],ch
db 0x73, 0x6D                           ; 048A jnc 0x4f9
db 0x61                                 ; 048C popa
db 0x74, 0x63                           ; 048D jz 0x4f2
db 0x68, 0x00, 0x4F                     ; 048F push word 0x4f00
db 0x75, 0x74                           ; 0492 jnz 0x508
db 0x20, 0x6F, 0x66                     ; 0494 and [bx+0x66],ch
db 0x20, 0x73, 0x74                     ; 0497 and [bp+di+0x74],dh
db 0x72, 0x69                           ; 049A jc 0x505
db 0x6E                                 ; 049C outsb
db 0x67, 0x20, 0x73, 0x70               ; 049D and [ebx+0x70],dh
db 0x61                                 ; 04A1 popa
db 0x63, 0x65, 0x00                     ; 04A2 arpl [di+0x0],sp
db 0x53                                 ; 04A5 push bx
db 0x74, 0x72                           ; 04A6 jz 0x51a
db 0x69, 0x6E, 0x67, 0x20, 0x74         ; 04A8 imul bp,[bp+0x67],word 0x7420
db 0x6F                                 ; 04AD outsw
db 0x6F                                 ; 04AE outsw
db 0x20, 0x6C, 0x6F                     ; 04AF and [si+0x6f],ch
db 0x6E                                 ; 04B2 outsb
db 0x67, 0x00, 0x53, 0x74               ; 04B3 add [ebx+0x74],dl
db 0x72, 0x69                           ; 04B7 jc 0x522
db 0x6E                                 ; 04B9 outsb
db 0x67, 0x20, 0x66, 0x6F               ; 04BA and [esi+0x6f],ah
db 0x72, 0x6D                           ; 04BE jc 0x52d
db 0x75, 0x6C                           ; 04C0 jnz 0x52e
db 0x61                                 ; 04C2 popa
db 0x20, 0x74, 0x6F                     ; 04C3 and [si+0x6f],dh
db 0x6F                                 ; 04C6 outsw
db 0x20, 0x63, 0x6F                     ; 04C7 and [bp+di+0x6f],ah
db 0x6D                                 ; 04CA insw
db 0x70, 0x6C                           ; 04CB jo 0x539
db 0x65, 0x78, 0x00                     ; 04CD gs js 0x4d0
db 0x43                                 ; 04D0 inc bx
db 0x61                                 ; 04D1 popa
db 0x6E                                 ; 04D2 outsb
db 0x27                                 ; 04D3 daa
db 0x74, 0x20                           ; 04D4 jz 0x4f6
db 0x63, 0x6F, 0x6E                     ; 04D6 arpl [bx+0x6e],bp
db 0x74, 0x69                           ; 04D9 jz 0x544
db 0x6E                                 ; 04DB outsb
db 0x75, 0x65                           ; 04DC jnz 0x543
db 0x00, 0x55, 0x6E                     ; 04DE add [di+0x6e],dl
db 0x64, 0x65, 0x66, 0x69, 0x6E, 0x65, 0x64       ; 04E1 imul ebp,[gs:bp+0x65],dword 0x73752064
db 0x20, 0x75, 0x73                     ; 04E8
db 0x65, 0x72, 0x20                     ; 04EB gs jc 0x50e
db 0x66, 0x75, 0x6E                     ; 04EE o32 jnz 0x55f
db 0x63, 0x74, 0x69                     ; 04F1 arpl [si+0x69],si
db 0x6F                                 ; 04F4 outsw
db 0x6E                                 ; 04F5 outsb
db 0x00, 0x4E, 0x6F                     ; 04F6 add [bp+0x6f],cl
db 0x20, 0x52, 0x45                     ; 04F9 and [bp+si+0x45],dl
db 0x53                                 ; 04FC push bx
db 0x55                                 ; 04FD push bp
db 0x4D                                 ; 04FE dec bp
db 0x45                                 ; 04FF inc bp
db 0x00, 0x52, 0x45                     ; 0500 add [bp+si+0x45],dl
db 0x53                                 ; 0503 push bx
db 0x55                                 ; 0504 push bp
db 0x4D                                 ; 0505 dec bp
db 0x45                                 ; 0506 inc bp
db 0x20, 0x77, 0x69                     ; 0507 and [bx+0x69],dh
db 0x74, 0x68                           ; 050A jz 0x574
db 0x6F                                 ; 050C outsw
db 0x75, 0x74                           ; 050D jnz 0x583
db 0x20, 0x65, 0x72                     ; 050F and [di+0x72],ah
db 0x72, 0x6F                           ; 0512 jc 0x583
db 0x72, 0x00                           ; 0514 jc 0x516
db 0x55                                 ; 0516 push bp
db 0x6E                                 ; 0517 outsb
db 0x70, 0x72                           ; 0518 jo 0x58c
db 0x69, 0x6E, 0x74, 0x61, 0x62         ; 051A imul bp,[bp+0x74],word 0x6261
db 0x6C                                 ; 051F insb
db 0x65, 0x20, 0x65, 0x72               ; 0520 and [gs:di+0x72],ah
db 0x72, 0x6F                           ; 0524 jc 0x595
db 0x72, 0x00                           ; 0526 jc 0x528
db 0x4D                                 ; 0528 dec bp
db 0x69, 0x73, 0x73, 0x69, 0x6E         ; 0529 imul si,[bp+di+0x73],word 0x6e69
db 0x67, 0x20, 0x6F, 0x70               ; 052E and [edi+0x70],ch
db 0x65, 0x72, 0x61                     ; 0532 gs jc 0x596
db 0x6E                                 ; 0535 outsb
db 0x64, 0x00, 0x4C, 0x69               ; 0536 add [fs:si+0x69],cl
db 0x6E                                 ; 053A outsb
db 0x65, 0x20, 0x62, 0x75               ; 053B and [gs:bp+si+0x75],ah
db 0x66, 0x66, 0x65, 0x72, 0x20         ; 053F gs o32 jc 0x564
db 0x6F                                 ; 0544 outsw
db 0x76, 0x65                           ; 0545 jna 0x5ac
db 0x72, 0x66                           ; 0547 jc 0x5af
db 0x6C                                 ; 0549 insb
db 0x6F                                 ; 054A outsw
db 0x77, 0x00                           ; 054B ja 0x54d
db 0x44                                 ; 054D inc sp
db 0x65, 0x76, 0x69                     ; 054E gs jna 0x5ba
db 0x63, 0x65, 0x20                     ; 0551 arpl [di+0x20],sp
db 0x54                                 ; 0554 push sp
db 0x69, 0x6D, 0x65, 0x6F, 0x75         ; 0555 imul bp,[di+0x65],word 0x756f
db 0x74, 0x00                           ; 055A jz 0x55c
db 0x44                                 ; 055C inc sp
db 0x65, 0x76, 0x69                     ; 055D gs jna 0x5c9
db 0x63, 0x65, 0x20                     ; 0560 arpl [di+0x20],sp
db 0x46                                 ; 0563 inc si
db 0x61                                 ; 0564 popa
db 0x75, 0x6C                           ; 0565 jnz 0x5d3
db 0x74, 0x00                           ; 0567 jz 0x569
db 0x46                                 ; 0569 inc si
db 0x4F                                 ; 056A dec di
db 0x52                                 ; 056B push dx
db 0x20, 0x57, 0x69                     ; 056C and [bx+0x69],dl
db 0x74, 0x68                           ; 056F jz 0x5d9
db 0x6F                                 ; 0571 outsw
db 0x75, 0x74                           ; 0572 jnz 0x5e8
db 0x20, 0x4E, 0x45                     ; 0574 and [bp+0x45],cl
db 0x58                                 ; 0577 pop ax
db 0x54                                 ; 0578 push sp
db 0x00, 0x4F, 0x75                     ; 0579 add [bx+0x75],cl
db 0x74, 0x20                           ; 057C jz 0x59e
db 0x6F                                 ; 057E outsw
db 0x66, 0x20, 0x50, 0x61               ; 057F o32 and [bx+si+0x61],dl
db 0x70, 0x65                           ; 0583 jo 0x5ea
db 0x72, 0x00                           ; 0585 jc 0x587
db 0x3F                                 ; 0587 aas
db 0x00, 0x57, 0x48                     ; 0588 add [bx+0x48],dl
db 0x49                                 ; 058B dec cx
db 0x4C                                 ; 058C dec sp
db 0x45                                 ; 058D inc bp
db 0x20, 0x77, 0x69                     ; 058E and [bx+0x69],dh
db 0x74, 0x68                           ; 0591 jz 0x5fb
db 0x6F                                 ; 0593 outsw
db 0x75, 0x74                           ; 0594 jnz 0x60a
db 0x20, 0x57, 0x45                     ; 0596 and [bx+0x45],dl
db 0x4E                                 ; 0599 dec si
db 0x44                                 ; 059A inc sp
db 0x00, 0x57, 0x45                     ; 059B add [bx+0x45],dl
db 0x4E                                 ; 059E dec si
db 0x44                                 ; 059F inc sp
db 0x20, 0x77, 0x69                     ; 05A0 and [bx+0x69],dh
db 0x74, 0x68                           ; 05A3 jz 0x60d
db 0x6F                                 ; 05A5 outsw
db 0x75, 0x74                           ; 05A6 jnz 0x61c
db 0x20, 0x57, 0x48                     ; 05A8 and [bx+0x48],dl
db 0x49                                 ; 05AB dec cx
db 0x4C                                 ; 05AC dec sp
db 0x45                                 ; 05AD inc bp
db 0x00, 0x46, 0x49                     ; 05AE add [bp+0x49],al
db 0x45                                 ; 05B1 inc bp
db 0x4C                                 ; 05B2 dec sp
db 0x44                                 ; 05B3 inc sp
db 0x20, 0x6F, 0x76                     ; 05B4 and [bx+0x76],ch
db 0x65, 0x72, 0x66                     ; 05B7 gs jc 0x620
db 0x6C                                 ; 05BA insb
db 0x6F                                 ; 05BB outsw
db 0x77, 0x00                           ; 05BC ja 0x5be
db 0x49                                 ; 05BE dec cx
db 0x6E                                 ; 05BF outsb
db 0x74, 0x65                           ; 05C0 jz 0x627
db 0x72, 0x6E                           ; 05C2 jc 0x632
db 0x61                                 ; 05C4 popa
db 0x6C                                 ; 05C5 insb
db 0x20, 0x65, 0x72                     ; 05C6 and [di+0x72],ah
db 0x72, 0x6F                           ; 05C9 jc 0x63a
db 0x72, 0x00                           ; 05CB jc 0x5cd
db 0x42                                 ; 05CD inc dx
db 0x61                                 ; 05CE popa
db 0x64, 0x20, 0x66, 0x69               ; 05CF and [fs:bp+0x69],ah
db 0x6C                                 ; 05D3 insb
db 0x65, 0x20, 0x6E, 0x75               ; 05D4 and [gs:bp+0x75],ch
db 0x6D                                 ; 05D8 insw
db 0x62, 0x65, 0x72                     ; 05D9 bound sp,[di+0x72]
db 0x00, 0x46, 0x69                     ; 05DC add [bp+0x69],al
db 0x6C                                 ; 05DF insb
db 0x65, 0x20, 0x6E, 0x6F               ; 05E0 and [gs:bp+0x6f],ch
db 0x74, 0x20                           ; 05E4 jz 0x606
db 0x66, 0x6F                           ; 05E6 outsd
db 0x75, 0x6E                           ; 05E8 jnz 0x658
db 0x64, 0x00, 0x42, 0x61               ; 05EA add [fs:bp+si+0x61],al
db 0x64, 0x20, 0x66, 0x69               ; 05EE and [fs:bp+0x69],ah
db 0x6C                                 ; 05F2 insb
db 0x65, 0x20, 0x6D, 0x6F               ; 05F3 and [gs:di+0x6f],ch
db 0x64, 0x65, 0x00, 0x46, 0x69         ; 05F7 add [gs:bp+0x69],al
db 0x6C                                 ; 05FC insb
db 0x65, 0x20, 0x61, 0x6C               ; 05FD and [gs:bx+di+0x6c],ah
db 0x72, 0x65                           ; 0601 jc 0x668
db 0x61                                 ; 0603 popa
db 0x64, 0x79, 0x20                     ; 0604 fs jns 0x627
db 0x6F                                 ; 0607 outsw
db 0x70, 0x65                           ; 0608 jo 0x66f
db 0x6E                                 ; 060A outsb
db 0x00, 0x3F                           ; 060B add [bx],bh
db 0x00, 0x44, 0x65                     ; 060D add [si+0x65],al
db 0x76, 0x69                           ; 0610 jna 0x67b
db 0x63, 0x65, 0x20                     ; 0612 arpl [di+0x20],sp
db 0x49                                 ; 0615 dec cx
db 0x2F                                 ; 0616 das
db 0x4F                                 ; 0617 dec di
db 0x20, 0x45, 0x72                     ; 0618 and [di+0x72],al
db 0x72, 0x6F                           ; 061B jc 0x68c
db 0x72, 0x00                           ; 061D jc 0x61f
db 0x46                                 ; 061F inc si
db 0x69, 0x6C, 0x65, 0x20, 0x61         ; 0620 imul bp,[si+0x65],word 0x6120
db 0x6C                                 ; 0625 insb
db 0x72, 0x65                           ; 0626 jc 0x68d
db 0x61                                 ; 0628 popa
db 0x64, 0x79, 0x20                     ; 0629 fs jns 0x64c
db 0x65, 0x78, 0x69                     ; 062C gs js 0x698
db 0x73, 0x74                           ; 062F jnc 0x6a5
db 0x73, 0x00                           ; 0631 jnc 0x633
db 0x3F                                 ; 0633 aas
db 0x00, 0x3F                           ; 0634 add [bx],bh
db 0x00, 0x44, 0x69                     ; 0636 add [si+0x69],al
db 0x73, 0x6B                           ; 0639 jnc 0x6a6
db 0x20, 0x66, 0x75                     ; 063B and [bp+0x75],ah
db 0x6C                                 ; 063E insb
db 0x6C                                 ; 063F insb
db 0x00, 0x49, 0x6E                     ; 0640 add [bx+di+0x6e],cl
db 0x70, 0x75                           ; 0643 jo 0x6ba
db 0x74, 0x20                           ; 0645 jz 0x667
db 0x70, 0x61                           ; 0647 jo 0x6aa
db 0x73, 0x74                           ; 0649 jnc 0x6bf
db 0x20, 0x65, 0x6E                     ; 064B and [di+0x6e],ah
db 0x64, 0x00, 0x42, 0x61               ; 064E add [fs:bp+si+0x61],al
db 0x64, 0x20, 0x72, 0x65               ; 0652 and [fs:bp+si+0x65],dh
db 0x63, 0x6F, 0x72                     ; 0656 arpl [bx+0x72],bp
db 0x64, 0x20, 0x6E, 0x75               ; 0659 and [fs:bp+0x75],ch
db 0x6D                                 ; 065D insw
db 0x62, 0x65, 0x72                     ; 065E bound sp,[di+0x72]
db 0x00, 0x42, 0x61                     ; 0661 add [bp+si+0x61],al
db 0x64, 0x20, 0x66, 0x69               ; 0664 and [fs:bp+0x69],ah
db 0x6C                                 ; 0668 insb
db 0x65, 0x20, 0x6E, 0x61               ; 0669 and [gs:bp+0x61],ch
db 0x6D                                 ; 066D insw
db 0x65, 0x00, 0x3F                     ; 066E add [gs:bx],bh
db 0x00, 0x44, 0x69                     ; 0671 add [si+0x69],al
db 0x72, 0x65                           ; 0674 jc 0x6db
db 0x63, 0x74, 0x20                     ; 0676 arpl [si+0x20],si
db 0x73, 0x74                           ; 0679 jnc 0x6ef
db 0x61                                 ; 067B popa
db 0x74, 0x65                           ; 067C jz 0x6e3
db 0x6D                                 ; 067E insw
db 0x65, 0x6E                           ; 067F gs outsb
db 0x74, 0x20                           ; 0681 jz 0x6a3
db 0x69, 0x6E, 0x20, 0x66, 0x69         ; 0683 imul bp,[bp+0x20],word 0x6966
db 0x6C                                 ; 0688 insb
db 0x65, 0x00, 0x54, 0x6F               ; 0689 add [gs:si+0x6f],dl
db 0x6F                                 ; 068D outsw
db 0x20, 0x6D, 0x61                     ; 068E and [di+0x61],ch
db 0x6E                                 ; 0691 outsb
db 0x79, 0x20                           ; 0692 jns 0x6b4
db 0x66, 0x69, 0x6C, 0x65, 0x73, 0x00

; 145 bytes get copied from here to 0060:0000
L_069A:
  db 0x00, 0x00
  db 0x00, 0xC3
db 0x1E                                 ; 069E push ds
db 0x10, 0x00                           ; 069F adc [bx+si],al
db 0x52                                 ; 06A1 push dx
db 0xC7                                 ; 06A2 db 0xC7
db 0x4F                                 ; 06A3 dec di
db 0x80, 0x52, 0xC7, 0x4F               ; 06A4 adc byte [bp+si-0x39],0x4f
db 0x80, 0xE4, 0x00                     ; 06A8 and ah,0x0
db 0xCB                                 ; 06AB retf
db 0xFF                                 ; 06AC db 0xFF
db 0xFF                                 ; 06AD db 0xFF
db 0xFF                                 ; 06AE db 0xFF
db 0xFF                                 ; 06AF db 0xFF
db 0xFF                                 ; 06B0 db 0xFF
db 0xFF                                 ; 06B1 db 0xFF
db 0xFF                                 ; 06B2 db 0xFF
db 0xFF                                 ; 06B3 db 0xFF
db 0xFF                                 ; 06B4 db 0xFF
db 0xFF                                 ; 06B5 db 0xFF
db 0xFF                                 ; 06B6 db 0xFF
db 0xFF                                 ; 06B7 db 0xFF
db 0xFF                                 ; 06B8 db 0xFF
db 0xFF                                 ; 06B9 db 0xFF
db 0xFF                                 ; 06BA db 0xFF
db 0xFF                                 ; 06BB db 0xFF
db 0xFF                                 ; 06BC db 0xFF
db 0xFF                                 ; 06BD db 0xFF
db 0xFF                                 ; 06BE db 0xFF
db 0xFF, 0x01                           ; 06BF inc word [bx+di]
db 0x00, 0x00                           ; 06C1 add [bx+si],al
db 0x50                                 ; 06C3 push ax
db 0x38, 0x00                           ; 06C4 cmp [bx+si],al
db 0x72, 0x07                           ; 06C6 jc 0x6cf
db 0xFE                                 ; 06C8 db 0xFE
db 0xFF, 0x0F                           ; 06C9 dec word [bx]
db 0x07                                 ; 06CB pop es
db 0x0A, 0x04                           ; 06CC or al,[si]
db 0x00, 0x00                           ; 06CE add [bx+si],al
db 0x00, 0x00                           ; 06D0 add [bx+si],al
db 0x00, 0x00                           ; 06D2 add [bx+si],al
db 0x00, 0x00                           ; 06D4 add [bx+si],al
db 0x00, 0x00                           ; 06D6 add [bx+si],al
db 0x00, 0x00                           ; 06D8 add [bx+si],al
db 0x00, 0x00                           ; 06DA add [bx+si],al
db 0x00, 0x00                           ; 06DC add [bx+si],al
db 0x00, 0x00                           ; 06DE add [bx+si],al
db 0x00, 0x00                           ; 06E0 add [bx+si],al
db 0x00, 0x00                           ; 06E2 add [bx+si],al
db 0x00, 0x07                           ; 06E4 add [bx],al
db 0x00, 0x00                           ; 06E6 add [bx+si],al
db 0x07                                 ; 06E8 pop es
db 0x07                                 ; 06E9 pop es
db 0x20, 0x00                           ; 06EA and [bx+si],al
db 0x00, 0x00                           ; 06EC add [bx+si],al
db 0x00, 0x00                           ; 06EE add [bx+si],al
db 0x00, 0x00                           ; 06F0 add [bx+si],al
db 0x00, 0x00                           ; 06F2 add [bx+si],al
db 0x00, 0x01                           ; 06F4 add [bx+di],al
db 0x18, 0x18                           ; 06F6 sbb [bx+si],bl
db 0x00, 0x00                           ; 06F8 add [bx+si],al
db 0x00, 0x00                           ; 06FA add [bx+si],al
db 0x50                                 ; 06FC push ax
db 0x00, 0x01                           ; 06FD add [bx+di],al
db 0x00, 0x00                           ; 06FF add [bx+si],al
db 0x00, 0x07                           ; 0701 add [bx],al
db 0x07                                 ; 0703 pop es
db 0x00, 0x00                           ; 0704 add [bx+si],al
db 0x00, 0x00                           ; 0706 add [bx+si],al
db 0x00, 0x00                           ; 0708 add [bx+si],al
db 0x00, 0x01                           ; 070A add [bx+di],al
db 0x00, 0x00                           ; 070C add [bx+si],al
db 0x01, 0x01                           ; 070E add [bx+di],ax
db 0x01, 0x01                           ; 0710 add [bx+di],ax
db 0x01, 0x01                           ; 0712 add [bx+di],ax
db 0x01, 0x01                           ; 0714 add [bx+di],ax
db 0x01, 0x01                           ; 0716 add [bx+di],ax
db 0x01, 0x01                           ; 0718 add [bx+di],ax
db 0x01, 0x01                           ; 071A add [bx+di],ax
db 0x01, 0x01                           ; 071C add [bx+di],ax
db 0x01, 0x01                           ; 071E add [bx+di],ax
db 0x01, 0x01                           ; 0720 add [bx+di],ax
db 0x01, 0x01                           ; 0722 add [bx+di],ax
db 0x01, 0x01                           ; 0724 add [bx+di],ax
db 0x01, 0x01                           ; 0726 add [bx+di],ax
db 0x20, 0x69, 0x6E                     ; 0728 and [bx+di+0x6e],ch

L_072B:
db 0x20, 0x00                           ; 072B and [bx+si],al
db 0x4F                                 ; 072D dec di
db 0x6B, 0xFF, 0x0D                     ; 072E imul di,di,byte +0xd
db 0x00, 0x42, 0x72                     ; 0731 add [bp+si+0x72],al
db 0x65, 0x61                           ; 0734 gs popa
db 0x6B, 0x00, 0xBB                     ; 0736 imul ax,[bx+si],byte -0x45
db 0x04, 0x00                           ; 0739 add al,0x0
db 0x03, 0xDC                           ; 073B add bx,sp
db 0x43                                 ; 073D inc bx
db 0x8A, 0x07                           ; 073E mov al,[bx]
db 0x43                                 ; 0740 inc bx
db 0x3C, 0xB1                           ; 0741 cmp al,0xb1
db 0x75, 0x07                           ; 0743 jnz 0x74c
db 0xB9, 0x06, 0x00                     ; 0745 mov cx,0x6
db 0x03, 0xD9                           ; 0748 add bx,cx
db 0xEB, 0xF1                           ; 074A jmp short 0x73d
db 0x3C, 0x82                           ; 074C cmp al,0x82
db 0x74, 0x01                           ; 074E jz 0x751
db 0xC3                                 ; 0750 ret
db 0x8A, 0x0F                           ; 0751 mov cl,[bx]
db 0x43                                 ; 0753 inc bx
db 0x8A, 0x2F                           ; 0754 mov ch,[bx]
db 0x43                                 ; 0756 inc bx
db 0x53                                 ; 0757 push bx
db 0x8B, 0xD9                           ; 0758 mov bx,cx
db 0x0B, 0xD2                           ; 075A or dx,dx
db 0x87, 0xDA                           ; 075C xchg bx,dx
db 0x74, 0x04                           ; 075E jz 0x764
db 0x87, 0xDA                           ; 0760 xchg bx,dx
db 0x3B, 0xDA                           ; 0762 cmp bx,dx
db 0xB9, 0x10, 0x00                     ; 0764 mov cx,0x10
db 0x5B                                 ; 0767 pop bx
db 0x74, 0xE6                           ; 0768 jz 0x750
db 0x03, 0xD9                           ; 076A add bx,cx
db 0xEB, 0xCF                           ; 076C jmp short 0x73d
db 0xB9, 0xB5, 0x08                     ; 076E mov cx,0x8b5
db 0xE9, 0x91, 0x00                     ; 0771 jmp 0x805
db 0xCD, 0x86                           ; 0774 int 0x86
db 0x8B, 0x1E, 0x2E, 0x00               ; 0776 mov bx,[0x2e]
db 0x8A, 0xC7                           ; 077A mov al,bh
db 0x22, 0xC3                           ; 077C and al,bl
db 0xFE, 0xC0                           ; 077E inc al
db 0x74, 0x09                           ; 0780 jz 0x78b
db 0xA0, 0x4F, 0x03                     ; 0782 mov al,[0x34f]
db 0x0A, 0xC0                           ; 0785 or al,al
db 0xB2, 0x13                           ; 0787 mov dl,0x13
db 0x75, 0x4D                           ; 0789 jnz 0x7d8
db 0xE9, 0xBD, 0x26                     ; 078B jmp 0x2e4b
db 0xB2, 0x3D                           ; 078E mov dl,0x3d
db 0xB9, 0xB2, 0x39                     ; 0790 mov cx,0x39b2
db 0xB9, 0xB2, 0x36                     ; 0793 mov cx,0x36b2
db 0xB9, 0xB2, 0x35                     ; 0796 mov cx,0x35b2
db 0xB9, 0xB2, 0x34                     ; 0799 mov cx,0x34b2
db 0xB9, 0xB2, 0x33                     ; 079C mov cx,0x33b2
db 0xB9, 0xB2, 0x3E                     ; 079F mov cx,0x3eb2
db 0xB9, 0xB2, 0x37                     ; 07A2 mov cx,0x37b2
db 0xB9, 0xB2, 0x40                     ; 07A5 mov cx,0x40b2
db 0xB9, 0xB2, 0x3F                     ; 07A8 mov cx,0x3fb2
db 0xB9, 0xB2, 0x32                     ; 07AB mov cx,0x32b2
db 0xB9, 0xB2, 0x43                     ; 07AE mov cx,0x43b2
db 0xB9, 0xB2, 0x3A                     ; 07B1 mov cx,0x3ab2
db 0xEB, 0x22                           ; 07B4 jmp short 0x7d8
db 0x8B, 0x1E, 0x37, 0x03               ; 07B6 mov bx,[0x337]
db 0x89, 0x1E, 0x2E, 0x00               ; 07BA mov [0x2e],bx
db 0xB2, 0x02                           ; 07BE mov dl,0x2
db 0xB9, 0xB2, 0x0B                     ; 07C0 mov cx,0xbb2
db 0xB9, 0xB2, 0x01                     ; 07C3 mov cx,0x1b2
db 0xB9, 0xB2, 0x0A                     ; 07C6 mov cx,0xab2
db 0xB9, 0xB2, 0x12                     ; 07C9 mov cx,0x12b2
db 0xB9, 0xB2, 0x14                     ; 07CC mov cx,0x14b2
db 0xB9, 0xB2, 0x06                     ; 07CF mov cx,0x6b2
db 0xB9, 0xB2, 0x16                     ; 07D2 mov cx,0x16b2
db 0xB9, 0xB2, 0x0D                     ; 07D5 mov cx,0xdb2
db 0x32, 0xC0                           ; 07D8 xor al,al
db 0xA2, 0x36, 0x05                     ; 07DA mov [0x536],al
db 0xA2, 0x5F, 0x00                     ; 07DD mov [0x5f],al
db 0xA2, 0x62, 0x04                     ; 07E0 mov [0x462],al
db 0xA2, 0x60, 0x00                     ; 07E3 mov [0x60],al
db 0x8B, 0x1E, 0x2E, 0x00               ; 07E6 mov bx,[0x2e]
db 0x89, 0x1E, 0x47, 0x03               ; 07EA mov [0x347],bx
db 0x32, 0xC0                           ; 07EE xor al,al
db 0xA2, 0x65, 0x04                     ; 07F0 mov [0x465],al
db 0xA2, 0x6B, 0x04                     ; 07F3 mov [0x46b],al
db 0x8A, 0xC7                           ; 07F6 mov al,bh
db 0x22, 0xC3                           ; 07F8 and al,bl
db 0xFE, 0xC0                           ; 07FA inc al
db 0x74, 0x04                           ; 07FC jz 0x802
db 0x89, 0x1E, 0x49, 0x03               ; 07FE mov [0x349],bx
db 0xB9, 0x0C, 0x08                     ; 0802 mov cx,0x80c
db 0x8B, 0x1E, 0x45, 0x03               ; 0805 mov bx,[0x345]
db 0xE9, 0xB1, 0x25                     ; 0809 jmp 0x2dbd
db 0x59                                 ; 080C pop cx
db 0x8A, 0xC2                           ; 080D mov al,dl
db 0x8A, 0xCA                           ; 080F mov cl,dl
db 0xA2, 0x28, 0x00                     ; 0811 mov [0x28],al
db 0x8B, 0x1E, 0x43, 0x03               ; 0814 mov bx,[0x343]
db 0x89, 0x1E, 0x4B, 0x03               ; 0818 mov [0x34b],bx
db 0x87, 0xDA                           ; 081C xchg bx,dx
db 0x8B, 0x1E, 0x47, 0x03               ; 081E mov bx,[0x347]
db 0x8A, 0xC7                           ; 0822 mov al,bh
db 0x22, 0xC3                           ; 0824 and al,bl
db 0xFE, 0xC0                           ; 0826 inc al
db 0x74, 0x0A                           ; 0828 jz 0x834
db 0x89, 0x1E, 0x54, 0x03               ; 082A mov [0x354],bx
db 0x87, 0xDA                           ; 082E xchg bx,dx
db 0x89, 0x1E, 0x56, 0x03               ; 0830 mov [0x356],bx
db 0x8B, 0x1E, 0x4D, 0x03               ; 0834 mov bx,[0x34d]
db 0x0B, 0xDB                           ; 0838 or bx,bx
db 0x87, 0xDA                           ; 083A xchg bx,dx
db 0xBB, 0x4F, 0x03                     ; 083C mov bx,0x34f
db 0x74, 0x0B                           ; 083F jz 0x84c
db 0x22, 0x07                           ; 0841 and al,[bx]
db 0x75, 0x07                           ; 0843 jnz 0x84c
db 0xFE, 0x0F                           ; 0845 dec byte [bx]
db 0x87, 0xDA                           ; 0847 xchg bx,dx
db 0xE9, 0x73, 0x06                     ; 0849 jmp 0xebf
db 0x32, 0xC0                           ; 084C xor al,al
db 0x88, 0x07                           ; 084E mov [bx],al
db 0x8A, 0xD1                           ; 0850 mov dl,cl
db 0xE8, 0x08, 0x24                     ; 0852 call 0x2c5d
db 0xBB, 0xB4, 0x03                     ; 0855 mov bx,0x3b4
db 0xCD, 0x87                           ; 0858 int 0x87
db 0x8A, 0xC2                           ; 085A mov al,dl
db 0x3C, 0x44                           ; 085C cmp al,0x44
db 0x73, 0x08                           ; 085E jnc 0x868
db 0x3C, 0x32                           ; 0860 cmp al,0x32
db 0x73, 0x06                           ; 0862 jnc 0x86a
db 0x3C, 0x1F                           ; 0864 cmp al,0x1f
db 0x72, 0x06                           ; 0866 jc 0x86e
db 0xB0, 0x28                           ; 0868 mov al,0x28
db 0x2C, 0x13                           ; 086A sub al,0x13
db 0x8A, 0xD0                           ; 086C mov dl,al
db 0x2E, 0x8A, 0x07                     ; 086E mov al,[cs:bx]
db 0x43                                 ; 0871 inc bx
db 0x0A, 0xC0                           ; 0872 or al,al
db 0x75, 0xF8                           ; 0874 jnz 0x86e
db 0x4B                                 ; 0876 dec bx
db 0x43                                 ; 0877 inc bx
db 0xFE, 0xCA                           ; 0878 dec dl
db 0x75, 0xF2                           ; 087A jnz 0x86e
db 0x53                                 ; 087C push bx
db 0x8B, 0x1E, 0x47, 0x03               ; 087D mov bx,[0x347]
db 0x5E                                 ; 0881 pop si
db 0x87, 0xDE                           ; 0882 xchg bx,si
db 0x56                                 ; 0884 push si
db 0xCD, 0x88                           ; 0885 int 0x88
db 0x2E, 0x8A, 0x07                     ; 0887 mov al,[cs:bx]
db 0x3C, 0x3F                           ; 088A cmp al,0x3f
db 0x75, 0x06                           ; 088C jnz 0x894
db 0x5B                                 ; 088E pop bx
db 0xBB, 0xB4, 0x03                     ; 088F mov bx,0x3b4
db 0xEB, 0xD4                           ; 0892 jmp short 0x868
db 0xE8, 0xBE, 0x72                     ; 0894 call 0x7b55
db 0x5B                                 ; 0897 pop bx
db 0xBA, 0xFE, 0xFF                     ; 0898 mov dx,0xfffe
db 0x3B, 0xDA                           ; 089B cmp bx,dx
db 0xCD, 0x89                           ; 089D int 0x89
db 0x75, 0x03                           ; 089F jnz 0x8a4
db 0xE9, 0xEE, 0x75                     ; 08A1 jmp 0x7e92
db 0x8A, 0xC7                           ; 08A4 mov al,bh
db 0x22, 0xC3                           ; 08A6 and al,bl
db 0xFE, 0xC0                           ; 08A8 inc al
db 0x74, 0x03                           ; 08AA jz 0x8af
db 0xE8, 0x99, 0x5C                     ; 08AC call 0x6548
db 0xB0, 0xFF                           ; 08AF mov al,0xff
db 0xE8, 0xF1, 0x22                     ; 08B1 call 0x2ba5
db 0xB0, 0x59                           ; 08B4 mov al,0x59
db 0xCD, 0x8A                           ; 08B6 int 0x8a
db 0x32, 0xC0                           ; 08B8 xor al,al
db 0xA2, 0x6F, 0x00                     ; 08BA mov [0x6f],al
db 0xE8, 0x3B, 0x3C                     ; 08BD call 0x44fb
db 0xE8, 0x9A, 0x23                     ; 08C0 call 0x2c5d
db 0xBB, 0x2D, 0x07                     ; 08C3 mov bx,0x72d
db 0xE8, 0x8C, 0x72                     ; 08C6 call 0x7b55
db 0xA0, 0x28, 0x00                     ; 08C9 mov al,[0x28]
db 0x2C, 0x02                           ; 08CC sub al,0x2
db 0x75, 0x03                           ; 08CE jnz 0x8d3
db 0xE8, 0xEE, 0x2D                     ; 08D0 call 0x36c1
db 0xCD, 0x8B                           ; 08D3 int 0x8b
db 0xBB, 0xFF, 0xFF                     ; 08D5 mov bx,0xffff
db 0x89, 0x1E, 0x2E, 0x00               ; 08D8 mov [0x2e],bx
db 0xA0, 0x3E, 0x03                     ; 08DC mov al,[0x33e]
db 0x0A, 0xC0                           ; 08DF or al,al
db 0x74, 0x49                           ; 08E1 jz 0x92c
db 0x8B, 0x1E, 0x3F, 0x03               ; 08E3 mov bx,[0x33f]
db 0x53                                 ; 08E7 push bx
db 0xE8, 0x65, 0x5C                     ; 08E8 call 0x6550
db 0x5A                                 ; 08EB pop dx
db 0x52                                 ; 08EC push dx
db 0xE8, 0x77, 0x01                     ; 08ED call 0xa67
db 0xB0, 0x2A                           ; 08F0 mov al,0x2a
db 0x72, 0x02                           ; 08F2 jc 0x8f6
db 0xB0, 0x20                           ; 08F4 mov al,0x20
db 0xE8, 0xAC, 0x22                     ; 08F6 call 0x2ba5
db 0xE8, 0x84, 0x28                     ; 08F9 call 0x3180
db 0x5A                                 ; 08FC pop dx
db 0x73, 0x0E                           ; 08FD jnc 0x90d
db 0x32, 0xC0                           ; 08FF xor al,al
db 0xA2, 0x3E, 0x03                     ; 0901 mov [0x33e],al
db 0xEB, 0xB0                           ; 0904 jmp short 0x8b6
db 0x32, 0xC0                           ; 0906 xor al,al
db 0xA2, 0x3E, 0x03                     ; 0908 mov [0x33e],al
db 0xEB, 0x15                           ; 090B jmp short 0x922
db 0x8B, 0x1E, 0x41, 0x03               ; 090D mov bx,[0x341]
db 0x03, 0xDA                           ; 0911 add bx,dx
db 0x72, 0xF1                           ; 0913 jc 0x906
db 0x52                                 ; 0915 push dx
db 0xBA, 0xF9, 0xFF                     ; 0916 mov dx,0xfff9
db 0x3B, 0xDA                           ; 0919 cmp bx,dx
db 0x5A                                 ; 091B pop dx
db 0x73, 0xE8                           ; 091C jnc 0x906
db 0x89, 0x1E, 0x3F, 0x03               ; 091E mov [0x33f],bx
db 0xA0, 0xF7, 0x01                     ; 0922 mov al,[0x1f7]
db 0x0A, 0xC0                           ; 0925 or al,al
db 0x74, 0xAA                           ; 0927 jz 0x8d3
db 0xE9, 0xA8, 0x2D                     ; 0929 jmp 0x36d4
db 0xE8, 0x51, 0x28                     ; 092C call 0x3180
db 0x72, 0xA2                           ; 092F jc 0x8d3
db 0xE8, 0xE9, 0x05                     ; 0931 call 0xf1d
db 0xFE, 0xC0                           ; 0934 inc al
db 0xFE, 0xC8                           ; 0936 dec al
db 0x74, 0x99                           ; 0938 jz 0x8d3
db 0x9C                                 ; 093A pushf
db 0xE8, 0x2D, 0x07                     ; 093B call 0x106b
db 0x73, 0x08                           ; 093E jnc 0x948
db 0xE8, 0x93, 0x26                     ; 0940 call 0x2fd6
db 0x75, 0x03                           ; 0943 jnz 0x948
db 0xE9, 0x76, 0xFE                     ; 0945 jmp 0x7be
db 0xE8, 0x38, 0x04                     ; 0948 call 0xd83
db 0x8A, 0x07                           ; 094B mov al,[bx]
db 0x3C, 0x20                           ; 094D cmp al,0x20
db 0x75, 0x03                           ; 094F jnz 0x954
db 0xE8, 0xF0, 0x5B                     ; 0951 call 0x6544
db 0x52                                 ; 0954 push dx
db 0xE8, 0x31, 0x01                     ; 0955 call 0xa89
db 0x5A                                 ; 0958 pop dx
db 0x9D                                 ; 0959 popf
db 0x89, 0x1E, 0x43, 0x03               ; 095A mov [0x343],bx
db 0xCD, 0x8C                           ; 095E int 0x8c
db 0x72, 0x03                           ; 0960 jc 0x965
db 0xE9, 0x6F, 0x3B                     ; 0962 jmp 0x44d4
db 0x52                                 ; 0965 push dx
db 0x51                                 ; 0966 push cx
db 0xE8, 0xEE, 0x3D                     ; 0967 call 0x4758
db 0xE8, 0xB0, 0x05                     ; 096A call 0xf1d
db 0x0A, 0xC0                           ; 096D or al,al
db 0x9C                                 ; 096F pushf
db 0x89, 0x16, 0x49, 0x03               ; 0970 mov [0x349],dx
db 0xE8, 0xF0, 0x00                     ; 0974 call 0xa67
db 0x72, 0x09                           ; 0977 jc 0x982
db 0x9D                                 ; 0979 popf
db 0x9C                                 ; 097A pushf
db 0x75, 0x03                           ; 097B jnz 0x980
db 0xE9, 0xB0, 0x07                     ; 097D jmp 0x1130
db 0x0A, 0xC0                           ; 0980 or al,al
db 0x51                                 ; 0982 push cx
db 0x9C                                 ; 0983 pushf
db 0x53                                 ; 0984 push bx
db 0xE8, 0xAD, 0x1A                     ; 0985 call 0x2435
db 0x5B                                 ; 0988 pop bx
db 0x9D                                 ; 0989 popf
db 0x59                                 ; 098A pop cx
db 0x51                                 ; 098B push cx
db 0x73, 0x03                           ; 098C jnc 0x991
db 0xE8, 0xD6, 0x18                     ; 098E call 0x2267
db 0x5A                                 ; 0991 pop dx
db 0x9D                                 ; 0992 popf
db 0x52                                 ; 0993 push dx
db 0x74, 0x47                           ; 0994 jz 0x9dd
db 0x5A                                 ; 0996 pop dx
db 0xA0, 0x6B, 0x04                     ; 0997 mov al,[0x46b]
db 0x0A, 0xC0                           ; 099A or al,al
db 0x75, 0x08                           ; 099C jnz 0x9a6
db 0x8B, 0x1E, 0x0A, 0x03               ; 099E mov bx,[0x30a]
db 0x89, 0x1E, 0x2F, 0x03               ; 09A2 mov [0x32f],bx
db 0x8B, 0x1E, 0x58, 0x03               ; 09A6 mov bx,[0x358]
db 0x5E                                 ; 09AA pop si
db 0x87, 0xDE                           ; 09AB xchg bx,si
db 0x56                                 ; 09AD push si
db 0x59                                 ; 09AE pop cx
db 0x53                                 ; 09AF push bx
db 0x03, 0xD9                           ; 09B0 add bx,cx
db 0x53                                 ; 09B2 push bx
db 0xE8, 0x15, 0x5B                     ; 09B3 call 0x64cb
db 0x5B                                 ; 09B6 pop bx
db 0x89, 0x1E, 0x58, 0x03               ; 09B7 mov [0x358],bx
db 0x87, 0xDA                           ; 09BB xchg bx,dx
db 0x88, 0x3F                           ; 09BD mov [bx],bh
db 0x59                                 ; 09BF pop cx
db 0x5A                                 ; 09C0 pop dx
db 0x53                                 ; 09C1 push bx
db 0x43                                 ; 09C2 inc bx
db 0x43                                 ; 09C3 inc bx
db 0x89, 0x17                           ; 09C4 mov [bx],dx
db 0x43                                 ; 09C6 inc bx
db 0x43                                 ; 09C7 inc bx
db 0xBA, 0xB8, 0x00                     ; 09C8 mov dx,0xb8
db 0x49                                 ; 09CB dec cx
db 0x49                                 ; 09CC dec cx
db 0x49                                 ; 09CD dec cx
db 0x49                                 ; 09CE dec cx
db 0x8B, 0xF2                           ; 09CF mov si,dx
db 0xAC                                 ; 09D1 lodsb
db 0x88, 0x07                           ; 09D2 mov [bx],al
db 0x43                                 ; 09D4 inc bx
db 0x42                                 ; 09D5 inc dx
db 0x49                                 ; 09D6 dec cx
db 0x8A, 0xC1                           ; 09D7 mov al,cl
db 0x0A, 0xC5                           ; 09D9 or al,ch
db 0x75, 0xF2                           ; 09DB jnz 0x9cf
db 0xCD, 0x8D                           ; 09DD int 0x8d
db 0x5A                                 ; 09DF pop dx
db 0xE8, 0x1E, 0x00                     ; 09E0 call 0xa01
db 0x8B, 0x1E, 0xE9, 0x04               ; 09E3 mov bx,[0x4e9]
db 0x89, 0x1E, 0x52, 0x03               ; 09E7 mov [0x352],bx
db 0xE8, 0x49, 0x23                     ; 09EB call 0x2d37
db 0xCD, 0x8E                           ; 09EE int 0x8e
db 0x8B, 0x1E, 0x52, 0x03               ; 09F0 mov bx,[0x352]
db 0x89, 0x1E, 0xE9, 0x04               ; 09F4 mov [0x4e9],bx
db 0xE9, 0xD8, 0xFE                     ; 09F8 jmp 0x8d3
db 0x8B, 0x1E, 0x30, 0x00               ; 09FB mov bx,[0x30]
db 0x87, 0xDA                           ; 09FF xchg bx,dx
db 0x8A, 0xFE                           ; 0A01 mov bh,dh
db 0x8A, 0xDA                           ; 0A03 mov bl,dl
db 0x8A, 0x07                           ; 0A05 mov al,[bx]
db 0x43                                 ; 0A07 inc bx
db 0x0A, 0x07                           ; 0A08 or al,[bx]
db 0x75, 0x01                           ; 0A0A jnz 0xa0d
db 0xC3                                 ; 0A0C ret
db 0x43                                 ; 0A0D inc bx
db 0x43                                 ; 0A0E inc bx
db 0x43                                 ; 0A0F inc bx
db 0x8A, 0x07                           ; 0A10 mov al,[bx]
db 0x0A, 0xC0                           ; 0A12 or al,al
db 0x74, 0x10                           ; 0A14 jz 0xa26
db 0x3C, 0x20                           ; 0A16 cmp al,0x20
db 0x73, 0xF5                           ; 0A18 jnc 0xa0f
db 0x3C, 0x0B                           ; 0A1A cmp al,0xb
db 0x72, 0xF1                           ; 0A1C jc 0xa0f
db 0xE8, 0xFD, 0x04                     ; 0A1E call 0xf1e
db 0xE8, 0xF9, 0x04                     ; 0A21 call 0xf1d
db 0xEB, 0xEC                           ; 0A24 jmp short 0xa12
db 0x43                                 ; 0A26 inc bx
db 0x87, 0xDA                           ; 0A27 xchg bx,dx
db 0x89, 0x17                           ; 0A29 mov [bx],dx
db 0xEB, 0xD4                           ; 0A2B jmp short 0xa01
db 0xBA, 0x00, 0x00                     ; 0A2D mov dx,0x0
db 0x52                                 ; 0A30 push dx
db 0x74, 0x17                           ; 0A31 jz 0xa4a
db 0x3C, 0x2C                           ; 0A33 cmp al,0x2c
db 0x74, 0x13                           ; 0A35 jz 0xa4a
db 0x5A                                 ; 0A37 pop dx
db 0xE8, 0x23, 0x06                     ; 0A38 call 0x105e
db 0x52                                 ; 0A3B push dx
db 0x74, 0x1D                           ; 0A3C jz 0xa5b
db 0x3C, 0x2C                           ; 0A3E cmp al,0x2c
db 0x74, 0x19                           ; 0A40 jz 0xa5b
db 0xE8, 0xAF, 0x23                     ; 0A42 call 0x2df4
db 0xEA, 0x74, 0x02, 0x3C, 0x2C         ; 0A45 jmp 0x2c3c:0x274
db 0xBA, 0xFA, 0xFF                     ; 0A4A mov dx,0xfffa
db 0x74, 0x03                           ; 0A4D jz 0xa52
db 0xE8, 0x0C, 0x06                     ; 0A4F call 0x105e
db 0x74, 0x07                           ; 0A52 jz 0xa5b
db 0x3C, 0x2C                           ; 0A54 cmp al,0x2c
db 0x74, 0x03                           ; 0A56 jz 0xa5b
db 0xE9, 0x63, 0xFD                     ; 0A58 jmp 0x7be
db 0x89, 0x1E, 0x3B, 0x03               ; 0A5B mov [0x33b],bx
db 0x87, 0xDA                           ; 0A5F xchg bx,dx
db 0x5A                                 ; 0A61 pop dx
db 0x5E                                 ; 0A62 pop si
db 0x87, 0xDE                           ; 0A63 xchg bx,si
db 0x56                                 ; 0A65 push si
db 0x53                                 ; 0A66 push bx
db 0x8B, 0x1E, 0x30, 0x00               ; 0A67 mov bx,[0x30]
db 0x8B, 0xCB                           ; 0A6B mov cx,bx
db 0x8A, 0x07                           ; 0A6D mov al,[bx]
db 0x43                                 ; 0A6F inc bx
db 0x0A, 0x07                           ; 0A70 or al,[bx]
db 0x9F                                 ; 0A72 lahf
db 0x4B                                 ; 0A73 dec bx
db 0x9E                                 ; 0A74 sahf
db 0x74, 0x95                           ; 0A75 jz 0xa0c
db 0x43                                 ; 0A77 inc bx
db 0x43                                 ; 0A78 inc bx
db 0x8B, 0x1F                           ; 0A79 mov bx,[bx]
db 0x3B, 0xDA                           ; 0A7B cmp bx,dx
db 0x8B, 0xD9                           ; 0A7D mov bx,cx
db 0x8B, 0x1F                           ; 0A7F mov bx,[bx]
db 0xF5                                 ; 0A81 cmc
db 0x74, 0x88                           ; 0A82 jz 0xa0c
db 0xF5                                 ; 0A84 cmc
db 0x73, 0x85                           ; 0A85 jnc 0xa0c
db 0xEB, 0xE2                           ; 0A87 jmp short 0xa6b
db 0x32, 0xC0                           ; 0A89 xor al,al
db 0xA2, 0xFD, 0x02                     ; 0A8B mov [0x2fd],al
db 0xA2, 0xFC, 0x02                     ; 0A8E mov [0x2fc],al
db 0xCD, 0x8F                           ; 0A91 int 0x8f
db 0xB9, 0x3B, 0x01                     ; 0A93 mov cx,0x13b
db 0xBA, 0xB8, 0x00                     ; 0A96 mov dx,0xb8
db 0x8A, 0x07                           ; 0A99 mov al,[bx]
db 0x0A, 0xC0                           ; 0A9B or al,al
db 0x75, 0x20                           ; 0A9D jnz 0xabf
db 0xBB, 0x40, 0x01                     ; 0A9F mov bx,0x140
db 0x8A, 0xC3                           ; 0AA2 mov al,bl
db 0x2A, 0xC1                           ; 0AA4 sub al,cl
db 0x8A, 0xC8                           ; 0AA6 mov cl,al
db 0x8A, 0xC7                           ; 0AA8 mov al,bh
db 0x1A, 0xC5                           ; 0AAA sbb al,ch
db 0x8A, 0xE8                           ; 0AAC mov ch,al
db 0xBB, 0xB7, 0x00                     ; 0AAE mov bx,0xb7
db 0x32, 0xC0                           ; 0AB1 xor al,al
db 0x8B, 0xFA                           ; 0AB3 mov di,dx
db 0xAA                                 ; 0AB5 stosb
db 0x42                                 ; 0AB6 inc dx
db 0x8B, 0xFA                           ; 0AB7 mov di,dx
db 0xAA                                 ; 0AB9 stosb
db 0x42                                 ; 0ABA inc dx
db 0x8B, 0xFA                           ; 0ABB mov di,dx
db 0xAA                                 ; 0ABD stosb
db 0xC3                                 ; 0ABE ret
db 0x3C, 0x22                           ; 0ABF cmp al,0x22
db 0x75, 0x03                           ; 0AC1 jnz 0xac6
db 0xE9, 0x33, 0x00                     ; 0AC3 jmp 0xaf9
db 0x3C, 0x20                           ; 0AC6 cmp al,0x20
db 0x74, 0x09                           ; 0AC8 jz 0xad3
db 0xA0, 0xFC, 0x02                     ; 0ACA mov al,[0x2fc]
db 0x0A, 0xC0                           ; 0ACD or al,al
db 0x8A, 0x07                           ; 0ACF mov al,[bx]
db 0x74, 0x2F                           ; 0AD1 jz 0xb02
db 0x43                                 ; 0AD3 inc bx
db 0x50                                 ; 0AD4 push ax
db 0xE8, 0x54, 0x02                     ; 0AD5 call 0xd2c
db 0x58                                 ; 0AD8 pop ax
db 0x2C, 0x3A                           ; 0AD9 sub al,0x3a
db 0x74, 0x06                           ; 0ADB jz 0xae3
db 0x3C, 0x4A                           ; 0ADD cmp al,0x4a
db 0x75, 0x08                           ; 0ADF jnz 0xae9
db 0xB0, 0x01                           ; 0AE1 mov al,0x1
db 0xA2, 0xFC, 0x02                     ; 0AE3 mov [0x2fc],al
db 0xA2, 0xFD, 0x02                     ; 0AE6 mov [0x2fd],al
db 0x2C, 0x55                           ; 0AE9 sub al,0x55
db 0x75, 0xAC                           ; 0AEB jnz 0xa99
db 0x50                                 ; 0AED push ax
db 0x8A, 0x07                           ; 0AEE mov al,[bx]
db 0x0A, 0xC0                           ; 0AF0 or al,al
db 0x58                                 ; 0AF2 pop ax
db 0x74, 0xAA                           ; 0AF3 jz 0xa9f
db 0x3A, 0x07                           ; 0AF5 cmp al,[bx]
db 0x74, 0xDA                           ; 0AF7 jz 0xad3
db 0x50                                 ; 0AF9 push ax
db 0x8A, 0x07                           ; 0AFA mov al,[bx]
db 0x43                                 ; 0AFC inc bx
db 0xE8, 0x2C, 0x02                     ; 0AFD call 0xd2c
db 0xEB, 0xEC                           ; 0B00 jmp short 0xaee
db 0x43                                 ; 0B02 inc bx
db 0x0A, 0xC0                           ; 0B03 or al,al
db 0x78, 0x92                           ; 0B05 js 0xa99
db 0x4B                                 ; 0B07 dec bx
db 0x3C, 0x3F                           ; 0B08 cmp al,0x3f
db 0xB0, 0x91                           ; 0B0A mov al,0x91
db 0x52                                 ; 0B0C push dx
db 0x51                                 ; 0B0D push cx
db 0x75, 0x03                           ; 0B0E jnz 0xb13
db 0xE9, 0xE2, 0x00                     ; 0B10 jmp 0xbf5
db 0xBA, 0x6B, 0x03                     ; 0B13 mov dx,0x36b
db 0xE8, 0xD2, 0x0E                     ; 0B16 call 0x19eb
db 0xE8, 0x29, 0x24                     ; 0B19 call 0x2f45
db 0x73, 0x03                           ; 0B1C jnc 0xb21
db 0xE9, 0x2E, 0x01                     ; 0B1E jmp 0xc4f
db 0x53                                 ; 0B21 push bx
db 0xBA, 0x5E, 0x0B                     ; 0B22 mov dx,0xb5e
db 0xE8, 0x20, 0x00                     ; 0B25 call 0xb48
db 0x75, 0x3E                           ; 0B28 jnz 0xb68
db 0xE8, 0xF0, 0x03                     ; 0B2A call 0xf1d
db 0xBA, 0x62, 0x0B                     ; 0B2D mov dx,0xb62
db 0xE8, 0x15, 0x00                     ; 0B30 call 0xb48
db 0xB0, 0x89                           ; 0B33 mov al,0x89
db 0x75, 0x03                           ; 0B35 jnz 0xb3a
db 0xEB, 0x0B                           ; 0B37 jmp short 0xb44
db 0x90                                 ; 0B39 nop
db 0xBA, 0x65, 0x0B                     ; 0B3A mov dx,0xb65
db 0xE8, 0x08, 0x00                     ; 0B3D call 0xb48
db 0x75, 0x26                           ; 0B40 jnz 0xb68
db 0xB0, 0x8D                           ; 0B42 mov al,0x8d
db 0x59                                 ; 0B44 pop cx
db 0xE9, 0xAD, 0x00                     ; 0B45 jmp 0xbf5
db 0x8B, 0xF2                           ; 0B48 mov si,dx
db 0x2E, 0xAC                           ; 0B4A cs lodsb
db 0x0A, 0xC0                           ; 0B4C or al,al
db 0x75, 0x01                           ; 0B4E jnz 0xb51
db 0xC3                                 ; 0B50 ret
db 0x8A, 0xC8                           ; 0B51 mov cl,al
db 0xE8, 0x95, 0x0E                     ; 0B53 call 0x19eb
db 0x3A, 0xC1                           ; 0B56 cmp al,cl
db 0x75, 0xF6                           ; 0B58 jnz 0xb50
db 0x43                                 ; 0B5A inc bx
db 0x42                                 ; 0B5B inc dx
db 0xEB, 0xEA                           ; 0B5C jmp short 0xb48
db 0x47                                 ; 0B5E inc di
db 0x4F                                 ; 0B5F dec di
db 0x20, 0x00                           ; 0B60 and [bx+si],al
db 0x54                                 ; 0B62 push sp
db 0x4F                                 ; 0B63 dec di
db 0x00, 0x55, 0x42                     ; 0B64 add [di+0x42],dl
db 0x00, 0x5B, 0xE8                     ; 0B67 add [bp+di-0x18],bl
db 0x7F, 0x0E                           ; 0B6A jg 0xb7a
db 0x53                                 ; 0B6C push bx
db 0xCD, 0x90                           ; 0B6D int 0x90
db 0xBB, 0x03, 0x01                     ; 0B6F mov bx,0x103
db 0x2C, 0x41                           ; 0B72 sub al,0x41
db 0x02, 0xC0                           ; 0B74 add al,al
db 0x8A, 0xC8                           ; 0B76 mov cl,al
db 0xB5, 0x00                           ; 0B78 mov ch,0x0
db 0x03, 0xD9                           ; 0B7A add bx,cx
db 0x2E, 0x8B, 0x17                     ; 0B7C mov dx,[cs:bx]
db 0x5B                                 ; 0B7F pop bx
db 0x43                                 ; 0B80 inc bx
db 0x53                                 ; 0B81 push bx
db 0xE8, 0x66, 0x0E                     ; 0B82 call 0x19eb
db 0x8A, 0xC8                           ; 0B85 mov cl,al
db 0x8B, 0xF2                           ; 0B87 mov si,dx
db 0x2E, 0xAC                           ; 0B89 cs lodsb
db 0x24, 0x7F                           ; 0B8B and al,0x7f
db 0x75, 0x03                           ; 0B8D jnz 0xb92
db 0xE9, 0xAB, 0x01                     ; 0B8F jmp 0xd3d
db 0x43                                 ; 0B92 inc bx
db 0x3A, 0xC1                           ; 0B93 cmp al,cl
db 0x75, 0x50                           ; 0B95 jnz 0xbe7
db 0x8B, 0xF2                           ; 0B97 mov si,dx
db 0x2E, 0xAC                           ; 0B99 cs lodsb
db 0x42                                 ; 0B9B inc dx
db 0x0A, 0xC0                           ; 0B9C or al,al
db 0x79, 0xE2                           ; 0B9E jns 0xb82
db 0x8A, 0xC1                           ; 0BA0 mov al,cl
db 0x3C, 0x28                           ; 0BA2 cmp al,0x28
db 0x74, 0x1D                           ; 0BA4 jz 0xbc3
db 0x8B, 0xF2                           ; 0BA6 mov si,dx
db 0x2E, 0xAC                           ; 0BA8 cs lodsb
db 0x3C, 0xD1                           ; 0BAA cmp al,0xd1
db 0x74, 0x15                           ; 0BAC jz 0xbc3
db 0x3C, 0xD0                           ; 0BAE cmp al,0xd0
db 0x74, 0x11                           ; 0BB0 jz 0xbc3
db 0xE8, 0x36, 0x0E                     ; 0BB2 call 0x19eb
db 0x3C, 0x2E                           ; 0BB5 cmp al,0x2e
db 0x74, 0x03                           ; 0BB7 jz 0xbbc
db 0xE8, 0xC7, 0x15                     ; 0BB9 call 0x2183
db 0xB0, 0x00                           ; 0BBC mov al,0x0
db 0x72, 0x03                           ; 0BBE jc 0xbc3
db 0xE9, 0x7A, 0x01                     ; 0BC0 jmp 0xd3d
db 0x58                                 ; 0BC3 pop ax
db 0x8B, 0xF2                           ; 0BC4 mov si,dx
db 0x2E, 0xAC                           ; 0BC6 cs lodsb
db 0xCD, 0x91                           ; 0BC8 int 0x91
db 0x0A, 0xC0                           ; 0BCA or al,al
db 0x79, 0x03                           ; 0BCC jns 0xbd1
db 0xE9, 0x23, 0x00                     ; 0BCE jmp 0xbf4
db 0x59                                 ; 0BD1 pop cx
db 0x5A                                 ; 0BD2 pop dx
db 0x0C, 0x80                           ; 0BD3 or al,0x80
db 0x50                                 ; 0BD5 push ax
db 0xB0, 0xFF                           ; 0BD6 mov al,0xff
db 0xE8, 0x51, 0x01                     ; 0BD8 call 0xd2c
db 0x32, 0xC0                           ; 0BDB xor al,al
db 0xA2, 0xFD, 0x02                     ; 0BDD mov [0x2fd],al
db 0x58                                 ; 0BE0 pop ax
db 0xE8, 0x48, 0x01                     ; 0BE1 call 0xd2c
db 0xE9, 0xB2, 0xFE                     ; 0BE4 jmp 0xa99
db 0x5B                                 ; 0BE7 pop bx
db 0x8B, 0xF2                           ; 0BE8 mov si,dx
db 0x2E, 0xAC                           ; 0BEA cs lodsb
db 0x42                                 ; 0BEC inc dx
db 0x0A, 0xC0                           ; 0BED or al,al
db 0x79, 0xF7                           ; 0BEF jns 0xbe8
db 0x42                                 ; 0BF1 inc dx
db 0xEB, 0x8D                           ; 0BF2 jmp short 0xb81
db 0x4B                                 ; 0BF4 dec bx
db 0x50                                 ; 0BF5 push ax
db 0xCD, 0x92                           ; 0BF6 int 0x92
db 0xBA, 0x0C, 0x0C                     ; 0BF8 mov dx,0xc0c
db 0x8A, 0xC8                           ; 0BFB mov cl,al
db 0x8B, 0xF2                           ; 0BFD mov si,dx
db 0x2E, 0xAC                           ; 0BFF cs lodsb
db 0x0A, 0xC0                           ; 0C01 or al,al
db 0x74, 0x17                           ; 0C03 jz 0xc1c
db 0x42                                 ; 0C05 inc dx
db 0x3A, 0xC1                           ; 0C06 cmp al,cl
db 0x75, 0xF3                           ; 0C08 jnz 0xbfd
db 0xEB, 0x14                           ; 0C0A jmp short 0xc20
db 0x8C, 0xAA, 0xAB, 0xA9               ; 0C0C mov [bp+si+0xa9ab],gs
db 0xA6                                 ; 0C10 cmpsb
db 0xA8, 0xD4                           ; 0C11 test al,0xd4
db 0xA1, 0x8A, 0x93                     ; 0C13 mov ax,[0x938a]
db 0x9E                                 ; 0C16 sahf
db 0x89, 0x8E, 0xCD, 0x8D               ; 0C17 mov [bp+0x8dcd],cx
db 0x00, 0x32                           ; 0C1B add [bp+si],dh
db 0xC0, 0xEB, 0x02                     ; 0C1D shr bl,0x2
db 0xB0, 0x01                           ; 0C20 mov al,0x1
db 0xA2, 0xFD, 0x02                     ; 0C22 mov [0x2fd],al
db 0x58                                 ; 0C25 pop ax
db 0x59                                 ; 0C26 pop cx
db 0x5A                                 ; 0C27 pop dx
db 0x3C, 0xA1                           ; 0C28 cmp al,0xa1
db 0x50                                 ; 0C2A push ax
db 0x75, 0x03                           ; 0C2B jnz 0xc30
db 0xE8, 0xFA, 0x00                     ; 0C2D call 0xd2a
db 0x58                                 ; 0C30 pop ax
db 0x3C, 0xB1                           ; 0C31 cmp al,0xb1
db 0x75, 0x05                           ; 0C33 jnz 0xc3a
db 0xE8, 0xF4, 0x00                     ; 0C35 call 0xd2c
db 0xB0, 0xE9                           ; 0C38 mov al,0xe9
db 0x3C, 0xD9                           ; 0C3A cmp al,0xd9
db 0x74, 0x03                           ; 0C3C jz 0xc41
db 0xE9, 0xC6, 0x00                     ; 0C3E jmp 0xd07
db 0x50                                 ; 0C41 push ax
db 0xE8, 0xE5, 0x00                     ; 0C42 call 0xd2a
db 0xB0, 0x8F                           ; 0C45 mov al,0x8f
db 0xE8, 0xE2, 0x00                     ; 0C47 call 0xd2c
db 0x58                                 ; 0C4A pop ax
db 0x50                                 ; 0C4B push ax
db 0xE9, 0xAD, 0xFE                     ; 0C4C jmp 0xafc
db 0x8A, 0x07                           ; 0C4F mov al,[bx]
db 0x3C, 0x2E                           ; 0C51 cmp al,0x2e
db 0x74, 0x0E                           ; 0C53 jz 0xc63
db 0x3C, 0x3A                           ; 0C55 cmp al,0x3a
db 0x72, 0x03                           ; 0C57 jc 0xc5c
db 0xE9, 0x90, 0x00                     ; 0C59 jmp 0xcec
db 0x3C, 0x30                           ; 0C5C cmp al,0x30
db 0x73, 0x03                           ; 0C5E jnc 0xc63
db 0xE9, 0x89, 0x00                     ; 0C60 jmp 0xcec
db 0xA0, 0xFD, 0x02                     ; 0C63 mov al,[0x2fd]
db 0x0A, 0xC0                           ; 0C66 or al,al
db 0x8A, 0x07                           ; 0C68 mov al,[bx]
db 0x59                                 ; 0C6A pop cx
db 0x5A                                 ; 0C6B pop dx
db 0x79, 0x03                           ; 0C6C jns 0xc71
db 0xE9, 0x62, 0xFE                     ; 0C6E jmp 0xad3
db 0x74, 0x27                           ; 0C71 jz 0xc9a
db 0x3C, 0x2E                           ; 0C73 cmp al,0x2e
db 0x75, 0x03                           ; 0C75 jnz 0xc7a
db 0xE9, 0x59, 0xFE                     ; 0C77 jmp 0xad3
db 0xB0, 0x0E                           ; 0C7A mov al,0xe
db 0xE8, 0xAD, 0x00                     ; 0C7C call 0xd2c
db 0x52                                 ; 0C7F push dx
db 0xE8, 0xE8, 0x03                     ; 0C80 call 0x106b
db 0xE8, 0xFD, 0x00                     ; 0C83 call 0xd83
db 0x5E                                 ; 0C86 pop si
db 0x87, 0xDE                           ; 0C87 xchg bx,si
db 0x56                                 ; 0C89 push si
db 0x87, 0xDA                           ; 0C8A xchg bx,dx
db 0x8A, 0xC3                           ; 0C8C mov al,bl
db 0xE8, 0x9B, 0x00                     ; 0C8E call 0xd2c
db 0x8A, 0xC7                           ; 0C91 mov al,bh
db 0x5B                                 ; 0C93 pop bx
db 0xE8, 0x95, 0x00                     ; 0C94 call 0xd2c
db 0xE9, 0xFF, 0xFD                     ; 0C97 jmp 0xa99
db 0x52                                 ; 0C9A push dx
db 0x51                                 ; 0C9B push cx
db 0x8A, 0x07                           ; 0C9C mov al,[bx]
db 0xE8, 0x1F, 0x5D                     ; 0C9E call 0x69c0
db 0xE8, 0xDF, 0x00                     ; 0CA1 call 0xd83
db 0x59                                 ; 0CA4 pop cx
db 0x5A                                 ; 0CA5 pop dx
db 0x53                                 ; 0CA6 push bx
db 0xA0, 0xFB, 0x02                     ; 0CA7 mov al,[0x2fb]
db 0x3C, 0x02                           ; 0CAA cmp al,0x2
db 0x75, 0x1A                           ; 0CAC jnz 0xcc8
db 0x8B, 0x1E, 0xA3, 0x04               ; 0CAE mov bx,[0x4a3]
db 0x8A, 0xC7                           ; 0CB2 mov al,bh
db 0x0A, 0xC0                           ; 0CB4 or al,al
db 0xB0, 0x02                           ; 0CB6 mov al,0x2
db 0x75, 0x0E                           ; 0CB8 jnz 0xcc8
db 0x8A, 0xC3                           ; 0CBA mov al,bl
db 0x8A, 0xFB                           ; 0CBC mov bh,bl
db 0xB3, 0x0F                           ; 0CBE mov bl,0xf
db 0x3C, 0x0A                           ; 0CC0 cmp al,0xa
db 0x73, 0xC8                           ; 0CC2 jnc 0xc8c
db 0x04, 0x11                           ; 0CC4 add al,0x11
db 0xEB, 0xCB                           ; 0CC6 jmp short 0xc93
db 0x50                                 ; 0CC8 push ax
db 0xD0, 0xC8                           ; 0CC9 ror al,1
db 0x04, 0x1B                           ; 0CCB add al,0x1b
db 0xE8, 0x5C, 0x00                     ; 0CCD call 0xd2c
db 0xBB, 0xA3, 0x04                     ; 0CD0 mov bx,0x4a3
db 0xE8, 0x4F, 0x0E                     ; 0CD3 call 0x1b25
db 0x72, 0x03                           ; 0CD6 jc 0xcdb
db 0xBB, 0x9F, 0x04                     ; 0CD8 mov bx,0x49f
db 0x58                                 ; 0CDB pop ax
db 0x50                                 ; 0CDC push ax
db 0x8A, 0x07                           ; 0CDD mov al,[bx]
db 0xE8, 0x4A, 0x00                     ; 0CDF call 0xd2c
db 0x58                                 ; 0CE2 pop ax
db 0x43                                 ; 0CE3 inc bx
db 0xFE, 0xC8                           ; 0CE4 dec al
db 0x75, 0xF4                           ; 0CE6 jnz 0xcdc
db 0x5B                                 ; 0CE8 pop bx
db 0xE9, 0xAD, 0xFD                     ; 0CE9 jmp 0xa99
db 0xBA, 0x6A, 0x03                     ; 0CEC mov dx,0x36a
db 0x42                                 ; 0CEF inc dx
db 0x8B, 0xF2                           ; 0CF0 mov si,dx
db 0x2E, 0xAC                           ; 0CF2 cs lodsb
db 0x24, 0x7F                           ; 0CF4 and al,0x7f
db 0x75, 0x03                           ; 0CF6 jnz 0xcfb
db 0xE9, 0x6B, 0x00                     ; 0CF8 jmp 0xd66
db 0x42                                 ; 0CFB inc dx
db 0x3A, 0x07                           ; 0CFC cmp al,[bx]
db 0x8B, 0xF2                           ; 0CFE mov si,dx
db 0x2E, 0xAC                           ; 0D00 cs lodsb
db 0x75, 0xEB                           ; 0D02 jnz 0xcef
db 0xE9, 0x6F, 0x00                     ; 0D04 jmp 0xd76
db 0x3C, 0x26                           ; 0D07 cmp al,0x26
db 0x74, 0x03                           ; 0D09 jz 0xd0e
db 0xE9, 0xC5, 0xFD                     ; 0D0B jmp 0xad3
db 0x53                                 ; 0D0E push bx
db 0xE8, 0x0B, 0x02                     ; 0D0F call 0xf1d
db 0x5B                                 ; 0D12 pop bx
db 0xE8, 0xD7, 0x0C                     ; 0D13 call 0x19ed
db 0x3C, 0x48                           ; 0D16 cmp al,0x48
db 0xB0, 0x0B                           ; 0D18 mov al,0xb
db 0x75, 0x02                           ; 0D1A jnz 0xd1e
db 0xB0, 0x0C                           ; 0D1C mov al,0xc
db 0xE8, 0x0B, 0x00                     ; 0D1E call 0xd2c
db 0x52                                 ; 0D21 push dx
db 0x51                                 ; 0D22 push cx
db 0xE8, 0xD9, 0x0C                     ; 0D23 call 0x19ff
db 0x59                                 ; 0D26 pop cx
db 0xE9, 0x5C, 0xFF                     ; 0D27 jmp 0xc86
db 0xB0, 0x3A                           ; 0D2A mov al,0x3a
db 0x8B, 0xFA                           ; 0D2C mov di,dx
db 0xAA                                 ; 0D2E stosb
db 0x42                                 ; 0D2F inc dx
db 0x49                                 ; 0D30 dec cx
db 0x8A, 0xC1                           ; 0D31 mov al,cl
db 0x0A, 0xC5                           ; 0D33 or al,ch
db 0x74, 0x01                           ; 0D35 jz 0xd38
db 0xC3                                 ; 0D37 ret
db 0xB2, 0x17                           ; 0D38 mov dl,0x17
db 0xE9, 0x9B, 0xFA                     ; 0D3A jmp 0x7d8
db 0xCD, 0x93                           ; 0D3D int 0x93
db 0x5B                                 ; 0D3F pop bx
db 0x4B                                 ; 0D40 dec bx
db 0xFE, 0xC8                           ; 0D41 dec al
db 0xA2, 0xFD, 0x02                     ; 0D43 mov [0x2fd],al
db 0x59                                 ; 0D46 pop cx
db 0x5A                                 ; 0D47 pop dx
db 0xE8, 0xA0, 0x0C                     ; 0D48 call 0x19eb
db 0xE8, 0xDE, 0xFF                     ; 0D4B call 0xd2c
db 0x43                                 ; 0D4E inc bx
db 0xE8, 0x99, 0x0C                     ; 0D4F call 0x19eb
db 0xE8, 0xF0, 0x21                     ; 0D52 call 0x2f45
db 0x73, 0xF4                           ; 0D55 jnc 0xd4b
db 0x3C, 0x3A                           ; 0D57 cmp al,0x3a
db 0x73, 0x08                           ; 0D59 jnc 0xd63
db 0x3C, 0x30                           ; 0D5B cmp al,0x30
db 0x73, 0xEC                           ; 0D5D jnc 0xd4b
db 0x3C, 0x2E                           ; 0D5F cmp al,0x2e
db 0x74, 0xE8                           ; 0D61 jz 0xd4b
db 0xE9, 0x33, 0xFD                     ; 0D63 jmp 0xa99
db 0x8A, 0x07                           ; 0D66 mov al,[bx]
db 0x3C, 0x20                           ; 0D68 cmp al,0x20
db 0x73, 0x0A                           ; 0D6A jnc 0xd76
db 0x3C, 0x09                           ; 0D6C cmp al,0x9
db 0x74, 0x06                           ; 0D6E jz 0xd76
db 0x3C, 0x0A                           ; 0D70 cmp al,0xa
db 0x74, 0x02                           ; 0D72 jz 0xd76
db 0xB0, 0x20                           ; 0D74 mov al,0x20
db 0x50                                 ; 0D76 push ax
db 0xA0, 0xFD, 0x02                     ; 0D77 mov al,[0x2fd]
db 0xFE, 0xC0                           ; 0D7A inc al
db 0x74, 0x02                           ; 0D7C jz 0xd80
db 0xFE, 0xC8                           ; 0D7E dec al
db 0xE9, 0x9F, 0xFE                     ; 0D80 jmp 0xc22
db 0x4B                                 ; 0D83 dec bx
db 0x8A, 0x07                           ; 0D84 mov al,[bx]
db 0x3C, 0x20                           ; 0D86 cmp al,0x20
db 0x74, 0xF9                           ; 0D88 jz 0xd83
db 0x3C, 0x09                           ; 0D8A cmp al,0x9
db 0x74, 0xF5                           ; 0D8C jz 0xd83
db 0x3C, 0x0A                           ; 0D8E cmp al,0xa
db 0x74, 0xF1                           ; 0D90 jz 0xd83
db 0x43                                 ; 0D92 inc bx
db 0xC3                                 ; 0D93 ret
db 0xB0, 0x64                           ; 0D94 mov al,0x64
db 0xA2, 0x39, 0x03                     ; 0D96 mov [0x339],al
db 0xE8, 0xD2, 0x29                     ; 0D99 call 0x376e
db 0xE8, 0x55, 0x20                     ; 0D9C call 0x2df4
db 0xE7, 0x52                           ; 0D9F out 0x52,ax
db 0x89, 0x16, 0x3B, 0x03               ; 0DA1 mov [0x33b],dx
db 0xA0, 0xFB, 0x02                     ; 0DA5 mov al,[0x2fb]
db 0x50                                 ; 0DA8 push ax
db 0xE8, 0x7B, 0x09                     ; 0DA9 call 0x1727
db 0x58                                 ; 0DAC pop ax
db 0x53                                 ; 0DAD push bx
db 0xE8, 0x29, 0x10                     ; 0DAE call 0x1dda
db 0xBB, 0x56, 0x04                     ; 0DB1 mov bx,0x456
db 0xE8, 0x45, 0x56                     ; 0DB4 call 0x63fc
db 0x5B                                 ; 0DB7 pop bx
db 0x5A                                 ; 0DB8 pop dx
db 0x59                                 ; 0DB9 pop cx
db 0x53                                 ; 0DBA push bx
db 0xE8, 0x9D, 0x03                     ; 0DBB call 0x115b
db 0x89, 0x1E, 0x35, 0x03               ; 0DBE mov [0x335],bx
db 0xBB, 0x02, 0x00                     ; 0DC2 mov bx,0x2
db 0x03, 0xDC                           ; 0DC5 add bx,sp
db 0xE8, 0x73, 0xF9                     ; 0DC7 call 0x73d
db 0x75, 0x1E                           ; 0DCA jnz 0xdea
db 0x03, 0xD9                           ; 0DCC add bx,cx
db 0x52                                 ; 0DCE push dx
db 0x4B                                 ; 0DCF dec bx
db 0x8A, 0x37                           ; 0DD0 mov dh,[bx]
db 0x4B                                 ; 0DD2 dec bx
db 0x8A, 0x17                           ; 0DD3 mov dl,[bx]
db 0x43                                 ; 0DD5 inc bx
db 0x43                                 ; 0DD6 inc bx
db 0x53                                 ; 0DD7 push bx
db 0x8B, 0x1E, 0x35, 0x03               ; 0DD8 mov bx,[0x335]
db 0x3B, 0xDA                           ; 0DDC cmp bx,dx
db 0x5B                                 ; 0DDE pop bx
db 0x5A                                 ; 0DDF pop dx
db 0x75, 0xE5                           ; 0DE0 jnz 0xdc7
db 0x5A                                 ; 0DE2 pop dx
db 0x8B, 0xE3                           ; 0DE3 mov sp,bx
db 0x89, 0x1E, 0x45, 0x03               ; 0DE5 mov [0x345],bx
db 0xB1, 0x5A                           ; 0DE9 mov cl,0x5a
db 0x87, 0xDA                           ; 0DEB xchg bx,dx
db 0xB1, 0x08                           ; 0DED mov cl,0x8
db 0xE8, 0xE3, 0x1E                     ; 0DEF call 0x2cd5
db 0x53                                 ; 0DF2 push bx
db 0x8B, 0x1E, 0x35, 0x03               ; 0DF3 mov bx,[0x335]
db 0x5E                                 ; 0DF7 pop si
db 0x87, 0xDE                           ; 0DF8 xchg bx,si
db 0x56                                 ; 0DFA push si
db 0x53                                 ; 0DFB push bx
db 0x8B, 0x1E, 0x2E, 0x00               ; 0DFC mov bx,[0x2e]
db 0x5E                                 ; 0E00 pop si
db 0x87, 0xDE                           ; 0E01 xchg bx,si
db 0x56                                 ; 0E03 push si
db 0xE8, 0xED, 0x1F                     ; 0E04 call 0x2df4
db 0xCC                                 ; 0E07 int3
db 0xE8, 0x1A, 0x0D                     ; 0E08 call 0x1b25
db 0x75, 0x03                           ; 0E0B jnz 0xe10
db 0xE9, 0xC6, 0xF9                     ; 0E0D jmp 0x7d6
db 0x72, 0x03                           ; 0E10 jc 0xe15
db 0xE9, 0xC1, 0xF9                     ; 0E12 jmp 0x7d6
db 0x9C                                 ; 0E15 pushf
db 0xE8, 0x0E, 0x09                     ; 0E16 call 0x1727
db 0x9D                                 ; 0E19 popf
db 0x53                                 ; 0E1A push bx
db 0x78, 0x03                           ; 0E1B js 0xe20
db 0xE9, 0x1C, 0x00                     ; 0E1D jmp 0xe3c
db 0xE8, 0x8A, 0x5D                     ; 0E20 call 0x6bad
db 0x5E                                 ; 0E23 pop si
db 0x87, 0xDE                           ; 0E24 xchg bx,si
db 0x56                                 ; 0E26 push si
db 0xBA, 0x01, 0x00                     ; 0E27 mov dx,0x1
db 0x8A, 0x07                           ; 0E2A mov al,[bx]
db 0x3C, 0xCF                           ; 0E2C cmp al,0xcf
db 0x75, 0x03                           ; 0E2E jnz 0xe33
db 0xE8, 0xD4, 0x10                     ; 0E30 call 0x1f07
db 0x52                                 ; 0E33 push dx
db 0x53                                 ; 0E34 push bx
db 0x87, 0xDA                           ; 0E35 xchg bx,dx
db 0xE8, 0xC6, 0x56                     ; 0E37 call 0x6500
db 0xEB, 0x27                           ; 0E3A jmp short 0xe63
db 0xE8, 0x12, 0x5D                     ; 0E3C call 0x6b51
db 0xE8, 0xB1, 0x55                     ; 0E3F call 0x63f3
db 0x5B                                 ; 0E42 pop bx
db 0x51                                 ; 0E43 push cx
db 0x52                                 ; 0E44 push dx
db 0xB9, 0x00, 0x81                     ; 0E45 mov cx,0x8100
db 0x8A, 0xF1                           ; 0E48 mov dh,cl
db 0x8A, 0xD6                           ; 0E4A mov dl,dh
db 0xCD, 0x94                           ; 0E4C int 0x94
db 0x8A, 0x07                           ; 0E4E mov al,[bx]
db 0x3C, 0xCF                           ; 0E50 cmp al,0xcf
db 0xB0, 0x01                           ; 0E52 mov al,0x1
db 0x75, 0x0E                           ; 0E54 jnz 0xe64
db 0xE8, 0xCF, 0x08                     ; 0E56 call 0x1728
db 0x53                                 ; 0E59 push bx
db 0xE8, 0xF4, 0x5C                     ; 0E5A call 0x6b51
db 0xE8, 0x93, 0x55                     ; 0E5D call 0x63f3
db 0xE8, 0x12, 0x6D                     ; 0E60 call 0x7b75
db 0x5B                                 ; 0E63 pop bx
db 0x51                                 ; 0E64 push cx
db 0x52                                 ; 0E65 push dx
db 0x8A, 0xC8                           ; 0E66 mov cl,al
db 0xE8, 0xBA, 0x0C                     ; 0E68 call 0x1b25
db 0x8A, 0xE8                           ; 0E6B mov ch,al
db 0x51                                 ; 0E6D push cx
db 0x4B                                 ; 0E6E dec bx
db 0xE8, 0xAB, 0x00                     ; 0E6F call 0xf1d
db 0x74, 0x03                           ; 0E72 jz 0xe77
db 0xE9, 0x47, 0xF9                     ; 0E74 jmp 0x7be
db 0xE8, 0x81, 0x16                     ; 0E77 call 0x24fb
db 0xE8, 0xA0, 0x00                     ; 0E7A call 0xf1d
db 0x53                                 ; 0E7D push bx
db 0x53                                 ; 0E7E push bx
db 0x8B, 0x1E, 0x5A, 0x04               ; 0E7F mov bx,[0x45a]
db 0x89, 0x1E, 0x2E, 0x00               ; 0E83 mov [0x2e],bx
db 0x8B, 0x1E, 0x3B, 0x03               ; 0E87 mov bx,[0x33b]
db 0x5E                                 ; 0E8B pop si
db 0x87, 0xDE                           ; 0E8C xchg bx,si
db 0x56                                 ; 0E8E push si
db 0xB5, 0x82                           ; 0E8F mov ch,0x82
db 0x51                                 ; 0E91 push cx
db 0x9F                                 ; 0E92 lahf
db 0x86, 0xC4                           ; 0E93 xchg al,ah
db 0x50                                 ; 0E95 push ax
db 0x86, 0xC4                           ; 0E96 xchg al,ah
db 0x9F                                 ; 0E98 lahf
db 0x86, 0xC4                           ; 0E99 xchg al,ah
db 0x50                                 ; 0E9B push ax
db 0x86, 0xC4                           ; 0E9C xchg al,ah
db 0xE9, 0xCF, 0x64                     ; 0E9E jmp 0x7370
db 0xB5, 0x82                           ; 0EA1 mov ch,0x82
db 0x51                                 ; 0EA3 push cx
db 0xEB, 0x42                           ; 0EA4 jmp short 0xee8
db 0xE9, 0xCB, 0xF8                     ; 0EA6 jmp 0x774
db 0xE9, 0x12, 0xF9                     ; 0EA9 jmp 0x7be
db 0xC3                                 ; 0EAC ret
db 0xE8, 0x75, 0x00                     ; 0EAD call 0xf25
db 0xEB, 0x50                           ; 0EB0 jmp short 0xf02
db 0xCD, 0x95                           ; 0EB2 int 0x95
db 0xE9, 0x63, 0x0F                     ; 0EB4 jmp 0x1e1a
db 0xE9, 0xD5, 0x02                     ; 0EB7 jmp 0x118f
db 0x0A, 0xC0                           ; 0EBA or al,al
db 0x75, 0xEB                           ; 0EBC jnz 0xea9
db 0x43                                 ; 0EBE inc bx
db 0x8A, 0x07                           ; 0EBF mov al,[bx]
db 0x43                                 ; 0EC1 inc bx
db 0x0A, 0x07                           ; 0EC2 or al,[bx]
db 0x74, 0xE0                           ; 0EC4 jz 0xea6
db 0x43                                 ; 0EC6 inc bx
db 0x8B, 0x17                           ; 0EC7 mov dx,[bx]
db 0x43                                 ; 0EC9 inc bx
db 0x89, 0x16, 0x2E, 0x00               ; 0ECA mov [0x2e],dx
db 0xF6, 0x06, 0x76, 0x04, 0xFF         ; 0ECE test byte [0x476],0xff
db 0x74, 0x26                           ; 0ED3 jz 0xefb
db 0x53                                 ; 0ED5 push bx
db 0xB0, 0x5B                           ; 0ED6 mov al,0x5b
db 0xE8, 0xCA, 0x1C                     ; 0ED8 call 0x2ba5
db 0x87, 0xDA                           ; 0EDB xchg bx,dx
db 0xE8, 0x70, 0x56                     ; 0EDD call 0x6550
db 0xB0, 0x5D                           ; 0EE0 mov al,0x5d
db 0xE8, 0xC0, 0x1C                     ; 0EE2 call 0x2ba5
db 0x5B                                 ; 0EE5 pop bx
db 0xEB, 0x13                           ; 0EE6 jmp short 0xefb
db 0xCD, 0x96                           ; 0EE8 int 0x96
db 0xE8, 0x9B, 0x1D                     ; 0EEA call 0x2c88
db 0x89, 0x26, 0x45, 0x03               ; 0EED mov [0x345],sp
db 0x89, 0x1E, 0x43, 0x03               ; 0EF1 mov [0x343],bx
db 0x8A, 0x07                           ; 0EF5 mov al,[bx]
db 0x3C, 0x3A                           ; 0EF7 cmp al,0x3a
db 0x75, 0xBF                           ; 0EF9 jnz 0xeba
db 0x43                                 ; 0EFB inc bx
db 0x8A, 0x07                           ; 0EFC mov al,[bx]
db 0x3C, 0x3A                           ; 0EFE cmp al,0x3a
db 0x72, 0xAB                           ; 0F00 jc 0xead
db 0xBA, 0xE8, 0x0E                     ; 0F02 mov dx,0xee8
db 0x52                                 ; 0F05 push dx
db 0x74, 0xA4                           ; 0F06 jz 0xeac
db 0x2C, 0x81                           ; 0F08 sub al,0x81
db 0x72, 0xAB                           ; 0F0A jc 0xeb7
db 0x3C, 0x4A                           ; 0F0C cmp al,0x4a
db 0x73, 0xA2                           ; 0F0E jnc 0xeb2
db 0x32, 0xE4                           ; 0F10 xor ah,ah
db 0x02, 0xC0                           ; 0F12 add al,al
db 0x8B, 0xF0                           ; 0F14 mov si,ax
db 0xCD, 0x97                           ; 0F16 int 0x97
db 0x2E, 0xFF, 0xB4, 0x25, 0x00         ; 0F18 push word [cs:si+0x25]
db 0x43                                 ; 0F1D inc bx
db 0x8A, 0x07                           ; 0F1E mov al,[bx]
db 0x3C, 0x3A                           ; 0F20 cmp al,0x3a
db 0x72, 0x01                           ; 0F22 jc 0xf25
db 0xC3                                 ; 0F24 ret
db 0x3C, 0x20                           ; 0F25 cmp al,0x20
db 0x74, 0xF4                           ; 0F27 jz 0xf1d
db 0x72, 0x08                           ; 0F29 jc 0xf33
db 0x3C, 0x30                           ; 0F2B cmp al,0x30
db 0xF5                                 ; 0F2D cmc
db 0xFE, 0xC0                           ; 0F2E inc al
db 0xFE, 0xC8                           ; 0F30 dec al
db 0xC3                                 ; 0F32 ret
db 0x0A, 0xC0                           ; 0F33 or al,al
db 0x74, 0xFB                           ; 0F35 jz 0xf32
db 0x3C, 0x0B                           ; 0F37 cmp al,0xb
db 0x72, 0x72                           ; 0F39 jc 0xfad
db 0x3C, 0x1E                           ; 0F3B cmp al,0x1e
db 0x75, 0x06                           ; 0F3D jnz 0xf45
db 0xA0, 0x00, 0x03                     ; 0F3F mov al,[0x300]
db 0x0A, 0xC0                           ; 0F42 or al,al
db 0xC3                                 ; 0F44 ret
db 0x3C, 0x10                           ; 0F45 cmp al,0x10
db 0x74, 0x3C                           ; 0F47 jz 0xf85
db 0x50                                 ; 0F49 push ax
db 0x43                                 ; 0F4A inc bx
db 0xA2, 0x00, 0x03                     ; 0F4B mov [0x300],al
db 0x2C, 0x1C                           ; 0F4E sub al,0x1c
db 0x73, 0x39                           ; 0F50 jnc 0xf8b
db 0x2C, 0xF5                           ; 0F52 sub al,0xf5
db 0x73, 0x07                           ; 0F54 jnc 0xf5d
db 0x3C, 0xFE                           ; 0F56 cmp al,0xfe
db 0x75, 0x1B                           ; 0F58 jnz 0xf75
db 0x8A, 0x07                           ; 0F5A mov al,[bx]
db 0x43                                 ; 0F5C inc bx
db 0x89, 0x1E, 0xFE, 0x02               ; 0F5D mov [0x2fe],bx
db 0xB7, 0x00                           ; 0F61 mov bh,0x0
db 0x8A, 0xD8                           ; 0F63 mov bl,al
db 0x89, 0x1E, 0x02, 0x03               ; 0F65 mov [0x302],bx
db 0xB0, 0x02                           ; 0F69 mov al,0x2
db 0xA2, 0x01, 0x03                     ; 0F6B mov [0x301],al
db 0xBB, 0x04, 0x00                     ; 0F6E mov bx,0x4
db 0x58                                 ; 0F71 pop ax
db 0x0A, 0xC0                           ; 0F72 or al,al
db 0xC3                                 ; 0F74 ret
db 0x8A, 0x07                           ; 0F75 mov al,[bx]
db 0x43                                 ; 0F77 inc bx
db 0x43                                 ; 0F78 inc bx
db 0x89, 0x1E, 0xFE, 0x02               ; 0F79 mov [0x2fe],bx
db 0x4B                                 ; 0F7D dec bx
db 0x8A, 0x3F                           ; 0F7E mov bh,[bx]
db 0xEB, 0xE1                           ; 0F80 jmp short 0xf63
db 0xE8, 0x37, 0x00                     ; 0F82 call 0xfbc
db 0x8B, 0x1E, 0xFE, 0x02               ; 0F85 mov bx,[0x2fe]
db 0xEB, 0x93                           ; 0F89 jmp short 0xf1e
db 0xFE, 0xC0                           ; 0F8B inc al
db 0xD0, 0xC0                           ; 0F8D rol al,1
db 0xA2, 0x01, 0x03                     ; 0F8F mov [0x301],al
db 0x52                                 ; 0F92 push dx
db 0x51                                 ; 0F93 push cx
db 0xBA, 0x02, 0x03                     ; 0F94 mov dx,0x302
db 0x87, 0xDA                           ; 0F97 xchg bx,dx
db 0x8A, 0xE8                           ; 0F99 mov ch,al
db 0xE8, 0x13, 0x55                     ; 0F9B call 0x64b1
db 0x87, 0xDA                           ; 0F9E xchg bx,dx
db 0x59                                 ; 0FA0 pop cx
db 0x5A                                 ; 0FA1 pop dx
db 0x89, 0x1E, 0xFE, 0x02               ; 0FA2 mov [0x2fe],bx
db 0x58                                 ; 0FA6 pop ax
db 0xBB, 0x04, 0x00                     ; 0FA7 mov bx,0x4
db 0x0A, 0xC0                           ; 0FAA or al,al
db 0xC3                                 ; 0FAC ret
db 0x3C, 0x09                           ; 0FAD cmp al,0x9
db 0x72, 0x03                           ; 0FAF jc 0xfb4
db 0xE9, 0x69, 0xFF                     ; 0FB1 jmp 0xf1d
db 0x3C, 0x30                           ; 0FB4 cmp al,0x30
db 0xF5                                 ; 0FB6 cmc
db 0xFE, 0xC0                           ; 0FB7 inc al
db 0xFE, 0xC8                           ; 0FB9 dec al
db 0xC3                                 ; 0FBB ret
db 0xA0, 0x00, 0x03                     ; 0FBC mov al,[0x300]
db 0x3C, 0x0F                           ; 0FBF cmp al,0xf
db 0x73, 0x17                           ; 0FC1 jnc 0xfda
db 0x3C, 0x0D                           ; 0FC3 cmp al,0xd
db 0x72, 0x13                           ; 0FC5 jc 0xfda
db 0x8B, 0x1E, 0x02, 0x03               ; 0FC7 mov bx,[0x302]
db 0x75, 0x0A                           ; 0FCB jnz 0xfd7
db 0x43                                 ; 0FCD inc bx
db 0x43                                 ; 0FCE inc bx
db 0x43                                 ; 0FCF inc bx
db 0x8A, 0x17                           ; 0FD0 mov dl,[bx]
db 0x43                                 ; 0FD2 inc bx
db 0x8A, 0x37                           ; 0FD3 mov dh,[bx]
db 0x87, 0xDA                           ; 0FD5 xchg bx,dx
db 0xE9, 0x6A, 0x54                     ; 0FD7 jmp 0x6444
db 0xA0, 0x01, 0x03                     ; 0FDA mov al,[0x301]
db 0xA2, 0xFB, 0x02                     ; 0FDD mov [0x2fb],al
db 0x3C, 0x08                           ; 0FE0 cmp al,0x8
db 0x74, 0x11                           ; 0FE2 jz 0xff5
db 0x8B, 0x1E, 0x02, 0x03               ; 0FE4 mov bx,[0x302]
db 0x89, 0x1E, 0xA3, 0x04               ; 0FE8 mov [0x4a3],bx
db 0x8B, 0x1E, 0x04, 0x03               ; 0FEC mov bx,[0x304]
db 0x89, 0x1E, 0xA5, 0x04               ; 0FF0 mov [0x4a5],bx
db 0xC3                                 ; 0FF4 ret
db 0xBB, 0x02, 0x03                     ; 0FF5 mov bx,0x302
db 0xE9, 0x9D, 0x54                     ; 0FF8 jmp 0x6498
db 0xB2, 0x03                           ; 0FFB mov dl,0x3
db 0xB9, 0xB2, 0x02                     ; 0FFD mov cx,0x2b2
db 0xB9, 0xB2, 0x04                     ; 1000 mov cx,0x4b2
db 0xB9, 0xB2, 0x08                     ; 1003 mov cx,0x8b2
db 0xE8, 0x3A, 0x1F                     ; 1006 call 0x2f43
db 0xB9, 0xBE, 0x07                     ; 1009 mov cx,0x7be
db 0x51                                 ; 100C push cx
db 0x72, 0xE5                           ; 100D jc 0xff4
db 0x2C, 0x41                           ; 100F sub al,0x41
db 0x8A, 0xC8                           ; 1011 mov cl,al
db 0x8A, 0xE8                           ; 1013 mov ch,al
db 0xE8, 0x05, 0xFF                     ; 1015 call 0xf1d
db 0x3C, 0xEA                           ; 1018 cmp al,0xea
db 0x75, 0x0F                           ; 101A jnz 0x102b
db 0xE8, 0xFE, 0xFE                     ; 101C call 0xf1d
db 0xE8, 0x21, 0x1F                     ; 101F call 0x2f43
db 0x72, 0xD0                           ; 1022 jc 0xff4
db 0x2C, 0x41                           ; 1024 sub al,0x41
db 0x8A, 0xE8                           ; 1026 mov ch,al
db 0xE8, 0xF2, 0xFE                     ; 1028 call 0xf1d
db 0x8A, 0xC5                           ; 102B mov al,ch
db 0x2A, 0xC1                           ; 102D sub al,cl
db 0x72, 0xC3                           ; 102F jc 0xff4
db 0xFE, 0xC0                           ; 1031 inc al
db 0x5E                                 ; 1033 pop si
db 0x87, 0xDE                           ; 1034 xchg bx,si
db 0x56                                 ; 1036 push si
db 0xBB, 0x60, 0x03                     ; 1037 mov bx,0x360
db 0xB5, 0x00                           ; 103A mov ch,0x0
db 0x03, 0xD9                           ; 103C add bx,cx
db 0x88, 0x17                           ; 103E mov [bx],dl
db 0x43                                 ; 1040 inc bx
db 0xFE, 0xC8                           ; 1041 dec al
db 0x75, 0xF9                           ; 1043 jnz 0x103e
db 0x5B                                 ; 1045 pop bx
db 0x8A, 0x07                           ; 1046 mov al,[bx]
db 0x3C, 0x2C                           ; 1048 cmp al,0x2c
db 0x75, 0xA8                           ; 104A jnz 0xff4
db 0xE8, 0xCE, 0xFE                     ; 104C call 0xf1d
db 0xEB, 0xB5                           ; 104F jmp short 0x1006
db 0xE8, 0xC9, 0xFE                     ; 1051 call 0xf1d
db 0xE8, 0xB3, 0x0E                     ; 1054 call 0x1f0a
db 0x79, 0x9B                           ; 1057 jns 0xff4
db 0xB2, 0x05                           ; 1059 mov dl,0x5
db 0xE9, 0x7A, 0xF7                     ; 105B jmp 0x7d8
db 0x8A, 0x07                           ; 105E mov al,[bx]
db 0x3C, 0x2E                           ; 1060 cmp al,0x2e
db 0x8B, 0x16, 0x49, 0x03               ; 1062 mov dx,[0x349]
db 0x75, 0x03                           ; 1066 jnz 0x106b
db 0xE9, 0xB2, 0xFE                     ; 1068 jmp 0xf1d
db 0x4B                                 ; 106B dec bx
db 0xE8, 0xAE, 0xFE                     ; 106C call 0xf1d
db 0x3C, 0x0E                           ; 106F cmp al,0xe
db 0x74, 0x02                           ; 1071 jz 0x1075
db 0x3C, 0x0D                           ; 1073 cmp al,0xd
db 0x8B, 0x16, 0x02, 0x03               ; 1075 mov dx,[0x302]
db 0x75, 0x03                           ; 1079 jnz 0x107e
db 0xE9, 0x9F, 0xFE                     ; 107B jmp 0xf1d
db 0x32, 0xC0                           ; 107E xor al,al
db 0xA2, 0x00, 0x03                     ; 1080 mov [0x300],al
db 0x4B                                 ; 1083 dec bx
db 0xBA, 0x00, 0x00                     ; 1084 mov dx,0x0
db 0xE8, 0x93, 0xFE                     ; 1087 call 0xf1d
db 0x72, 0x01                           ; 108A jc 0x108d
db 0xC3                                 ; 108C ret
db 0x53                                 ; 108D push bx
db 0x9F                                 ; 108E lahf
db 0x50                                 ; 108F push ax
db 0xBB, 0x98, 0x19                     ; 1090 mov bx,0x1998
db 0x3B, 0xDA                           ; 1093 cmp bx,dx
db 0x72, 0x1B                           ; 1095 jc 0x10b2
db 0x8A, 0xFE                           ; 1097 mov bh,dh
db 0x8A, 0xDA                           ; 1099 mov bl,dl
db 0x03, 0xDA                           ; 109B add bx,dx
db 0x03, 0xDB                           ; 109D add bx,bx
db 0x03, 0xDA                           ; 109F add bx,dx
db 0x03, 0xDB                           ; 10A1 add bx,bx
db 0x58                                 ; 10A3 pop ax
db 0x9E                                 ; 10A4 sahf
db 0x2C, 0x30                           ; 10A5 sub al,0x30
db 0x8A, 0xD0                           ; 10A7 mov dl,al
db 0xB6, 0x00                           ; 10A9 mov dh,0x0
db 0x03, 0xDA                           ; 10AB add bx,dx
db 0x87, 0xDA                           ; 10AD xchg bx,dx
db 0x5B                                 ; 10AF pop bx
db 0xEB, 0xD5                           ; 10B0 jmp short 0x1087
db 0x58                                 ; 10B2 pop ax
db 0x9E                                 ; 10B3 sahf
db 0x5B                                 ; 10B4 pop bx
db 0xC3                                 ; 10B5 ret
db 0x75, 0x03                           ; 10B6 jnz 0x10bb
db 0xE9, 0x7C, 0x1C                     ; 10B8 jmp 0x2d37
db 0x3C, 0x0E                           ; 10BB cmp al,0xe
db 0x74, 0x07                           ; 10BD jz 0x10c6
db 0x3C, 0x0D                           ; 10BF cmp al,0xd
db 0x74, 0x03                           ; 10C1 jz 0x10c6
db 0xE9, 0xA3, 0x30                     ; 10C3 jmp 0x4169
db 0xE8, 0x75, 0x1C                     ; 10C6 call 0x2d3e
db 0xB9, 0xE8, 0x0E                     ; 10C9 mov cx,0xee8
db 0xEB, 0x1E                           ; 10CC jmp short 0x10ec
db 0xB1, 0x03                           ; 10CE mov cl,0x3
db 0xE8, 0x02, 0x1C                     ; 10D0 call 0x2cd5
db 0xE8, 0x95, 0xFF                     ; 10D3 call 0x106b
db 0x59                                 ; 10D6 pop cx
db 0x53                                 ; 10D7 push bx
db 0x53                                 ; 10D8 push bx
db 0x8B, 0x1E, 0x2E, 0x00               ; 10D9 mov bx,[0x2e]
db 0x5E                                 ; 10DD pop si
db 0x87, 0xDE                           ; 10DE xchg bx,si
db 0x56                                 ; 10E0 push si
db 0xB0, 0x8D                           ; 10E1 mov al,0x8d
db 0x9F                                 ; 10E3 lahf
db 0x86, 0xC4                           ; 10E4 xchg al,ah
db 0x50                                 ; 10E6 push ax
db 0x86, 0xC4                           ; 10E7 xchg al,ah
db 0x51                                 ; 10E9 push cx
db 0xEB, 0x04                           ; 10EA jmp short 0x10f0
db 0x51                                 ; 10EC push cx
db 0xE8, 0x7B, 0xFF                     ; 10ED call 0x106b
db 0xA0, 0x00, 0x03                     ; 10F0 mov al,[0x300]
db 0x3C, 0x0D                           ; 10F3 cmp al,0xd
db 0x87, 0xDA                           ; 10F5 xchg bx,dx
db 0x74, 0xBC                           ; 10F7 jz 0x10b5
db 0x3C, 0x0E                           ; 10F9 cmp al,0xe
db 0x74, 0x03                           ; 10FB jz 0x1100
db 0xE9, 0xBE, 0xF6                     ; 10FD jmp 0x7be
db 0x87, 0xDA                           ; 1100 xchg bx,dx
db 0x53                                 ; 1102 push bx
db 0x8B, 0x1E, 0xFE, 0x02               ; 1103 mov bx,[0x2fe]
db 0x5E                                 ; 1107 pop si
db 0x87, 0xDE                           ; 1108 xchg bx,si
db 0x56                                 ; 110A push si
db 0xE8, 0x51, 0x00                     ; 110B call 0x115f
db 0x43                                 ; 110E inc bx
db 0x53                                 ; 110F push bx
db 0x8B, 0x1E, 0x2E, 0x00               ; 1110 mov bx,[0x2e]
db 0x3B, 0xDA                           ; 1114 cmp bx,dx
db 0x5B                                 ; 1116 pop bx
db 0x73, 0x03                           ; 1117 jnc 0x111c
db 0xE8, 0x4F, 0xF9                     ; 1119 call 0xa6b
db 0x72, 0x03                           ; 111C jc 0x1121
db 0xE8, 0x46, 0xF9                     ; 111E call 0xa67
db 0x73, 0x0D                           ; 1121 jnc 0x1130
db 0x49                                 ; 1123 dec cx
db 0xB0, 0x0D                           ; 1124 mov al,0xd
db 0xA2, 0x3D, 0x03                     ; 1126 mov [0x33d],al
db 0x5B                                 ; 1129 pop bx
db 0xE8, 0xFC, 0x12                     ; 112A call 0x2429
db 0x8B, 0xD9                           ; 112D mov bx,cx
db 0xC3                                 ; 112F ret
db 0xB2, 0x08                           ; 1130 mov dl,0x8
db 0xE9, 0xA3, 0xF6                     ; 1132 jmp 0x7d8
db 0xCD, 0x98                           ; 1135 int 0x98
db 0x75, 0xF6                           ; 1137 jnz 0x112f
db 0xB6, 0xFF                           ; 1139 mov dh,0xff
db 0xE8, 0xFA, 0xF5                     ; 113B call 0x738
db 0x8B, 0xE3                           ; 113E mov sp,bx
db 0x89, 0x1E, 0x45, 0x03               ; 1140 mov [0x345],bx
db 0x3C, 0x8D                           ; 1144 cmp al,0x8d
db 0xB2, 0x03                           ; 1146 mov dl,0x3
db 0x74, 0x03                           ; 1148 jz 0x114d
db 0xE9, 0x8B, 0xF6                     ; 114A jmp 0x7d8
db 0x5B                                 ; 114D pop bx
db 0x89, 0x1E, 0x2E, 0x00               ; 114E mov [0x2e],bx
db 0xBB, 0xE8, 0x0E                     ; 1152 mov bx,0xee8
db 0x5E                                 ; 1155 pop si
db 0x87, 0xDE                           ; 1156 xchg bx,si
db 0x56                                 ; 1158 push si
db 0xB0, 0x5B                           ; 1159 mov al,0x5b
db 0xB1, 0x3A                           ; 115B mov cl,0x3a
db 0xEB, 0x02                           ; 115D jmp short 0x1161
db 0xB1, 0x00                           ; 115F mov cl,0x0
db 0xB5, 0x00                           ; 1161 mov ch,0x0
db 0x8A, 0xC1                           ; 1163 mov al,cl
db 0x8A, 0xCD                           ; 1165 mov cl,ch
db 0x8A, 0xE8                           ; 1167 mov ch,al
db 0x4B                                 ; 1169 dec bx
db 0xE8, 0xB0, 0xFD                     ; 116A call 0xf1d
db 0x0A, 0xC0                           ; 116D or al,al
db 0x74, 0xBE                           ; 116F jz 0x112f
db 0x3A, 0xC5                           ; 1171 cmp al,ch
db 0x74, 0xBA                           ; 1173 jz 0x112f
db 0x43                                 ; 1175 inc bx
db 0x3C, 0x22                           ; 1176 cmp al,0x22
db 0x74, 0xE9                           ; 1178 jz 0x1163
db 0xFE, 0xC0                           ; 117A inc al
db 0x74, 0xEC                           ; 117C jz 0x116a
db 0x2C, 0x8C                           ; 117E sub al,0x8c
db 0x75, 0xE7                           ; 1180 jnz 0x1169
db 0x3A, 0xC5                           ; 1182 cmp al,ch
db 0x12, 0xC6                           ; 1184 adc al,dh
db 0x8A, 0xF0                           ; 1186 mov dh,al
db 0xEB, 0xDF                           ; 1188 jmp short 0x1169
db 0x58                                 ; 118A pop ax
db 0x04, 0x03                           ; 118B add al,0x3
db 0xEB, 0x14                           ; 118D jmp short 0x11a3
db 0xE8, 0xDC, 0x25                     ; 118F call 0x376e
db 0xE8, 0x5F, 0x1C                     ; 1192 call 0x2df4
db 0xE7, 0x89                           ; 1195 out 0x89,ax
db 0x16                                 ; 1197 push ss
db 0x3B, 0x03                           ; 1198 cmp ax,[bp+di]
db 0x52                                 ; 119A push dx
db 0xA0, 0xFB, 0x02                     ; 119B mov al,[0x2fb]
db 0x50                                 ; 119E push ax
db 0xE8, 0x85, 0x05                     ; 119F call 0x1727
db 0x58                                 ; 11A2 pop ax
db 0x5E                                 ; 11A3 pop si
db 0x87, 0xDE                           ; 11A4 xchg bx,si
db 0x56                                 ; 11A6 push si
db 0x8A, 0xE8                           ; 11A7 mov ch,al
db 0xA0, 0xFB, 0x02                     ; 11A9 mov al,[0x2fb]
db 0x3A, 0xC5                           ; 11AC cmp al,ch
db 0x8A, 0xC5                           ; 11AE mov al,ch
db 0x74, 0x06                           ; 11B0 jz 0x11b8
db 0xE8, 0x25, 0x0C                     ; 11B2 call 0x1dda
db 0xA0, 0xFB, 0x02                     ; 11B5 mov al,[0x2fb]
db 0xBA, 0xA3, 0x04                     ; 11B8 mov dx,0x4a3
db 0x3C, 0x05                           ; 11BB cmp al,0x5
db 0x72, 0x03                           ; 11BD jc 0x11c2
db 0xBA, 0x9F, 0x04                     ; 11BF mov dx,0x49f
db 0x53                                 ; 11C2 push bx
db 0x3C, 0x03                           ; 11C3 cmp al,0x3
db 0x75, 0x31                           ; 11C5 jnz 0x11f8
db 0x8B, 0x1E, 0xA3, 0x04               ; 11C7 mov bx,[0x4a3]
db 0x53                                 ; 11CB push bx
db 0x43                                 ; 11CC inc bx
db 0x8B, 0x17                           ; 11CD mov dx,[bx]
db 0x8B, 0x1E, 0x30, 0x00               ; 11CF mov bx,[0x30]
db 0x3B, 0xDA                           ; 11D3 cmp bx,dx
db 0x73, 0x11                           ; 11D5 jnc 0x11e8
db 0x8B, 0x1E, 0x5C, 0x03               ; 11D7 mov bx,[0x35c]
db 0x3B, 0xDA                           ; 11DB cmp bx,dx
db 0x5A                                 ; 11DD pop dx
db 0x73, 0x11                           ; 11DE jnc 0x11f1
db 0xBB, 0x2C, 0x03                     ; 11E0 mov bx,0x32c
db 0x3B, 0xDA                           ; 11E3 cmp bx,dx
db 0x73, 0x0A                           ; 11E5 jnc 0x11f1
db 0xB0, 0x5A                           ; 11E7 mov al,0x5a
db 0xE8, 0xE6, 0x16                     ; 11E9 call 0x28d2
db 0x87, 0xDA                           ; 11EC xchg bx,dx
db 0xE8, 0x33, 0x14                     ; 11EE call 0x2624
db 0xE8, 0xDE, 0x16                     ; 11F1 call 0x28d2
db 0x5E                                 ; 11F4 pop si
db 0x87, 0xDE                           ; 11F5 xchg bx,si
db 0x56                                 ; 11F7 push si
db 0xE8, 0xBE, 0x52                     ; 11F8 call 0x64b9
db 0x5A                                 ; 11FB pop dx
db 0x5B                                 ; 11FC pop bx
db 0xC3                                 ; 11FD ret
db 0x3C, 0xA7                           ; 11FE cmp al,0xa7
db 0x75, 0x32                           ; 1200 jnz 0x1234
db 0xE8, 0x18, 0xFD                     ; 1202 call 0xf1d
db 0xE8, 0xEC, 0x1B                     ; 1205 call 0x2df4
db 0x89, 0xE8                           ; 1208 mov ax,bp
db 0x5F                                 ; 120A pop di
db 0xFE, 0x0B                           ; 120B dec byte [bp+di]
db 0xD2                                 ; 120D db 0xD2
db 0x74, 0x0D                           ; 120E jz 0x121d
db 0xE8, 0x4F, 0xF8                     ; 1210 call 0xa62
db 0x8A, 0xF5                           ; 1213 mov dh,ch
db 0x8A, 0xD1                           ; 1215 mov dl,cl
db 0x5B                                 ; 1217 pop bx
db 0x72, 0x03                           ; 1218 jc 0x121d
db 0xE9, 0x13, 0xFF                     ; 121A jmp 0x1130
db 0x89, 0x16, 0x4D, 0x03               ; 121D mov [0x34d],dx
db 0x72, 0xDA                           ; 1221 jc 0x11fd
db 0xA0, 0x4F, 0x03                     ; 1223 mov al,[0x34f]
db 0x0A, 0xC0                           ; 1226 or al,al
db 0x8A, 0xC2                           ; 1228 mov al,dl
db 0x74, 0xD1                           ; 122A jz 0x11fd
db 0xA0, 0x28, 0x00                     ; 122C mov al,[0x28]
db 0x8A, 0xD0                           ; 122F mov dl,al
db 0xE9, 0xCE, 0xF5                     ; 1231 jmp 0x802
db 0xE8, 0xE5, 0x0C                     ; 1234 call 0x1f1c
db 0x8A, 0x07                           ; 1237 mov al,[bx]
db 0x8A, 0xE8                           ; 1239 mov ch,al
db 0x3C, 0x8D                           ; 123B cmp al,0x8d
db 0x74, 0x05                           ; 123D jz 0x1244
db 0xE8, 0xB2, 0x1B                     ; 123F call 0x2df4
db 0x89, 0x4B, 0x8A                     ; 1242 mov [bp+di-0x76],cx
db 0xCA, 0xFE, 0xC9                     ; 1245 retf 0xc9fe
db 0x8A, 0xC5                           ; 1248 mov al,ch
db 0x75, 0x03                           ; 124A jnz 0x124f
db 0xE9, 0xB9, 0xFC                     ; 124C jmp 0xf08
db 0xE8, 0x1A, 0xFE                     ; 124F call 0x106c
db 0x3C, 0x2C                           ; 1252 cmp al,0x2c
db 0x75, 0xA7                           ; 1254 jnz 0x11fd
db 0xEB, 0xEE                           ; 1256 jmp short 0x1246
db 0xA0, 0x4F, 0x03                     ; 1258 mov al,[0x34f]
db 0x0A, 0xC0                           ; 125B or al,al
db 0x75, 0x08                           ; 125D jnz 0x1267
db 0x33, 0xC0                           ; 125F xor ax,ax
db 0xA3, 0x4D, 0x03                     ; 1261 mov [0x34d],ax
db 0xE9, 0x66, 0xF5                     ; 1264 jmp 0x7cd
db 0xFE, 0xC0                           ; 1267 inc al
db 0xA2, 0x28, 0x00                     ; 1269 mov [0x28],al
db 0x80, 0x3F, 0x83                     ; 126C cmp byte [bx],0x83
db 0x74, 0x12                           ; 126F jz 0x1283
db 0xE8, 0xF7, 0xFD                     ; 1271 call 0x106b
db 0x75, 0x0C                           ; 1274 jnz 0x1282
db 0x0B, 0xD2                           ; 1276 or dx,dx
db 0x74, 0x10                           ; 1278 jz 0x128a
db 0xE8, 0x73, 0xFE                     ; 127A call 0x10f0
db 0x32, 0xC0                           ; 127D xor al,al
db 0xA2, 0x4F, 0x03                     ; 127F mov [0x34f],al
db 0xC3                                 ; 1282 ret
db 0xE8, 0x97, 0xFC                     ; 1283 call 0xf1d
db 0x75, 0xFA                           ; 1286 jnz 0x1282
db 0xEB, 0x07                           ; 1288 jmp short 0x1291
db 0x32, 0xC0                           ; 128A xor al,al
db 0xA2, 0x4F, 0x03                     ; 128C mov [0x34f],al
db 0xFE, 0xC0                           ; 128F inc al
db 0xA1, 0x47, 0x03                     ; 1291 mov ax,[0x347]
db 0xA3, 0x2E, 0x00                     ; 1294 mov [0x2e],ax
db 0x8B, 0x1E, 0x4B, 0x03               ; 1297 mov bx,[0x34b]
db 0x75, 0xE5                           ; 129B jnz 0x1282
db 0x80, 0x3F, 0x00                     ; 129D cmp byte [bx],0x0
db 0x75, 0x03                           ; 12A0 jnz 0x12a5
db 0x83, 0xC3, 0x04                     ; 12A2 add bx,byte +0x4
db 0x43                                 ; 12A5 inc bx
db 0xE9, 0x17, 0x19                     ; 12A6 jmp 0x2bc0
db 0xE8, 0x70, 0x0C                     ; 12A9 call 0x1f1c
db 0x75, 0xDA                           ; 12AC jnz 0x1288
db 0x0A, 0xC0                           ; 12AE or al,al
db 0x75, 0x03                           ; 12B0 jnz 0x12b5
db 0xE9, 0xA4, 0xFD                     ; 12B2 jmp 0x1059
db 0xE9, 0x20, 0xF5                     ; 12B5 jmp 0x7d8
db 0xBA, 0x0A, 0x00                     ; 12B8 mov dx,0xa
db 0x52                                 ; 12BB push dx
db 0x74, 0x1F                           ; 12BC jz 0x12dd
db 0xE8, 0x9D, 0xFD                     ; 12BE call 0x105e
db 0x87, 0xDA                           ; 12C1 xchg bx,dx
db 0x5E                                 ; 12C3 pop si
db 0x87, 0xDE                           ; 12C4 xchg bx,si
db 0x56                                 ; 12C6 push si
db 0x74, 0x16                           ; 12C7 jz 0x12df
db 0x87, 0xDA                           ; 12C9 xchg bx,dx
db 0xE8, 0x26, 0x1B                     ; 12CB call 0x2df4
db 0x2C, 0x8B                           ; 12CE sub al,0x8b
db 0x16                                 ; 12D0 push ss
db 0x41                                 ; 12D1 inc cx
db 0x03, 0x74, 0x08                     ; 12D2 add si,[si+0x8]
db 0xE8, 0x93, 0xFD                     ; 12D5 call 0x106b
db 0x74, 0x03                           ; 12D8 jz 0x12dd
db 0xE9, 0xE1, 0xF4                     ; 12DA jmp 0x7be
db 0x87, 0xDA                           ; 12DD xchg bx,dx
db 0x8A, 0xC7                           ; 12DF mov al,bh
db 0x0A, 0xC3                           ; 12E1 or al,bl
db 0x75, 0x03                           ; 12E3 jnz 0x12e8
db 0xE9, 0x71, 0xFD                     ; 12E5 jmp 0x1059
db 0x89, 0x1E, 0x41, 0x03               ; 12E8 mov [0x341],bx
db 0xA2, 0x3E, 0x03                     ; 12EC mov [0x33e],al
db 0x5B                                 ; 12EF pop bx
db 0x89, 0x1E, 0x3F, 0x03               ; 12F0 mov [0x33f],bx
db 0x59                                 ; 12F4 pop cx
db 0xE9, 0xDB, 0xF5                     ; 12F5 jmp 0x8d3
db 0xE8, 0x2C, 0x04                     ; 12F8 call 0x1727
db 0x8A, 0x07                           ; 12FB mov al,[bx]
db 0x3C, 0x2C                           ; 12FD cmp al,0x2c
db 0x75, 0x03                           ; 12FF jnz 0x1304
db 0xE8, 0x19, 0xFC                     ; 1301 call 0xf1d
db 0x3C, 0x89                           ; 1304 cmp al,0x89
db 0x74, 0x05                           ; 1306 jz 0x130d
db 0xE8, 0xE9, 0x1A                     ; 1308 call 0x2df4
db 0xCD, 0x4B                           ; 130B int 0x4b
db 0x53                                 ; 130D push bx
db 0xE8, 0xD2, 0x51                     ; 130E call 0x64e3
db 0x5B                                 ; 1311 pop bx
db 0x74, 0x19                           ; 1312 jz 0x132d
db 0xE8, 0x06, 0xFC                     ; 1314 call 0xf1d
db 0x75, 0x01                           ; 1317 jnz 0x131a
db 0xC3                                 ; 1319 ret
db 0x3C, 0x0E                           ; 131A cmp al,0xe
db 0x75, 0x03                           ; 131C jnz 0x1321
db 0xE9, 0xCC, 0xFD                     ; 131E jmp 0x10ed
db 0x3C, 0x0D                           ; 1321 cmp al,0xd
db 0x74, 0x03                           ; 1323 jz 0x1328
db 0xE9, 0xE0, 0xFB                     ; 1325 jmp 0xf08
db 0x8B, 0x1E, 0x02, 0x03               ; 1328 mov bx,[0x302]
db 0xC3                                 ; 132C ret
db 0xB6, 0x01                           ; 132D mov dh,0x1
db 0xE8, 0x29, 0xFE                     ; 132F call 0x115b
db 0x0A, 0xC0                           ; 1332 or al,al
db 0x74, 0xF6                           ; 1334 jz 0x132c
db 0xE8, 0xE4, 0xFB                     ; 1336 call 0xf1d
db 0x3C, 0xA1                           ; 1339 cmp al,0xa1
db 0x75, 0xF2                           ; 133B jnz 0x132f
db 0xFE, 0xCE                           ; 133D dec dh
db 0x75, 0xEE                           ; 133F jnz 0x132f
db 0xEB, 0xD1                           ; 1341 jmp short 0x1314
db 0xE8, 0x6A, 0x01                     ; 1343 call 0x14b0
db 0xEB, 0x03                           ; 1346 jmp short 0x134b
db 0xE8, 0x9B, 0x31                     ; 1348 call 0x44e6
db 0x4B                                 ; 134B dec bx
db 0xE8, 0xCE, 0xFB                     ; 134C call 0xf1d
db 0x75, 0x03                           ; 134F jnz 0x1354
db 0xE8, 0x1D, 0x19                     ; 1351 call 0x2c71
db 0x75, 0x03                           ; 1354 jnz 0x1359
db 0xE9, 0x3F, 0x01                     ; 1356 jmp 0x1498
db 0x3C, 0xD7                           ; 1359 cmp al,0xd7
db 0x75, 0x03                           ; 135B jnz 0x1360
db 0xE9, 0x6B, 0x27                     ; 135D jmp 0x3acb
db 0x3C, 0xCE                           ; 1360 cmp al,0xce
db 0x75, 0x03                           ; 1362 jnz 0x1367
db 0xE9, 0xAB, 0x00                     ; 1364 jmp 0x1412
db 0x3C, 0xD2                           ; 1367 cmp al,0xd2
db 0x75, 0x03                           ; 1369 jnz 0x136e
db 0xE9, 0xA4, 0x00                     ; 136B jmp 0x1412
db 0x53                                 ; 136E push bx
db 0x3C, 0x2C                           ; 136F cmp al,0x2c
db 0x74, 0x6D                           ; 1371 jz 0x13e0
db 0x3C, 0x3B                           ; 1373 cmp al,0x3b
db 0x75, 0x03                           ; 1375 jnz 0x137a
db 0xE9, 0x17, 0x01                     ; 1377 jmp 0x1491
db 0x59                                 ; 137A pop cx
db 0xE8, 0xA9, 0x03                     ; 137B call 0x1727
db 0x53                                 ; 137E push bx
db 0xE8, 0xA3, 0x07                     ; 137F call 0x1b25
db 0x74, 0x0F                           ; 1382 jz 0x1393
db 0xE8, 0x41, 0x5D                     ; 1384 call 0x70c8
db 0xE8, 0xC2, 0x12                     ; 1387 call 0x264c
db 0xC6, 0x07, 0x20                     ; 138A mov byte [bx],0x20
db 0x8B, 0x1E, 0xA3, 0x04               ; 138D mov bx,[0x4a3]
db 0xFE, 0x07                           ; 1391 inc byte [bx]
db 0xCD, 0x99                           ; 1393 int 0x99
db 0x8B, 0x1E, 0xA3, 0x04               ; 1395 mov bx,[0x4a3]
db 0x53                                 ; 1399 push bx
db 0xE8, 0x39, 0x1C                     ; 139A call 0x2fd6
db 0x74, 0x0D                           ; 139D jz 0x13ac
db 0xE8, 0x31, 0x01                     ; 139F call 0x14d3
db 0x78, 0x03                           ; 13A2 js 0x13a7
db 0xE9, 0x31, 0x00                     ; 13A4 jmp 0x13d8
db 0xE8, 0x81, 0x45                     ; 13A7 call 0x592b
db 0xEB, 0x03                           ; 13AA jmp short 0x13af
db 0xA0, 0x29, 0x00                     ; 13AC mov al,[0x29]
db 0x8A, 0xE8                           ; 13AF mov ch,al
db 0xFE, 0xC0                           ; 13B1 inc al
db 0x74, 0x23                           ; 13B3 jz 0x13d8
db 0xE8, 0x1E, 0x1C                     ; 13B5 call 0x2fd6
db 0x74, 0x07                           ; 13B8 jz 0x13c1
db 0xE8, 0x66, 0x45                     ; 13BA call 0x5923
db 0x8A, 0x07                           ; 13BD mov al,[bx]
db 0xEB, 0x03                           ; 13BF jmp short 0x13c4
db 0xE8, 0x68, 0x3B                     ; 13C1 call 0x4f2c
db 0x5B                                 ; 13C4 pop bx
db 0x53                                 ; 13C5 push bx
db 0x0A, 0xC0                           ; 13C6 or al,al
db 0x74, 0x0E                           ; 13C8 jz 0x13d8
db 0x02, 0x07                           ; 13CA add al,[bx]
db 0xF5                                 ; 13CC cmc
db 0x73, 0x04                           ; 13CD jnc 0x13d3
db 0xFE, 0xC8                           ; 13CF dec al
db 0x3A, 0xC5                           ; 13D1 cmp al,ch
db 0x72, 0x03                           ; 13D3 jc 0x13d8
db 0xE8, 0x99, 0x18                     ; 13D5 call 0x2c71
db 0x5B                                 ; 13D8 pop bx
db 0xE8, 0xDE, 0x12                     ; 13D9 call 0x26ba
db 0x5B                                 ; 13DC pop bx
db 0xE9, 0x6B, 0xFF                     ; 13DD jmp 0x134b
db 0xCD, 0x9A                           ; 13E0 int 0x9a
db 0xB9, 0x32, 0x00                     ; 13E2 mov cx,0x32
db 0x8B, 0x1E, 0xE9, 0x04               ; 13E5 mov bx,[0x4e9]
db 0x03, 0xD9                           ; 13E9 add bx,cx
db 0xE8, 0xE8, 0x1B                     ; 13EB call 0x2fd6
db 0x8A, 0x07                           ; 13EE mov al,[bx]
db 0x75, 0x18                           ; 13F0 jnz 0x140a
db 0xA0, 0x2A, 0x00                     ; 13F2 mov al,[0x2a]
db 0x8A, 0xE8                           ; 13F5 mov ch,al
db 0xE8, 0x32, 0x3B                     ; 13F7 call 0x4f2c
db 0x3C, 0xFF                           ; 13FA cmp al,0xff
db 0x74, 0x0C                           ; 13FC jz 0x140a
db 0x3A, 0xC5                           ; 13FE cmp al,ch
db 0x72, 0x03                           ; 1400 jc 0x1405
db 0xE8, 0x6C, 0x18                     ; 1402 call 0x2c71
db 0x72, 0x03                           ; 1405 jc 0x140a
db 0xE9, 0x87, 0x00                     ; 1407 jmp 0x1491
db 0x2C, 0x0E                           ; 140A sub al,0xe
db 0x73, 0xFC                           ; 140C jnc 0x140a
db 0xF6, 0xD0                           ; 140E not al
db 0xEB, 0x72                           ; 1410 jmp short 0x1484
db 0x50                                 ; 1412 push ax
db 0xE8, 0x07, 0xFB                     ; 1413 call 0xf1d
db 0xE8, 0xF1, 0x0A                     ; 1416 call 0x1f0a
db 0x58                                 ; 1419 pop ax
db 0x50                                 ; 141A push ax
db 0x3C, 0xD2                           ; 141B cmp al,0xd2
db 0x74, 0x01                           ; 141D jz 0x1420
db 0x4A                                 ; 141F dec dx
db 0x8A, 0xC6                           ; 1420 mov al,dh
db 0x0A, 0xC0                           ; 1422 or al,al
db 0x78, 0x03                           ; 1424 js 0x1429
db 0xE9, 0x03, 0x00                     ; 1426 jmp 0x142c
db 0xBA, 0x00, 0x00                     ; 1429 mov dx,0x0
db 0x53                                 ; 142C push bx
db 0xE8, 0xA6, 0x1B                     ; 142D call 0x2fd6
db 0x74, 0x0D                           ; 1430 jz 0x143f
db 0xE8, 0x9E, 0x00                     ; 1432 call 0x14d3
db 0x78, 0x03                           ; 1435 js 0x143a
db 0xE9, 0x15, 0x00                     ; 1437 jmp 0x144f
db 0xE8, 0xEE, 0x44                     ; 143A call 0x592b
db 0xEB, 0x03                           ; 143D jmp short 0x1442
db 0xA0, 0x29, 0x00                     ; 143F mov al,[0x29]
db 0x8A, 0xD8                           ; 1442 mov bl,al
db 0xFE, 0xC0                           ; 1444 inc al
db 0x74, 0x07                           ; 1446 jz 0x144f
db 0xB7, 0x00                           ; 1448 mov bh,0x0
db 0xE8, 0x7C, 0x51                     ; 144A call 0x65c9
db 0x87, 0xDA                           ; 144D xchg bx,dx
db 0x5B                                 ; 144F pop bx
db 0xE8, 0xA1, 0x19                     ; 1450 call 0x2df4
db 0x29, 0x4B, 0x58                     ; 1453 sub [bp+di+0x58],cx
db 0x2C, 0xD2                           ; 1456 sub al,0xd2
db 0x53                                 ; 1458 push bx
db 0x74, 0x13                           ; 1459 jz 0x146e
db 0xB9, 0x32, 0x00                     ; 145B mov cx,0x32
db 0x8B, 0x1E, 0xE9, 0x04               ; 145E mov bx,[0x4e9]
db 0x03, 0xD9                           ; 1462 add bx,cx
db 0xE8, 0x6F, 0x1B                     ; 1464 call 0x2fd6
db 0x8A, 0x07                           ; 1467 mov al,[bx]
db 0x75, 0x03                           ; 1469 jnz 0x146e
db 0xE8, 0xBE, 0x3A                     ; 146B call 0x4f2c
db 0xF6, 0xD0                           ; 146E not al
db 0x02, 0xC2                           ; 1470 add al,dl
db 0x72, 0x10                           ; 1472 jc 0x1484
db 0xFE, 0xC0                           ; 1474 inc al
db 0x74, 0x19                           ; 1476 jz 0x1491
db 0xE8, 0xF6, 0x17                     ; 1478 call 0x2c71
db 0x8A, 0xC2                           ; 147B mov al,dl
db 0xFE, 0xC8                           ; 147D dec al
db 0x79, 0x03                           ; 147F jns 0x1484
db 0xE9, 0x0D, 0x00                     ; 1481 jmp 0x1491
db 0xFE, 0xC0                           ; 1484 inc al
db 0x8A, 0xE8                           ; 1486 mov ch,al
db 0xB0, 0x20                           ; 1488 mov al,0x20
db 0xE8, 0x18, 0x17                     ; 148A call 0x2ba5
db 0xFE, 0xCD                           ; 148D dec ch
db 0x75, 0xF9                           ; 148F jnz 0x148a
db 0x5B                                 ; 1491 pop bx
db 0xE8, 0x88, 0xFA                     ; 1492 call 0xf1d
db 0xE9, 0xBC, 0xFE                     ; 1495 jmp 0x1354
db 0xCD, 0x9B                           ; 1498 int 0x9b
db 0x32, 0xC0                           ; 149A xor al,al
db 0x53                                 ; 149C push bx
db 0x52                                 ; 149D push dx
db 0x51                                 ; 149E push cx
db 0xE8, 0x86, 0x2C                     ; 149F call 0x4128
db 0x59                                 ; 14A2 pop cx
db 0x5A                                 ; 14A3 pop dx
db 0x32, 0xC0                           ; 14A4 xor al,al
db 0x8A, 0xF8                           ; 14A6 mov bh,al
db 0x8A, 0xD8                           ; 14A8 mov bl,al
db 0x89, 0x1E, 0xE9, 0x04               ; 14AA mov [0x4e9],bx
db 0x5B                                 ; 14AE pop bx
db 0xC3                                 ; 14AF ret
db 0x53                                 ; 14B0 push bx
db 0x32, 0xC0                           ; 14B1 xor al,al
db 0x9F                                 ; 14B3 lahf
db 0x86, 0xC4                           ; 14B4 xchg al,ah
db 0x50                                 ; 14B6 push ax
db 0x86, 0xC4                           ; 14B7 xchg al,ah
db 0xE8, 0xE9, 0x2A                     ; 14B9 call 0x3fa5
db 0x74, 0x03                           ; 14BC jz 0x14c1
db 0xE9, 0xE2, 0xF2                     ; 14BE jmp 0x7a3
db 0x53                                 ; 14C1 push bx
db 0xB9, 0x2E, 0x00                     ; 14C2 mov cx,0x2e
db 0xB2, 0x02                           ; 14C5 mov dl,0x2
db 0xB6, 0xFD                           ; 14C7 mov dh,0xfd
db 0x03, 0xD9                           ; 14C9 add bx,cx
db 0x88, 0x37                           ; 14CB mov [bx],dh
db 0xB0, 0x00                           ; 14CD mov al,0x0
db 0x5B                                 ; 14CF pop bx
db 0xE9, 0xE6, 0x29                     ; 14D0 jmp 0x3eb9
db 0xE8, 0x33, 0x2E                     ; 14D3 call 0x4309
db 0x0A, 0xC0                           ; 14D6 or al,al
db 0xC3                                 ; 14D8 ret
db 0x3C, 0x85                           ; 14D9 cmp al,0x85
db 0x74, 0x03                           ; 14DB jz 0x14e0
db 0xE9, 0xA2, 0x33                     ; 14DD jmp 0x4882
db 0xE8, 0x11, 0x19                     ; 14E0 call 0x2df4
db 0x85, 0x3C                           ; 14E3 test [si],di
db 0x23, 0x75, 0x03                     ; 14E5 and si,[di+0x3]
db 0xE9, 0x28, 0x30                     ; 14E8 jmp 0x4513
db 0xE8, 0x30, 0x1D                     ; 14EB call 0x321e
db 0xE8, 0x73, 0x00                     ; 14EE call 0x1564
db 0xE8, 0x7A, 0x22                     ; 14F1 call 0x376e
db 0xE8, 0x44, 0x4F                     ; 14F4 call 0x643b
db 0x52                                 ; 14F7 push dx
db 0x53                                 ; 14F8 push bx
db 0xE8, 0xA7, 0x1C                     ; 14F9 call 0x31a3
db 0x5A                                 ; 14FC pop dx
db 0x59                                 ; 14FD pop cx
db 0x73, 0x03                           ; 14FE jnc 0x1503
db 0xE9, 0x45, 0x19                     ; 1500 jmp 0x2e48
db 0x51                                 ; 1503 push cx
db 0x52                                 ; 1504 push dx
db 0xB5, 0x00                           ; 1505 mov ch,0x0
db 0xE8, 0x45, 0x11                     ; 1507 call 0x264f
db 0x5B                                 ; 150A pop bx
db 0xB0, 0x03                           ; 150B mov al,0x3
db 0xE9, 0x93, 0xFC                     ; 150D jmp 0x11a3
db 0x3F                                 ; 1510 aas
db 0x52                                 ; 1511 push dx
db 0x65, 0x64, 0x6F                     ; 1512 fs outsw
db 0x20, 0x66, 0x72                     ; 1515 and [bp+0x72],ah
db 0x6F                                 ; 1518 outsw
db 0x6D                                 ; 1519 insw
db 0x20, 0x73, 0x74                     ; 151A and [bp+di+0x74],dh
db 0x61                                 ; 151D popa
db 0x72, 0x74                           ; 151E jc 0x1594
db 0x0D, 0x00, 0x43                     ; 1520 or ax,0x4300
db 0x8A, 0x07                           ; 1523 mov al,[bx]
db 0x0A, 0xC0                           ; 1525 or al,al
db 0x75, 0x03                           ; 1527 jnz 0x152c
db 0xE9, 0x92, 0xF2                     ; 1529 jmp 0x7be
db 0x3C, 0x22                           ; 152C cmp al,0x22
db 0x75, 0xF2                           ; 152E jnz 0x1522
db 0xE9, 0x9B, 0x00                     ; 1530 jmp 0x15ce
db 0x5B                                 ; 1533 pop bx
db 0x5B                                 ; 1534 pop bx
db 0xEB, 0x0C                           ; 1535 jmp short 0x1543
db 0xCD, 0x9C                           ; 1537 int 0x9c
db 0xA0, 0x3A, 0x03                     ; 1539 mov al,[0x33a]
db 0x0A, 0xC0                           ; 153C or al,al
db 0x74, 0x03                           ; 153E jz 0x1543
db 0xE9, 0x73, 0xF2                     ; 1540 jmp 0x7b6
db 0x59                                 ; 1543 pop cx
db 0xBB, 0x10, 0x15                     ; 1544 mov bx,0x1510
db 0xE8, 0x0B, 0x66                     ; 1547 call 0x7b55
db 0x8B, 0x1E, 0x43, 0x03               ; 154A mov bx,[0x343]
db 0xC3                                 ; 154E ret
db 0xE8, 0x94, 0x2F                     ; 154F call 0x44e6
db 0x53                                 ; 1552 push bx
db 0xBB, 0xF6, 0x01                     ; 1553 mov bx,0x1f6
db 0xE9, 0xE0, 0x00                     ; 1556 jmp 0x1639
db 0x3C, 0x23                           ; 1559 cmp al,0x23
db 0x74, 0xF2                           ; 155B jz 0x154f
db 0xE8, 0xBE, 0x1C                     ; 155D call 0x321e
db 0xB9, 0x8C, 0x15                     ; 1560 mov cx,0x158c
db 0x51                                 ; 1563 push cx
db 0x3C, 0x22                           ; 1564 cmp al,0x22
db 0xB0, 0x00                           ; 1566 mov al,0x0
db 0xB0, 0xFF                           ; 1568 mov al,0xff
db 0xA2, 0x5F, 0x04                     ; 156A mov [0x45f],al
db 0x75, 0xDF                           ; 156D jnz 0x154e
db 0xE8, 0xDB, 0x10                     ; 156F call 0x264d
db 0x8A, 0x07                           ; 1572 mov al,[bx]
db 0x3C, 0x2C                           ; 1574 cmp al,0x2c
db 0x75, 0x0A                           ; 1576 jnz 0x1582
db 0x32, 0xC0                           ; 1578 xor al,al
db 0xA2, 0x5F, 0x04                     ; 157A mov [0x45f],al
db 0xE8, 0x9D, 0xF9                     ; 157D call 0xf1d
db 0xEB, 0x04                           ; 1580 jmp short 0x1586
db 0xE8, 0x6F, 0x18                     ; 1582 call 0x2df4
db 0x3B, 0x53, 0xE8                     ; 1585 cmp dx,[bp+di-0x18]
db 0x30, 0x11                           ; 1588 xor [bx+di],dl
db 0x5B                                 ; 158A pop bx
db 0xC3                                 ; 158B ret
db 0x53                                 ; 158C push bx
db 0xA0, 0x5F, 0x04                     ; 158D mov al,[0x45f]
db 0x0A, 0xC0                           ; 1590 or al,al
db 0x74, 0x0A                           ; 1592 jz 0x159e
db 0xB0, 0x3F                           ; 1594 mov al,0x3f
db 0xE8, 0x0C, 0x16                     ; 1596 call 0x2ba5
db 0xB0, 0x20                           ; 1599 mov al,0x20
db 0xE8, 0x07, 0x16                     ; 159B call 0x2ba5
db 0xE8, 0x02, 0x1C                     ; 159E call 0x31a3
db 0x59                                 ; 15A1 pop cx
db 0x73, 0x03                           ; 15A2 jnc 0x15a7
db 0xE9, 0xA1, 0x18                     ; 15A4 jmp 0x2e48
db 0x51                                 ; 15A7 push cx
db 0x32, 0xC0                           ; 15A8 xor al,al
db 0xA2, 0x3A, 0x03                     ; 15AA mov [0x33a],al
db 0xC6, 0x07, 0x2C                     ; 15AD mov byte [bx],0x2c
db 0x87, 0xDA                           ; 15B0 xchg bx,dx
db 0x5B                                 ; 15B2 pop bx
db 0x53                                 ; 15B3 push bx
db 0x52                                 ; 15B4 push dx
db 0x52                                 ; 15B5 push dx
db 0x4B                                 ; 15B6 dec bx
db 0xB0, 0x80                           ; 15B7 mov al,0x80
db 0xA2, 0x39, 0x03                     ; 15B9 mov [0x339],al
db 0xE8, 0x5E, 0xF9                     ; 15BC call 0xf1d
db 0xE8, 0xA5, 0x22                     ; 15BF call 0x3867
db 0x8A, 0x07                           ; 15C2 mov al,[bx]
db 0x4B                                 ; 15C4 dec bx
db 0x3C, 0x28                           ; 15C5 cmp al,0x28
db 0x75, 0x20                           ; 15C7 jnz 0x15e9
db 0x43                                 ; 15C9 inc bx
db 0xB5, 0x00                           ; 15CA mov ch,0x0
db 0xFE, 0xC5                           ; 15CC inc ch
db 0xE8, 0x4C, 0xF9                     ; 15CE call 0xf1d
db 0x75, 0x03                           ; 15D1 jnz 0x15d6
db 0xE9, 0xE8, 0xF1                     ; 15D3 jmp 0x7be
db 0x3C, 0x22                           ; 15D6 cmp al,0x22
db 0x75, 0x03                           ; 15D8 jnz 0x15dd
db 0xE9, 0x45, 0xFF                     ; 15DA jmp 0x1522
db 0x3C, 0x28                           ; 15DD cmp al,0x28
db 0x74, 0xEB                           ; 15DF jz 0x15cc
db 0x3C, 0x29                           ; 15E1 cmp al,0x29
db 0x75, 0xE9                           ; 15E3 jnz 0x15ce
db 0xFE, 0xCD                           ; 15E5 dec ch
db 0x75, 0xE5                           ; 15E7 jnz 0x15ce
db 0xE8, 0x31, 0xF9                     ; 15E9 call 0xf1d
db 0x74, 0x07                           ; 15EC jz 0x15f5
db 0x3C, 0x2C                           ; 15EE cmp al,0x2c
db 0x74, 0x03                           ; 15F0 jz 0x15f5
db 0xE9, 0xC9, 0xF1                     ; 15F2 jmp 0x7be
db 0x5E                                 ; 15F5 pop si
db 0x87, 0xDE                           ; 15F6 xchg bx,si
db 0x56                                 ; 15F8 push si
db 0x8A, 0x07                           ; 15F9 mov al,[bx]
db 0x3C, 0x2C                           ; 15FB cmp al,0x2c
db 0x74, 0x03                           ; 15FD jz 0x1602
db 0xE9, 0x31, 0xFF                     ; 15FF jmp 0x1533
db 0xB0, 0x01                           ; 1602 mov al,0x1
db 0xA2, 0xA9, 0x04                     ; 1604 mov [0x4a9],al
db 0xE8, 0x62, 0x00                     ; 1607 call 0x166c
db 0xA0, 0xA9, 0x04                     ; 160A mov al,[0x4a9]
db 0xFE, 0xC8                           ; 160D dec al
db 0x74, 0x03                           ; 160F jz 0x1614
db 0xE9, 0x1F, 0xFF                     ; 1611 jmp 0x1533
db 0x53                                 ; 1614 push bx
db 0xE8, 0x0D, 0x05                     ; 1615 call 0x1b25
db 0x75, 0x03                           ; 1618 jnz 0x161d
db 0xE8, 0x8C, 0x12                     ; 161A call 0x28a9
db 0x5B                                 ; 161D pop bx
db 0x4B                                 ; 161E dec bx
db 0xE8, 0xFB, 0xF8                     ; 161F call 0xf1d
db 0x5E                                 ; 1622 pop si
db 0x87, 0xDE                           ; 1623 xchg bx,si
db 0x56                                 ; 1625 push si
db 0x8A, 0x07                           ; 1626 mov al,[bx]
db 0x3C, 0x2C                           ; 1628 cmp al,0x2c
db 0x74, 0x8B                           ; 162A jz 0x15b7
db 0x5B                                 ; 162C pop bx
db 0x4B                                 ; 162D dec bx
db 0xE8, 0xEC, 0xF8                     ; 162E call 0xf1d
db 0x0A, 0xC0                           ; 1631 or al,al
db 0x5B                                 ; 1633 pop bx
db 0x74, 0x03                           ; 1634 jz 0x1639
db 0xE9, 0x0A, 0xFF                     ; 1636 jmp 0x1543
db 0xC6, 0x07, 0x2C                     ; 1639 mov byte [bx],0x2c
db 0xEB, 0x06                           ; 163C jmp short 0x1644
db 0x53                                 ; 163E push bx
db 0x8B, 0x1E, 0x5E, 0x03               ; 163F mov bx,[0x35e]
db 0x0D, 0x32, 0xC0                     ; 1643 or ax,0xc032
db 0xA2, 0x3A, 0x03                     ; 1646 mov [0x33a],al
db 0x5E                                 ; 1649 pop si
db 0x87, 0xDE                           ; 164A xchg bx,si
db 0x56                                 ; 164C push si
db 0xEB, 0x04                           ; 164D jmp short 0x1653
db 0xE8, 0xA2, 0x17                     ; 164F call 0x2df4
db 0x2C, 0xE8                           ; 1652 sub al,0xe8
db 0x18, 0x21                           ; 1654 sbb [bx+di],ah
db 0x5E                                 ; 1656 pop si
db 0x87, 0xDE                           ; 1657 xchg bx,si
db 0x56                                 ; 1659 push si
db 0x52                                 ; 165A push dx
db 0x8A, 0x07                           ; 165B mov al,[bx]
db 0x3C, 0x2C                           ; 165D cmp al,0x2c
db 0x74, 0x0A                           ; 165F jz 0x166b
db 0xA0, 0x3A, 0x03                     ; 1661 mov al,[0x33a]
db 0x0A, 0xC0                           ; 1664 or al,al
db 0x74, 0x03                           ; 1666 jz 0x166b
db 0xE9, 0x8B, 0x00                     ; 1668 jmp 0x16f6
db 0x0D, 0x32, 0xC0                     ; 166B or ax,0xc032
db 0xA2, 0x52, 0x04                     ; 166E mov [0x452],al
db 0xE8, 0x62, 0x19                     ; 1671 call 0x2fd6
db 0x74, 0x03                           ; 1674 jz 0x1679
db 0xE9, 0x8B, 0x2E                     ; 1676 jmp 0x4504
db 0xE8, 0xA9, 0x04                     ; 1679 call 0x1b25
db 0x50                                 ; 167C push ax
db 0x75, 0x38                           ; 167D jnz 0x16b7
db 0xE8, 0x9B, 0xF8                     ; 167F call 0xf1d
db 0x8A, 0xF0                           ; 1682 mov dh,al
db 0x8A, 0xE8                           ; 1684 mov ch,al
db 0x3C, 0x22                           ; 1686 cmp al,0x22
db 0x74, 0x0E                           ; 1688 jz 0x1698
db 0xA0, 0x3A, 0x03                     ; 168A mov al,[0x33a]
db 0x0A, 0xC0                           ; 168D or al,al
db 0x8A, 0xF0                           ; 168F mov dh,al
db 0x74, 0x02                           ; 1691 jz 0x1695
db 0xB6, 0x3A                           ; 1693 mov dh,0x3a
db 0xB5, 0x2C                           ; 1695 mov ch,0x2c
db 0x4B                                 ; 1697 dec bx
db 0xE8, 0xB6, 0x0F                     ; 1698 call 0x2651
db 0x58                                 ; 169B pop ax
db 0x04, 0x03                           ; 169C add al,0x3
db 0x8A, 0xC8                           ; 169E mov cl,al
db 0xA0, 0x52, 0x04                     ; 16A0 mov al,[0x452]
db 0x0A, 0xC0                           ; 16A3 or al,al
db 0x75, 0x01                           ; 16A5 jnz 0x16a8
db 0xC3                                 ; 16A7 ret
db 0x8A, 0xC1                           ; 16A8 mov al,cl
db 0x87, 0xDA                           ; 16AA xchg bx,dx
db 0xBB, 0xCA, 0x16                     ; 16AC mov bx,0x16ca
db 0x5E                                 ; 16AF pop si
db 0x87, 0xDE                           ; 16B0 xchg bx,si
db 0x56                                 ; 16B2 push si
db 0x52                                 ; 16B3 push dx
db 0xE9, 0xF0, 0xFA                     ; 16B4 jmp 0x11a7
db 0xE8, 0x63, 0xF8                     ; 16B7 call 0xf1d
db 0x58                                 ; 16BA pop ax
db 0x50                                 ; 16BB push ax
db 0x3C, 0x05                           ; 16BC cmp al,0x5
db 0xB9, 0x9B, 0x16                     ; 16BE mov cx,0x169b
db 0x51                                 ; 16C1 push cx
db 0x73, 0x03                           ; 16C2 jnc 0x16c7
db 0xE9, 0xF9, 0x52                     ; 16C4 jmp 0x69c0
db 0xE9, 0xFD, 0x52                     ; 16C7 jmp 0x69c7
db 0x4B                                 ; 16CA dec bx
db 0xE8, 0x4F, 0xF8                     ; 16CB call 0xf1d
db 0x74, 0x07                           ; 16CE jz 0x16d7
db 0x3C, 0x2C                           ; 16D0 cmp al,0x2c
db 0x74, 0x03                           ; 16D2 jz 0x16d7
db 0xE9, 0x60, 0xFE                     ; 16D4 jmp 0x1537
db 0x5E                                 ; 16D7 pop si
db 0x87, 0xDE                           ; 16D8 xchg bx,si
db 0x56                                 ; 16DA push si
db 0x4B                                 ; 16DB dec bx
db 0xE8, 0x3E, 0xF8                     ; 16DC call 0xf1d
db 0x74, 0x03                           ; 16DF jz 0x16e4
db 0xE9, 0x6B, 0xFF                     ; 16E1 jmp 0x164f
db 0x5A                                 ; 16E4 pop dx
db 0xA0, 0x3A, 0x03                     ; 16E5 mov al,[0x33a]
db 0x0A, 0xC0                           ; 16E8 or al,al
db 0x87, 0xDA                           ; 16EA xchg bx,dx
db 0x74, 0x03                           ; 16EC jz 0x16f1
db 0xE9, 0x35, 0x17                     ; 16EE jmp 0x2e26
db 0x52                                 ; 16F1 push dx
db 0x5B                                 ; 16F2 pop bx
db 0xE9, 0xA2, 0xFD                     ; 16F3 jmp 0x1498
db 0xE8, 0x62, 0xFA                     ; 16F6 call 0x115b
db 0x0A, 0xC0                           ; 16F9 or al,al
db 0x75, 0x15                           ; 16FB jnz 0x1712
db 0x43                                 ; 16FD inc bx
db 0x8A, 0x07                           ; 16FE mov al,[bx]
db 0x43                                 ; 1700 inc bx
db 0x0A, 0x07                           ; 1701 or al,[bx]
db 0xB2, 0x04                           ; 1703 mov dl,0x4
db 0x75, 0x03                           ; 1705 jnz 0x170a
db 0xE9, 0xCE, 0xF0                     ; 1707 jmp 0x7d8
db 0x43                                 ; 170A inc bx
db 0x8B, 0x17                           ; 170B mov dx,[bx]
db 0x43                                 ; 170D inc bx
db 0x89, 0x16, 0x37, 0x03               ; 170E mov [0x337],dx
db 0xE8, 0x08, 0xF8                     ; 1712 call 0xf1d
db 0x3C, 0x84                           ; 1715 cmp al,0x84
db 0x75, 0xDD                           ; 1717 jnz 0x16f6
db 0xE9, 0x4F, 0xFF                     ; 1719 jmp 0x166b
db 0xE8, 0xD5, 0x16                     ; 171C call 0x2df4
db 0xE7, 0xE9                           ; 171F out 0xe9,ax
db 0x04, 0x00                           ; 1721 add al,0x0
db 0xE8, 0xCE, 0x16                     ; 1723 call 0x2df4
db 0x28, 0x4B, 0xB6                     ; 1726 sub [bp+di-0x4a],cl
db 0x00, 0x52, 0xB1                     ; 1729 add [bp+si-0x4f],dl
db 0x01, 0xE8                           ; 172C add ax,bp
db 0xA5                                 ; 172E movsw
db 0x15, 0xCD, 0x9D                     ; 172F adc ax,0x9dcd
db 0xE8, 0xB2, 0x01                     ; 1732 call 0x18e7
db 0x32, 0xC0                           ; 1735 xor al,al
db 0xA2, 0xA8, 0x04                     ; 1737 mov [0x4a8],al
db 0x89, 0x1E, 0x52, 0x03               ; 173A mov [0x352],bx
db 0x8B, 0x1E, 0x52, 0x03               ; 173E mov bx,[0x352]
db 0x59                                 ; 1742 pop cx
db 0x8A, 0x07                           ; 1743 mov al,[bx]
db 0x89, 0x1E, 0x31, 0x03               ; 1745 mov [0x331],bx
db 0x3C, 0xE6                           ; 1749 cmp al,0xe6
db 0x73, 0x01                           ; 174B jnc 0x174e
db 0xC3                                 ; 174D ret
db 0x3C, 0xE9                           ; 174E cmp al,0xe9
db 0x72, 0x75                           ; 1750 jc 0x17c7
db 0x2C, 0xE9                           ; 1752 sub al,0xe9
db 0x8A, 0xD0                           ; 1754 mov dl,al
db 0x75, 0x0C                           ; 1756 jnz 0x1764
db 0xA0, 0xFB, 0x02                     ; 1758 mov al,[0x2fb]
db 0x3C, 0x03                           ; 175B cmp al,0x3
db 0x8A, 0xC2                           ; 175D mov al,dl
db 0x75, 0x03                           ; 175F jnz 0x1764
db 0xE9, 0xD6, 0x10                     ; 1761 jmp 0x283a
db 0x3C, 0x0C                           ; 1764 cmp al,0xc
db 0x73, 0xE5                           ; 1766 jnc 0x174d
db 0xBB, 0x80, 0x03                     ; 1768 mov bx,0x380
db 0xB6, 0x00                           ; 176B mov dh,0x0
db 0x03, 0xDA                           ; 176D add bx,dx
db 0x8A, 0xC5                           ; 176F mov al,ch
db 0x2E, 0x8A, 0x37                     ; 1771 mov dh,[cs:bx]
db 0x3A, 0xC6                           ; 1774 cmp al,dh
db 0x73, 0xD5                           ; 1776 jnc 0x174d
db 0x51                                 ; 1778 push cx
db 0xB9, 0x3E, 0x17                     ; 1779 mov cx,0x173e
db 0x51                                 ; 177C push cx
db 0x8A, 0xC6                           ; 177D mov al,dh
db 0xCD, 0x9E                           ; 177F int 0x9e
db 0x3C, 0x7F                           ; 1781 cmp al,0x7f
db 0x74, 0x64                           ; 1783 jz 0x17e9
db 0x3C, 0x51                           ; 1785 cmp al,0x51
db 0x72, 0x6D                           ; 1787 jc 0x17f6
db 0x24, 0xFE                           ; 1789 and al,0xfe
db 0x3C, 0x7A                           ; 178B cmp al,0x7a
db 0x74, 0x67                           ; 178D jz 0x17f6
db 0xA0, 0xFB, 0x02                     ; 178F mov al,[0x2fb]
db 0x2C, 0x03                           ; 1792 sub al,0x3
db 0x75, 0x03                           ; 1794 jnz 0x1799
db 0xE9, 0x3D, 0xF0                     ; 1796 jmp 0x7d6
db 0x0A, 0xC0                           ; 1799 or al,al
db 0xFF, 0x36, 0xA3, 0x04               ; 179B push word [0x4a3]
db 0x79, 0x03                           ; 179F jns 0x17a4
db 0xE9, 0x11, 0x00                     ; 17A1 jmp 0x17b5
db 0xFF, 0x36, 0xA5, 0x04               ; 17A4 push word [0x4a5]
db 0x7A, 0x03                           ; 17A8 jpe 0x17ad
db 0xE9, 0x08, 0x00                     ; 17AA jmp 0x17b5
db 0xFF, 0x36, 0x9F, 0x04               ; 17AD push word [0x49f]
db 0xFF, 0x36, 0xA1, 0x04               ; 17B1 push word [0x4a1]
db 0x04, 0x03                           ; 17B5 add al,0x3
db 0x8A, 0xCA                           ; 17B7 mov cl,dl
db 0x8A, 0xE8                           ; 17B9 mov ch,al
db 0x51                                 ; 17BB push cx
db 0xB9, 0x23, 0x18                     ; 17BC mov cx,0x1823
db 0x51                                 ; 17BF push cx
db 0x8B, 0x1E, 0x31, 0x03               ; 17C0 mov bx,[0x331]
db 0xE9, 0x63, 0xFF                     ; 17C4 jmp 0x172a
db 0xB6, 0x00                           ; 17C7 mov dh,0x0
db 0x2C, 0xE6                           ; 17C9 sub al,0xe6
db 0x72, 0x34                           ; 17CB jc 0x1801
db 0x3C, 0x03                           ; 17CD cmp al,0x3
db 0x73, 0x30                           ; 17CF jnc 0x1801
db 0x3C, 0x01                           ; 17D1 cmp al,0x1
db 0xD0, 0xD0                           ; 17D3 rcl al,1
db 0x32, 0xC6                           ; 17D5 xor al,dh
db 0x3A, 0xC6                           ; 17D7 cmp al,dh
db 0x8A, 0xF0                           ; 17D9 mov dh,al
db 0x73, 0x03                           ; 17DB jnc 0x17e0
db 0xE9, 0xDE, 0xEF                     ; 17DD jmp 0x7be
db 0x89, 0x1E, 0x31, 0x03               ; 17E0 mov [0x331],bx
db 0xE8, 0x36, 0xF7                     ; 17E4 call 0xf1d
db 0xEB, 0xE0                           ; 17E7 jmp short 0x17c9
db 0xE8, 0x65, 0x53                     ; 17E9 call 0x6b51
db 0xE8, 0x37, 0x4C                     ; 17EC call 0x6426
db 0xB9, 0x29, 0x65                     ; 17EF mov cx,0x6529
db 0xB6, 0x7F                           ; 17F2 mov dh,0x7f
db 0xEB, 0xC9                           ; 17F4 jmp short 0x17bf
db 0x52                                 ; 17F6 push dx
db 0xE8, 0xB3, 0x53                     ; 17F7 call 0x6bad
db 0x5A                                 ; 17FA pop dx
db 0x53                                 ; 17FB push bx
db 0xB9, 0x31, 0x1B                     ; 17FC mov cx,0x1b31
db 0xEB, 0xBE                           ; 17FF jmp short 0x17bf
db 0x8A, 0xC5                           ; 1801 mov al,ch
db 0x3C, 0x64                           ; 1803 cmp al,0x64
db 0x72, 0x01                           ; 1805 jc 0x1808
db 0xC3                                 ; 1807 ret
db 0x51                                 ; 1808 push cx
db 0x52                                 ; 1809 push dx
db 0xBA, 0x04, 0x64                     ; 180A mov dx,0x6404
db 0xBB, 0x03, 0x1B                     ; 180D mov bx,0x1b03
db 0x53                                 ; 1810 push bx
db 0xE8, 0x11, 0x03                     ; 1811 call 0x1b25
db 0x74, 0x03                           ; 1814 jz 0x1819
db 0xE9, 0x76, 0xFF                     ; 1816 jmp 0x178f
db 0x8B, 0x1E, 0xA3, 0x04               ; 1819 mov bx,[0x4a3]
db 0x53                                 ; 181D push bx
db 0xB9, 0xC8, 0x25                     ; 181E mov cx,0x25c8
db 0xEB, 0x9C                           ; 1821 jmp short 0x17bf
db 0x59                                 ; 1823 pop cx
db 0x8A, 0xC1                           ; 1824 mov al,cl
db 0xA2, 0xFC, 0x02                     ; 1826 mov [0x2fc],al
db 0xA0, 0xFB, 0x02                     ; 1829 mov al,[0x2fb]
db 0x3A, 0xC5                           ; 182C cmp al,ch
db 0x75, 0x0D                           ; 182E jnz 0x183d
db 0x3C, 0x02                           ; 1830 cmp al,0x2
db 0x74, 0x28                           ; 1832 jz 0x185c
db 0x3C, 0x04                           ; 1834 cmp al,0x4
db 0x75, 0x03                           ; 1836 jnz 0x183b
db 0xE9, 0x7F, 0x00                     ; 1838 jmp 0x18ba
db 0x73, 0x39                           ; 183B jnc 0x1876
db 0x8A, 0xF0                           ; 183D mov dh,al
db 0x8A, 0xC5                           ; 183F mov al,ch
db 0x3C, 0x08                           ; 1841 cmp al,0x8
db 0x74, 0x2E                           ; 1843 jz 0x1873
db 0x8A, 0xC6                           ; 1845 mov al,dh
db 0x3C, 0x08                           ; 1847 cmp al,0x8
db 0x74, 0x57                           ; 1849 jz 0x18a2
db 0x8A, 0xC5                           ; 184B mov al,ch
db 0x3C, 0x04                           ; 184D cmp al,0x4
db 0x74, 0x66                           ; 184F jz 0x18b7
db 0x8A, 0xC6                           ; 1851 mov al,dh
db 0x3C, 0x03                           ; 1853 cmp al,0x3
db 0x75, 0x03                           ; 1855 jnz 0x185a
db 0xE9, 0x7C, 0xEF                     ; 1857 jmp 0x7d6
db 0x73, 0x65                           ; 185A jnc 0x18c1
db 0xBB, 0xAA, 0x03                     ; 185C mov bx,0x3aa
db 0xB5, 0x00                           ; 185F mov ch,0x0
db 0x03, 0xD9                           ; 1861 add bx,cx
db 0x03, 0xD9                           ; 1863 add bx,cx
db 0x2E, 0x8A, 0x0F                     ; 1865 mov cl,[cs:bx]
db 0x43                                 ; 1868 inc bx
db 0x2E, 0x8A, 0x2F                     ; 1869 mov ch,[cs:bx]
db 0x5A                                 ; 186C pop dx
db 0x8B, 0x1E, 0xA3, 0x04               ; 186D mov bx,[0x4a3]
db 0x51                                 ; 1871 push cx
db 0xC3                                 ; 1872 ret
db 0xE8, 0x0C, 0x53                     ; 1873 call 0x6b82
db 0xE8, 0x25, 0x4C                     ; 1876 call 0x649e
db 0x5B                                 ; 1879 pop bx
db 0x89, 0x1E, 0xA1, 0x04               ; 187A mov [0x4a1],bx
db 0x5B                                 ; 187E pop bx
db 0x89, 0x1E, 0x9F, 0x04               ; 187F mov [0x49f],bx
db 0x59                                 ; 1883 pop cx
db 0x5A                                 ; 1884 pop dx
db 0xE8, 0x5A, 0x4B                     ; 1885 call 0x63e2
db 0xE8, 0xF7, 0x52                     ; 1888 call 0x6b82
db 0xBB, 0x96, 0x03                     ; 188B mov bx,0x396
db 0xA0, 0xFC, 0x02                     ; 188E mov al,[0x2fc]
db 0xD0, 0xC0                           ; 1891 rol al,1
db 0x02, 0xC3                           ; 1893 add al,bl
db 0x8A, 0xD8                           ; 1895 mov bl,al
db 0x12, 0xC7                           ; 1897 adc al,bh
db 0x2A, 0xC3                           ; 1899 sub al,bl
db 0x8A, 0xF8                           ; 189B mov bh,al
db 0x2E, 0x8B, 0x1F                     ; 189D mov bx,[cs:bx]
db 0xFF, 0xE3                           ; 18A0 jmp bx
db 0x8A, 0xC5                           ; 18A2 mov al,ch
db 0x50                                 ; 18A4 push ax
db 0xE8, 0xF6, 0x4B                     ; 18A5 call 0x649e
db 0x58                                 ; 18A8 pop ax
db 0xA2, 0xFB, 0x02                     ; 18A9 mov [0x2fb],al
db 0x3C, 0x04                           ; 18AC cmp al,0x4
db 0x74, 0xD3                           ; 18AE jz 0x1883
db 0x5B                                 ; 18B0 pop bx
db 0x89, 0x1E, 0xA3, 0x04               ; 18B1 mov [0x4a3],bx
db 0xEB, 0xD1                           ; 18B5 jmp short 0x1888
db 0xE8, 0x97, 0x52                     ; 18B7 call 0x6b51
db 0x59                                 ; 18BA pop cx
db 0x5A                                 ; 18BB pop dx
db 0xBB, 0xA0, 0x03                     ; 18BC mov bx,0x3a0
db 0xEB, 0xCD                           ; 18BF jmp short 0x188e
db 0x5B                                 ; 18C1 pop bx
db 0xE8, 0x61, 0x4B                     ; 18C2 call 0x6426
db 0xE8, 0x65, 0x4A                     ; 18C5 call 0x632d
db 0xE8, 0x28, 0x4B                     ; 18C8 call 0x63f3
db 0x5B                                 ; 18CB pop bx
db 0x89, 0x1E, 0xA5, 0x04               ; 18CC mov [0x4a5],bx
db 0x5B                                 ; 18D0 pop bx
db 0x89, 0x1E, 0xA3, 0x04               ; 18D1 mov [0x4a3],bx
db 0xEB, 0xE5                           ; 18D5 jmp short 0x18bc
db 0x53                                 ; 18D7 push bx
db 0x87, 0xDA                           ; 18D8 xchg bx,dx
db 0xE8, 0x50, 0x4A                     ; 18DA call 0x632d
db 0x5B                                 ; 18DD pop bx
db 0xE8, 0x45, 0x4B                     ; 18DE call 0x6426
db 0xE8, 0x49, 0x4A                     ; 18E1 call 0x632d
db 0xE9, 0x3B, 0x4C                     ; 18E4 jmp 0x6522
db 0xE8, 0x33, 0xF6                     ; 18E7 call 0xf1d
db 0x75, 0x03                           ; 18EA jnz 0x18ef
db 0xE9, 0xE4, 0xEE                     ; 18EC jmp 0x7d3
db 0x73, 0x03                           ; 18EF jnc 0x18f4
db 0xE9, 0xCC, 0x50                     ; 18F1 jmp 0x69c0
db 0xE8, 0x4E, 0x16                     ; 18F4 call 0x2f45
db 0x72, 0x03                           ; 18F7 jc 0x18fc
db 0xE9, 0xDB, 0x00                     ; 18F9 jmp 0x19d7
db 0x3C, 0x20                           ; 18FC cmp al,0x20
db 0x73, 0x03                           ; 18FE jnc 0x1903
db 0xE9, 0x7F, 0xF6                     ; 1900 jmp 0xf82
db 0xCD, 0x9F                           ; 1903 int 0x9f
db 0xFE, 0xC0                           ; 1905 inc al
db 0x75, 0x03                           ; 1907 jnz 0x190c
db 0xE9, 0x6A, 0x01                     ; 1909 jmp 0x1a76
db 0xFE, 0xC8                           ; 190C dec al
db 0x3C, 0xE9                           ; 190E cmp al,0xe9
db 0x74, 0xD5                           ; 1910 jz 0x18e7
db 0x3C, 0xEA                           ; 1912 cmp al,0xea
db 0x75, 0x03                           ; 1914 jnz 0x1919
db 0xE9, 0xAF, 0x00                     ; 1916 jmp 0x19c8
db 0x3C, 0x22                           ; 1919 cmp al,0x22
db 0x75, 0x03                           ; 191B jnz 0x1920
db 0xE9, 0x2D, 0x0D                     ; 191D jmp 0x264d
db 0x3C, 0xD3                           ; 1920 cmp al,0xd3
db 0x75, 0x03                           ; 1922 jnz 0x1927
db 0xE9, 0xEC, 0x01                     ; 1924 jmp 0x1b13
db 0x3C, 0x26                           ; 1927 cmp al,0x26
db 0x75, 0x03                           ; 1929 jnz 0x192e
db 0xE9, 0xD1, 0x00                     ; 192B jmp 0x19ff
db 0x3C, 0xD5                           ; 192E cmp al,0xd5
db 0x75, 0x0C                           ; 1930 jnz 0x193e
db 0xE8, 0xE8, 0xF5                     ; 1932 call 0xf1d
db 0xA0, 0x28, 0x00                     ; 1935 mov al,[0x28]
db 0x53                                 ; 1938 push bx
db 0xE8, 0x43, 0x02                     ; 1939 call 0x1b7f
db 0x5B                                 ; 193C pop bx
db 0xC3                                 ; 193D ret
db 0x3C, 0xD4                           ; 193E cmp al,0xd4
db 0x75, 0x0D                           ; 1940 jnz 0x194f
db 0xE8, 0xD8, 0xF5                     ; 1942 call 0xf1d
db 0x53                                 ; 1945 push bx
db 0x8B, 0x1E, 0x47, 0x03               ; 1946 mov bx,[0x347]
db 0xE8, 0xF7, 0x4A                     ; 194A call 0x6444
db 0x5B                                 ; 194D pop bx
db 0xC3                                 ; 194E ret
db 0x3C, 0xDA                           ; 194F cmp al,0xda
db 0x75, 0x2E                           ; 1951 jnz 0x1981
db 0xE8, 0xC7, 0xF5                     ; 1953 call 0xf1d
db 0xE8, 0x9B, 0x14                     ; 1956 call 0x2df4
db 0x28, 0x3C                           ; 1959 sub [si],bh
db 0x23, 0x75, 0x0D                     ; 195B and si,[di+0xd]
db 0xE8, 0xB8, 0x05                     ; 195E call 0x1f19
db 0x53                                 ; 1961 push bx
db 0xE8, 0x40, 0x26                     ; 1962 call 0x3fa5
db 0x87, 0xDA                           ; 1965 xchg bx,dx
db 0x5B                                 ; 1967 pop bx
db 0xE9, 0x03, 0x00                     ; 1968 jmp 0x196e
db 0xE8, 0xF9, 0x1E                     ; 196B call 0x3867
db 0xE8, 0x83, 0x14                     ; 196E call 0x2df4
db 0x29, 0x53, 0x87                     ; 1971 sub [bp+di-0x79],dx
db 0xDA, 0x0B                           ; 1974 fimul dword [bp+di]
db 0xDB                                 ; 1976 db 0xDB
db 0x75, 0x03                           ; 1977 jnz 0x197c
db 0xE9, 0xDD, 0xF6                     ; 1979 jmp 0x1059
db 0xE8, 0x8D, 0x4B                     ; 197C call 0x650c
db 0x5B                                 ; 197F pop bx
db 0xC3                                 ; 1980 ret
db 0x3C, 0xD0                           ; 1981 cmp al,0xd0
db 0x75, 0x03                           ; 1983 jnz 0x1988
db 0xE9, 0x00, 0x02                     ; 1985 jmp 0x1b88
db 0x3C, 0xD8                           ; 1988 cmp al,0xd8
db 0x75, 0x03                           ; 198A jnz 0x198f
db 0xE9, 0x8B, 0x10                     ; 198C jmp 0x2a1a
db 0x3C, 0xC8                           ; 198F cmp al,0xc8
db 0x75, 0x03                           ; 1991 jnz 0x1996
db 0xE9, 0xD0, 0x3B                     ; 1993 jmp 0x5566
db 0x3C, 0xDC                           ; 1996 cmp al,0xdc
db 0x75, 0x03                           ; 1998 jnz 0x199d
db 0xE9, 0x4C, 0x2E                     ; 199A jmp 0x47e9
db 0x3C, 0xDE                           ; 199D cmp al,0xde
db 0x75, 0x03                           ; 199F jnz 0x19a4
db 0xE9, 0xF9, 0x12                     ; 19A1 jmp 0x2c9d
db 0x3C, 0xD6                           ; 19A4 cmp al,0xd6
db 0x75, 0x03                           ; 19A6 jnz 0x19ab
db 0xE9, 0x70, 0x0F                     ; 19A8 jmp 0x291b
db 0x3C, 0x85                           ; 19AB cmp al,0x85
db 0x75, 0x03                           ; 19AD jnz 0x19b2
db 0xE9, 0x38, 0x2A                     ; 19AF jmp 0x43ea
db 0x3C, 0xDB                           ; 19B2 cmp al,0xdb
db 0x75, 0x03                           ; 19B4 jnz 0x19b9
db 0xE9, 0xA4, 0x3B                     ; 19B6 jmp 0x555d
db 0x3C, 0xD1                           ; 19B9 cmp al,0xd1
db 0x75, 0x03                           ; 19BB jnz 0x19c0
db 0xE9, 0x7E, 0x02                     ; 19BD jmp 0x1c3e
db 0xE8, 0x60, 0xFD                     ; 19C0 call 0x1723
db 0xE8, 0x2E, 0x14                     ; 19C3 call 0x2df4
db 0x29, 0xC3                           ; 19C6 sub bx,ax
db 0xB6, 0x7D                           ; 19C8 mov dh,0x7d
db 0xE8, 0x5D, 0xFD                     ; 19CA call 0x172a
db 0x8B, 0x1E, 0x52, 0x03               ; 19CD mov bx,[0x352]
db 0x53                                 ; 19D1 push bx
db 0xE8, 0xD6, 0x63                     ; 19D2 call 0x7dab
db 0x5B                                 ; 19D5 pop bx
db 0xC3                                 ; 19D6 ret
db 0xE8, 0x94, 0x1D                     ; 19D7 call 0x376e
db 0x53                                 ; 19DA push bx
db 0x87, 0xDA                           ; 19DB xchg bx,dx
db 0x89, 0x1E, 0xA3, 0x04               ; 19DD mov [0x4a3],bx
db 0xE8, 0x41, 0x01                     ; 19E1 call 0x1b25
db 0x74, 0x03                           ; 19E4 jz 0x19e9
db 0xE8, 0xAF, 0x4A                     ; 19E6 call 0x6498
db 0x5B                                 ; 19E9 pop bx
db 0xC3                                 ; 19EA ret
db 0x8A, 0x07                           ; 19EB mov al,[bx]
db 0x3C, 0x61                           ; 19ED cmp al,0x61
db 0x72, 0xF9                           ; 19EF jc 0x19ea
db 0x3C, 0x7B                           ; 19F1 cmp al,0x7b
db 0x73, 0xF5                           ; 19F3 jnc 0x19ea
db 0x24, 0x5F                           ; 19F5 and al,0x5f
db 0xC3                                 ; 19F7 ret
db 0x3C, 0x26                           ; 19F8 cmp al,0x26
db 0x74, 0x03                           ; 19FA jz 0x19ff
db 0xE9, 0x6C, 0xF6                     ; 19FC jmp 0x106b
db 0xBA, 0x00, 0x00                     ; 19FF mov dx,0x0
db 0xE8, 0x18, 0xF5                     ; 1A02 call 0xf1d
db 0xE8, 0xE5, 0xFF                     ; 1A05 call 0x19ed
db 0x3C, 0x4F                           ; 1A08 cmp al,0x4f
db 0x74, 0x39                           ; 1A0A jz 0x1a45
db 0x3C, 0x48                           ; 1A0C cmp al,0x48
db 0x75, 0x34                           ; 1A0E jnz 0x1a44
db 0xB5, 0x05                           ; 1A10 mov ch,0x5
db 0x43                                 ; 1A12 inc bx
db 0x8A, 0x07                           ; 1A13 mov al,[bx]
db 0xE8, 0xD5, 0xFF                     ; 1A15 call 0x19ed
db 0xE8, 0x2A, 0x15                     ; 1A18 call 0x2f45
db 0x87, 0xDA                           ; 1A1B xchg bx,dx
db 0x73, 0x0A                           ; 1A1D jnc 0x1a29
db 0x3C, 0x3A                           ; 1A1F cmp al,0x3a
db 0x73, 0x4D                           ; 1A21 jnc 0x1a70
db 0x2C, 0x30                           ; 1A23 sub al,0x30
db 0x72, 0x49                           ; 1A25 jc 0x1a70
db 0xEB, 0x06                           ; 1A27 jmp short 0x1a2f
db 0x3C, 0x47                           ; 1A29 cmp al,0x47
db 0x73, 0x43                           ; 1A2B jnc 0x1a70
db 0x2C, 0x37                           ; 1A2D sub al,0x37
db 0x03, 0xDB                           ; 1A2F add bx,bx
db 0x03, 0xDB                           ; 1A31 add bx,bx
db 0x03, 0xDB                           ; 1A33 add bx,bx
db 0x03, 0xDB                           ; 1A35 add bx,bx
db 0x0A, 0xC3                           ; 1A37 or al,bl
db 0x8A, 0xD8                           ; 1A39 mov bl,al
db 0x87, 0xDA                           ; 1A3B xchg bx,dx
db 0xFE, 0xCD                           ; 1A3D dec ch
db 0x75, 0xD1                           ; 1A3F jnz 0x1a12
db 0xE9, 0x8C, 0xED                     ; 1A41 jmp 0x7d0
db 0x4B                                 ; 1A44 dec bx
db 0xE8, 0xD5, 0xF4                     ; 1A45 call 0xf1d
db 0x87, 0xDA                           ; 1A48 xchg bx,dx
db 0x73, 0x24                           ; 1A4A jnc 0x1a70
db 0x3C, 0x38                           ; 1A4C cmp al,0x38
db 0x72, 0x03                           ; 1A4E jc 0x1a53
db 0xE9, 0x6B, 0xED                     ; 1A50 jmp 0x7be
db 0xB9, 0xD0, 0x07                     ; 1A53 mov cx,0x7d0
db 0x51                                 ; 1A56 push cx
db 0x03, 0xDB                           ; 1A57 add bx,bx
db 0x72, 0x9C                           ; 1A59 jc 0x19f7
db 0x03, 0xDB                           ; 1A5B add bx,bx
db 0x72, 0x98                           ; 1A5D jc 0x19f7
db 0x03, 0xDB                           ; 1A5F add bx,bx
db 0x72, 0x94                           ; 1A61 jc 0x19f7
db 0x59                                 ; 1A63 pop cx
db 0xB5, 0x00                           ; 1A64 mov ch,0x0
db 0x2C, 0x30                           ; 1A66 sub al,0x30
db 0x8A, 0xC8                           ; 1A68 mov cl,al
db 0x03, 0xD9                           ; 1A6A add bx,cx
db 0x87, 0xDA                           ; 1A6C xchg bx,dx
db 0xEB, 0xD5                           ; 1A6E jmp short 0x1a45
db 0xE8, 0x99, 0x4A                     ; 1A70 call 0x650c
db 0x87, 0xDA                           ; 1A73 xchg bx,dx
db 0xC3                                 ; 1A75 ret
db 0x43                                 ; 1A76 inc bx
db 0x8A, 0x07                           ; 1A77 mov al,[bx]
db 0x2C, 0x81                           ; 1A79 sub al,0x81
db 0x3C, 0x07                           ; 1A7B cmp al,0x7
db 0x75, 0x0E                           ; 1A7D jnz 0x1a8d
db 0x53                                 ; 1A7F push bx
db 0xE8, 0x9A, 0xF4                     ; 1A80 call 0xf1d
db 0x3C, 0x28                           ; 1A83 cmp al,0x28
db 0x5B                                 ; 1A85 pop bx
db 0x74, 0x03                           ; 1A86 jz 0x1a8b
db 0xE9, 0xE0, 0x49                     ; 1A88 jmp 0x646b
db 0xB0, 0x07                           ; 1A8B mov al,0x7
db 0xB5, 0x00                           ; 1A8D mov ch,0x0
db 0xD0, 0xC0                           ; 1A8F rol al,1
db 0x8A, 0xC8                           ; 1A91 mov cl,al
db 0x51                                 ; 1A93 push cx
db 0xE8, 0x86, 0xF4                     ; 1A94 call 0xf1d
db 0x8A, 0xC1                           ; 1A97 mov al,cl
db 0x3C, 0x05                           ; 1A99 cmp al,0x5
db 0x73, 0x22                           ; 1A9B jnc 0x1abf
db 0xE8, 0x83, 0xFC                     ; 1A9D call 0x1723
db 0xE8, 0x51, 0x13                     ; 1AA0 call 0x2df4
db 0x2C, 0xE8                           ; 1AA3 sub al,0xe8
db 0x94                                 ; 1AA5 xchg ax,sp
db 0x49                                 ; 1AA6 dec cx
db 0x87, 0xDA                           ; 1AA7 xchg bx,dx
db 0x8B, 0x1E, 0xA3, 0x04               ; 1AA9 mov bx,[0x4a3]
db 0x5E                                 ; 1AAD pop si
db 0x87, 0xDE                           ; 1AAE xchg bx,si
db 0x56                                 ; 1AB0 push si
db 0x53                                 ; 1AB1 push bx
db 0x87, 0xDA                           ; 1AB2 xchg bx,dx
db 0xE8, 0x65, 0x04                     ; 1AB4 call 0x1f1c
db 0x87, 0xDA                           ; 1AB7 xchg bx,dx
db 0x5E                                 ; 1AB9 pop si
db 0x87, 0xDE                           ; 1ABA xchg bx,si
db 0x56                                 ; 1ABC push si
db 0xEB, 0x21                           ; 1ABD jmp short 0x1ae0
db 0xE8, 0xFE, 0xFE                     ; 1ABF call 0x19c0
db 0x5E                                 ; 1AC2 pop si
db 0x87, 0xDE                           ; 1AC3 xchg bx,si
db 0x56                                 ; 1AC5 push si
db 0x8A, 0xC3                           ; 1AC6 mov al,bl
db 0x3C, 0x0C                           ; 1AC8 cmp al,0xc
db 0x72, 0x0B                           ; 1ACA jc 0x1ad7
db 0x3C, 0x1B                           ; 1ACC cmp al,0x1b
db 0xCD, 0xA1                           ; 1ACE int 0xa1
db 0x53                                 ; 1AD0 push bx
db 0x73, 0x03                           ; 1AD1 jnc 0x1ad6
db 0xE8, 0x7B, 0x50                     ; 1AD3 call 0x6b51
db 0x5B                                 ; 1AD6 pop bx
db 0xBA, 0xD5, 0x19                     ; 1AD7 mov dx,0x19d5
db 0x52                                 ; 1ADA push dx
db 0xB0, 0x01                           ; 1ADB mov al,0x1
db 0xA2, 0xA8, 0x04                     ; 1ADD mov [0x4a8],al
db 0xB9, 0xB9, 0x00                     ; 1AE0 mov cx,0xb9
db 0xCD, 0xA0                           ; 1AE3 int 0xa0
db 0x03, 0xD9                           ; 1AE5 add bx,cx
db 0x2E, 0xFF, 0x27                     ; 1AE7 jmp near [cs:bx]
db 0xFE, 0xCE                           ; 1AEA dec dh
db 0x3C, 0xEA                           ; 1AEC cmp al,0xea
db 0x74, 0x85                           ; 1AEE jz 0x1a75
db 0x3C, 0x2D                           ; 1AF0 cmp al,0x2d
db 0x74, 0x81                           ; 1AF2 jz 0x1a75
db 0xFE, 0xC6                           ; 1AF4 inc dh
db 0x3C, 0x2B                           ; 1AF6 cmp al,0x2b
db 0x75, 0x01                           ; 1AF8 jnz 0x1afb
db 0xC3                                 ; 1AFA ret
db 0x3C, 0xE9                           ; 1AFB cmp al,0xe9
db 0x74, 0xFB                           ; 1AFD jz 0x1afa
db 0x9F                                 ; 1AFF lahf
db 0x4B                                 ; 1B00 dec bx
db 0x9E                                 ; 1B01 sahf
db 0xC3                                 ; 1B02 ret
db 0xFE, 0xC0                           ; 1B03 inc al
db 0x12, 0xC0                           ; 1B05 adc al,al
db 0x59                                 ; 1B07 pop cx
db 0x22, 0xC5                           ; 1B08 and al,ch
db 0x04, 0xFF                           ; 1B0A add al,0xff
db 0x1A, 0xC0                           ; 1B0C sbb al,al
db 0xE8, 0xF8, 0x49                     ; 1B0E call 0x6509
db 0xEB, 0x0F                           ; 1B11 jmp short 0x1b22
db 0xB6, 0x5A                           ; 1B13 mov dh,0x5a
db 0xE8, 0x12, 0xFC                     ; 1B15 call 0x172a
db 0xE8, 0x92, 0x50                     ; 1B18 call 0x6bad
db 0xF7, 0xD3                           ; 1B1B not bx
db 0x89, 0x1E, 0xA3, 0x04               ; 1B1D mov [0x4a3],bx
db 0x59                                 ; 1B21 pop cx
db 0xE9, 0x19, 0xFC                     ; 1B22 jmp 0x173e
db 0xA0, 0xFB, 0x02                     ; 1B25 mov al,[0x2fb]
db 0x3C, 0x08                           ; 1B28 cmp al,0x8
db 0xFE, 0xC8                           ; 1B2A dec al
db 0xFE, 0xC8                           ; 1B2C dec al
db 0xFE, 0xC8                           ; 1B2E dec al
db 0xC3                                 ; 1B30 ret
db 0x8A, 0xC5                           ; 1B31 mov al,ch
db 0x50                                 ; 1B33 push ax
db 0xE8, 0x76, 0x50                     ; 1B34 call 0x6bad
db 0x58                                 ; 1B37 pop ax
db 0x5A                                 ; 1B38 pop dx
db 0x3C, 0x7A                           ; 1B39 cmp al,0x7a
db 0x75, 0x03                           ; 1B3B jnz 0x1b40
db 0xE9, 0x89, 0x4A                     ; 1B3D jmp 0x65c9
db 0x3C, 0x7B                           ; 1B40 cmp al,0x7b
db 0x75, 0x03                           ; 1B42 jnz 0x1b47
db 0xE9, 0x66, 0x48                     ; 1B44 jmp 0x63ad
db 0xB9, 0x0C, 0x65                     ; 1B47 mov cx,0x650c
db 0x51                                 ; 1B4A push cx
db 0x3C, 0x46                           ; 1B4B cmp al,0x46
db 0x75, 0x03                           ; 1B4D jnz 0x1b52
db 0x0B, 0xDA                           ; 1B4F or bx,dx
db 0xC3                                 ; 1B51 ret
db 0x3C, 0x50                           ; 1B52 cmp al,0x50
db 0x75, 0x03                           ; 1B54 jnz 0x1b59
db 0x23, 0xDA                           ; 1B56 and bx,dx
db 0xC3                                 ; 1B58 ret
db 0x3C, 0x3C                           ; 1B59 cmp al,0x3c
db 0x75, 0x03                           ; 1B5B jnz 0x1b60
db 0x33, 0xDA                           ; 1B5D xor bx,dx
db 0xC3                                 ; 1B5F ret
db 0x3C, 0x32                           ; 1B60 cmp al,0x32
db 0x75, 0x05                           ; 1B62 jnz 0x1b69
db 0x33, 0xDA                           ; 1B64 xor bx,dx
db 0xF7, 0xD3                           ; 1B66 not bx
db 0xC3                                 ; 1B68 ret
db 0xF7, 0xD3                           ; 1B69 not bx
db 0x23, 0xDA                           ; 1B6B and bx,dx
db 0xF7, 0xD3                           ; 1B6D not bx
db 0xC3                                 ; 1B6F ret

L_1B70:
  sub     bx, dx
  jmp     L_6444

db 0xA0, 0x63, 0x00                     ; 1B75 mov al,[0x63]
db 0xEB, 0x03                           ; 1B78 jmp short 0x1b7d
db 0xE8, 0xAF, 0x33                     ; 1B7A call 0x4f2c
db 0xFE, 0xC0                           ; 1B7D inc al
db 0x8A, 0xD8                           ; 1B7F mov bl,al
db 0x32, 0xC0                           ; 1B81 xor al,al
db 0x8A, 0xF8                           ; 1B83 mov bh,al
db 0xE9, 0x84, 0x49                     ; 1B85 jmp 0x650c
db 0xE8, 0x2E, 0x00                     ; 1B88 call 0x1bb9
db 0x52                                 ; 1B8B push dx
db 0xE8, 0x31, 0xFE                     ; 1B8C call 0x19c0
db 0x5E                                 ; 1B8F pop si
db 0x87, 0xDE                           ; 1B90 xchg bx,si
db 0x56                                 ; 1B92 push si
db 0x8B, 0x17                           ; 1B93 mov dx,[bx]
db 0x80, 0xFA, 0xFF                     ; 1B95 cmp dl,0xff
db 0x75, 0x03                           ; 1B98 jnz 0x1b9d
db 0xE9, 0xBC, 0xF4                     ; 1B9A jmp 0x1059
db 0x0E                                 ; 1B9D push cs
db 0xBB, 0x07, 0x65                     ; 1B9E mov bx,0x6507
db 0x53                                 ; 1BA1 push bx
db 0xFF, 0x36, 0x50, 0x03               ; 1BA2 push word [0x350]
db 0x52                                 ; 1BA6 push dx
db 0xA0, 0xFB, 0x02                     ; 1BA7 mov al,[0x2fb]
db 0x50                                 ; 1BAA push ax
db 0x3C, 0x03                           ; 1BAB cmp al,0x3
db 0x75, 0x03                           ; 1BAD jnz 0x1bb2
db 0xE8, 0xF7, 0x0C                     ; 1BAF call 0x28a9
db 0x58                                 ; 1BB2 pop ax
db 0x87, 0xDA                           ; 1BB3 xchg bx,dx
db 0xBB, 0xA3, 0x04                     ; 1BB5 mov bx,0x4a3
db 0xCB                                 ; 1BB8 retf
db 0xE8, 0x61, 0xF3                     ; 1BB9 call 0xf1d
db 0xB9, 0x00, 0x00                     ; 1BBC mov cx,0x0
db 0x3C, 0x1B                           ; 1BBF cmp al,0x1b
db 0x73, 0x10                           ; 1BC1 jnc 0x1bd3
db 0x3C, 0x11                           ; 1BC3 cmp al,0x11
db 0x72, 0x0C                           ; 1BC5 jc 0x1bd3
db 0xE8, 0x53, 0xF3                     ; 1BC7 call 0xf1d
db 0xA0, 0x02, 0x03                     ; 1BCA mov al,[0x302]
db 0x0A, 0xC0                           ; 1BCD or al,al
db 0xD0, 0xD0                           ; 1BCF rcl al,1
db 0x8A, 0xC8                           ; 1BD1 mov cl,al
db 0x87, 0xDA                           ; 1BD3 xchg bx,dx
db 0xBB, 0x12, 0x00                     ; 1BD5 mov bx,0x12
db 0x03, 0xD9                           ; 1BD8 add bx,cx
db 0x87, 0xDA                           ; 1BDA xchg bx,dx
db 0xC3                                 ; 1BDC ret
db 0xE8, 0xD9, 0xFF                     ; 1BDD call 0x1bb9
db 0x52                                 ; 1BE0 push dx
db 0xE8, 0x10, 0x12                     ; 1BE1 call 0x2df4
db 0xE7, 0xE8                           ; 1BE4 out 0xe8,ax
db 0xC2, 0x06, 0x5E                     ; 1BE6 ret 0x5e06
db 0x87, 0xDE                           ; 1BE9 xchg bx,si
db 0x56                                 ; 1BEB push si
db 0x89, 0x17                           ; 1BEC mov [bx],dx
db 0x5B                                 ; 1BEE pop bx
db 0xC3                                 ; 1BEF ret
db 0x3C, 0xD0                           ; 1BF0 cmp al,0xd0
db 0x74, 0xE9                           ; 1BF2 jz 0x1bdd
db 0x3C, 0xD1                           ; 1BF4 cmp al,0xd1
db 0x74, 0x1C                           ; 1BF6 jz 0x1c14
db 0xE8, 0xF9, 0x11                     ; 1BF8 call 0x2df4
db 0x53                                 ; 1BFB push bx
db 0xE8, 0xF5, 0x11                     ; 1BFC call 0x2df4
db 0x45                                 ; 1BFF inc bp
db 0xE8, 0xF1, 0x11                     ; 1C00 call 0x2df4
db 0x47                                 ; 1C03 inc di
db 0x8C, 0xDA                           ; 1C04 mov dx,ds
db 0x74, 0x07                           ; 1C06 jz 0x1c0f
db 0xE8, 0xE9, 0x11                     ; 1C08 call 0x2df4
db 0xE7, 0xE8                           ; 1C0B out 0xe8,ax
db 0x9B                                 ; 1C0D wait
db 0x06                                 ; 1C0E push es
db 0x89, 0x16, 0x50, 0x03               ; 1C0F mov [0x350],dx
db 0xC3                                 ; 1C13 ret
db 0xE8, 0xF3, 0x01                     ; 1C14 call 0x1e0a
db 0xE8, 0xE0, 0x01                     ; 1C17 call 0x1dfa
db 0x87, 0xDA                           ; 1C1A xchg bx,dx
db 0x89, 0x17                           ; 1C1C mov [bx],dx
db 0x87, 0xDA                           ; 1C1E xchg bx,dx
db 0x8A, 0x07                           ; 1C20 mov al,[bx]
db 0x3C, 0x28                           ; 1C22 cmp al,0x28
db 0x74, 0x03                           ; 1C24 jz 0x1c29
db 0xE9, 0x32, 0xF5                     ; 1C26 jmp 0x115b
db 0xE8, 0xF1, 0xF2                     ; 1C29 call 0xf1d
db 0xE8, 0x3F, 0x1B                     ; 1C2C call 0x376e
db 0x8A, 0x07                           ; 1C2F mov al,[bx]
db 0x3C, 0x29                           ; 1C31 cmp al,0x29
db 0x75, 0x03                           ; 1C33 jnz 0x1c38
db 0xE9, 0x23, 0xF5                     ; 1C35 jmp 0x115b
db 0xE8, 0xB9, 0x11                     ; 1C38 call 0x2df4
db 0x2C, 0xEB                           ; 1C3B sub al,0xeb
db 0xEE                                 ; 1C3D out dx,al
db 0xE8, 0xC9, 0x01                     ; 1C3E call 0x1e0a
db 0xA0, 0xFB, 0x02                     ; 1C41 mov al,[0x2fb]
db 0x0A, 0xC0                           ; 1C44 or al,al
db 0x50                                 ; 1C46 push ax
db 0x89, 0x1E, 0x52, 0x03               ; 1C47 mov [0x352],bx
db 0x87, 0xDA                           ; 1C4B xchg bx,dx
db 0x8B, 0x1F                           ; 1C4D mov bx,[bx]
db 0x0B, 0xDB                           ; 1C4F or bx,bx
db 0x75, 0x03                           ; 1C51 jnz 0x1c56
db 0xE9, 0x74, 0xEB                     ; 1C53 jmp 0x7ca
db 0x8A, 0x07                           ; 1C56 mov al,[bx]
db 0x3C, 0x28                           ; 1C58 cmp al,0x28
db 0x74, 0x03                           ; 1C5A jz 0x1c5f
db 0xE9, 0xCE, 0x00                     ; 1C5C jmp 0x1d2d
db 0xE8, 0xBB, 0xF2                     ; 1C5F call 0xf1d
db 0x89, 0x1E, 0x31, 0x03               ; 1C62 mov [0x331],bx
db 0x87, 0xDA                           ; 1C66 xchg bx,dx
db 0x8B, 0x1E, 0x52, 0x03               ; 1C68 mov bx,[0x352]
db 0xE8, 0x85, 0x11                     ; 1C6C call 0x2df4
db 0x28, 0x32                           ; 1C6F sub [bp+si],dh
db 0xC0, 0x50, 0x53, 0x87               ; 1C71 rcl byte [bx+si+0x53],0x87
db 0xDA, 0xB0, 0x80, 0xA2               ; 1C75 fidiv dword [bx+si+0xa280]
db 0x39, 0x03                           ; 1C79 cmp [bp+di],ax
db 0xE8, 0xF0, 0x1A                     ; 1C7B call 0x376e
db 0x87, 0xDA                           ; 1C7E xchg bx,dx
db 0x5E                                 ; 1C80 pop si
db 0x87, 0xDE                           ; 1C81 xchg bx,si
db 0x56                                 ; 1C83 push si
db 0xA0, 0xFB, 0x02                     ; 1C84 mov al,[0x2fb]
db 0x50                                 ; 1C87 push ax
db 0x52                                 ; 1C88 push dx
db 0xE8, 0x9B, 0xFA                     ; 1C89 call 0x1727
db 0x89, 0x1E, 0x52, 0x03               ; 1C8C mov [0x352],bx
db 0x5B                                 ; 1C90 pop bx
db 0x89, 0x1E, 0x31, 0x03               ; 1C91 mov [0x331],bx
db 0x58                                 ; 1C95 pop ax
db 0xE8, 0x41, 0x01                     ; 1C96 call 0x1dda
db 0xB1, 0x04                           ; 1C99 mov cl,0x4
db 0xE8, 0x37, 0x10                     ; 1C9B call 0x2cd5
db 0xBB, 0xF8, 0xFF                     ; 1C9E mov bx,0xfff8
db 0x03, 0xDC                           ; 1CA1 add bx,sp
db 0x8B, 0xE3                           ; 1CA3 mov sp,bx
db 0xE8, 0xF9, 0x47                     ; 1CA5 call 0x64a1
db 0xA0, 0xFB, 0x02                     ; 1CA8 mov al,[0x2fb]
db 0x50                                 ; 1CAB push ax
db 0x8B, 0x1E, 0x52, 0x03               ; 1CAC mov bx,[0x352]
db 0x8A, 0x07                           ; 1CB0 mov al,[bx]
db 0x3C, 0x29                           ; 1CB2 cmp al,0x29
db 0x74, 0x13                           ; 1CB4 jz 0x1cc9
db 0xE8, 0x3B, 0x11                     ; 1CB6 call 0x2df4
db 0x2C, 0x53                           ; 1CB9 sub al,0x53
db 0x8B, 0x1E, 0x31, 0x03               ; 1CBB mov bx,[0x331]
db 0xE8, 0x32, 0x11                     ; 1CBF call 0x2df4
db 0x2C, 0xEB                           ; 1CC2 sub al,0xeb
db 0xB1, 0x58                           ; 1CC4 mov cl,0x58
db 0xA2, 0xE4, 0x03                     ; 1CC6 mov [0x3e4],al
db 0x58                                 ; 1CC9 pop ax
db 0x0A, 0xC0                           ; 1CCA or al,al
db 0x74, 0x4E                           ; 1CCC jz 0x1d1c
db 0xA2, 0xFB, 0x02                     ; 1CCE mov [0x2fb],al
db 0xBB, 0x00, 0x00                     ; 1CD1 mov bx,0x0
db 0x03, 0xDC                           ; 1CD4 add bx,sp
db 0xE8, 0xBF, 0x47                     ; 1CD6 call 0x6498
db 0xBB, 0x08, 0x00                     ; 1CD9 mov bx,0x8
db 0x03, 0xDC                           ; 1CDC add bx,sp
db 0x8B, 0xE3                           ; 1CDE mov sp,bx
db 0x5A                                 ; 1CE0 pop dx
db 0xB3, 0x03                           ; 1CE1 mov bl,0x3
db 0xFE, 0xC3                           ; 1CE3 inc bl
db 0x4A                                 ; 1CE5 dec dx
db 0x8B, 0xF2                           ; 1CE6 mov si,dx
db 0xAC                                 ; 1CE8 lodsb
db 0x0A, 0xC0                           ; 1CE9 or al,al
db 0x78, 0xF6                           ; 1CEB js 0x1ce3
db 0x4A                                 ; 1CED dec dx
db 0x4A                                 ; 1CEE dec dx
db 0x4A                                 ; 1CEF dec dx
db 0xA0, 0xFB, 0x02                     ; 1CF0 mov al,[0x2fb]
db 0x02, 0xC3                           ; 1CF3 add al,bl
db 0x8A, 0xE8                           ; 1CF5 mov ch,al
db 0xA0, 0xE4, 0x03                     ; 1CF7 mov al,[0x3e4]
db 0x8A, 0xC8                           ; 1CFA mov cl,al
db 0x02, 0xC5                           ; 1CFC add al,ch
db 0x3C, 0x64                           ; 1CFE cmp al,0x64
db 0x72, 0x03                           ; 1D00 jc 0x1d05
db 0xE9, 0x54, 0xF3                     ; 1D02 jmp 0x1059
db 0x50                                 ; 1D05 push ax
db 0x8A, 0xC3                           ; 1D06 mov al,bl
db 0xB5, 0x00                           ; 1D08 mov ch,0x0
db 0xBB, 0xE6, 0x03                     ; 1D0A mov bx,0x3e6
db 0x03, 0xD9                           ; 1D0D add bx,cx
db 0x8A, 0xC8                           ; 1D0F mov cl,al
db 0xE8, 0xDF, 0x00                     ; 1D11 call 0x1df3
db 0xB9, 0xC5, 0x1C                     ; 1D14 mov cx,0x1cc5
db 0x51                                 ; 1D17 push cx
db 0x51                                 ; 1D18 push cx
db 0xE9, 0x99, 0xF4                     ; 1D19 jmp 0x11b5
db 0x8B, 0x1E, 0x52, 0x03               ; 1D1C mov bx,[0x352]
db 0xE8, 0xFA, 0xF1                     ; 1D20 call 0xf1d
db 0x53                                 ; 1D23 push bx
db 0x8B, 0x1E, 0x31, 0x03               ; 1D24 mov bx,[0x331]
db 0xE8, 0xC9, 0x10                     ; 1D28 call 0x2df4
db 0x29, 0xB0, 0x52, 0x89               ; 1D2B sub [bx+si+0x8952],si
db 0x1E                                 ; 1D2F push ds
db 0x31, 0x03                           ; 1D30 xor [bp+di],ax
db 0xA0, 0x7C, 0x03                     ; 1D32 mov al,[0x37c]
db 0x04, 0x04                           ; 1D35 add al,0x4
db 0x50                                 ; 1D37 push ax
db 0xD0, 0xC8                           ; 1D38 ror al,1
db 0x8A, 0xC8                           ; 1D3A mov cl,al
db 0xE8, 0x96, 0x0F                     ; 1D3C call 0x2cd5
db 0x58                                 ; 1D3F pop ax
db 0x8A, 0xC8                           ; 1D40 mov cl,al
db 0xF6, 0xD0                           ; 1D42 not al
db 0xFE, 0xC0                           ; 1D44 inc al
db 0x8A, 0xD8                           ; 1D46 mov bl,al
db 0xB7, 0xFF                           ; 1D48 mov bh,0xff
db 0x03, 0xDC                           ; 1D4A add bx,sp
db 0x8B, 0xE3                           ; 1D4C mov sp,bx
db 0x53                                 ; 1D4E push bx
db 0xBA, 0x7A, 0x03                     ; 1D4F mov dx,0x37a
db 0xE8, 0x9E, 0x00                     ; 1D52 call 0x1df3
db 0x5B                                 ; 1D55 pop bx
db 0x89, 0x1E, 0x7A, 0x03               ; 1D56 mov [0x37a],bx
db 0x8B, 0x1E, 0xE4, 0x03               ; 1D5A mov bx,[0x3e4]
db 0x89, 0x1E, 0x7C, 0x03               ; 1D5E mov [0x37c],bx
db 0x8B, 0xCB                           ; 1D62 mov cx,bx
db 0xBB, 0x7E, 0x03                     ; 1D64 mov bx,0x37e
db 0xBA, 0xE6, 0x03                     ; 1D67 mov dx,0x3e6
db 0xE8, 0x86, 0x00                     ; 1D6A call 0x1df3
db 0x8A, 0xF8                           ; 1D6D mov bh,al
db 0x8A, 0xD8                           ; 1D6F mov bl,al
db 0x89, 0x1E, 0xE4, 0x03               ; 1D71 mov [0x3e4],bx
db 0x8B, 0x1E, 0x50, 0x04               ; 1D75 mov bx,[0x450]
db 0x43                                 ; 1D79 inc bx
db 0x89, 0x1E, 0x50, 0x04               ; 1D7A mov [0x450],bx
db 0x8A, 0xC7                           ; 1D7E mov al,bh
db 0x0A, 0xC3                           ; 1D80 or al,bl
db 0xA2, 0x4D, 0x04                     ; 1D82 mov [0x44d],al
db 0x8B, 0x1E, 0x31, 0x03               ; 1D85 mov bx,[0x331]
db 0xE8, 0x90, 0xF9                     ; 1D89 call 0x171c
db 0x4B                                 ; 1D8C dec bx
db 0xE8, 0x8D, 0xF1                     ; 1D8D call 0xf1d
db 0x74, 0x03                           ; 1D90 jz 0x1d95
db 0xE9, 0x29, 0xEA                     ; 1D92 jmp 0x7be
db 0xE8, 0x8D, 0xFD                     ; 1D95 call 0x1b25
db 0x75, 0x11                           ; 1D98 jnz 0x1dab
db 0xBA, 0x2C, 0x03                     ; 1D9A mov dx,0x32c
db 0x8B, 0x1E, 0xA3, 0x04               ; 1D9D mov bx,[0x4a3]
db 0x3B, 0xDA                           ; 1DA1 cmp bx,dx
db 0x72, 0x06                           ; 1DA3 jc 0x1dab
db 0xE8, 0x7C, 0x08                     ; 1DA5 call 0x2624
db 0xE8, 0xE6, 0x08                     ; 1DA8 call 0x2691
db 0x8B, 0x1E, 0x7A, 0x03               ; 1DAB mov bx,[0x37a]
db 0x8A, 0xF7                           ; 1DAF mov dh,bh
db 0x8A, 0xD3                           ; 1DB1 mov dl,bl
db 0x43                                 ; 1DB3 inc bx
db 0x43                                 ; 1DB4 inc bx
db 0x8A, 0x0F                           ; 1DB5 mov cl,[bx]
db 0x43                                 ; 1DB7 inc bx
db 0x8A, 0x2F                           ; 1DB8 mov ch,[bx]
db 0x41                                 ; 1DBA inc cx
db 0x41                                 ; 1DBB inc cx
db 0x41                                 ; 1DBC inc cx
db 0x41                                 ; 1DBD inc cx
db 0xBB, 0x7A, 0x03                     ; 1DBE mov bx,0x37a
db 0xE8, 0x2F, 0x00                     ; 1DC1 call 0x1df3
db 0x87, 0xDA                           ; 1DC4 xchg bx,dx
db 0x8B, 0xE3                           ; 1DC6 mov sp,bx
db 0x8B, 0x1E, 0x50, 0x04               ; 1DC8 mov bx,[0x450]
db 0x4B                                 ; 1DCC dec bx
db 0x89, 0x1E, 0x50, 0x04               ; 1DCD mov [0x450],bx
db 0x8A, 0xC7                           ; 1DD1 mov al,bh
db 0x0A, 0xC3                           ; 1DD3 or al,bl
db 0xA2, 0x4D, 0x04                     ; 1DD5 mov [0x44d],al
db 0x5B                                 ; 1DD8 pop bx
db 0x58                                 ; 1DD9 pop ax
db 0x53                                 ; 1DDA push bx
db 0x24, 0x07                           ; 1DDB and al,0x7
db 0xBB, 0x8C, 0x03                     ; 1DDD mov bx,0x38c
db 0x8A, 0xC8                           ; 1DE0 mov cl,al
db 0xB5, 0x00                           ; 1DE2 mov ch,0x0
db 0x03, 0xD9                           ; 1DE4 add bx,cx
db 0xE8, 0xFC, 0xFC                     ; 1DE6 call 0x1ae5
db 0x5B                                 ; 1DE9 pop bx
db 0xC3                                 ; 1DEA ret
db 0x8B, 0xF2                           ; 1DEB mov si,dx
db 0xAC                                 ; 1DED lodsb
db 0x88, 0x07                           ; 1DEE mov [bx],al
db 0x43                                 ; 1DF0 inc bx
db 0x42                                 ; 1DF1 inc dx
db 0x49                                 ; 1DF2 dec cx
db 0x8A, 0xC5                           ; 1DF3 mov al,ch
db 0x0A, 0xC1                           ; 1DF5 or al,cl
db 0x75, 0xF2                           ; 1DF7 jnz 0x1deb
db 0xC3                                 ; 1DF9 ret
db 0x53                                 ; 1DFA push bx
db 0x8B, 0x1E, 0x2E, 0x00               ; 1DFB mov bx,[0x2e]
db 0x43                                 ; 1DFF inc bx
db 0x0B, 0xDB                           ; 1E00 or bx,bx
db 0x5B                                 ; 1E02 pop bx
db 0x75, 0xF4                           ; 1E03 jnz 0x1df9
db 0xB2, 0x0C                           ; 1E05 mov dl,0xc
db 0xE9, 0xCE, 0xE9                     ; 1E07 jmp 0x7d8
db 0xE8, 0xE7, 0x0F                     ; 1E0A call 0x2df4
db 0xD1                                 ; 1E0D db 0xD1
db 0xB0, 0x80                           ; 1E0E mov al,0x80
db 0xA2, 0x39, 0x03                     ; 1E10 mov [0x339],al
db 0x0A, 0x07                           ; 1E13 or al,[bx]
db 0x8A, 0xC8                           ; 1E15 mov cl,al
db 0xE9, 0x5B, 0x19                     ; 1E17 jmp 0x3775
db 0x3C, 0x7E                           ; 1E1A cmp al,0x7e
db 0x74, 0x03                           ; 1E1C jz 0x1e21
db 0xE9, 0x9D, 0xE9                     ; 1E1E jmp 0x7be
db 0x43                                 ; 1E21 inc bx
db 0x8A, 0x07                           ; 1E22 mov al,[bx]
db 0x43                                 ; 1E24 inc bx
db 0x3C, 0x83                           ; 1E25 cmp al,0x83
db 0x75, 0x03                           ; 1E27 jnz 0x1e2c
db 0xE9, 0xA4, 0x0C                     ; 1E29 jmp 0x2ad0
db 0x3C, 0xA0                           ; 1E2C cmp al,0xa0
db 0x75, 0x03                           ; 1E2E jnz 0x1e33
db 0xE9, 0xAF, 0x37                     ; 1E30 jmp 0x55e2
db 0x3C, 0xA2                           ; 1E33 cmp al,0xa2
db 0x75, 0x03                           ; 1E35 jnz 0x1e3a
db 0xE9, 0xC0, 0x38                     ; 1E37 jmp 0x56fa
db 0xE9, 0x81, 0xE9                     ; 1E3A jmp 0x7be
db 0xE8, 0x75, 0x04                     ; 1E3D call 0x22b5
db 0x87, 0xDA                           ; 1E40 xchg bx,dx
db 0xEC                                 ; 1E42 in al,dx
db 0xE9, 0x39, 0xFD                     ; 1E43 jmp 0x1b7f
db 0xE8, 0x61, 0x04                     ; 1E46 call 0x22aa
db 0x52                                 ; 1E49 push dx
db 0xE8, 0xA7, 0x0F                     ; 1E4A call 0x2df4
db 0x2C, 0xE8                           ; 1E4D sub al,0xe8
db 0xCB                                 ; 1E4F retf
db 0x00, 0x5A, 0xC3                     ; 1E50 add [bp+si-0x3d],bl
db 0xE8, 0xF0, 0xFF                     ; 1E53 call 0x1e46
db 0xEE                                 ; 1E56 out dx,al
db 0xC3                                 ; 1E57 ret
db 0xE8, 0xEB, 0xFF                     ; 1E58 call 0x1e46
db 0x52                                 ; 1E5B push dx
db 0x50                                 ; 1E5C push ax
db 0xB2, 0x00                           ; 1E5D mov dl,0x0
db 0x4B                                 ; 1E5F dec bx
db 0xE8, 0xBA, 0xF0                     ; 1E60 call 0xf1d
db 0x74, 0x07                           ; 1E63 jz 0x1e6c
db 0xE8, 0x8C, 0x0F                     ; 1E65 call 0x2df4
db 0x2C, 0xE8                           ; 1E68 sub al,0xe8
db 0xB0, 0x00                           ; 1E6A mov al,0x0
db 0x58                                 ; 1E6C pop ax
db 0x8A, 0xF0                           ; 1E6D mov dh,al
db 0x5E                                 ; 1E6F pop si
db 0x87, 0xDE                           ; 1E70 xchg bx,si
db 0x56                                 ; 1E72 push si
db 0xA0, 0x5E, 0x00                     ; 1E73 mov al,[0x5e]
db 0x0A, 0xC0                           ; 1E76 or al,al
db 0x75, 0x0B                           ; 1E78 jnz 0x1e85
db 0x87, 0xDA                           ; 1E7A xchg bx,dx
db 0xEC                                 ; 1E7C in al,dx
db 0x87, 0xDA                           ; 1E7D xchg bx,dx
db 0x32, 0xC2                           ; 1E7F xor al,dl
db 0x22, 0xC6                           ; 1E81 and al,dh
db 0x74, 0xEE                           ; 1E83 jz 0x1e73
db 0x5B                                 ; 1E85 pop bx
db 0xC3                                 ; 1E86 ret
db 0xE9, 0x34, 0xE9                     ; 1E87 jmp 0x7be
db 0x3C, 0x23                           ; 1E8A cmp al,0x23
db 0x74, 0x3C                           ; 1E8C jz 0x1eca
db 0xE8, 0x96, 0xF8                     ; 1E8E call 0x1727
db 0xE8, 0x91, 0xFC                     ; 1E91 call 0x1b25
db 0x75, 0x58                           ; 1E94 jnz 0x1eee
db 0xE8, 0x7D, 0x20                     ; 1E96 call 0x3f16
db 0x8A, 0xC6                           ; 1E99 mov al,dh
db 0xB6, 0x00                           ; 1E9B mov dh,0x0
db 0xF6, 0xD0                           ; 1E9D not al
db 0x0A, 0xC0                           ; 1E9F or al,al
db 0x79, 0x03                           ; 1EA1 jns 0x1ea6
db 0xE9, 0xB3, 0xF1                     ; 1EA3 jmp 0x1059
db 0x8A, 0xD0                           ; 1EA6 mov dl,al
db 0x52                                 ; 1EA8 push dx
db 0xE8, 0x48, 0x0F                     ; 1EA9 call 0x2df4
db 0x2C, 0xE8                           ; 1EAC sub al,0xe8
db 0x6C                                 ; 1EAE insb
db 0x00, 0x5A, 0x9F                     ; 1EAF add [bp+si-0x61],bl
db 0x86, 0xC4                           ; 1EB2 xchg al,ah
db 0x50                                 ; 1EB4 push ax
db 0x86, 0xC4                           ; 1EB5 xchg al,ah
db 0x53                                 ; 1EB7 push bx
db 0x52                                 ; 1EB8 push dx
db 0x8A, 0xC2                           ; 1EB9 mov al,dl
db 0x02, 0xC0                           ; 1EBB add al,al
db 0x8A, 0xD0                           ; 1EBD mov dl,al
db 0xB0, 0x14                           ; 1EBF mov al,0x14
db 0x9F                                 ; 1EC1 lahf
db 0x86, 0xC4                           ; 1EC2 xchg al,ah
db 0x50                                 ; 1EC4 push ax
db 0x86, 0xC4                           ; 1EC5 xchg al,ah
db 0xE9, 0x08, 0x20                     ; 1EC7 jmp 0x3ed2
db 0xE8, 0x50, 0xF0                     ; 1ECA call 0xf1d
db 0xE8, 0x4C, 0x00                     ; 1ECD call 0x1f1c
db 0x50                                 ; 1ED0 push ax
db 0xE8, 0x20, 0x0F                     ; 1ED1 call 0x2df4
db 0x2C, 0xE8                           ; 1ED4 sub al,0xe8
db 0x44                                 ; 1ED6 inc sp
db 0x00, 0x58, 0x53                     ; 1ED7 add [bx+si+0x53],bl
db 0x52                                 ; 1EDA push dx
db 0xE8, 0xC7, 0x20                     ; 1EDB call 0x3fa5
db 0xE8, 0x32, 0x3A                     ; 1EDE call 0x5913
db 0x0A, 0xC0                           ; 1EE1 or al,al
db 0x79, 0x03                           ; 1EE3 jns 0x1ee8
db 0xE9, 0x71, 0xF1                     ; 1EE5 jmp 0x1059
db 0x43                                 ; 1EE8 inc bx
db 0x5A                                 ; 1EE9 pop dx
db 0x88, 0x17                           ; 1EEA mov [bx],dl
db 0x5B                                 ; 1EEC pop bx
db 0xC3                                 ; 1EED ret
db 0xE8, 0x2E, 0x00                     ; 1EEE call 0x1f1f
db 0xE8, 0xB8, 0x33                     ; 1EF1 call 0x52ac
db 0xA2, 0x29, 0x00                     ; 1EF4 mov [0x29],al
db 0x2C, 0x0E                           ; 1EF7 sub al,0xe
db 0x73, 0xFC                           ; 1EF9 jnc 0x1ef7
db 0x04, 0x1C                           ; 1EFB add al,0x1c
db 0xF6, 0xD0                           ; 1EFD not al
db 0xFE, 0xC0                           ; 1EFF inc al
db 0x02, 0xC2                           ; 1F01 add al,dl
db 0xA2, 0x2A, 0x00                     ; 1F03 mov [0x2a],al
db 0xC3                                 ; 1F06 ret
db 0xE8, 0x13, 0xF0                     ; 1F07 call 0xf1d
db 0xE8, 0x1A, 0xF8                     ; 1F0A call 0x1727
db 0x53                                 ; 1F0D push bx
db 0xE8, 0x9C, 0x4C                     ; 1F0E call 0x6bad
db 0x87, 0xDA                           ; 1F11 xchg bx,dx
db 0x5B                                 ; 1F13 pop bx
db 0x8A, 0xC6                           ; 1F14 mov al,dh
db 0x0A, 0xC0                           ; 1F16 or al,al
db 0xC3                                 ; 1F18 ret
db 0xE8, 0x01, 0xF0                     ; 1F19 call 0xf1d
db 0xE8, 0x08, 0xF8                     ; 1F1C call 0x1727
db 0xE8, 0xEB, 0xFF                     ; 1F1F call 0x1f0d
db 0x74, 0x03                           ; 1F22 jz 0x1f27
db 0xE9, 0x32, 0xF1                     ; 1F24 jmp 0x1059
db 0x4B                                 ; 1F27 dec bx
db 0xE8, 0xF2, 0xEF                     ; 1F28 call 0xf1d
db 0x8A, 0xC2                           ; 1F2B mov al,dl
db 0xC3                                 ; 1F2D ret
db 0xE8, 0x7F, 0xF5                     ; 1F2E call 0x14b0
db 0x4B                                 ; 1F31 dec bx
db 0xE8, 0xE8, 0xEF                     ; 1F32 call 0xf1d
db 0xCD, 0xA2                           ; 1F35 int 0xa2
db 0x59                                 ; 1F37 pop cx
db 0xE8, 0xF2, 0xEA                     ; 1F38 call 0xa2d
db 0x51                                 ; 1F3B push cx
db 0xE8, 0x19, 0x28                     ; 1F3C call 0x4758
db 0x8B, 0x1E, 0x3B, 0x03               ; 1F3F mov bx,[0x33b]
db 0x4B                                 ; 1F43 dec bx
db 0xE8, 0xD6, 0xEF                     ; 1F44 call 0xf1d
db 0x74, 0x0E                           ; 1F47 jz 0x1f57
db 0xE8, 0xA8, 0x0E                     ; 1F49 call 0x2df4
db 0x2C, 0xE8                           ; 1F4C sub al,0xe8
db 0xC3                                 ; 1F4E ret
db 0x1F                                 ; 1F4F pop ds
db 0xB2, 0x02                           ; 1F50 mov dl,0x2
db 0x32, 0xC0                           ; 1F52 xor al,al
db 0xE8, 0x96, 0x21                     ; 1F54 call 0x40ed
db 0xBB, 0xFF, 0xFF                     ; 1F57 mov bx,0xffff
db 0x89, 0x1E, 0x2E, 0x00               ; 1F5A mov [0x2e],bx
db 0xE8, 0x75, 0x10                     ; 1F5E call 0x2fd6
db 0x75, 0x05                           ; 1F61 jnz 0x1f68
db 0xB0, 0x01                           ; 1F63 mov al,0x1
db 0xA2, 0x6F, 0x00                     ; 1F65 mov [0x6f],al
db 0x5B                                 ; 1F68 pop bx
db 0x5A                                 ; 1F69 pop dx
db 0x8A, 0x0F                           ; 1F6A mov cl,[bx]
db 0x43                                 ; 1F6C inc bx
db 0x8A, 0x2F                           ; 1F6D mov ch,[bx]
db 0x43                                 ; 1F6F inc bx
db 0x8A, 0xC5                           ; 1F70 mov al,ch
db 0x0A, 0xC1                           ; 1F72 or al,cl
db 0x75, 0x03                           ; 1F74 jnz 0x1f79
db 0xE9, 0x3D, 0xE9                     ; 1F76 jmp 0x8b6
db 0xE8, 0x90, 0xE0                     ; 1F79 call 0xc
db 0x75, 0x03                           ; 1F7C jnz 0x1f81
db 0xE8, 0x07, 0x0D                     ; 1F7E call 0x2c88
db 0x51                                 ; 1F81 push cx
db 0x8A, 0x0F                           ; 1F82 mov cl,[bx]
db 0x43                                 ; 1F84 inc bx
db 0x8A, 0x2F                           ; 1F85 mov ch,[bx]
db 0x43                                 ; 1F87 inc bx
db 0x51                                 ; 1F88 push cx
db 0x5E                                 ; 1F89 pop si
db 0x87, 0xDE                           ; 1F8A xchg bx,si
db 0x56                                 ; 1F8C push si
db 0x87, 0xDA                           ; 1F8D xchg bx,dx
db 0x3B, 0xDA                           ; 1F8F cmp bx,dx
db 0x59                                 ; 1F91 pop cx
db 0x73, 0x03                           ; 1F92 jnc 0x1f97
db 0xE9, 0x1E, 0xE9                     ; 1F94 jmp 0x8b5
db 0x5E                                 ; 1F97 pop si
db 0x87, 0xDE                           ; 1F98 xchg bx,si
db 0x56                                 ; 1F9A push si
db 0x53                                 ; 1F9B push bx
db 0x51                                 ; 1F9C push cx
db 0x87, 0xDA                           ; 1F9D xchg bx,dx
db 0x89, 0x1E, 0x49, 0x03               ; 1F9F mov [0x349],bx
db 0xE8, 0xAA, 0x45                     ; 1FA3 call 0x6550
db 0x5B                                 ; 1FA6 pop bx
db 0x8A, 0x07                           ; 1FA7 mov al,[bx]
db 0x3C, 0x09                           ; 1FA9 cmp al,0x9
db 0x74, 0x05                           ; 1FAB jz 0x1fb2
db 0xB0, 0x20                           ; 1FAD mov al,0x20
db 0xE8, 0xF3, 0x0B                     ; 1FAF call 0x2ba5
db 0xE8, 0x18, 0x00                     ; 1FB2 call 0x1fcd
db 0xBB, 0xF7, 0x01                     ; 1FB5 mov bx,0x1f7
db 0xE8, 0x05, 0x00                     ; 1FB8 call 0x1fc0
db 0xE8, 0xB3, 0x0C                     ; 1FBB call 0x2c71
db 0xEB, 0x97                           ; 1FBE jmp short 0x1f57
db 0x8A, 0x07                           ; 1FC0 mov al,[bx]
db 0x0A, 0xC0                           ; 1FC2 or al,al
db 0x75, 0x01                           ; 1FC4 jnz 0x1fc7
db 0xC3                                 ; 1FC6 ret
db 0xE8, 0x69, 0x17                     ; 1FC7 call 0x3733
db 0x43                                 ; 1FCA inc bx
db 0xEB, 0xF3                           ; 1FCB jmp short 0x1fc0
db 0xB9, 0xF7, 0x01                     ; 1FCD mov cx,0x1f7
db 0xB6, 0xFF                           ; 1FD0 mov dh,0xff
db 0x32, 0xC0                           ; 1FD2 xor al,al
db 0xA2, 0xFC, 0x02                     ; 1FD4 mov [0x2fc],al
db 0x32, 0xC0                           ; 1FD7 xor al,al
db 0xA2, 0x5E, 0x04                     ; 1FD9 mov [0x45e],al
db 0xE8, 0x79, 0x27                     ; 1FDC call 0x4758
db 0xEB, 0x06                           ; 1FDF jmp short 0x1fe7
db 0x41                                 ; 1FE1 inc cx
db 0x43                                 ; 1FE2 inc bx
db 0xFE, 0xCE                           ; 1FE3 dec dh
db 0x74, 0xDF                           ; 1FE5 jz 0x1fc6
db 0x8A, 0x07                           ; 1FE7 mov al,[bx]
db 0x0A, 0xC0                           ; 1FE9 or al,al
db 0x8B, 0xF9                           ; 1FEB mov di,cx
db 0xAA                                 ; 1FED stosb
db 0x74, 0xD6                           ; 1FEE jz 0x1fc6
db 0x3C, 0x0B                           ; 1FF0 cmp al,0xb
db 0x72, 0x28                           ; 1FF2 jc 0x201c
db 0x3C, 0x20                           ; 1FF4 cmp al,0x20
db 0x8A, 0xD0                           ; 1FF6 mov dl,al
db 0x72, 0x38                           ; 1FF8 jc 0x2032
db 0x3C, 0x22                           ; 1FFA cmp al,0x22
db 0x75, 0x0A                           ; 1FFC jnz 0x2008
db 0xA0, 0xFC, 0x02                     ; 1FFE mov al,[0x2fc]
db 0x34, 0x01                           ; 2001 xor al,0x1
db 0xA2, 0xFC, 0x02                     ; 2003 mov [0x2fc],al
db 0xB0, 0x22                           ; 2006 mov al,0x22
db 0x3C, 0x3A                           ; 2008 cmp al,0x3a
db 0x75, 0x10                           ; 200A jnz 0x201c
db 0xA0, 0xFC, 0x02                     ; 200C mov al,[0x2fc]
db 0xD0, 0xD8                           ; 200F rcr al,1
db 0x72, 0x07                           ; 2011 jc 0x201a
db 0xD0, 0xD0                           ; 2013 rcl al,1
db 0x24, 0xFD                           ; 2015 and al,0xfd
db 0xA2, 0xFC, 0x02                     ; 2017 mov [0x2fc],al
db 0xB0, 0x3A                           ; 201A mov al,0x3a
db 0x0A, 0xC0                           ; 201C or al,al
db 0x79, 0x03                           ; 201E jns 0x2023
db 0xE9, 0x3C, 0x00                     ; 2020 jmp 0x205f
db 0x8A, 0xD0                           ; 2023 mov dl,al
db 0x3C, 0x2E                           ; 2025 cmp al,0x2e
db 0x74, 0x09                           ; 2027 jz 0x2032
db 0xE8, 0x57, 0x01                     ; 2029 call 0x2183
db 0x73, 0x04                           ; 202C jnc 0x2032
db 0x32, 0xC0                           ; 202E xor al,al
db 0xEB, 0x18                           ; 2030 jmp short 0x204a
db 0xA0, 0x5E, 0x04                     ; 2032 mov al,[0x45e]
db 0x0A, 0xC0                           ; 2035 or al,al
db 0x74, 0x0F                           ; 2037 jz 0x2048
db 0xFE, 0xC0                           ; 2039 inc al
db 0x75, 0x0B                           ; 203B jnz 0x2048
db 0xB0, 0x20                           ; 203D mov al,0x20
db 0x8B, 0xF9                           ; 203F mov di,cx
db 0xAA                                 ; 2041 stosb
db 0x41                                 ; 2042 inc cx
db 0xFE, 0xCE                           ; 2043 dec dh
db 0x75, 0x01                           ; 2045 jnz 0x2048
db 0xC3                                 ; 2047 ret
db 0xB0, 0x01                           ; 2048 mov al,0x1
db 0xA2, 0x5E, 0x04                     ; 204A mov [0x45e],al
db 0x8A, 0xC2                           ; 204D mov al,dl
db 0x3C, 0x0B                           ; 204F cmp al,0xb
db 0x72, 0x07                           ; 2051 jc 0x205a
db 0x3C, 0x20                           ; 2053 cmp al,0x20
db 0x73, 0x03                           ; 2055 jnc 0x205a
db 0xE9, 0x37, 0x01                     ; 2057 jmp 0x2191
db 0x8B, 0xF9                           ; 205A mov di,cx
db 0xAA                                 ; 205C stosb
db 0xEB, 0x82                           ; 205D jmp short 0x1fe1
db 0xA0, 0xFC, 0x02                     ; 205F mov al,[0x2fc]
db 0xD0, 0xD8                           ; 2062 rcr al,1
db 0x72, 0x43                           ; 2064 jc 0x20a9
db 0xD0, 0xD8                           ; 2066 rcr al,1
db 0xD0, 0xD8                           ; 2068 rcr al,1
db 0x73, 0x52                           ; 206A jnc 0x20be
db 0x8A, 0x07                           ; 206C mov al,[bx]
db 0x3C, 0xD9                           ; 206E cmp al,0xd9
db 0x53                                 ; 2070 push bx
db 0x51                                 ; 2071 push cx
db 0xBB, 0xA5, 0x20                     ; 2072 mov bx,0x20a5
db 0x53                                 ; 2075 push bx
db 0x75, 0xCF                           ; 2076 jnz 0x2047
db 0x49                                 ; 2078 dec cx
db 0x8B, 0xF1                           ; 2079 mov si,cx
db 0xAC                                 ; 207B lodsb
db 0x3C, 0x4D                           ; 207C cmp al,0x4d
db 0x75, 0xC7                           ; 207E jnz 0x2047
db 0x49                                 ; 2080 dec cx
db 0x8B, 0xF1                           ; 2081 mov si,cx
db 0xAC                                 ; 2083 lodsb
db 0x3C, 0x45                           ; 2084 cmp al,0x45
db 0x75, 0xBF                           ; 2086 jnz 0x2047
db 0x49                                 ; 2088 dec cx
db 0x8B, 0xF1                           ; 2089 mov si,cx
db 0xAC                                 ; 208B lodsb
db 0x3C, 0x52                           ; 208C cmp al,0x52
db 0x75, 0xB7                           ; 208E jnz 0x2047
db 0x49                                 ; 2090 dec cx
db 0x8B, 0xF1                           ; 2091 mov si,cx
db 0xAC                                 ; 2093 lodsb
db 0x3C, 0x3A                           ; 2094 cmp al,0x3a
db 0x75, 0xAF                           ; 2096 jnz 0x2047
db 0x58                                 ; 2098 pop ax
db 0x58                                 ; 2099 pop ax
db 0x5B                                 ; 209A pop bx
db 0xFE, 0xC6                           ; 209B inc dh
db 0xFE, 0xC6                           ; 209D inc dh
db 0xFE, 0xC6                           ; 209F inc dh
db 0xFE, 0xC6                           ; 20A1 inc dh
db 0xEB, 0x2D                           ; 20A3 jmp short 0x20d2
db 0x59                                 ; 20A5 pop cx
db 0x5B                                 ; 20A6 pop bx
db 0x8A, 0x07                           ; 20A7 mov al,[bx]
db 0xE9, 0x35, 0xFF                     ; 20A9 jmp 0x1fe1
db 0xA0, 0xFC, 0x02                     ; 20AC mov al,[0x2fc]
db 0x0C, 0x02                           ; 20AF or al,0x2
db 0xA2, 0xFC, 0x02                     ; 20B1 mov [0x2fc],al
db 0x32, 0xC0                           ; 20B4 xor al,al
db 0xC3                                 ; 20B6 ret
db 0xA0, 0xFC, 0x02                     ; 20B7 mov al,[0x2fc]
db 0x0C, 0x04                           ; 20BA or al,0x4
db 0xEB, 0xF3                           ; 20BC jmp short 0x20b1
db 0xD0, 0xD0                           ; 20BE rcl al,1
db 0x72, 0xE7                           ; 20C0 jc 0x20a9
db 0x8A, 0x07                           ; 20C2 mov al,[bx]
db 0x3C, 0x84                           ; 20C4 cmp al,0x84
db 0x75, 0x03                           ; 20C6 jnz 0x20cb
db 0xE8, 0xE1, 0xFF                     ; 20C8 call 0x20ac
db 0x3C, 0x8F                           ; 20CB cmp al,0x8f
db 0x75, 0x03                           ; 20CD jnz 0x20d2
db 0xE8, 0xE5, 0xFF                     ; 20CF call 0x20b7
db 0x8A, 0x07                           ; 20D2 mov al,[bx]
db 0xFE, 0xC0                           ; 20D4 inc al
db 0x8A, 0x07                           ; 20D6 mov al,[bx]
db 0x75, 0x05                           ; 20D8 jnz 0x20df
db 0x43                                 ; 20DA inc bx
db 0x8A, 0x07                           ; 20DB mov al,[bx]
db 0x24, 0x7F                           ; 20DD and al,0x7f
db 0x43                                 ; 20DF inc bx
db 0x3C, 0xA1                           ; 20E0 cmp al,0xa1
db 0x75, 0x03                           ; 20E2 jnz 0x20e7
db 0xE8, 0xF8, 0x43                     ; 20E4 call 0x64df
db 0x3C, 0xB1                           ; 20E7 cmp al,0xb1
db 0x75, 0x0A                           ; 20E9 jnz 0x20f5
db 0x8A, 0x07                           ; 20EB mov al,[bx]
db 0x43                                 ; 20ED inc bx
db 0x3C, 0xE9                           ; 20EE cmp al,0xe9
db 0xB0, 0xB1                           ; 20F0 mov al,0xb1
db 0x74, 0x01                           ; 20F2 jz 0x20f5
db 0x4B                                 ; 20F4 dec bx
db 0x53                                 ; 20F5 push bx
db 0x51                                 ; 20F6 push cx
db 0x52                                 ; 20F7 push dx
db 0xCD, 0xA3                           ; 20F8 int 0xa3
db 0xBB, 0x36, 0x01                     ; 20FA mov bx,0x136
db 0x8A, 0xE8                           ; 20FD mov ch,al
db 0xB1, 0x40                           ; 20FF mov cl,0x40
db 0xFE, 0xC1                           ; 2101 inc cl
db 0x43                                 ; 2103 inc bx
db 0x8A, 0xF7                           ; 2104 mov dh,bh
db 0x8A, 0xD3                           ; 2106 mov dl,bl
db 0x2E, 0x8A, 0x07                     ; 2108 mov al,[cs:bx]
db 0x0A, 0xC0                           ; 210B or al,al
db 0x74, 0xF2                           ; 210D jz 0x2101
db 0x9F                                 ; 210F lahf
db 0x43                                 ; 2110 inc bx
db 0x9E                                 ; 2111 sahf
db 0x79, 0xF4                           ; 2112 jns 0x2108
db 0x2E, 0x8A, 0x07                     ; 2114 mov al,[cs:bx]
db 0x3A, 0xC5                           ; 2117 cmp al,ch
db 0x75, 0xE8                           ; 2119 jnz 0x2103
db 0x87, 0xDA                           ; 211B xchg bx,dx
db 0x3C, 0xD0                           ; 211D cmp al,0xd0
db 0x74, 0x02                           ; 211F jz 0x2123
db 0x3C, 0xD1                           ; 2121 cmp al,0xd1
db 0x8A, 0xC1                           ; 2123 mov al,cl
db 0x5A                                 ; 2125 pop dx
db 0x59                                 ; 2126 pop cx
db 0x8A, 0xD0                           ; 2127 mov dl,al
db 0x75, 0x0C                           ; 2129 jnz 0x2137
db 0xA0, 0x5E, 0x04                     ; 212B mov al,[0x45e]
db 0x0A, 0xC0                           ; 212E or al,al
db 0xB0, 0x00                           ; 2130 mov al,0x0
db 0xA2, 0x5E, 0x04                     ; 2132 mov [0x45e],al
db 0xEB, 0x15                           ; 2135 jmp short 0x214c
db 0x3C, 0x5B                           ; 2137 cmp al,0x5b
db 0x75, 0x07                           ; 2139 jnz 0x2142
db 0x32, 0xC0                           ; 213B xor al,al
db 0xA2, 0x5E, 0x04                     ; 213D mov [0x45e],al
db 0xEB, 0x1D                           ; 2140 jmp short 0x215f
db 0xA0, 0x5E, 0x04                     ; 2142 mov al,[0x45e]
db 0x0A, 0xC0                           ; 2145 or al,al
db 0xB0, 0xFF                           ; 2147 mov al,0xff
db 0xA2, 0x5E, 0x04                     ; 2149 mov [0x45e],al
db 0x74, 0x0D                           ; 214C jz 0x215b
db 0xB0, 0x20                           ; 214E mov al,0x20
db 0x8B, 0xF9                           ; 2150 mov di,cx
db 0xAA                                 ; 2152 stosb
db 0x41                                 ; 2153 inc cx
db 0xFE, 0xCE                           ; 2154 dec dh
db 0x75, 0x03                           ; 2156 jnz 0x215b
db 0xE9, 0xA4, 0x05                     ; 2158 jmp 0x26ff
db 0x8A, 0xC2                           ; 215B mov al,dl
db 0xEB, 0x06                           ; 215D jmp short 0x2165
db 0x2E, 0x8A, 0x07                     ; 215F mov al,[cs:bx]
db 0x43                                 ; 2162 inc bx
db 0x8A, 0xD0                           ; 2163 mov dl,al
db 0x24, 0x7F                           ; 2165 and al,0x7f
db 0x8B, 0xF9                           ; 2167 mov di,cx
db 0xAA                                 ; 2169 stosb
db 0x41                                 ; 216A inc cx
db 0xFE, 0xCE                           ; 216B dec dh
db 0x75, 0x03                           ; 216D jnz 0x2172
db 0xE9, 0x8D, 0x05                     ; 216F jmp 0x26ff
db 0x0A, 0xC2                           ; 2172 or al,dl
db 0x79, 0xE9                           ; 2174 jns 0x215f
db 0x3C, 0xA8                           ; 2176 cmp al,0xa8
db 0x75, 0x05                           ; 2178 jnz 0x217f
db 0x32, 0xC0                           ; 217A xor al,al
db 0xA2, 0x5E, 0x04                     ; 217C mov [0x45e],al
db 0x5B                                 ; 217F pop bx
db 0xE9, 0x64, 0xFE                     ; 2180 jmp 0x1fe7
db 0xE8, 0xBF, 0x0D                     ; 2183 call 0x2f45
db 0x72, 0x01                           ; 2186 jc 0x2189
db 0xC3                                 ; 2188 ret
db 0x3C, 0x30                           ; 2189 cmp al,0x30
db 0x72, 0xFB                           ; 218B jc 0x2188
db 0x3C, 0x3A                           ; 218D cmp al,0x3a
db 0xF5                                 ; 218F cmc
db 0xC3                                 ; 2190 ret
db 0x4B                                 ; 2191 dec bx
db 0xE8, 0x88, 0xED                     ; 2192 call 0xf1d
db 0x52                                 ; 2195 push dx
db 0x51                                 ; 2196 push cx
db 0x50                                 ; 2197 push ax
db 0xE8, 0x21, 0xEE                     ; 2198 call 0xfbc
db 0x58                                 ; 219B pop ax
db 0xB9, 0xB5, 0x21                     ; 219C mov cx,0x21b5
db 0x51                                 ; 219F push cx
db 0x3C, 0x0B                           ; 21A0 cmp al,0xb
db 0x75, 0x03                           ; 21A2 jnz 0x21a7
db 0xE9, 0x60, 0x42                     ; 21A4 jmp 0x6407
db 0x3C, 0x0C                           ; 21A7 cmp al,0xc
db 0x75, 0x03                           ; 21A9 jnz 0x21ae
db 0xE9, 0x63, 0x42                     ; 21AB jmp 0x6411
db 0x8B, 0x1E, 0x02, 0x03               ; 21AE mov bx,[0x302]
db 0xE9, 0x13, 0x4F                     ; 21B2 jmp 0x70c8
db 0x59                                 ; 21B5 pop cx
db 0x5A                                 ; 21B6 pop dx
db 0xA0, 0x00, 0x03                     ; 21B7 mov al,[0x300]
db 0xB2, 0x4F                           ; 21BA mov dl,0x4f
db 0x3C, 0x0B                           ; 21BC cmp al,0xb
db 0x74, 0x06                           ; 21BE jz 0x21c6
db 0x3C, 0x0C                           ; 21C0 cmp al,0xc
db 0xB2, 0x48                           ; 21C2 mov dl,0x48
db 0x75, 0x14                           ; 21C4 jnz 0x21da
db 0xB0, 0x26                           ; 21C6 mov al,0x26
db 0x8B, 0xF9                           ; 21C8 mov di,cx
db 0xAA                                 ; 21CA stosb
db 0x41                                 ; 21CB inc cx
db 0xFE, 0xCE                           ; 21CC dec dh
db 0x74, 0xC0                           ; 21CE jz 0x2190
db 0x8A, 0xC2                           ; 21D0 mov al,dl
db 0x8B, 0xF9                           ; 21D2 mov di,cx
db 0xAA                                 ; 21D4 stosb
db 0x41                                 ; 21D5 inc cx
db 0xFE, 0xCE                           ; 21D6 dec dh
db 0x74, 0xB6                           ; 21D8 jz 0x2190
db 0xA0, 0x01, 0x03                     ; 21DA mov al,[0x301]
db 0x3C, 0x04                           ; 21DD cmp al,0x4
db 0xB2, 0x00                           ; 21DF mov dl,0x0
db 0x72, 0x06                           ; 21E1 jc 0x21e9
db 0xB2, 0x21                           ; 21E3 mov dl,0x21
db 0x74, 0x02                           ; 21E5 jz 0x21e9
db 0xB2, 0x23                           ; 21E7 mov dl,0x23
db 0x8A, 0x07                           ; 21E9 mov al,[bx]
db 0x3C, 0x20                           ; 21EB cmp al,0x20
db 0x75, 0x03                           ; 21ED jnz 0x21f2
db 0xE8, 0x52, 0x43                     ; 21EF call 0x6544
db 0x8A, 0x07                           ; 21F2 mov al,[bx]
db 0x43                                 ; 21F4 inc bx
db 0x0A, 0xC0                           ; 21F5 or al,al
db 0x74, 0x2A                           ; 21F7 jz 0x2223
db 0x8B, 0xF9                           ; 21F9 mov di,cx
db 0xAA                                 ; 21FB stosb
db 0x41                                 ; 21FC inc cx
db 0xFE, 0xCE                           ; 21FD dec dh
db 0x74, 0x8F                           ; 21FF jz 0x2190
db 0xA0, 0x01, 0x03                     ; 2201 mov al,[0x301]
db 0x3C, 0x04                           ; 2204 cmp al,0x4
db 0x72, 0xEA                           ; 2206 jc 0x21f2
db 0x9F                                 ; 2208 lahf
db 0x49                                 ; 2209 dec cx
db 0x9E                                 ; 220A sahf
db 0x8B, 0xF1                           ; 220B mov si,cx
db 0xAC                                 ; 220D lodsb
db 0x9F                                 ; 220E lahf
db 0x41                                 ; 220F inc cx
db 0x9E                                 ; 2210 sahf
db 0x75, 0x04                           ; 2211 jnz 0x2217
db 0x3C, 0x2E                           ; 2213 cmp al,0x2e
db 0x74, 0x08                           ; 2215 jz 0x221f
db 0x3C, 0x44                           ; 2217 cmp al,0x44
db 0x74, 0x04                           ; 2219 jz 0x221f
db 0x3C, 0x45                           ; 221B cmp al,0x45
db 0x75, 0xD3                           ; 221D jnz 0x21f2
db 0xB2, 0x00                           ; 221F mov dl,0x0
db 0xEB, 0xCF                           ; 2221 jmp short 0x21f2
db 0x8A, 0xC2                           ; 2223 mov al,dl
db 0x0A, 0xC0                           ; 2225 or al,al
db 0x74, 0x09                           ; 2227 jz 0x2232
db 0x8B, 0xF9                           ; 2229 mov di,cx
db 0xAA                                 ; 222B stosb
db 0x41                                 ; 222C inc cx
db 0xFE, 0xCE                           ; 222D dec dh
db 0x75, 0x01                           ; 222F jnz 0x2232
db 0xC3                                 ; 2231 ret
db 0x8B, 0x1E, 0xFE, 0x02               ; 2232 mov bx,[0x2fe]
db 0xE9, 0xAE, 0xFD                     ; 2236 jmp 0x1fe7
db 0xE8, 0xF1, 0xE7                     ; 2239 call 0xa2d
db 0x51                                 ; 223C push cx
db 0xE8, 0xF5, 0x01                     ; 223D call 0x2435
db 0x59                                 ; 2240 pop cx
db 0x5A                                 ; 2241 pop dx
db 0x51                                 ; 2242 push cx
db 0x51                                 ; 2243 push cx
db 0xE8, 0x20, 0xE8                     ; 2244 call 0xa67
db 0x73, 0x0B                           ; 2247 jnc 0x2254
db 0x8A, 0xF7                           ; 2249 mov dh,bh
db 0x8A, 0xD3                           ; 224B mov dl,bl
db 0x5E                                 ; 224D pop si
db 0x87, 0xDE                           ; 224E xchg bx,si
db 0x56                                 ; 2250 push si
db 0x53                                 ; 2251 push bx
db 0x3B, 0xDA                           ; 2252 cmp bx,dx
db 0x72, 0x03                           ; 2254 jc 0x2259
db 0xE9, 0x00, 0xEE                     ; 2256 jmp 0x1059
db 0xBB, 0x2D, 0x07                     ; 2259 mov bx,0x72d
db 0xE8, 0xF6, 0x58                     ; 225C call 0x7b55
db 0x59                                 ; 225F pop cx
db 0xBB, 0xDD, 0x09                     ; 2260 mov bx,0x9dd
db 0x5E                                 ; 2263 pop si
db 0x87, 0xDE                           ; 2264 xchg bx,si
db 0x56                                 ; 2266 push si
db 0x87, 0xDA                           ; 2267 xchg bx,dx
db 0x8B, 0x1E, 0x58, 0x03               ; 2269 mov bx,[0x358]
db 0x8B, 0xF2                           ; 226D mov si,dx
db 0xAC                                 ; 226F lodsb
db 0x8B, 0xF9                           ; 2270 mov di,cx
db 0xAA                                 ; 2272 stosb
db 0x41                                 ; 2273 inc cx
db 0x42                                 ; 2274 inc dx
db 0x3B, 0xDA                           ; 2275 cmp bx,dx
db 0x75, 0xF4                           ; 2277 jnz 0x226d
db 0x8B, 0xD9                           ; 2279 mov bx,cx
db 0x89, 0x1E, 0x58, 0x03               ; 227B mov [0x358],bx
db 0xC3                                 ; 227F ret
db 0xE8, 0x32, 0x00                     ; 2280 call 0x22b5
db 0xE8, 0xC3, 0x24                     ; 2283 call 0x4749
db 0x1E                                 ; 2286 push ds
db 0x8E, 0x1E, 0x50, 0x03               ; 2287 mov ds,[0x350]
db 0x8A, 0x07                           ; 228B mov al,[bx]
db 0x1F                                 ; 228D pop ds
db 0xE9, 0xEE, 0xF8                     ; 228E jmp 0x1b7f
db 0xE8, 0x16, 0x00                     ; 2291 call 0x22aa
db 0x52                                 ; 2294 push dx
db 0xE8, 0xB1, 0x24                     ; 2295 call 0x4749
db 0xE8, 0x59, 0x0B                     ; 2298 call 0x2df4
db 0x2C, 0xE8                           ; 229B sub al,0xe8
db 0x7D, 0xFC                           ; 229D jnl 0x229b
db 0x5A                                 ; 229F pop dx
db 0x06                                 ; 22A0 push es
db 0x8E, 0x06, 0x50, 0x03               ; 22A1 mov es,[0x350]
db 0x8B, 0xFA                           ; 22A5 mov di,dx
db 0xAA                                 ; 22A7 stosb
db 0x07                                 ; 22A8 pop es
db 0xC3                                 ; 22A9 ret
db 0xE8, 0x7A, 0xF4                     ; 22AA call 0x1727
db 0x53                                 ; 22AD push bx
db 0xE8, 0x04, 0x00                     ; 22AE call 0x22b5
db 0x87, 0xDA                           ; 22B1 xchg bx,dx
db 0x5B                                 ; 22B3 pop bx
db 0xC3                                 ; 22B4 ret
db 0xB9, 0xAD, 0x6B                     ; 22B5 mov cx,0x6bad
db 0x51                                 ; 22B8 push cx
db 0xE8, 0x69, 0xF8                     ; 22B9 call 0x1b25
db 0x78, 0xF6                           ; 22BC js 0x22b4
db 0xCD, 0xA4                           ; 22BE int 0xa4
db 0xA0, 0xA6, 0x04                     ; 22C0 mov al,[0x4a6]
db 0x3C, 0x90                           ; 22C3 cmp al,0x90
db 0x75, 0xED                           ; 22C5 jnz 0x22b4
db 0xE8, 0xAB, 0x58                     ; 22C7 call 0x7b75
db 0x78, 0xE8                           ; 22CA js 0x22b4
db 0xE8, 0x82, 0x48                     ; 22CC call 0x6b51
db 0xB9, 0x80, 0x91                     ; 22CF mov cx,0x9180
db 0xBA, 0x00, 0x00                     ; 22D2 mov dx,0x0
db 0xE9, 0x3A, 0x40                     ; 22D5 jmp 0x6312
db 0xB9, 0x0A, 0x00                     ; 22D8 mov cx,0xa
db 0x51                                 ; 22DB push cx
db 0x8A, 0xF5                           ; 22DC mov dh,ch
db 0x8A, 0xD5                           ; 22DE mov dl,ch
db 0x74, 0x35                           ; 22E0 jz 0x2317
db 0x3C, 0x2C                           ; 22E2 cmp al,0x2c
db 0x74, 0x0B                           ; 22E4 jz 0x22f1
db 0x52                                 ; 22E6 push dx
db 0xE8, 0x74, 0xED                     ; 22E7 call 0x105e
db 0x8A, 0xEE                           ; 22EA mov ch,dh
db 0x8A, 0xCA                           ; 22EC mov cl,dl
db 0x5A                                 ; 22EE pop dx
db 0x74, 0x26                           ; 22EF jz 0x2317
db 0xE8, 0x00, 0x0B                     ; 22F1 call 0x2df4
db 0x2C, 0xE8                           ; 22F4 sub al,0xe8
db 0x66, 0xED                           ; 22F6 in eax,dx
db 0x74, 0x1D                           ; 22F8 jz 0x2317
db 0x58                                 ; 22FA pop ax
db 0xE8, 0xF6, 0x0A                     ; 22FB call 0x2df4
db 0x2C, 0x52                           ; 22FE sub al,0x52
db 0xE8, 0x68, 0xED                     ; 2300 call 0x106b
db 0x74, 0x03                           ; 2303 jz 0x2308
db 0xE9, 0xB6, 0xE4                     ; 2305 jmp 0x7be
db 0x0B, 0xD2                           ; 2308 or dx,dx
db 0x75, 0x03                           ; 230A jnz 0x230f
db 0xE9, 0x4A, 0xED                     ; 230C jmp 0x1059
db 0x87, 0xDA                           ; 230F xchg bx,dx
db 0x5E                                 ; 2311 pop si
db 0x87, 0xDE                           ; 2312 xchg bx,si
db 0x56                                 ; 2314 push si
db 0x87, 0xDA                           ; 2315 xchg bx,dx
db 0x51                                 ; 2317 push cx
db 0xE8, 0x4C, 0xE7                     ; 2318 call 0xa67
db 0x5A                                 ; 231B pop dx
db 0x52                                 ; 231C push dx
db 0x51                                 ; 231D push cx
db 0xE8, 0x46, 0xE7                     ; 231E call 0xa67
db 0x8B, 0xD9                           ; 2321 mov bx,cx
db 0x5A                                 ; 2323 pop dx
db 0x3B, 0xDA                           ; 2324 cmp bx,dx
db 0x87, 0xDA                           ; 2326 xchg bx,dx
db 0x73, 0x03                           ; 2328 jnc 0x232d
db 0xE9, 0x2C, 0xED                     ; 232A jmp 0x1059
db 0x5A                                 ; 232D pop dx
db 0x59                                 ; 232E pop cx
db 0x58                                 ; 232F pop ax
db 0x53                                 ; 2330 push bx
db 0x52                                 ; 2331 push dx
db 0xEB, 0x15                           ; 2332 jmp short 0x2349
db 0x03, 0xD9                           ; 2334 add bx,cx
db 0x73, 0x03                           ; 2336 jnc 0x233b
db 0xE9, 0x1E, 0xED                     ; 2338 jmp 0x1059
db 0x87, 0xDA                           ; 233B xchg bx,dx
db 0x53                                 ; 233D push bx
db 0xBB, 0xF9, 0xFF                     ; 233E mov bx,0xfff9
db 0x3B, 0xDA                           ; 2341 cmp bx,dx
db 0x5B                                 ; 2343 pop bx
db 0x73, 0x03                           ; 2344 jnc 0x2349
db 0xE9, 0x10, 0xED                     ; 2346 jmp 0x1059
db 0x52                                 ; 2349 push dx
db 0x8B, 0x17                           ; 234A mov dx,[bx]
db 0x0B, 0xD2                           ; 234C or dx,dx
db 0x87, 0xDA                           ; 234E xchg bx,dx
db 0x5A                                 ; 2350 pop dx
db 0x74, 0x0C                           ; 2351 jz 0x235f
db 0x8A, 0x07                           ; 2353 mov al,[bx]
db 0x43                                 ; 2355 inc bx
db 0x0A, 0x07                           ; 2356 or al,[bx]
db 0x9F                                 ; 2358 lahf
db 0x4B                                 ; 2359 dec bx
db 0x9E                                 ; 235A sahf
db 0x87, 0xDA                           ; 235B xchg bx,dx
db 0x75, 0xD5                           ; 235D jnz 0x2334
db 0x51                                 ; 235F push cx
db 0xE8, 0x24, 0x00                     ; 2360 call 0x2387
db 0x59                                 ; 2363 pop cx
db 0x5A                                 ; 2364 pop dx
db 0x5B                                 ; 2365 pop bx
db 0x52                                 ; 2366 push dx
db 0x8B, 0x17                           ; 2367 mov dx,[bx]
db 0x43                                 ; 2369 inc bx
db 0x0B, 0xD2                           ; 236A or dx,dx
db 0x74, 0x14                           ; 236C jz 0x2382
db 0x87, 0xDA                           ; 236E xchg bx,dx
db 0x5E                                 ; 2370 pop si
db 0x87, 0xDE                           ; 2371 xchg bx,si
db 0x56                                 ; 2373 push si
db 0x87, 0xDA                           ; 2374 xchg bx,dx
db 0x43                                 ; 2376 inc bx
db 0x89, 0x17                           ; 2377 mov [bx],dx
db 0x87, 0xDA                           ; 2379 xchg bx,dx
db 0x03, 0xD9                           ; 237B add bx,cx
db 0x87, 0xDA                           ; 237D xchg bx,dx
db 0x5B                                 ; 237F pop bx
db 0xEB, 0xE4                           ; 2380 jmp short 0x2366
db 0xB9, 0xB5, 0x08                     ; 2382 mov cx,0x8b5
db 0x51                                 ; 2385 push cx
db 0x3C, 0x0D                           ; 2386 cmp al,0xd
db 0x32, 0xC0                           ; 2388 xor al,al
db 0xA2, 0x3D, 0x03                     ; 238A mov [0x33d],al
db 0x8B, 0x1E, 0x30, 0x00               ; 238D mov bx,[0x30]
db 0x4B                                 ; 2391 dec bx
db 0x43                                 ; 2392 inc bx
db 0x8A, 0x07                           ; 2393 mov al,[bx]
db 0x43                                 ; 2395 inc bx
db 0x0A, 0x07                           ; 2396 or al,[bx]
db 0x75, 0x01                           ; 2398 jnz 0x239b
db 0xC3                                 ; 239A ret
db 0x43                                 ; 239B inc bx
db 0x8B, 0x17                           ; 239C mov dx,[bx]
db 0x43                                 ; 239E inc bx
db 0xE8, 0x7B, 0xEB                     ; 239F call 0xf1d
db 0x0A, 0xC0                           ; 23A2 or al,al
db 0x74, 0xEC                           ; 23A4 jz 0x2392
db 0x8A, 0xC8                           ; 23A6 mov cl,al
db 0xA0, 0x3D, 0x03                     ; 23A8 mov al,[0x33d]
db 0x0A, 0xC0                           ; 23AB or al,al
db 0x8A, 0xC1                           ; 23AD mov al,cl
db 0x74, 0x5B                           ; 23AF jz 0x240c
db 0xCD, 0xA5                           ; 23B1 int 0xa5
db 0x3C, 0xA7                           ; 23B3 cmp al,0xa7
db 0x75, 0x18                           ; 23B5 jnz 0x23cf
db 0xE8, 0x63, 0xEB                     ; 23B7 call 0xf1d
db 0x3C, 0x89                           ; 23BA cmp al,0x89
db 0x75, 0xE4                           ; 23BC jnz 0x23a2
db 0xE8, 0x5C, 0xEB                     ; 23BE call 0xf1d
db 0x3C, 0x0E                           ; 23C1 cmp al,0xe
db 0x75, 0xDD                           ; 23C3 jnz 0x23a2
db 0x52                                 ; 23C5 push dx
db 0xE8, 0xAC, 0xEC                     ; 23C6 call 0x1075
db 0x0B, 0xD2                           ; 23C9 or dx,dx
db 0x75, 0x0A                           ; 23CB jnz 0x23d7
db 0xEB, 0x29                           ; 23CD jmp short 0x23f8
db 0x3C, 0x0E                           ; 23CF cmp al,0xe
db 0x75, 0xCC                           ; 23D1 jnz 0x239f
db 0x52                                 ; 23D3 push dx
db 0xE8, 0x9E, 0xEC                     ; 23D4 call 0x1075
db 0x53                                 ; 23D7 push bx
db 0xE8, 0x8C, 0xE6                     ; 23D8 call 0xa67
db 0x9F                                 ; 23DB lahf
db 0x49                                 ; 23DC dec cx
db 0x9E                                 ; 23DD sahf
db 0xB0, 0x0D                           ; 23DE mov al,0xd
db 0x72, 0x3F                           ; 23E0 jc 0x2421
db 0xE8, 0x78, 0x08                     ; 23E2 call 0x2c5d
db 0xBB, 0xFC, 0x23                     ; 23E5 mov bx,0x23fc
db 0x52                                 ; 23E8 push dx
db 0xE8, 0x69, 0x57                     ; 23E9 call 0x7b55
db 0x5B                                 ; 23EC pop bx
db 0xE8, 0x60, 0x41                     ; 23ED call 0x6550
db 0x59                                 ; 23F0 pop cx
db 0x5B                                 ; 23F1 pop bx
db 0x53                                 ; 23F2 push bx
db 0x51                                 ; 23F3 push cx
db 0xE8, 0x51, 0x41                     ; 23F4 call 0x6548
db 0x5B                                 ; 23F7 pop bx
db 0x5A                                 ; 23F8 pop dx
db 0x4B                                 ; 23F9 dec bx
db 0xEB, 0xA3                           ; 23FA jmp short 0x239f
db 0x55                                 ; 23FC push bp
db 0x6E                                 ; 23FD outsb
db 0x64, 0x65, 0x66, 0x69, 0x6E, 0x65, 0x64
db 0x20, 0x6C, 0x69
db 0x6E                                 ; 2408 outsb
db 0x65, 0x20, 0x00                     ; 2409 and [gs:bx+si],al
db 0x3C, 0x0D                           ; 240C cmp al,0xd
db 0x75, 0xEA                           ; 240E jnz 0x23fa
db 0x52                                 ; 2410 push dx
db 0xE8, 0x61, 0xEC                     ; 2411 call 0x1075
db 0x53                                 ; 2414 push bx
db 0x87, 0xDA                           ; 2415 xchg bx,dx
db 0x43                                 ; 2417 inc bx
db 0x43                                 ; 2418 inc bx
db 0x43                                 ; 2419 inc bx
db 0x8A, 0x0F                           ; 241A mov cl,[bx]
db 0x43                                 ; 241C inc bx
db 0x8A, 0x2F                           ; 241D mov ch,[bx]
db 0xB0, 0x0E                           ; 241F mov al,0xe
db 0xBB, 0xF7, 0x23                     ; 2421 mov bx,0x23f7
db 0x53                                 ; 2424 push bx
db 0x8B, 0x1E, 0xFE, 0x02               ; 2425 mov bx,[0x2fe]
db 0x53                                 ; 2429 push bx
db 0x4B                                 ; 242A dec bx
db 0x88, 0x2F                           ; 242B mov [bx],ch
db 0x4B                                 ; 242D dec bx
db 0x88, 0x0F                           ; 242E mov [bx],cl
db 0x4B                                 ; 2430 dec bx
db 0x88, 0x07                           ; 2431 mov [bx],al
db 0x5B                                 ; 2433 pop bx
db 0xC3                                 ; 2434 ret
db 0xA0, 0x3D, 0x03                     ; 2435 mov al,[0x33d]
db 0x0A, 0xC0                           ; 2438 or al,al
db 0x74, 0xF8                           ; 243A jz 0x2434
db 0xE9, 0x49, 0xFF                     ; 243C jmp 0x2388
db 0xE8, 0xB2, 0x09                     ; 243F call 0x2df4
db 0x42                                 ; 2442 inc dx
db 0xE8, 0xAE, 0x09                     ; 2443 call 0x2df4
db 0x41                                 ; 2446 inc cx
db 0xE8, 0xAA, 0x09                     ; 2447 call 0x2df4
db 0x53                                 ; 244A push bx
db 0xE8, 0xA6, 0x09                     ; 244B call 0x2df4
db 0x45                                 ; 244E inc bp
db 0xA0, 0x5D, 0x04                     ; 244F mov al,[0x45d]
db 0x0A, 0xC0                           ; 2452 or al,al
db 0x74, 0x03                           ; 2454 jz 0x2459
db 0xE9, 0x6E, 0xE3                     ; 2456 jmp 0x7c7
db 0x53                                 ; 2459 push bx
db 0x8B, 0x1E, 0x5A, 0x03               ; 245A mov bx,[0x35a]
db 0x87, 0xDA                           ; 245E xchg bx,dx
db 0x8B, 0x1E, 0x5C, 0x03               ; 2460 mov bx,[0x35c]
db 0x3B, 0xDA                           ; 2464 cmp bx,dx
db 0x74, 0x03                           ; 2466 jz 0x246b
db 0xE9, 0x5C, 0xE3                     ; 2468 jmp 0x7c7
db 0x5B                                 ; 246B pop bx
db 0x8A, 0x07                           ; 246C mov al,[bx]
db 0x2C, 0x30                           ; 246E sub al,0x30
db 0x73, 0x03                           ; 2470 jnc 0x2475
db 0xE9, 0x49, 0xE3                     ; 2472 jmp 0x7be
db 0x3C, 0x02                           ; 2475 cmp al,0x2
db 0x72, 0x03                           ; 2477 jc 0x247c
db 0xE9, 0x42, 0xE3                     ; 2479 jmp 0x7be
db 0xA2, 0x5C, 0x04                     ; 247C mov [0x45c],al
db 0xFE, 0xC0                           ; 247F inc al
db 0xA2, 0x5D, 0x04                     ; 2481 mov [0x45d],al
db 0xE8, 0x96, 0xEA                     ; 2484 call 0xf1d
db 0xC3                                 ; 2487 ret
db 0x2E, 0x8A, 0x07                     ; 2488 mov al,[cs:bx]
db 0x0A, 0xC0                           ; 248B or al,al
db 0x74, 0xF8                           ; 248D jz 0x2487
db 0xE8, 0x03, 0x00                     ; 248F call 0x2495
db 0x43                                 ; 2492 inc bx
db 0xEB, 0xF3                           ; 2493 jmp short 0x2488
db 0x9F                                 ; 2495 lahf
db 0x86, 0xC4                           ; 2496 xchg al,ah
db 0x50                                 ; 2498 push ax
db 0x86, 0xC4                           ; 2499 xchg al,ah
db 0xE9, 0x19, 0x07                     ; 249B jmp 0x2bb7
db 0x74, 0x09                           ; 249E jz 0x24a9
db 0xE8, 0x84, 0xF2                     ; 24A0 call 0x1727
db 0x53                                 ; 24A3 push bx
db 0xE8, 0x06, 0x47                     ; 24A4 call 0x6bad
db 0xEB, 0x20                           ; 24A7 jmp short 0x24c9
db 0x53                                 ; 24A9 push bx
db 0xBB, 0xD2, 0x24                     ; 24AA mov bx,0x24d2
db 0xE8, 0xA5, 0x56                     ; 24AD call 0x7b55
db 0xE8, 0xE1, 0x0C                     ; 24B0 call 0x3194
db 0x5A                                 ; 24B3 pop dx
db 0x73, 0x03                           ; 24B4 jnc 0x24b9
db 0xE9, 0x8F, 0x09                     ; 24B6 jmp 0x2e48
db 0x52                                 ; 24B9 push dx
db 0x43                                 ; 24BA inc bx
db 0x8A, 0x07                           ; 24BB mov al,[bx]
db 0xE8, 0x00, 0x45                     ; 24BD call 0x69c0
db 0x8A, 0x07                           ; 24C0 mov al,[bx]
db 0x0A, 0xC0                           ; 24C2 or al,al
db 0x75, 0xE4                           ; 24C4 jnz 0x24aa
db 0xE8, 0xE4, 0x46                     ; 24C6 call 0x6bad
db 0x89, 0x1E, 0x0C, 0x00               ; 24C9 mov [0xc],bx
db 0xE8, 0x9E, 0x3F                     ; 24CD call 0x646e
db 0x5B                                 ; 24D0 pop bx
db 0xC3                                 ; 24D1 ret
db 0x52                                 ; 24D2 push dx
db 0x61                                 ; 24D3 popa
db 0x6E                                 ; 24D4 outsb
db 0x64, 0x6F                           ; 24D5 fs outsw
db 0x6D                                 ; 24D7 insw
db 0x20, 0x6E, 0x75                     ; 24D8 and [bp+0x75],ch
db 0x6D                                 ; 24DB insw
db 0x62, 0x65, 0x72                     ; 24DC bound sp,[di+0x72]
db 0x20, 0x73, 0x65                     ; 24DF and [bp+di+0x65],dh
db 0x65, 0x64, 0x20, 0x28               ; 24E2 and [fs:bx+si],ch
db 0x2D, 0x33, 0x32                     ; 24E6 sub ax,0x3233
db 0x37                                 ; 24E9 aaa
db 0x36, 0x38, 0x20                     ; 24EA cmp [ss:bx+si],ah
db 0x74, 0x6F                           ; 24ED jz 0x255e
db 0x20, 0x33                           ; 24EF and [bp+di],dh
db 0x32, 0x37                           ; 24F1 xor dh,[bx]
db 0x36, 0x37                           ; 24F3 ss aaa
db 0x29, 0x00                           ; 24F5 sub [bx+si],ax
db 0xB1, 0x1D                           ; 24F7 mov cl,0x1d
db 0xEB, 0x02                           ; 24F9 jmp short 0x24fd
db 0xB1, 0x1A                           ; 24FB mov cl,0x1a
db 0xB5, 0x00                           ; 24FD mov ch,0x0
db 0x87, 0xDA                           ; 24FF xchg bx,dx
db 0x8B, 0x1E, 0x2E, 0x00               ; 2501 mov bx,[0x2e]
db 0x89, 0x1E, 0x5A, 0x04               ; 2505 mov [0x45a],bx
db 0x87, 0xDA                           ; 2509 xchg bx,dx
db 0xFE, 0xC5                           ; 250B inc ch
db 0x4B                                 ; 250D dec bx
db 0xE8, 0x0C, 0xEA                     ; 250E call 0xf1d
db 0x74, 0x17                           ; 2511 jz 0x252a
db 0x3C, 0x22                           ; 2513 cmp al,0x22
db 0x75, 0x0B                           ; 2515 jnz 0x2522
db 0xE8, 0x03, 0xEA                     ; 2517 call 0xf1d
db 0x0A, 0xC0                           ; 251A or al,al
db 0x74, 0x0C                           ; 251C jz 0x252a
db 0x3C, 0x22                           ; 251E cmp al,0x22
db 0x75, 0xF5                           ; 2520 jnz 0x2517
db 0x3C, 0xA1                           ; 2522 cmp al,0xa1
db 0x74, 0x1D                           ; 2524 jz 0x2543
db 0x3C, 0xCD                           ; 2526 cmp al,0xcd
db 0x75, 0xE4                           ; 2528 jnz 0x250e
db 0x0A, 0xC0                           ; 252A or al,al
db 0x75, 0x15                           ; 252C jnz 0x2543
db 0x43                                 ; 252E inc bx
db 0x8A, 0x07                           ; 252F mov al,[bx]
db 0x43                                 ; 2531 inc bx
db 0x0A, 0x07                           ; 2532 or al,[bx]
db 0x8A, 0xD1                           ; 2534 mov dl,cl
db 0x75, 0x03                           ; 2536 jnz 0x253b
db 0xE9, 0x9D, 0xE2                     ; 2538 jmp 0x7d8
db 0x43                                 ; 253B inc bx
db 0x8B, 0x17                           ; 253C mov dx,[bx]
db 0x43                                 ; 253E inc bx
db 0x89, 0x16, 0x5A, 0x04               ; 253F mov [0x45a],dx
db 0xE8, 0xD7, 0xE9                     ; 2543 call 0xf1d
db 0x3C, 0x8F                           ; 2546 cmp al,0x8f
db 0x75, 0x07                           ; 2548 jnz 0x2551
db 0x51                                 ; 254A push cx
db 0xE8, 0x11, 0xEC                     ; 254B call 0x115f
db 0x59                                 ; 254E pop cx
db 0xEB, 0xD9                           ; 254F jmp short 0x252a
db 0x3C, 0x84                           ; 2551 cmp al,0x84
db 0x75, 0x07                           ; 2553 jnz 0x255c
db 0x51                                 ; 2555 push cx
db 0xE8, 0x02, 0xEC                     ; 2556 call 0x115b
db 0x59                                 ; 2559 pop cx
db 0xEB, 0xCE                           ; 255A jmp short 0x252a
db 0x8A, 0xC1                           ; 255C mov al,cl
db 0x3C, 0x1A                           ; 255E cmp al,0x1a
db 0x8A, 0x07                           ; 2560 mov al,[bx]
db 0x74, 0x0D                           ; 2562 jz 0x2571
db 0x3C, 0xB1                           ; 2564 cmp al,0xb1
db 0x74, 0xA3                           ; 2566 jz 0x250b
db 0x3C, 0xB2                           ; 2568 cmp al,0xb2
db 0x75, 0xA1                           ; 256A jnz 0x250d
db 0xFE, 0xCD                           ; 256C dec ch
db 0x75, 0x9D                           ; 256E jnz 0x250d
db 0xC3                                 ; 2570 ret
db 0x3C, 0x82                           ; 2571 cmp al,0x82
db 0x74, 0x96                           ; 2573 jz 0x250b
db 0x3C, 0x83                           ; 2575 cmp al,0x83
db 0x75, 0x94                           ; 2577 jnz 0x250d
db 0xFE, 0xCD                           ; 2579 dec ch
db 0x74, 0xF3                           ; 257B jz 0x2570
db 0xE8, 0x9D, 0xE9                     ; 257D call 0xf1d
db 0x74, 0xA8                           ; 2580 jz 0x252a
db 0x87, 0xDA                           ; 2582 xchg bx,dx
db 0x8B, 0x1E, 0x2E, 0x00               ; 2584 mov bx,[0x2e]
db 0x53                                 ; 2588 push bx
db 0x8B, 0x1E, 0x5A, 0x04               ; 2589 mov bx,[0x45a]
db 0x89, 0x1E, 0x2E, 0x00               ; 258D mov [0x2e],bx
db 0x87, 0xDA                           ; 2591 xchg bx,dx
db 0x51                                 ; 2593 push cx
db 0xE8, 0xD7, 0x11                     ; 2594 call 0x376e
db 0x59                                 ; 2597 pop cx
db 0x4B                                 ; 2598 dec bx
db 0xE8, 0x81, 0xE9                     ; 2599 call 0xf1d
db 0xBA, 0x2A, 0x25                     ; 259C mov dx,0x252a
db 0x74, 0x08                           ; 259F jz 0x25a9
db 0xE8, 0x50, 0x08                     ; 25A1 call 0x2df4
db 0x2C, 0x4B                           ; 25A4 sub al,0x4b
db 0xBA, 0x79, 0x25                     ; 25A6 mov dx,0x2579
db 0x5E                                 ; 25A9 pop si
db 0x87, 0xDE                           ; 25AA xchg bx,si
db 0x56                                 ; 25AC push si
db 0x89, 0x1E, 0x2E, 0x00               ; 25AD mov [0x2e],bx
db 0x5B                                 ; 25B1 pop bx
db 0x52                                 ; 25B2 push dx
db 0xC3                                 ; 25B3 ret
db 0x9F                                 ; 25B4 lahf
db 0x50                                 ; 25B5 push ax
db 0xA0, 0xA8, 0x04                     ; 25B6 mov al,[0x4a8]
db 0xA2, 0xA9, 0x04                     ; 25B9 mov [0x4a9],al
db 0x58                                 ; 25BC pop ax
db 0x9E                                 ; 25BD sahf
db 0x9F                                 ; 25BE lahf
db 0x50                                 ; 25BF push ax
db 0x32, 0xC0                           ; 25C0 xor al,al
db 0xA2, 0xA8, 0x04                     ; 25C2 mov [0x4a8],al
db 0x58                                 ; 25C5 pop ax
db 0x9E                                 ; 25C6 sahf
db 0xC3                                 ; 25C7 ret
db 0xE8, 0xDB, 0x02                     ; 25C8 call 0x28a6
db 0x8A, 0x07                           ; 25CB mov al,[bx]
db 0x43                                 ; 25CD inc bx
db 0x8A, 0x0F                           ; 25CE mov cl,[bx]
db 0x43                                 ; 25D0 inc bx
db 0x8A, 0x2F                           ; 25D1 mov ch,[bx]
db 0x5A                                 ; 25D3 pop dx
db 0x51                                 ; 25D4 push cx
db 0x50                                 ; 25D5 push ax
db 0xE8, 0xD6, 0x02                     ; 25D6 call 0x28af
db 0x58                                 ; 25D9 pop ax
db 0x8A, 0xF0                           ; 25DA mov dh,al
db 0x8A, 0x17                           ; 25DC mov dl,[bx]
db 0x43                                 ; 25DE inc bx
db 0x8A, 0x0F                           ; 25DF mov cl,[bx]
db 0x43                                 ; 25E1 inc bx
db 0x8A, 0x2F                           ; 25E2 mov ch,[bx]
db 0x5B                                 ; 25E4 pop bx
db 0x8A, 0xC2                           ; 25E5 mov al,dl
db 0x0A, 0xC6                           ; 25E7 or al,dh
db 0x75, 0x01                           ; 25E9 jnz 0x25ec
db 0xC3                                 ; 25EB ret
db 0x8A, 0xC6                           ; 25EC mov al,dh
db 0x2C, 0x01                           ; 25EE sub al,0x1
db 0x72, 0xF9                           ; 25F0 jc 0x25eb
db 0x32, 0xC0                           ; 25F2 xor al,al
db 0x3A, 0xC2                           ; 25F4 cmp al,dl
db 0xFE, 0xC0                           ; 25F6 inc al
db 0x73, 0xF1                           ; 25F8 jnc 0x25eb
db 0xFE, 0xCE                           ; 25FA dec dh
db 0xFE, 0xCA                           ; 25FC dec dl
db 0x8B, 0xF1                           ; 25FE mov si,cx
db 0xAC                                 ; 2600 lodsb
db 0x41                                 ; 2601 inc cx
db 0x3A, 0x07                           ; 2602 cmp al,[bx]
db 0x9F                                 ; 2604 lahf
db 0x43                                 ; 2605 inc bx
db 0x9E                                 ; 2606 sahf
db 0x74, 0xDC                           ; 2607 jz 0x25e5
db 0xF5                                 ; 2609 cmc
db 0xE9, 0x72, 0x3F                     ; 260A jmp 0x657f
db 0xE8, 0xF7, 0x3D                     ; 260D call 0x6407
db 0xEB, 0x08                           ; 2610 jmp short 0x261a
db 0xE8, 0xFC, 0x3D                     ; 2612 call 0x6411
db 0xEB, 0x03                           ; 2615 jmp short 0x261a
db 0xE8, 0xAE, 0x4A                     ; 2617 call 0x70c8
db 0xE8, 0x2F, 0x00                     ; 261A call 0x264c
db 0xE8, 0x89, 0x02                     ; 261D call 0x28a9
db 0xB9, 0x17, 0x29                     ; 2620 mov cx,0x2917
db 0x51                                 ; 2623 push cx
db 0x8A, 0x07                           ; 2624 mov al,[bx]
db 0x43                                 ; 2626 inc bx
db 0x53                                 ; 2627 push bx
db 0xE8, 0xAB, 0x00                     ; 2628 call 0x26d6
db 0x5B                                 ; 262B pop bx
db 0x8A, 0x0F                           ; 262C mov cl,[bx]
db 0x43                                 ; 262E inc bx
db 0x8A, 0x2F                           ; 262F mov ch,[bx]
db 0xE8, 0x0D, 0x00                     ; 2631 call 0x2641
db 0x53                                 ; 2634 push bx
db 0x8A, 0xD8                           ; 2635 mov bl,al
db 0xE8, 0x5B, 0x02                     ; 2637 call 0x2895
db 0x5A                                 ; 263A pop dx
db 0xC3                                 ; 263B ret
db 0xB0, 0x01                           ; 263C mov al,0x1
db 0xE8, 0x95, 0x00                     ; 263E call 0x26d6
db 0xBB, 0x2C, 0x03                     ; 2641 mov bx,0x32c
db 0x53                                 ; 2644 push bx
db 0x88, 0x07                           ; 2645 mov [bx],al
db 0x43                                 ; 2647 inc bx
db 0x89, 0x17                           ; 2648 mov [bx],dx
db 0x5B                                 ; 264A pop bx
db 0xC3                                 ; 264B ret
db 0x4B                                 ; 264C dec bx
db 0xB5, 0x22                           ; 264D mov ch,0x22
db 0x8A, 0xF5                           ; 264F mov dh,ch
db 0x53                                 ; 2651 push bx
db 0xB1, 0xFF                           ; 2652 mov cl,0xff
db 0x43                                 ; 2654 inc bx
db 0x8A, 0x07                           ; 2655 mov al,[bx]
db 0xFE, 0xC1                           ; 2657 inc cl
db 0x0A, 0xC0                           ; 2659 or al,al
db 0x74, 0x08                           ; 265B jz 0x2665
db 0x3A, 0xC6                           ; 265D cmp al,dh
db 0x74, 0x04                           ; 265F jz 0x2665
db 0x3A, 0xC5                           ; 2661 cmp al,ch
db 0x75, 0xEF                           ; 2663 jnz 0x2654
db 0x3C, 0x22                           ; 2665 cmp al,0x22
db 0x75, 0x03                           ; 2667 jnz 0x266c
db 0xE8, 0xB1, 0xE8                     ; 2669 call 0xf1d
db 0x53                                 ; 266C push bx
db 0x8A, 0xC5                           ; 266D mov al,ch
db 0x3C, 0x2C                           ; 266F cmp al,0x2c
db 0x75, 0x0D                           ; 2671 jnz 0x2680
db 0xFE, 0xC1                           ; 2673 inc cl
db 0xFE, 0xC9                           ; 2675 dec cl
db 0x74, 0x07                           ; 2677 jz 0x2680
db 0x4B                                 ; 2679 dec bx
db 0x8A, 0x07                           ; 267A mov al,[bx]
db 0x3C, 0x20                           ; 267C cmp al,0x20
db 0x74, 0xF5                           ; 267E jz 0x2675
db 0x5B                                 ; 2680 pop bx
db 0x5E                                 ; 2681 pop si
db 0x87, 0xDE                           ; 2682 xchg bx,si
db 0x56                                 ; 2684 push si
db 0x43                                 ; 2685 inc bx
db 0x87, 0xDA                           ; 2686 xchg bx,dx
db 0x8A, 0xC1                           ; 2688 mov al,cl
db 0xE8, 0xB4, 0xFF                     ; 268A call 0x2641
db 0xBA, 0x2C, 0x03                     ; 268D mov dx,0x32c
db 0xB0, 0x52                           ; 2690 mov al,0x52
db 0x8B, 0x1E, 0x0C, 0x03               ; 2692 mov bx,[0x30c]
db 0x89, 0x1E, 0xA3, 0x04               ; 2696 mov [0x4a3],bx
db 0xB0, 0x03                           ; 269A mov al,0x3
db 0xA2, 0xFB, 0x02                     ; 269C mov [0x2fb],al
db 0xE8, 0x17, 0x3E                     ; 269F call 0x64b9
db 0xBA, 0x2F, 0x03                     ; 26A2 mov dx,0x32f
db 0x3B, 0xDA                           ; 26A5 cmp bx,dx
db 0x89, 0x1E, 0x0C, 0x03               ; 26A7 mov [0x30c],bx
db 0x5B                                 ; 26AB pop bx
db 0x8A, 0x07                           ; 26AC mov al,[bx]
db 0x75, 0x9B                           ; 26AE jnz 0x264b
db 0xBA, 0x10, 0x00                     ; 26B0 mov dx,0x10
db 0xE9, 0x22, 0xE1                     ; 26B3 jmp 0x7d8
db 0x43                                 ; 26B6 inc bx
db 0xE8, 0x92, 0xFF                     ; 26B7 call 0x264c
db 0xE8, 0xEC, 0x01                     ; 26BA call 0x28a9
db 0xE8, 0x7C, 0x3E                     ; 26BD call 0x653c
db 0xFE, 0xC6                           ; 26C0 inc dh
db 0xFE, 0xCE                           ; 26C2 dec dh
db 0x74, 0x85                           ; 26C4 jz 0x264b
db 0x8B, 0xF1                           ; 26C6 mov si,cx
db 0xAC                                 ; 26C8 lodsb
db 0xE8, 0xD9, 0x04                     ; 26C9 call 0x2ba5
db 0x3C, 0x0D                           ; 26CC cmp al,0xd
db 0x75, 0x03                           ; 26CE jnz 0x26d3
db 0xE8, 0xA5, 0x05                     ; 26D0 call 0x2c78
db 0x41                                 ; 26D3 inc cx
db 0xEB, 0xEC                           ; 26D4 jmp short 0x26c2
db 0x0A, 0xC0                           ; 26D6 or al,al
db 0xEB, 0x02                           ; 26D8 jmp short 0x26dc
db 0x58                                 ; 26DA pop ax
db 0x9E                                 ; 26DB sahf
db 0x9F                                 ; 26DC lahf
db 0x50                                 ; 26DD push ax
db 0x8B, 0x1E, 0x5C, 0x03               ; 26DE mov bx,[0x35c]
db 0x87, 0xDA                           ; 26E2 xchg bx,dx
db 0x8B, 0x1E, 0x2F, 0x03               ; 26E4 mov bx,[0x32f]
db 0xF6, 0xD0                           ; 26E8 not al
db 0x8A, 0xC8                           ; 26EA mov cl,al
db 0xB5, 0xFF                           ; 26EC mov ch,0xff
db 0x03, 0xD9                           ; 26EE add bx,cx
db 0x43                                 ; 26F0 inc bx
db 0x3B, 0xDA                           ; 26F1 cmp bx,dx
db 0x72, 0x0F                           ; 26F3 jc 0x2704
db 0x89, 0x1E, 0x2F, 0x03               ; 26F5 mov [0x32f],bx
db 0x43                                 ; 26F9 inc bx
db 0x87, 0xDA                           ; 26FA xchg bx,dx
db 0x58                                 ; 26FC pop ax
db 0x9E                                 ; 26FD sahf
db 0xC3                                 ; 26FE ret
db 0x58                                 ; 26FF pop ax
db 0x86, 0xC4                           ; 2700 xchg al,ah
db 0x9E                                 ; 2702 sahf
db 0xC3                                 ; 2703 ret
db 0x58                                 ; 2704 pop ax
db 0x9E                                 ; 2705 sahf
db 0xBA, 0x0E, 0x00                     ; 2706 mov dx,0xe
db 0x75, 0x03                           ; 2709 jnz 0x270e
db 0xE9, 0xCA, 0xE0                     ; 270B jmp 0x7d8
db 0x3A, 0xC0                           ; 270E cmp al,al
db 0x9F                                 ; 2710 lahf
db 0x50                                 ; 2711 push ax
db 0xB9, 0xDA, 0x26                     ; 2712 mov cx,0x26da
db 0x51                                 ; 2715 push cx
db 0x8B, 0x1E, 0x0A, 0x03               ; 2716 mov bx,[0x30a]
db 0x89, 0x1E, 0x2F, 0x03               ; 271A mov [0x32f],bx
db 0xBB, 0x00, 0x00                     ; 271E mov bx,0x0
db 0x53                                 ; 2721 push bx
db 0x8B, 0x1E, 0x5C, 0x03               ; 2722 mov bx,[0x35c]
db 0x53                                 ; 2726 push bx
db 0xBB, 0x0E, 0x03                     ; 2727 mov bx,0x30e
db 0x8B, 0x16, 0x0C, 0x03               ; 272A mov dx,[0x30c]
db 0x3B, 0xDA                           ; 272E cmp bx,dx
db 0xB9, 0x2A, 0x27                     ; 2730 mov cx,0x272a
db 0x74, 0x03                           ; 2733 jz 0x2738
db 0xE9, 0x9C, 0x00                     ; 2735 jmp 0x27d4
db 0xBB, 0xE2, 0x03                     ; 2738 mov bx,0x3e2
db 0x89, 0x1E, 0x4E, 0x04               ; 273B mov [0x44e],bx
db 0x8B, 0x1E, 0x5A, 0x03               ; 273F mov bx,[0x35a]
db 0x89, 0x1E, 0x4B, 0x04               ; 2743 mov [0x44b],bx
db 0x8B, 0x1E, 0x58, 0x03               ; 2747 mov bx,[0x358]
db 0x8B, 0x16, 0x4B, 0x04               ; 274B mov dx,[0x44b]
db 0x3B, 0xDA                           ; 274F cmp bx,dx
db 0x74, 0x1B                           ; 2751 jz 0x276e
db 0x8A, 0x07                           ; 2753 mov al,[bx]
db 0x43                                 ; 2755 inc bx
db 0x43                                 ; 2756 inc bx
db 0x43                                 ; 2757 inc bx
db 0x50                                 ; 2758 push ax
db 0xE8, 0x45, 0x13                     ; 2759 call 0x3aa1
db 0x58                                 ; 275C pop ax
db 0x3C, 0x03                           ; 275D cmp al,0x3
db 0x75, 0x05                           ; 275F jnz 0x2766
db 0xE8, 0x71, 0x00                     ; 2761 call 0x27d5
db 0x32, 0xC0                           ; 2764 xor al,al
db 0x8A, 0xD0                           ; 2766 mov dl,al
db 0xB6, 0x00                           ; 2768 mov dh,0x0
db 0x03, 0xDA                           ; 276A add bx,dx
db 0xEB, 0xDD                           ; 276C jmp short 0x274b
db 0x8B, 0x1E, 0x4E, 0x04               ; 276E mov bx,[0x44e]
db 0x8B, 0x17                           ; 2772 mov dx,[bx]
db 0x0B, 0xD2                           ; 2774 or dx,dx
db 0x8B, 0x1E, 0x5A, 0x03               ; 2776 mov bx,[0x35a]
db 0x74, 0x19                           ; 277A jz 0x2795
db 0x87, 0xDA                           ; 277C xchg bx,dx
db 0x89, 0x1E, 0x4E, 0x04               ; 277E mov [0x44e],bx
db 0x43                                 ; 2782 inc bx
db 0x43                                 ; 2783 inc bx
db 0x8B, 0x17                           ; 2784 mov dx,[bx]
db 0x43                                 ; 2786 inc bx
db 0x43                                 ; 2787 inc bx
db 0x87, 0xDA                           ; 2788 xchg bx,dx
db 0x03, 0xDA                           ; 278A add bx,dx
db 0x89, 0x1E, 0x4B, 0x04               ; 278C mov [0x44b],bx
db 0x87, 0xDA                           ; 2790 xchg bx,dx
db 0xEB, 0xB7                           ; 2792 jmp short 0x274b
db 0x59                                 ; 2794 pop cx
db 0x8B, 0x16, 0x5C, 0x03               ; 2795 mov dx,[0x35c]
db 0x3B, 0xDA                           ; 2799 cmp bx,dx
db 0x75, 0x03                           ; 279B jnz 0x27a0
db 0xE9, 0x6C, 0x00                     ; 279D jmp 0x280c
db 0x8A, 0x07                           ; 27A0 mov al,[bx]
db 0x43                                 ; 27A2 inc bx
db 0x50                                 ; 27A3 push ax
db 0x43                                 ; 27A4 inc bx
db 0x43                                 ; 27A5 inc bx
db 0xE8, 0xF8, 0x12                     ; 27A6 call 0x3aa1
db 0x8A, 0x0F                           ; 27A9 mov cl,[bx]
db 0x43                                 ; 27AB inc bx
db 0x8A, 0x2F                           ; 27AC mov ch,[bx]
db 0x43                                 ; 27AE inc bx
db 0x58                                 ; 27AF pop ax
db 0x53                                 ; 27B0 push bx
db 0x03, 0xD9                           ; 27B1 add bx,cx
db 0x3C, 0x03                           ; 27B3 cmp al,0x3
db 0x75, 0xDD                           ; 27B5 jnz 0x2794
db 0x89, 0x1E, 0x33, 0x03               ; 27B7 mov [0x333],bx
db 0x5B                                 ; 27BB pop bx
db 0x8A, 0x0F                           ; 27BC mov cl,[bx]
db 0xB5, 0x00                           ; 27BE mov ch,0x0
db 0x03, 0xD9                           ; 27C0 add bx,cx
db 0x03, 0xD9                           ; 27C2 add bx,cx
db 0x43                                 ; 27C4 inc bx
db 0x87, 0xDA                           ; 27C5 xchg bx,dx
db 0x8B, 0x1E, 0x33, 0x03               ; 27C7 mov bx,[0x333]
db 0x87, 0xDA                           ; 27CB xchg bx,dx
db 0x3B, 0xDA                           ; 27CD cmp bx,dx
db 0x74, 0xC4                           ; 27CF jz 0x2795
db 0xB9, 0xC5, 0x27                     ; 27D1 mov cx,0x27c5
db 0x51                                 ; 27D4 push cx
db 0x32, 0xC0                           ; 27D5 xor al,al
db 0x0A, 0x07                           ; 27D7 or al,[bx]
db 0x9F                                 ; 27D9 lahf
db 0x43                                 ; 27DA inc bx
db 0x9E                                 ; 27DB sahf
db 0x8A, 0x17                           ; 27DC mov dl,[bx]
db 0x9F                                 ; 27DE lahf
db 0x43                                 ; 27DF inc bx
db 0x9E                                 ; 27E0 sahf
db 0x8A, 0x37                           ; 27E1 mov dh,[bx]
db 0x9F                                 ; 27E3 lahf
db 0x43                                 ; 27E4 inc bx
db 0x9E                                 ; 27E5 sahf
db 0x75, 0x01                           ; 27E6 jnz 0x27e9
db 0xC3                                 ; 27E8 ret
db 0x8B, 0xCB                           ; 27E9 mov cx,bx
db 0x8B, 0x1E, 0x2F, 0x03               ; 27EB mov bx,[0x32f]
db 0x3B, 0xDA                           ; 27EF cmp bx,dx
db 0x8B, 0xD9                           ; 27F1 mov bx,cx
db 0x72, 0xF3                           ; 27F3 jc 0x27e8
db 0x5B                                 ; 27F5 pop bx
db 0x5E                                 ; 27F6 pop si
db 0x87, 0xDE                           ; 27F7 xchg bx,si
db 0x56                                 ; 27F9 push si
db 0x3B, 0xDA                           ; 27FA cmp bx,dx
db 0x5E                                 ; 27FC pop si
db 0x87, 0xDE                           ; 27FD xchg bx,si
db 0x56                                 ; 27FF push si
db 0x53                                 ; 2800 push bx
db 0x8B, 0xD9                           ; 2801 mov bx,cx
db 0x73, 0xE3                           ; 2803 jnc 0x27e8
db 0x59                                 ; 2805 pop cx
db 0x58                                 ; 2806 pop ax
db 0x58                                 ; 2807 pop ax
db 0x53                                 ; 2808 push bx
db 0x52                                 ; 2809 push dx
db 0x51                                 ; 280A push cx
db 0xC3                                 ; 280B ret
db 0x5A                                 ; 280C pop dx
db 0x5B                                 ; 280D pop bx
db 0x0B, 0xDB                           ; 280E or bx,bx
db 0x74, 0xF9                           ; 2810 jz 0x280b
db 0x4B                                 ; 2812 dec bx
db 0x8A, 0x2F                           ; 2813 mov ch,[bx]
db 0x4B                                 ; 2815 dec bx
db 0x8A, 0x0F                           ; 2816 mov cl,[bx]
db 0x53                                 ; 2818 push bx
db 0x4B                                 ; 2819 dec bx
db 0x8A, 0x1F                           ; 281A mov bl,[bx]
db 0xB7, 0x00                           ; 281C mov bh,0x0
db 0x03, 0xD9                           ; 281E add bx,cx
db 0x8A, 0xF5                           ; 2820 mov dh,ch
db 0x8A, 0xD1                           ; 2822 mov dl,cl
db 0x4B                                 ; 2824 dec bx
db 0x8B, 0xCB                           ; 2825 mov cx,bx
db 0x8B, 0x1E, 0x2F, 0x03               ; 2827 mov bx,[0x32f]
db 0xE8, 0xA0, 0x3C                     ; 282B call 0x64ce
db 0x5B                                 ; 282E pop bx
db 0x88, 0x0F                           ; 282F mov [bx],cl
db 0x43                                 ; 2831 inc bx
db 0x88, 0x2F                           ; 2832 mov [bx],ch
db 0x8B, 0xD9                           ; 2834 mov bx,cx
db 0x4B                                 ; 2836 dec bx
db 0xE9, 0xE0, 0xFE                     ; 2837 jmp 0x271a
db 0x51                                 ; 283A push cx
db 0x53                                 ; 283B push bx
db 0x8B, 0x1E, 0xA3, 0x04               ; 283C mov bx,[0x4a3]
db 0x5E                                 ; 2840 pop si
db 0x87, 0xDE                           ; 2841 xchg bx,si
db 0x56                                 ; 2843 push si
db 0xE8, 0xA0, 0xF0                     ; 2844 call 0x18e7
db 0x5E                                 ; 2847 pop si
db 0x87, 0xDE                           ; 2848 xchg bx,si
db 0x56                                 ; 284A push si
db 0xE8, 0xED, 0x3B                     ; 284B call 0x643b
db 0x8A, 0x07                           ; 284E mov al,[bx]
db 0x53                                 ; 2850 push bx
db 0x8B, 0x1E, 0xA3, 0x04               ; 2851 mov bx,[0x4a3]
db 0x53                                 ; 2855 push bx
db 0x02, 0x07                           ; 2856 add al,[bx]
db 0xBA, 0x0F, 0x00                     ; 2858 mov dx,0xf
db 0x73, 0x03                           ; 285B jnc 0x2860
db 0xE9, 0x78, 0xDF                     ; 285D jmp 0x7d8
db 0xE8, 0xDB, 0xFD                     ; 2860 call 0x263e
db 0x5A                                 ; 2863 pop dx
db 0xE8, 0x48, 0x00                     ; 2864 call 0x28af
db 0x5E                                 ; 2867 pop si
db 0x87, 0xDE                           ; 2868 xchg bx,si
db 0x56                                 ; 286A push si
db 0xE8, 0x3F, 0x00                     ; 286B call 0x28ad
db 0x53                                 ; 286E push bx
db 0x8B, 0x1E, 0x2D, 0x03               ; 286F mov bx,[0x32d]
db 0x87, 0xDA                           ; 2873 xchg bx,dx
db 0xE8, 0x0E, 0x00                     ; 2875 call 0x2886
db 0xE8, 0x0B, 0x00                     ; 2878 call 0x2886
db 0xBB, 0x3A, 0x17                     ; 287B mov bx,0x173a
db 0x5E                                 ; 287E pop si
db 0x87, 0xDE                           ; 287F xchg bx,si
db 0x56                                 ; 2881 push si
db 0x53                                 ; 2882 push bx
db 0xE9, 0x07, 0xFE                     ; 2883 jmp 0x268d
db 0x5B                                 ; 2886 pop bx
db 0x5E                                 ; 2887 pop si
db 0x87, 0xDE                           ; 2888 xchg bx,si
db 0x56                                 ; 288A push si
db 0x8A, 0x07                           ; 288B mov al,[bx]
db 0x43                                 ; 288D inc bx
db 0x8A, 0x0F                           ; 288E mov cl,[bx]
db 0x43                                 ; 2890 inc bx
db 0x8A, 0x2F                           ; 2891 mov ch,[bx]
db 0x8A, 0xD8                           ; 2893 mov bl,al
db 0xFE, 0xC3                           ; 2895 inc bl
db 0xFE, 0xCB                           ; 2897 dec bl
db 0x75, 0x01                           ; 2899 jnz 0x289c
db 0xC3                                 ; 289B ret
db 0x8B, 0xF1                           ; 289C mov si,cx
db 0xAC                                 ; 289E lodsb
db 0x8B, 0xFA                           ; 289F mov di,dx
db 0xAA                                 ; 28A1 stosb
db 0x41                                 ; 28A2 inc cx
db 0x42                                 ; 28A3 inc dx
db 0xEB, 0xF1                           ; 28A4 jmp short 0x2897
db 0xE8, 0x92, 0x3B                     ; 28A6 call 0x643b
db 0x8B, 0x1E, 0xA3, 0x04               ; 28A9 mov bx,[0x4a3]
db 0x87, 0xDA                           ; 28AD xchg bx,dx
db 0xE8, 0x20, 0x00                     ; 28AF call 0x28d2
db 0x87, 0xDA                           ; 28B2 xchg bx,dx
db 0x75, 0xE5                           ; 28B4 jnz 0x289b
db 0x52                                 ; 28B6 push dx
db 0x8A, 0xF5                           ; 28B7 mov dh,ch
db 0x8A, 0xD1                           ; 28B9 mov dl,cl
db 0x4A                                 ; 28BB dec dx
db 0x8A, 0x0F                           ; 28BC mov cl,[bx]
db 0x8B, 0x1E, 0x2F, 0x03               ; 28BE mov bx,[0x32f]
db 0x3B, 0xDA                           ; 28C2 cmp bx,dx
db 0x75, 0x0A                           ; 28C4 jnz 0x28d0
db 0x32, 0xC0                           ; 28C6 xor al,al
db 0x8A, 0xE8                           ; 28C8 mov ch,al
db 0x03, 0xD9                           ; 28CA add bx,cx
db 0x89, 0x1E, 0x2F, 0x03               ; 28CC mov [0x32f],bx
db 0x5B                                 ; 28D0 pop bx
db 0xC3                                 ; 28D1 ret
db 0xCD, 0xEE                           ; 28D2 int 0xee
db 0x8B, 0x1E, 0x0C, 0x03               ; 28D4 mov bx,[0x30c]
db 0x4B                                 ; 28D8 dec bx
db 0x8A, 0x2F                           ; 28D9 mov ch,[bx]
db 0x4B                                 ; 28DB dec bx
db 0x8A, 0x0F                           ; 28DC mov cl,[bx]
db 0x4B                                 ; 28DE dec bx
db 0x3B, 0xDA                           ; 28DF cmp bx,dx
db 0x75, 0xEE                           ; 28E1 jnz 0x28d1
db 0x89, 0x1E, 0x0C, 0x03               ; 28E3 mov [0x30c],bx
db 0xC3                                 ; 28E7 ret
db 0xB9, 0x7F, 0x1B                     ; 28E8 mov cx,0x1b7f
db 0x51                                 ; 28EB push cx
db 0xE8, 0xB7, 0xFF                     ; 28EC call 0x28a6
db 0x32, 0xC0                           ; 28EF xor al,al
db 0x8A, 0xF0                           ; 28F1 mov dh,al
db 0x8A, 0x07                           ; 28F3 mov al,[bx]
db 0x0A, 0xC0                           ; 28F5 or al,al
db 0xC3                                 ; 28F7 ret
db 0xB9, 0x7F, 0x1B                     ; 28F8 mov cx,0x1b7f
db 0x51                                 ; 28FB push cx
db 0xE8, 0xED, 0xFF                     ; 28FC call 0x28ec
db 0x75, 0x03                           ; 28FF jnz 0x2904
db 0xE9, 0x55, 0xE7                     ; 2901 jmp 0x1059
db 0x43                                 ; 2904 inc bx
db 0x8B, 0x17                           ; 2905 mov dx,[bx]
db 0x8B, 0xF2                           ; 2907 mov si,dx
db 0xAC                                 ; 2909 lodsb
db 0xC3                                 ; 290A ret
db 0xE8, 0x2E, 0xFD                     ; 290B call 0x263c
db 0xE8, 0x0E, 0xF6                     ; 290E call 0x1f1f
db 0x8B, 0x1E, 0x2D, 0x03               ; 2911 mov bx,[0x32d]
db 0x88, 0x17                           ; 2915 mov [bx],dl
db 0x59                                 ; 2917 pop cx
db 0xE9, 0x72, 0xFD                     ; 2918 jmp 0x268d
db 0xE8, 0xFF, 0xE5                     ; 291B call 0xf1d
db 0xE8, 0xD3, 0x04                     ; 291E call 0x2df4
db 0x28, 0xE8                           ; 2921 sub al,ch
db 0xF7, 0xF5                           ; 2923 div bp
db 0x52                                 ; 2925 push dx
db 0xE8, 0xCB, 0x04                     ; 2926 call 0x2df4
db 0x2C, 0xE8                           ; 2929 sub al,0xe8
db 0xFA                                 ; 292B cli
db 0xED                                 ; 292C in ax,dx
db 0xE8, 0xC4, 0x04                     ; 292D call 0x2df4
db 0x29, 0x5E, 0x87                     ; 2930 sub [bp-0x79],bx
db 0xDE, 0x56, 0x53                     ; 2933 ficom word [bp+0x53]
db 0xE8, 0xEC, 0xF1                     ; 2936 call 0x1b25
db 0x74, 0x05                           ; 2939 jz 0x2940
db 0xE8, 0xE1, 0xF5                     ; 293B call 0x1f1f
db 0xEB, 0x03                           ; 293E jmp short 0x2943
db 0xE8, 0xB9, 0xFF                     ; 2940 call 0x28fc
db 0x5A                                 ; 2943 pop dx
db 0xE8, 0x05, 0x00                     ; 2944 call 0x294c
db 0xE8, 0xD5, 0xF5                     ; 2947 call 0x1f1f
db 0xB0, 0x20                           ; 294A mov al,0x20
db 0x50                                 ; 294C push ax
db 0x8A, 0xC2                           ; 294D mov al,dl
db 0xE8, 0xEC, 0xFC                     ; 294F call 0x263e
db 0x8A, 0xE8                           ; 2952 mov ch,al
db 0x58                                 ; 2954 pop ax
db 0xFE, 0xC5                           ; 2955 inc ch
db 0xFE, 0xCD                           ; 2957 dec ch
db 0x74, 0xBC                           ; 2959 jz 0x2917
db 0x8B, 0x1E, 0x2D, 0x03               ; 295B mov bx,[0x32d]
db 0x88, 0x07                           ; 295F mov [bx],al
db 0x43                                 ; 2961 inc bx
db 0xFE, 0xCD                           ; 2962 dec ch
db 0x75, 0xF9                           ; 2964 jnz 0x295f
db 0xEB, 0xAF                           ; 2966 jmp short 0x2917
db 0xE8, 0xA3, 0x00                     ; 2968 call 0x2a0e
db 0x32, 0xC0                           ; 296B xor al,al
db 0x5E                                 ; 296D pop si
db 0x87, 0xDE                           ; 296E xchg bx,si
db 0x56                                 ; 2970 push si
db 0x8A, 0xC8                           ; 2971 mov cl,al
db 0xB0, 0x53                           ; 2973 mov al,0x53
db 0x53                                 ; 2975 push bx
db 0x8A, 0x07                           ; 2976 mov al,[bx]
db 0x3A, 0xC5                           ; 2978 cmp al,ch
db 0x72, 0x03                           ; 297A jc 0x297f
db 0x8A, 0xC5                           ; 297C mov al,ch
db 0xBA, 0xB1, 0x00                     ; 297E mov dx,0xb1
db 0x51                                 ; 2981 push cx
db 0xE8, 0x51, 0xFD                     ; 2982 call 0x26d6
db 0x59                                 ; 2985 pop cx
db 0x5B                                 ; 2986 pop bx
db 0x53                                 ; 2987 push bx
db 0x43                                 ; 2988 inc bx
db 0x8A, 0x2F                           ; 2989 mov ch,[bx]
db 0x43                                 ; 298B inc bx
db 0x8A, 0x3F                           ; 298C mov bh,[bx]
db 0x8A, 0xDD                           ; 298E mov bl,ch
db 0xB5, 0x00                           ; 2990 mov ch,0x0
db 0x03, 0xD9                           ; 2992 add bx,cx
db 0x8B, 0xCB                           ; 2994 mov cx,bx
db 0xE8, 0xA8, 0xFC                     ; 2996 call 0x2641
db 0x8A, 0xD8                           ; 2999 mov bl,al
db 0xE8, 0xF7, 0xFE                     ; 299B call 0x2895
db 0x5A                                 ; 299E pop dx
db 0xE8, 0x0D, 0xFF                     ; 299F call 0x28af
db 0xE9, 0xE8, 0xFC                     ; 29A2 jmp 0x268d
db 0xE8, 0x66, 0x00                     ; 29A5 call 0x2a0e
db 0x5A                                 ; 29A8 pop dx
db 0x52                                 ; 29A9 push dx
db 0x8B, 0xF2                           ; 29AA mov si,dx
db 0xAC                                 ; 29AC lodsb
db 0x2A, 0xC5                           ; 29AD sub al,ch
db 0xEB, 0xBC                           ; 29AF jmp short 0x296d
db 0x87, 0xDA                           ; 29B1 xchg bx,dx
db 0x8A, 0x07                           ; 29B3 mov al,[bx]
db 0xE8, 0x5C, 0x00                     ; 29B5 call 0x2a14
db 0xFE, 0xC5                           ; 29B8 inc ch
db 0xFE, 0xCD                           ; 29BA dec ch
db 0x75, 0x03                           ; 29BC jnz 0x29c1
db 0xE9, 0x98, 0xE6                     ; 29BE jmp 0x1059
db 0x51                                 ; 29C1 push cx
db 0xE8, 0xB5, 0x01                     ; 29C2 call 0x2b7a
db 0x58                                 ; 29C5 pop ax
db 0x86, 0xC4                           ; 29C6 xchg al,ah
db 0x9E                                 ; 29C8 sahf
db 0x5E                                 ; 29C9 pop si
db 0x87, 0xDE                           ; 29CA xchg bx,si
db 0x56                                 ; 29CC push si
db 0xB9, 0x75, 0x29                     ; 29CD mov cx,0x2975
db 0x51                                 ; 29D0 push cx
db 0xFE, 0xC8                           ; 29D1 dec al
db 0x3A, 0x07                           ; 29D3 cmp al,[bx]
db 0xB5, 0x00                           ; 29D5 mov ch,0x0
db 0x72, 0x01                           ; 29D7 jc 0x29da
db 0xC3                                 ; 29D9 ret
db 0x8A, 0xC8                           ; 29DA mov cl,al
db 0x8A, 0x07                           ; 29DC mov al,[bx]
db 0x2A, 0xC1                           ; 29DE sub al,cl
db 0x3A, 0xC2                           ; 29E0 cmp al,dl
db 0x8A, 0xE8                           ; 29E2 mov ch,al
db 0x72, 0xF3                           ; 29E4 jc 0x29d9
db 0x8A, 0xEA                           ; 29E6 mov ch,dl
db 0xC3                                 ; 29E8 ret
db 0xE8, 0x00, 0xFF                     ; 29E9 call 0x28ec
db 0x75, 0x03                           ; 29EC jnz 0x29f1
db 0xE9, 0x8E, 0xF1                     ; 29EE jmp 0x1b7f
db 0x8A, 0xD0                           ; 29F1 mov dl,al
db 0x43                                 ; 29F3 inc bx
db 0x8B, 0x1F                           ; 29F4 mov bx,[bx]
db 0x53                                 ; 29F6 push bx
db 0x03, 0xDA                           ; 29F7 add bx,dx
db 0x8A, 0x2F                           ; 29F9 mov ch,[bx]
db 0x88, 0x37                           ; 29FB mov [bx],dh
db 0x5E                                 ; 29FD pop si
db 0x87, 0xDE                           ; 29FE xchg bx,si
db 0x56                                 ; 2A00 push si
db 0x51                                 ; 2A01 push cx
db 0x4B                                 ; 2A02 dec bx
db 0xE8, 0x17, 0xE5                     ; 2A03 call 0xf1d
db 0xE8, 0xBE, 0x3F                     ; 2A06 call 0x69c7
db 0x59                                 ; 2A09 pop cx
db 0x5B                                 ; 2A0A pop bx
db 0x88, 0x2F                           ; 2A0B mov [bx],ch
db 0xC3                                 ; 2A0D ret
db 0x87, 0xDA                           ; 2A0E xchg bx,dx
db 0xE8, 0xE1, 0x03                     ; 2A10 call 0x2df4
db 0x29, 0x59, 0x5A                     ; 2A13 sub [bx+di+0x5a],bx
db 0x51                                 ; 2A16 push cx
db 0x8A, 0xEA                           ; 2A17 mov ch,dl
db 0xC3                                 ; 2A19 ret
db 0xE8, 0x00, 0xE5                     ; 2A1A call 0xf1d
db 0xE8, 0x03, 0xED                     ; 2A1D call 0x1723
db 0xE8, 0x02, 0xF1                     ; 2A20 call 0x1b25
db 0xB0, 0x01                           ; 2A23 mov al,0x1
db 0x50                                 ; 2A25 push ax
db 0x74, 0x16                           ; 2A26 jz 0x2a3e
db 0x58                                 ; 2A28 pop ax
db 0xE8, 0xF3, 0xF4                     ; 2A29 call 0x1f1f
db 0x0A, 0xC0                           ; 2A2C or al,al
db 0x75, 0x03                           ; 2A2E jnz 0x2a33
db 0xE9, 0x26, 0xE6                     ; 2A30 jmp 0x1059
db 0x50                                 ; 2A33 push ax
db 0xE8, 0xBD, 0x03                     ; 2A34 call 0x2df4
db 0x2C, 0xE8                           ; 2A37 sub al,0xe8
db 0xEC                                 ; 2A39 in al,dx
db 0xEC                                 ; 2A3A in al,dx
db 0xE8, 0xFD, 0x39                     ; 2A3B call 0x643b
db 0xE8, 0xB3, 0x03                     ; 2A3E call 0x2df4
db 0x2C, 0x53                           ; 2A41 sub al,0x53
db 0x8B, 0x1E, 0xA3, 0x04               ; 2A43 mov bx,[0x4a3]
db 0x5E                                 ; 2A47 pop si
db 0x87, 0xDE                           ; 2A48 xchg bx,si
db 0x56                                 ; 2A4A push si
db 0xE8, 0xD9, 0xEC                     ; 2A4B call 0x1727
db 0xE8, 0xA3, 0x03                     ; 2A4E call 0x2df4
db 0x29, 0x53, 0xE8                     ; 2A51 sub [bp+di-0x18],dx
db 0x50                                 ; 2A54 push ax
db 0xFE, 0x87, 0xDA, 0x59               ; 2A55 inc byte [bx+0x59da]
db 0x5B                                 ; 2A59 pop bx
db 0x58                                 ; 2A5A pop ax
db 0x51                                 ; 2A5B push cx
db 0xB9, 0x07, 0x65                     ; 2A5C mov cx,0x6507
db 0x51                                 ; 2A5F push cx
db 0xB9, 0x7F, 0x1B                     ; 2A60 mov cx,0x1b7f
db 0x51                                 ; 2A63 push cx
db 0x50                                 ; 2A64 push ax
db 0x52                                 ; 2A65 push dx
db 0xE8, 0x44, 0xFE                     ; 2A66 call 0x28ad
db 0x5A                                 ; 2A69 pop dx
db 0x58                                 ; 2A6A pop ax
db 0x8A, 0xE8                           ; 2A6B mov ch,al
db 0xFE, 0xC8                           ; 2A6D dec al
db 0x8A, 0xC8                           ; 2A6F mov cl,al
db 0x3A, 0x07                           ; 2A71 cmp al,[bx]
db 0xB0, 0x00                           ; 2A73 mov al,0x0
db 0x73, 0xA2                           ; 2A75 jnc 0x2a19
db 0x8B, 0xF2                           ; 2A77 mov si,dx
db 0xAC                                 ; 2A79 lodsb
db 0x0A, 0xC0                           ; 2A7A or al,al
db 0x8A, 0xC5                           ; 2A7C mov al,ch
db 0x74, 0x99                           ; 2A7E jz 0x2a19
db 0x8A, 0x07                           ; 2A80 mov al,[bx]
db 0x43                                 ; 2A82 inc bx
db 0x8A, 0x2F                           ; 2A83 mov ch,[bx]
db 0x43                                 ; 2A85 inc bx
db 0x8A, 0x3F                           ; 2A86 mov bh,[bx]
db 0x8A, 0xDD                           ; 2A88 mov bl,ch
db 0xB5, 0x00                           ; 2A8A mov ch,0x0
db 0x03, 0xD9                           ; 2A8C add bx,cx
db 0x2A, 0xC1                           ; 2A8E sub al,cl
db 0x8A, 0xE8                           ; 2A90 mov ch,al
db 0x51                                 ; 2A92 push cx
db 0x52                                 ; 2A93 push dx
db 0x5E                                 ; 2A94 pop si
db 0x87, 0xDE                           ; 2A95 xchg bx,si
db 0x56                                 ; 2A97 push si
db 0x8A, 0x0F                           ; 2A98 mov cl,[bx]
db 0x43                                 ; 2A9A inc bx
db 0x8B, 0x17                           ; 2A9B mov dx,[bx]
db 0x5B                                 ; 2A9D pop bx
db 0x53                                 ; 2A9E push bx
db 0x52                                 ; 2A9F push dx
db 0x51                                 ; 2AA0 push cx
db 0x8B, 0xF2                           ; 2AA1 mov si,dx
db 0xAC                                 ; 2AA3 lodsb
db 0x3A, 0x07                           ; 2AA4 cmp al,[bx]
db 0x75, 0x1E                           ; 2AA6 jnz 0x2ac6
db 0x42                                 ; 2AA8 inc dx
db 0xFE, 0xC9                           ; 2AA9 dec cl
db 0x74, 0x0C                           ; 2AAB jz 0x2ab9
db 0x43                                 ; 2AAD inc bx
db 0xFE, 0xCD                           ; 2AAE dec ch
db 0x75, 0xEF                           ; 2AB0 jnz 0x2aa1
db 0x5A                                 ; 2AB2 pop dx
db 0x5A                                 ; 2AB3 pop dx
db 0x59                                 ; 2AB4 pop cx
db 0x5A                                 ; 2AB5 pop dx
db 0x32, 0xC0                           ; 2AB6 xor al,al
db 0xC3                                 ; 2AB8 ret
db 0x5B                                 ; 2AB9 pop bx
db 0x5A                                 ; 2ABA pop dx
db 0x5A                                 ; 2ABB pop dx
db 0x59                                 ; 2ABC pop cx
db 0x8A, 0xC5                           ; 2ABD mov al,ch
db 0x2A, 0xC7                           ; 2ABF sub al,bh
db 0x02, 0xC1                           ; 2AC1 add al,cl
db 0xFE, 0xC0                           ; 2AC3 inc al
db 0xC3                                 ; 2AC5 ret
db 0x59                                 ; 2AC6 pop cx
db 0x5A                                 ; 2AC7 pop dx
db 0x5B                                 ; 2AC8 pop bx
db 0x43                                 ; 2AC9 inc bx
db 0xFE, 0xCD                           ; 2ACA dec ch
db 0x75, 0xD0                           ; 2ACC jnz 0x2a9e
db 0xEB, 0xE5                           ; 2ACE jmp short 0x2ab5
db 0xE8, 0x21, 0x03                     ; 2AD0 call 0x2df4
db 0x28, 0xE8                           ; 2AD3 sub al,ch
db 0x97                                 ; 2AD5 xchg ax,di
db 0x0C, 0xE8                           ; 2AD6 or al,0xe8
db 0x61                                 ; 2AD8 popa
db 0x39, 0x53, 0x52                     ; 2AD9 cmp [bp+di+0x52],dx
db 0x87, 0xDA                           ; 2ADC xchg bx,dx
db 0x43                                 ; 2ADE inc bx
db 0x8B, 0x17                           ; 2ADF mov dx,[bx]
db 0x8B, 0x1E, 0x5C, 0x03               ; 2AE1 mov bx,[0x35c]
db 0x3B, 0xDA                           ; 2AE5 cmp bx,dx
db 0x72, 0x12                           ; 2AE7 jc 0x2afb
db 0x8B, 0x1E, 0x30, 0x00               ; 2AE9 mov bx,[0x30]
db 0x3B, 0xDA                           ; 2AED cmp bx,dx
db 0x73, 0x0A                           ; 2AEF jnc 0x2afb
db 0x5B                                 ; 2AF1 pop bx
db 0x53                                 ; 2AF2 push bx
db 0xE8, 0x2E, 0xFB                     ; 2AF3 call 0x2624
db 0x5B                                 ; 2AF6 pop bx
db 0x53                                 ; 2AF7 push bx
db 0xE8, 0xBE, 0x39                     ; 2AF8 call 0x64b9
db 0x5B                                 ; 2AFB pop bx
db 0x5E                                 ; 2AFC pop si
db 0x87, 0xDE                           ; 2AFD xchg bx,si
db 0x56                                 ; 2AFF push si
db 0xE8, 0xF1, 0x02                     ; 2B00 call 0x2df4
db 0x2C, 0xE8                           ; 2B03 sub al,0xe8
db 0x15, 0xF4, 0x0A                     ; 2B05 adc ax,0xaf4
db 0xC0                                 ; 2B08 db 0xC0
db 0x75, 0x03                           ; 2B09 jnz 0x2b0e
db 0xE9, 0x4B, 0xE5                     ; 2B0B jmp 0x1059
db 0x50                                 ; 2B0E push ax
db 0x8A, 0x07                           ; 2B0F mov al,[bx]
db 0xE8, 0x66, 0x00                     ; 2B11 call 0x2b7a
db 0x52                                 ; 2B14 push dx
db 0xE8, 0x04, 0xEC                     ; 2B15 call 0x171c
db 0x53                                 ; 2B18 push bx
db 0xE8, 0x8A, 0xFD                     ; 2B19 call 0x28a6
db 0x87, 0xDA                           ; 2B1C xchg bx,dx
db 0x5B                                 ; 2B1E pop bx
db 0x59                                 ; 2B1F pop cx
db 0x58                                 ; 2B20 pop ax
db 0x8A, 0xE8                           ; 2B21 mov ch,al
db 0x5E                                 ; 2B23 pop si
db 0x87, 0xDE                           ; 2B24 xchg bx,si
db 0x56                                 ; 2B26 push si
db 0x53                                 ; 2B27 push bx
db 0xBB, 0x07, 0x65                     ; 2B28 mov bx,0x6507
db 0x5E                                 ; 2B2B pop si
db 0x87, 0xDE                           ; 2B2C xchg bx,si
db 0x56                                 ; 2B2E push si
db 0x8A, 0xC1                           ; 2B2F mov al,cl
db 0x0A, 0xC0                           ; 2B31 or al,al
db 0x74, 0x90                           ; 2B33 jz 0x2ac5
db 0x8A, 0x07                           ; 2B35 mov al,[bx]
db 0x2A, 0xC5                           ; 2B37 sub al,ch
db 0x73, 0x03                           ; 2B39 jnc 0x2b3e
db 0xE9, 0x1B, 0xE5                     ; 2B3B jmp 0x1059
db 0xFE, 0xC0                           ; 2B3E inc al
db 0x3A, 0xC1                           ; 2B40 cmp al,cl
db 0x72, 0x02                           ; 2B42 jc 0x2b46
db 0x8A, 0xC1                           ; 2B44 mov al,cl
db 0x8A, 0xCD                           ; 2B46 mov cl,ch
db 0xFE, 0xC9                           ; 2B48 dec cl
db 0xB5, 0x00                           ; 2B4A mov ch,0x0
db 0x52                                 ; 2B4C push dx
db 0x43                                 ; 2B4D inc bx
db 0x8A, 0x17                           ; 2B4E mov dl,[bx]
db 0x43                                 ; 2B50 inc bx
db 0x8A, 0x3F                           ; 2B51 mov bh,[bx]
db 0x8A, 0xDA                           ; 2B53 mov bl,dl
db 0x03, 0xD9                           ; 2B55 add bx,cx
db 0x8A, 0xE8                           ; 2B57 mov ch,al
db 0x5A                                 ; 2B59 pop dx
db 0x87, 0xDA                           ; 2B5A xchg bx,dx
db 0x8A, 0x0F                           ; 2B5C mov cl,[bx]
db 0x43                                 ; 2B5E inc bx
db 0x8B, 0x1F                           ; 2B5F mov bx,[bx]
db 0x87, 0xDA                           ; 2B61 xchg bx,dx
db 0x8A, 0xC1                           ; 2B63 mov al,cl
db 0x0A, 0xC0                           ; 2B65 or al,al
db 0x75, 0x01                           ; 2B67 jnz 0x2b6a
db 0xC3                                 ; 2B69 ret
db 0x8B, 0xF2                           ; 2B6A mov si,dx
db 0xAC                                 ; 2B6C lodsb
db 0x88, 0x07                           ; 2B6D mov [bx],al
db 0x42                                 ; 2B6F inc dx
db 0x43                                 ; 2B70 inc bx
db 0xFE, 0xC9                           ; 2B71 dec cl
db 0x74, 0xF4                           ; 2B73 jz 0x2b69
db 0xFE, 0xCD                           ; 2B75 dec ch
db 0x75, 0xF1                           ; 2B77 jnz 0x2b6a
db 0xC3                                 ; 2B79 ret
db 0xB2, 0xFF                           ; 2B7A mov dl,0xff
db 0x3C, 0x29                           ; 2B7C cmp al,0x29
db 0x74, 0x07                           ; 2B7E jz 0x2b87
db 0xE8, 0x71, 0x02                     ; 2B80 call 0x2df4
db 0x2C, 0xE8                           ; 2B83 sub al,0xe8
db 0x95                                 ; 2B85 xchg ax,bp
db 0xF3, 0xE8, 0x6A, 0x02               ; 2B86 rep call 0x2df4
db 0x29, 0xC3                           ; 2B8A sub bx,ax
db 0xE8, 0x96, 0xEF                     ; 2B8C call 0x1b25
db 0x74, 0x03                           ; 2B8F jz 0x2b94
db 0xE9, 0x06, 0x00                     ; 2B91 jmp 0x2b9a
db 0xE8, 0x12, 0xFD                     ; 2B94 call 0x28a9
db 0xE8, 0x7C, 0xFB                     ; 2B97 call 0x2716
db 0x8B, 0x16, 0x5C, 0x03               ; 2B9A mov dx,[0x35c]
db 0x8B, 0x1E, 0x2F, 0x03               ; 2B9E mov bx,[0x32f]
db 0xE9, 0xCB, 0xEF                     ; 2BA2 jmp 0x1b70

L_2BA5:
db 0xCD, 0xB4                           ; 2BA5 int 0xb4
db 0x9F                                 ; 2BA7 lahf
db 0x86, 0xC4                           ; 2BA8 xchg al,ah
db 0x50                                 ; 2BAA push ax
db 0x86, 0xC4                           ; 2BAB xchg al,ah
db 0x53                                 ; 2BAD push bx
db 0xE8, 0x25, 0x04                     ; 2BAE call 0x2fd6
db 0x74, 0x03                           ; 2BB1 jz 0x2bb6
db 0xE9, 0xDD, 0x17                     ; 2BB3 jmp 0x4393
db 0x5B                                 ; 2BB6 pop bx
db 0x58                                 ; 2BB7 pop ax
db 0x86, 0xC4                           ; 2BB8 xchg al,ah
db 0x9E                                 ; 2BBA sahf
db 0x51                                 ; 2BBB push cx
db 0x9F                                 ; 2BBC lahf
db 0x50                                 ; 2BBD push ax
db 0xEB, 0x12                           ; 2BBE jmp short 0x2bd2
db 0x32, 0xC0                           ; 2BC0 xor al,al
db 0xA2, 0x4F, 0x03                     ; 2BC2 mov [0x34f],al
db 0xE9, 0x93, 0xE5                     ; 2BC5 jmp 0x115b
db 0x19, 0xFE                           ; 2BC8 sbb si,di
db 0xC8, 0xE8, 0x63, 0x23               ; 2BCA enter 0x63e8,0x23
db 0xB0, 0x08                           ; 2BCE mov al,0x8
db 0xEB, 0x38                           ; 2BD0 jmp short 0x2c0a
db 0x3C, 0x09                           ; 2BD2 cmp al,0x9
db 0x75, 0x10                           ; 2BD4 jnz 0x2be6
db 0xB0, 0x20                           ; 2BD6 mov al,0x20
db 0xE8, 0xCA, 0xFF                     ; 2BD8 call 0x2ba5
db 0xE8, 0x4E, 0x23                     ; 2BDB call 0x4f2c
db 0x24, 0x07                           ; 2BDE and al,0x7
db 0x75, 0xF4                           ; 2BE0 jnz 0x2bd6
db 0x58                                 ; 2BE2 pop ax
db 0x9E                                 ; 2BE3 sahf
db 0x59                                 ; 2BE4 pop cx
db 0xC3                                 ; 2BE5 ret
db 0x3C, 0x20                           ; 2BE6 cmp al,0x20
db 0x72, 0x20                           ; 2BE8 jc 0x2c0a
db 0xA0, 0x29, 0x00                     ; 2BEA mov al,[0x29]
db 0x8A, 0xE8                           ; 2BED mov ch,al
db 0xE8, 0x3A, 0x23                     ; 2BEF call 0x4f2c
db 0xFE, 0xC5                           ; 2BF2 inc ch
db 0x74, 0x0B                           ; 2BF4 jz 0x2c01
db 0xFE, 0xCD                           ; 2BF6 dec ch
db 0x3A, 0xC5                           ; 2BF8 cmp al,ch
db 0x75, 0x03                           ; 2BFA jnz 0x2bff
db 0xE8, 0x72, 0x00                     ; 2BFC call 0x2c71
db 0x74, 0x09                           ; 2BFF jz 0x2c0a
db 0x3C, 0xFF                           ; 2C01 cmp al,0xff
db 0x74, 0x05                           ; 2C03 jz 0x2c0a
db 0xFE, 0xC0                           ; 2C05 inc al
db 0xE8, 0x27, 0x23                     ; 2C07 call 0x4f31
db 0x58                                 ; 2C0A pop ax
db 0x9E                                 ; 2C0B sahf
db 0x59                                 ; 2C0C pop cx
db 0x9F                                 ; 2C0D lahf
db 0x50                                 ; 2C0E push ax
db 0x58                                 ; 2C0F pop ax
db 0x9E                                 ; 2C10 sahf
db 0xE8, 0x7C, 0x22                     ; 2C11 call 0x4e90
db 0xC3                                 ; 2C14 ret
db 0xCD, 0xB5                           ; 2C15 int 0xb5
db 0xE8, 0xBC, 0x03                     ; 2C17 call 0x2fd6
db 0x74, 0x3D                           ; 2C1A jz 0x2c59
db 0xE8, 0xB6, 0x17                     ; 2C1C call 0x43d5
db 0x73, 0xF3                           ; 2C1F jnc 0x2c14
db 0x51                                 ; 2C21 push cx
db 0x52                                 ; 2C22 push dx
db 0x53                                 ; 2C23 push bx
db 0xA0, 0x36, 0x05                     ; 2C24 mov al,[0x536]
db 0x24, 0xC8                           ; 2C27 and al,0xc8
db 0xA2, 0x36, 0x05                     ; 2C29 mov [0x536],al
db 0xE8, 0xCC, 0x18                     ; 2C2C call 0x44fb
db 0x5B                                 ; 2C2F pop bx
db 0x5A                                 ; 2C30 pop dx
db 0x59                                 ; 2C31 pop cx
db 0xA0, 0x6B, 0x04                     ; 2C32 mov al,[0x46b]
db 0x0A, 0xC0                           ; 2C35 or al,al
db 0x74, 0x03                           ; 2C37 jz 0x2c3c
db 0xE9, 0x0A, 0x31                     ; 2C39 jmp 0x5d46
db 0xA0, 0xEF, 0x04                     ; 2C3C mov al,[0x4ef]
db 0x0A, 0xC0                           ; 2C3F or al,al
db 0x74, 0x07                           ; 2C41 jz 0x2c4a
db 0xBB, 0xE8, 0x0E                     ; 2C43 mov bx,0xee8
db 0x53                                 ; 2C46 push bx
db 0xE9, 0xED, 0x00                     ; 2C47 jmp 0x2d37
db 0x53                                 ; 2C4A push bx
db 0x51                                 ; 2C4B push cx
db 0x52                                 ; 2C4C push dx
db 0xBB, 0x2D, 0x07                     ; 2C4D mov bx,0x72d
db 0xE8, 0x02, 0x4F                     ; 2C50 call 0x7b55
db 0x5A                                 ; 2C53 pop dx
db 0x59                                 ; 2C54 pop cx
db 0xB0, 0x0D                           ; 2C55 mov al,0xd
db 0x5B                                 ; 2C57 pop bx
db 0xC3                                 ; 2C58 ret
db 0xE8, 0x12, 0x21                     ; 2C59 call 0x4d6e
db 0xC3                                 ; 2C5C ret
db 0xE8, 0xCC, 0x22                     ; 2C5D call 0x4f2c
db 0x0A, 0xC0                           ; 2C60 or al,al
db 0x74, 0xF8                           ; 2C62 jz 0x2c5c
db 0xEB, 0x0B                           ; 2C64 jmp short 0x2c71
db 0xC6, 0x07, 0x00                     ; 2C66 mov byte [bx],0x0
db 0xE8, 0x6A, 0x03                     ; 2C69 call 0x2fd6
db 0xBB, 0xF6, 0x01                     ; 2C6C mov bx,0x1f6
db 0x75, 0x07                           ; 2C6F jnz 0x2c78
db 0xCD, 0xB6                           ; 2C71 int 0xb6
db 0xB0, 0x0D                           ; 2C73 mov al,0xd
db 0xE8, 0x2D, 0xFF                     ; 2C75 call 0x2ba5
L_2C78:
db 0xE8, 0x5B, 0x03                     ; 2C78 call 0x2fd6
  jz      L_2C80

  xor     al, al
  ret

L_2C80:
db 0x32, 0xC0                           ; 2C80 xor al,al
db 0xE8, 0xAC, 0x22                     ; 2C82 call 0x4f31
db 0x32, 0xC0                           ; 2C85 xor al,al
db 0xC3                                 ; 2C87 ret
db 0xCD, 0xB7                           ; 2C88 int 0xb7
db 0xA0, 0x5E, 0x00                     ; 2C8A mov al,[0x5e]
db 0x0A, 0xC0                           ; 2C8D or al,al
db 0x75, 0x01                           ; 2C8F jnz 0x2c92
db 0xC3                                 ; 2C91 ret
db 0xE8, 0xC4, 0xFF                     ; 2C92 call 0x2c59
db 0x75, 0x03                           ; 2C95 jnz 0x2c9a
db 0xE8, 0xDC, 0x01                     ; 2C97 call 0x2e76
db 0xE9, 0x90, 0x01                     ; 2C9A jmp 0x2e2d
db 0xE8, 0xEF, 0x32                     ; 2C9D call 0x5f8f
db 0x53                                 ; 2CA0 push bx
db 0xE8, 0xAF, 0x20                     ; 2CA1 call 0x4d53
db 0x74, 0x21                           ; 2CA4 jz 0x2cc7
db 0xE8, 0xB0, 0xFF                     ; 2CA6 call 0x2c59
db 0x0A, 0xC0                           ; 2CA9 or al,al
db 0x75, 0x10                           ; 2CAB jnz 0x2cbd
db 0x50                                 ; 2CAD push ax
db 0xB0, 0x02                           ; 2CAE mov al,0x2
db 0xE8, 0x8B, 0xF9                     ; 2CB0 call 0x263e
db 0x8B, 0x1E, 0x2D, 0x03               ; 2CB3 mov bx,[0x32d]
db 0x5A                                 ; 2CB7 pop dx
db 0x89, 0x17                           ; 2CB8 mov [bx],dx
db 0xE9, 0xD0, 0xF9                     ; 2CBA jmp 0x268d
db 0x50                                 ; 2CBD push ax
db 0xE8, 0x7B, 0xF9                     ; 2CBE call 0x263c
db 0x58                                 ; 2CC1 pop ax
db 0x8A, 0xD0                           ; 2CC2 mov dl,al
db 0xE8, 0x4A, 0xFC                     ; 2CC4 call 0x2911
db 0xBB, 0x06, 0x00                     ; 2CC7 mov bx,0x6
db 0x89, 0x1E, 0xA3, 0x04               ; 2CCA mov [0x4a3],bx
db 0xB0, 0x03                           ; 2CCE mov al,0x3
db 0xA2, 0xFB, 0x02                     ; 2CD0 mov [0x2fb],al
db 0x5B                                 ; 2CD3 pop bx
db 0xC3                                 ; 2CD4 ret
db 0x53                                 ; 2CD5 push bx
db 0x8B, 0x1E, 0x0A, 0x03               ; 2CD6 mov bx,[0x30a]
db 0xB5, 0x00                           ; 2CDA mov ch,0x0
db 0x03, 0xD9                           ; 2CDC add bx,cx
db 0x03, 0xD9                           ; 2CDE add bx,cx
db 0xB0, 0x26                           ; 2CE0 mov al,0x26
db 0x2A, 0xC3                           ; 2CE2 sub al,bl
db 0x8A, 0xD8                           ; 2CE4 mov bl,al
db 0xB0, 0xFF                           ; 2CE6 mov al,0xff
db 0x1A, 0xC7                           ; 2CE8 sbb al,bh
db 0x8A, 0xF8                           ; 2CEA mov bh,al
db 0x72, 0x06                           ; 2CEC jc 0x2cf4
db 0x03, 0xDC                           ; 2CEE add bx,sp
db 0x5B                                 ; 2CF0 pop bx
db 0x73, 0x01                           ; 2CF1 jnc 0x2cf4
db 0xC3                                 ; 2CF3 ret
db 0x8B, 0x1E, 0x2C, 0x00               ; 2CF4 mov bx,[0x2c]
db 0x4B                                 ; 2CF8 dec bx
db 0x4B                                 ; 2CF9 dec bx
db 0x89, 0x1E, 0x45, 0x03               ; 2CFA mov [0x345],bx
db 0xBA, 0x07, 0x00                     ; 2CFE mov dx,0x7
db 0xE9, 0xD4, 0xDA                     ; 2D01 jmp 0x7d8
db 0x39, 0x1E, 0x2F, 0x03               ; 2D04 cmp [0x32f],bx
db 0x73, 0xE9                           ; 2D08 jnc 0x2cf3
db 0x51                                 ; 2D0A push cx
db 0x52                                 ; 2D0B push dx
db 0x53                                 ; 2D0C push bx
db 0xE8, 0x06, 0xFA                     ; 2D0D call 0x2716
db 0x5B                                 ; 2D10 pop bx
db 0x5A                                 ; 2D11 pop dx
db 0x59                                 ; 2D12 pop cx
db 0x39, 0x1E, 0x2F, 0x03               ; 2D13 cmp [0x32f],bx
db 0x73, 0xDA                           ; 2D17 jnc 0x2cf3
db 0xEB, 0xE3                           ; 2D19 jmp short 0x2cfe
db 0x75, 0xD6                           ; 2D1B jnz 0x2cf3
db 0x8B, 0x1E, 0x30, 0x00               ; 2D1D mov bx,[0x30]
db 0xE8, 0x79, 0x01                     ; 2D21 call 0x2e9d
db 0xA2, 0x64, 0x04                     ; 2D24 mov [0x464],al
db 0xA2, 0x3E, 0x03                     ; 2D27 mov [0x33e],al
db 0xA2, 0x3D, 0x03                     ; 2D2A mov [0x33d],al
db 0x88, 0x07                           ; 2D2D mov [bx],al
db 0x43                                 ; 2D2F inc bx
db 0x88, 0x07                           ; 2D30 mov [bx],al
db 0x43                                 ; 2D32 inc bx
db 0x89, 0x1E, 0x58, 0x03               ; 2D33 mov [0x358],bx
db 0xCD, 0xAE                           ; 2D37 int 0xae
db 0x8B, 0x1E, 0x30, 0x00               ; 2D39 mov bx,[0x30]
db 0x4B                                 ; 2D3D dec bx
db 0xCD, 0xAF                           ; 2D3E int 0xaf
db 0x89, 0x1E, 0x3B, 0x03               ; 2D40 mov [0x33b],bx
db 0xA0, 0x65, 0x04                     ; 2D44 mov al,[0x465]
db 0x0A, 0xC0                           ; 2D47 or al,al
db 0x75, 0x17                           ; 2D49 jnz 0x2d62
db 0x32, 0xC0                           ; 2D4B xor al,al
db 0xA2, 0x5D, 0x04                     ; 2D4D mov [0x45d],al
db 0xA2, 0x5C, 0x04                     ; 2D50 mov [0x45c],al
db 0xB5, 0x1A                           ; 2D53 mov ch,0x1a
db 0xBB, 0x60, 0x03                     ; 2D55 mov bx,0x360
db 0xCD, 0xB0                           ; 2D58 int 0xb0
db 0xC6, 0x07, 0x04                     ; 2D5A mov byte [bx],0x4
db 0x43                                 ; 2D5D inc bx
db 0xFE, 0xCD                           ; 2D5E dec ch
db 0x75, 0xF8                           ; 2D60 jnz 0x2d5a
db 0xBA, 0x07, 0x00                     ; 2D62 mov dx,0x7
db 0xBB, 0x0B, 0x00                     ; 2D65 mov bx,0xb
db 0xE8, 0xC6, 0x37                     ; 2D68 call 0x6531
db 0x32, 0xC0                           ; 2D6B xor al,al
db 0xA2, 0x4F, 0x03                     ; 2D6D mov [0x34f],al
db 0x8A, 0xD8                           ; 2D70 mov bl,al
db 0x8A, 0xF8                           ; 2D72 mov bh,al
db 0x89, 0x1E, 0x4D, 0x03               ; 2D74 mov [0x34d],bx
db 0x89, 0x1E, 0x56, 0x03               ; 2D78 mov [0x356],bx
db 0x8B, 0x1E, 0x0A, 0x03               ; 2D7C mov bx,[0x30a]
db 0xA0, 0x6B, 0x04                     ; 2D80 mov al,[0x46b]
db 0x0A, 0xC0                           ; 2D83 or al,al
db 0x75, 0x04                           ; 2D85 jnz 0x2d8b
db 0x89, 0x1E, 0x2F, 0x03               ; 2D87 mov [0x32f],bx
db 0x32, 0xC0                           ; 2D8B xor al,al
db 0xE8, 0x7C, 0x00                     ; 2D8D call 0x2e0c
db 0x8B, 0x1E, 0x58, 0x03               ; 2D90 mov bx,[0x358]
db 0x89, 0x1E, 0x5A, 0x03               ; 2D94 mov [0x35a],bx
db 0x89, 0x1E, 0x5C, 0x03               ; 2D98 mov [0x35c],bx
db 0xA0, 0x65, 0x04                     ; 2D9C mov al,[0x465]
db 0x0A, 0xC0                           ; 2D9F or al,al
db 0x75, 0x03                           ; 2DA1 jnz 0x2da6
db 0xE8, 0xB4, 0x15                     ; 2DA3 call 0x435a
db 0xA0, 0x36, 0x05                     ; 2DA6 mov al,[0x536]
db 0x24, 0x01                           ; 2DA9 and al,0x1
db 0x75, 0x03                           ; 2DAB jnz 0x2db0
db 0xA2, 0x36, 0x05                     ; 2DAD mov [0x536],al
L_2DB0:
db 0x59                                 ; 2DB0 pop cx
db 0x8B, 0x1E, 0x2C, 0x00               ; 2DB1 mov bx,[0x2c]
db 0x4B                                 ; 2DB5 dec bx
db 0x4B                                 ; 2DB6 dec bx
db 0x89, 0x1E, 0x45, 0x03               ; 2DB7 mov [0x345],bx
db 0x43                                 ; 2DBB inc bx
db 0x43                                 ; 2DBC inc bx
db 0xCD, 0xB1                           ; 2DBD int 0xb1
db 0x8B, 0xE3                           ; 2DBF mov sp,bx
db 0xBB, 0x0E, 0x03                     ; 2DC1 mov bx,0x30e
db 0x89, 0x1E, 0x0C, 0x03               ; 2DC4 mov [0x30c],bx
db 0xE8, 0xF3, 0xF7                     ; 2DC8 call 0x25be
db 0xE8, 0xCA, 0xE6                     ; 2DCB call 0x1498
db 0x32, 0xC0                           ; 2DCE xor al,al
db 0x8A, 0xF8                           ; 2DD0 mov bh,al
db 0x8A, 0xD8                           ; 2DD2 mov bl,al
db 0x89, 0x1E, 0x7C, 0x03               ; 2DD4 mov [0x37c],bx
db 0xA2, 0x4D, 0x04                     ; 2DD8 mov [0x44d],al
db 0x89, 0x1E, 0xE4, 0x03               ; 2DDB mov [0x3e4],bx
db 0x89, 0x1E, 0x50, 0x04               ; 2DDF mov [0x450],bx
db 0x89, 0x1E, 0x7A, 0x03               ; 2DE3 mov [0x37a],bx
db 0xA2, 0x39, 0x03                     ; 2DE7 mov [0x339],al
db 0x53                                 ; 2DEA push bx
db 0x51                                 ; 2DEB push cx
db 0x8B, 0x1E, 0x3B, 0x03               ; 2DEC mov bx,[0x33b]
db 0xC3                                 ; 2DF0 ret
db 0x3B, 0xDA                           ; 2DF1 cmp bx,dx
db 0xC3                                 ; 2DF3 ret
db 0x5E                                 ; 2DF4 pop si
db 0x8B, 0xFB                           ; 2DF5 mov di,bx
db 0xFC                                 ; 2DF7 cld
db 0x2E, 0xA6                           ; 2DF8 cs cmpsb
db 0x56                                 ; 2DFA push si
db 0x8B, 0xDF                           ; 2DFB mov bx,di
db 0x75, 0x0A                           ; 2DFD jnz 0x2e09
db 0x8A, 0x07                           ; 2DFF mov al,[bx]
db 0x3C, 0x3A                           ; 2E01 cmp al,0x3a
db 0x72, 0x01                           ; 2E03 jc 0x2e06
db 0xC3                                 ; 2E05 ret
db 0xE9, 0x1C, 0xE1                     ; 2E06 jmp 0xf25
db 0xE9, 0xB2, 0xD9                     ; 2E09 jmp 0x7be
db 0x87, 0xDA                           ; 2E0C xchg bx,dx
db 0x8B, 0x1E, 0x30, 0x00               ; 2E0E mov bx,[0x30]
db 0x74, 0x11                           ; 2E12 jz 0x2e25
db 0x87, 0xDA                           ; 2E14 xchg bx,dx
db 0xE8, 0x52, 0xE2                     ; 2E16 call 0x106b
db 0x53                                 ; 2E19 push bx
db 0xE8, 0x4A, 0xDC                     ; 2E1A call 0xa67
db 0x8B, 0xD9                           ; 2E1D mov bx,cx
db 0x5A                                 ; 2E1F pop dx
db 0x72, 0x03                           ; 2E20 jc 0x2e25
db 0xE9, 0x0B, 0xE3                     ; 2E22 jmp 0x1130
db 0x4B                                 ; 2E25 dec bx
db 0x89, 0x1E, 0x5E, 0x03               ; 2E26 mov [0x35e],bx
db 0x87, 0xDA                           ; 2E2A xchg bx,dx
db 0xC3                                 ; 2E2C ret
db 0x75, 0xFD                           ; 2E2D jnz 0x2e2c
db 0xFE, 0xC0                           ; 2E2F inc al
db 0xEB, 0x09                           ; 2E31 jmp short 0x2e3c
db 0x75, 0xF7                           ; 2E33 jnz 0x2e2c
db 0x9C                                 ; 2E35 pushf
db 0x75, 0x03                           ; 2E36 jnz 0x2e3b
db 0xE8, 0x9F, 0x0A                     ; 2E38 call 0x38da
db 0x9D                                 ; 2E3B popf
db 0x89, 0x1E, 0x43, 0x03               ; 2E3C mov [0x343],bx
db 0xBB, 0x0E, 0x03                     ; 2E40 mov bx,0x30e
db 0x89, 0x1E, 0x0C, 0x03               ; 2E43 mov [0x30c],bx
db 0xBB, 0x0C, 0xFF                     ; 2E47 mov bx,0xff0c
db 0x59                                 ; 2E4A pop cx
db 0x8B, 0x1E, 0x2E, 0x00               ; 2E4B mov bx,[0x2e]
db 0x53                                 ; 2E4F push bx
db 0x9C                                 ; 2E50 pushf
db 0x8A, 0xC3                           ; 2E51 mov al,bl
db 0x22, 0xC7                           ; 2E53 and al,bh
db 0xFE, 0xC0                           ; 2E55 inc al
db 0x74, 0x0C                           ; 2E57 jz 0x2e65
db 0x89, 0x1E, 0x54, 0x03               ; 2E59 mov [0x354],bx
db 0x8B, 0x1E, 0x43, 0x03               ; 2E5D mov bx,[0x343]
db 0x89, 0x1E, 0x56, 0x03               ; 2E61 mov [0x356],bx
db 0xE8, 0xF5, 0xFD                     ; 2E65 call 0x2c5d
db 0x9D                                 ; 2E68 popf
db 0xBB, 0x32, 0x07                     ; 2E69 mov bx,0x732
db 0x74, 0x03                           ; 2E6C jz 0x2e71
db 0xE9, 0x14, 0xDA                     ; 2E6E jmp 0x885
db 0xE9, 0x41, 0xDA                     ; 2E71 jmp 0x8b5
db 0xB0, 0x0F                           ; 2E74 mov al,0xf
db 0x50                                 ; 2E76 push ax
db 0xB0, 0x5E                           ; 2E77 mov al,0x5e
db 0xE8, 0x29, 0xFD                     ; 2E79 call 0x2ba5
db 0x58                                 ; 2E7C pop ax
db 0x04, 0x40                           ; 2E7D add al,0x40
db 0xE8, 0x23, 0xFD                     ; 2E7F call 0x2ba5
db 0xE9, 0xEC, 0xFD                     ; 2E82 jmp 0x2c71
db 0x8B, 0x1E, 0x56, 0x03               ; 2E85 mov bx,[0x356]
db 0x0B, 0xDB                           ; 2E89 or bx,bx
db 0xBA, 0x11, 0x00                     ; 2E8B mov dx,0x11
db 0x75, 0x03                           ; 2E8E jnz 0x2e93
db 0xE9, 0x45, 0xD9                     ; 2E90 jmp 0x7d8
db 0x8B, 0x16, 0x54, 0x03               ; 2E93 mov dx,[0x354]
db 0x89, 0x16, 0x2E, 0x00               ; 2E97 mov [0x2e],dx
db 0xC3                                 ; 2E9B ret
db 0xB8, 0x32, 0xC0                     ; 2E9C mov ax,0xc032
db 0xA2, 0x76, 0x04                     ; 2E9F mov [0x476],al
db 0xC3                                 ; 2EA2 ret
db 0xE8, 0xC8, 0x08                     ; 2EA3 call 0x376e
db 0x52                                 ; 2EA6 push dx
db 0x53                                 ; 2EA7 push bx
db 0xBB, 0x6E, 0x04                     ; 2EA8 mov bx,0x46e
db 0xE8, 0x0B, 0x36                     ; 2EAB call 0x64b9
db 0x8B, 0x1E, 0x5A, 0x03               ; 2EAE mov bx,[0x35a]
db 0x5E                                 ; 2EB2 pop si
db 0x87, 0xDE                           ; 2EB3 xchg bx,si
db 0x56                                 ; 2EB5 push si
db 0xE8, 0x6C, 0xEC                     ; 2EB6 call 0x1b25
db 0x50                                 ; 2EB9 push ax
db 0xE8, 0x37, 0xFF                     ; 2EBA call 0x2df4
db 0x2C, 0xE8                           ; 2EBD sub al,0xe8
db 0xAD                                 ; 2EBF lodsw
db 0x08, 0x58, 0x8A                     ; 2EC0 or [bx+si-0x76],bl
db 0xE8, 0xE8, 0x5E                     ; 2EC3 call 0x8dae
db 0xEC                                 ; 2EC6 in al,dx
db 0x3A, 0xC5                           ; 2EC7 cmp al,ch
db 0x74, 0x03                           ; 2EC9 jz 0x2ece
db 0xE9, 0x08, 0xD9                     ; 2ECB jmp 0x7d6
db 0x5E                                 ; 2ECE pop si
db 0x87, 0xDE                           ; 2ECF xchg bx,si
db 0x56                                 ; 2ED1 push si
db 0x87, 0xDA                           ; 2ED2 xchg bx,dx
db 0x53                                 ; 2ED4 push bx
db 0x8B, 0x1E, 0x5A, 0x03               ; 2ED5 mov bx,[0x35a]
db 0x3B, 0xDA                           ; 2ED9 cmp bx,dx
db 0x75, 0x13                           ; 2EDB jnz 0x2ef0
db 0x5A                                 ; 2EDD pop dx
db 0x5B                                 ; 2EDE pop bx
db 0x5E                                 ; 2EDF pop si
db 0x87, 0xDE                           ; 2EE0 xchg bx,si
db 0x56                                 ; 2EE2 push si
db 0x52                                 ; 2EE3 push dx
db 0xE8, 0xD2, 0x35                     ; 2EE4 call 0x64b9
db 0x5B                                 ; 2EE7 pop bx
db 0xBA, 0x6E, 0x04                     ; 2EE8 mov dx,0x46e
db 0xE8, 0xCB, 0x35                     ; 2EEB call 0x64b9
db 0x5B                                 ; 2EEE pop bx
db 0xC3                                 ; 2EEF ret
db 0xE9, 0x66, 0xE1                     ; 2EF0 jmp 0x1059
db 0xB0, 0x01                           ; 2EF3 mov al,0x1
db 0xA2, 0x39, 0x03                     ; 2EF5 mov [0x339],al
db 0xE8, 0x73, 0x08                     ; 2EF8 call 0x376e
db 0x75, 0xF3                           ; 2EFB jnz 0x2ef0
db 0x53                                 ; 2EFD push bx
db 0xA2, 0x39, 0x03                     ; 2EFE mov [0x339],al
db 0x8A, 0xFD                           ; 2F01 mov bh,ch
db 0x8A, 0xD9                           ; 2F03 mov bl,cl
db 0x49                                 ; 2F05 dec cx
db 0x49                                 ; 2F06 dec cx
db 0x49                                 ; 2F07 dec cx
db 0x8B, 0xF1                           ; 2F08 mov si,cx
db 0xAC                                 ; 2F0A lodsb
db 0x49                                 ; 2F0B dec cx
db 0x0A, 0xC0                           ; 2F0C or al,al
db 0x78, 0xF8                           ; 2F0E js 0x2f08
db 0x49                                 ; 2F10 dec cx
db 0x49                                 ; 2F11 dec cx
db 0x03, 0xDA                           ; 2F12 add bx,dx
db 0x87, 0xDA                           ; 2F14 xchg bx,dx
db 0x8B, 0x1E, 0x5C, 0x03               ; 2F16 mov bx,[0x35c]
db 0x3B, 0xDA                           ; 2F1A cmp bx,dx
db 0x8B, 0xF2                           ; 2F1C mov si,dx
db 0xAC                                 ; 2F1E lodsb
db 0x8B, 0xF9                           ; 2F1F mov di,cx
db 0xAA                                 ; 2F21 stosb
db 0x9F                                 ; 2F22 lahf
db 0x42                                 ; 2F23 inc dx
db 0x9E                                 ; 2F24 sahf
db 0x9F                                 ; 2F25 lahf
db 0x41                                 ; 2F26 inc cx
db 0x9E                                 ; 2F27 sahf
db 0x75, 0xF0                           ; 2F28 jnz 0x2f1a
db 0x49                                 ; 2F2A dec cx
db 0x8B, 0xD9                           ; 2F2B mov bx,cx
db 0x89, 0x1E, 0x5C, 0x03               ; 2F2D mov [0x35c],bx
db 0x5B                                 ; 2F31 pop bx
db 0x8A, 0x07                           ; 2F32 mov al,[bx]
db 0x3C, 0x2C                           ; 2F34 cmp al,0x2c
db 0x75, 0xB7                           ; 2F36 jnz 0x2eef
db 0xE8, 0xE2, 0xDF                     ; 2F38 call 0xf1d
db 0xEB, 0xB6                           ; 2F3B jmp short 0x2ef3
db 0x58                                 ; 2F3D pop ax
db 0x86, 0xC4                           ; 2F3E xchg al,ah
db 0x9E                                 ; 2F40 sahf
db 0x5B                                 ; 2F41 pop bx
db 0xC3                                 ; 2F42 ret
db 0x8A, 0x07                           ; 2F43 mov al,[bx]
db 0x3C, 0x41                           ; 2F45 cmp al,0x41
db 0x72, 0xF9                           ; 2F47 jc 0x2f42
db 0x3C, 0x5B                           ; 2F49 cmp al,0x5b
db 0xF5                                 ; 2F4B cmc
db 0xC3                                 ; 2F4C ret
db 0xE9, 0xEE, 0xFD                     ; 2F4D jmp 0x2d3e
db 0x74, 0xFB                           ; 2F50 jz 0x2f4d
db 0x3C, 0x2C                           ; 2F52 cmp al,0x2c
db 0x74, 0x09                           ; 2F54 jz 0x2f5f
db 0xE8, 0xFB, 0xE0                     ; 2F56 call 0x1054
db 0x4B                                 ; 2F59 dec bx
db 0xE8, 0xC0, 0xDF                     ; 2F5A call 0xf1d
db 0x74, 0xEE                           ; 2F5D jz 0x2f4d
db 0xE8, 0x92, 0xFE                     ; 2F5F call 0x2df4
db 0x2C, 0x74                           ; 2F62 sub al,0x74
db 0xE8, 0x8B, 0x16                     ; 2F64 call 0x45f2
db 0x2C, 0x00                           ; 2F67 sub al,0x0
db 0x3C, 0x2C                           ; 2F69 cmp al,0x2c
db 0x74, 0x03                           ; 2F6B jz 0x2f70
db 0xE8, 0x47, 0x00                     ; 2F6D call 0x2fb7
db 0x4B                                 ; 2F70 dec bx
db 0xE8, 0xA9, 0xDF                     ; 2F71 call 0xf1d
db 0x52                                 ; 2F74 push dx
db 0x74, 0x4E                           ; 2F75 jz 0x2fc5
db 0xE8, 0x7A, 0xFE                     ; 2F77 call 0x2df4
db 0x2C, 0x74                           ; 2F7A sub al,0x74
db 0x48                                 ; 2F7C dec ax
db 0xE8, 0x37, 0x00                     ; 2F7D call 0x2fb7
db 0x4B                                 ; 2F80 dec bx
db 0xE8, 0x99, 0xDF                     ; 2F81 call 0xf1d
db 0x74, 0x03                           ; 2F84 jz 0x2f89
db 0xE9, 0x35, 0xD8                     ; 2F86 jmp 0x7be
db 0x5E                                 ; 2F89 pop si
db 0x87, 0xDE                           ; 2F8A xchg bx,si
db 0x56                                 ; 2F8C push si
db 0x53                                 ; 2F8D push bx
db 0xBB, 0xEE, 0x00                     ; 2F8E mov bx,0xee
db 0x3B, 0xDA                           ; 2F91 cmp bx,dx
db 0x73, 0x2D                           ; 2F93 jnc 0x2fc2
db 0x5B                                 ; 2F95 pop bx
db 0xE8, 0x36, 0x00                     ; 2F96 call 0x2fcf
db 0x72, 0x27                           ; 2F99 jc 0x2fc2
db 0x53                                 ; 2F9B push bx
db 0x8B, 0x1E, 0x58, 0x03               ; 2F9C mov bx,[0x358]
db 0xB9, 0x14, 0x00                     ; 2FA0 mov cx,0x14
db 0x03, 0xD9                           ; 2FA3 add bx,cx
db 0x3B, 0xDA                           ; 2FA5 cmp bx,dx
db 0x73, 0x19                           ; 2FA7 jnc 0x2fc2
db 0x87, 0xDA                           ; 2FA9 xchg bx,dx
db 0x89, 0x1E, 0x0A, 0x03               ; 2FAB mov [0x30a],bx
db 0x5B                                 ; 2FAF pop bx
db 0x89, 0x1E, 0x2C, 0x00               ; 2FB0 mov [0x2c],bx
db 0x5B                                 ; 2FB4 pop bx
db 0xEB, 0x96                           ; 2FB5 jmp short 0x2f4d
db 0xE8, 0xF0, 0xF2                     ; 2FB7 call 0x22aa
db 0x0B, 0xD2                           ; 2FBA or dx,dx
db 0x75, 0x03                           ; 2FBC jnz 0x2fc1
db 0xE9, 0x98, 0xE0                     ; 2FBE jmp 0x1059
db 0xC3                                 ; 2FC1 ret
db 0xE9, 0x2F, 0xFD                     ; 2FC2 jmp 0x2cf4
db 0x8B, 0x16, 0x2C, 0x00               ; 2FC5 mov dx,[0x2c]
db 0x2B, 0x16, 0x0A, 0x03               ; 2FC9 sub dx,[0x30a]
db 0xEB, 0xBA                           ; 2FCD jmp short 0x2f89
db 0x8B, 0xC3                           ; 2FCF mov ax,bx
db 0x2B, 0xC2                           ; 2FD1 sub ax,dx
db 0x8B, 0xD0                           ; 2FD3 mov dx,ax
db 0xC3                                 ; 2FD5 ret
db 0xCD, 0xB2                           ; 2FD6 int 0xb2
db 0x53                                 ; 2FD8 push bx
db 0x8B, 0x1E, 0xE9, 0x04               ; 2FD9 mov bx,[0x4e9]
db 0x0B, 0xDB                           ; 2FDD or bx,bx
db 0x5B                                 ; 2FDF pop bx
db 0xC3                                 ; 2FE0 ret
db 0x01, 0x30                           ; 2FE1 add [bx+si],si
db 0x4E                                 ; 2FE3 dec si
db 0x30, 0xE1                           ; 2FE4 xor cl,ah
db 0x30, 0xFD                           ; 2FE6 xor ch,bh
db 0x2F                                 ; 2FE8 das
db 0xF1                                 ; 2FE9 int1
db 0x2F                                 ; 2FEA das
db 0x5B                                 ; 2FEB pop bx
db 0x30, 0x30                           ; 2FEC xor [bx+si],dh
db 0x30, 0x1B                           ; 2FEE xor [bp+di],bl
db 0x30, 0x8B, 0x1E, 0x56               ; 2FF0 xor [bp+di+0x561e],cl
db 0x00, 0xE8                           ; 2FF4 add al,ch
db 0x4A                                 ; 2FF6 dec dx
db 0x00, 0x74, 0x01                     ; 2FF7 add [si+0x1],dh
db 0xC3                                 ; 2FFA ret
db 0xEB, 0x04                           ; 2FFB jmp short 0x3001
db 0xB0, 0x01                           ; 2FFD mov al,0x1
db 0xEB, 0x02                           ; 2FFF jmp short 0x3003
db 0xB0, 0xFF                           ; 3001 mov al,0xff
db 0xA2, 0x70, 0x00                     ; 3003 mov [0x70],al
db 0xFE, 0xC0                           ; 3006 inc al
db 0xE8, 0x65, 0x01                     ; 3008 call 0x3170
db 0xB7, 0x01                           ; 300B mov bh,0x1
db 0xE8, 0x0B, 0x00                     ; 300D call 0x301b
db 0x75, 0xE8                           ; 3010 jnz 0x2ffa
db 0xE8, 0x0B, 0x1F                     ; 3012 call 0x4f20
db 0xE8, 0x5E, 0x00                     ; 3015 call 0x3076
db 0x32, 0xC0                           ; 3018 xor al,al
db 0xC3                                 ; 301A ret
db 0xA0, 0x5C, 0x00                     ; 301B mov al,[0x5c]
db 0x3A, 0xC3                           ; 301E cmp al,bl
db 0x74, 0xF8                           ; 3020 jz 0x301a
db 0x73, 0x07                           ; 3022 jnc 0x302b
db 0x8A, 0xD8                           ; 3024 mov bl,al
db 0x32, 0xC0                           ; 3026 xor al,al
db 0xE9, 0xF5, 0x1E                     ; 3028 jmp 0x4f20
db 0xFE, 0xC3                           ; 302B inc bl
db 0xE9, 0xF0, 0x1E                     ; 302D jmp 0x4f20
db 0xA0, 0x5B, 0x00                     ; 3030 mov al,[0x5b]
db 0x3A, 0xC3                           ; 3033 cmp al,bl
db 0x74, 0xE3                           ; 3035 jz 0x301a
db 0xB0, 0x01                           ; 3037 mov al,0x1
db 0x3A, 0xC3                           ; 3039 cmp al,bl
db 0x74, 0xDD                           ; 303B jz 0x301a
db 0xFE, 0xCB                           ; 303D dec bl
db 0xE9, 0xDE, 0x1E                     ; 303F jmp 0x4f20
db 0xA0, 0x29, 0x00                     ; 3042 mov al,[0x29]
db 0x3A, 0xC7                           ; 3045 cmp al,bh
db 0x74, 0xD1                           ; 3047 jz 0x301a
db 0xFE, 0xC7                           ; 3049 inc bh
db 0xE9, 0xD2, 0x1E                     ; 304B jmp 0x4f20
db 0x8B, 0x1E, 0x5B, 0x00               ; 304E mov bx,[0x5b]
db 0xB7, 0x01                           ; 3052 mov bh,0x1
db 0x89, 0x1E, 0x58, 0x00               ; 3054 mov [0x58],bx
db 0xE9, 0xC5, 0x1E                     ; 3058 jmp 0x4f20
db 0x8B, 0x1E, 0x56, 0x00               ; 305B mov bx,[0x56]
db 0xE8, 0x09, 0x00                     ; 305F call 0x306b
db 0x75, 0xB6                           ; 3062 jnz 0x301a
db 0xA0, 0x29, 0x00                     ; 3064 mov al,[0x29]
db 0x8A, 0xF8                           ; 3067 mov bh,al
db 0xEB, 0xC5                           ; 3069 jmp short 0x3030
db 0xB0, 0x01                           ; 306B mov al,0x1
db 0x3A, 0xC7                           ; 306D cmp al,bh
db 0x74, 0xA9                           ; 306F jz 0x301a
db 0xFE, 0xCF                           ; 3071 dec bh
db 0xE9, 0xAA, 0x1E                     ; 3073 jmp 0x4f20
db 0xA0, 0x5B, 0x00                     ; 3076 mov al,[0x5b]
db 0x8A, 0xF8                           ; 3079 mov bh,al
db 0xA0, 0x5C, 0x00                     ; 307B mov al,[0x5c]
db 0x8A, 0xD8                           ; 307E mov bl,al
db 0x2A, 0xC7                           ; 3080 sub al,bh
db 0x72, 0x96                           ; 3082 jc 0x301a
db 0xFE, 0xC0                           ; 3084 inc al
db 0x50                                 ; 3086 push ax
db 0xE8, 0x99, 0x00                     ; 3087 call 0x3123
db 0xA0, 0x58, 0x00                     ; 308A mov al,[0x58]
db 0xFE, 0xC3                           ; 308D inc bl
db 0x3A, 0xC3                           ; 308F cmp al,bl
db 0xFE, 0xCB                           ; 3091 dec bl
db 0x73, 0x0D                           ; 3093 jnc 0x30a2
db 0x3A, 0xC7                           ; 3095 cmp al,bh
db 0x72, 0x09                           ; 3097 jc 0x30a2
db 0x75, 0x02                           ; 3099 jnz 0x309d
db 0xB0, 0x01                           ; 309B mov al,0x1
db 0xFE, 0xC8                           ; 309D dec al
db 0xA2, 0x58, 0x00                     ; 309F mov [0x58],al
db 0x58                                 ; 30A2 pop ax
db 0xFE, 0xC8                           ; 30A3 dec al
db 0x75, 0x03                           ; 30A5 jnz 0x30aa
db 0xE9, 0x03, 0x00                     ; 30A7 jmp 0x30ad
db 0xE8, 0xDE, 0x1E                     ; 30AA call 0x4f8b
db 0xC3                                 ; 30AD ret
db 0xA0, 0x5B, 0x00                     ; 30AE mov al,[0x5b]
db 0x8A, 0xD8                           ; 30B1 mov bl,al
db 0xA0, 0x5C, 0x00                     ; 30B3 mov al,[0x5c]
db 0x8A, 0xF8                           ; 30B6 mov bh,al
db 0x2A, 0xC3                           ; 30B8 sub al,bl
db 0x72, 0xF1                           ; 30BA jc 0x30ad
db 0xFE, 0xC0                           ; 30BC inc al
db 0x50                                 ; 30BE push ax
db 0xE8, 0x7F, 0x00                     ; 30BF call 0x3141
db 0xA0, 0x58, 0x00                     ; 30C2 mov al,[0x58]
db 0x3A, 0xC3                           ; 30C5 cmp al,bl
db 0x72, 0x10                           ; 30C7 jc 0x30d9
db 0x3A, 0xC7                           ; 30C9 cmp al,bh
db 0x78, 0x03                           ; 30CB js 0x30d0
db 0xE9, 0x09, 0x00                     ; 30CD jmp 0x30d9
db 0x75, 0x02                           ; 30D0 jnz 0x30d4
db 0xB0, 0xFF                           ; 30D2 mov al,0xff
db 0xFE, 0xC0                           ; 30D4 inc al
db 0xA2, 0x58, 0x00                     ; 30D6 mov [0x58],al
db 0x58                                 ; 30D9 pop ax
db 0xFE, 0xC8                           ; 30DA dec al
db 0x74, 0xCF                           ; 30DC jz 0x30ad
db 0xE9, 0xBB, 0x1E                     ; 30DE jmp 0x4f9c
db 0x8B, 0x1E, 0x5B, 0x00               ; 30E1 mov bx,[0x5b]
db 0xA0, 0x5D, 0x00                     ; 30E5 mov al,[0x5d]
db 0x8A, 0xE8                           ; 30E8 mov ch,al
db 0x8A, 0xC5                           ; 30EA mov al,ch
db 0x3A, 0xC3                           ; 30EC cmp al,bl
db 0x73, 0x02                           ; 30EE jnc 0x30f2
db 0x8A, 0xD8                           ; 30F0 mov bl,al
db 0x3A, 0xC7                           ; 30F2 cmp al,bh
db 0x73, 0x02                           ; 30F4 jnc 0x30f8
db 0x8A, 0xF8                           ; 30F6 mov bh,al
db 0x8A, 0xC7                           ; 30F8 mov al,bh
db 0xB7, 0x00                           ; 30FA mov bh,0x0
db 0x2A, 0xC3                           ; 30FC sub al,bl
db 0xFE, 0xC0                           ; 30FE inc al
db 0xBA, 0x72, 0x00                     ; 3100 mov dx,0x72
db 0x50                                 ; 3103 push ax
db 0x87, 0xDA                           ; 3104 xchg bx,dx
db 0x03, 0xDA                           ; 3106 add bx,dx
db 0x88, 0x07                           ; 3108 mov [bx],al
db 0x43                                 ; 310A inc bx
db 0x88, 0x07                           ; 310B mov [bx],al
db 0x43                                 ; 310D inc bx
db 0xFE, 0xC8                           ; 310E dec al
db 0x75, 0xF9                           ; 3110 jnz 0x310b
db 0x87, 0xDA                           ; 3112 xchg bx,dx
db 0x58                                 ; 3114 pop ax
db 0x32, 0xC0                           ; 3115 xor al,al
db 0xA2, 0x58, 0x00                     ; 3117 mov [0x58],al
db 0xA2, 0x59, 0x00                     ; 311A mov [0x59],al
db 0xA2, 0x5A, 0x00                     ; 311D mov [0x5a],al
db 0xE9, 0x2B, 0xFF                     ; 3120 jmp 0x304e
db 0x50                                 ; 3123 push ax
db 0xE8, 0x37, 0x00                     ; 3124 call 0x315e
db 0xB5, 0x01                           ; 3127 mov ch,0x1
db 0x8A, 0xC8                           ; 3129 mov cl,al
db 0x8A, 0xC5                           ; 312B mov al,ch
db 0x8B, 0xFA                           ; 312D mov di,dx
db 0xAA                                 ; 312F stosb
db 0x4A                                 ; 3130 dec dx
db 0x8B, 0xF2                           ; 3131 mov si,dx
db 0xAC                                 ; 3133 lodsb
db 0x8A, 0xE9                           ; 3134 mov ch,cl
db 0x8A, 0xC8                           ; 3136 mov cl,al
db 0x58                                 ; 3138 pop ax
db 0xFE, 0xC8                           ; 3139 dec al
db 0x75, 0x01                           ; 313B jnz 0x313e
db 0xC3                                 ; 313D ret
db 0x50                                 ; 313E push ax
db 0xEB, 0xEA                           ; 313F jmp short 0x312b
db 0x50                                 ; 3141 push ax
db 0xB5, 0x01                           ; 3142 mov ch,0x1
db 0xE8, 0x17, 0x00                     ; 3144 call 0x315e
db 0x8A, 0xC8                           ; 3147 mov cl,al
db 0x8A, 0xC5                           ; 3149 mov al,ch
db 0x8B, 0xFA                           ; 314B mov di,dx
db 0xAA                                 ; 314D stosb
db 0x42                                 ; 314E inc dx
db 0x8B, 0xF2                           ; 314F mov si,dx
db 0xAC                                 ; 3151 lodsb
db 0x8A, 0xE9                           ; 3152 mov ch,cl
db 0x8A, 0xC8                           ; 3154 mov cl,al
db 0x58                                 ; 3156 pop ax
db 0xFE, 0xC8                           ; 3157 dec al
db 0x74, 0xE2                           ; 3159 jz 0x313d
db 0x50                                 ; 315B push ax
db 0xEB, 0xEB                           ; 315C jmp short 0x3149
db 0x53                                 ; 315E push bx
db 0xBA, 0x74, 0x00                     ; 315F mov dx,0x74
db 0xB7, 0x00                           ; 3162 mov bh,0x0
db 0xFE, 0xCB                           ; 3164 dec bl
db 0x03, 0xDA                           ; 3166 add bx,dx
db 0x8A, 0x07                           ; 3168 mov al,[bx]
db 0x87, 0xDA                           ; 316A xchg bx,dx
db 0x5B                                 ; 316C pop bx
db 0x22, 0xC0                           ; 316D and al,al
db 0xC3                                 ; 316F ret
db 0x53                                 ; 3170 push bx
db 0xBA, 0x74, 0x00                     ; 3171 mov dx,0x74
db 0xB7, 0x00                           ; 3174 mov bh,0x0
db 0xFE, 0xCB                           ; 3176 dec bl
db 0x03, 0xDA                           ; 3178 add bx,dx
db 0x88, 0x07                           ; 317A mov [bx],al
db 0x87, 0xDA                           ; 317C xchg bx,dx
db 0x5B                                 ; 317E pop bx
db 0xC3                                 ; 317F ret
db 0xCD, 0xA6                           ; 3180 int 0xa6
db 0xE8, 0x41, 0x01                     ; 3182 call 0x32c6
db 0x8B, 0x1E, 0x56, 0x00               ; 3185 mov bx,[0x56]
db 0x89, 0x1E, 0x58, 0x00               ; 3189 mov [0x58],bx
db 0xA0, 0x29, 0x00                     ; 318D mov al,[0x29]
db 0xFE, 0xC0                           ; 3190 inc al
db 0xEB, 0x1E                           ; 3192 jmp short 0x31b2
db 0xB0, 0x3F                           ; 3194 mov al,0x3f
db 0xE8, 0x0C, 0xFA                     ; 3196 call 0x2ba5
db 0xB0, 0x20                           ; 3199 mov al,0x20
db 0xE8, 0x07, 0xFA                     ; 319B call 0x2ba5
db 0x32, 0xC0                           ; 319E xor al,al
db 0xA2, 0x27, 0x00                     ; 31A0 mov [0x27],al
db 0xCD, 0xA7                           ; 31A3 int 0xa7
db 0xE8, 0x1E, 0x01                     ; 31A5 call 0x32c6
db 0x8B, 0x1E, 0x56, 0x00               ; 31A8 mov bx,[0x56]
db 0x89, 0x1E, 0x58, 0x00               ; 31AC mov [0x58],bx
db 0x8A, 0xC7                           ; 31B0 mov al,bh
db 0xA2, 0x5A, 0x00                     ; 31B2 mov [0x5a],al
db 0xE8, 0xE1, 0x04                     ; 31B5 call 0x3699
db 0xFE, 0xCB                           ; 31B8 dec bl
db 0x74, 0x05                           ; 31BA jz 0x31c1
db 0xB0, 0x01                           ; 31BC mov al,0x1
db 0xE8, 0xAF, 0xFF                     ; 31BE call 0x3170
db 0xE8, 0x0C, 0x23                     ; 31C1 call 0x54d0
db 0xE8, 0x41, 0x23                     ; 31C4 call 0x5508
db 0xE8, 0xA4, 0x1B                     ; 31C7 call 0x4d6e
db 0xE8, 0x08, 0x23                     ; 31CA call 0x54d5
db 0xE8, 0x38, 0x23                     ; 31CD call 0x5508
db 0x0A, 0xC0                           ; 31D0 or al,al
db 0x75, 0x03                           ; 31D2 jnz 0x31d7
db 0xE8, 0x77, 0x1C                     ; 31D4 call 0x4e4e
db 0x50                                 ; 31D7 push ax
db 0x8B, 0x1E, 0x56, 0x00               ; 31D8 mov bx,[0x56]
db 0x8A, 0x26, 0x29, 0x00               ; 31DC mov ah,[0x29]
db 0xFE, 0xC4                           ; 31E0 inc ah
db 0x3A, 0x1E, 0x58, 0x00               ; 31E2 cmp bl,[0x58]
db 0x75, 0x12                           ; 31E6 jnz 0x31fa
db 0x3A, 0x3E, 0x59, 0x00               ; 31E8 cmp bh,[0x59]
db 0x73, 0x04                           ; 31EC jnc 0x31f2
db 0x88, 0x3E, 0x59, 0x00               ; 31EE mov [0x59],bh
db 0x3A, 0x3E, 0x5A, 0x00               ; 31F2 cmp bh,[0x5a]
db 0x76, 0x06                           ; 31F6 jna 0x31fe
db 0x8A, 0xE7                           ; 31F8 mov ah,bh
db 0x88, 0x26, 0x5A, 0x00               ; 31FA mov [0x5a],ah
db 0x58                                 ; 31FE pop ax
db 0xE8, 0x2F, 0x00                     ; 31FF call 0x3231
db 0x72, 0x10                           ; 3202 jc 0x3214
db 0x74, 0xBB                           ; 3204 jz 0x31c1
db 0xE8, 0x03, 0x02                     ; 3206 call 0x340c
db 0xE8, 0x99, 0xF9                     ; 3209 call 0x2ba5
db 0xEB, 0xB3                           ; 320C jmp short 0x31c1
db 0x01, 0xE8                           ; 320E add ax,bp
db 0x93                                 ; 3210 xchg ax,bx
db 0xF9                                 ; 3211 stc
db 0xEB, 0xAD                           ; 3212 jmp short 0x31c1
db 0x3C, 0x03                           ; 3214 cmp al,0x3
db 0xF9                                 ; 3216 stc
db 0x74, 0x01                           ; 3217 jz 0x321a
db 0xF5                                 ; 3219 cmc
db 0xBB, 0xF6, 0x01                     ; 321A mov bx,0x1f6
db 0xC3                                 ; 321D ret
db 0x3C, 0x3B                           ; 321E cmp al,0x3b
db 0x75, 0xFB                           ; 3220 jnz 0x321d
db 0xE9, 0xF8, 0xDC                     ; 3222 jmp 0xf1d
db 0x4B                                 ; 3225 dec bx
db 0x43                                 ; 3226 inc bx
db 0xFE, 0xC9                           ; 3227 dec cl
db 0x78, 0xF2                           ; 3229 js 0x321d
db 0x2E, 0x3A, 0x07                     ; 322B cmp al,[cs:bx]
db 0x75, 0xF6                           ; 322E jnz 0x3226
db 0xC3                                 ; 3230 ret
db 0xBB, 0x94, 0x32                     ; 3231 mov bx,0x3294
db 0xB1, 0x0E                           ; 3234 mov cl,0xe
db 0xE8, 0xEC, 0xFF                     ; 3236 call 0x3225
db 0x79, 0x03                           ; 3239 jns 0x323e
db 0xE9, 0x07, 0x00                     ; 323B jmp 0x3245
db 0x50                                 ; 323E push ax
db 0x32, 0xC0                           ; 323F xor al,al
db 0xA2, 0x72, 0x00                     ; 3241 mov [0x72],al
db 0x58                                 ; 3244 pop ax
db 0xBB, 0xA2, 0x32                     ; 3245 mov bx,0x32a2
db 0xB1, 0x0C                           ; 3248 mov cl,0xc
db 0xE8, 0xD8, 0xFF                     ; 324A call 0x3225
db 0x79, 0x03                           ; 324D jns 0x3252
db 0xE9, 0x20, 0x00                     ; 324F jmp 0x3272
db 0x50                                 ; 3252 push ax
db 0x8A, 0xC1                           ; 3253 mov al,cl
db 0x0A, 0xC0                           ; 3255 or al,al
db 0xD0, 0xC0                           ; 3257 rol al,1
db 0x8A, 0xC8                           ; 3259 mov cl,al
db 0x32, 0xC0                           ; 325B xor al,al
db 0x8A, 0xE8                           ; 325D mov ch,al
db 0xBB, 0xAE, 0x32                     ; 325F mov bx,0x32ae
db 0x03, 0xD9                           ; 3262 add bx,cx
db 0x2E, 0x8A, 0x17                     ; 3264 mov dl,[cs:bx]
db 0x43                                 ; 3267 inc bx
db 0x2E, 0x8A, 0x37                     ; 3268 mov dh,[cs:bx]
db 0x58                                 ; 326B pop ax
db 0x52                                 ; 326C push dx
db 0x8B, 0x1E, 0x56, 0x00               ; 326D mov bx,[0x56]
db 0xC3                                 ; 3271 ret
db 0x0A, 0xC0                           ; 3272 or al,al
db 0xC3                                 ; 3274 ret
db 0x03, 0x22                           ; 3275 add sp,[bp+si]
db 0xC0, 0xC3, 0x8B                     ; 3277 rol bl,0x8b
db 0x1E                                 ; 327A push ds
db 0x56                                 ; 327B push si
db 0x00, 0x80, 0xFF, 0x01               ; 327C add [bx+si+0x1ff],al
db 0x75, 0x0B                           ; 3280 jnz 0x328d
db 0xFE, 0xCB                           ; 3282 dec bl
db 0xE8, 0xD7, 0xFE                     ; 3284 call 0x315e
db 0x75, 0x07                           ; 3287 jnz 0x3290
db 0x8A, 0x3E, 0x29, 0x00               ; 3289 mov bh,[0x29]
db 0xE8, 0x90, 0x1C                     ; 328D call 0x4f20
db 0xE9, 0xDE, 0xF9                     ; 3290 jmp 0x2c71
db 0xC3                                 ; 3293 ret
db 0x0D, 0x02, 0x06                     ; 3294 or ax,0x602
db 0x05, 0x03, 0x0B                     ; 3297 add ax,0xb03
db 0x0C, 0x1C                           ; 329A or al,0x1c
db 0x1D, 0x1E, 0x1F                     ; 329C sbb ax,0x1f1e
db 0x0E                                 ; 329F push cs
db 0x7F, 0x1B                           ; 32A0 jg 0x32bd
db 0x09, 0x0A                           ; 32A2 or [bp+si],cx
db 0x08, 0x12                           ; 32A4 or [bp+si],dl
db 0x02, 0x06, 0x05, 0x03               ; 32A6 add al,[0x305]
db 0x0D, 0x0E, 0x7F                     ; 32AA or ax,0x7f0e
db 0x1B, 0xFF                           ; 32AD sbb di,di
db 0x34, 0x56                           ; 32AF xor al,0x56
db 0x34, 0xCA                           ; 32B1 xor al,0xca
db 0x34, 0x39                           ; 32B3 xor al,0x39
db 0x33, 0xAF, 0x33, 0x07               ; 32B5 xor bp,[bx+0x733]
db 0x35, 0x25, 0x35                     ; 32B9 xor ax,0x3525
db 0x4D                                 ; 32BC dec bp
db 0x35, 0x01, 0x34                     ; 32BD xor ax,0x3401
db 0x77, 0x34                           ; 32C0 ja 0x32f6
db 0xF3, 0x32, 0xC1                     ; 32C2 rep xor al,cl
db 0x33, 0xE8                           ; 32C5 xor bp,ax
db 0x0D, 0xFD, 0x74                     ; 32C7 or ax,0x74fd
db 0xC8, 0x58, 0xB5, 0xFE               ; 32CA enter 0xb558,0xfe
db 0xBB, 0xF7, 0x01                     ; 32CE mov bx,0x1f7
db 0xE8, 0x41, 0xF9                     ; 32D1 call 0x2c15
db 0x88, 0x07                           ; 32D4 mov [bx],al
db 0x3C, 0x0D                           ; 32D6 cmp al,0xd
db 0x74, 0x11                           ; 32D8 jz 0x32eb
db 0x3C, 0x0A                           ; 32DA cmp al,0xa
db 0x75, 0x06                           ; 32DC jnz 0x32e4
db 0x8A, 0xC5                           ; 32DE mov al,ch
db 0x3C, 0xFE                           ; 32E0 cmp al,0xfe
db 0x74, 0xED                           ; 32E2 jz 0x32d1
db 0x43                                 ; 32E4 inc bx
db 0xFE, 0xCD                           ; 32E5 dec ch
db 0x75, 0xE8                           ; 32E7 jnz 0x32d1
db 0xFE, 0xCD                           ; 32E9 dec ch
db 0x32, 0xC0                           ; 32EB xor al,al
db 0x88, 0x07                           ; 32ED mov [bx],al
db 0xBB, 0xF6, 0x01                     ; 32EF mov bx,0x1f6
db 0xC3                                 ; 32F2 ret
db 0xA0, 0x72, 0x00                     ; 32F3 mov al,[0x72]
db 0x0A, 0xC0                           ; 32F6 or al,al
db 0x74, 0x3A                           ; 32F8 jz 0x3334
db 0xE8, 0x61, 0xFE                     ; 32FA call 0x315e
db 0x50                                 ; 32FD push ax
db 0x87, 0xDA                           ; 32FE xchg bx,dx
db 0xC6, 0x07, 0x00                     ; 3300 mov byte [bx],0x0
db 0x87, 0xDA                           ; 3303 xchg bx,dx
db 0xFE, 0xC3                           ; 3305 inc bl
db 0xE8, 0x29, 0x03                     ; 3307 call 0x3633
db 0xA0, 0x29, 0x00                     ; 330A mov al,[0x29]
db 0x2A, 0xC7                           ; 330D sub al,bh
db 0x74, 0x0B                           ; 330F jz 0x331c
db 0xFE, 0xC0                           ; 3311 inc al
db 0x50                                 ; 3313 push ax
db 0xE8, 0xF5, 0x00                     ; 3314 call 0x340c
db 0x58                                 ; 3317 pop ax
db 0xFE, 0xC8                           ; 3318 dec al
db 0x75, 0xF7                           ; 331A jnz 0x3313
db 0x8B, 0x1E, 0x56, 0x00               ; 331C mov bx,[0x56]
db 0xE8, 0x3B, 0xFE                     ; 3320 call 0x315e
db 0x58                                 ; 3323 pop ax
db 0x8A, 0xC8                           ; 3324 mov cl,al
db 0x32, 0xC0                           ; 3326 xor al,al
db 0x8B, 0xFA                           ; 3328 mov di,dx
db 0xAA                                 ; 332A stosb
db 0x42                                 ; 332B inc dx
db 0x8A, 0xC1                           ; 332C mov al,cl
db 0x8B, 0xFA                           ; 332E mov di,dx
db 0xAA                                 ; 3330 stosb
db 0x32, 0xC0                           ; 3331 xor al,al
db 0xC3                                 ; 3333 ret
db 0xB0, 0x0A                           ; 3334 mov al,0xa
db 0x0A, 0xC0                           ; 3336 or al,al
db 0xC3                                 ; 3338 ret
db 0xE8, 0x5A, 0x02                     ; 3339 call 0x3596
db 0xBA, 0xF7, 0x01                     ; 333C mov dx,0x1f7
db 0xB5, 0xFE                           ; 333F mov ch,0xfe
db 0xA0, 0x58, 0x00                     ; 3341 mov al,[0x58]
db 0x3A, 0xC3                           ; 3344 cmp al,bl
db 0xB7, 0x01                           ; 3346 mov bh,0x1
db 0xA0, 0x29, 0x00                     ; 3348 mov al,[0x29]
db 0x75, 0x13                           ; 334B jnz 0x3360
db 0x8B, 0x1E, 0x58, 0x00               ; 334D mov bx,[0x58]
db 0x52                                 ; 3351 push dx
db 0xE8, 0x09, 0xFE                     ; 3352 call 0x315e
db 0x5A                                 ; 3355 pop dx
db 0xA0, 0x29, 0x00                     ; 3356 mov al,[0x29]
db 0x74, 0x05                           ; 3359 jz 0x3360
db 0xA0, 0x5A, 0x00                     ; 335B mov al,[0x5a]
db 0xFE, 0xC8                           ; 335E dec al
db 0xA2, 0x5A, 0x00                     ; 3360 mov [0x5a],al
db 0xE8, 0x3C, 0x02                     ; 3363 call 0x35a2
db 0x8A, 0xC5                           ; 3366 mov al,ch
db 0x22, 0xC0                           ; 3368 and al,al
db 0x74, 0x10                           ; 336A jz 0x337c
db 0x52                                 ; 336C push dx
db 0xE8, 0xEE, 0xFD                     ; 336D call 0x315e
db 0x5A                                 ; 3370 pop dx
db 0x75, 0x09                           ; 3371 jnz 0x337c
db 0xB7, 0x01                           ; 3373 mov bh,0x1
db 0xFE, 0xC3                           ; 3375 inc bl
db 0xA0, 0x29, 0x00                     ; 3377 mov al,[0x29]
db 0xEB, 0xE4                           ; 337A jmp short 0x3360
db 0x87, 0xDA                           ; 337C xchg bx,dx
db 0xB0, 0xFE                           ; 337E mov al,0xfe
db 0x2A, 0xC6                           ; 3380 sub al,dh
db 0x8A, 0xF0                           ; 3382 mov dh,al
db 0x4B                                 ; 3384 dec bx
db 0x8A, 0x07                           ; 3385 mov al,[bx]
db 0x3C, 0x20                           ; 3387 cmp al,0x20
db 0x74, 0x08                           ; 3389 jz 0x3393
db 0x0A, 0xC0                           ; 338B or al,al
db 0x75, 0x07                           ; 338D jnz 0x3396
db 0xFE, 0xCE                           ; 338F dec dh
db 0x74, 0x03                           ; 3391 jz 0x3396
db 0x4B                                 ; 3393 dec bx
db 0xEB, 0xEF                           ; 3394 jmp short 0x3385
db 0x43                                 ; 3396 inc bx
db 0xC6, 0x07, 0x00                     ; 3397 mov byte [bx],0x0
db 0x87, 0xDA                           ; 339A xchg bx,dx
db 0xB0, 0x0D                           ; 339C mov al,0xd
db 0x50                                 ; 339E push ax
db 0xB7, 0x01                           ; 339F mov bh,0x1
db 0xE8, 0x7C, 0x1B                     ; 33A1 call 0x4f20
db 0xB0, 0x0D                           ; 33A4 mov al,0xd
db 0xE8, 0xFC, 0xF7                     ; 33A6 call 0x2ba5
db 0xBB, 0xF6, 0x01                     ; 33A9 mov bx,0x1f6
db 0x58                                 ; 33AC pop ax
db 0xF9                                 ; 33AD stc
db 0xC3                                 ; 33AE ret
db 0x32, 0xC0                           ; 33AF xor al,al
db 0xA2, 0xF7, 0x01                     ; 33B1 mov [0x1f7],al
db 0xE8, 0xA7, 0xFD                     ; 33B4 call 0x315e
db 0x75, 0x04                           ; 33B7 jnz 0x33bd
db 0xFE, 0xC3                           ; 33B9 inc bl
db 0xEB, 0xF7                           ; 33BB jmp short 0x33b4
db 0xB0, 0x03                           ; 33BD mov al,0x3
db 0xEB, 0xDD                           ; 33BF jmp short 0x339e
db 0x8A, 0xC7                           ; 33C1 mov al,bh
db 0xFE, 0xC8                           ; 33C3 dec al
db 0x24, 0xF8                           ; 33C5 and al,0xf8
db 0x04, 0x08                           ; 33C7 add al,0x8
db 0xFE, 0xC0                           ; 33C9 inc al
db 0x8A, 0xE8                           ; 33CB mov ch,al
db 0xA0, 0x29, 0x00                     ; 33CD mov al,[0x29]
db 0x3A, 0xC5                           ; 33D0 cmp al,ch
db 0x73, 0x02                           ; 33D2 jnc 0x33d6
db 0x8A, 0xE8                           ; 33D4 mov ch,al
db 0x8A, 0xC5                           ; 33D6 mov al,ch
db 0xA0, 0x72, 0x00                     ; 33D8 mov al,[0x72]
db 0x0A, 0xC0                           ; 33DB or al,al
db 0x8A, 0xC5                           ; 33DD mov al,ch
db 0x75, 0x0C                           ; 33DF jnz 0x33ed
db 0x3A, 0xC7                           ; 33E1 cmp al,bh
db 0x74, 0x05                           ; 33E3 jz 0x33ea
db 0x8A, 0xF8                           ; 33E5 mov bh,al
db 0xE8, 0x36, 0x1B                     ; 33E7 call 0x4f20
db 0x32, 0xC0                           ; 33EA xor al,al
db 0xC3                                 ; 33EC ret
db 0x2A, 0xC7                           ; 33ED sub al,bh
db 0x74, 0xFB                           ; 33EF jz 0x33ec
db 0x50                                 ; 33F1 push ax
db 0xA0, 0x50, 0x00                     ; 33F2 mov al,[0x50]
db 0xE8, 0x14, 0x00                     ; 33F5 call 0x340c
db 0xE8, 0xAA, 0xF7                     ; 33F8 call 0x2ba5
db 0x58                                 ; 33FB pop ax
db 0xFE, 0xC8                           ; 33FC dec al
db 0x75, 0xF1                           ; 33FE jnz 0x33f1
db 0xC3                                 ; 3400 ret
db 0xA0, 0x72, 0x00                     ; 3401 mov al,[0x72]
db 0xF6, 0xD0                           ; 3404 not al
db 0xA2, 0x72, 0x00                     ; 3406 mov [0x72],al
db 0x32, 0xC0                           ; 3409 xor al,al
db 0xC3                                 ; 340B ret
db 0x53                                 ; 340C push bx
db 0x8B, 0x1E, 0x56, 0x00               ; 340D mov bx,[0x56]
db 0x50                                 ; 3411 push ax
db 0xA0, 0x72, 0x00                     ; 3412 mov al,[0x72]
db 0x0A, 0xC0                           ; 3415 or al,al
db 0x74, 0x03                           ; 3417 jz 0x341c
db 0xE8, 0x03, 0x00                     ; 3419 call 0x341f
db 0x58                                 ; 341C pop ax
db 0x5B                                 ; 341D pop bx
db 0xC3                                 ; 341E ret
db 0xA0, 0x58, 0x00                     ; 341F mov al,[0x58]
db 0x3A, 0xC3                           ; 3422 cmp al,bl
db 0x75, 0x10                           ; 3424 jnz 0x3436
db 0x53                                 ; 3426 push bx
db 0xBB, 0x5A, 0x00                     ; 3427 mov bx,0x5a
db 0xFE, 0x07                           ; 342A inc byte [bx]
db 0xA0, 0x29, 0x00                     ; 342C mov al,[0x29]
db 0x3A, 0x07                           ; 342F cmp al,[bx]
db 0x73, 0x02                           ; 3431 jnc 0x3435
db 0x88, 0x07                           ; 3433 mov [bx],al
db 0x5B                                 ; 3435 pop bx
db 0xA0, 0x50, 0x00                     ; 3436 mov al,[0x50]
db 0x8A, 0xC8                           ; 3439 mov cl,al
db 0xE8, 0x8F, 0x01                     ; 343B call 0x35cd
db 0x72, 0x10                           ; 343E jc 0x3450
db 0x74, 0xDC                           ; 3440 jz 0x341e
db 0x50                                 ; 3442 push ax
db 0x32, 0xC0                           ; 3443 xor al,al
db 0xE8, 0x28, 0xFD                     ; 3445 call 0x3170
db 0xFE, 0xC3                           ; 3448 inc bl
db 0xE8, 0xE6, 0x01                     ; 344A call 0x3633
db 0x58                                 ; 344D pop ax
db 0xFE, 0xCB                           ; 344E dec bl
db 0xFE, 0xC3                           ; 3450 inc bl
db 0xB7, 0x01                           ; 3452 mov bh,0x1
db 0xEB, 0xE3                           ; 3454 jmp short 0x3439
db 0x53                                 ; 3456 push bx
db 0xA0, 0x29, 0x00                     ; 3457 mov al,[0x29]
db 0x3A, 0xC7                           ; 345A cmp al,bh
db 0x75, 0x0B                           ; 345C jnz 0x3469
db 0xB7, 0x00                           ; 345E mov bh,0x0
db 0xA0, 0x5D, 0x00                     ; 3460 mov al,[0x5d]
db 0x3A, 0xC3                           ; 3463 cmp al,bl
db 0x75, 0x00                           ; 3465 jnz 0x3467
db 0xFE, 0xC3                           ; 3467 inc bl
db 0xFE, 0xC7                           ; 3469 inc bh
db 0xE8, 0x09, 0x00                     ; 346B call 0x3477
db 0x5B                                 ; 346E pop bx
db 0x53                                 ; 346F push bx
db 0xE8, 0xAD, 0x1A                     ; 3470 call 0x4f20
db 0x32, 0xC0                           ; 3473 xor al,al
db 0x5B                                 ; 3475 pop bx
db 0xC3                                 ; 3476 ret
db 0xB0, 0x01                           ; 3477 mov al,0x1
db 0x3A, 0xC7                           ; 3479 cmp al,bh
db 0x74, 0x04                           ; 347B jz 0x3481
db 0xFE, 0xCF                           ; 347D dec bh
db 0xEB, 0x14                           ; 347F jmp short 0x3495
db 0x53                                 ; 3481 push bx
db 0xFE, 0xCB                           ; 3482 dec bl
db 0x74, 0x0E                           ; 3484 jz 0x3494
db 0xA0, 0x29, 0x00                     ; 3486 mov al,[0x29]
db 0x8A, 0xF8                           ; 3489 mov bh,al
db 0xE8, 0xD0, 0xFC                     ; 348B call 0x315e
db 0x75, 0x04                           ; 348E jnz 0x3494
db 0x5E                                 ; 3490 pop si
db 0x87, 0xDE                           ; 3491 xchg bx,si
db 0x56                                 ; 3493 push si
db 0x5B                                 ; 3494 pop bx
db 0xE8, 0x88, 0x1A                     ; 3495 call 0x4f20
db 0xA0, 0x58, 0x00                     ; 3498 mov al,[0x58]
db 0x3A, 0xC3                           ; 349B cmp al,bl
db 0x75, 0x0A                           ; 349D jnz 0x34a9
db 0xA0, 0x5A, 0x00                     ; 349F mov al,[0x5a]
db 0xFE, 0xC8                           ; 34A2 dec al
db 0x74, 0x03                           ; 34A4 jz 0x34a9
db 0xA2, 0x5A, 0x00                     ; 34A6 mov [0x5a],al
db 0xE8, 0x42, 0x01                     ; 34A9 call 0x35ee
db 0x53                                 ; 34AC push bx
db 0xE8, 0xAE, 0xFC                     ; 34AD call 0x315e
db 0x75, 0x11                           ; 34B0 jnz 0x34c3
db 0xFE, 0xC3                           ; 34B2 inc bl
db 0xB7, 0x01                           ; 34B4 mov bh,0x1
db 0xE8, 0x05, 0x02                     ; 34B6 call 0x36be
db 0x5E                                 ; 34B9 pop si
db 0x87, 0xDE                           ; 34BA xchg bx,si
db 0x56                                 ; 34BC push si
db 0xE8, 0xFB, 0x01                     ; 34BD call 0x36bb
db 0x5B                                 ; 34C0 pop bx
db 0xEB, 0xE6                           ; 34C1 jmp short 0x34a9
db 0x5B                                 ; 34C3 pop bx
db 0xE8, 0x55, 0x1A                     ; 34C4 call 0x4f1c
db 0x32, 0xC0                           ; 34C7 xor al,al
db 0xC3                                 ; 34C9 ret
db 0xE8, 0x91, 0xFC                     ; 34CA call 0x315e
db 0x75, 0x0B                           ; 34CD jnz 0x34da
db 0xA0, 0x5B, 0x00                     ; 34CF mov al,[0x5b]
db 0x3A, 0xC3                           ; 34D2 cmp al,bl
db 0x74, 0x04                           ; 34D4 jz 0x34da
db 0xFE, 0xC3                           ; 34D6 inc bl
db 0xEB, 0xF0                           ; 34D8 jmp short 0x34ca
db 0xA0, 0x29, 0x00                     ; 34DA mov al,[0x29]
db 0x8A, 0xF8                           ; 34DD mov bh,al
db 0xA0, 0x50, 0x00                     ; 34DF mov al,[0x50]
db 0x8A, 0xC8                           ; 34E2 mov cl,al
db 0x51                                 ; 34E4 push cx
db 0xE8, 0x22, 0x1A                     ; 34E5 call 0x4f0a
db 0x59                                 ; 34E8 pop cx
db 0x0A, 0xC0                           ; 34E9 or al,al
db 0x74, 0x0C                           ; 34EB jz 0x34f9
db 0x3A, 0xC1                           ; 34ED cmp al,cl
db 0x74, 0x08                           ; 34EF jz 0x34f9
db 0xFE, 0xC7                           ; 34F1 inc bh
db 0xE8, 0x2A, 0x1A                     ; 34F3 call 0x4f20
db 0x32, 0xC0                           ; 34F6 xor al,al
db 0xC3                                 ; 34F8 ret
db 0xFE, 0xCF                           ; 34F9 dec bh
db 0x74, 0xF4                           ; 34FB jz 0x34f1
db 0xEB, 0xE5                           ; 34FD jmp short 0x34e4
db 0xE8, 0x94, 0x00                     ; 34FF call 0x3596
db 0xB7, 0x01                           ; 3502 mov bh,0x1
db 0xE8, 0x19, 0x1A                     ; 3504 call 0x4f20
db 0x53                                 ; 3507 push bx
db 0xA0, 0x50, 0x00                     ; 3508 mov al,[0x50]
db 0xE8, 0xAD, 0x01                     ; 350B call 0x36bb
db 0x5B                                 ; 350E pop bx
db 0xFE, 0xC7                           ; 350F inc bh
db 0xA0, 0x29, 0x00                     ; 3511 mov al,[0x29]
db 0xFE, 0xC0                           ; 3514 inc al
db 0x3A, 0xC7                           ; 3516 cmp al,bh
db 0x75, 0xED                           ; 3518 jnz 0x3507
db 0xE8, 0x41, 0xFC                     ; 351A call 0x315e
db 0x75, 0xA5                           ; 351D jnz 0x34c4
db 0xB7, 0x01                           ; 351F mov bh,0x1
db 0xFE, 0xC3                           ; 3521 inc bl
db 0xEB, 0xE2                           ; 3523 jmp short 0x3507
db 0xC6, 0x06, 0x70, 0x00, 0x00         ; 3525 mov byte [0x70],0x0
db 0xEB, 0x08                           ; 352A jmp short 0x3534
db 0xE8, 0xE5, 0x19                     ; 352C call 0x4f14
db 0xE8, 0x4A, 0x01                     ; 352F call 0x367c
db 0x72, 0x0F                           ; 3532 jc 0x3543
db 0xE8, 0x40, 0x00                     ; 3534 call 0x3577
db 0x74, 0x13                           ; 3537 jz 0x354c
db 0xEB, 0xF1                           ; 3539 jmp short 0x352c
db 0xE8, 0xD6, 0x19                     ; 353B call 0x4f14
db 0xE8, 0x3B, 0x01                     ; 353E call 0x367c
db 0x73, 0x07                           ; 3541 jnc 0x354a
db 0xE8, 0x31, 0x00                     ; 3543 call 0x3577
db 0x74, 0x04                           ; 3546 jz 0x354c
db 0xEB, 0xF1                           ; 3548 jmp short 0x353b
db 0x32, 0xC0                           ; 354A xor al,al
db 0xC3                                 ; 354C ret
db 0x32, 0xC0                           ; 354D xor al,al
db 0xA2, 0x70, 0x00                     ; 354F mov [0x70],al
db 0xEB, 0x08                           ; 3552 jmp short 0x355c
db 0xE8, 0xBD, 0x19                     ; 3554 call 0x4f14
db 0xE8, 0x22, 0x01                     ; 3557 call 0x367c
db 0x73, 0x0F                           ; 355A jnc 0x356b
db 0xE8, 0x26, 0x00                     ; 355C call 0x3585
db 0x74, 0xEB                           ; 355F jz 0x354c
db 0xEB, 0xF1                           ; 3561 jmp short 0x3554
db 0xE8, 0xAE, 0x19                     ; 3563 call 0x4f14
db 0xE8, 0x13, 0x01                     ; 3566 call 0x367c
db 0x72, 0x07                           ; 3569 jc 0x3572
db 0xE8, 0x17, 0x00                     ; 356B call 0x3585
db 0x74, 0xDC                           ; 356E jz 0x354c
db 0xEB, 0xF1                           ; 3570 jmp short 0x3563
db 0xE8, 0x02, 0x00                     ; 3572 call 0x3577
db 0xEB, 0xD3                           ; 3575 jmp short 0x354a
db 0x8B, 0x1E, 0x56, 0x00               ; 3577 mov bx,[0x56]
db 0xE8, 0xC4, 0xFA                     ; 357B call 0x3042
db 0x75, 0xCC                           ; 357E jnz 0x354c
db 0xB7, 0x01                           ; 3580 mov bh,0x1
db 0xE9, 0x96, 0xFA                     ; 3582 jmp 0x301b
db 0x8B, 0x1E, 0x56, 0x00               ; 3585 mov bx,[0x56]
db 0xE8, 0xDF, 0xFA                     ; 3589 call 0x306b
db 0x75, 0xBE                           ; 358C jnz 0x354c
db 0xA0, 0x29, 0x00                     ; 358E mov al,[0x29]
db 0x8A, 0xF8                           ; 3591 mov bh,al
db 0xE9, 0x9A, 0xFA                     ; 3593 jmp 0x3030
db 0xFE, 0xCB                           ; 3596 dec bl
db 0x74, 0x05                           ; 3598 jz 0x359f
db 0xE8, 0xC1, 0xFB                     ; 359A call 0x315e
db 0x74, 0xF7                           ; 359D jz 0x3596
db 0xFE, 0xC3                           ; 359F inc bl
db 0xC3                                 ; 35A1 ret
db 0x51                                 ; 35A2 push cx
db 0xA0, 0x5A, 0x00                     ; 35A3 mov al,[0x5a]
db 0x3A, 0xC7                           ; 35A6 cmp al,bh
db 0x72, 0x1A                           ; 35A8 jc 0x35c4
db 0xE8, 0x67, 0x19                     ; 35AA call 0x4f14
db 0xE8, 0x16, 0x00                     ; 35AD call 0x35c6
db 0x8B, 0xFA                           ; 35B0 mov di,dx
db 0xAA                                 ; 35B2 stosb
db 0x42                                 ; 35B3 inc dx
db 0x5E                                 ; 35B4 pop si
db 0x87, 0xDE                           ; 35B5 xchg bx,si
db 0x56                                 ; 35B7 push si
db 0xFE, 0xCF                           ; 35B8 dec bh
db 0x5E                                 ; 35BA pop si
db 0x87, 0xDE                           ; 35BB xchg bx,si
db 0x56                                 ; 35BD push si
db 0x74, 0x04                           ; 35BE jz 0x35c4
db 0xFE, 0xC7                           ; 35C0 inc bh
db 0xEB, 0xDF                           ; 35C2 jmp short 0x35a3
db 0x59                                 ; 35C4 pop cx
db 0xC3                                 ; 35C5 ret
db 0x0A, 0xC0                           ; 35C6 or al,al
db 0x75, 0x02                           ; 35C8 jnz 0x35cc
db 0xB0, 0x20                           ; 35CA mov al,0x20
db 0xC3                                 ; 35CC ret
db 0xE8, 0x45, 0x00                     ; 35CD call 0x3615
db 0x50                                 ; 35D0 push ax
db 0xE8, 0x8A, 0xFB                     ; 35D1 call 0x315e
db 0x74, 0x15                           ; 35D4 jz 0x35eb
db 0x58                                 ; 35D6 pop ax
db 0x22, 0xC0                           ; 35D7 and al,al
db 0x74, 0xF1                           ; 35D9 jz 0x35cc
db 0x3C, 0x20                           ; 35DB cmp al,0x20
db 0x74, 0xED                           ; 35DD jz 0x35cc
db 0xA0, 0x50, 0x00                     ; 35DF mov al,[0x50]
db 0x3A, 0xC1                           ; 35E2 cmp al,cl
db 0x74, 0xE6                           ; 35E4 jz 0x35cc
db 0x8A, 0xC1                           ; 35E6 mov al,cl
db 0x22, 0xC0                           ; 35E8 and al,al
db 0xC3                                 ; 35EA ret
db 0x58                                 ; 35EB pop ax
db 0xF9                                 ; 35EC stc
db 0xC3                                 ; 35ED ret
db 0xA0, 0x29, 0x00                     ; 35EE mov al,[0x29]
db 0x3A, 0xC7                           ; 35F1 cmp al,bh
db 0x74, 0x19                           ; 35F3 jz 0x360e
db 0xFE, 0xC7                           ; 35F5 inc bh
db 0xE8, 0xC4, 0x00                     ; 35F7 call 0x36be
db 0x53                                 ; 35FA push bx
db 0xFE, 0xCF                           ; 35FB dec bh
db 0xE8, 0xBB, 0x00                     ; 35FD call 0x36bb
db 0x5B                                 ; 3600 pop bx
db 0xFE, 0xC7                           ; 3601 inc bh
db 0xA0, 0x29, 0x00                     ; 3603 mov al,[0x29]
db 0xFE, 0xC0                           ; 3606 inc al
db 0x3A, 0xC7                           ; 3608 cmp al,bh
db 0x75, 0xEB                           ; 360A jnz 0x35f7
db 0xFE, 0xCF                           ; 360C dec bh
db 0xA0, 0x50, 0x00                     ; 360E mov al,[0x50]
db 0xE8, 0xA7, 0x00                     ; 3611 call 0x36bb
db 0xC3                                 ; 3614 ret
db 0x53                                 ; 3615 push bx
db 0x51                                 ; 3616 push cx
db 0xE8, 0xA4, 0x00                     ; 3617 call 0x36be
db 0x59                                 ; 361A pop cx
db 0x50                                 ; 361B push ax
db 0x8A, 0xC1                           ; 361C mov al,cl
db 0xE8, 0x9A, 0x00                     ; 361E call 0x36bb
db 0x58                                 ; 3621 pop ax
db 0x8A, 0xC8                           ; 3622 mov cl,al
db 0xA0, 0x29, 0x00                     ; 3624 mov al,[0x29]
db 0xFE, 0xC0                           ; 3627 inc al
db 0xFE, 0xC7                           ; 3629 inc bh
db 0x3A, 0xC7                           ; 362B cmp al,bh
db 0x75, 0xE7                           ; 362D jnz 0x3616
db 0x8A, 0xC1                           ; 362F mov al,cl
db 0x5B                                 ; 3631 pop bx
db 0xC3                                 ; 3632 ret
db 0x53                                 ; 3633 push bx
db 0xA0, 0x5C, 0x00                     ; 3634 mov al,[0x5c]
db 0x2A, 0xC3                           ; 3637 sub al,bl
db 0x72, 0x2F                           ; 3639 jc 0x366a
db 0x74, 0x22                           ; 363B jz 0x365f
db 0x8B, 0x1E, 0x5B, 0x00               ; 363D mov bx,[0x5b]
db 0x5E                                 ; 3641 pop si
db 0x87, 0xDE                           ; 3642 xchg bx,si
db 0x56                                 ; 3644 push si
db 0x53                                 ; 3645 push bx
db 0x8A, 0xC3                           ; 3646 mov al,bl
db 0xA2, 0x5B, 0x00                     ; 3648 mov [0x5b],al
db 0xA0, 0x5D, 0x00                     ; 364B mov al,[0x5d]
db 0xA2, 0x5C, 0x00                     ; 364E mov [0x5c],al
db 0xE8, 0x5A, 0xFA                     ; 3651 call 0x30ae
db 0x5B                                 ; 3654 pop bx
db 0x5E                                 ; 3655 pop si
db 0x87, 0xDE                           ; 3656 xchg bx,si
db 0x56                                 ; 3658 push si
db 0x89, 0x1E, 0x5B, 0x00               ; 3659 mov [0x5b],bx
db 0x5B                                 ; 365D pop bx
db 0x53                                 ; 365E push bx
db 0xB7, 0x01                           ; 365F mov bh,0x1
db 0xE8, 0xF9, 0x18                     ; 3661 call 0x4f5d
db 0x5B                                 ; 3664 pop bx
db 0xB0, 0x01                           ; 3665 mov al,0x1
db 0xE9, 0x06, 0xFB                     ; 3667 jmp 0x3170
db 0x8B, 0x1E, 0x56, 0x00               ; 366A mov bx,[0x56]
db 0xFE, 0xCB                           ; 366E dec bl
db 0x74, 0x03                           ; 3670 jz 0x3675
db 0xE8, 0xAB, 0x18                     ; 3672 call 0x4f20
db 0xE8, 0xFE, 0xF9                     ; 3675 call 0x3076
db 0x5B                                 ; 3678 pop bx
db 0xFE, 0xCB                           ; 3679 dec bl
db 0xC3                                 ; 367B ret
db 0x3C, 0x30                           ; 367C cmp al,0x30
db 0x72, 0xFB                           ; 367E jc 0x367b
db 0x3C, 0x3A                           ; 3680 cmp al,0x3a
db 0x72, 0x12                           ; 3682 jc 0x3696
db 0x3C, 0x41                           ; 3684 cmp al,0x41
db 0x72, 0xF3                           ; 3686 jc 0x367b
db 0x3C, 0x5B                           ; 3688 cmp al,0x5b
db 0x72, 0x0A                           ; 368A jc 0x3696
db 0x3C, 0x61                           ; 368C cmp al,0x61
db 0x72, 0xEB                           ; 368E jc 0x367b
db 0x3C, 0x7B                           ; 3690 cmp al,0x7b
db 0x72, 0x02                           ; 3692 jc 0x3696
db 0xF9                                 ; 3694 stc
db 0xC3                                 ; 3695 ret
db 0x22, 0xC0                           ; 3696 and al,al
db 0xC3                                 ; 3698 ret
db 0x53                                 ; 3699 push bx
db 0xB7, 0x01                           ; 369A mov bh,0x1
db 0xA0, 0x29, 0x00                     ; 369C mov al,[0x29]
db 0x8A, 0xE8                           ; 369F mov ch,al
db 0x51                                 ; 36A1 push cx
db 0xE8, 0x65, 0x18                     ; 36A2 call 0x4f0a
db 0x59                                 ; 36A5 pop cx
db 0x3C, 0xFF                           ; 36A6 cmp al,0xff
db 0x74, 0x08                           ; 36A8 jz 0x36b2
db 0xFE, 0xC7                           ; 36AA inc bh
db 0xFE, 0xCD                           ; 36AC dec ch
db 0x75, 0xF1                           ; 36AE jnz 0x36a1
db 0x5B                                 ; 36B0 pop bx
db 0xC3                                 ; 36B1 ret
db 0x5B                                 ; 36B2 pop bx
db 0x53                                 ; 36B3 push bx
db 0xB7, 0x01                           ; 36B4 mov bh,0x1
db 0xE8, 0xA4, 0x18                     ; 36B6 call 0x4f5d
db 0x5B                                 ; 36B9 pop bx
db 0xC3                                 ; 36BA ret
db 0xE9, 0x43, 0x18                     ; 36BB jmp 0x4f01
db 0xE9, 0x49, 0x18                     ; 36BE jmp 0x4f0a
db 0xA2, 0x28, 0x00                     ; 36C1 mov [0x28],al
db 0x8B, 0x1E, 0x47, 0x03               ; 36C4 mov bx,[0x347]
db 0x0A, 0xC7                           ; 36C8 or al,bh
db 0x22, 0xC3                           ; 36CA and al,bl
db 0xFE, 0xC0                           ; 36CC inc al
db 0x87, 0xDA                           ; 36CE xchg bx,dx
db 0x74, 0xE8                           ; 36D0 jz 0x36ba
db 0xEB, 0x13                           ; 36D2 jmp short 0x36e7
db 0xBB, 0xF6, 0x01                     ; 36D4 mov bx,0x1f6
db 0x74, 0xE1                           ; 36D7 jz 0x36ba
db 0xF9                                 ; 36D9 stc
db 0x9C                                 ; 36DA pushf
db 0x43                                 ; 36DB inc bx
db 0xE9, 0x75, 0xD2                     ; 36DC jmp 0x954
db 0xE8, 0x7C, 0xD9                     ; 36DF call 0x105e
db 0x74, 0x03                           ; 36E2 jz 0x36e7
db 0xE9, 0x72, 0xD9                     ; 36E4 jmp 0x1059
db 0x5B                                 ; 36E7 pop bx
db 0x89, 0x16, 0x49, 0x03               ; 36E8 mov [0x349],dx
db 0xE8, 0x78, 0xD3                     ; 36EC call 0xa67
db 0x72, 0x03                           ; 36EF jc 0x36f4
db 0xE9, 0x3C, 0xDA                     ; 36F1 jmp 0x1130
db 0x8B, 0xD9                           ; 36F4 mov bx,cx
db 0x43                                 ; 36F6 inc bx
db 0x43                                 ; 36F7 inc bx
db 0x8B, 0x17                           ; 36F8 mov dx,[bx]
db 0x43                                 ; 36FA inc bx
db 0x43                                 ; 36FB inc bx
db 0x53                                 ; 36FC push bx
db 0x87, 0xDA                           ; 36FD xchg bx,dx
db 0xE8, 0x4E, 0x2E                     ; 36FF call 0x6550
db 0x5B                                 ; 3702 pop bx
db 0x8A, 0x07                           ; 3703 mov al,[bx]
db 0x3C, 0x09                           ; 3705 cmp al,0x9
db 0x74, 0x05                           ; 3707 jz 0x370e
db 0xB0, 0x20                           ; 3709 mov al,0x20
db 0xE8, 0x97, 0xF4                     ; 370B call 0x2ba5
db 0xE8, 0xBC, 0xE8                     ; 370E call 0x1fcd
db 0xBB, 0xF7, 0x01                     ; 3711 mov bx,0x1f7
db 0xE8, 0xA9, 0xE8                     ; 3714 call 0x1fc0
db 0xE8, 0x5F, 0xFB                     ; 3717 call 0x3279
db 0x8B, 0x1E, 0x56, 0x00               ; 371A mov bx,[0x56]
db 0xFE, 0xCB                           ; 371E dec bl
db 0x74, 0x09                           ; 3720 jz 0x372b
db 0xFE, 0xCB                           ; 3722 dec bl
db 0x74, 0x05                           ; 3724 jz 0x372b
db 0xE8, 0x35, 0xFA                     ; 3726 call 0x315e
db 0x74, 0xF7                           ; 3729 jz 0x3722
db 0xFE, 0xC3                           ; 372B inc bl
db 0xE8, 0xF0, 0x17                     ; 372D call 0x4f20
db 0xE9, 0xA0, 0xD1                     ; 3730 jmp 0x8d3
db 0x3C, 0x0A                           ; 3733 cmp al,0xa
db 0x74, 0x03                           ; 3735 jz 0x373a
db 0xE9, 0x6B, 0xF4                     ; 3737 jmp 0x2ba5
db 0x53                                 ; 373A push bx
db 0x8B, 0x1E, 0xE9, 0x04               ; 373B mov bx,[0x4e9]
db 0x8A, 0xC7                           ; 373F mov al,bh
db 0x0A, 0xC3                           ; 3741 or al,bl
db 0x5B                                 ; 3743 pop bx
db 0xB0, 0x0A                           ; 3744 mov al,0xa
db 0x75, 0x08                           ; 3746 jnz 0x3750
db 0x50                                 ; 3748 push ax
db 0xB0, 0x0D                           ; 3749 mov al,0xd
db 0xE8, 0x57, 0xF4                     ; 374B call 0x2ba5
db 0x58                                 ; 374E pop ax
db 0xC3                                 ; 374F ret
db 0xE8, 0x52, 0xF4                     ; 3750 call 0x2ba5
db 0xB0, 0x0D                           ; 3753 mov al,0xd
db 0xE8, 0x4D, 0xF4                     ; 3755 call 0x2ba5
db 0xB0, 0x0A                           ; 3758 mov al,0xa
db 0xC3                                 ; 375A ret
db 0x4B                                 ; 375B dec bx
db 0xE8, 0xBE, 0xD7                     ; 375C call 0xf1d
db 0x75, 0x01                           ; 375F jnz 0x3762
db 0xC3                                 ; 3761 ret
db 0xE8, 0x8F, 0xF6                     ; 3762 call 0x2df4
db 0x2C, 0xB9                           ; 3765 sub al,0xb9
db 0x5B                                 ; 3767 pop bx
db 0x37                                 ; 3768 aaa
db 0x51                                 ; 3769 push cx
db 0xB0, 0xC8                           ; 376A mov al,0xc8
db 0xEB, 0x02                           ; 376C jmp short 0x3770
db 0x32, 0xC0                           ; 376E xor al,al
db 0xA2, 0xFA, 0x02                     ; 3770 mov [0x2fa],al
db 0x8A, 0x0F                           ; 3773 mov cl,[bx]
db 0xCD, 0xB3                           ; 3775 int 0xb3
db 0xE8, 0xC9, 0xF7                     ; 3777 call 0x2f43
db 0x73, 0x03                           ; 377A jnc 0x377f
db 0xE9, 0x3F, 0xD0                     ; 377C jmp 0x7be
db 0x32, 0xC0                           ; 377F xor al,al
db 0x8A, 0xE8                           ; 3781 mov ch,al
db 0xA2, 0x8E, 0x00                     ; 3783 mov [0x8e],al
db 0x43                                 ; 3786 inc bx
db 0x8A, 0x07                           ; 3787 mov al,[bx]
db 0x3C, 0x2E                           ; 3789 cmp al,0x2e
db 0x72, 0x42                           ; 378B jc 0x37cf
db 0x74, 0x0D                           ; 378D jz 0x379c
db 0x3C, 0x3A                           ; 378F cmp al,0x3a
db 0x73, 0x04                           ; 3791 jnc 0x3797
db 0x3C, 0x30                           ; 3793 cmp al,0x30
db 0x73, 0x05                           ; 3795 jnc 0x379c
db 0xE8, 0xAB, 0xF7                     ; 3797 call 0x2f45
db 0x72, 0x33                           ; 379A jc 0x37cf
db 0x8A, 0xE8                           ; 379C mov ch,al
db 0x51                                 ; 379E push cx
db 0xB5, 0xFF                           ; 379F mov ch,0xff
db 0xBA, 0x8E, 0x00                     ; 37A1 mov dx,0x8e
db 0x0C, 0x80                           ; 37A4 or al,0x80
db 0xFE, 0xC5                           ; 37A6 inc ch
db 0x8B, 0xFA                           ; 37A8 mov di,dx
db 0xAA                                 ; 37AA stosb
db 0x42                                 ; 37AB inc dx
db 0x43                                 ; 37AC inc bx
db 0x8A, 0x07                           ; 37AD mov al,[bx]
db 0x3C, 0x3A                           ; 37AF cmp al,0x3a
db 0x73, 0x04                           ; 37B1 jnc 0x37b7
db 0x3C, 0x30                           ; 37B3 cmp al,0x30
db 0x73, 0xED                           ; 37B5 jnc 0x37a4
db 0xE8, 0x8B, 0xF7                     ; 37B7 call 0x2f45
db 0x73, 0xE8                           ; 37BA jnc 0x37a4
db 0x3C, 0x2E                           ; 37BC cmp al,0x2e
db 0x74, 0xE4                           ; 37BE jz 0x37a4
db 0x8A, 0xC5                           ; 37C0 mov al,ch
db 0x3C, 0x27                           ; 37C2 cmp al,0x27
db 0x72, 0x03                           ; 37C4 jc 0x37c9
db 0xE9, 0xF5, 0xCF                     ; 37C6 jmp 0x7be
db 0x59                                 ; 37C9 pop cx
db 0xA2, 0x8E, 0x00                     ; 37CA mov [0x8e],al
db 0x8A, 0x07                           ; 37CD mov al,[bx]
db 0x3C, 0x26                           ; 37CF cmp al,0x26
db 0x73, 0x1E                           ; 37D1 jnc 0x37f1
db 0xBA, 0x03, 0x38                     ; 37D3 mov dx,0x3803
db 0x52                                 ; 37D6 push dx
db 0xB6, 0x02                           ; 37D7 mov dh,0x2
db 0x3C, 0x25                           ; 37D9 cmp al,0x25
db 0x74, 0x84                           ; 37DB jz 0x3761
db 0xFE, 0xC6                           ; 37DD inc dh
db 0x3C, 0x24                           ; 37DF cmp al,0x24
db 0x75, 0x01                           ; 37E1 jnz 0x37e4
db 0xC3                                 ; 37E3 ret
db 0xFE, 0xC6                           ; 37E4 inc dh
db 0x3C, 0x21                           ; 37E6 cmp al,0x21
db 0x74, 0xF9                           ; 37E8 jz 0x37e3
db 0xB6, 0x08                           ; 37EA mov dh,0x8
db 0x3C, 0x23                           ; 37EC cmp al,0x23
db 0x74, 0xF3                           ; 37EE jz 0x37e3
db 0x58                                 ; 37F0 pop ax
db 0x8A, 0xC1                           ; 37F1 mov al,cl
db 0x24, 0x7F                           ; 37F3 and al,0x7f
db 0x8A, 0xD0                           ; 37F5 mov dl,al
db 0xB6, 0x00                           ; 37F7 mov dh,0x0
db 0x53                                 ; 37F9 push bx
db 0xBB, 0x1F, 0x03                     ; 37FA mov bx,0x31f
db 0x03, 0xDA                           ; 37FD add bx,dx
db 0x8A, 0x37                           ; 37FF mov dh,[bx]
db 0x5B                                 ; 3801 pop bx
db 0x4B                                 ; 3802 dec bx
db 0x8A, 0xC6                           ; 3803 mov al,dh
db 0xA2, 0xFB, 0x02                     ; 3805 mov [0x2fb],al
db 0xE8, 0x12, 0xD7                     ; 3808 call 0xf1d
db 0xA0, 0x39, 0x03                     ; 380B mov al,[0x339]
db 0xFE, 0xC8                           ; 380E dec al
db 0x75, 0x03                           ; 3810 jnz 0x3815
db 0xE9, 0x7A, 0x01                     ; 3812 jmp 0x398f
db 0x78, 0x03                           ; 3815 js 0x381a
db 0xE9, 0x10, 0x00                     ; 3817 jmp 0x382a
db 0x8A, 0x07                           ; 381A mov al,[bx]
db 0x2C, 0x28                           ; 381C sub al,0x28
db 0x75, 0x03                           ; 381E jnz 0x3823
db 0xE9, 0xCD, 0x00                     ; 3820 jmp 0x38f0
db 0x2C, 0x33                           ; 3823 sub al,0x33
db 0x75, 0x03                           ; 3825 jnz 0x382a
db 0xE9, 0xC6, 0x00                     ; 3827 jmp 0x38f0
db 0x32, 0xC0                           ; 382A xor al,al
db 0xA2, 0x39, 0x03                     ; 382C mov [0x339],al
db 0x53                                 ; 382F push bx
db 0xA0, 0x4D, 0x04                     ; 3830 mov al,[0x44d]
db 0x0A, 0xC0                           ; 3833 or al,al
db 0xA2, 0x4A, 0x04                     ; 3835 mov [0x44a],al
db 0x74, 0x1E                           ; 3838 jz 0x3858
db 0x8B, 0x1E, 0x7C, 0x03               ; 383A mov bx,[0x37c]
db 0xBA, 0x7E, 0x03                     ; 383E mov dx,0x37e
db 0x03, 0xDA                           ; 3841 add bx,dx
db 0x89, 0x1E, 0x4B, 0x04               ; 3843 mov [0x44b],bx
db 0x87, 0xDA                           ; 3847 xchg bx,dx
db 0xE9, 0xF7, 0x2D                     ; 3849 jmp 0x6643
db 0xA0, 0x4A, 0x04                     ; 384C mov al,[0x44a]
db 0x0A, 0xC0                           ; 384F or al,al
db 0x74, 0x24                           ; 3851 jz 0x3877
db 0x32, 0xC0                           ; 3853 xor al,al
db 0xA2, 0x4A, 0x04                     ; 3855 mov [0x44a],al
db 0x8B, 0x1E, 0x5A, 0x03               ; 3858 mov bx,[0x35a]
db 0x89, 0x1E, 0x4B, 0x04               ; 385C mov [0x44b],bx
db 0x8B, 0x1E, 0x58, 0x03               ; 3860 mov bx,[0x358]
db 0xE9, 0xDC, 0x2D                     ; 3864 jmp 0x6643
db 0xE8, 0x04, 0xFF                     ; 3867 call 0x376e
db 0xC3                                 ; 386A ret
db 0x32, 0xC0                           ; 386B xor al,al
db 0x8A, 0xF0                           ; 386D mov dh,al
db 0x8A, 0xD0                           ; 386F mov dl,al
db 0x59                                 ; 3871 pop cx
db 0x5E                                 ; 3872 pop si
db 0x87, 0xDE                           ; 3873 xchg bx,si
db 0x56                                 ; 3875 push si
db 0xC3                                 ; 3876 ret
db 0x5B                                 ; 3877 pop bx
db 0x5E                                 ; 3878 pop si
db 0x87, 0xDE                           ; 3879 xchg bx,si
db 0x56                                 ; 387B push si
db 0x52                                 ; 387C push dx
db 0xBA, 0x6A, 0x38                     ; 387D mov dx,0x386a
db 0x3B, 0xDA                           ; 3880 cmp bx,dx
db 0x74, 0xE7                           ; 3882 jz 0x386b
db 0xBA, 0xDA, 0x19                     ; 3884 mov dx,0x19da
db 0x3B, 0xDA                           ; 3887 cmp bx,dx
db 0x5A                                 ; 3889 pop dx
db 0x74, 0x49                           ; 388A jz 0x38d5
db 0x5E                                 ; 388C pop si
db 0x87, 0xDE                           ; 388D xchg bx,si
db 0x56                                 ; 388F push si
db 0x53                                 ; 3890 push bx
db 0x51                                 ; 3891 push cx
db 0xA0, 0xFB, 0x02                     ; 3892 mov al,[0x2fb]
db 0x8A, 0xE8                           ; 3895 mov ch,al
db 0xA0, 0x8E, 0x00                     ; 3897 mov al,[0x8e]
db 0x02, 0xC5                           ; 389A add al,ch
db 0xFE, 0xC0                           ; 389C inc al
db 0x8A, 0xC8                           ; 389E mov cl,al
db 0x51                                 ; 38A0 push cx
db 0xB5, 0x00                           ; 38A1 mov ch,0x0
db 0x41                                 ; 38A3 inc cx
db 0x41                                 ; 38A4 inc cx
db 0x41                                 ; 38A5 inc cx
db 0x8B, 0x1E, 0x5C, 0x03               ; 38A6 mov bx,[0x35c]
db 0x53                                 ; 38AA push bx
db 0x03, 0xD9                           ; 38AB add bx,cx
db 0x59                                 ; 38AD pop cx
db 0x53                                 ; 38AE push bx
db 0xE8, 0x19, 0x2C                     ; 38AF call 0x64cb
db 0x5B                                 ; 38B2 pop bx
db 0x89, 0x1E, 0x5C, 0x03               ; 38B3 mov [0x35c],bx
db 0x8B, 0xD9                           ; 38B7 mov bx,cx
db 0x89, 0x1E, 0x5A, 0x03               ; 38B9 mov [0x35a],bx
db 0x4B                                 ; 38BD dec bx
db 0xC6, 0x07, 0x00                     ; 38BE mov byte [bx],0x0
db 0x3B, 0xDA                           ; 38C1 cmp bx,dx
db 0x75, 0xF8                           ; 38C3 jnz 0x38bd
db 0x5A                                 ; 38C5 pop dx
db 0x88, 0x37                           ; 38C6 mov [bx],dh
db 0x43                                 ; 38C8 inc bx
db 0x5A                                 ; 38C9 pop dx
db 0x89, 0x17                           ; 38CA mov [bx],dx
db 0x43                                 ; 38CC inc bx
db 0xE8, 0xDD, 0x01                     ; 38CD call 0x3aad
db 0x87, 0xDA                           ; 38D0 xchg bx,dx
db 0x42                                 ; 38D2 inc dx
db 0x5B                                 ; 38D3 pop bx
db 0xC3                                 ; 38D4 ret
db 0xE8, 0x86, 0x42                     ; 38D5 call 0x7b5e
db 0xEB, 0x08                           ; 38D8 jmp short 0x38e2
db 0xC6, 0x06, 0x4F, 0x03, 0x00         ; 38DA mov byte [0x34f],0x0
db 0xE9, 0x78, 0x0A                     ; 38DF jmp 0x435a
db 0xE8, 0x40, 0xE2                     ; 38E2 call 0x1b25
db 0x75, 0x07                           ; 38E5 jnz 0x38ee
db 0xBB, 0x06, 0x00                     ; 38E7 mov bx,0x6
db 0x89, 0x1E, 0xA3, 0x04               ; 38EA mov [0x4a3],bx
db 0x5B                                 ; 38EE pop bx
db 0xC3                                 ; 38EF ret
db 0x53                                 ; 38F0 push bx
db 0x8B, 0x1E, 0xFA, 0x02               ; 38F1 mov bx,[0x2fa]
db 0x5E                                 ; 38F5 pop si
db 0x87, 0xDE                           ; 38F6 xchg bx,si
db 0x56                                 ; 38F8 push si
db 0x8A, 0xF0                           ; 38F9 mov dh,al
db 0x52                                 ; 38FB push dx
db 0x51                                 ; 38FC push cx
db 0xBA, 0x8E, 0x00                     ; 38FD mov dx,0x8e
db 0x8B, 0xF2                           ; 3900 mov si,dx
db 0xAC                                 ; 3902 lodsb
db 0x0A, 0xC0                           ; 3903 or al,al
db 0x74, 0x3D                           ; 3905 jz 0x3944
db 0x87, 0xDA                           ; 3907 xchg bx,dx
db 0x04, 0x02                           ; 3909 add al,0x2
db 0xD0, 0xD8                           ; 390B rcr al,1
db 0x8A, 0xC8                           ; 390D mov cl,al
db 0xE8, 0xC3, 0xF3                     ; 390F call 0x2cd5
db 0x8A, 0xC1                           ; 3912 mov al,cl
db 0x8A, 0x0F                           ; 3914 mov cl,[bx]
db 0x43                                 ; 3916 inc bx
db 0x8A, 0x2F                           ; 3917 mov ch,[bx]
db 0x43                                 ; 3919 inc bx
db 0x51                                 ; 391A push cx
db 0xFE, 0xC8                           ; 391B dec al
db 0x75, 0xF5                           ; 391D jnz 0x3914
db 0x53                                 ; 391F push bx
db 0xA0, 0x8E, 0x00                     ; 3920 mov al,[0x8e]
db 0x50                                 ; 3923 push ax
db 0x87, 0xDA                           ; 3924 xchg bx,dx
db 0xE8, 0x28, 0xD7                     ; 3926 call 0x1051
db 0x58                                 ; 3929 pop ax
db 0x89, 0x1E, 0xB5, 0x00               ; 392A mov [0xb5],bx
db 0x5B                                 ; 392E pop bx
db 0x04, 0x02                           ; 392F add al,0x2
db 0xD0, 0xD8                           ; 3931 rcr al,1
db 0x59                                 ; 3933 pop cx
db 0x4B                                 ; 3934 dec bx
db 0x88, 0x2F                           ; 3935 mov [bx],ch
db 0x4B                                 ; 3937 dec bx
db 0x88, 0x0F                           ; 3938 mov [bx],cl
db 0xFE, 0xC8                           ; 393A dec al
db 0x75, 0xF5                           ; 393C jnz 0x3933
db 0x8B, 0x1E, 0xB5, 0x00               ; 393E mov bx,[0xb5]
db 0xEB, 0x08                           ; 3942 jmp short 0x394c
db 0xE8, 0x0A, 0xD7                     ; 3944 call 0x1051
db 0x32, 0xC0                           ; 3947 xor al,al
db 0xA2, 0x8E, 0x00                     ; 3949 mov [0x8e],al
db 0xA0, 0x5C, 0x04                     ; 394C mov al,[0x45c]
db 0x0A, 0xC0                           ; 394F or al,al
db 0x74, 0x08                           ; 3951 jz 0x395b
db 0x0B, 0xD2                           ; 3953 or dx,dx
db 0x75, 0x03                           ; 3955 jnz 0x395a
db 0xE9, 0x5F, 0x00                     ; 3957 jmp 0x39b9
db 0x4A                                 ; 395A dec dx
db 0x59                                 ; 395B pop cx
db 0x58                                 ; 395C pop ax
db 0x86, 0xC4                           ; 395D xchg al,ah
db 0x9E                                 ; 395F sahf
db 0x87, 0xDA                           ; 3960 xchg bx,dx
db 0x5E                                 ; 3962 pop si
db 0x87, 0xDE                           ; 3963 xchg bx,si
db 0x56                                 ; 3965 push si
db 0x53                                 ; 3966 push bx
db 0x87, 0xDA                           ; 3967 xchg bx,dx
db 0xFE, 0xC0                           ; 3969 inc al
db 0x8A, 0xF0                           ; 396B mov dh,al
db 0x8A, 0x07                           ; 396D mov al,[bx]
db 0x3C, 0x2C                           ; 396F cmp al,0x2c
db 0x74, 0x88                           ; 3971 jz 0x38fb
db 0x3C, 0x29                           ; 3973 cmp al,0x29
db 0x74, 0x07                           ; 3975 jz 0x397e
db 0x3C, 0x5D                           ; 3977 cmp al,0x5d
db 0x74, 0x03                           ; 3979 jz 0x397e
db 0xE9, 0x40, 0xCE                     ; 397B jmp 0x7be
db 0xE8, 0x9C, 0xD5                     ; 397E call 0xf1d
db 0x89, 0x1E, 0x52, 0x03               ; 3981 mov [0x352],bx
db 0x5B                                 ; 3985 pop bx
db 0x89, 0x1E, 0xFA, 0x02               ; 3986 mov [0x2fa],bx
db 0xB2, 0x00                           ; 398A mov dl,0x0
db 0x52                                 ; 398C push dx
db 0xEB, 0x07                           ; 398D jmp short 0x3996
db 0x53                                 ; 398F push bx
db 0x9F                                 ; 3990 lahf
db 0x86, 0xC4                           ; 3991 xchg al,ah
db 0x50                                 ; 3993 push ax
db 0x86, 0xC4                           ; 3994 xchg al,ah
db 0x8B, 0x1E, 0x5A, 0x03               ; 3996 mov bx,[0x35a]
db 0xE9, 0xAF, 0x2C                     ; 399A jmp 0x664c
db 0xA0, 0xFA, 0x02                     ; 399D mov al,[0x2fa]
db 0x0A, 0xC0                           ; 39A0 or al,al
db 0x74, 0x03                           ; 39A2 jz 0x39a7
db 0xE9, 0x20, 0xCE                     ; 39A4 jmp 0x7c7
db 0x58                                 ; 39A7 pop ax
db 0x86, 0xC4                           ; 39A8 xchg al,ah
db 0x9E                                 ; 39AA sahf
db 0x8B, 0xCB                           ; 39AB mov cx,bx
db 0x75, 0x03                           ; 39AD jnz 0x39b2
db 0xE9, 0x55, 0x2B                     ; 39AF jmp 0x6507
db 0x2A, 0x07                           ; 39B2 sub al,[bx]
db 0x75, 0x03                           ; 39B4 jnz 0x39b9
db 0xE9, 0x98, 0x00                     ; 39B6 jmp 0x3a51
db 0xBA, 0x09, 0x00                     ; 39B9 mov dx,0x9
db 0xE9, 0x19, 0xCE                     ; 39BC jmp 0x7d8
db 0xA0, 0xFB, 0x02                     ; 39BF mov al,[0x2fb]
db 0x88, 0x07                           ; 39C2 mov [bx],al
db 0x43                                 ; 39C4 inc bx
db 0x8A, 0xD0                           ; 39C5 mov dl,al
db 0xB6, 0x00                           ; 39C7 mov dh,0x0
db 0x58                                 ; 39C9 pop ax
db 0x86, 0xC4                           ; 39CA xchg al,ah
db 0x9E                                 ; 39CC sahf
db 0x75, 0x03                           ; 39CD jnz 0x39d2
db 0xE9, 0xCA, 0x00                     ; 39CF jmp 0x3a9c
db 0x88, 0x0F                           ; 39D2 mov [bx],cl
db 0x43                                 ; 39D4 inc bx
db 0x88, 0x2F                           ; 39D5 mov [bx],ch
db 0xE8, 0xD3, 0x00                     ; 39D7 call 0x3aad
db 0x43                                 ; 39DA inc bx
db 0x8A, 0xC8                           ; 39DB mov cl,al
db 0xE8, 0xF5, 0xF2                     ; 39DD call 0x2cd5
db 0x43                                 ; 39E0 inc bx
db 0x43                                 ; 39E1 inc bx
db 0x89, 0x1E, 0x31, 0x03               ; 39E2 mov [0x331],bx
db 0x88, 0x0F                           ; 39E6 mov [bx],cl
db 0x43                                 ; 39E8 inc bx
db 0xA0, 0xFA, 0x02                     ; 39E9 mov al,[0x2fa]
db 0xD0, 0xD0                           ; 39EC rcl al,1
db 0x8A, 0xC1                           ; 39EE mov al,cl
db 0x72, 0x0F                           ; 39F0 jc 0x3a01
db 0x9F                                 ; 39F2 lahf
db 0x50                                 ; 39F3 push ax
db 0xA0, 0x5C, 0x04                     ; 39F4 mov al,[0x45c]
db 0x34, 0x0B                           ; 39F7 xor al,0xb
db 0x8A, 0xC8                           ; 39F9 mov cl,al
db 0xB5, 0x00                           ; 39FB mov ch,0x0
db 0x58                                 ; 39FD pop ax
db 0x9E                                 ; 39FE sahf
db 0x73, 0x04                           ; 39FF jnc 0x3a05
db 0x59                                 ; 3A01 pop cx
db 0x9F                                 ; 3A02 lahf
db 0x41                                 ; 3A03 inc cx
db 0x9E                                 ; 3A04 sahf
db 0x88, 0x0F                           ; 3A05 mov [bx],cl
db 0x9F                                 ; 3A07 lahf
db 0x50                                 ; 3A08 push ax
db 0x43                                 ; 3A09 inc bx
db 0x88, 0x2F                           ; 3A0A mov [bx],ch
db 0x43                                 ; 3A0C inc bx
db 0xE8, 0x7A, 0x2A                     ; 3A0D call 0x648a
db 0x58                                 ; 3A10 pop ax
db 0x9E                                 ; 3A11 sahf
db 0xFE, 0xC8                           ; 3A12 dec al
db 0x75, 0xDA                           ; 3A14 jnz 0x39f0
db 0x9F                                 ; 3A16 lahf
db 0x50                                 ; 3A17 push ax
db 0x8A, 0xEE                           ; 3A18 mov ch,dh
db 0x8A, 0xCA                           ; 3A1A mov cl,dl
db 0x87, 0xDA                           ; 3A1C xchg bx,dx
db 0x03, 0xDA                           ; 3A1E add bx,dx
db 0x73, 0x03                           ; 3A20 jnc 0x3a25
db 0xE9, 0xCF, 0xF2                     ; 3A22 jmp 0x2cf4
db 0xE8, 0xDC, 0xF2                     ; 3A25 call 0x2d04
db 0x89, 0x1E, 0x5C, 0x03               ; 3A28 mov [0x35c],bx
db 0x4B                                 ; 3A2C dec bx
db 0xC6, 0x07, 0x00                     ; 3A2D mov byte [bx],0x0
db 0x3B, 0xDA                           ; 3A30 cmp bx,dx
db 0x75, 0xF8                           ; 3A32 jnz 0x3a2c
db 0x32, 0xC0                           ; 3A34 xor al,al
db 0x41                                 ; 3A36 inc cx
db 0x8A, 0xF0                           ; 3A37 mov dh,al
db 0x8B, 0x1E, 0x31, 0x03               ; 3A39 mov bx,[0x331]
db 0x8A, 0x17                           ; 3A3D mov dl,[bx]
db 0x87, 0xDA                           ; 3A3F xchg bx,dx
db 0x03, 0xDB                           ; 3A41 add bx,bx
db 0x03, 0xD9                           ; 3A43 add bx,cx
db 0x87, 0xDA                           ; 3A45 xchg bx,dx
db 0x4B                                 ; 3A47 dec bx
db 0x4B                                 ; 3A48 dec bx
db 0x89, 0x17                           ; 3A49 mov [bx],dx
db 0x43                                 ; 3A4B inc bx
db 0x43                                 ; 3A4C inc bx
db 0x58                                 ; 3A4D pop ax
db 0x9E                                 ; 3A4E sahf
db 0x72, 0x46                           ; 3A4F jc 0x3a97
db 0x8A, 0xE8                           ; 3A51 mov ch,al
db 0x8A, 0xC8                           ; 3A53 mov cl,al
db 0x8A, 0x07                           ; 3A55 mov al,[bx]
db 0x43                                 ; 3A57 inc bx
db 0xB6, 0x5B                           ; 3A58 mov dh,0x5b
db 0x8B, 0x17                           ; 3A5A mov dx,[bx]
db 0x43                                 ; 3A5C inc bx
db 0x43                                 ; 3A5D inc bx
db 0x5E                                 ; 3A5E pop si
db 0x87, 0xDE                           ; 3A5F xchg bx,si
db 0x56                                 ; 3A61 push si
db 0x50                                 ; 3A62 push ax
db 0x3B, 0xDA                           ; 3A63 cmp bx,dx
db 0x72, 0x03                           ; 3A65 jc 0x3a6a
db 0xE9, 0x4F, 0xFF                     ; 3A67 jmp 0x39b9
db 0xE8, 0x1D, 0x2A                     ; 3A6A call 0x648a
db 0x03, 0xDA                           ; 3A6D add bx,dx
db 0x58                                 ; 3A6F pop ax
db 0xFE, 0xC8                           ; 3A70 dec al
db 0x8B, 0xCB                           ; 3A72 mov cx,bx
db 0x75, 0xE3                           ; 3A74 jnz 0x3a59
db 0xA0, 0xFB, 0x02                     ; 3A76 mov al,[0x2fb]
db 0x8B, 0xCB                           ; 3A79 mov cx,bx
db 0x03, 0xDB                           ; 3A7B add bx,bx
db 0x2C, 0x04                           ; 3A7D sub al,0x4
db 0x72, 0x08                           ; 3A7F jc 0x3a89
db 0x03, 0xDB                           ; 3A81 add bx,bx
db 0x0A, 0xC0                           ; 3A83 or al,al
db 0x74, 0x0B                           ; 3A85 jz 0x3a92
db 0x03, 0xDB                           ; 3A87 add bx,bx
db 0x0A, 0xC0                           ; 3A89 or al,al
db 0x7A, 0x03                           ; 3A8B jpe 0x3a90
db 0xE9, 0x02, 0x00                     ; 3A8D jmp 0x3a92
db 0x03, 0xD9                           ; 3A90 add bx,cx
db 0x59                                 ; 3A92 pop cx
db 0x03, 0xD9                           ; 3A93 add bx,cx
db 0x87, 0xDA                           ; 3A95 xchg bx,dx
db 0x8B, 0x1E, 0x52, 0x03               ; 3A97 mov bx,[0x352]
db 0xC3                                 ; 3A9B ret
db 0xF9                                 ; 3A9C stc
db 0x1A, 0xC0                           ; 3A9D sbb al,al
db 0x5B                                 ; 3A9F pop bx
db 0xC3                                 ; 3AA0 ret
db 0x8A, 0x07                           ; 3AA1 mov al,[bx]
db 0x43                                 ; 3AA3 inc bx
db 0x51                                 ; 3AA4 push cx
db 0xB5, 0x00                           ; 3AA5 mov ch,0x0
db 0x8A, 0xC8                           ; 3AA7 mov cl,al
db 0x03, 0xD9                           ; 3AA9 add bx,cx
db 0x59                                 ; 3AAB pop cx
db 0xC3                                 ; 3AAC ret
db 0x51                                 ; 3AAD push cx
db 0x52                                 ; 3AAE push dx
db 0x9F                                 ; 3AAF lahf
db 0x50                                 ; 3AB0 push ax
db 0xBA, 0x8E, 0x00                     ; 3AB1 mov dx,0x8e
db 0x8B, 0xF2                           ; 3AB4 mov si,dx
db 0xAC                                 ; 3AB6 lodsb
db 0x8A, 0xE8                           ; 3AB7 mov ch,al
db 0xFE, 0xC5                           ; 3AB9 inc ch
db 0x8B, 0xF2                           ; 3ABB mov si,dx
db 0xAC                                 ; 3ABD lodsb
db 0x42                                 ; 3ABE inc dx
db 0x43                                 ; 3ABF inc bx
db 0x88, 0x07                           ; 3AC0 mov [bx],al
db 0xFE, 0xCD                           ; 3AC2 dec ch
db 0x75, 0xF5                           ; 3AC4 jnz 0x3abb
db 0x58                                 ; 3AC6 pop ax
db 0x9E                                 ; 3AC7 sahf
db 0x5A                                 ; 3AC8 pop dx
db 0x59                                 ; 3AC9 pop cx
db 0xC3                                 ; 3ACA ret
db 0xE8, 0x5A, 0xDC                     ; 3ACB call 0x1728
db 0xE8, 0x6A, 0x29                     ; 3ACE call 0x643b
db 0xE8, 0x20, 0xF3                     ; 3AD1 call 0x2df4
db 0x3B, 0x87, 0xDA, 0x8B               ; 3AD4 cmp ax,[bx+0x8bda]
db 0x1E                                 ; 3AD8 push ds
db 0xA3, 0x04, 0xEB                     ; 3AD9 mov [0xeb04],ax
db 0x0A, 0xA0, 0x3A, 0x03               ; 3ADC or ah,[bx+si+0x33a]
db 0x0A, 0xC0                           ; 3AE0 or al,al
db 0x74, 0x11                           ; 3AE2 jz 0x3af5
db 0x5A                                 ; 3AE4 pop dx
db 0x87, 0xDA                           ; 3AE5 xchg bx,dx
db 0x53                                 ; 3AE7 push bx
db 0x32, 0xC0                           ; 3AE8 xor al,al
db 0xA2, 0x3A, 0x03                     ; 3AEA mov [0x33a],al
db 0xFE, 0xC0                           ; 3AED inc al
db 0x9C                                 ; 3AEF pushf
db 0x52                                 ; 3AF0 push dx
db 0x8A, 0x2F                           ; 3AF1 mov ch,[bx]
db 0x0A, 0xED                           ; 3AF3 or ch,ch
db 0x75, 0x03                           ; 3AF5 jnz 0x3afa
db 0xE9, 0x5F, 0xD5                     ; 3AF7 jmp 0x1059
db 0x43                                 ; 3AFA inc bx
db 0x8B, 0x1F                           ; 3AFB mov bx,[bx]
db 0xEB, 0x24                           ; 3AFD jmp short 0x3b23
db 0x8A, 0xD5                           ; 3AFF mov dl,ch
db 0x53                                 ; 3B01 push bx
db 0xB1, 0x02                           ; 3B02 mov cl,0x2
db 0x8A, 0x07                           ; 3B04 mov al,[bx]
db 0x43                                 ; 3B06 inc bx
db 0x3C, 0x5C                           ; 3B07 cmp al,0x5c
db 0x75, 0x03                           ; 3B09 jnz 0x3b0e
db 0xE9, 0x9D, 0x01                     ; 3B0B jmp 0x3cab
db 0x3C, 0x20                           ; 3B0E cmp al,0x20
db 0x75, 0x06                           ; 3B10 jnz 0x3b18
db 0xFE, 0xC1                           ; 3B12 inc cl
db 0xFE, 0xCD                           ; 3B14 dec ch
db 0x75, 0xEC                           ; 3B16 jnz 0x3b04
db 0x5B                                 ; 3B18 pop bx
db 0x8A, 0xEA                           ; 3B19 mov ch,dl
db 0xB0, 0x5C                           ; 3B1B mov al,0x5c
db 0xE8, 0xD6, 0x01                     ; 3B1D call 0x3cf6
db 0xE8, 0x82, 0xF0                     ; 3B20 call 0x2ba5
db 0x32, 0xC0                           ; 3B23 xor al,al
db 0x8A, 0xD0                           ; 3B25 mov dl,al
db 0x8A, 0xF0                           ; 3B27 mov dh,al
db 0xE8, 0xCA, 0x01                     ; 3B29 call 0x3cf6
db 0x8A, 0xF0                           ; 3B2C mov dh,al
db 0x8A, 0x07                           ; 3B2E mov al,[bx]
db 0x43                                 ; 3B30 inc bx
db 0x3C, 0x21                           ; 3B31 cmp al,0x21
db 0x75, 0x03                           ; 3B33 jnz 0x3b38
db 0xE9, 0x6F, 0x01                     ; 3B35 jmp 0x3ca7
db 0x3C, 0x23                           ; 3B38 cmp al,0x23
db 0x74, 0x52                           ; 3B3A jz 0x3b8e
db 0x3C, 0x26                           ; 3B3C cmp al,0x26
db 0x75, 0x03                           ; 3B3E jnz 0x3b43
db 0xE9, 0x60, 0x01                     ; 3B40 jmp 0x3ca3
db 0xFE, 0xCD                           ; 3B43 dec ch
db 0x75, 0x03                           ; 3B45 jnz 0x3b4a
db 0xE9, 0x2E, 0x01                     ; 3B47 jmp 0x3c78
db 0x3C, 0x2B                           ; 3B4A cmp al,0x2b
db 0xB0, 0x08                           ; 3B4C mov al,0x8
db 0x74, 0xD9                           ; 3B4E jz 0x3b29
db 0x4B                                 ; 3B50 dec bx
db 0x8A, 0x07                           ; 3B51 mov al,[bx]
db 0x43                                 ; 3B53 inc bx
db 0x3C, 0x2E                           ; 3B54 cmp al,0x2e
db 0x74, 0x55                           ; 3B56 jz 0x3bad
db 0x3C, 0x5F                           ; 3B58 cmp al,0x5f
db 0x75, 0x03                           ; 3B5A jnz 0x3b5f
db 0xE9, 0x37, 0x01                     ; 3B5C jmp 0x3c96
db 0x3C, 0x5C                           ; 3B5F cmp al,0x5c
db 0x74, 0x9C                           ; 3B61 jz 0x3aff
db 0x3A, 0x07                           ; 3B63 cmp al,[bx]
db 0x75, 0xB6                           ; 3B65 jnz 0x3b1d
db 0x3C, 0x24                           ; 3B67 cmp al,0x24
db 0x74, 0x18                           ; 3B69 jz 0x3b83
db 0x3C, 0x2A                           ; 3B6B cmp al,0x2a
db 0x75, 0xAE                           ; 3B6D jnz 0x3b1d
db 0x8A, 0xC5                           ; 3B6F mov al,ch
db 0x43                                 ; 3B71 inc bx
db 0x3C, 0x02                           ; 3B72 cmp al,0x2
db 0x72, 0x04                           ; 3B74 jc 0x3b7a
db 0x8A, 0x07                           ; 3B76 mov al,[bx]
db 0x3C, 0x24                           ; 3B78 cmp al,0x24
db 0xB0, 0x20                           ; 3B7A mov al,0x20
db 0x75, 0x0A                           ; 3B7C jnz 0x3b88
db 0xFE, 0xCD                           ; 3B7E dec ch
db 0xFE, 0xC2                           ; 3B80 inc dl
db 0xBE, 0x32, 0xC0                     ; 3B82 mov si,0xc032
db 0x04, 0x10                           ; 3B85 add al,0x10
db 0x43                                 ; 3B87 inc bx
db 0xFE, 0xC2                           ; 3B88 inc dl
db 0x02, 0xC6                           ; 3B8A add al,dh
db 0x8A, 0xF0                           ; 3B8C mov dh,al
db 0xFE, 0xC2                           ; 3B8E inc dl
db 0xB1, 0x00                           ; 3B90 mov cl,0x0
db 0xFE, 0xCD                           ; 3B92 dec ch
db 0x74, 0x61                           ; 3B94 jz 0x3bf7
db 0x8A, 0x07                           ; 3B96 mov al,[bx]
db 0x43                                 ; 3B98 inc bx
db 0x3C, 0x2E                           ; 3B99 cmp al,0x2e
db 0x74, 0x1E                           ; 3B9B jz 0x3bbb
db 0x3C, 0x23                           ; 3B9D cmp al,0x23
db 0x74, 0xED                           ; 3B9F jz 0x3b8e
db 0x3C, 0x2C                           ; 3BA1 cmp al,0x2c
db 0x75, 0x23                           ; 3BA3 jnz 0x3bc8
db 0x8A, 0xC6                           ; 3BA5 mov al,dh
db 0x0C, 0x40                           ; 3BA7 or al,0x40
db 0x8A, 0xF0                           ; 3BA9 mov dh,al
db 0xEB, 0xE1                           ; 3BAB jmp short 0x3b8e
db 0x8A, 0x07                           ; 3BAD mov al,[bx]
db 0x3C, 0x23                           ; 3BAF cmp al,0x23
db 0xB0, 0x2E                           ; 3BB1 mov al,0x2e
db 0x74, 0x03                           ; 3BB3 jz 0x3bb8
db 0xE9, 0x65, 0xFF                     ; 3BB5 jmp 0x3b1d
db 0xB1, 0x01                           ; 3BB8 mov cl,0x1
db 0x43                                 ; 3BBA inc bx
db 0xFE, 0xC1                           ; 3BBB inc cl
db 0xFE, 0xCD                           ; 3BBD dec ch
db 0x74, 0x36                           ; 3BBF jz 0x3bf7
db 0x8A, 0x07                           ; 3BC1 mov al,[bx]
db 0x43                                 ; 3BC3 inc bx
db 0x3C, 0x23                           ; 3BC4 cmp al,0x23
db 0x74, 0xF3                           ; 3BC6 jz 0x3bbb
db 0x52                                 ; 3BC8 push dx
db 0xBA, 0xF4, 0x3B                     ; 3BC9 mov dx,0x3bf4
db 0x52                                 ; 3BCC push dx
db 0x8A, 0xF7                           ; 3BCD mov dh,bh
db 0x8A, 0xD3                           ; 3BCF mov dl,bl
db 0x3C, 0x5E                           ; 3BD1 cmp al,0x5e
db 0x74, 0x01                           ; 3BD3 jz 0x3bd6
db 0xC3                                 ; 3BD5 ret
db 0x3A, 0x07                           ; 3BD6 cmp al,[bx]
db 0x75, 0xFB                           ; 3BD8 jnz 0x3bd5
db 0x43                                 ; 3BDA inc bx
db 0x3A, 0x07                           ; 3BDB cmp al,[bx]
db 0x75, 0xF6                           ; 3BDD jnz 0x3bd5
db 0x43                                 ; 3BDF inc bx
db 0x3A, 0x07                           ; 3BE0 cmp al,[bx]
db 0x75, 0xF1                           ; 3BE2 jnz 0x3bd5
db 0x43                                 ; 3BE4 inc bx
db 0x8A, 0xC5                           ; 3BE5 mov al,ch
db 0x2C, 0x04                           ; 3BE7 sub al,0x4
db 0x72, 0xEA                           ; 3BE9 jc 0x3bd5
db 0x5A                                 ; 3BEB pop dx
db 0x5A                                 ; 3BEC pop dx
db 0x8A, 0xE8                           ; 3BED mov ch,al
db 0xFE, 0xC6                           ; 3BEF inc dh
db 0x43                                 ; 3BF1 inc bx
db 0xEB, 0x03                           ; 3BF2 jmp short 0x3bf7
db 0x87, 0xDA                           ; 3BF4 xchg bx,dx
db 0x5A                                 ; 3BF6 pop dx
db 0x8A, 0xC6                           ; 3BF7 mov al,dh
db 0x4B                                 ; 3BF9 dec bx
db 0xFE, 0xC2                           ; 3BFA inc dl
db 0x24, 0x08                           ; 3BFC and al,0x8
db 0x75, 0x1C                           ; 3BFE jnz 0x3c1c
db 0xFE, 0xCA                           ; 3C00 dec dl
db 0x8A, 0xC5                           ; 3C02 mov al,ch
db 0x0A, 0xC0                           ; 3C04 or al,al
db 0x74, 0x14                           ; 3C06 jz 0x3c1c
db 0x8A, 0x07                           ; 3C08 mov al,[bx]
db 0x2C, 0x2D                           ; 3C0A sub al,0x2d
db 0x74, 0x06                           ; 3C0C jz 0x3c14
db 0x3C, 0xFE                           ; 3C0E cmp al,0xfe
db 0x75, 0x0A                           ; 3C10 jnz 0x3c1c
db 0xB0, 0x08                           ; 3C12 mov al,0x8
db 0x04, 0x04                           ; 3C14 add al,0x4
db 0x02, 0xC6                           ; 3C16 add al,dh
db 0x8A, 0xF0                           ; 3C18 mov dh,al
db 0xFE, 0xCD                           ; 3C1A dec ch
db 0x5B                                 ; 3C1C pop bx
db 0x9D                                 ; 3C1D popf
db 0x74, 0x65                           ; 3C1E jz 0x3c85
db 0x51                                 ; 3C20 push cx
db 0x52                                 ; 3C21 push dx
db 0xE8, 0x02, 0xDB                     ; 3C22 call 0x1727
db 0x5A                                 ; 3C25 pop dx
db 0x59                                 ; 3C26 pop cx
db 0x51                                 ; 3C27 push cx
db 0x53                                 ; 3C28 push bx
db 0x8A, 0xEA                           ; 3C29 mov ch,dl
db 0x8A, 0xC5                           ; 3C2B mov al,ch
db 0x02, 0xC1                           ; 3C2D add al,cl
db 0x3C, 0x19                           ; 3C2F cmp al,0x19
db 0x72, 0x03                           ; 3C31 jc 0x3c36
db 0xE9, 0x23, 0xD4                     ; 3C33 jmp 0x1059
db 0x8A, 0xC6                           ; 3C36 mov al,dh
db 0x0C, 0x80                           ; 3C38 or al,0x80
db 0xE8, 0x5C, 0x3B                     ; 3C3A call 0x7799
db 0xE8, 0x77, 0xEA                     ; 3C3D call 0x26b7
db 0x5B                                 ; 3C40 pop bx
db 0x4B                                 ; 3C41 dec bx
db 0xE8, 0xD8, 0xD2                     ; 3C42 call 0xf1d
db 0xF9                                 ; 3C45 stc
db 0x74, 0x11                           ; 3C46 jz 0x3c59
db 0xA2, 0x3A, 0x03                     ; 3C48 mov [0x33a],al
db 0x3C, 0x3B                           ; 3C4B cmp al,0x3b
db 0x74, 0x07                           ; 3C4D jz 0x3c56
db 0x3C, 0x2C                           ; 3C4F cmp al,0x2c
db 0x74, 0x03                           ; 3C51 jz 0x3c56
db 0xE9, 0x68, 0xCB                     ; 3C53 jmp 0x7be
db 0xE8, 0xC4, 0xD2                     ; 3C56 call 0xf1d
db 0x59                                 ; 3C59 pop cx
db 0x87, 0xDA                           ; 3C5A xchg bx,dx
db 0x5B                                 ; 3C5C pop bx
db 0x53                                 ; 3C5D push bx
db 0x9C                                 ; 3C5E pushf
db 0x52                                 ; 3C5F push dx
db 0x8A, 0x07                           ; 3C60 mov al,[bx]
db 0x2A, 0xC5                           ; 3C62 sub al,ch
db 0x43                                 ; 3C64 inc bx
db 0xB6, 0x00                           ; 3C65 mov dh,0x0
db 0x8A, 0xD0                           ; 3C67 mov dl,al
db 0x8B, 0x1F                           ; 3C69 mov bx,[bx]
db 0x03, 0xDA                           ; 3C6B add bx,dx
db 0x8A, 0xC5                           ; 3C6D mov al,ch
db 0x0A, 0xC0                           ; 3C6F or al,al
db 0x74, 0x03                           ; 3C71 jz 0x3c76
db 0xE9, 0xAD, 0xFE                     ; 3C73 jmp 0x3b23
db 0xEB, 0x06                           ; 3C76 jmp short 0x3c7e
db 0xE8, 0x7B, 0x00                     ; 3C78 call 0x3cf6
db 0xE8, 0x27, 0xEF                     ; 3C7B call 0x2ba5
db 0x5B                                 ; 3C7E pop bx
db 0x9D                                 ; 3C7F popf
db 0x74, 0x03                           ; 3C80 jz 0x3c85
db 0xE9, 0x58, 0xFE                     ; 3C82 jmp 0x3add
db 0x73, 0x03                           ; 3C85 jnc 0x3c8a
db 0xE8, 0xE7, 0xEF                     ; 3C87 call 0x2c71
db 0x5E                                 ; 3C8A pop si
db 0x87, 0xDE                           ; 3C8B xchg bx,si
db 0x56                                 ; 3C8D push si
db 0xE8, 0x1C, 0xEC                     ; 3C8E call 0x28ad
db 0x5B                                 ; 3C91 pop bx
db 0xE9, 0x03, 0xD8                     ; 3C92 jmp 0x1498
db 0xC3                                 ; 3C95 ret
db 0xE8, 0x5D, 0x00                     ; 3C96 call 0x3cf6
db 0xFE, 0xCD                           ; 3C99 dec ch
db 0x8A, 0x07                           ; 3C9B mov al,[bx]
db 0x43                                 ; 3C9D inc bx
db 0xE8, 0x04, 0xEF                     ; 3C9E call 0x2ba5
db 0xEB, 0xCA                           ; 3CA1 jmp short 0x3c6d
db 0xB1, 0x00                           ; 3CA3 mov cl,0x0
db 0xEB, 0x05                           ; 3CA5 jmp short 0x3cac
db 0xB1, 0x01                           ; 3CA7 mov cl,0x1
db 0xEB, 0x01                           ; 3CA9 jmp short 0x3cac
db 0x58                                 ; 3CAB pop ax
db 0xFE, 0xCD                           ; 3CAC dec ch
db 0xE8, 0x45, 0x00                     ; 3CAE call 0x3cf6
db 0x5B                                 ; 3CB1 pop bx
db 0x9D                                 ; 3CB2 popf
db 0x74, 0xD0                           ; 3CB3 jz 0x3c85
db 0x51                                 ; 3CB5 push cx
db 0xE8, 0x6E, 0xDA                     ; 3CB6 call 0x1727
db 0xE8, 0x7F, 0x27                     ; 3CB9 call 0x643b
db 0x59                                 ; 3CBC pop cx
db 0x51                                 ; 3CBD push cx
db 0x53                                 ; 3CBE push bx
db 0x8B, 0x1E, 0xA3, 0x04               ; 3CBF mov bx,[0x4a3]
db 0x8A, 0xE9                           ; 3CC3 mov ch,cl
db 0xB1, 0x00                           ; 3CC5 mov cl,0x0
db 0x8A, 0xC5                           ; 3CC7 mov al,ch
db 0x50                                 ; 3CC9 push ax
db 0x8A, 0xC5                           ; 3CCA mov al,ch
db 0x0A, 0xC0                           ; 3CCC or al,al
db 0x74, 0x03                           ; 3CCE jz 0x3cd3
db 0xE8, 0xA1, 0xEC                     ; 3CD0 call 0x2974
db 0xE8, 0xE4, 0xE9                     ; 3CD3 call 0x26ba
db 0x8B, 0x1E, 0xA3, 0x04               ; 3CD6 mov bx,[0x4a3]
db 0x58                                 ; 3CDA pop ax
db 0x0A, 0xC0                           ; 3CDB or al,al
db 0x75, 0x03                           ; 3CDD jnz 0x3ce2
db 0xE9, 0x5E, 0xFF                     ; 3CDF jmp 0x3c40
db 0x2A, 0x07                           ; 3CE2 sub al,[bx]
db 0x8A, 0xE8                           ; 3CE4 mov ch,al
db 0xB0, 0x20                           ; 3CE6 mov al,0x20
db 0xFE, 0xC5                           ; 3CE8 inc ch
db 0xFE, 0xCD                           ; 3CEA dec ch
db 0x75, 0x03                           ; 3CEC jnz 0x3cf1
db 0xE9, 0x4F, 0xFF                     ; 3CEE jmp 0x3c40
db 0xE8, 0xB1, 0xEE                     ; 3CF1 call 0x2ba5
db 0xEB, 0xF4                           ; 3CF4 jmp short 0x3cea
db 0x50                                 ; 3CF6 push ax
db 0x8A, 0xC6                           ; 3CF7 mov al,dh
db 0x0A, 0xC0                           ; 3CF9 or al,al
db 0xB0, 0x2B                           ; 3CFB mov al,0x2b
db 0x74, 0x03                           ; 3CFD jz 0x3d02
db 0xE8, 0xA3, 0xEE                     ; 3CFF call 0x2ba5
db 0x58                                 ; 3D02 pop ax
db 0xC3                                 ; 3D03 ret
db 0x89, 0x1E, 0x35, 0x03               ; 3D04 mov [0x335],bx
db 0xE8, 0xEC, 0xE7                     ; 3D08 call 0x24f7
db 0xE8, 0x0F, 0xD2                     ; 3D0B call 0xf1d
db 0x87, 0xDA                           ; 3D0E xchg bx,dx
db 0xE8, 0x66, 0x00                     ; 3D10 call 0x3d79
db 0x9F                                 ; 3D13 lahf
db 0x44                                 ; 3D14 inc sp
db 0x9E                                 ; 3D15 sahf
db 0x9F                                 ; 3D16 lahf
db 0x44                                 ; 3D17 inc sp
db 0x9E                                 ; 3D18 sahf
db 0x75, 0x08                           ; 3D19 jnz 0x3d23
db 0x03, 0xD9                           ; 3D1B add bx,cx
db 0x8B, 0xE3                           ; 3D1D mov sp,bx
db 0x89, 0x1E, 0x45, 0x03               ; 3D1F mov [0x345],bx
db 0x8B, 0x1E, 0x2E, 0x00               ; 3D23 mov bx,[0x2e]
db 0x53                                 ; 3D27 push bx
db 0x8B, 0x1E, 0x35, 0x03               ; 3D28 mov bx,[0x335]
db 0x53                                 ; 3D2C push bx
db 0x52                                 ; 3D2D push dx
db 0xEB, 0x28                           ; 3D2E jmp short 0x3d58
db 0x74, 0x03                           ; 3D30 jz 0x3d35
db 0xE9, 0x89, 0xCA                     ; 3D32 jmp 0x7be
db 0x87, 0xDA                           ; 3D35 xchg bx,dx
db 0xE8, 0x3F, 0x00                     ; 3D37 call 0x3d79
db 0x75, 0x67                           ; 3D3A jnz 0x3da3
db 0x8B, 0xE3                           ; 3D3C mov sp,bx
db 0x89, 0x1E, 0x45, 0x03               ; 3D3E mov [0x345],bx
db 0x8B, 0x16, 0x2E, 0x00               ; 3D42 mov dx,[0x2e]
db 0x89, 0x16, 0x5A, 0x04               ; 3D46 mov [0x45a],dx
db 0x43                                 ; 3D4A inc bx
db 0x43                                 ; 3D4B inc bx
db 0x8B, 0x17                           ; 3D4C mov dx,[bx]
db 0x43                                 ; 3D4E inc bx
db 0x43                                 ; 3D4F inc bx
db 0x8B, 0x1F                           ; 3D50 mov bx,[bx]
db 0x89, 0x1E, 0x2E, 0x00               ; 3D52 mov [0x2e],bx
db 0x87, 0xDA                           ; 3D56 xchg bx,dx
db 0xE8, 0xCC, 0xD9                     ; 3D58 call 0x1727
db 0x53                                 ; 3D5B push bx
db 0xE8, 0x84, 0x27                     ; 3D5C call 0x64e3
db 0x5B                                 ; 3D5F pop bx
db 0x74, 0x09                           ; 3D60 jz 0x3d6b
db 0xB9, 0xB1, 0x00                     ; 3D62 mov cx,0xb1
db 0x8A, 0xE9                           ; 3D65 mov ch,cl
db 0x51                                 ; 3D67 push cx
db 0xE9, 0x7D, 0xD1                     ; 3D68 jmp 0xee8
db 0x8B, 0x1E, 0x5A, 0x04               ; 3D6B mov bx,[0x45a]
db 0x89, 0x1E, 0x2E, 0x00               ; 3D6F mov [0x2e],bx
db 0x5B                                 ; 3D73 pop bx
db 0x59                                 ; 3D74 pop cx
db 0x59                                 ; 3D75 pop cx
db 0xE9, 0x6F, 0xD1                     ; 3D76 jmp 0xee8
db 0xBB, 0x04, 0x00                     ; 3D79 mov bx,0x4
db 0x03, 0xDC                           ; 3D7C add bx,sp
db 0x43                                 ; 3D7E inc bx
db 0x8A, 0x07                           ; 3D7F mov al,[bx]
db 0x43                                 ; 3D81 inc bx
db 0xB9, 0x82, 0x00                     ; 3D82 mov cx,0x82
db 0x3A, 0xC1                           ; 3D85 cmp al,cl
db 0x75, 0x07                           ; 3D87 jnz 0x3d90
db 0xB9, 0x12, 0x00                     ; 3D89 mov cx,0x12
db 0x03, 0xD9                           ; 3D8C add bx,cx
db 0xEB, 0xEE                           ; 3D8E jmp short 0x3d7e
db 0xB9, 0xB1, 0x00                     ; 3D90 mov cx,0xb1
db 0x3A, 0xC1                           ; 3D93 cmp al,cl
db 0x74, 0x01                           ; 3D95 jz 0x3d98
db 0xC3                                 ; 3D97 ret
db 0x39, 0x17                           ; 3D98 cmp [bx],dx
db 0xB9, 0x06, 0x00                     ; 3D9A mov cx,0x6
db 0x74, 0xF8                           ; 3D9D jz 0x3d97
db 0x03, 0xD9                           ; 3D9F add bx,cx
db 0xEB, 0xDB                           ; 3DA1 jmp short 0x3d7e
db 0xBA, 0x1E, 0x00                     ; 3DA3 mov dx,0x1e
db 0xE9, 0x2F, 0xCA                     ; 3DA6 jmp 0x7d8
db 0xE8, 0x3A, 0x07                     ; 3DA9 call 0x44e6
db 0x4B                                 ; 3DAC dec bx
db 0xE8, 0x6D, 0xD1                     ; 3DAD call 0xf1d
db 0x74, 0x55                           ; 3DB0 jz 0x3e07
db 0xE8, 0x72, 0xD9                     ; 3DB2 call 0x1727
db 0x53                                 ; 3DB5 push bx
db 0xE8, 0x6C, 0xDD                     ; 3DB6 call 0x1b25
db 0x74, 0x3D                           ; 3DB9 jz 0x3df8
db 0xE8, 0x0A, 0x33                     ; 3DBB call 0x70c8
db 0xE8, 0x8B, 0xE8                     ; 3DBE call 0x264c
db 0x8B, 0x1E, 0xA3, 0x04               ; 3DC1 mov bx,[0x4a3]
db 0x43                                 ; 3DC5 inc bx
db 0x8A, 0x17                           ; 3DC6 mov dl,[bx]
db 0x43                                 ; 3DC8 inc bx
db 0x8A, 0x37                           ; 3DC9 mov dh,[bx]
db 0x8B, 0xF2                           ; 3DCB mov si,dx
db 0xAC                                 ; 3DCD lodsb
db 0x3C, 0x20                           ; 3DCE cmp al,0x20
db 0x75, 0x09                           ; 3DD0 jnz 0x3ddb
db 0x42                                 ; 3DD2 inc dx
db 0x88, 0x37                           ; 3DD3 mov [bx],dh
db 0x4B                                 ; 3DD5 dec bx
db 0x88, 0x17                           ; 3DD6 mov [bx],dl
db 0x4B                                 ; 3DD8 dec bx
db 0xFE, 0x0F                           ; 3DD9 dec byte [bx]
db 0xE8, 0xDC, 0xE8                     ; 3DDB call 0x26ba
db 0x5B                                 ; 3DDE pop bx
db 0x4B                                 ; 3DDF dec bx
db 0xE8, 0x3A, 0xD1                     ; 3DE0 call 0xf1d
db 0x74, 0x22                           ; 3DE3 jz 0x3e07
db 0x3C, 0x3B                           ; 3DE5 cmp al,0x3b
db 0x74, 0x05                           ; 3DE7 jz 0x3dee
db 0xE8, 0x08, 0xF0                     ; 3DE9 call 0x2df4
db 0x2C, 0x4B                           ; 3DEC sub al,0x4b
db 0xE8, 0x2C, 0xD1                     ; 3DEE call 0xf1d
db 0xB0, 0x2C                           ; 3DF1 mov al,0x2c
db 0xE8, 0xAF, 0xED                     ; 3DF3 call 0x2ba5
db 0xEB, 0xBA                           ; 3DF6 jmp short 0x3db2
db 0xB0, 0x22                           ; 3DF8 mov al,0x22
db 0xE8, 0xA8, 0xED                     ; 3DFA call 0x2ba5
db 0xE8, 0xBA, 0xE8                     ; 3DFD call 0x26ba
db 0xB0, 0x22                           ; 3E00 mov al,0x22
db 0xE8, 0xA0, 0xED                     ; 3E02 call 0x2ba5
db 0xEB, 0xD7                           ; 3E05 jmp short 0x3dde
db 0xE8, 0x67, 0xEE                     ; 3E07 call 0x2c71
db 0xE9, 0x8B, 0xD6                     ; 3E0A jmp 0x1498
db 0xCD, 0xA8                           ; 3E0D int 0xa8
db 0x53                                 ; 3E0F push bx
db 0x8A, 0xF2                           ; 3E10 mov dh,dl
db 0xE8, 0x87, 0x01                     ; 3E12 call 0x3f9c
db 0x74, 0x09                           ; 3E15 jz 0x3e20
db 0x3C, 0x3A                           ; 3E17 cmp al,0x3a
db 0x74, 0x0F                           ; 3E19 jz 0x3e2a
db 0xE8, 0x7E, 0x01                     ; 3E1B call 0x3f9c
db 0x79, 0xF7                           ; 3E1E jns 0x3e17
db 0x8A, 0xD6                           ; 3E20 mov dl,dh
db 0x5B                                 ; 3E22 pop bx
db 0x32, 0xC0                           ; 3E23 xor al,al
db 0xB0, 0xFC                           ; 3E25 mov al,0xfc
db 0xCD, 0xAB                           ; 3E27 int 0xab
db 0xC3                                 ; 3E29 ret
db 0x8A, 0xC6                           ; 3E2A mov al,dh
db 0x2A, 0xC2                           ; 3E2C sub al,dl
db 0xFE, 0xC8                           ; 3E2E dec al
db 0x3C, 0x02                           ; 3E30 cmp al,0x2
db 0x73, 0x05                           ; 3E32 jnc 0x3e39
db 0xCD, 0xAC                           ; 3E34 int 0xac
db 0xE9, 0x61, 0xC9                     ; 3E36 jmp 0x79a
db 0x3C, 0x05                           ; 3E39 cmp al,0x5
db 0x72, 0x03                           ; 3E3B jc 0x3e40
db 0xE9, 0x5A, 0xC9                     ; 3E3D jmp 0x79a
db 0x59                                 ; 3E40 pop cx
db 0x52                                 ; 3E41 push dx
db 0x51                                 ; 3E42 push cx
db 0x8A, 0xC8                           ; 3E43 mov cl,al
db 0x8A, 0xE8                           ; 3E45 mov ch,al
db 0xBA, 0x9C, 0x3E                     ; 3E47 mov dx,0x3e9c
db 0x5E                                 ; 3E4A pop si
db 0x87, 0xDE                           ; 3E4B xchg bx,si
db 0x56                                 ; 3E4D push si
db 0x53                                 ; 3E4E push bx
db 0x8A, 0x07                           ; 3E4F mov al,[bx]
db 0x3C, 0x61                           ; 3E51 cmp al,0x61
db 0x72, 0x06                           ; 3E53 jc 0x3e5b
db 0x3C, 0x7B                           ; 3E55 cmp al,0x7b
db 0x73, 0x02                           ; 3E57 jnc 0x3e5b
db 0x2C, 0x20                           ; 3E59 sub al,0x20
db 0x51                                 ; 3E5B push cx
db 0x8A, 0xE8                           ; 3E5C mov ch,al
db 0x8B, 0xF2                           ; 3E5E mov si,dx
db 0x2E, 0xAC                           ; 3E60 cs lodsb
db 0x43                                 ; 3E62 inc bx
db 0x42                                 ; 3E63 inc dx
db 0x3A, 0xC5                           ; 3E64 cmp al,ch
db 0x59                                 ; 3E66 pop cx
db 0x75, 0x15                           ; 3E67 jnz 0x3e7e
db 0xFE, 0xC9                           ; 3E69 dec cl
db 0x75, 0xE2                           ; 3E6B jnz 0x3e4f
db 0x8B, 0xF2                           ; 3E6D mov si,dx
db 0x2E, 0xAC                           ; 3E6F cs lodsb
db 0x0A, 0xC0                           ; 3E71 or al,al
db 0x78, 0x03                           ; 3E73 js 0x3e78
db 0xE9, 0x06, 0x00                     ; 3E75 jmp 0x3e7e
db 0x5B                                 ; 3E78 pop bx
db 0x5B                                 ; 3E79 pop bx
db 0x5A                                 ; 3E7A pop dx
db 0x0A, 0xC0                           ; 3E7B or al,al
db 0xC3                                 ; 3E7D ret
db 0x0A, 0xC0                           ; 3E7E or al,al
db 0x78, 0xEB                           ; 3E80 js 0x3e6d
db 0x8B, 0xF2                           ; 3E82 mov si,dx
db 0x2E, 0xAC                           ; 3E84 cs lodsb
db 0x0A, 0xC0                           ; 3E86 or al,al
db 0x9F                                 ; 3E88 lahf
db 0x42                                 ; 3E89 inc dx
db 0x9E                                 ; 3E8A sahf
db 0x79, 0xF5                           ; 3E8B jns 0x3e82
db 0x8A, 0xCD                           ; 3E8D mov cl,ch
db 0x5B                                 ; 3E8F pop bx
db 0x53                                 ; 3E90 push bx
db 0x8B, 0xF2                           ; 3E91 mov si,dx
db 0x2E, 0xAC                           ; 3E93 cs lodsb
db 0x0A, 0xC0                           ; 3E95 or al,al
db 0x75, 0xB6                           ; 3E97 jnz 0x3e4f
db 0xE9, 0xFE, 0xC8                     ; 3E99 jmp 0x79a
db 0x4B                                 ; 3E9C dec bx
db 0x59                                 ; 3E9D pop cx
db 0x42                                 ; 3E9E inc dx
db 0x44                                 ; 3E9F inc sp
db 0xFF, 0x53, 0x43                     ; 3EA0 call near [bp+di+0x43]
db 0x52                                 ; 3EA3 push dx
db 0x4E                                 ; 3EA4 dec si
db 0xFE, 0x4C, 0x50                     ; 3EA5 dec byte [si+0x50]
db 0x54                                 ; 3EA8 push sp
db 0x31, 0xFD                           ; 3EA9 xor bp,di
db 0x43                                 ; 3EAB inc bx
db 0x41                                 ; 3EAC inc cx
db 0x53                                 ; 3EAD push bx
db 0x31, 0xFC                           ; 3EAE xor sp,di
db 0x00, 0x7B, 0x58                     ; 3EB0 add [bp+di+0x58],bh
db 0x91                                 ; 3EB3 xchg ax,cx
db 0x58                                 ; 3EB4 pop ax
db 0xA7                                 ; 3EB5 cmpsw
db 0x58                                 ; 3EB6 pop ax
db 0xBD, 0x58, 0xCD                     ; 3EB7 mov bp,0xcd58
db 0xA9, 0x53, 0x52                     ; 3EBA test ax,0x5253
db 0x9F                                 ; 3EBD lahf
db 0x86, 0xC4                           ; 3EBE xchg al,ah
db 0x50                                 ; 3EC0 push ax
db 0x86, 0xC4                           ; 3EC1 xchg al,ah
db 0xBA, 0x2E, 0x00                     ; 3EC3 mov dx,0x2e
db 0x03, 0xDA                           ; 3EC6 add bx,dx
db 0xB0, 0xFF                           ; 3EC8 mov al,0xff
db 0x2A, 0x07                           ; 3ECA sub al,[bx]
db 0x02, 0xC0                           ; 3ECC add al,al
db 0x8A, 0xD0                           ; 3ECE mov dl,al
db 0xCD, 0xAA                           ; 3ED0 int 0xaa
db 0xB6, 0x00                           ; 3ED2 mov dh,0x0
db 0xBB, 0xB1, 0x3E                     ; 3ED4 mov bx,0x3eb1
db 0x03, 0xDA                           ; 3ED7 add bx,dx
db 0x2E, 0x8A, 0x17                     ; 3ED9 mov dl,[cs:bx]
db 0x43                                 ; 3EDC inc bx
db 0x2E, 0x8A, 0x37                     ; 3EDD mov dh,[cs:bx]
db 0x58                                 ; 3EE0 pop ax
db 0x86, 0xC4                           ; 3EE1 xchg al,ah
db 0x9E                                 ; 3EE3 sahf
db 0x8A, 0xD8                           ; 3EE4 mov bl,al
db 0xB7, 0x00                           ; 3EE6 mov bh,0x0
db 0x03, 0xDA                           ; 3EE8 add bx,dx
db 0x2E, 0x8A, 0x17                     ; 3EEA mov dl,[cs:bx]
db 0x43                                 ; 3EED inc bx
db 0x2E, 0x8A, 0x37                     ; 3EEE mov dh,[cs:bx]
db 0x87, 0xDA                           ; 3EF1 xchg bx,dx
db 0x5A                                 ; 3EF3 pop dx
db 0x5E                                 ; 3EF4 pop si
db 0x87, 0xDE                           ; 3EF5 xchg bx,si
db 0x56                                 ; 3EF7 push si
db 0xC3                                 ; 3EF8 ret
db 0x47                                 ; 3EF9 inc di
db 0x00, 0x00                           ; 3EFA add [bx+si],al
db 0x00, 0x00                           ; 3EFC add [bx+si],al
db 0x00, 0x00                           ; 3EFE add [bx+si],al
db 0x00, 0x00                           ; 3F00 add [bx+si],al
db 0x00, 0x00                           ; 3F02 add [bx+si],al
db 0x00, 0x00                           ; 3F04 add [bx+si],al
db 0x00, 0x00                           ; 3F06 add [bx+si],al
db 0x00, 0x00                           ; 3F08 add [bx+si],al
db 0x00, 0x00                           ; 3F0A add [bx+si],al
db 0x00, 0x00                           ; 3F0C add [bx+si],al
db 0x00, 0x00                           ; 3F0E add [bx+si],al
db 0x00, 0x00                           ; 3F10 add [bx+si],al
db 0x00, 0xE8                           ; 3F12 add al,ch
db 0x11, 0xD8                           ; 3F14 adc ax,bx
db 0x53                                 ; 3F16 push bx
db 0xE8, 0x8C, 0xE9                     ; 3F17 call 0x28a6
db 0x8A, 0x07                           ; 3F1A mov al,[bx]
db 0x0A, 0xC0                           ; 3F1C or al,al
db 0x74, 0x4F                           ; 3F1E jz 0x3f6f
db 0x43                                 ; 3F20 inc bx
db 0x8A, 0x17                           ; 3F21 mov dl,[bx]
db 0x43                                 ; 3F23 inc bx
db 0x8A, 0x3F                           ; 3F24 mov bh,[bx]
db 0x8A, 0xDA                           ; 3F26 mov bl,dl
db 0x8A, 0xD0                           ; 3F28 mov dl,al
db 0x32, 0xC0                           ; 3F2A xor al,al
db 0xA2, 0xFF, 0x06                     ; 3F2C mov [0x6ff],al
db 0xE8, 0xDB, 0xFE                     ; 3F2F call 0x3e0d
db 0x9F                                 ; 3F32 lahf
db 0x86, 0xC4                           ; 3F33 xchg al,ah
db 0x50                                 ; 3F35 push ax
db 0x86, 0xC4                           ; 3F36 xchg al,ah
db 0xB9, 0xF0, 0x04                     ; 3F38 mov cx,0x4f0
db 0xB6, 0x0B                           ; 3F3B mov dh,0xb
db 0xFE, 0xC2                           ; 3F3D inc dl
db 0xFE, 0xCA                           ; 3F3F dec dl
db 0x74, 0x4D                           ; 3F41 jz 0x3f90
db 0x8A, 0x07                           ; 3F43 mov al,[bx]
db 0x3C, 0x20                           ; 3F45 cmp al,0x20
db 0x72, 0x26                           ; 3F47 jc 0x3f6f
db 0x3C, 0x2E                           ; 3F49 cmp al,0x2e
db 0x74, 0x28                           ; 3F4B jz 0x3f75
db 0x8B, 0xF9                           ; 3F4D mov di,cx
db 0xAA                                 ; 3F4F stosb
db 0x41                                 ; 3F50 inc cx
db 0x43                                 ; 3F51 inc bx
db 0xFE, 0xCE                           ; 3F52 dec dh
db 0x75, 0xE9                           ; 3F54 jnz 0x3f3f
db 0x58                                 ; 3F56 pop ax
db 0x86, 0xC4                           ; 3F57 xchg al,ah
db 0x9E                                 ; 3F59 sahf
db 0x9F                                 ; 3F5A lahf
db 0x86, 0xC4                           ; 3F5B xchg al,ah
db 0x50                                 ; 3F5D push ax
db 0x86, 0xC4                           ; 3F5E xchg al,ah
db 0x8A, 0xF0                           ; 3F60 mov dh,al
db 0xA0, 0xF0, 0x04                     ; 3F62 mov al,[0x4f0]
db 0xFE, 0xC0                           ; 3F65 inc al
db 0x74, 0x06                           ; 3F67 jz 0x3f6f
db 0x58                                 ; 3F69 pop ax
db 0x86, 0xC4                           ; 3F6A xchg al,ah
db 0x9E                                 ; 3F6C sahf
db 0x5B                                 ; 3F6D pop bx
db 0xC3                                 ; 3F6E ret
db 0xE9, 0x28, 0xC8                     ; 3F6F jmp 0x79a
db 0x43                                 ; 3F72 inc bx
db 0xEB, 0xCA                           ; 3F73 jmp short 0x3f3f
db 0xB0, 0x01                           ; 3F75 mov al,0x1
db 0xA2, 0xFF, 0x06                     ; 3F77 mov [0x6ff],al
db 0x8A, 0xC6                           ; 3F7A mov al,dh
db 0x3C, 0x0B                           ; 3F7C cmp al,0xb
db 0x74, 0xEF                           ; 3F7E jz 0x3f6f
db 0x3C, 0x03                           ; 3F80 cmp al,0x3
db 0x72, 0xEB                           ; 3F82 jc 0x3f6f
db 0x74, 0xEC                           ; 3F84 jz 0x3f72
db 0xB0, 0x20                           ; 3F86 mov al,0x20
db 0x8B, 0xF9                           ; 3F88 mov di,cx
db 0xAA                                 ; 3F8A stosb
db 0x41                                 ; 3F8B inc cx
db 0xFE, 0xCE                           ; 3F8C dec dh
db 0xEB, 0xEA                           ; 3F8E jmp short 0x3f7a
db 0xB0, 0x20                           ; 3F90 mov al,0x20
db 0x8B, 0xF9                           ; 3F92 mov di,cx
db 0xAA                                 ; 3F94 stosb
db 0x41                                 ; 3F95 inc cx
db 0xFE, 0xCE                           ; 3F96 dec dh
db 0x75, 0xF6                           ; 3F98 jnz 0x3f90
db 0xEB, 0xBA                           ; 3F9A jmp short 0x3f56
db 0x8A, 0x07                           ; 3F9C mov al,[bx]
db 0x43                                 ; 3F9E inc bx
db 0xFE, 0xCA                           ; 3F9F dec dl
db 0xC3                                 ; 3FA1 ret
db 0xE8, 0x7A, 0xDF                     ; 3FA2 call 0x1f1f
db 0x8A, 0xD8                           ; 3FA5 mov bl,al
db 0xA0, 0xDF, 0x04                     ; 3FA7 mov al,[0x4df]
db 0x3A, 0xC3                           ; 3FAA cmp al,bl
db 0x73, 0x03                           ; 3FAC jnc 0x3fb1
db 0xE9, 0xE9, 0xC7                     ; 3FAE jmp 0x79a
db 0xB7, 0x00                           ; 3FB1 mov bh,0x0
db 0x03, 0xDB                           ; 3FB3 add bx,bx
db 0x87, 0xDA                           ; 3FB5 xchg bx,dx
db 0x8B, 0x1E, 0xE0, 0x04               ; 3FB7 mov bx,[0x4e0]
db 0x03, 0xDA                           ; 3FBB add bx,dx
db 0x8B, 0x1F                           ; 3FBD mov bx,[bx]
db 0xA0, 0x36, 0x05                     ; 3FBF mov al,[0x536]
db 0xFE, 0xC0                           ; 3FC2 inc al
db 0x74, 0xDB                           ; 3FC4 jz 0x3fa1
db 0x8A, 0x07                           ; 3FC6 mov al,[bx]
db 0x0A, 0xC0                           ; 3FC8 or al,al
db 0x74, 0xD5                           ; 3FCA jz 0x3fa1
db 0x53                                 ; 3FCC push bx
db 0xBA, 0x2E, 0x00                     ; 3FCD mov dx,0x2e
db 0x03, 0xDA                           ; 3FD0 add bx,dx
db 0x8A, 0x07                           ; 3FD2 mov al,[bx]
db 0x3C, 0x09                           ; 3FD4 cmp al,0x9
db 0x73, 0x05                           ; 3FD6 jnc 0x3fdd
db 0xCD, 0xDC                           ; 3FD8 int 0xdc
db 0xE9, 0xC0, 0xC7                     ; 3FDA jmp 0x79d
db 0x5B                                 ; 3FDD pop bx
db 0x8A, 0x07                           ; 3FDE mov al,[bx]
db 0x0A, 0xC0                           ; 3FE0 or al,al
db 0xF9                                 ; 3FE2 stc
db 0xC3                                 ; 3FE3 ret
db 0x4B                                 ; 3FE4 dec bx
db 0xE8, 0x35, 0xCF                     ; 3FE5 call 0xf1d
db 0x3C, 0x23                           ; 3FE8 cmp al,0x23
db 0x75, 0x03                           ; 3FEA jnz 0x3fef
db 0xE8, 0x2E, 0xCF                     ; 3FEC call 0xf1d
db 0xE8, 0x2A, 0xDF                     ; 3FEF call 0x1f1c
db 0x5E                                 ; 3FF2 pop si
db 0x87, 0xDE                           ; 3FF3 xchg bx,si
db 0x56                                 ; 3FF5 push si
db 0x53                                 ; 3FF6 push bx
db 0xE8, 0xAB, 0xFF                     ; 3FF7 call 0x3fa5
db 0x75, 0x03                           ; 3FFA jnz 0x3fff
db 0xE9, 0x9B, 0xC7                     ; 3FFC jmp 0x79a
db 0x89, 0x1E, 0xE9, 0x04               ; 3FFF mov [0x4e9],bx
db 0xC3                                 ; 4003 ret
db 0xB9, 0x98, 0x14                     ; 4004 mov cx,0x1498
db 0x51                                 ; 4007 push cx
db 0xE8, 0x1C, 0xD7                     ; 4008 call 0x1727
db 0x8A, 0x07                           ; 400B mov al,[bx]
db 0x3C, 0x2C                           ; 400D cmp al,0x2c
db 0x75, 0x59                           ; 400F jnz 0x406a
db 0x53                                 ; 4011 push bx
db 0xE8, 0x91, 0xE8                     ; 4012 call 0x28a6
db 0x8A, 0x07                           ; 4015 mov al,[bx]
db 0x0A, 0xC0                           ; 4017 or al,al
db 0x75, 0x03                           ; 4019 jnz 0x401e
db 0xE9, 0x7C, 0xC7                     ; 401B jmp 0x79a
db 0x43                                 ; 401E inc bx
db 0x8B, 0x1F                           ; 401F mov bx,[bx]
db 0x8A, 0x07                           ; 4021 mov al,[bx]
db 0x24, 0xDF                           ; 4023 and al,0xdf
db 0xB2, 0x01                           ; 4025 mov dl,0x1
db 0x3C, 0x49                           ; 4027 cmp al,0x49
db 0x74, 0x15                           ; 4029 jz 0x4040
db 0xB2, 0x02                           ; 402B mov dl,0x2
db 0x3C, 0x4F                           ; 402D cmp al,0x4f
db 0x74, 0x0F                           ; 402F jz 0x4040
db 0xB2, 0x04                           ; 4031 mov dl,0x4
db 0x3C, 0x52                           ; 4033 cmp al,0x52
db 0x74, 0x09                           ; 4035 jz 0x4040
db 0xB2, 0x08                           ; 4037 mov dl,0x8
db 0x3C, 0x41                           ; 4039 cmp al,0x41
db 0x74, 0x03                           ; 403B jz 0x4040
db 0xE9, 0x54, 0xC7                     ; 403D jmp 0x794
db 0x5B                                 ; 4040 pop bx
db 0x52                                 ; 4041 push dx
db 0xE8, 0xAF, 0xED                     ; 4042 call 0x2df4
db 0x2C, 0x3C                           ; 4045 sub al,0x3c
db 0x23, 0x75, 0x03                     ; 4047 and si,[di+0x3]
db 0xE8, 0xD0, 0xCE                     ; 404A call 0xf1d
db 0xE8, 0xCC, 0xDE                     ; 404D call 0x1f1c
db 0xE8, 0xA1, 0xED                     ; 4050 call 0x2df4
db 0x2C, 0x8A                           ; 4053 sub al,0x8a
db 0xC2, 0x0A, 0xC0                     ; 4055 ret 0xc00a
db 0x75, 0x03                           ; 4058 jnz 0x405d
db 0xE9, 0x3D, 0xC7                     ; 405A jmp 0x79a
db 0x50                                 ; 405D push ax
db 0xE8, 0xB2, 0xFE                     ; 405E call 0x3f13
db 0x58                                 ; 4061 pop ax
db 0x59                                 ; 4062 pop cx
db 0x8A, 0xD1                           ; 4063 mov dl,cl
db 0xCD, 0xDD                           ; 4065 int 0xdd
db 0xE9, 0x83, 0x00                     ; 4067 jmp 0x40ed
db 0xE8, 0xA9, 0xFE                     ; 406A call 0x3f16
db 0x8A, 0x07                           ; 406D mov al,[bx]
db 0x3C, 0x82                           ; 406F cmp al,0x82
db 0xB2, 0x04                           ; 4071 mov dl,0x4
db 0x75, 0x59                           ; 4073 jnz 0x40ce
db 0xE8, 0xA5, 0xCE                     ; 4075 call 0xf1d
db 0x3C, 0x85                           ; 4078 cmp al,0x85
db 0xB2, 0x01                           ; 407A mov dl,0x1
db 0x74, 0x4D                           ; 407C jz 0x40cb
db 0x3C, 0x4F                           ; 407E cmp al,0x4f
db 0x74, 0x20                           ; 4080 jz 0x40a2
db 0x3C, 0x49                           ; 4082 cmp al,0x49
db 0x74, 0x37                           ; 4084 jz 0x40bd
db 0xE8, 0x6B, 0xED                     ; 4086 call 0x2df4
db 0x41                                 ; 4089 inc cx
db 0xE8, 0x67, 0xED                     ; 408A call 0x2df4
db 0x50                                 ; 408D push ax
db 0xE8, 0x63, 0xED                     ; 408E call 0x2df4
db 0x50                                 ; 4091 push ax
db 0xE8, 0x5F, 0xED                     ; 4092 call 0x2df4
db 0x45                                 ; 4095 inc bp
db 0xE8, 0x5B, 0xED                     ; 4096 call 0x2df4
db 0x4E                                 ; 4099 dec si
db 0xE8, 0x57, 0xED                     ; 409A call 0x2df4
db 0x44                                 ; 409D inc sp
db 0xB2, 0x08                           ; 409E mov dl,0x8
db 0xEB, 0x2C                           ; 40A0 jmp short 0x40ce
db 0xE8, 0x78, 0xCE                     ; 40A2 call 0xf1d
db 0xE8, 0x4C, 0xED                     ; 40A5 call 0x2df4
db 0x55                                 ; 40A8 push bp
db 0xE8, 0x48, 0xED                     ; 40A9 call 0x2df4
db 0x54                                 ; 40AC push sp
db 0xE8, 0x44, 0xED                     ; 40AD call 0x2df4
db 0x50                                 ; 40B0 push ax
db 0xE8, 0x40, 0xED                     ; 40B1 call 0x2df4
db 0x55                                 ; 40B4 push bp
db 0xE8, 0x3C, 0xED                     ; 40B5 call 0x2df4
db 0x54                                 ; 40B8 push sp
db 0xB2, 0x02                           ; 40B9 mov dl,0x2
db 0xEB, 0x11                           ; 40BB jmp short 0x40ce
db 0xE8, 0x5D, 0xCE                     ; 40BD call 0xf1d
db 0xE8, 0x31, 0xED                     ; 40C0 call 0x2df4
db 0x42                                 ; 40C3 inc dx
db 0xE8, 0x2D, 0xED                     ; 40C4 call 0x2df4
db 0x4D                                 ; 40C7 dec bp
db 0xB2, 0x20                           ; 40C8 mov dl,0x20
db 0x4B                                 ; 40CA dec bx
db 0xE8, 0x4F, 0xCE                     ; 40CB call 0xf1d
db 0xE8, 0x23, 0xED                     ; 40CE call 0x2df4
db 0x41                                 ; 40D1 inc cx
db 0xE8, 0x1F, 0xED                     ; 40D2 call 0x2df4
db 0x53                                 ; 40D5 push bx
db 0x52                                 ; 40D6 push dx
db 0x8A, 0x07                           ; 40D7 mov al,[bx]
db 0x3C, 0x23                           ; 40D9 cmp al,0x23
db 0x75, 0x03                           ; 40DB jnz 0x40e0
db 0xE8, 0x3D, 0xCE                     ; 40DD call 0xf1d
db 0xE8, 0x39, 0xDE                     ; 40E0 call 0x1f1c
db 0x0A, 0xC0                           ; 40E3 or al,al
db 0x75, 0x03                           ; 40E5 jnz 0x40ea
db 0xE9, 0xB0, 0xC6                     ; 40E7 jmp 0x79a
db 0xCD, 0xDE                           ; 40EA int 0xde
db 0xB4, 0x52                           ; 40EC mov ah,0x52
db 0x4B                                 ; 40EE dec bx
db 0x8A, 0xD0                           ; 40EF mov dl,al
db 0xE8, 0x29, 0xCE                     ; 40F1 call 0xf1d
db 0x74, 0x03                           ; 40F4 jz 0x40f9
db 0xE9, 0xC5, 0xC6                     ; 40F6 jmp 0x7be
db 0x5E                                 ; 40F9 pop si
db 0x87, 0xDE                           ; 40FA xchg bx,si
db 0x56                                 ; 40FC push si
db 0x8A, 0xC2                           ; 40FD mov al,dl
db 0x9F                                 ; 40FF lahf
db 0x86, 0xC4                           ; 4100 xchg al,ah
db 0x50                                 ; 4102 push ax
db 0x86, 0xC4                           ; 4103 xchg al,ah
db 0x53                                 ; 4105 push bx
db 0xE8, 0x9C, 0xFE                     ; 4106 call 0x3fa5
db 0x74, 0x03                           ; 4109 jz 0x410e
db 0xE9, 0x95, 0xC6                     ; 410B jmp 0x7a3
db 0x5A                                 ; 410E pop dx
db 0x8A, 0xC6                           ; 410F mov al,dh
db 0x3C, 0x09                           ; 4111 cmp al,0x9
db 0xCD, 0xDF                           ; 4113 int 0xdf
db 0x73, 0x03                           ; 4115 jnc 0x411a
db 0xE9, 0x83, 0xC6                     ; 4117 jmp 0x79d
db 0x53                                 ; 411A push bx
db 0xB9, 0x2E, 0x00                     ; 411B mov cx,0x2e
db 0x03, 0xD9                           ; 411E add bx,cx
db 0x88, 0x37                           ; 4120 mov [bx],dh
db 0xB0, 0x00                           ; 4122 mov al,0x0
db 0x5B                                 ; 4124 pop bx
db 0xE9, 0x91, 0xFD                     ; 4125 jmp 0x3eb9
db 0x53                                 ; 4128 push bx
db 0x0A, 0xC0                           ; 4129 or al,al
db 0x75, 0x0A                           ; 412B jnz 0x4137
db 0xA0, 0x36, 0x05                     ; 412D mov al,[0x536]
db 0x24, 0x01                           ; 4130 and al,0x1
db 0x74, 0x03                           ; 4132 jz 0x4137
db 0xE9, 0x46, 0x03                     ; 4134 jmp 0x447d
db 0xE8, 0x6B, 0xFE                     ; 4137 call 0x3fa5
db 0x74, 0x15                           ; 413A jz 0x4151
db 0x89, 0x1E, 0xE9, 0x04               ; 413C mov [0x4e9],bx
db 0x53                                 ; 4140 push bx
db 0xB0, 0x02                           ; 4141 mov al,0x2
db 0x73, 0x03                           ; 4143 jnc 0x4148
db 0xE9, 0x71, 0xFD                     ; 4145 jmp 0x3eb9
db 0xCD, 0xE0                           ; 4148 int 0xe0
db 0xE9, 0x50, 0xC6                     ; 414A jmp 0x79d
db 0xE8, 0x24, 0x03                     ; 414D call 0x4474
db 0x5B                                 ; 4150 pop bx
db 0x53                                 ; 4151 push bx
db 0xBA, 0x31, 0x00                     ; 4152 mov dx,0x31
db 0x03, 0xDA                           ; 4155 add bx,dx
db 0x88, 0x07                           ; 4157 mov [bx],al
db 0x8A, 0xF8                           ; 4159 mov bh,al
db 0x8A, 0xD8                           ; 415B mov bl,al
db 0x89, 0x1E, 0xE9, 0x04               ; 415D mov [0x4e9],bx
db 0x5B                                 ; 4161 pop bx
db 0x02, 0x07                           ; 4162 add al,[bx]
db 0xC6, 0x07, 0x00                     ; 4164 mov byte [bx],0x0
db 0x5B                                 ; 4167 pop bx
db 0xC3                                 ; 4168 ret
db 0xF9                                 ; 4169 stc
db 0xEB, 0x03                           ; 416A jmp short 0x416f
db 0x0D, 0x32, 0xC0                     ; 416C or ax,0xc032
db 0x9F                                 ; 416F lahf
db 0x50                                 ; 4170 push ax
db 0xE8, 0x9F, 0xFD                     ; 4171 call 0x3f13
db 0xCD, 0xE9                           ; 4174 int 0xe9
db 0x58                                 ; 4176 pop ax
db 0x9E                                 ; 4177 sahf
db 0x9F                                 ; 4178 lahf
db 0x50                                 ; 4179 push ax
db 0x74, 0x14                           ; 417A jz 0x4190
db 0x8A, 0x07                           ; 417C mov al,[bx]
db 0x2C, 0x2C                           ; 417E sub al,0x2c
db 0x0A, 0xC0                           ; 4180 or al,al
db 0x75, 0x0C                           ; 4182 jnz 0x4190
db 0xE8, 0x96, 0xCD                     ; 4184 call 0xf1d
db 0xE8, 0x6A, 0xEC                     ; 4187 call 0x2df4
db 0x52                                 ; 418A push dx
db 0x58                                 ; 418B pop ax
db 0x9E                                 ; 418C sahf
db 0xF9                                 ; 418D stc
db 0x9F                                 ; 418E lahf
db 0x50                                 ; 418F push ax
db 0x9F                                 ; 4190 lahf
db 0x50                                 ; 4191 push ax
db 0x32, 0xC0                           ; 4192 xor al,al
db 0xB2, 0x01                           ; 4194 mov dl,0x1
db 0xE8, 0x54, 0xFF                     ; 4196 call 0x40ed
db 0x8B, 0x1E, 0xE9, 0x04               ; 4199 mov bx,[0x4e9]
db 0xB9, 0x31, 0x00                     ; 419D mov cx,0x31
db 0x03, 0xD9                           ; 41A0 add bx,cx
db 0x58                                 ; 41A2 pop ax
db 0x9E                                 ; 41A3 sahf
db 0x1A, 0xC0                           ; 41A4 sbb al,al
db 0x24, 0x80                           ; 41A6 and al,0x80
db 0x0C, 0x01                           ; 41A8 or al,0x1
db 0xA2, 0x36, 0x05                     ; 41AA mov [0x536],al
db 0x58                                 ; 41AD pop ax
db 0x9E                                 ; 41AE sahf
db 0x9F                                 ; 41AF lahf
db 0x50                                 ; 41B0 push ax
db 0x1A, 0xC0                           ; 41B1 sbb al,al
db 0xA2, 0xEF, 0x04                     ; 41B3 mov [0x4ef],al
db 0x8A, 0x07                           ; 41B6 mov al,[bx]
db 0x0A, 0xC0                           ; 41B8 or al,al
db 0x79, 0x03                           ; 41BA jns 0x41bf
db 0xE9, 0xD8, 0x00                     ; 41BC jmp 0x4297
db 0x58                                 ; 41BF pop ax
db 0x9E                                 ; 41C0 sahf
db 0x74, 0x03                           ; 41C1 jz 0x41c6
db 0xE8, 0x57, 0xEB                     ; 41C3 call 0x2d1d
db 0x32, 0xC0                           ; 41C6 xor al,al
db 0xE8, 0x2C, 0xFE                     ; 41C8 call 0x3ff7
db 0xE9, 0x05, 0xC7                     ; 41CB jmp 0x8d3
db 0xE8, 0x42, 0xFD                     ; 41CE call 0x3f13
db 0xCD, 0xEA                           ; 41D1 int 0xea
db 0x4B                                 ; 41D3 dec bx
db 0xE8, 0x46, 0xCD                     ; 41D4 call 0xf1d
db 0xB2, 0x80                           ; 41D7 mov dl,0x80
db 0xF9                                 ; 41D9 stc
db 0x75, 0x03                           ; 41DA jnz 0x41df
db 0xE8, 0x79, 0x05                     ; 41DC call 0x4758
db 0x74, 0x18                           ; 41DF jz 0x41f9
db 0xE8, 0x10, 0xEC                     ; 41E1 call 0x2df4
db 0x2C, 0x3C                           ; 41E4 sub al,0x3c
db 0x50                                 ; 41E6 push ax
db 0xB2, 0x92                           ; 41E7 mov dl,0x92
db 0x75, 0x06                           ; 41E9 jnz 0x41f1
db 0xE8, 0x2F, 0xCD                     ; 41EB call 0xf1d
db 0xF9                                 ; 41EE stc
db 0xEB, 0x08                           ; 41EF jmp short 0x41f9
db 0xE8, 0x00, 0xEC                     ; 41F1 call 0x2df4
db 0x41                                 ; 41F4 inc cx
db 0x0A, 0xC0                           ; 41F5 or al,al
db 0xB2, 0x02                           ; 41F7 mov dl,0x2
db 0x9F                                 ; 41F9 lahf
db 0x50                                 ; 41FA push ax
db 0x8A, 0xC2                           ; 41FB mov al,dl
db 0x24, 0x10                           ; 41FD and al,0x10
db 0xA2, 0x62, 0x04                     ; 41FF mov [0x462],al
db 0x58                                 ; 4202 pop ax
db 0x9E                                 ; 4203 sahf
db 0x9F                                 ; 4204 lahf
db 0x50                                 ; 4205 push ax
db 0xFE, 0xC0                           ; 4206 inc al
db 0xA2, 0x5F, 0x00                     ; 4208 mov [0x5f],al
db 0x32, 0xC0                           ; 420B xor al,al
db 0xE8, 0xDD, 0xFE                     ; 420D call 0x40ed
db 0x58                                 ; 4210 pop ax
db 0x9E                                 ; 4211 sahf
db 0x53                                 ; 4212 push bx
db 0x8B, 0x1E, 0xE9, 0x04               ; 4213 mov bx,[0x4e9]
db 0x8A, 0x07                           ; 4217 mov al,[bx]
db 0x5B                                 ; 4219 pop bx
db 0x24, 0x80                           ; 421A and al,0x80
db 0x75, 0x03                           ; 421C jnz 0x4221
db 0xE9, 0x14, 0xDD                     ; 421E jmp 0x1f35
db 0x53                                 ; 4221 push bx
db 0xE8, 0x63, 0xE1                     ; 4222 call 0x2388
db 0xA0, 0x62, 0x04                     ; 4225 mov al,[0x462]
db 0x0A, 0xC0                           ; 4228 or al,al
db 0x74, 0x03                           ; 422A jz 0x422f
db 0xE8, 0x79, 0x04                     ; 422C call 0x46a8
db 0x8B, 0x1E, 0x58, 0x03               ; 422F mov bx,[0x358]
db 0x89, 0x1E, 0x04, 0x07               ; 4233 mov [0x704],bx
db 0x8B, 0x1E, 0x30, 0x00               ; 4237 mov bx,[0x30]
db 0x53                                 ; 423B push bx
db 0x8B, 0x1E, 0xE9, 0x04               ; 423C mov bx,[0x4e9]
db 0xE8, 0xC6, 0x00                     ; 4240 call 0x4309
db 0x0A, 0xC0                           ; 4243 or al,al
db 0x79, 0x03                           ; 4245 jns 0x424a
db 0xE9, 0x16, 0x00                     ; 4247 jmp 0x4260
db 0xE9, 0x4D, 0xC5                     ; 424A jmp 0x79a
db 0xA0, 0x62, 0x04                     ; 424D mov al,[0x462]
db 0x0A, 0xC0                           ; 4250 or al,al
db 0x74, 0x03                           ; 4252 jz 0x4257
db 0xE8, 0xA2, 0x04                     ; 4254 call 0x46f9
db 0x5B                                 ; 4257 pop bx
db 0x32, 0xC0                           ; 4258 xor al,al
db 0xA2, 0x62, 0x04                     ; 425A mov [0x462],al
db 0xE9, 0xC8, 0xFE                     ; 425D jmp 0x4128
db 0x5B                                 ; 4260 pop bx
db 0xE8, 0x02, 0x00                     ; 4261 call 0x4266
db 0xEB, 0xE7                           ; 4264 jmp short 0x424d
db 0xE8, 0xA0, 0x00                     ; 4266 call 0x4309
db 0x3C, 0xFC                           ; 4269 cmp al,0xfc
db 0x75, 0x03                           ; 426B jnz 0x4270
db 0xE9, 0x0D, 0x1A                     ; 426D jmp 0x5c7d
db 0xCD, 0xEB                           ; 4270 int 0xeb
db 0xE9, 0x1F, 0xC5                     ; 4272 jmp 0x794
db 0x8B, 0x1E, 0x30, 0x00               ; 4275 mov bx,[0x30]
db 0x0A, 0xC0                           ; 4279 or al,al
db 0xE8, 0x03, 0x00                     ; 427B call 0x4281
db 0xE9, 0x46, 0x00                     ; 427E jmp 0x42c7
db 0x9F                                 ; 4281 lahf
db 0x50                                 ; 4282 push ax
db 0xE8, 0x83, 0x00                     ; 4283 call 0x4309
db 0x3C, 0xFC                           ; 4286 cmp al,0xfc
db 0x75, 0x03                           ; 4288 jnz 0x428d
db 0xE9, 0x36, 0x1A                     ; 428A jmp 0x5cc3
db 0x58                                 ; 428D pop ax
db 0x9E                                 ; 428E sahf
db 0xCD, 0xEC                           ; 428F int 0xec
db 0xE9, 0x00, 0xC5                     ; 4291 jmp 0x794
db 0x58                                 ; 4294 pop ax
db 0x9E                                 ; 4295 sahf
db 0xC3                                 ; 4296 ret
db 0x24, 0x20                           ; 4297 and al,0x20
db 0xA2, 0x63, 0x04                     ; 4299 mov [0x463],al
db 0x58                                 ; 429C pop ax
db 0x9E                                 ; 429D sahf
db 0x75, 0x03                           ; 429E jnz 0x42a3
db 0xE9, 0xF1, 0xC4                     ; 42A0 jmp 0x794
db 0xE8, 0x77, 0xEA                     ; 42A3 call 0x2d1d
db 0xA0, 0x63, 0x04                     ; 42A6 mov al,[0x463]
db 0xA2, 0x64, 0x04                     ; 42A9 mov [0x464],al
db 0xE8, 0xAB, 0x00                     ; 42AC call 0x435a
db 0x32, 0xC0                           ; 42AF xor al,al
db 0xE8, 0xF1, 0xFC                     ; 42B1 call 0x3fa5
db 0xC6, 0x07, 0x80                     ; 42B4 mov byte [bx],0x80
db 0x89, 0x1E, 0xE9, 0x04               ; 42B7 mov [0x4e9],bx
db 0xE8, 0x4B, 0x00                     ; 42BB call 0x4309
db 0x0A, 0xC0                           ; 42BE or al,al
db 0x78, 0xB3                           ; 42C0 js 0x4275
db 0xCD, 0xED                           ; 42C2 int 0xed
db 0xE9, 0xD3, 0xC4                     ; 42C4 jmp 0x79a
db 0xA0, 0x64, 0x04                     ; 42C7 mov al,[0x464]
db 0x0A, 0xC0                           ; 42CA or al,al
db 0x74, 0x03                           ; 42CC jz 0x42d1
db 0xE8, 0x28, 0x04                     ; 42CE call 0x46f9
db 0xE8, 0x27, 0xC7                     ; 42D1 call 0x9fb
db 0x43                                 ; 42D4 inc bx
db 0x43                                 ; 42D5 inc bx
db 0x89, 0x1E, 0x58, 0x03               ; 42D6 mov [0x358],bx
db 0xE8, 0x5A, 0xEA                     ; 42DA call 0x2d37
db 0x32, 0xC0                           ; 42DD xor al,al
db 0xA2, 0x36, 0x05                     ; 42DF mov [0x536],al
db 0xE8, 0x43, 0xFE                     ; 42E2 call 0x4128
db 0xA0, 0xEF, 0x04                     ; 42E5 mov al,[0x4ef]
db 0x0A, 0xC0                           ; 42E8 or al,al
db 0x74, 0x03                           ; 42EA jz 0x42ef
db 0xE9, 0xF9, 0xCB                     ; 42EC jmp 0xee8
db 0xE9, 0xC4, 0xC5                     ; 42EF jmp 0x8b6
db 0x87, 0xDA                           ; 42F2 xchg bx,dx
db 0x8B, 0x1E, 0x2F, 0x03               ; 42F4 mov bx,[0x32f]
db 0x87, 0xDA                           ; 42F8 xchg bx,dx
db 0x3B, 0xDA                           ; 42FA cmp bx,dx
db 0x72, 0x98                           ; 42FC jc 0x4296
db 0xE8, 0x1C, 0xEA                     ; 42FE call 0x2d1d
db 0x32, 0xC0                           ; 4301 xor al,al
db 0xA2, 0x36, 0x05                     ; 4303 mov [0x536],al
db 0xE9, 0xEB, 0xE9                     ; 4306 jmp 0x2cf4
db 0x53                                 ; 4309 push bx
db 0x52                                 ; 430A push dx
db 0x8B, 0x1E, 0xE9, 0x04               ; 430B mov bx,[0x4e9]
db 0xBA, 0x2E, 0x00                     ; 430F mov dx,0x2e
db 0x03, 0xDA                           ; 4312 add bx,dx
db 0x8A, 0x07                           ; 4314 mov al,[bx]
db 0x5A                                 ; 4316 pop dx
db 0x5B                                 ; 4317 pop bx
db 0xC3                                 ; 4318 ret
db 0x75, 0x1E                           ; 4319 jnz 0x4339
db 0x53                                 ; 431B push bx
db 0x51                                 ; 431C push cx
db 0x50                                 ; 431D push ax
db 0xBA, 0x26, 0x43                     ; 431E mov dx,0x4326
db 0x52                                 ; 4321 push dx
db 0x51                                 ; 4322 push cx
db 0x0A, 0xC0                           ; 4323 or al,al
db 0xC3                                 ; 4325 ret
db 0x58                                 ; 4326 pop ax
db 0x59                                 ; 4327 pop cx
db 0xFE, 0xC8                           ; 4328 dec al
db 0x79, 0xF0                           ; 432A jns 0x431c
db 0x5B                                 ; 432C pop bx
db 0xC3                                 ; 432D ret
db 0x59                                 ; 432E pop cx
db 0x5B                                 ; 432F pop bx
db 0x8A, 0x07                           ; 4330 mov al,[bx]
db 0x3C, 0x2C                           ; 4332 cmp al,0x2c
db 0x75, 0xF7                           ; 4334 jnz 0x432d
db 0xE8, 0xE4, 0xCB                     ; 4336 call 0xf1d
db 0x51                                 ; 4339 push cx
db 0x8A, 0x07                           ; 433A mov al,[bx]
db 0x3C, 0x23                           ; 433C cmp al,0x23
db 0x75, 0x03                           ; 433E jnz 0x4343
db 0xE8, 0xDA, 0xCB                     ; 4340 call 0xf1d
db 0xE8, 0xD6, 0xDB                     ; 4343 call 0x1f1c
db 0x5E                                 ; 4346 pop si
db 0x87, 0xDE                           ; 4347 xchg bx,si
db 0x56                                 ; 4349 push si
db 0x53                                 ; 434A push bx
db 0xBA, 0x2E, 0x43                     ; 434B mov dx,0x432e
db 0x52                                 ; 434E push dx
db 0xF9                                 ; 434F stc
db 0xFF, 0xE3                           ; 4350 jmp bx
db 0xB9, 0x28, 0x41                     ; 4352 mov cx,0x4128
db 0xA0, 0xDF, 0x04                     ; 4355 mov al,[0x4df]
db 0xEB, 0xBF                           ; 4358 jmp short 0x4319
db 0xA0, 0x36, 0x05                     ; 435A mov al,[0x536]
db 0x0A, 0xC0                           ; 435D or al,al
db 0x78, 0xCC                           ; 435F js 0x432d
db 0xB9, 0x28, 0x41                     ; 4361 mov cx,0x4128
db 0x32, 0xC0                           ; 4364 xor al,al
db 0xA0, 0xDF, 0x04                     ; 4366 mov al,[0x4df]
db 0xEB, 0xAE                           ; 4369 jmp short 0x4319
db 0x32, 0xC0                           ; 436B xor al,al
db 0x8A, 0xE8                           ; 436D mov ch,al
db 0x8A, 0xC5                           ; 436F mov al,ch
db 0xE8, 0x31, 0xFC                     ; 4371 call 0x3fa5
db 0xC6, 0x07, 0x00                     ; 4374 mov byte [bx],0x0
db 0xA0, 0xDF, 0x04                     ; 4377 mov al,[0x4df]
db 0xFE, 0xC5                           ; 437A inc ch
db 0x2A, 0xC5                           ; 437C sub al,ch
db 0x73, 0xEF                           ; 437E jnc 0x436f
db 0x32, 0xC0                           ; 4380 xor al,al
db 0xA2, 0x36, 0x05                     ; 4382 mov [0x536],al
db 0xE8, 0x95, 0xE9                     ; 4385 call 0x2d1d
db 0x8B, 0x1E, 0x30, 0x00               ; 4388 mov bx,[0x30]
db 0x4B                                 ; 438C dec bx
db 0xC6, 0x07, 0x00                     ; 438D mov byte [bx],0x0
db 0xE9, 0xDB, 0xC3                     ; 4390 jmp 0x76e
db 0x5B                                 ; 4393 pop bx
db 0x58                                 ; 4394 pop ax
db 0x86, 0xC4                           ; 4395 xchg al,ah
db 0x9E                                 ; 4397 sahf
db 0x53                                 ; 4398 push bx
db 0x52                                 ; 4399 push dx
db 0x51                                 ; 439A push cx
db 0x9F                                 ; 439B lahf
db 0x86, 0xC4                           ; 439C xchg al,ah
db 0x50                                 ; 439E push ax
db 0x86, 0xC4                           ; 439F xchg al,ah
db 0x8B, 0x1E, 0xE9, 0x04               ; 43A1 mov bx,[0x4e9]
db 0xB0, 0x06                           ; 43A5 mov al,0x6
db 0xE8, 0x05, 0x00                     ; 43A7 call 0x43af
db 0xCD, 0xE3                           ; 43AA int 0xe3
db 0xE9, 0xEB, 0xC3                     ; 43AC jmp 0x79a
db 0x9F                                 ; 43AF lahf
db 0x86, 0xC4                           ; 43B0 xchg al,ah
db 0x50                                 ; 43B2 push ax
db 0x86, 0xC4                           ; 43B3 xchg al,ah
db 0x52                                 ; 43B5 push dx
db 0x87, 0xDA                           ; 43B6 xchg bx,dx
db 0xBB, 0x2E, 0x00                     ; 43B8 mov bx,0x2e
db 0x03, 0xDA                           ; 43BB add bx,dx
db 0x8A, 0x07                           ; 43BD mov al,[bx]
db 0x87, 0xDA                           ; 43BF xchg bx,dx
db 0x5A                                 ; 43C1 pop dx
db 0x3C, 0x09                           ; 43C2 cmp al,0x9
db 0x73, 0x03                           ; 43C4 jnc 0x43c9
db 0xE9, 0xCA, 0x00                     ; 43C6 jmp 0x4493
db 0x58                                 ; 43C9 pop ax
db 0x86, 0xC4                           ; 43CA xchg al,ah
db 0x9E                                 ; 43CC sahf
db 0x5E                                 ; 43CD pop si
db 0x87, 0xDE                           ; 43CE xchg bx,si
db 0x56                                 ; 43D0 push si
db 0x5B                                 ; 43D1 pop bx
db 0xE9, 0xE4, 0xFA                     ; 43D2 jmp 0x3eb9
db 0x51                                 ; 43D5 push cx
db 0x53                                 ; 43D6 push bx
db 0x52                                 ; 43D7 push dx
db 0x8B, 0x1E, 0xE9, 0x04               ; 43D8 mov bx,[0x4e9]
db 0xB0, 0x08                           ; 43DC mov al,0x8
db 0xE8, 0xCE, 0xFF                     ; 43DE call 0x43af
db 0xCD, 0xE4                           ; 43E1 int 0xe4
db 0xE9, 0xB4, 0xC3                     ; 43E3 jmp 0x79a
db 0x5A                                 ; 43E6 pop dx
db 0x5B                                 ; 43E7 pop bx
db 0x59                                 ; 43E8 pop cx
db 0xC3                                 ; 43E9 ret
db 0xE8, 0x30, 0xCB                     ; 43EA call 0xf1d
db 0xE8, 0x04, 0xEA                     ; 43ED call 0x2df4
db 0x24, 0xE8                           ; 43F0 and al,0xe8
db 0x00, 0xEA                           ; 43F2 add dl,ch
db 0x28, 0x53, 0x8B                     ; 43F4 sub [bp+di-0x75],dl
db 0x1E                                 ; 43F7 push ds
db 0xE9, 0x04, 0x53                     ; 43F8 jmp 0x96ff
db 0xBB, 0x00, 0x00                     ; 43FB mov bx,0x0
db 0x89, 0x1E, 0xE9, 0x04               ; 43FE mov [0x4e9],bx
db 0x5B                                 ; 4402 pop bx
db 0x5E                                 ; 4403 pop si
db 0x87, 0xDE                           ; 4404 xchg bx,si
db 0x56                                 ; 4406 push si
db 0xE8, 0x12, 0xDB                     ; 4407 call 0x1f1c
db 0x52                                 ; 440A push dx
db 0x8A, 0x07                           ; 440B mov al,[bx]
db 0x3C, 0x2C                           ; 440D cmp al,0x2c
db 0x75, 0x0B                           ; 440F jnz 0x441c
db 0xE8, 0x09, 0xCB                     ; 4411 call 0xf1d
db 0xE8, 0xCD, 0xFB                     ; 4414 call 0x3fe4
db 0x5B                                 ; 4417 pop bx
db 0x32, 0xC0                           ; 4418 xor al,al
db 0x8A, 0x07                           ; 441A mov al,[bx]
db 0x9F                                 ; 441C lahf
db 0x50                                 ; 441D push ax
db 0xE8, 0xD3, 0xE9                     ; 441E call 0x2df4
db 0x29, 0x58, 0x9E                     ; 4421 sub [bx+si-0x62],bx
db 0x5E                                 ; 4424 pop si
db 0x87, 0xDE                           ; 4425 xchg bx,si
db 0x56                                 ; 4427 push si
db 0x9F                                 ; 4428 lahf
db 0x50                                 ; 4429 push ax
db 0x8A, 0xC3                           ; 442A mov al,bl
db 0x0A, 0xC0                           ; 442C or al,al
db 0x75, 0x03                           ; 442E jnz 0x4433
db 0xE9, 0x26, 0xCC                     ; 4430 jmp 0x1059
db 0x53                                 ; 4433 push bx
db 0xE8, 0x07, 0xE2                     ; 4434 call 0x263e
db 0x87, 0xDA                           ; 4437 xchg bx,dx
db 0x59                                 ; 4439 pop cx
db 0x58                                 ; 443A pop ax
db 0x9E                                 ; 443B sahf
db 0x9F                                 ; 443C lahf
db 0x50                                 ; 443D push ax
db 0x74, 0x28                           ; 443E jz 0x4468
db 0xE8, 0x16, 0xE8                     ; 4440 call 0x2c59
db 0x3C, 0x03                           ; 4443 cmp al,0x3
db 0x74, 0x13                           ; 4445 jz 0x445a
db 0x88, 0x07                           ; 4447 mov [bx],al
db 0x43                                 ; 4449 inc bx
db 0xFE, 0xC9                           ; 444A dec cl
db 0x75, 0xEC                           ; 444C jnz 0x443a
db 0x58                                 ; 444E pop ax
db 0x9E                                 ; 444F sahf
db 0x59                                 ; 4450 pop cx
db 0x5B                                 ; 4451 pop bx
db 0x89, 0x1E, 0xE9, 0x04               ; 4452 mov [0x4e9],bx
db 0x51                                 ; 4456 push cx
db 0xE9, 0x33, 0xE2                     ; 4457 jmp 0x268d
db 0x58                                 ; 445A pop ax
db 0x9E                                 ; 445B sahf
db 0x8B, 0x1E, 0x2E, 0x00               ; 445C mov bx,[0x2e]
db 0x89, 0x1E, 0x47, 0x03               ; 4460 mov [0x347],bx
db 0x5B                                 ; 4464 pop bx
db 0xE9, 0x06, 0xC3                     ; 4465 jmp 0x76e
db 0xE8, 0x6A, 0xFF                     ; 4468 call 0x43d5
db 0x73, 0x03                           ; 446B jnc 0x4470
db 0xE9, 0x30, 0xC3                     ; 446D jmp 0x7a0
db 0xEB, 0xD5                           ; 4470 jmp short 0x4447
db 0xCD, 0xE5                           ; 4472 int 0xe5
db 0xE8, 0x12, 0x00                     ; 4474 call 0x4489
db 0x53                                 ; 4477 push bx
db 0xB5, 0x01                           ; 4478 mov ch,0x1
db 0xE8, 0x02, 0x00                     ; 447A call 0x447f
db 0x5B                                 ; 447D pop bx
db 0xC3                                 ; 447E ret
db 0x32, 0xC0                           ; 447F xor al,al
db 0x88, 0x07                           ; 4481 mov [bx],al
db 0x43                                 ; 4483 inc bx
db 0xFE, 0xCD                           ; 4484 dec ch
db 0x75, 0xF9                           ; 4486 jnz 0x4481
db 0xC3                                 ; 4488 ret
db 0x8B, 0x1E, 0xE9, 0x04               ; 4489 mov bx,[0x4e9]
db 0xBA, 0x33, 0x00                     ; 448D mov dx,0x33
db 0x03, 0xDA                           ; 4490 add bx,dx
db 0xC3                                 ; 4492 ret
db 0x58                                 ; 4493 pop ax
db 0x86, 0xC4                           ; 4494 xchg al,ah
db 0x9E                                 ; 4496 sahf
db 0xC3                                 ; 4497 ret
db 0xE8, 0x07, 0xFB                     ; 4498 call 0x3fa2
db 0x75, 0x03                           ; 449B jnz 0x44a0
db 0xE9, 0xFA, 0xC2                     ; 449D jmp 0x79a
db 0xB0, 0x0A                           ; 44A0 mov al,0xa
db 0x73, 0x03                           ; 44A2 jnc 0x44a7
db 0xE9, 0x12, 0xFA                     ; 44A4 jmp 0x3eb9
db 0xCD, 0xE6                           ; 44A7 int 0xe6
db 0xE9, 0xEE, 0xC2                     ; 44A9 jmp 0x79a
db 0xE8, 0xF3, 0xFA                     ; 44AC call 0x3fa2
db 0x75, 0x03                           ; 44AF jnz 0x44b4
db 0xE9, 0xE6, 0xC2                     ; 44B1 jmp 0x79a
db 0xB0, 0x0C                           ; 44B4 mov al,0xc
db 0x73, 0x03                           ; 44B6 jnc 0x44bb
db 0xE9, 0xFE, 0xF9                     ; 44B8 jmp 0x3eb9
db 0xCD, 0xE7                           ; 44BB int 0xe7
db 0xE9, 0xDA, 0xC2                     ; 44BD jmp 0x79a
db 0xE8, 0xDF, 0xFA                     ; 44C0 call 0x3fa2
db 0x75, 0x03                           ; 44C3 jnz 0x44c8
db 0xE9, 0xD2, 0xC2                     ; 44C5 jmp 0x79a
db 0xB0, 0x0E                           ; 44C8 mov al,0xe
db 0x73, 0x03                           ; 44CA jnc 0x44cf
db 0xE9, 0xEA, 0xF9                     ; 44CC jmp 0x3eb9
db 0xCD, 0xE8                           ; 44CF int 0xe8
db 0xE9, 0xC6, 0xC2                     ; 44D1 jmp 0x79a
db 0xE8, 0xFF, 0xEA                     ; 44D4 call 0x2fd6
db 0x75, 0x03                           ; 44D7 jnz 0x44dc
db 0xE9, 0x1F, 0xCA                     ; 44D9 jmp 0xefb
db 0x32, 0xC0                           ; 44DC xor al,al
db 0xE8, 0x47, 0xFC                     ; 44DE call 0x4128
db 0xB2, 0x42                           ; 44E1 mov dl,0x42
db 0xE9, 0xF2, 0xC2                     ; 44E3 jmp 0x7d8
db 0x3C, 0x23                           ; 44E6 cmp al,0x23
db 0x75, 0xAD                           ; 44E8 jnz 0x4497
db 0xE8, 0x2C, 0xDA                     ; 44EA call 0x1f19
db 0xE8, 0x04, 0xE9                     ; 44ED call 0x2df4
db 0x2C, 0x8A                           ; 44F0 sub al,0x8a
db 0xC2, 0x53, 0xE8                     ; 44F2 ret 0xe853
db 0x00, 0xFB                           ; 44F5 add bl,bh
db 0x5B                                 ; 44F7 pop bx
db 0x8A, 0x07                           ; 44F8 mov al,[bx]
db 0xC3                                 ; 44FA ret
db 0xB9, 0xEC, 0x2D                     ; 44FB mov cx,0x2dec
db 0x51                                 ; 44FE push cx
db 0x32, 0xC0                           ; 44FF xor al,al
db 0xE9, 0x24, 0xFC                     ; 4501 jmp 0x4128
db 0xE8, 0x1E, 0xD6                     ; 4504 call 0x1b25
db 0xB9, 0x9B, 0x16                     ; 4507 mov cx,0x169b
db 0xBA, 0x20, 0x2C                     ; 450A mov dx,0x2c20
db 0x75, 0x1B                           ; 450D jnz 0x452a
db 0x8A, 0xD6                           ; 450F mov dl,dh
db 0xEB, 0x17                           ; 4511 jmp short 0x452a
db 0xB9, 0x98, 0x14                     ; 4513 mov cx,0x1498
db 0x51                                 ; 4516 push cx
db 0xE8, 0xCC, 0xFF                     ; 4517 call 0x44e6
db 0xE8, 0x51, 0xF2                     ; 451A call 0x376e
db 0xE8, 0x1B, 0x1F                     ; 451D call 0x643b
db 0x52                                 ; 4520 push dx
db 0xB9, 0x8A, 0x11                     ; 4521 mov cx,0x118a
db 0x32, 0xC0                           ; 4524 xor al,al
db 0x8A, 0xF0                           ; 4526 mov dh,al
db 0x8A, 0xD0                           ; 4528 mov dl,al
db 0x50                                 ; 452A push ax
db 0x51                                 ; 452B push cx
db 0x53                                 ; 452C push bx
db 0xE8, 0xA5, 0xFE                     ; 452D call 0x43d5
db 0x73, 0x03                           ; 4530 jnc 0x4535
db 0xE9, 0x6B, 0xC2                     ; 4532 jmp 0x7a0
db 0x3C, 0x20                           ; 4535 cmp al,0x20
db 0x75, 0x06                           ; 4537 jnz 0x453f
db 0xFE, 0xC6                           ; 4539 inc dh
db 0xFE, 0xCE                           ; 453B dec dh
db 0x75, 0xEE                           ; 453D jnz 0x452d
db 0x3C, 0x22                           ; 453F cmp al,0x22
db 0x75, 0x13                           ; 4541 jnz 0x4556
db 0x8A, 0xE8                           ; 4543 mov ch,al
db 0x8A, 0xC2                           ; 4545 mov al,dl
db 0x3C, 0x2C                           ; 4547 cmp al,0x2c
db 0x8A, 0xC5                           ; 4549 mov al,ch
db 0x75, 0x09                           ; 454B jnz 0x4556
db 0x8A, 0xF5                           ; 454D mov dh,ch
db 0x8A, 0xD5                           ; 454F mov dl,ch
db 0xE8, 0x81, 0xFE                     ; 4551 call 0x43d5
db 0x72, 0x53                           ; 4554 jc 0x45a9
db 0xBB, 0xF7, 0x01                     ; 4556 mov bx,0x1f7
db 0xB5, 0xFF                           ; 4559 mov ch,0xff
db 0x8A, 0xC8                           ; 455B mov cl,al
db 0x8A, 0xC6                           ; 455D mov al,dh
db 0x3C, 0x22                           ; 455F cmp al,0x22
db 0x8A, 0xC1                           ; 4561 mov al,cl
db 0x74, 0x2E                           ; 4563 jz 0x4593
db 0x3C, 0x0D                           ; 4565 cmp al,0xd
db 0x53                                 ; 4567 push bx
db 0x74, 0x59                           ; 4568 jz 0x45c3
db 0x5B                                 ; 456A pop bx
db 0x3C, 0x0A                           ; 456B cmp al,0xa
db 0x75, 0x24                           ; 456D jnz 0x4593
db 0x8A, 0xC8                           ; 456F mov cl,al
db 0x8A, 0xC2                           ; 4571 mov al,dl
db 0x3C, 0x2C                           ; 4573 cmp al,0x2c
db 0x8A, 0xC1                           ; 4575 mov al,cl
db 0x74, 0x03                           ; 4577 jz 0x457c
db 0xE8, 0x89, 0x00                     ; 4579 call 0x4605
db 0x53                                 ; 457C push bx
db 0xE8, 0x55, 0xFE                     ; 457D call 0x43d5
db 0x5B                                 ; 4580 pop bx
db 0x72, 0x26                           ; 4581 jc 0x45a9
db 0x3C, 0x0D                           ; 4583 cmp al,0xd
db 0x75, 0x0C                           ; 4585 jnz 0x4593
db 0x8A, 0xC2                           ; 4587 mov al,dl
db 0x3C, 0x20                           ; 4589 cmp al,0x20
db 0x74, 0x15                           ; 458B jz 0x45a2
db 0x3C, 0x2C                           ; 458D cmp al,0x2c
db 0xB0, 0x0D                           ; 458F mov al,0xd
db 0x74, 0x0F                           ; 4591 jz 0x45a2
db 0x0A, 0xC0                           ; 4593 or al,al
db 0x74, 0x0B                           ; 4595 jz 0x45a2
db 0x3A, 0xC6                           ; 4597 cmp al,dh
db 0x74, 0x0E                           ; 4599 jz 0x45a9
db 0x3A, 0xC2                           ; 459B cmp al,dl
db 0x74, 0x0A                           ; 459D jz 0x45a9
db 0xE8, 0x63, 0x00                     ; 459F call 0x4605
db 0x53                                 ; 45A2 push bx
db 0xE8, 0x2F, 0xFE                     ; 45A3 call 0x43d5
db 0x5B                                 ; 45A6 pop bx
db 0x73, 0xB2                           ; 45A7 jnc 0x455b
db 0x53                                 ; 45A9 push bx
db 0x3C, 0x22                           ; 45AA cmp al,0x22
db 0x74, 0x04                           ; 45AC jz 0x45b2
db 0x3C, 0x20                           ; 45AE cmp al,0x20
db 0x75, 0x25                           ; 45B0 jnz 0x45d7
db 0xE8, 0x20, 0xFE                     ; 45B2 call 0x43d5
db 0x72, 0x20                           ; 45B5 jc 0x45d7
db 0x3C, 0x20                           ; 45B7 cmp al,0x20
db 0x74, 0xF7                           ; 45B9 jz 0x45b2
db 0x3C, 0x2C                           ; 45BB cmp al,0x2c
db 0x74, 0x18                           ; 45BD jz 0x45d7
db 0x3C, 0x0D                           ; 45BF cmp al,0xd
db 0x75, 0x04                           ; 45C1 jnz 0x45c7
db 0xCD, 0xE1                           ; 45C3 int 0xe1
db 0x74, 0x10                           ; 45C5 jz 0x45d7
db 0x8B, 0x1E, 0xE9, 0x04               ; 45C7 mov bx,[0x4e9]
db 0x8A, 0xC8                           ; 45CB mov cl,al
db 0xB0, 0x12                           ; 45CD mov al,0x12
db 0xE8, 0xDD, 0xFD                     ; 45CF call 0x43af
db 0xCD, 0xE2                           ; 45D2 int 0xe2
db 0xE9, 0xC3, 0xC1                     ; 45D4 jmp 0x79a
db 0x5B                                 ; 45D7 pop bx
db 0xC6, 0x07, 0x00                     ; 45D8 mov byte [bx],0x0
db 0xBB, 0xF6, 0x01                     ; 45DB mov bx,0x1f6
db 0x8A, 0xC2                           ; 45DE mov al,dl
db 0x2C, 0x20                           ; 45E0 sub al,0x20
db 0x74, 0x07                           ; 45E2 jz 0x45eb
db 0xB5, 0x00                           ; 45E4 mov ch,0x0
db 0xE8, 0x66, 0xE0                     ; 45E6 call 0x264f
db 0x5B                                 ; 45E9 pop bx
db 0xC3                                 ; 45EA ret
db 0xE8, 0x37, 0xD5                     ; 45EB call 0x1b25
db 0x9F                                 ; 45EE lahf
db 0x50                                 ; 45EF push ax
db 0xE8, 0x2A, 0xC9                     ; 45F0 call 0xf1d
db 0x58                                 ; 45F3 pop ax
db 0x9E                                 ; 45F4 sahf
db 0x9F                                 ; 45F5 lahf
db 0x50                                 ; 45F6 push ax
db 0x73, 0x03                           ; 45F7 jnc 0x45fc
db 0xE8, 0xC4, 0x23                     ; 45F9 call 0x69c0
db 0x58                                 ; 45FC pop ax
db 0x9E                                 ; 45FD sahf
db 0x72, 0x03                           ; 45FE jc 0x4603
db 0xE8, 0xC4, 0x23                     ; 4600 call 0x69c7
db 0x5B                                 ; 4603 pop bx
db 0xC3                                 ; 4604 ret
db 0x0A, 0xC0                           ; 4605 or al,al
db 0x74, 0xFB                           ; 4607 jz 0x4604
db 0x88, 0x07                           ; 4609 mov [bx],al
db 0x43                                 ; 460B inc bx
db 0xFE, 0xCD                           ; 460C dec ch
db 0x75, 0xF4                           ; 460E jnz 0x4604
db 0x59                                 ; 4610 pop cx
db 0xEB, 0xC5                           ; 4611 jmp short 0x45d8
db 0xE8, 0x46, 0x00                     ; 4613 call 0x465c
db 0xA2, 0x60, 0x00                     ; 4616 mov [0x60],al
db 0xFE, 0xC0                           ; 4619 inc al
db 0x74, 0x03                           ; 461B jz 0x4620
db 0xE9, 0x9E, 0xC1                     ; 461D jmp 0x7be
db 0x53                                 ; 4620 push bx
db 0x51                                 ; 4621 push cx
db 0xB2, 0x02                           ; 4622 mov dl,0x2
db 0xE8, 0xC6, 0xFA                     ; 4624 call 0x40ed
db 0x5B                                 ; 4627 pop bx
db 0xE8, 0x3B, 0xFC                     ; 4628 call 0x4266
db 0x32, 0xC0                           ; 462B xor al,al
db 0xA2, 0x60, 0x00                     ; 462D mov [0x60],al
db 0xE9, 0x1A, 0xFC                     ; 4630 jmp 0x424d
db 0xE8, 0x26, 0x00                     ; 4633 call 0x465c
db 0x0A, 0xC0                           ; 4636 or al,al
db 0x74, 0x07                           ; 4638 jz 0x4641
db 0xFE, 0xC0                           ; 463A inc al
db 0x75, 0x03                           ; 463C jnz 0x4641
db 0xE9, 0x7D, 0xC1                     ; 463E jmp 0x7be
db 0xFE, 0xC8                           ; 4641 dec al
db 0xA2, 0x60, 0x00                     ; 4643 mov [0x60],al
db 0x53                                 ; 4646 push bx
db 0x51                                 ; 4647 push cx
db 0x32, 0xC0                           ; 4648 xor al,al
db 0xB2, 0x01                           ; 464A mov dl,0x1
db 0xE8, 0x9E, 0xFA                     ; 464C call 0x40ed
db 0x5B                                 ; 464F pop bx
db 0xE8, 0x2E, 0xFC                     ; 4650 call 0x4281
db 0x32, 0xC0                           ; 4653 xor al,al
db 0xA2, 0x60, 0x00                     ; 4655 mov [0x60],al
db 0x5B                                 ; 4658 pop bx
db 0xE9, 0xCC, 0xFA                     ; 4659 jmp 0x4128
db 0xE8, 0xB4, 0xF8                     ; 465C call 0x3f13
db 0x52                                 ; 465F push dx
db 0x4B                                 ; 4660 dec bx
db 0xE8, 0xB9, 0xC8                     ; 4661 call 0xf1d
db 0x5A                                 ; 4664 pop dx
db 0x75, 0x03                           ; 4665 jnz 0x466a
db 0xB0, 0x01                           ; 4667 mov al,0x1
db 0xC3                                 ; 4669 ret
db 0x52                                 ; 466A push dx
db 0xE8, 0x86, 0xE7                     ; 466B call 0x2df4
db 0x2C, 0xE8                           ; 466E sub al,0xe8
db 0x2B, 0x00                           ; 4670 sub ax,[bx+si]
db 0x52                                 ; 4672 push dx
db 0x4B                                 ; 4673 dec bx
db 0xE8, 0xA6, 0xC8                     ; 4674 call 0xf1d
db 0x75, 0x05                           ; 4677 jnz 0x467e
db 0x59                                 ; 4679 pop cx
db 0x5A                                 ; 467A pop dx
db 0x32, 0xC0                           ; 467B xor al,al
db 0xC3                                 ; 467D ret
db 0xE8, 0x73, 0xE7                     ; 467E call 0x2df4
db 0x2C, 0xE8                           ; 4681 sub al,0xe8
db 0x18, 0x00                           ; 4683 sbb [bx+si],al
db 0x59                                 ; 4685 pop cx
db 0x87, 0xDA                           ; 4686 xchg bx,dx
db 0x03, 0xD9                           ; 4688 add bx,cx
db 0x89, 0x1E, 0x04, 0x07               ; 468A mov [0x704],bx
db 0x87, 0xDA                           ; 468E xchg bx,dx
db 0x4B                                 ; 4690 dec bx
db 0xE8, 0x89, 0xC8                     ; 4691 call 0xf1d
db 0x74, 0x03                           ; 4694 jz 0x4699
db 0xE9, 0x25, 0xC1                     ; 4696 jmp 0x7be
db 0x5A                                 ; 4699 pop dx
db 0xB0, 0xFF                           ; 469A mov al,0xff
db 0xC3                                 ; 469C ret
db 0xE8, 0x87, 0xD0                     ; 469D call 0x1727
db 0x53                                 ; 46A0 push bx
db 0xE8, 0x11, 0xDC                     ; 46A1 call 0x22b5
db 0x5A                                 ; 46A4 pop dx
db 0x87, 0xDA                           ; 46A5 xchg bx,dx
db 0xC3                                 ; 46A7 ret
db 0xB9, 0x0B, 0x0D                     ; 46A8 mov cx,0xd0b
db 0x8B, 0x1E, 0x30, 0x00               ; 46AB mov bx,[0x30]
db 0x87, 0xDA                           ; 46AF xchg bx,dx
db 0x8B, 0x1E, 0x58, 0x03               ; 46B1 mov bx,[0x358]
db 0x3B, 0xDA                           ; 46B5 cmp bx,dx
db 0x75, 0x01                           ; 46B7 jnz 0x46ba
db 0xC3                                 ; 46B9 ret
db 0xBB, 0x17, 0x62                     ; 46BA mov bx,0x6217
db 0x8A, 0xC3                           ; 46BD mov al,bl
db 0x02, 0xC1                           ; 46BF add al,cl
db 0x8A, 0xD8                           ; 46C1 mov bl,al
db 0x8A, 0xC7                           ; 46C3 mov al,bh
db 0x14, 0x00                           ; 46C5 adc al,0x0
db 0x8A, 0xF8                           ; 46C7 mov bh,al
db 0x8B, 0xF2                           ; 46C9 mov si,dx
db 0xAC                                 ; 46CB lodsb
db 0x2A, 0xC5                           ; 46CC sub al,ch
db 0x2E, 0x32, 0x07                     ; 46CE xor al,[cs:bx]
db 0x50                                 ; 46D1 push ax
db 0xBB, 0x76, 0x61                     ; 46D2 mov bx,0x6176
db 0x8A, 0xC3                           ; 46D5 mov al,bl
db 0x02, 0xC5                           ; 46D7 add al,ch
db 0x8A, 0xD8                           ; 46D9 mov bl,al
db 0x8A, 0xC7                           ; 46DB mov al,bh
db 0x14, 0x00                           ; 46DD adc al,0x0
db 0x8A, 0xF8                           ; 46DF mov bh,al
db 0x58                                 ; 46E1 pop ax
db 0x2E, 0x32, 0x07                     ; 46E2 xor al,[cs:bx]
db 0x02, 0xC1                           ; 46E5 add al,cl
db 0x8B, 0xFA                           ; 46E7 mov di,dx
db 0xAA                                 ; 46E9 stosb
db 0x42                                 ; 46EA inc dx
db 0xFE, 0xC9                           ; 46EB dec cl
db 0x75, 0x02                           ; 46ED jnz 0x46f1
db 0xB1, 0x0B                           ; 46EF mov cl,0xb
db 0xFE, 0xCD                           ; 46F1 dec ch
db 0x75, 0xBC                           ; 46F3 jnz 0x46b1
db 0xB5, 0x0D                           ; 46F5 mov ch,0xd
db 0xEB, 0xB8                           ; 46F7 jmp short 0x46b1
db 0xB9, 0x0B, 0x0D                     ; 46F9 mov cx,0xd0b
db 0x8B, 0x1E, 0x30, 0x00               ; 46FC mov bx,[0x30]
db 0x87, 0xDA                           ; 4700 xchg bx,dx
db 0x8B, 0x1E, 0x58, 0x03               ; 4702 mov bx,[0x358]
db 0x3B, 0xDA                           ; 4706 cmp bx,dx
db 0x74, 0xAF                           ; 4708 jz 0x46b9
db 0xBB, 0x76, 0x61                     ; 470A mov bx,0x6176
db 0x8A, 0xC3                           ; 470D mov al,bl
db 0x02, 0xC5                           ; 470F add al,ch
db 0x8A, 0xD8                           ; 4711 mov bl,al
db 0x8A, 0xC7                           ; 4713 mov al,bh
db 0x14, 0x00                           ; 4715 adc al,0x0
db 0x8A, 0xF8                           ; 4717 mov bh,al
db 0x8B, 0xF2                           ; 4719 mov si,dx
db 0xAC                                 ; 471B lodsb
db 0x2A, 0xC1                           ; 471C sub al,cl
db 0x2E, 0x32, 0x07                     ; 471E xor al,[cs:bx]
db 0x50                                 ; 4721 push ax
db 0xBB, 0x17, 0x62                     ; 4722 mov bx,0x6217
db 0x8A, 0xC3                           ; 4725 mov al,bl
db 0x02, 0xC1                           ; 4727 add al,cl
db 0x8A, 0xD8                           ; 4729 mov bl,al
db 0x8A, 0xC7                           ; 472B mov al,bh
db 0x14, 0x00                           ; 472D adc al,0x0
db 0x8A, 0xF8                           ; 472F mov bh,al
db 0x58                                 ; 4731 pop ax
db 0x2E, 0x32, 0x07                     ; 4732 xor al,[cs:bx]
db 0x02, 0xC5                           ; 4735 add al,ch
db 0x8B, 0xFA                           ; 4737 mov di,dx
db 0xAA                                 ; 4739 stosb
db 0x42                                 ; 473A inc dx
db 0xFE, 0xC9                           ; 473B dec cl
db 0x75, 0x02                           ; 473D jnz 0x4741
db 0xB1, 0x0B                           ; 473F mov cl,0xb
db 0xFE, 0xCD                           ; 4741 dec ch
db 0x75, 0xBD                           ; 4743 jnz 0x4702
db 0xB5, 0x0D                           ; 4745 mov ch,0xd
db 0xEB, 0xB9                           ; 4747 jmp short 0x4702
db 0x53                                 ; 4749 push bx
db 0x8B, 0x1E, 0x2E, 0x00               ; 474A mov bx,[0x2e]
db 0x8A, 0xC7                           ; 474E mov al,bh
db 0x22, 0xC3                           ; 4750 and al,bl
db 0x5B                                 ; 4752 pop bx
db 0xFE, 0xC0                           ; 4753 inc al
db 0x74, 0x01                           ; 4755 jz 0x4758
db 0xC3                                 ; 4757 ret
db 0x9F                                 ; 4758 lahf
db 0x50                                 ; 4759 push ax
db 0xA0, 0x64, 0x04                     ; 475A mov al,[0x464]
db 0x0A, 0xC0                           ; 475D or al,al
db 0x74, 0x03                           ; 475F jz 0x4764
db 0xE9, 0xF5, 0xC8                     ; 4761 jmp 0x1059
db 0x58                                 ; 4764 pop ax
db 0x9E                                 ; 4765 sahf
db 0xC3                                 ; 4766 ret
db 0x8A, 0x07                           ; 4767 mov al,[bx]
db 0x3C, 0x40                           ; 4769 cmp al,0x40
db 0x75, 0x03                           ; 476B jnz 0x4770
db 0xE8, 0xAD, 0xC7                     ; 476D call 0xf1d
db 0xB9, 0x00, 0x00                     ; 4770 mov cx,0x0
db 0x8A, 0xF5                           ; 4773 mov dh,ch
db 0x8A, 0xD1                           ; 4775 mov dl,cl
db 0x3C, 0xEA                           ; 4777 cmp al,0xea
db 0x74, 0x1F                           ; 4779 jz 0x479a
db 0x8A, 0x07                           ; 477B mov al,[bx]
db 0x3C, 0xCF                           ; 477D cmp al,0xcf
db 0x9C                                 ; 477F pushf
db 0x75, 0x03                           ; 4780 jnz 0x4785
db 0xE8, 0x98, 0xC7                     ; 4782 call 0xf1d
db 0xE8, 0x6C, 0xE6                     ; 4785 call 0x2df4
db 0x28, 0xE8                           ; 4788 sub al,ch
db 0x7E, 0xD7                           ; 478A jng 0x4763
db 0x52                                 ; 478C push dx
db 0xE8, 0x64, 0xE6                     ; 478D call 0x2df4
db 0x2C, 0xE8                           ; 4790 sub al,0xe8
db 0x76, 0xD7                           ; 4792 jna 0x476b
db 0xE8, 0x5D, 0xE6                     ; 4794 call 0x2df4
db 0x29, 0x59, 0x9D                     ; 4797 sub [bx+di-0x63],bx
db 0x53                                 ; 479A push bx
db 0x8B, 0x1E, 0x3D, 0x05               ; 479B mov bx,[0x53d]
db 0x74, 0x03                           ; 479F jz 0x47a4
db 0xBB, 0x00, 0x00                     ; 47A1 mov bx,0x0
db 0x9F                                 ; 47A4 lahf
db 0x03, 0xD9                           ; 47A5 add bx,cx
db 0xD1, 0xDE                           ; 47A7 rcr si,1
db 0x9E                                 ; 47A9 sahf
db 0xD1, 0xD6                           ; 47AA rcl si,1
db 0x89, 0x1E, 0x3D, 0x05               ; 47AC mov [0x53d],bx
db 0x89, 0x1E, 0x37, 0x05               ; 47B0 mov [0x537],bx
db 0x8B, 0xCB                           ; 47B4 mov cx,bx
db 0x8B, 0x1E, 0x3B, 0x05               ; 47B6 mov bx,[0x53b]
db 0x74, 0x03                           ; 47BA jz 0x47bf
db 0xBB, 0x00, 0x00                     ; 47BC mov bx,0x0
db 0x03, 0xDA                           ; 47BF add bx,dx
db 0x89, 0x1E, 0x3B, 0x05               ; 47C1 mov [0x53b],bx
db 0x89, 0x1E, 0x39, 0x05               ; 47C5 mov [0x539],bx
db 0x87, 0xDA                           ; 47C9 xchg bx,dx
db 0x5B                                 ; 47CB pop bx
db 0xC3                                 ; 47CC ret
db 0x32, 0xC0                           ; 47CD xor al,al
db 0xEB, 0x02                           ; 47CF jmp short 0x47d3
db 0xB0, 0x03                           ; 47D1 mov al,0x3
db 0x50                                 ; 47D3 push ax
db 0xE8, 0xA4, 0xFF                     ; 47D4 call 0x477b
db 0x58                                 ; 47D7 pop ax
db 0xE8, 0x2E, 0x00                     ; 47D8 call 0x4809
db 0x53                                 ; 47DB push bx
db 0xE8, 0x28, 0x03                     ; 47DC call 0x4b07
db 0x73, 0x06                           ; 47DF jnc 0x47e7
db 0xE8, 0x7F, 0x02                     ; 47E1 call 0x4a63
db 0xE8, 0x55, 0x02                     ; 47E4 call 0x4a3c
db 0x5B                                 ; 47E7 pop bx
db 0xC3                                 ; 47E8 ret
db 0xE8, 0x31, 0xC7                     ; 47E9 call 0xf1d
db 0xE8, 0x8C, 0xFF                     ; 47EC call 0x477b
db 0x53                                 ; 47EF push bx
db 0xE8, 0x14, 0x03                     ; 47F0 call 0x4b07
db 0xBB, 0xFF, 0xFF                     ; 47F3 mov bx,0xffff
db 0x73, 0x0A                           ; 47F6 jnc 0x4802
db 0xE8, 0x68, 0x02                     ; 47F8 call 0x4a63
db 0xE8, 0x1B, 0x02                     ; 47FB call 0x4a19
db 0x8A, 0xD8                           ; 47FE mov bl,al
db 0xB7, 0x00                           ; 4800 mov bh,0x0
db 0xE8, 0x07, 0x1D                     ; 4802 call 0x650c
db 0x5B                                 ; 4805 pop bx
db 0xC3                                 ; 4806 ret
db 0xB0, 0x03                           ; 4807 mov al,0x3
db 0x51                                 ; 4809 push cx
db 0x52                                 ; 480A push dx
db 0x8A, 0xD0                           ; 480B mov dl,al
db 0x4B                                 ; 480D dec bx
db 0xE8, 0x0C, 0xC7                     ; 480E call 0xf1d
db 0x74, 0x0B                           ; 4811 jz 0x481e
db 0xE8, 0xDE, 0xE5                     ; 4813 call 0x2df4
db 0x2C, 0x3C                           ; 4816 sub al,0x3c
db 0x2C, 0x74                           ; 4818 sub al,0x74
db 0x03, 0xE8                           ; 481A add bp,ax
db 0xFE                                 ; 481C db 0xFE
db 0xD6                                 ; 481D salc
db 0x8A, 0xC2                           ; 481E mov al,dl
db 0x53                                 ; 4820 push bx
db 0xE8, 0xC1, 0x02                     ; 4821 call 0x4ae5
db 0x73, 0x03                           ; 4824 jnc 0x4829
db 0xE9, 0x30, 0xC8                     ; 4826 jmp 0x1059
db 0x5B                                 ; 4829 pop bx
db 0x5A                                 ; 482A pop dx
db 0x59                                 ; 482B pop cx
db 0xE9, 0xEF, 0xC6                     ; 482C jmp 0xf1e
db 0x8B, 0x1E, 0x37, 0x05               ; 482F mov bx,[0x537]
db 0x8A, 0xC3                           ; 4833 mov al,bl
db 0x2A, 0xC1                           ; 4835 sub al,cl
db 0x8A, 0xD8                           ; 4837 mov bl,al
db 0x8A, 0xC7                           ; 4839 mov al,bh
db 0x1A, 0xC5                           ; 483B sbb al,ch
db 0x8A, 0xF8                           ; 483D mov bh,al
db 0x73, 0xC5                           ; 483F jnc 0x4806
db 0x32, 0xC0                           ; 4841 xor al,al
db 0x2A, 0xC3                           ; 4843 sub al,bl
db 0x8A, 0xD8                           ; 4845 mov bl,al
db 0x1A, 0xC7                           ; 4847 sbb al,bh
db 0x2A, 0xC3                           ; 4849 sub al,bl
db 0x8A, 0xF8                           ; 484B mov bh,al
db 0xF9                                 ; 484D stc
db 0xC3                                 ; 484E ret
db 0x8B, 0x1E, 0x39, 0x05               ; 484F mov bx,[0x539]
db 0x8A, 0xC3                           ; 4853 mov al,bl
db 0x2A, 0xC2                           ; 4855 sub al,dl
db 0x8A, 0xD8                           ; 4857 mov bl,al
db 0x8A, 0xC7                           ; 4859 mov al,bh
db 0x1A, 0xC6                           ; 485B sbb al,dh
db 0x8A, 0xF8                           ; 485D mov bh,al
db 0xEB, 0xDE                           ; 485F jmp short 0x483f
db 0x53                                 ; 4861 push bx
db 0x8B, 0x1E, 0x39, 0x05               ; 4862 mov bx,[0x539]
db 0x87, 0xDA                           ; 4866 xchg bx,dx
db 0x89, 0x1E, 0x39, 0x05               ; 4868 mov [0x539],bx
db 0x5B                                 ; 486C pop bx
db 0xC3                                 ; 486D ret
db 0xE8, 0xF0, 0xFF                     ; 486E call 0x4861
db 0x53                                 ; 4871 push bx
db 0x51                                 ; 4872 push cx
db 0x8B, 0x1E, 0x37, 0x05               ; 4873 mov bx,[0x537]
db 0x5E                                 ; 4877 pop si
db 0x87, 0xDE                           ; 4878 xchg bx,si
db 0x56                                 ; 487A push si
db 0x89, 0x1E, 0x37, 0x05               ; 487B mov [0x537],bx
db 0x59                                 ; 487F pop cx
db 0x5B                                 ; 4880 pop bx
db 0xC3                                 ; 4881 ret
db 0xE8, 0xE2, 0xFE                     ; 4882 call 0x4767
db 0x51                                 ; 4885 push cx
db 0x52                                 ; 4886 push dx
db 0xE8, 0x6A, 0xE5                     ; 4887 call 0x2df4
db 0xEA, 0xE8, 0xED, 0xFE, 0xE8         ; 488A jmp 0xe8fe:0xede8
db 0x76, 0xFF                           ; 488F jna 0x4890
db 0x5A                                 ; 4891 pop dx
db 0x59                                 ; 4892 pop cx
db 0x74, 0x53                           ; 4893 jz 0x48e8
db 0xE8, 0x5C, 0xE5                     ; 4895 call 0x2df4
db 0x2C, 0xE8                           ; 4898 sub al,0xe8
db 0x58                                 ; 489A pop ax
db 0xE5, 0x42                           ; 489B in ax,0x42
db 0x75, 0x03                           ; 489D jnz 0x48a2
db 0xE9, 0x60, 0x00                     ; 489F jmp 0x4902
db 0xE8, 0x4F, 0xE5                     ; 48A2 call 0x2df4
db 0x46                                 ; 48A5 inc si
db 0x53                                 ; 48A6 push bx
db 0xE8, 0x5D, 0x02                     ; 48A7 call 0x4b07
db 0xE8, 0xC1, 0xFF                     ; 48AA call 0x486e
db 0xE8, 0x57, 0x02                     ; 48AD call 0x4b07
db 0xE8, 0x9C, 0xFF                     ; 48B0 call 0x484f
db 0x73, 0x03                           ; 48B3 jnc 0x48b8
db 0xE8, 0xA9, 0xFF                     ; 48B5 call 0x4861
db 0x43                                 ; 48B8 inc bx
db 0x53                                 ; 48B9 push bx
db 0xE8, 0x72, 0xFF                     ; 48BA call 0x482f
db 0x73, 0x03                           ; 48BD jnc 0x48c2
db 0xE8, 0xAF, 0xFF                     ; 48BF call 0x4871
db 0x43                                 ; 48C2 inc bx
db 0x53                                 ; 48C3 push bx
db 0xE8, 0x9C, 0x01                     ; 48C4 call 0x4a63
db 0x5A                                 ; 48C7 pop dx
db 0x59                                 ; 48C8 pop cx
db 0x52                                 ; 48C9 push dx
db 0x51                                 ; 48CA push cx
db 0xE8, 0xDB, 0x00                     ; 48CB call 0x49a9
db 0x50                                 ; 48CE push ax
db 0x53                                 ; 48CF push bx
db 0x87, 0xDA                           ; 48D0 xchg bx,dx
db 0xE8, 0x69, 0x02                     ; 48D2 call 0x4b3e
db 0x5B                                 ; 48D5 pop bx
db 0x58                                 ; 48D6 pop ax
db 0xE8, 0xD7, 0x00                     ; 48D7 call 0x49b1
db 0xE8, 0xF8, 0x00                     ; 48DA call 0x49d5
db 0x59                                 ; 48DD pop cx
db 0x5A                                 ; 48DE pop dx
db 0x49                                 ; 48DF dec cx
db 0x8A, 0xC5                           ; 48E0 mov al,ch
db 0x0A, 0xC1                           ; 48E2 or al,cl
db 0x75, 0xE3                           ; 48E4 jnz 0x48c9
db 0x5B                                 ; 48E6 pop bx
db 0xC3                                 ; 48E7 ret
db 0x51                                 ; 48E8 push cx
db 0x52                                 ; 48E9 push dx
db 0x53                                 ; 48EA push bx
db 0xE8, 0x45, 0x00                     ; 48EB call 0x4933
db 0x8B, 0x1E, 0x3D, 0x05               ; 48EE mov bx,[0x53d]
db 0x89, 0x1E, 0x37, 0x05               ; 48F2 mov [0x537],bx
db 0x8B, 0x1E, 0x3B, 0x05               ; 48F6 mov bx,[0x53b]
db 0x89, 0x1E, 0x39, 0x05               ; 48FA mov [0x539],bx
db 0x5B                                 ; 48FE pop bx
db 0x5A                                 ; 48FF pop dx
db 0x59                                 ; 4900 pop cx
db 0xC3                                 ; 4901 ret
db 0x53                                 ; 4902 push bx
db 0x8B, 0x1E, 0x39, 0x05               ; 4903 mov bx,[0x539]
db 0x53                                 ; 4907 push bx
db 0x52                                 ; 4908 push dx
db 0x87, 0xDA                           ; 4909 xchg bx,dx
db 0xE8, 0xDA, 0xFF                     ; 490B call 0x48e8
db 0x5B                                 ; 490E pop bx
db 0x89, 0x1E, 0x39, 0x05               ; 490F mov [0x539],bx
db 0x87, 0xDA                           ; 4913 xchg bx,dx
db 0xE8, 0xD0, 0xFF                     ; 4915 call 0x48e8
db 0x5B                                 ; 4918 pop bx
db 0x89, 0x1E, 0x39, 0x05               ; 4919 mov [0x539],bx
db 0x8B, 0x1E, 0x37, 0x05               ; 491D mov bx,[0x537]
db 0x51                                 ; 4921 push cx
db 0x8B, 0xCB                           ; 4922 mov cx,bx
db 0xE8, 0xC1, 0xFF                     ; 4924 call 0x48e8
db 0x5B                                 ; 4927 pop bx
db 0x89, 0x1E, 0x37, 0x05               ; 4928 mov [0x537],bx
db 0x8B, 0xCB                           ; 492C mov cx,bx
db 0xE8, 0xB7, 0xFF                     ; 492E call 0x48e8
db 0x5B                                 ; 4931 pop bx
db 0xC3                                 ; 4932 ret
db 0xCD, 0xB8                           ; 4933 int 0xb8
db 0xE8, 0xCF, 0x01                     ; 4935 call 0x4b07
db 0xE8, 0x33, 0xFF                     ; 4938 call 0x486e
db 0xE8, 0xC9, 0x01                     ; 493B call 0x4b07
db 0xE8, 0x0E, 0xFF                     ; 493E call 0x484f
db 0x73, 0x03                           ; 4941 jnc 0x4946
db 0xE8, 0x28, 0xFF                     ; 4943 call 0x486e
db 0x52                                 ; 4946 push dx
db 0x53                                 ; 4947 push bx
db 0xE8, 0xE4, 0xFE                     ; 4948 call 0x482f
db 0x87, 0xDA                           ; 494B xchg bx,dx
db 0xBB, 0xF1, 0x49                     ; 494D mov bx,0x49f1
db 0x73, 0x03                           ; 4950 jnc 0x4955
db 0xBB, 0x05, 0x4A                     ; 4952 mov bx,0x4a05
db 0x5E                                 ; 4955 pop si
db 0x87, 0xDE                           ; 4956 xchg bx,si
db 0x56                                 ; 4958 push si
db 0x3B, 0xDA                           ; 4959 cmp bx,dx
db 0x73, 0x14                           ; 495B jnc 0x4971
db 0x89, 0x1E, 0xFD, 0x06               ; 495D mov [0x6fd],bx
db 0x5B                                 ; 4961 pop bx
db 0x89, 0x1E, 0xF7, 0x06               ; 4962 mov [0x6f7],bx
db 0xBB, 0xD5, 0x49                     ; 4966 mov bx,0x49d5
db 0x89, 0x1E, 0xF9, 0x06               ; 4969 mov [0x6f9],bx
db 0x87, 0xDA                           ; 496D xchg bx,dx
db 0xEB, 0x16                           ; 496F jmp short 0x4987
db 0x5E                                 ; 4971 pop si
db 0x87, 0xDE                           ; 4972 xchg bx,si
db 0x56                                 ; 4974 push si
db 0x89, 0x1E, 0xF9, 0x06               ; 4975 mov [0x6f9],bx
db 0xBB, 0xD5, 0x49                     ; 4979 mov bx,0x49d5
db 0x89, 0x1E, 0xF7, 0x06               ; 497C mov [0x6f7],bx
db 0x87, 0xDA                           ; 4980 xchg bx,dx
db 0x89, 0x1E, 0xFD, 0x06               ; 4982 mov [0x6fd],bx
db 0x5B                                 ; 4986 pop bx
db 0x5A                                 ; 4987 pop dx
db 0x53                                 ; 4988 push bx
db 0x89, 0x1E, 0xFB, 0x06               ; 4989 mov [0x6fb],bx
db 0xE8, 0xD3, 0x00                     ; 498D call 0x4a63
db 0x5A                                 ; 4990 pop dx
db 0x52                                 ; 4991 push dx
db 0xE8, 0x05, 0x00                     ; 4992 call 0x499a
db 0x59                                 ; 4995 pop cx
db 0x41                                 ; 4996 inc cx
db 0xE9, 0x20, 0x02                     ; 4997 jmp 0x4bba
db 0x8A, 0xC6                           ; 499A mov al,dh
db 0x0A, 0xC0                           ; 499C or al,al
db 0xD0, 0xD8                           ; 499E rcr al,1
db 0x8A, 0xF0                           ; 49A0 mov dh,al
db 0x8A, 0xC2                           ; 49A2 mov al,dl
db 0xD0, 0xD8                           ; 49A4 rcr al,1
db 0x8A, 0xD0                           ; 49A6 mov dl,al
db 0xC3                                 ; 49A8 ret
db 0x8B, 0x1E, 0xF3, 0x06               ; 49A9 mov bx,[0x6f3]
db 0xA0, 0xF5, 0x06                     ; 49AD mov al,[0x6f5]
db 0xC3                                 ; 49B0 ret
db 0x89, 0x1E, 0xF3, 0x06               ; 49B1 mov [0x6f3],bx
db 0xA2, 0xF5, 0x06                     ; 49B5 mov [0x6f5],al
db 0xC3                                 ; 49B8 ret
db 0x8B, 0x1E, 0xF3, 0x06               ; 49B9 mov bx,[0x6f3]
db 0x81, 0xFB, 0x00, 0x20               ; 49BD cmp bx,0x2000
db 0x72, 0x09                           ; 49C1 jc 0x49cc
db 0x81, 0xEB, 0x00, 0x20               ; 49C3 sub bx,0x2000
db 0x89, 0x1E, 0xF3, 0x06               ; 49C7 mov [0x6f3],bx
db 0xC3                                 ; 49CB ret
db 0x81, 0xC3, 0x50, 0x20               ; 49CC add bx,0x2050
db 0x89, 0x1E, 0xF3, 0x06               ; 49D0 mov [0x6f3],bx
db 0xC3                                 ; 49D4 ret
db 0x8B, 0x1E, 0xF3, 0x06               ; 49D5 mov bx,[0x6f3]
db 0x81, 0xFB, 0x00, 0x20               ; 49D9 cmp bx,0x2000
db 0x72, 0x09                           ; 49DD jc 0x49e8
db 0x81, 0xEB, 0xB0, 0x1F               ; 49DF sub bx,0x1fb0
db 0x89, 0x1E, 0xF3, 0x06               ; 49E3 mov [0x6f3],bx
db 0xC3                                 ; 49E7 ret
db 0x81, 0xC3, 0x00, 0x20               ; 49E8 add bx,0x2000
db 0x89, 0x1E, 0xF3, 0x06               ; 49EC mov [0x6f3],bx
db 0xC3                                 ; 49F0 ret
db 0x8A, 0xC1                           ; 49F1 mov al,cl
db 0x8A, 0x0E, 0x55, 0x00               ; 49F3 mov cl,[0x55]
db 0xD2, 0x0E, 0xF5, 0x06               ; 49F7 ror byte [0x6f5],cl
db 0x8A, 0xC8                           ; 49FB mov cl,al
db 0x72, 0x01                           ; 49FD jc 0x4a00
db 0xC3                                 ; 49FF ret
db 0xFF, 0x06, 0xF3, 0x06               ; 4A00 inc word [0x6f3]
db 0xC3                                 ; 4A04 ret
db 0x8A, 0xC1                           ; 4A05 mov al,cl
db 0x8A, 0x0E, 0x55, 0x00               ; 4A07 mov cl,[0x55]
db 0xD2, 0x06, 0xF5, 0x06               ; 4A0B rol byte [0x6f5],cl
db 0x8A, 0xC8                           ; 4A0F mov cl,al
db 0x72, 0x01                           ; 4A11 jc 0x4a14
db 0xC3                                 ; 4A13 ret
db 0xFF, 0x0E, 0xF3, 0x06               ; 4A14 dec word [0x6f3]
db 0xC3                                 ; 4A18 ret
db 0x8C, 0xC6                           ; 4A19 mov si,es
db 0xBF, 0x00, 0xB8                     ; 4A1B mov di,0xb800
db 0x8E, 0xC7                           ; 4A1E mov es,di
db 0x8B, 0x1E, 0xF3, 0x06               ; 4A20 mov bx,[0x6f3]
db 0x26, 0x8A, 0x07                     ; 4A24 mov al,[es:bx]
db 0x8A, 0x16, 0xF5, 0x06               ; 4A27 mov dl,[0x6f5]
db 0x22, 0xC2                           ; 4A2B and al,dl
db 0x8A, 0x0E, 0x55, 0x00               ; 4A2D mov cl,[0x55]
db 0xD2, 0xEA                           ; 4A31 shr dl,cl
db 0x72, 0x04                           ; 4A33 jc 0x4a39
db 0xD2, 0xE8                           ; 4A35 shr al,cl
db 0xEB, 0xF8                           ; 4A37 jmp short 0x4a31
db 0x8E, 0xC6                           ; 4A39 mov es,si
db 0xC3                                 ; 4A3B ret
db 0x8C, 0xC6                           ; 4A3C mov si,es
db 0xBF, 0x00, 0xB8                     ; 4A3E mov di,0xb800
db 0x8E, 0xC7                           ; 4A41 mov es,di
db 0x8B, 0x1E, 0xF3, 0x06               ; 4A43 mov bx,[0x6f3]
db 0x8B, 0xE9                           ; 4A47 mov bp,cx
db 0xA0, 0xF5, 0x06                     ; 4A49 mov al,[0x6f5]
db 0xF6, 0xD0                           ; 4A4C not al
db 0x26, 0x22, 0x07                     ; 4A4E and al,[es:bx]
db 0x8A, 0x0E, 0xF6, 0x06               ; 4A51 mov cl,[0x6f6]
db 0x22, 0x0E, 0xF5, 0x06               ; 4A55 and cl,[0x6f5]
db 0x0A, 0xC1                           ; 4A59 or al,cl
db 0x26, 0x88, 0x07                     ; 4A5B mov [es:bx],al
db 0x8B, 0xCD                           ; 4A5E mov cx,bp
db 0x8E, 0xC6                           ; 4A60 mov es,si
db 0xC3                                 ; 4A62 ret
db 0x8B, 0xE9                           ; 4A63 mov bp,cx
db 0xD1, 0xEA                           ; 4A65 shr dx,1
db 0x9F                                 ; 4A67 lahf
db 0x8B, 0xDA                           ; 4A68 mov bx,dx
db 0xB1, 0x02                           ; 4A6A mov cl,0x2
db 0xD3, 0xE2                           ; 4A6C shl dx,cl
db 0x03, 0xD3                           ; 4A6E add dx,bx
db 0xB1, 0x04                           ; 4A70 mov cl,0x4
db 0xD3, 0xE2                           ; 4A72 shl dx,cl
db 0x9E                                 ; 4A74 sahf
db 0x73, 0x04                           ; 4A75 jnc 0x4a7b
db 0x81, 0xC2, 0x00, 0x20               ; 4A77 add dx,0x2000
db 0x89, 0x16, 0xF3, 0x06               ; 4A7B mov [0x6f3],dx
db 0x8B, 0xD5                           ; 4A7F mov dx,bp
db 0x8A, 0xCA                           ; 4A81 mov cl,dl
db 0xF6, 0x06, 0x55, 0x00, 0x01         ; 4A83 test byte [0x55],0x1
db 0x74, 0x14                           ; 4A88 jz 0x4a9e
db 0xB0, 0x07                           ; 4A8A mov al,0x7
db 0x22, 0xC8                           ; 4A8C and cl,al
db 0xB0, 0x80                           ; 4A8E mov al,0x80
db 0xD2, 0xE8                           ; 4A90 shr al,cl
db 0xA2, 0xF5, 0x06                     ; 4A92 mov [0x6f5],al
db 0xB1, 0x03                           ; 4A95 mov cl,0x3
db 0xD3, 0xEA                           ; 4A97 shr dx,cl
db 0x01, 0x16, 0xF3, 0x06               ; 4A99 add [0x6f3],dx
db 0xC3                                 ; 4A9D ret
db 0xB0, 0x03                           ; 4A9E mov al,0x3
db 0x22, 0xC8                           ; 4AA0 and cl,al
db 0x02, 0xC9                           ; 4AA2 add cl,cl
db 0xB0, 0xC0                           ; 4AA4 mov al,0xc0
db 0xD2, 0xE8                           ; 4AA6 shr al,cl
db 0xA2, 0xF5, 0x06                     ; 4AA8 mov [0x6f5],al
db 0xB1, 0x02                           ; 4AAB mov cl,0x2
db 0xD3, 0xEA                           ; 4AAD shr dx,cl
db 0x01, 0x16, 0xF3, 0x06               ; 4AAF add [0x6f3],dx
db 0xC3                                 ; 4AB3 ret
db 0xA0, 0x48, 0x00                     ; 4AB4 mov al,[0x48]
db 0xC7, 0x06, 0x3B, 0x05, 0x64, 0x00   ; 4AB7 mov word [0x53b],0x64
db 0x3C, 0x06                           ; 4ABD cmp al,0x6
db 0x74, 0x12                           ; 4ABF jz 0x4ad3
db 0x73, 0x1C                           ; 4AC1 jnc 0x4adf
db 0x3C, 0x04                           ; 4AC3 cmp al,0x4
db 0x72, 0x18                           ; 4AC5 jc 0x4adf
db 0xC6, 0x06, 0x55, 0x00, 0x02         ; 4AC7 mov byte [0x55],0x2
db 0xC7, 0x06, 0x3D, 0x05, 0xA0, 0x00   ; 4ACC mov word [0x53d],0xa0
db 0xC3                                 ; 4AD2 ret
db 0xC6, 0x06, 0x55, 0x00, 0x01         ; 4AD3 mov byte [0x55],0x1
db 0xC7, 0x06, 0x3D, 0x05, 0x40, 0x01   ; 4AD8 mov word [0x53d],0x140
db 0xC3                                 ; 4ADE ret
db 0xC6, 0x06, 0x55, 0x00, 0x00         ; 4ADF mov byte [0x55],0x0
db 0xC3                                 ; 4AE4 ret
db 0x3C, 0x04                           ; 4AE5 cmp al,0x4
db 0x73, 0x0F                           ; 4AE7 jnc 0x4af8
db 0xF6, 0x06, 0x55, 0x00, 0x01         ; 4AE9 test byte [0x55],0x1
db 0x74, 0x0C                           ; 4AEE jz 0x4afc
db 0x24, 0x01                           ; 4AF0 and al,0x1
db 0xF6, 0xD8                           ; 4AF2 neg al
db 0xA2, 0xF6, 0x06                     ; 4AF4 mov [0x6f6],al
db 0xF8                                 ; 4AF7 clc
db 0xC3                                 ; 4AF8 ret
db 0xE9, 0x5D, 0xC5                     ; 4AF9 jmp 0x1059
db 0x24, 0x03                           ; 4AFC and al,0x3
db 0xB1, 0x55                           ; 4AFE mov cl,0x55
db 0xF6, 0xE1                           ; 4B00 mul cl
db 0xA2, 0xF6, 0x06                     ; 4B02 mov [0x6f6],al
db 0xF8                                 ; 4B05 clc
db 0xC3                                 ; 4B06 ret
db 0xA0, 0x55, 0x00                     ; 4B07 mov al,[0x55]
db 0x0A, 0xC0                           ; 4B0A or al,al
db 0x74, 0xEB                           ; 4B0C jz 0x4af9
db 0x0A, 0xED                           ; 4B0E or ch,ch
db 0x78, 0x27                           ; 4B10 js 0x4b39
db 0xBB, 0x80, 0x02                     ; 4B12 mov bx,0x280
db 0x84, 0x06, 0x01, 0x00               ; 4B15 test [0x1],al
db 0x74, 0x03                           ; 4B19 jz 0x4b1e
db 0xBB, 0x40, 0x01                     ; 4B1B mov bx,0x140
db 0x3B, 0xCB                           ; 4B1E cmp cx,bx
db 0x9F                                 ; 4B20 lahf
db 0x72, 0x03                           ; 4B21 jc 0x4b26
db 0x4B                                 ; 4B23 dec bx
db 0x8B, 0xCB                           ; 4B24 mov cx,bx
db 0x0A, 0xF6                           ; 4B26 or dh,dh
db 0x78, 0x0C                           ; 4B28 js 0x4b36
db 0x81, 0xFA, 0xC8, 0x00               ; 4B2A cmp dx,0xc8
db 0x72, 0x04                           ; 4B2E jc 0x4b34
db 0xBA, 0xC7, 0x00                     ; 4B30 mov dx,0xc7
db 0xC3                                 ; 4B33 ret
db 0x9E                                 ; 4B34 sahf
db 0xC3                                 ; 4B35 ret
db 0x33, 0xD2                           ; 4B36 xor dx,dx
db 0xC3                                 ; 4B38 ret
db 0x33, 0xC9                           ; 4B39 xor cx,cx
db 0x9F                                 ; 4B3B lahf
db 0xEB, 0xE8                           ; 4B3C jmp short 0x4b26
db 0x8C, 0xC6                           ; 4B3E mov si,es
db 0xBF, 0x00, 0xB8                     ; 4B40 mov di,0xb800
db 0x8E, 0xC7                           ; 4B43 mov es,di
db 0x8B, 0xD3                           ; 4B45 mov dx,bx
db 0x0B, 0xD2                           ; 4B47 or dx,dx
db 0x74, 0x6C                           ; 4B49 jz 0x4bb7
db 0x8B, 0x1E, 0xF3, 0x06               ; 4B4B mov bx,[0x6f3]
db 0x26, 0x8A, 0x2F                     ; 4B4F mov ch,[es:bx]
db 0xA0, 0xF5, 0x06                     ; 4B52 mov al,[0x6f5]
db 0x8A, 0xE0                           ; 4B55 mov ah,al
db 0xF6, 0xD0                           ; 4B57 not al
db 0x8A, 0x0E, 0x55, 0x00               ; 4B59 mov cl,[0x55]
db 0x8A, 0x1E, 0xF6, 0x06               ; 4B5D mov bl,[0x6f6]
db 0x22, 0xE8                           ; 4B61 and ch,al
db 0x8A, 0xFC                           ; 4B63 mov bh,ah
db 0x22, 0xFB                           ; 4B65 and bh,bl
db 0x0A, 0xEF                           ; 4B67 or ch,bh
db 0x4A                                 ; 4B69 dec dx
db 0x74, 0x40                           ; 4B6A jz 0x4bac
db 0xD2, 0xC8                           ; 4B6C ror al,cl
db 0xD2, 0xCC                           ; 4B6E ror ah,cl
db 0x73, 0xEF                           ; 4B70 jnc 0x4b61
db 0x8B, 0x1E, 0xF3, 0x06               ; 4B72 mov bx,[0x6f3]
db 0x26, 0x88, 0x2F                     ; 4B76 mov [es:bx],ch
db 0xFF, 0x06, 0xF3, 0x06               ; 4B79 inc word [0x6f3]
db 0x88, 0x26, 0xF5, 0x06               ; 4B7D mov [0x6f5],ah
db 0x8B, 0xCA                           ; 4B81 mov cx,dx
db 0xD1, 0xE9                           ; 4B83 shr cx,1
db 0xD1, 0xE9                           ; 4B85 shr cx,1
db 0xF6, 0x06, 0x55, 0x00, 0x01         ; 4B87 test byte [0x55],0x1
db 0x75, 0x06                           ; 4B8C jnz 0x4b94
db 0x81, 0xE2, 0x03, 0x00               ; 4B8E and dx,0x3
db 0xEB, 0x06                           ; 4B92 jmp short 0x4b9a
db 0x81, 0xE2, 0x07, 0x00               ; 4B94 and dx,0x7
db 0xD1, 0xE9                           ; 4B98 shr cx,1
db 0xE3, 0xAB                           ; 4B9A jcxz 0x4b47
db 0xFC                                 ; 4B9C cld
db 0xA0, 0xF6, 0x06                     ; 4B9D mov al,[0x6f6]
db 0x8B, 0x3E, 0xF3, 0x06               ; 4BA0 mov di,[0x6f3]
db 0xF3, 0xAA                           ; 4BA4 rep stosb
db 0x89, 0x3E, 0xF3, 0x06               ; 4BA6 mov [0x6f3],di
db 0xEB, 0x9B                           ; 4BAA jmp short 0x4b47
db 0x8B, 0x1E, 0xF3, 0x06               ; 4BAC mov bx,[0x6f3]
db 0x26, 0x88, 0x2F                     ; 4BB0 mov [es:bx],ch
db 0x88, 0x26, 0xF5, 0x06               ; 4BB3 mov [0x6f5],ah
db 0x8E, 0xC6                           ; 4BB7 mov es,si
db 0xC3                                 ; 4BB9 ret
db 0xE8, 0x7F, 0xFE                     ; 4BBA call 0x4a3c
db 0x03, 0x16, 0xFD, 0x06               ; 4BBD add dx,[0x6fd]
db 0x3B, 0x16, 0xFB, 0x06               ; 4BC1 cmp dx,[0x6fb]
db 0x72, 0x09                           ; 4BC5 jc 0x4bd0
db 0x2B, 0x16, 0xFB, 0x06               ; 4BC7 sub dx,[0x6fb]
db 0x3E, 0xFF, 0x16, 0xF9, 0x06         ; 4BCB call near [ds:0x6f9]
db 0x3E, 0xFF, 0x16, 0xF7, 0x06         ; 4BD0 call near [ds:0x6f7]
db 0xE2, 0xE3                           ; 4BD5 loop 0x4bba
db 0xC3                                 ; 4BD7 ret
db 0x53                                 ; 4BD8 push bx
db 0xE8, 0xA3, 0xCF                     ; 4BD9 call 0x1b7f
db 0x5B                                 ; 4BDC pop bx
db 0xC3                                 ; 4BDD ret
db 0x53                                 ; 4BDE push bx
db 0xE8, 0x27, 0x19                     ; 4BDF call 0x6509
db 0x5B                                 ; 4BE2 pop bx
db 0xC3                                 ; 4BE3 ret
db 0xF6, 0x80, 0x3E, 0x71, 0x00         ; 4BE4 test byte [bx+si+0x713e],0x0
db 0x00, 0x74, 0x03                     ; 4BE9 add [si+0x3],dh
db 0xE9, 0xF9, 0x04                     ; 4BEC jmp 0x50e8
db 0xC3                                 ; 4BEF ret
db 0xA0, 0x29, 0x00                     ; 4BF0 mov al,[0x29]
db 0x8A, 0xD0                           ; 4BF3 mov dl,al
db 0xE8, 0xFF, 0xD2                     ; 4BF5 call 0x1ef7
db 0xE9, 0xED, 0x04                     ; 4BF8 jmp 0x50e8
db 0x00, 0x00                           ; 4BFB add [bx+si],al
db 0x00

; startup after setting up stack
L_4BFE:
  mov     ah, 0x0f
  int     0x10
  mov     [0x0048], al                  ; Store old video mode

  mov     ah, 40
  cmp     al, 2
  jb      L_4C18

  mov     ah, 80
  cmp     al, 7
  jne     L_4C18

  mov     cx, 0x0b0c
  mov     [0x68], cx

L_4C18:
  mov     [0x29], ah                    ; Number of columns

  cli
  mov     bx, ds
  mov     [0x350], bx

  push    ds
  mov     dx, 0
  mov     ds, dx
  mov     [0x510], bx
  mov     bx, L_4D34
  mov     [0x6c], bx                    ; int 1b address
  mov     bx, L_5744
  mov     [0x70], bx                    ; int 1c address
  mov     [0x6e], cs
  mov     [0x72], cs
  pop     ds

  call    L_4C79                        ; copy strings to 60:653

  ; set int 0x86-0xff addresses to default (empty) handler
  mov     bx, 0x86 * 4
  mov     cx, 0
  mov     es, cx
  mov     cx, 0x100 - 0x86

L_4C52:
;  mov    [es:bx + 2], cs
;  mov    [es:bx], word L_4C94
db 0x26, 0x8C, 0x8F, 0x02, 0x00         ; 4C52 mov [es:bx+0x2],cs
db 0x26, 0xC7, 0x07, 0x94, 0x4C         ; 4C57 mov word [es:bx],0x4c94

  add     bx, byte 4
  loopne  L_4C52


  mov     bx, ds
  mov     es, bx

  call    L_2DB0

  sti

  mov     ah, 1
  int     0x17                          ; initialize parallel port

  call    L_52E7

  mov     bx, L_4C9B                    ; startup message
  call    L_7B55

  jmp     L_7EDC


L_4C79:
  mov     si, L_4CED
  mov     bx, 0x0653
  mov     cx, 10                        ; copy 10 strings

L_4C82:
  push    bx

L_4C83:
  cld
  cs      lodsb
;  mov     [bx], al
  db 0x88, 0x07
  inc     bx
  or      al, al
  jnz     L_4C83

  pop     bx

  add     bx, byte 16
  loopnz  L_4C82

  ret

; default interrupt handler
L_4C94:
  iret


L_4C95:
  ds
  jmp     far [0x700]
  retf


L_4C9B:
  db      "The IBM Personal Computer Basic", 0xff, 0x0d
  db      "Version C1.10 Copyright IBM Corp 1981", 0xff, 0x0d, 0x00

  db "25-Apr-81"


; default function keys
L_4CED:
  db "LIST ", 0
  db "RUN", 0x0d, 0
  db "LOAD", 0x22, 0
  db "SAVE", 0x22, 0
  db "CONT", 0x0d, 0
  db 0x2c, 0x22, "LPT1:", 0x22, 0x0d, 0
  db "TRON", 0x0d, 0
  db "TROFF", 0x0d, 0
  db "KEY ", 0
  db "SCREEN 0,0,0", 0x0d, 0

L_4D34:
  pushf
db 0x50                                 ; 4D35 push ax
db 0x1E                                 ; 4D36 push ds
db 0x52                                 ; 4D37 push dx
db 0xBA, 0x00, 0x00                     ; 4D38 mov dx,0x0
db 0x8E, 0xDA                           ; 4D3B mov ds,dx
db 0x8E, 0x1E, 0x10, 0x05               ; 4D3D mov ds,[0x510]
db 0xE8, 0x3A, 0x0A                     ; 4D41 call 0x577e
db 0x88, 0x16, 0x6A, 0x00               ; 4D44 mov [0x6a],dl
db 0xFE, 0xCA                           ; 4D48 dec dl
db 0x88, 0x16, 0x5E, 0x00               ; 4D4A mov [0x5e],dl
db 0x5A                                 ; 4D4E pop dx
db 0x1F                                 ; 4D4F pop ds
db 0x58                                 ; 4D50 pop ax
db 0x9D                                 ; 4D51 popf
db 0xCF                                 ; 4D52 iret
db 0x56                                 ; 4D53 push si
db 0xA0, 0x5E, 0x00                     ; 4D54 mov al,[0x5e]
db 0x0A, 0xC0                           ; 4D57 or al,al
db 0x75, 0x11                           ; 4D59 jnz 0x4d6c
db 0xA0, 0x6A, 0x00                     ; 4D5B mov al,[0x6a]
db 0x0A, 0xC0                           ; 4D5E or al,al
db 0x75, 0x0A                           ; 4D60 jnz 0x4d6c
db 0xB4, 0x01                           ; 4D62 mov ah,0x1
db 0xCD, 0x16                           ; 4D64 int 0x16
db 0xB0, 0x00                           ; 4D66 mov al,0x0
db 0x74, 0x02                           ; 4D68 jz 0x4d6c
db 0xFE, 0xC8                           ; 4D6A dec al
db 0x5E                                 ; 4D6C pop si
db 0xC3                                 ; 4D6D ret
db 0xA0, 0x5E, 0x00                     ; 4D6E mov al,[0x5e]
db 0x0A, 0xC0                           ; 4D71 or al,al
db 0x74, 0x08                           ; 4D73 jz 0x4d7d
db 0x32, 0xC0                           ; 4D75 xor al,al
db 0xA2, 0x5E, 0x00                     ; 4D77 mov [0x5e],al
db 0xB0, 0x03                           ; 4D7A mov al,0x3
db 0xC3                                 ; 4D7C ret
db 0x56                                 ; 4D7D push si
db 0x57                                 ; 4D7E push di
db 0xA0, 0x6A, 0x00                     ; 4D7F mov al,[0x6a]
db 0x0A, 0xC0                           ; 4D82 or al,al
db 0x75, 0x74                           ; 4D84 jnz 0x4dfa
db 0xB4, 0x00                           ; 4D86 mov ah,0x0
db 0xCD, 0x16                           ; 4D88 int 0x16
db 0x0A, 0xC0                           ; 4D8A or al,al
db 0x74, 0x03                           ; 4D8C jz 0x4d91
db 0x5F                                 ; 4D8E pop di
db 0x5E                                 ; 4D8F pop si
db 0xC3                                 ; 4D90 ret
db 0x53                                 ; 4D91 push bx
db 0x80, 0xFC, 0x3B                     ; 4D92 cmp ah,0x3b
db 0x72, 0x05                           ; 4D95 jc 0x4d9c
db 0x80, 0xFC, 0x45                     ; 4D97 cmp ah,0x45
db 0x72, 0x3C                           ; 4D9A jc 0x4dd8
db 0x8B, 0x1E, 0x2E, 0x00               ; 4D9C mov bx,[0x2e]
db 0x43                                 ; 4DA0 inc bx
db 0x0B, 0xDB                           ; 4DA1 or bx,bx
db 0x75, 0x13                           ; 4DA3 jnz 0x4db8
db 0xBB, 0x34, 0x4E                     ; 4DA5 mov bx,0x4e34
db 0xB1, 0x1A                           ; 4DA8 mov cl,0x1a
db 0x2E, 0x3A, 0x27                     ; 4DAA cmp ah,[cs:bx]
db 0x74, 0x0C                           ; 4DAD jz 0x4dbb
db 0x43                                 ; 4DAF inc bx
db 0xFE, 0xC0                           ; 4DB0 inc al
db 0xFE, 0xC9                           ; 4DB2 dec cl
db 0x75, 0xF4                           ; 4DB4 jnz 0x4daa
db 0x32, 0xC0                           ; 4DB6 xor al,al
db 0x5B                                 ; 4DB8 pop bx
db 0xEB, 0xD3                           ; 4DB9 jmp short 0x4d8e
db 0x32, 0xE4                           ; 4DBB xor ah,ah
db 0xD0, 0xE0                           ; 4DBD shl al,1
db 0x8B, 0xD8                           ; 4DBF mov bx,ax
db 0x2E, 0x8B, 0x9F, 0x03, 0x01         ; 4DC1 mov bx,[cs:bx+0x103]
db 0x89, 0x1E, 0x6B, 0x00               ; 4DC6 mov [0x6b],bx
db 0xFE, 0x0E, 0x6A, 0x00               ; 4DCA dec byte [0x6a]
db 0xD0, 0xE8                           ; 4DCE shr al,1
db 0x04, 0x41                           ; 4DD0 add al,0x41
db 0x8C, 0x0E, 0x6D, 0x00               ; 4DD2 mov [0x6d],cs
db 0xEB, 0xE0                           ; 4DD6 jmp short 0x4db8
db 0x50                                 ; 4DD8 push ax
db 0x86, 0xC4                           ; 4DD9 xchg al,ah
db 0x2C, 0x3B                           ; 4DDB sub al,0x3b
db 0xB3, 0x10                           ; 4DDD mov bl,0x10
db 0xF6, 0xE3                           ; 4DDF mul bl
db 0xBB, 0x53, 0x06                     ; 4DE1 mov bx,0x653
db 0x03, 0xD8                           ; 4DE4 add bx,ax
db 0xF6, 0x07, 0xFF                     ; 4DE6 test byte [bx],0xff
db 0x58                                 ; 4DE9 pop ax
db 0x74, 0xCC                           ; 4DEA jz 0x4db8
db 0x89, 0x1E, 0x6B, 0x00               ; 4DEC mov [0x6b],bx
db 0x8C, 0x1E, 0x6D, 0x00               ; 4DF0 mov [0x6d],ds
db 0xFE, 0x0E, 0x6A, 0x00               ; 4DF4 dec byte [0x6a]
db 0xEB, 0x0C                           ; 4DF8 jmp short 0x4e06
db 0x53                                 ; 4DFA push bx
db 0xFE, 0xC8                           ; 4DFB dec al
db 0x75, 0x07                           ; 4DFD jnz 0x4e06
db 0xA2, 0x6A, 0x00                     ; 4DFF mov [0x6a],al
db 0xB0, 0x20                           ; 4E02 mov al,0x20
db 0xEB, 0xB2                           ; 4E04 jmp short 0x4db8
db 0x1E                                 ; 4E06 push ds
db 0xC5, 0x1E, 0x6B, 0x00               ; 4E07 lds bx,[0x6b]
db 0x8A, 0x07                           ; 4E0B mov al,[bx]
db 0x1F                                 ; 4E0D pop ds
db 0xFF, 0x06, 0x6B, 0x00               ; 4E0E inc word [0x6b]
db 0x0A, 0xC0                           ; 4E12 or al,al
db 0x74, 0x02                           ; 4E14 jz 0x4e18
db 0x79, 0xA0                           ; 4E16 jns 0x4db8
db 0x32, 0xE4                           ; 4E18 xor ah,ah
db 0x8C, 0xCB                           ; 4E1A mov bx,cs
db 0x8A, 0x1E, 0x6E, 0x00               ; 4E1C mov bl,[0x6e]
db 0x3A, 0xDF                           ; 4E20 cmp bl,bh
db 0x72, 0x04                           ; 4E22 jc 0x4e28
db 0xFE, 0xC4                           ; 4E24 inc ah
db 0x24, 0x7F                           ; 4E26 and al,0x7f
db 0x88, 0x26, 0x6A, 0x00               ; 4E28 mov [0x6a],ah
db 0x0A, 0xC0                           ; 4E2C or al,al
db 0x75, 0x88                           ; 4E2E jnz 0x4db8
db 0x5B                                 ; 4E30 pop bx
db 0xE9, 0x77, 0x11                     ; 4E31 jmp 0x5fab
db 0x1E                                 ; 4E34 push ds
db 0x30, 0x2E, 0x20, 0x12               ; 4E35 xor [0x1220],ch
db 0x21, 0x22                           ; 4E39 and [bp+si],sp
db 0x23, 0x17                           ; 4E3B and dx,[bx]
db 0x24, 0x25                           ; 4E3D and al,0x25
db 0x26, 0x32, 0x31                     ; 4E3F xor dh,[es:bx+di]
db 0x18, 0x19                           ; 4E42 sbb [bx+di],bl
db 0x10, 0x13                           ; 4E44 adc [bp+di],dl
db 0x1F                                 ; 4E46 pop ds
db 0x14, 0x16                           ; 4E47 adc al,0x16
db 0x2F                                 ; 4E49 das
db 0x11, 0x2D                           ; 4E4A adc [di],bp
db 0x15, 0x2C, 0x53                     ; 4E4C adc ax,0x532c
db 0x51                                 ; 4E4F push cx
db 0x56                                 ; 4E50 push si
db 0xBE, 0x6C, 0x4E                     ; 4E51 mov si,0x4e6c
db 0xB1, 0x0E                           ; 4E54 mov cl,0xe
db 0xFC                                 ; 4E56 cld
db 0x2E, 0xAC                           ; 4E57 cs lodsb
db 0x3A, 0xE0                           ; 4E59 cmp ah,al
db 0x74, 0x09                           ; 4E5B jz 0x4e66
db 0x46                                 ; 4E5D inc si
db 0xFE, 0xC9                           ; 4E5E dec cl
db 0x75, 0xF4                           ; 4E60 jnz 0x4e56
db 0x32, 0xC0                           ; 4E62 xor al,al
db 0xEB, 0x02                           ; 4E64 jmp short 0x4e68
db 0x2E, 0xAC                           ; 4E66 cs lodsb
db 0x5E                                 ; 4E68 pop si
db 0x59                                 ; 4E69 pop cx
db 0x5B                                 ; 4E6A pop bx
db 0xC3                                 ; 4E6B ret
db 0x47                                 ; 4E6C inc di
db 0x0B, 0x48, 0x1E                     ; 4E6D or cx,[bx+si+0x1e]
db 0x4B                                 ; 4E70 dec bx
db 0x1D, 0x4D, 0x1C                     ; 4E71 sbb ax,0x1c4d
db 0x50                                 ; 4E74 push ax
db 0x1F                                 ; 4E75 pop ds
db 0x1C, 0x0A                           ; 4E76 sbb al,0xa
db 0x74, 0x06                           ; 4E78 jz 0x4e80
db 0x73, 0x02                           ; 4E7A jnc 0x4e7e
db 0x76, 0x01                           ; 4E7C jna 0x4e7f
db 0x52                                 ; 4E7E push dx
db 0x12, 0x53, 0x7F                     ; 4E7F adc dl,[bp+di+0x7f]
db 0x4F                                 ; 4E82 dec di
db 0x0E                                 ; 4E83 push cs
db 0x75, 0x05                           ; 4E84 jnz 0x4e8b
db 0x77, 0x0C                           ; 4E86 ja 0x4e94
db 0x1F                                 ; 4E88 pop ds
db 0x1E                                 ; 4E89 push ds
db 0x1D, 0x1C, 0x0D                     ; 4E8A sbb ax,0xd1c
db 0x0C, 0x0B                           ; 4E8D or al,0xb
db 0x0A, 0x9C, 0x53, 0x51               ; 4E8F or bl,[si+0x5153]
db 0x52                                 ; 4E93 push dx
db 0x50                                 ; 4E94 push ax
db 0x3C, 0x07                           ; 4E95 cmp al,0x7
db 0x74, 0x4D                           ; 4E97 jz 0x4ee6
db 0x3C, 0x0D                           ; 4E99 cmp al,0xd
db 0x75, 0x0A                           ; 4E9B jnz 0x4ea7
db 0xF6, 0x06, 0x6F, 0x00, 0xFF         ; 4E9D test byte [0x6f],0xff
db 0x74, 0x03                           ; 4EA2 jz 0x4ea7
db 0xE8, 0x8B, 0x00                     ; 4EA4 call 0x4f32
db 0xE8, 0x2C, 0x02                     ; 4EA7 call 0x50d6
db 0x74, 0x04                           ; 4EAA jz 0x4eb0
db 0x3C, 0xFF                           ; 4EAC cmp al,0xff
db 0x74, 0x39                           ; 4EAE jz 0x4ee9
db 0x3C, 0x0C                           ; 4EB0 cmp al,0xc
db 0x74, 0x22                           ; 4EB2 jz 0x4ed6
db 0xBB, 0x87, 0x4E                     ; 4EB4 mov bx,0x4e87
db 0xB9, 0x08, 0x00                     ; 4EB7 mov cx,0x8
db 0x43                                 ; 4EBA inc bx
db 0xFE, 0xC9                           ; 4EBB dec cl
db 0x78, 0x1C                           ; 4EBD js 0x4edb
db 0x2E, 0x3A, 0x07                     ; 4EBF cmp al,[cs:bx]
db 0x75, 0xF6                           ; 4EC2 jnz 0x4eba
db 0xD0, 0xE1                           ; 4EC4 shl cl,1
db 0x8B, 0xD9                           ; 4EC6 mov bx,cx
db 0xB9, 0xE9, 0x4E                     ; 4EC8 mov cx,0x4ee9
db 0x51                                 ; 4ECB push cx
db 0x2E, 0xFF, 0xB7, 0xE1, 0x2F         ; 4ECC push word [cs:bx+0x2fe1]
db 0x8B, 0x1E, 0x56, 0x00               ; 4ED1 mov bx,[0x56]
db 0xC3                                 ; 4ED5 ret
db 0xE8, 0x0E, 0x04                     ; 4ED6 call 0x52e7
db 0xEB, 0x0E                           ; 4ED9 jmp short 0x4ee9
db 0xE8, 0x3E, 0x00                     ; 4EDB call 0x4f1c
db 0xE8, 0x0E, 0x00                     ; 4EDE call 0x4eef
db 0xE8, 0x0D, 0xE1                     ; 4EE1 call 0x2ff1
db 0xEB, 0x03                           ; 4EE4 jmp short 0x4ee9
db 0xE8, 0x24, 0x09                     ; 4EE6 call 0x580d
db 0x58                                 ; 4EE9 pop ax
db 0x5A                                 ; 4EEA pop dx
db 0x59                                 ; 4EEB pop cx
db 0x5B                                 ; 4EEC pop bx
db 0x9D                                 ; 4EED popf
db 0xC3                                 ; 4EEE ret
db 0x50                                 ; 4EEF push ax
db 0x8A, 0x3E, 0x49, 0x00               ; 4EF0 mov bh,[0x49]
db 0x8A, 0x1E, 0x4E, 0x00               ; 4EF4 mov bl,[0x4e]
db 0xB9, 0x01, 0x00                     ; 4EF8 mov cx,0x1
db 0xB4, 0x09                           ; 4EFB mov ah,0x9
db 0xCD, 0x10                           ; 4EFD int 0x10
db 0x58                                 ; 4EFF pop ax
db 0xC3                                 ; 4F00 ret
db 0x53                                 ; 4F01 push bx
db 0xE8, 0x71, 0x00                     ; 4F02 call 0x4f76
db 0xE8, 0xE7, 0xFF                     ; 4F05 call 0x4eef
db 0x5B                                 ; 4F08 pop bx
db 0xC3                                 ; 4F09 ret
db 0x53                                 ; 4F0A push bx
db 0xE8, 0x68, 0x00                     ; 4F0B call 0x4f76
db 0xB4, 0x08                           ; 4F0E mov ah,0x8
db 0xCD, 0x10                           ; 4F10 int 0x10
db 0x5B                                 ; 4F12 pop bx
db 0xC3                                 ; 4F13 ret
db 0xE8, 0xF3, 0xFF                     ; 4F14 call 0x4f0a
db 0x8A, 0xE8                           ; 4F17 mov ch,al
db 0x8A, 0xCC                           ; 4F19 mov cl,ah
db 0xC3                                 ; 4F1B ret
db 0x8B, 0x1E, 0x56, 0x00               ; 4F1C mov bx,[0x56]
db 0x89, 0x1E, 0x56, 0x00               ; 4F20 mov [0x56],bx
db 0x9C                                 ; 4F24 pushf
db 0x53                                 ; 4F25 push bx
db 0xE8, 0x4D, 0x00                     ; 4F26 call 0x4f76
db 0x5B                                 ; 4F29 pop bx
db 0x9D                                 ; 4F2A popf
db 0xC3                                 ; 4F2B ret
db 0xA0, 0x57, 0x00                     ; 4F2C mov al,[0x57]
db 0xFE, 0xC8                           ; 4F2F dec al
db 0xC3                                 ; 4F31 ret
db 0x50                                 ; 4F32 push ax
db 0x8A, 0x0E, 0x29, 0x00               ; 4F33 mov cl,[0x29]
db 0x2A, 0x0E, 0x57, 0x00               ; 4F37 sub cl,[0x57]
db 0xFE, 0xC1                           ; 4F3B inc cl
db 0xB5, 0x00                           ; 4F3D mov ch,0x0
db 0x8A, 0x3E, 0x49, 0x00               ; 4F3F mov bh,[0x49]
db 0x8A, 0x1E, 0x4F, 0x00               ; 4F43 mov bl,[0x4f]
db 0xB0, 0x20                           ; 4F47 mov al,0x20
db 0xB4, 0x09                           ; 4F49 mov ah,0x9
db 0xCD, 0x10                           ; 4F4B int 0x10
db 0x8B, 0x16, 0x56, 0x00               ; 4F4D mov dx,[0x56]
db 0x86, 0xF2                           ; 4F51 xchg dh,dl
db 0xFE, 0xCE                           ; 4F53 dec dh
db 0xFE, 0xCA                           ; 4F55 dec dl
db 0xB4, 0x02                           ; 4F57 mov ah,0x2
db 0xCD, 0x10                           ; 4F59 int 0x10
db 0x58                                 ; 4F5B pop ax
db 0xC3                                 ; 4F5C ret
db 0x53                                 ; 4F5D push bx
db 0xE8, 0x15, 0x00                     ; 4F5E call 0x4f76
db 0x8A, 0x3E, 0x49, 0x00               ; 4F61 mov bh,[0x49]
db 0x8A, 0x1E, 0x4F, 0x00               ; 4F65 mov bl,[0x4f]
db 0x8A, 0x0E, 0x29, 0x00               ; 4F69 mov cl,[0x29]
db 0xB5, 0x00                           ; 4F6D mov ch,0x0
db 0xB0, 0x20                           ; 4F6F mov al,0x20
db 0xB4, 0x09                           ; 4F71 mov ah,0x9
db 0xCD, 0x10                           ; 4F73 int 0x10
db 0x5B                                 ; 4F75 pop bx
db 0x50                                 ; 4F76 push ax
db 0x52                                 ; 4F77 push dx
db 0x8B, 0xD3                           ; 4F78 mov dx,bx
db 0x86, 0xF2                           ; 4F7A xchg dh,dl
db 0xFE, 0xCE                           ; 4F7C dec dh
db 0xFE, 0xCA                           ; 4F7E dec dl
db 0x8A, 0x3E, 0x49, 0x00               ; 4F80 mov bh,[0x49]
db 0xB4, 0x02                           ; 4F84 mov ah,0x2
db 0xCD, 0x10                           ; 4F86 int 0x10
db 0x5A                                 ; 4F88 pop dx
db 0x58                                 ; 4F89 pop ax
db 0xC3                                 ; 4F8A ret
db 0x53                                 ; 4F8B push bx
db 0x52                                 ; 4F8C push dx
db 0xB1, 0x00                           ; 4F8D mov cl,0x0
db 0x8A, 0xEF                           ; 4F8F mov ch,bh
db 0x8A, 0xF3                           ; 4F91 mov dh,bl
db 0xE8, 0x1B, 0x00                     ; 4F93 call 0x4fb1
db 0xB4, 0x06                           ; 4F96 mov ah,0x6
db 0xCD, 0x10                           ; 4F98 int 0x10
db 0xEB, 0x0F                           ; 4F9A jmp short 0x4fab
db 0x53                                 ; 4F9C push bx
db 0x52                                 ; 4F9D push dx
db 0xB1, 0x00                           ; 4F9E mov cl,0x0
db 0x8A, 0xEB                           ; 4FA0 mov ch,bl
db 0x8A, 0xF7                           ; 4FA2 mov dh,bh
db 0xE8, 0x0A, 0x00                     ; 4FA4 call 0x4fb1
db 0xB4, 0x07                           ; 4FA7 mov ah,0x7
db 0xCD, 0x10                           ; 4FA9 int 0x10
db 0xE8, 0x1C, 0x00                     ; 4FAB call 0x4fca
db 0x5A                                 ; 4FAE pop dx
db 0x5B                                 ; 4FAF pop bx
db 0xC3                                 ; 4FB0 ret
db 0xE8, 0x11, 0x00                     ; 4FB1 call 0x4fc5
db 0x8A, 0x16, 0x29, 0x00               ; 4FB4 mov dl,[0x29]
db 0xFE, 0xCA                           ; 4FB8 dec dl
db 0xFE, 0xCE                           ; 4FBA dec dh
db 0xFE, 0xCD                           ; 4FBC dec ch
db 0xB0, 0x01                           ; 4FBE mov al,0x1
db 0x8A, 0x3E, 0x4F, 0x00               ; 4FC0 mov bh,[0x4f]
db 0xC3                                 ; 4FC4 ret
db 0xA0, 0x49, 0x00                     ; 4FC5 mov al,[0x49]
db 0xEB, 0x03                           ; 4FC8 jmp short 0x4fcd
db 0xA0, 0x4A, 0x00                     ; 4FCA mov al,[0x4a]
db 0xE8, 0x06, 0x01                     ; 4FCD call 0x50d6
db 0x75, 0x20                           ; 4FD0 jnz 0x4ff2
db 0x8A, 0x26, 0x48, 0x00               ; 4FD2 mov ah,[0x48]
db 0x80, 0xFC, 0x07                     ; 4FD6 cmp ah,0x7
db 0x74, 0x17                           ; 4FD9 jz 0x4ff2
db 0x52                                 ; 4FDB push dx
db 0xBA, 0x00, 0x08                     ; 4FDC mov dx,0x800
db 0x80, 0xFC, 0x02                     ; 4FDF cmp ah,0x2
db 0x72, 0x02                           ; 4FE2 jc 0x4fe6
db 0xD0, 0xE6                           ; 4FE4 shl dh,1
db 0x32, 0xE4                           ; 4FE6 xor ah,ah
db 0xF7, 0xE2                           ; 4FE8 mul dx
db 0x1E                                 ; 4FEA push ds
db 0x8E, 0xDA                           ; 4FEB mov ds,dx
db 0xA3, 0x4E, 0x04                     ; 4FED mov [0x44e],ax
db 0x1F                                 ; 4FF0 pop ds
db 0x5A                                 ; 4FF1 pop dx
db 0xC3                                 ; 4FF2 ret
db 0x9C                                 ; 4FF3 pushf
db 0x53                                 ; 4FF4 push bx
db 0x52                                 ; 4FF5 push dx
db 0x50                                 ; 4FF6 push ax
db 0xBA, 0x00, 0x00                     ; 4FF7 mov dx,0x0
db 0xB4, 0x00                           ; 4FFA mov ah,0x0
db 0xCD, 0x17                           ; 4FFC int 0x17
db 0x8A, 0xC4                           ; 4FFE mov al,ah
db 0x80, 0xE4, 0x28                     ; 5000 and ah,0x28
db 0x80, 0xFC, 0x28                     ; 5003 cmp ah,0x28
db 0x74, 0x0D                           ; 5006 jz 0x5015
db 0xF6, 0xC4, 0x08                     ; 5008 test ah,0x8
db 0x75, 0x0C                           ; 500B jnz 0x5019
db 0xA8, 0x01                           ; 500D test al,0x1
db 0x74, 0x0D                           ; 500F jz 0x501e
db 0xB2, 0x18                           ; 5011 mov dl,0x18
db 0xEB, 0x06                           ; 5013 jmp short 0x501b
db 0xB2, 0x1B                           ; 5015 mov dl,0x1b
db 0xEB, 0x02                           ; 5017 jmp short 0x501b
db 0xB2, 0x19                           ; 5019 mov dl,0x19
db 0xE9, 0xBA, 0xB7                     ; 501B jmp 0x7d8
db 0x58                                 ; 501E pop ax
db 0x50                                 ; 501F push ax
db 0x3C, 0x0D                           ; 5020 cmp al,0xd
db 0xE9, 0x5D, 0x0F                     ; 5022 jmp 0x5f82
db 0x58                                 ; 5025 pop ax
db 0x5A                                 ; 5026 pop dx
db 0x5B                                 ; 5027 pop bx
db 0x9D                                 ; 5028 popf
db 0xC3                                 ; 5029 ret
db 0x3C, 0x93                           ; 502A cmp al,0x93
db 0x74, 0x60                           ; 502C jz 0x508e
db 0x3C, 0x95                           ; 502E cmp al,0x95
db 0x74, 0x46                           ; 5030 jz 0x5078
db 0x3C, 0xDD                           ; 5032 cmp al,0xdd
db 0x74, 0x46                           ; 5034 jz 0x507c
db 0xE8, 0xE3, 0xCE                     ; 5036 call 0x1f1c
db 0x0A, 0xC0                           ; 5039 or al,al
db 0x74, 0x38                           ; 503B jz 0x5075
db 0xFE, 0xC8                           ; 503D dec al
db 0x3C, 0x0A                           ; 503F cmp al,0xa
db 0x73, 0x32                           ; 5041 jnc 0x5075
db 0xBA, 0x10, 0x00                     ; 5043 mov dx,0x10
db 0xF6, 0xE2                           ; 5046 mul dl
db 0x8A, 0xD0                           ; 5048 mov dl,al
db 0x81, 0xC2, 0x53, 0x06               ; 504A add dx,0x653
db 0x52                                 ; 504E push dx
db 0xE8, 0xA2, 0xDD                     ; 504F call 0x2df4
db 0x2C, 0xE8                           ; 5052 sub al,0xe8
db 0xD1, 0xC6                           ; 5054 rol si,1
db 0x53                                 ; 5056 push bx
db 0xE8, 0x4C, 0xD8                     ; 5057 call 0x28a6
db 0x8A, 0x0F                           ; 505A mov cl,[bx]
db 0x80, 0xF9, 0x0F                     ; 505C cmp cl,0xf
db 0x72, 0x02                           ; 505F jc 0x5063
db 0xB1, 0x0F                           ; 5061 mov cl,0xf
db 0x43                                 ; 5063 inc bx
db 0x8B, 0x37                           ; 5064 mov si,[bx]
db 0x5B                                 ; 5066 pop bx
db 0x5F                                 ; 5067 pop di
db 0x53                                 ; 5068 push bx
db 0xB5, 0x00                           ; 5069 mov ch,0x0
db 0xFC                                 ; 506B cld
db 0xF3, 0xA4                           ; 506C rep movsb
db 0x88, 0x2D                           ; 506E mov [di],ch
db 0xE8, 0x72, 0xFB                     ; 5070 call 0x4be5
db 0x5B                                 ; 5073 pop bx
db 0xC3                                 ; 5074 ret
db 0xE9, 0xE1, 0xBF                     ; 5075 jmp 0x1059
db 0xB0, 0xFF                           ; 5078 mov al,0xff
db 0xEB, 0x02                           ; 507A jmp short 0x507e
db 0xB0, 0x00                           ; 507C mov al,0x0
db 0x3A, 0x06, 0x71, 0x00               ; 507E cmp al,[0x71]
db 0xA2, 0x71, 0x00                     ; 5082 mov [0x71],al
db 0x74, 0x03                           ; 5085 jz 0x508a
db 0xE8, 0x5E, 0x00                     ; 5087 call 0x50e8
db 0xE8, 0x90, 0xBE                     ; 508A call 0xf1d
db 0xC3                                 ; 508D ret
db 0x53                                 ; 508E push bx
db 0xBE, 0x53, 0x06                     ; 508F mov si,0x653
db 0xB9, 0x0A, 0x00                     ; 5092 mov cx,0xa
db 0xFE, 0xC5                           ; 5095 inc ch
db 0x56                                 ; 5097 push si
db 0xB0, 0x46                           ; 5098 mov al,0x46
db 0xE8, 0x08, 0xDB                     ; 509A call 0x2ba5
db 0x51                                 ; 509D push cx
db 0x8A, 0xDD                           ; 509E mov bl,ch
db 0xB7, 0x00                           ; 50A0 mov bh,0x0
db 0xE8, 0xAB, 0x14                     ; 50A2 call 0x6550
db 0xB0, 0x20                           ; 50A5 mov al,0x20
db 0xE8, 0xFB, 0xDA                     ; 50A7 call 0x2ba5
db 0x59                                 ; 50AA pop cx
db 0x5E                                 ; 50AB pop si
db 0x56                                 ; 50AC push si
db 0x51                                 ; 50AD push cx
db 0xFC                                 ; 50AE cld
db 0xAC                                 ; 50AF lodsb
db 0x0A, 0xC0                           ; 50B0 or al,al
db 0x74, 0x05                           ; 50B2 jz 0x50b9
db 0xE8, 0x13, 0x00                     ; 50B4 call 0x50ca
db 0xEB, 0xF5                           ; 50B7 jmp short 0x50ae
db 0xB0, 0x0D                           ; 50B9 mov al,0xd
db 0xE8, 0xE7, 0xDA                     ; 50BB call 0x2ba5
db 0x59                                 ; 50BE pop cx
db 0x5E                                 ; 50BF pop si
db 0x83, 0xC6, 0x10                     ; 50C0 add si,byte +0x10
db 0xFE, 0xC9                           ; 50C3 dec cl
db 0x75, 0xCE                           ; 50C5 jnz 0x5095
db 0x5B                                 ; 50C7 pop bx
db 0xEB, 0xC0                           ; 50C8 jmp short 0x508a
db 0x56                                 ; 50CA push si
db 0x3C, 0x0D                           ; 50CB cmp al,0xd
db 0x75, 0x02                           ; 50CD jnz 0x50d1
db 0xB0, 0x1B                           ; 50CF mov al,0x1b
db 0xE8, 0xD1, 0xDA                     ; 50D1 call 0x2ba5
db 0x5E                                 ; 50D4 pop si
db 0xC3                                 ; 50D5 ret
db 0x50                                 ; 50D6 push ax
db 0xA0, 0x48, 0x00                     ; 50D7 mov al,[0x48]
db 0x3C, 0x07                           ; 50DA cmp al,0x7
db 0x74, 0x04                           ; 50DC jz 0x50e2
db 0x3C, 0x04                           ; 50DE cmp al,0x4
db 0x73, 0x02                           ; 50E0 jnc 0x50e4
db 0x32, 0xC0                           ; 50E2 xor al,al
db 0x0A, 0xC0                           ; 50E4 or al,al
db 0x58                                 ; 50E6 pop ax
db 0xC3                                 ; 50E7 ret
db 0x53                                 ; 50E8 push bx
db 0xCD, 0xAD                           ; 50E9 int 0xad
db 0xB6, 0x18                           ; 50EB mov dh,0x18
db 0xB2, 0x00                           ; 50ED mov dl,0x0
db 0x8A, 0x3E, 0x49, 0x00               ; 50EF mov bh,[0x49]
db 0xB4, 0x02                           ; 50F3 mov ah,0x2
db 0xCD, 0x10                           ; 50F5 int 0x10
db 0xA0, 0x71, 0x00                     ; 50F7 mov al,[0x71]
db 0x0A, 0xC0                           ; 50FA or al,al
db 0x75, 0x13                           ; 50FC jnz 0x5111
db 0x8A, 0x1E, 0x4F, 0x00               ; 50FE mov bl,[0x4f]
db 0x8A, 0x0E, 0x29, 0x00               ; 5102 mov cl,[0x29]
db 0xB5, 0x00                           ; 5106 mov ch,0x0
db 0xB4, 0x09                           ; 5108 mov ah,0x9
db 0xCD, 0x10                           ; 510A int 0x10
db 0xE8, 0x0D, 0xFE                     ; 510C call 0x4f1c
db 0x5B                                 ; 510F pop bx
db 0xC3                                 ; 5110 ret
db 0xB3, 0x07                           ; 5111 mov bl,0x7
db 0xE8, 0xC0, 0xFF                     ; 5113 call 0x50d6
db 0x75, 0x09                           ; 5116 jnz 0x5121
db 0xA0, 0x4C, 0x00                     ; 5118 mov al,[0x4c]
db 0x0A, 0xC0                           ; 511B or al,al
db 0x75, 0x02                           ; 511D jnz 0x5121
db 0xB3, 0x70                           ; 511F mov bl,0x70
db 0xBE, 0x53, 0x06                     ; 5121 mov si,0x653
db 0xB5, 0x05                           ; 5124 mov ch,0x5
db 0xA0, 0x29, 0x00                     ; 5126 mov al,[0x29]
db 0x3C, 0x28                           ; 5129 cmp al,0x28
db 0xB0, 0x31                           ; 512B mov al,0x31
db 0x74, 0x02                           ; 512D jz 0x5131
db 0xB5, 0x0A                           ; 512F mov ch,0xa
db 0x50                                 ; 5131 push ax
db 0x53                                 ; 5132 push bx
db 0x8A, 0x1E, 0x4E, 0x00               ; 5133 mov bl,[0x4e]
db 0xE8, 0x37, 0x00                     ; 5137 call 0x5171
db 0x5B                                 ; 513A pop bx
db 0x56                                 ; 513B push si
db 0xB1, 0x06                           ; 513C mov cl,0x6
db 0x51                                 ; 513E push cx
db 0xFC                                 ; 513F cld
db 0xAC                                 ; 5140 lodsb
db 0x0A, 0xC0                           ; 5141 or al,al
db 0x9C                                 ; 5143 pushf
db 0x56                                 ; 5144 push si
db 0x75, 0x02                           ; 5145 jnz 0x5149
db 0x32, 0xC0                           ; 5147 xor al,al
db 0xE8, 0x25, 0x00                     ; 5149 call 0x5171
db 0x5E                                 ; 514C pop si
db 0x9D                                 ; 514D popf
db 0x75, 0x01                           ; 514E jnz 0x5151
db 0x4E                                 ; 5150 dec si
db 0x59                                 ; 5151 pop cx
db 0xFE, 0xC9                           ; 5152 dec cl
db 0x75, 0xE8                           ; 5154 jnz 0x513e
db 0xE8, 0x16, 0x00                     ; 5156 call 0x516f
db 0x5E                                 ; 5159 pop si
db 0x83, 0xC6, 0x10                     ; 515A add si,byte +0x10
db 0x58                                 ; 515D pop ax
db 0xFE, 0xC0                           ; 515E inc al
db 0x3C, 0x3A                           ; 5160 cmp al,0x3a
db 0x72, 0x02                           ; 5162 jc 0x5166
db 0xB0, 0x30                           ; 5164 mov al,0x30
db 0xFE, 0xCD                           ; 5166 dec ch
db 0x75, 0xC7                           ; 5168 jnz 0x5131
db 0xE8, 0xAF, 0xFD                     ; 516A call 0x4f1c
db 0x5B                                 ; 516D pop bx
db 0xC3                                 ; 516E ret
db 0x32, 0xC0                           ; 516F xor al,al
db 0x53                                 ; 5171 push bx
db 0x0A, 0xC0                           ; 5172 or al,al
db 0x75, 0x06                           ; 5174 jnz 0x517c
db 0xB0, 0x20                           ; 5176 mov al,0x20
db 0x8A, 0x1E, 0x4F, 0x00               ; 5178 mov bl,[0x4f]
db 0x3C, 0x0D                           ; 517C cmp al,0xd
db 0x75, 0x02                           ; 517E jnz 0x5182
db 0xB0, 0x1B                           ; 5180 mov al,0x1b
db 0x51                                 ; 5182 push cx
db 0xB9, 0x01, 0x00                     ; 5183 mov cx,0x1
db 0xB4, 0x09                           ; 5186 mov ah,0x9
db 0xCD, 0x10                           ; 5188 int 0x10
db 0xFE, 0xC2                           ; 518A inc dl
db 0xB4, 0x02                           ; 518C mov ah,0x2
db 0xCD, 0x10                           ; 518E int 0x10
db 0x59                                 ; 5190 pop cx
db 0x5B                                 ; 5191 pop bx
db 0xC3                                 ; 5192 ret
db 0x8A, 0x0E, 0x49, 0x00               ; 5193 mov cl,[0x49]
db 0xB5, 0x00                           ; 5197 mov ch,0x0
db 0x8A, 0x26, 0x48, 0x00               ; 5199 mov ah,[0x48]
db 0xF6, 0xC4, 0x01                     ; 519D test ah,0x1
db 0x74, 0x03                           ; 51A0 jz 0x51a5
db 0x80, 0xCD, 0x80                     ; 51A2 or ch,0x80
db 0x80, 0xFC, 0x04                     ; 51A5 cmp ah,0x4
db 0x72, 0x09                           ; 51A8 jc 0x51b3
db 0xFE, 0xC5                           ; 51AA inc ch
db 0x80, 0xFC, 0x06                     ; 51AC cmp ah,0x6
db 0x72, 0x02                           ; 51AF jc 0x51b3
db 0xFE, 0xC5                           ; 51B1 inc ch
db 0x51                                 ; 51B3 push cx
db 0x3C, 0x2C                           ; 51B4 cmp al,0x2c
db 0x74, 0x0C                           ; 51B6 jz 0x51c4
db 0xE8, 0x61, 0xCD                     ; 51B8 call 0x1f1c
db 0x59                                 ; 51BB pop cx
db 0x8A, 0xE8                           ; 51BC mov ch,al
db 0x51                                 ; 51BE push cx
db 0xE8, 0x5C, 0xBD                     ; 51BF call 0xf1e
db 0x74, 0x40                           ; 51C2 jz 0x5204
db 0xE8, 0x2D, 0xDC                     ; 51C4 call 0x2df4
db 0x2C, 0x3C                           ; 51C7 sub al,0x3c
db 0x2C, 0x74                           ; 51C9 sub al,0x74
db 0x15, 0xE8, 0x4D                     ; 51CB adc ax,0x4de8
db 0xCD, 0x0A                           ; 51CE int 0xa
db 0xC0                                 ; 51D0 db 0xC0
db 0x74, 0x02                           ; 51D1 jz 0x51d5
db 0xB0, 0x80                           ; 51D3 mov al,0x80
db 0x59                                 ; 51D5 pop cx
db 0x80, 0xE5, 0x03                     ; 51D6 and ch,0x3
db 0x0A, 0xE8                           ; 51D9 or ch,al
db 0x51                                 ; 51DB push cx
db 0xE8, 0x3F, 0xBD                     ; 51DC call 0xf1e
db 0x74, 0x23                           ; 51DF jz 0x5204
db 0xE8, 0x10, 0xDC                     ; 51E1 call 0x2df4
db 0x2C, 0x3C                           ; 51E4 sub al,0x3c
db 0x2C, 0x74                           ; 51E6 sub al,0x74
db 0x0C, 0xE8                           ; 51E8 or al,0xe8
db 0x30, 0xCD                           ; 51EA xor ch,cl
db 0x59                                 ; 51EC pop cx
db 0x8A, 0xC8                           ; 51ED mov cl,al
db 0x51                                 ; 51EF push cx
db 0xE8, 0x2B, 0xBD                     ; 51F0 call 0xf1e
db 0x74, 0x0F                           ; 51F3 jz 0x5204
db 0xE8, 0xFC, 0xDB                     ; 51F5 call 0x2df4
db 0x2C, 0xE8                           ; 51F8 sub al,0xe8
db 0x20, 0xCD                           ; 51FA and ch,cl
db 0x8A, 0xF0                           ; 51FC mov dh,al
db 0x59                                 ; 51FE pop cx
db 0xEB, 0x06                           ; 51FF jmp short 0x5207
db 0xE9, 0x55, 0xBE                     ; 5201 jmp 0x1059
db 0x59                                 ; 5204 pop cx
db 0x8A, 0xF1                           ; 5205 mov dh,cl
db 0x8A, 0x26, 0x29, 0x00               ; 5207 mov ah,[0x29]
db 0x8A, 0xC5                           ; 520B mov al,ch
db 0x24, 0x7F                           ; 520D and al,0x7f
db 0x0A, 0xC0                           ; 520F or al,al
db 0x74, 0x0A                           ; 5211 jz 0x521d
db 0x32, 0xD2                           ; 5213 xor dl,dl
db 0x0A, 0xD6                           ; 5215 or dl,dh
db 0x0A, 0xD1                           ; 5217 or dl,cl
db 0x75, 0xE6                           ; 5219 jnz 0x5201
db 0xEB, 0x1B                           ; 521B jmp short 0x5238
db 0x80, 0xFC, 0x28                     ; 521D cmp ah,0x28
db 0x74, 0x0C                           ; 5220 jz 0x522e
db 0x80, 0xFE, 0x04                     ; 5222 cmp dh,0x4
db 0x73, 0xDA                           ; 5225 jnc 0x5201
db 0x80, 0xF9, 0x04                     ; 5227 cmp cl,0x4
db 0x72, 0x0C                           ; 522A jc 0x5238
db 0xEB, 0xD3                           ; 522C jmp short 0x5201
db 0x80, 0xFE, 0x08                     ; 522E cmp dh,0x8
db 0x73, 0xCE                           ; 5231 jnc 0x5201
db 0x80, 0xF9, 0x08                     ; 5233 cmp cl,0x8
db 0x73, 0xC9                           ; 5236 jnc 0x5201
db 0x8A, 0xD1                           ; 5238 mov dl,cl
db 0x0A, 0xC0                           ; 523A or al,al
db 0x74, 0x20                           ; 523C jz 0x525e
db 0x80, 0x3E, 0x48, 0x00, 0x07         ; 523E cmp byte [0x48],0x7
db 0x74, 0x5C                           ; 5243 jz 0x52a1
db 0xB1, 0x06                           ; 5245 mov cl,0x6
db 0x3C, 0x02                           ; 5247 cmp al,0x2
db 0xB4, 0x50                           ; 5249 mov ah,0x50
db 0x74, 0x2A                           ; 524B jz 0x5277
db 0xB4, 0x28                           ; 524D mov ah,0x28
db 0xFE, 0xC9                           ; 524F dec cl
db 0xFE, 0xC8                           ; 5251 dec al
db 0x75, 0xAC                           ; 5253 jnz 0x5201
db 0xF6, 0xC5, 0x80                     ; 5255 test ch,0x80
db 0x75, 0x1D                           ; 5258 jnz 0x5277
db 0xFE, 0xC9                           ; 525A dec cl
db 0xEB, 0x19                           ; 525C jmp short 0x5277
db 0xB1, 0x02                           ; 525E mov cl,0x2
db 0x80, 0xFC, 0x28                     ; 5260 cmp ah,0x28
db 0x74, 0x09                           ; 5263 jz 0x526e
db 0xF6, 0xC5, 0x80                     ; 5265 test ch,0x80
db 0x74, 0x0D                           ; 5268 jz 0x5277
db 0xFE, 0xC1                           ; 526A inc cl
db 0xEB, 0x09                           ; 526C jmp short 0x5277
db 0xFE, 0xC9                           ; 526E dec cl
db 0xF6, 0xC5, 0x80                     ; 5270 test ch,0x80
db 0x75, 0x02                           ; 5273 jnz 0x5277
db 0xFE, 0xC9                           ; 5275 dec cl
db 0x88, 0x26, 0x29, 0x00               ; 5277 mov [0x29],ah
db 0xA1, 0x48, 0x00                     ; 527B mov ax,[0x48]
db 0x88, 0x0E, 0x48, 0x00               ; 527E mov [0x48],cl
db 0x89, 0x16, 0x49, 0x00               ; 5282 mov [0x49],dx
db 0x3A, 0xC1                           ; 5286 cmp al,cl
db 0x74, 0x1A                           ; 5288 jz 0x52a4
db 0xB8, 0x07, 0x00                     ; 528A mov ax,0x7
db 0xA3, 0x4B, 0x00                     ; 528D mov [0x4b],ax
db 0x86, 0xC4                           ; 5290 xchg al,ah
db 0xA3, 0x4D, 0x00                     ; 5292 mov [0x4d],ax
db 0x88, 0x26, 0x4F, 0x00               ; 5295 mov [0x4f],ah
db 0xE8, 0x3A, 0xFE                     ; 5299 call 0x50d6
db 0x74, 0x03                           ; 529C jz 0x52a1
db 0xA2, 0x4F, 0x00                     ; 529E mov [0x4f],al
db 0xE8, 0x6E, 0x00                     ; 52A1 call 0x5312
db 0xA0, 0x4A, 0x00                     ; 52A4 mov al,[0x4a]
db 0xB4, 0x05                           ; 52A7 mov ah,0x5
db 0xCD, 0x10                           ; 52A9 int 0x10
db 0xC3                                 ; 52AB ret
db 0x3A, 0x06, 0x29, 0x00               ; 52AC cmp al,[0x29]
db 0x74, 0x34                           ; 52B0 jz 0x52e6
db 0x8A, 0x26, 0x48, 0x00               ; 52B2 mov ah,[0x48]
db 0x3C, 0x50                           ; 52B6 cmp al,0x50
db 0x74, 0x07                           ; 52B8 jz 0x52c1
db 0x3C, 0x28                           ; 52BA cmp al,0x28
db 0x74, 0x03                           ; 52BC jz 0x52c1
db 0xE9, 0x98, 0xBD                     ; 52BE jmp 0x1059
db 0x80, 0xFC, 0x07                     ; 52C1 cmp ah,0x7
db 0x75, 0x04                           ; 52C4 jnz 0x52ca
db 0xB0, 0x50                           ; 52C6 mov al,0x50
db 0xEB, 0x1C                           ; 52C8 jmp short 0x52e6
db 0x80, 0xF4, 0x02                     ; 52CA xor ah,0x2
db 0x80, 0xFC, 0x07                     ; 52CD cmp ah,0x7
db 0x75, 0x02                           ; 52D0 jnz 0x52d4
db 0xFE, 0xCC                           ; 52D2 dec ah
db 0x50                                 ; 52D4 push ax
db 0xA2, 0x29, 0x00                     ; 52D5 mov [0x29],al
db 0x88, 0x26, 0x48, 0x00               ; 52D8 mov [0x48],ah
db 0xC7, 0x06, 0x49, 0x00, 0x00, 0x00   ; 52DC mov word [0x49],0x0
db 0xE8, 0x2D, 0x00                     ; 52E2 call 0x5312
db 0x58                                 ; 52E5 pop ax
db 0xC3                                 ; 52E6 ret
L_52E7:
db 0x53                                 ; 52E7 push bx
db 0xE8, 0xDA, 0xFC                     ; 52E8 call 0x4fc5
db 0xB2, 0x27                           ; 52EB mov dl,0x27
db 0x80, 0x3E, 0x29, 0x00, 0x28         ; 52ED cmp byte [0x29],0x28
db 0x74, 0x02                           ; 52F2 jz 0x52f6
db 0xB2, 0x4F                           ; 52F4 mov dl,0x4f
db 0xB6, 0x18                           ; 52F6 mov dh,0x18
db 0x8A, 0x3E, 0x4F, 0x00               ; 52F8 mov bh,[0x4f]
db 0xB9, 0x00, 0x00                     ; 52FC mov cx,0x0
db 0x8A, 0xC1                           ; 52FF mov al,cl
db 0xB4, 0x06                           ; 5301 mov ah,0x6
db 0xCD, 0x10                           ; 5303 int 0x10
db 0xBA, 0x00, 0x00                     ; 5305 mov dx,0x0
db 0x8A, 0x3E, 0x49, 0x00               ; 5308 mov bh,[0x49]
db 0xB4, 0x02                           ; 530C mov ah,0x2
db 0xCD, 0x10                           ; 530E int 0x10
db 0xEB, 0x0F                           ; 5310 jmp short 0x5321
db 0x53                                 ; 5312 push bx
db 0xB9, 0x00, 0x00                     ; 5313 mov cx,0x0
db 0x89, 0x0E, 0x49, 0x00               ; 5316 mov [0x49],cx
db 0xA0, 0x48, 0x00                     ; 531A mov al,[0x48]
db 0xB4, 0x00                           ; 531D mov ah,0x0
db 0xCD, 0x10                           ; 531F int 0x10
db 0xE8, 0xBD, 0xDD                     ; 5321 call 0x30e1
db 0xE8, 0xC9, 0xF8                     ; 5324 call 0x4bf0
db 0xE8, 0x8A, 0xF7                     ; 5327 call 0x4ab4
db 0xE8, 0x9D, 0xFC                     ; 532A call 0x4fca
db 0x5B                                 ; 532D pop bx
db 0xC3                                 ; 532E ret
db 0xE8, 0xA4, 0xFD                     ; 532F call 0x50d6
db 0x74, 0x5B                           ; 5332 jz 0x538f
db 0xB1, 0x00                           ; 5334 mov cl,0x0
db 0xBE, 0x51, 0x00                     ; 5336 mov si,0x51
db 0x80, 0x3E, 0x48, 0x00, 0x06         ; 5339 cmp byte [0x48],0x6
db 0x75, 0x03                           ; 533E jnz 0x5343
db 0xE9, 0x16, 0xBD                     ; 5340 jmp 0x1059
db 0x8A, 0x2C                           ; 5343 mov ch,[si]
db 0x56                                 ; 5345 push si
db 0x51                                 ; 5346 push cx
db 0xE8, 0xD4, 0xBB                     ; 5347 call 0xf1e
db 0x74, 0x40                           ; 534A jz 0x538c
db 0x3C, 0x2C                           ; 534C cmp al,0x2c
db 0x74, 0x07                           ; 534E jz 0x5357
db 0xE8, 0xC9, 0xCB                     ; 5350 call 0x1f1c
db 0x59                                 ; 5353 pop cx
db 0x8A, 0xE8                           ; 5354 mov ch,al
db 0x51                                 ; 5356 push cx
db 0x59                                 ; 5357 pop cx
db 0x51                                 ; 5358 push cx
db 0x53                                 ; 5359 push bx
db 0x8A, 0xF9                           ; 535A mov bh,cl
db 0x8A, 0xDD                           ; 535C mov bl,ch
db 0x80, 0xFF, 0x00                     ; 535E cmp bh,0x0
db 0x75, 0x08                           ; 5361 jnz 0x536b
db 0x80, 0xFB, 0x08                     ; 5363 cmp bl,0x8
db 0x72, 0x03                           ; 5366 jc 0x536b
db 0x80, 0xCB, 0x10                     ; 5368 or bl,0x10
db 0xB4, 0x0B                           ; 536B mov ah,0xb
db 0xCD, 0x10                           ; 536D int 0x10
db 0x5B                                 ; 536F pop bx
db 0xE8, 0xAB, 0xBB                     ; 5370 call 0xf1e
db 0x74, 0x03                           ; 5373 jz 0x5378
db 0xE8, 0xA5, 0xBB                     ; 5375 call 0xf1d
db 0x59                                 ; 5378 pop cx
db 0x5E                                 ; 5379 pop si
db 0x88, 0x2C                           ; 537A mov [si],ch
db 0x74, 0x08                           ; 537C jz 0x5386
db 0x46                                 ; 537E inc si
db 0xFE, 0xC1                           ; 537F inc cl
db 0x80, 0xF9, 0x04                     ; 5381 cmp cl,0x4
db 0x72, 0xBD                           ; 5384 jc 0x5343
db 0xC6, 0x06, 0x4F, 0x00, 0x00         ; 5386 mov byte [0x4f],0x0
db 0xC3                                 ; 538B ret
db 0x59                                 ; 538C pop cx
db 0x5E                                 ; 538D pop si
db 0xC3                                 ; 538E ret
db 0xFF, 0x36, 0x4D, 0x00               ; 538F push word [0x4d]
db 0xFF, 0x36, 0x4B, 0x00               ; 5393 push word [0x4b]
db 0x3C, 0x2C                           ; 5397 cmp al,0x2c
db 0x74, 0x10                           ; 5399 jz 0x53ab
db 0xE8, 0x7E, 0xCB                     ; 539B call 0x1f1c
db 0x3C, 0x20                           ; 539E cmp al,0x20
db 0x73, 0x18                           ; 53A0 jnc 0x53ba
db 0x59                                 ; 53A2 pop cx
db 0x8A, 0xC8                           ; 53A3 mov cl,al
db 0x51                                 ; 53A5 push cx
db 0xE8, 0x75, 0xBB                     ; 53A6 call 0xf1e
db 0x74, 0x2C                           ; 53A9 jz 0x53d7
db 0xE8, 0x46, 0xDA                     ; 53AB call 0x2df4
db 0x2C, 0x3C                           ; 53AE sub al,0x3c
db 0x2C, 0x74                           ; 53B0 sub al,0x74
db 0x13, 0xE8                           ; 53B2 adc bp,ax
db 0x66, 0xCB                           ; 53B4 o32 retf
db 0x3C, 0x10                           ; 53B6 cmp al,0x10
db 0x72, 0x03                           ; 53B8 jc 0x53bd
db 0xE9, 0x9C, 0xBC                     ; 53BA jmp 0x1059
db 0x59                                 ; 53BD pop cx
db 0x8A, 0xE8                           ; 53BE mov ch,al
db 0x51                                 ; 53C0 push cx
db 0xE8, 0x5A, 0xBB                     ; 53C1 call 0xf1e
db 0x74, 0x11                           ; 53C4 jz 0x53d7
db 0xE8, 0x2B, 0xDA                     ; 53C6 call 0x2df4
db 0x2C, 0xE8                           ; 53C9 sub al,0xe8
db 0x4F                                 ; 53CB dec di
db 0xCB                                 ; 53CC retf
db 0x3C, 0x10                           ; 53CD cmp al,0x10
db 0x73, 0xE9                           ; 53CF jnc 0x53ba
db 0x59                                 ; 53D1 pop cx
db 0x5A                                 ; 53D2 pop dx
db 0x8A, 0xD0                           ; 53D3 mov dl,al
db 0x52                                 ; 53D5 push dx
db 0x51                                 ; 53D6 push cx
db 0x59                                 ; 53D7 pop cx
db 0x5A                                 ; 53D8 pop dx
db 0x8A, 0xF1                           ; 53D9 mov dh,cl
db 0x80, 0xE6, 0x0F                     ; 53DB and dh,0xf
db 0x89, 0x0E, 0x4B, 0x00               ; 53DE mov [0x4b],cx
db 0x8A, 0xC5                           ; 53E2 mov al,ch
db 0xD0, 0xE0                           ; 53E4 shl al,1
db 0x24, 0x10                           ; 53E6 and al,0x10
db 0x0A, 0xC2                           ; 53E8 or al,dl
db 0x80, 0xE5, 0x07                     ; 53EA and ch,0x7
db 0xD0, 0xE5                           ; 53ED shl ch,1
db 0xD0, 0xE5                           ; 53EF shl ch,1
db 0xD0, 0xE5                           ; 53F1 shl ch,1
db 0xD0, 0xE5                           ; 53F3 shl ch,1
db 0xF6, 0xC1, 0x10                     ; 53F5 test cl,0x10
db 0x74, 0x03                           ; 53F8 jz 0x53fd
db 0x80, 0xCD, 0x80                     ; 53FA or ch,0x80
db 0x0A, 0xEE                           ; 53FD or ch,dh
db 0x53                                 ; 53FF push bx
db 0x8A, 0xD8                           ; 5400 mov bl,al
db 0xB7, 0x00                           ; 5402 mov bh,0x0
db 0x24, 0x0F                           ; 5404 and al,0xf
db 0xA2, 0x4D, 0x00                     ; 5406 mov [0x4d],al
db 0x88, 0x2E, 0x4E, 0x00               ; 5409 mov [0x4e],ch
db 0x88, 0x2E, 0x4F, 0x00               ; 540D mov [0x4f],ch
db 0xB4, 0x0B                           ; 5411 mov ah,0xb
db 0xCD, 0x10                           ; 5413 int 0x10
db 0x5B                                 ; 5415 pop bx
db 0xC3                                 ; 5416 ret
db 0xFF, 0x36, 0x56, 0x00               ; 5417 push word [0x56]
db 0x3C, 0x2C                           ; 541B cmp al,0x2c
db 0x74, 0x20                           ; 541D jz 0x543f
db 0xE8, 0xFA, 0xCA                     ; 541F call 0x1f1c
db 0x0A, 0xC0                           ; 5422 or al,al
db 0x74, 0x5B                           ; 5424 jz 0x5481
db 0x3C, 0x1A                           ; 5426 cmp al,0x1a
db 0x73, 0x57                           ; 5428 jnc 0x5481
db 0x8A, 0x26, 0x71, 0x00               ; 542A mov ah,[0x71]
db 0x0A, 0xE4                           ; 542E or ah,ah
db 0x74, 0x04                           ; 5430 jz 0x5436
db 0x3C, 0x19                           ; 5432 cmp al,0x19
db 0x73, 0x4B                           ; 5434 jnc 0x5481
db 0x5A                                 ; 5436 pop dx
db 0x8A, 0xD0                           ; 5437 mov dl,al
db 0x52                                 ; 5439 push dx
db 0xE8, 0xE1, 0xBA                     ; 543A call 0xf1e
db 0x74, 0x7B                           ; 543D jz 0x54ba
db 0xE8, 0xB2, 0xD9                     ; 543F call 0x2df4
db 0x2C, 0x3C                           ; 5442 sub al,0x3c
db 0x2C, 0x74                           ; 5444 sub al,0x74
db 0x18, 0xE8                           ; 5446 sbb al,ch
db 0xD2, 0xCA                           ; 5448 ror dl,cl
db 0x0A, 0xC0                           ; 544A or al,al
db 0x74, 0x33                           ; 544C jz 0x5481
db 0x8A, 0x26, 0x29, 0x00               ; 544E mov ah,[0x29]
db 0x3A, 0xE0                           ; 5452 cmp ah,al
db 0x72, 0x2B                           ; 5454 jc 0x5481
db 0x5A                                 ; 5456 pop dx
db 0x8A, 0xF0                           ; 5457 mov dh,al
db 0x52                                 ; 5459 push dx
db 0xE8, 0xC1, 0xBA                     ; 545A call 0xf1e
db 0x74, 0x5B                           ; 545D jz 0x54ba
db 0xFF, 0x36, 0x68, 0x00               ; 545F push word [0x68]
db 0xE8, 0x8E, 0xD9                     ; 5463 call 0x2df4
db 0x2C, 0x3C                           ; 5466 sub al,0x3c
db 0x2C, 0x74                           ; 5468 sub al,0x74
db 0x19, 0xE8                           ; 546A sbb ax,bp
db 0xAE                                 ; 546C scasb
db 0xCA, 0x0A, 0xC0                     ; 546D retf 0xc00a
db 0xB0, 0x00                           ; 5470 mov al,0x0
db 0x75, 0x02                           ; 5472 jnz 0x5476
db 0xB0, 0x20                           ; 5474 mov al,0x20
db 0x59                                 ; 5476 pop cx
db 0x0A, 0xE8                           ; 5477 or ch,al
db 0x51                                 ; 5479 push cx
db 0xE8, 0xA1, 0xBA                     ; 547A call 0xf1e
db 0x74, 0x2D                           ; 547D jz 0x54ac
db 0xEB, 0x03                           ; 547F jmp short 0x5484
db 0xE9, 0xD5, 0xBB                     ; 5481 jmp 0x1059
db 0xE8, 0x6D, 0xD9                     ; 5484 call 0x2df4
db 0x2C, 0xE8                           ; 5487 sub al,0xe8
db 0x91                                 ; 5489 xchg ax,cx
db 0xCA, 0x3C, 0x20                     ; 548A retf 0x203c
db 0x73, 0xF2                           ; 548D jnc 0x5481
db 0x59                                 ; 548F pop cx
db 0x80, 0xE5, 0x20                     ; 5490 and ch,0x20
db 0x0A, 0xE8                           ; 5493 or ch,al
db 0x8A, 0xC8                           ; 5495 mov cl,al
db 0x51                                 ; 5497 push cx
db 0xE8, 0x83, 0xBA                     ; 5498 call 0xf1e
db 0x74, 0x0F                           ; 549B jz 0x54ac
db 0xE8, 0x54, 0xD9                     ; 549D call 0x2df4
db 0x2C, 0xE8                           ; 54A0 sub al,0xe8
db 0x78, 0xCA                           ; 54A2 js 0x546e
db 0x3C, 0x20                           ; 54A4 cmp al,0x20
db 0x73, 0xD9                           ; 54A6 jnc 0x5481
db 0x59                                 ; 54A8 pop cx
db 0x8A, 0xC8                           ; 54A9 mov cl,al
db 0x51                                 ; 54AB push cx
db 0x59                                 ; 54AC pop cx
db 0x51                                 ; 54AD push cx
db 0x80, 0xE5, 0x0F                     ; 54AE and ch,0xf
db 0x89, 0x0E, 0x68, 0x00               ; 54B1 mov [0x68],cx
db 0x59                                 ; 54B5 pop cx
db 0xB4, 0x01                           ; 54B6 mov ah,0x1
db 0xCD, 0x10                           ; 54B8 int 0x10
db 0x5A                                 ; 54BA pop dx
db 0x89, 0x16, 0x56, 0x00               ; 54BB mov [0x56],dx
db 0x86, 0xF2                           ; 54BF xchg dh,dl
db 0xFE, 0xCE                           ; 54C1 dec dh
db 0xFE, 0xCA                           ; 54C3 dec dl
db 0x53                                 ; 54C5 push bx
db 0x8A, 0x3E, 0x49, 0x00               ; 54C6 mov bh,[0x49]
db 0xB4, 0x02                           ; 54CA mov ah,0x2
db 0xCD, 0x10                           ; 54CC int 0x10
db 0x5B                                 ; 54CE pop bx
db 0xC3                                 ; 54CF ret
db 0x50                                 ; 54D0 push ax
db 0xB0, 0x00                           ; 54D1 mov al,0x0
db 0xEB, 0x03                           ; 54D3 jmp short 0x54d8
db 0x50                                 ; 54D5 push ax
db 0xB0, 0x20                           ; 54D6 mov al,0x20
db 0x9C                                 ; 54D8 pushf
db 0x51                                 ; 54D9 push cx
db 0x53                                 ; 54DA push bx
db 0x50                                 ; 54DB push ax
db 0xE8, 0x3D, 0xFA                     ; 54DC call 0x4f1c
db 0x58                                 ; 54DF pop ax
db 0x5B                                 ; 54E0 pop bx
db 0x8B, 0x0E, 0x68, 0x00               ; 54E1 mov cx,[0x68]
db 0xF6, 0x06, 0x72, 0x00, 0xFF         ; 54E5 test byte [0x72],0xff
db 0x74, 0x02                           ; 54EA jz 0x54ee
db 0xB5, 0x04                           ; 54EC mov ch,0x4
db 0x0A, 0xE8                           ; 54EE or ch,al
db 0xB4, 0x01                           ; 54F0 mov ah,0x1
db 0xCD, 0x10                           ; 54F2 int 0x10
db 0x59                                 ; 54F4 pop cx
db 0x9D                                 ; 54F5 popf
db 0x58                                 ; 54F6 pop ax
db 0xC3                                 ; 54F7 ret
db 0x00, 0x00                           ; 54F8 add [bx+si],al
db 0x00, 0x00                           ; 54FA add [bx+si],al
db 0xFF                                 ; 54FC db 0xFF
db 0xFF                                 ; 54FD db 0xFF
db 0xFF                                 ; 54FE db 0xFF
db 0xFF                                 ; 54FF db 0xFF
db 0xFF                                 ; 5500 db 0xFF
db 0xFF                                 ; 5501 db 0xFF
db 0xFF                                 ; 5502 db 0xFF
db 0xFF                                 ; 5503 db 0xFF
db 0xFF                                 ; 5504 db 0xFF
db 0xFF                                 ; 5505 db 0xFF
db 0xFF                                 ; 5506 db 0xFF
db 0xFF, 0x50, 0x9C                     ; 5507 call near [bx+si-0x64]
db 0xE8, 0xC9, 0xFB                     ; 550A call 0x50d6
db 0x74, 0x4B                           ; 550D jz 0x555a
db 0x53                                 ; 550F push bx
db 0x51                                 ; 5510 push cx
db 0x52                                 ; 5511 push dx
db 0x8C, 0xC6                           ; 5512 mov si,es
db 0xBF, 0x00, 0x00                     ; 5514 mov di,0x0
db 0x8E, 0xC7                           ; 5517 mov es,di
db 0x26, 0xFF, 0x36, 0x7C, 0x00         ; 5519 push word [es:0x7c]
db 0x26, 0xFF, 0x36, 0x7E, 0x00         ; 551E push word [es:0x7e]
db 0x26, 0xC7, 0x06, 0x7C, 0x00, 0xF8, 0x54     ; 5523 mov word [es:0x7c],0x54f8
db 0x26, 0x8C, 0x0E, 0x7E, 0x00         ; 552A mov [es:0x7e],cs
db 0x8E, 0xC6                           ; 552F mov es,si
db 0xB0, 0x81                           ; 5531 mov al,0x81
db 0x02, 0x06, 0x72, 0x00               ; 5533 add al,[0x72]
db 0xB3, 0x83                           ; 5537 mov bl,0x83
db 0x8A, 0x3E, 0x49, 0x00               ; 5539 mov bh,[0x49]
db 0xB9, 0x01, 0x00                     ; 553D mov cx,0x1
db 0xB4, 0x09                           ; 5540 mov ah,0x9
db 0xCD, 0x10                           ; 5542 int 0x10
db 0x8C, 0xC6                           ; 5544 mov si,es
db 0xBF, 0x00, 0x00                     ; 5546 mov di,0x0
db 0x8E, 0xC7                           ; 5549 mov es,di
db 0x26, 0x8F, 0x06, 0x7E, 0x00         ; 554B pop word [es:0x7e]
db 0x26, 0x8F, 0x06, 0x7C, 0x00         ; 5550 pop word [es:0x7c]
db 0x8E, 0xC6                           ; 5555 mov es,si
db 0x5A                                 ; 5557 pop dx
db 0x59                                 ; 5558 pop cx
db 0x5B                                 ; 5559 pop bx
db 0x9D                                 ; 555A popf
db 0x58                                 ; 555B pop ax
db 0xC3                                 ; 555C ret
db 0xE8, 0xBD, 0xB9                     ; 555D call 0xf1d
db 0xA0, 0x56, 0x00                     ; 5560 mov al,[0x56]
db 0xE9, 0x72, 0xF6                     ; 5563 jmp 0x4bd8
db 0xE8, 0xB4, 0xB9                     ; 5566 call 0xf1d
db 0xE8, 0x65, 0x00                     ; 5569 call 0x55d1
db 0x0A, 0xEE                           ; 556C or ch,dh
db 0x75, 0x56                           ; 556E jnz 0x55c6
db 0x0A, 0xEA                           ; 5570 or ch,dl
db 0x0A, 0xE9                           ; 5572 or ch,cl
db 0x74, 0x50                           ; 5574 jz 0x55c6
db 0x8A, 0x26, 0x29, 0x00               ; 5576 mov ah,[0x29]
db 0x3A, 0xE2                           ; 557A cmp ah,dl
db 0x72, 0x48                           ; 557C jc 0x55c6
db 0x80, 0xF9, 0x1A                     ; 557E cmp cl,0x1a
db 0x73, 0x43                           ; 5581 jnc 0x55c6
db 0xA0, 0x71, 0x00                     ; 5583 mov al,[0x71]
db 0x0A, 0xC0                           ; 5586 or al,al
db 0x74, 0x05                           ; 5588 jz 0x558f
db 0x80, 0xF9, 0x19                     ; 558A cmp cl,0x19
db 0x73, 0x37                           ; 558D jnc 0x55c6
db 0x53                                 ; 558F push bx
db 0x8A, 0xF1                           ; 5590 mov dh,cl
db 0xFE, 0xCE                           ; 5592 dec dh
db 0xFE, 0xCA                           ; 5594 dec dl
db 0x8A, 0x3E, 0x49, 0x00               ; 5596 mov bh,[0x49]
db 0xB4, 0x02                           ; 559A mov ah,0x2
db 0xCD, 0x10                           ; 559C int 0x10
db 0xB4, 0x08                           ; 559E mov ah,0x8
db 0xCD, 0x10                           ; 55A0 int 0x10
db 0x5B                                 ; 55A2 pop bx
db 0x50                                 ; 55A3 push ax
db 0xE8, 0x77, 0xB9                     ; 55A4 call 0xf1e
db 0x3C, 0x2C                           ; 55A7 cmp al,0x2c
db 0x74, 0x04                           ; 55A9 jz 0x55af
db 0xB0, 0x00                           ; 55AB mov al,0x0
db 0xEB, 0x07                           ; 55AD jmp short 0x55b6
db 0xE8, 0x42, 0xD8                     ; 55AF call 0x2df4
db 0x2C, 0xE8                           ; 55B2 sub al,0xe8
db 0x66, 0xC9                           ; 55B4 o32 leave
db 0x50                                 ; 55B6 push ax
db 0xE8, 0x3A, 0xD8                     ; 55B7 call 0x2df4
db 0x29, 0x58, 0x0A                     ; 55BA sub [bx+si+0xa],bx
db 0xC0, 0x58, 0x74, 0x02               ; 55BD rcr byte [bx+si+0x74],0x2
db 0x8A, 0xC4                           ; 55C1 mov al,ah
db 0xE9, 0x12, 0xF6                     ; 55C3 jmp 0x4bd8
db 0xE9, 0x90, 0xBA                     ; 55C6 jmp 0x1059
db 0xE8, 0x05, 0x00                     ; 55C9 call 0x55d1
db 0xE8, 0x25, 0xD8                     ; 55CC call 0x2df4
db 0x29, 0xC3                           ; 55CF sub bx,ax
db 0xE8, 0x20, 0xD8                     ; 55D1 call 0x2df4
db 0x28, 0xE8                           ; 55D4 sub al,ch
db 0x32, 0xC9                           ; 55D6 xor cl,cl
db 0x52                                 ; 55D8 push dx
db 0xE8, 0x18, 0xD8                     ; 55D9 call 0x2df4
db 0x2C, 0xE8                           ; 55DC sub al,0xe8
db 0x2A, 0xC9                           ; 55DE sub cl,cl
db 0x59                                 ; 55E0 pop cx
db 0xC3                                 ; 55E1 ret
db 0xE8, 0x38, 0xB9                     ; 55E2 call 0xf1d
db 0x3C, 0x95                           ; 55E5 cmp al,0x95
db 0x74, 0x08                           ; 55E7 jz 0x55f1
db 0xE8, 0x08, 0xD8                     ; 55E9 call 0x2df4
db 0xDD, 0x32                           ; 55EC fnsave [bp+si]
db 0xC0, 0xEB, 0x05                     ; 55EE shr bl,0x5
db 0xE8, 0x29, 0xB9                     ; 55F1 call 0xf1d
db 0xB0, 0xFF                           ; 55F4 mov al,0xff
db 0xA2, 0x34, 0x00                     ; 55F6 mov [0x34],al
db 0xC3                                 ; 55F9 ret
db 0xA0, 0x34, 0x00                     ; 55FA mov al,[0x34]
db 0x0A, 0xC0                           ; 55FD or al,al
db 0x74, 0x30                           ; 55FF jz 0x5631
db 0xE8, 0x1B, 0xC9                     ; 5601 call 0x1f1f
db 0x3C, 0x0A                           ; 5604 cmp al,0xa
db 0x73, 0x29                           ; 5606 jnc 0x5631
db 0x53                                 ; 5608 push bx
db 0x56                                 ; 5609 push si
db 0xBA, 0x1A, 0x56                     ; 560A mov dx,0x561a
db 0x52                                 ; 560D push dx
db 0x32, 0xE4                           ; 560E xor ah,ah
db 0xD1, 0xE0                           ; 5610 shl ax,1
db 0x8B, 0xF0                           ; 5612 mov si,ax
db 0x2E, 0xFF, 0xB4, 0x1D, 0x56         ; 5614 push word [cs:si+0x561d]
db 0xC3                                 ; 5619 ret
db 0x5E                                 ; 561A pop si
db 0x5B                                 ; 561B pop bx
db 0xC3                                 ; 561C ret
db 0x34, 0x56                           ; 561D xor al,0x56
db 0x3F                                 ; 561F aas
db 0x56                                 ; 5620 push si
db 0x46                                 ; 5621 inc si
db 0x56                                 ; 5622 push si
db 0x4C                                 ; 5623 dec sp
db 0x56                                 ; 5624 push si
db 0x6B, 0x56, 0x72, 0x56               ; 5625 imul dx,[bp+0x72],byte +0x56
db 0x78, 0x56                           ; 5629 js 0x5681
db 0x80, 0x56, 0x88, 0x56               ; 562B adc byte [bp-0x78],0x56
db 0x90                                 ; 562F nop
db 0x56                                 ; 5630 push si
db 0xE9, 0x25, 0xBA                     ; 5631 jmp 0x1059
db 0xBB, 0x35, 0x00                     ; 5634 mov bx,0x35
db 0x8A, 0x07                           ; 5637 mov al,[bx]
db 0xC6, 0x07, 0x00                     ; 5639 mov byte [bx],0x0
db 0xE9, 0x9F, 0xF5                     ; 563C jmp 0x4bde
db 0x8B, 0x1E, 0x37, 0x00               ; 563F mov bx,[0x37]
db 0xE9, 0xC6, 0x0E                     ; 5643 jmp 0x650c
db 0xA0, 0x39, 0x00                     ; 5646 mov al,[0x39]
db 0xE9, 0x8C, 0xF5                     ; 5649 jmp 0x4bd8
db 0xB4, 0x04                           ; 564C mov ah,0x4
db 0xCD, 0x10                           ; 564E int 0x10
db 0x50                                 ; 5650 push ax
db 0x0A, 0xE4                           ; 5651 or ah,ah
db 0x74, 0x0C                           ; 5653 jz 0x5661
db 0x89, 0x1E, 0x3A, 0x00               ; 5655 mov [0x3a],bx
db 0x88, 0x2E, 0x3C, 0x00               ; 5659 mov [0x3c],ch
db 0x89, 0x16, 0x3F, 0x00               ; 565D mov [0x3f],dx
db 0x58                                 ; 5661 pop ax
db 0x8A, 0xC4                           ; 5662 mov al,ah
db 0xFE, 0xC8                           ; 5664 dec al
db 0xF6, 0xD0                           ; 5666 not al
db 0xE9, 0x73, 0xF5                     ; 5668 jmp 0x4bde
db 0x8B, 0x1E, 0x3A, 0x00               ; 566B mov bx,[0x3a]
db 0xE9, 0x9A, 0x0E                     ; 566F jmp 0x650c
db 0xA0, 0x3C, 0x00                     ; 5672 mov al,[0x3c]
db 0xE9, 0x60, 0xF5                     ; 5675 jmp 0x4bd8
db 0xA0, 0x3E, 0x00                     ; 5678 mov al,[0x3e]
db 0xFE, 0xC0                           ; 567B inc al
db 0xE9, 0x58, 0xF5                     ; 567D jmp 0x4bd8
db 0xA0, 0x3D, 0x00                     ; 5680 mov al,[0x3d]
db 0xFE, 0xC0                           ; 5683 inc al
db 0xE9, 0x50, 0xF5                     ; 5685 jmp 0x4bd8
db 0xA0, 0x40, 0x00                     ; 5688 mov al,[0x40]
db 0xFE, 0xC0                           ; 568B inc al
db 0xE9, 0x48, 0xF5                     ; 568D jmp 0x4bd8
db 0xA0, 0x3F, 0x00                     ; 5690 mov al,[0x3f]
db 0xFE, 0xC0                           ; 5693 inc al
db 0xE9, 0x40, 0xF5                     ; 5695 jmp 0x4bd8
db 0xE8, 0x84, 0xC8                     ; 5698 call 0x1f1f
db 0x0A, 0xC0                           ; 569B or al,al
db 0x74, 0x12                           ; 569D jz 0x56b1
db 0x3C, 0x04                           ; 569F cmp al,0x4
db 0x73, 0x54                           ; 56A1 jnc 0x56f7
db 0xB4, 0x00                           ; 56A3 mov ah,0x0
db 0x53                                 ; 56A5 push bx
db 0xBB, 0x41, 0x00                     ; 56A6 mov bx,0x41
db 0x03, 0xD8                           ; 56A9 add bx,ax
db 0x8A, 0x07                           ; 56AB mov al,[bx]
db 0x5B                                 ; 56AD pop bx
db 0xE9, 0x27, 0xF5                     ; 56AE jmp 0x4bd8
db 0x53                                 ; 56B1 push bx
db 0xBA, 0x01, 0x02                     ; 56B2 mov dx,0x201
db 0xB9, 0x01, 0x01                     ; 56B5 mov cx,0x101
db 0xBB, 0x0F, 0x00                     ; 56B8 mov bx,0xf
db 0xFA                                 ; 56BB cli
db 0xEE                                 ; 56BC out dx,al
db 0xEC                                 ; 56BD in al,dx
db 0x24, 0x0F                           ; 56BE and al,0xf
db 0x3A, 0xC3                           ; 56C0 cmp al,bl
db 0xE1, 0xF9                           ; 56C2 loope 0x56bd
db 0xE3, 0x0B                           ; 56C4 jcxz 0x56d1
db 0x32, 0xC3                           ; 56C6 xor al,bl
db 0x8A, 0xE1                           ; 56C8 mov ah,cl
db 0x50                                 ; 56CA push ax
db 0xFE, 0xC7                           ; 56CB inc bh
db 0x32, 0xD8                           ; 56CD xor bl,al
db 0xEB, 0xEC                           ; 56CF jmp short 0x56bd
db 0x0A, 0xFF                           ; 56D1 or bh,bh
db 0x74, 0x1A                           ; 56D3 jz 0x56ef
db 0x8A, 0xD7                           ; 56D5 mov dl,bh
db 0xBB, 0x41, 0x00                     ; 56D7 mov bx,0x41
db 0xB9, 0x04, 0x00                     ; 56DA mov cx,0x4
db 0x58                                 ; 56DD pop ax
db 0xF6, 0xD4                           ; 56DE not ah
db 0x02, 0xE2                           ; 56E0 add ah,dl
db 0xD0, 0xE8                           ; 56E2 shr al,1
db 0x73, 0x02                           ; 56E4 jnc 0x56e8
db 0x88, 0x27                           ; 56E6 mov [bx],ah
db 0x43                                 ; 56E8 inc bx
db 0xE2, 0xF7                           ; 56E9 loop 0x56e2
db 0xFE, 0xCA                           ; 56EB dec dl
db 0x75, 0xE8                           ; 56ED jnz 0x56d7
db 0xFB                                 ; 56EF sti
db 0x5B                                 ; 56F0 pop bx
db 0xA0, 0x41, 0x00                     ; 56F1 mov al,[0x41]
db 0xE9, 0xE1, 0xF4                     ; 56F4 jmp 0x4bd8
db 0xE9, 0x5F, 0xB9                     ; 56F7 jmp 0x1059
db 0xE8, 0x20, 0xB8                     ; 56FA call 0xf1d
db 0x3C, 0x95                           ; 56FD cmp al,0x95
db 0x74, 0x08                           ; 56FF jz 0x5709
db 0xE8, 0xF0, 0xD6                     ; 5701 call 0x2df4
db 0xDD, 0x32                           ; 5704 fnsave [bp+si]
db 0xC0, 0xEB, 0x05                     ; 5706 shr bl,0x5
db 0xE8, 0x11, 0xB8                     ; 5709 call 0xf1d
db 0xB0, 0xFF                           ; 570C mov al,0xff
db 0xA2, 0x45, 0x00                     ; 570E mov [0x45],al
db 0xC3                                 ; 5711 ret
db 0xA0, 0x45, 0x00                     ; 5712 mov al,[0x45]
db 0x0A, 0xC0                           ; 5715 or al,al
db 0x74, 0xDE                           ; 5717 jz 0x56f7
db 0xE8, 0x03, 0xC8                     ; 5719 call 0x1f1f
db 0x3C, 0x04                           ; 571C cmp al,0x4
db 0x73, 0xD7                           ; 571E jnc 0x56f7
db 0xA8, 0x01                           ; 5720 test al,0x1
db 0x74, 0x0E                           ; 5722 jz 0x5732
db 0xB4, 0x10                           ; 5724 mov ah,0x10
db 0xFE, 0xC8                           ; 5726 dec al
db 0x74, 0x02                           ; 5728 jz 0x572c
db 0xB4, 0x40                           ; 572A mov ah,0x40
db 0xE8, 0xD0, 0x00                     ; 572C call 0x57ff
db 0xE9, 0xAC, 0xF4                     ; 572F jmp 0x4bde
db 0x53                                 ; 5732 push bx
db 0xBB, 0x46, 0x00                     ; 5733 mov bx,0x46
db 0x0A, 0xC0                           ; 5736 or al,al
db 0x74, 0x01                           ; 5738 jz 0x573b
db 0x43                                 ; 573A inc bx
db 0x8A, 0x07                           ; 573B mov al,[bx]
db 0xC6, 0x07, 0x00                     ; 573D mov byte [bx],0x0
db 0x5B                                 ; 5740 pop bx
db 0xE9, 0x9A, 0xF4                     ; 5741 jmp 0x4bde

L_5744:
db 0x9C                                 ; 5744 pushf
db 0x50                                 ; 5745 push ax
db 0x55                                 ; 5746 push bp
db 0x56                                 ; 5747 push si
db 0x57                                 ; 5748 push di
db 0x1E                                 ; 5749 push ds
db 0xBA, 0x00, 0x00                     ; 574A mov dx,0x0
db 0x8E, 0xDA                           ; 574D mov ds,dx
db 0x8E, 0x1E, 0x10, 0x05               ; 574F mov ds,[0x510]
db 0xA1, 0x66, 0x00                     ; 5753 mov ax,[0x66]
db 0x0A, 0xC4                           ; 5756 or al,ah
db 0x74, 0x09                           ; 5758 jz 0x5763
db 0xFF, 0x0E, 0x66, 0x00               ; 575A dec word [0x66]
db 0x75, 0x03                           ; 575E jnz 0x5763
db 0xE8, 0x27, 0x00                     ; 5760 call 0x578a
db 0xA0, 0x34, 0x00                     ; 5763 mov al,[0x34]
db 0x0A, 0xC0                           ; 5766 or al,al
db 0x74, 0x03                           ; 5768 jz 0x576d
db 0xE8, 0x36, 0x00                     ; 576A call 0x57a3
db 0xA0, 0x45, 0x00                     ; 576D mov al,[0x45]
db 0x0A, 0xC0                           ; 5770 or al,al
db 0x74, 0x03                           ; 5772 jz 0x5777
db 0xE8, 0x69, 0x00                     ; 5774 call 0x57e0
db 0x1F                                 ; 5777 pop ds
db 0x5F                                 ; 5778 pop di
db 0x5E                                 ; 5779 pop si
db 0x5D                                 ; 577A pop bp
db 0x58                                 ; 577B pop ax
db 0x9D                                 ; 577C popf
db 0xCF                                 ; 577D iret
db 0xC6, 0x06, 0x65, 0x00, 0x00         ; 577E mov byte [0x65],0x0
db 0xA1, 0x66, 0x00                     ; 5783 mov ax,[0x66]
db 0x0B, 0xC0                           ; 5786 or ax,ax
db 0x74, 0x18                           ; 5788 jz 0x57a2
db 0x52                                 ; 578A push dx
db 0xFA                                 ; 578B cli
db 0xF6, 0x06, 0x65, 0x00, 0xFF         ; 578C test byte [0x65],0xff
db 0x75, 0x07                           ; 5791 jnz 0x579a
db 0xBA, 0x61, 0x00                     ; 5793 mov dx,0x61
db 0xEC                                 ; 5796 in al,dx
db 0x24, 0xFC                           ; 5797 and al,0xfc
db 0xEE                                 ; 5799 out dx,al
db 0xC7, 0x06, 0x66, 0x00, 0x00, 0x00   ; 579A mov word [0x66],0x0
db 0xFB                                 ; 57A0 sti
db 0x5A                                 ; 57A1 pop dx
db 0xC3                                 ; 57A2 ret
db 0x53                                 ; 57A3 push bx
db 0x51                                 ; 57A4 push cx
db 0x52                                 ; 57A5 push dx
db 0xB4, 0x04                           ; 57A6 mov ah,0x4
db 0xCD, 0x10                           ; 57A8 int 0x10
db 0x50                                 ; 57AA push ax
db 0x0A, 0xE4                           ; 57AB or ah,ah
db 0x74, 0x0C                           ; 57AD jz 0x57bb
db 0x89, 0x1E, 0x3A, 0x00               ; 57AF mov [0x3a],bx
db 0x88, 0x2E, 0x3C, 0x00               ; 57B3 mov [0x3c],ch
db 0x89, 0x16, 0x3F, 0x00               ; 57B7 mov [0x3f],dx
db 0x58                                 ; 57BB pop ax
db 0xA0, 0x36, 0x00                     ; 57BC mov al,[0x36]
db 0x32, 0xC4                           ; 57BF xor al,ah
db 0x74, 0x19                           ; 57C1 jz 0x57dc
db 0x0A, 0xE4                           ; 57C3 or ah,ah
db 0x88, 0x26, 0x36, 0x00               ; 57C5 mov [0x36],ah
db 0x74, 0x11                           ; 57C9 jz 0x57dc
db 0x89, 0x1E, 0x37, 0x00               ; 57CB mov [0x37],bx
db 0x88, 0x2E, 0x39, 0x00               ; 57CF mov [0x39],ch
db 0x89, 0x16, 0x3D, 0x00               ; 57D3 mov [0x3d],dx
db 0xB0, 0xFF                           ; 57D7 mov al,0xff
db 0xA2, 0x35, 0x00                     ; 57D9 mov [0x35],al
db 0x5A                                 ; 57DC pop dx
db 0x59                                 ; 57DD pop cx
db 0x5B                                 ; 57DE pop bx
db 0xC3                                 ; 57DF ret
db 0x53                                 ; 57E0 push bx
db 0xBB, 0x46, 0x00                     ; 57E1 mov bx,0x46
db 0x80, 0x3F, 0x00                     ; 57E4 cmp byte [bx],0x0
db 0x75, 0x07                           ; 57E7 jnz 0x57f0
db 0xB4, 0x10                           ; 57E9 mov ah,0x10
db 0xE8, 0x11, 0x00                     ; 57EB call 0x57ff
db 0x88, 0x07                           ; 57EE mov [bx],al
db 0x43                                 ; 57F0 inc bx
db 0x80, 0x3F, 0x00                     ; 57F1 cmp byte [bx],0x0
db 0x75, 0x07                           ; 57F4 jnz 0x57fd
db 0xB4, 0x40                           ; 57F6 mov ah,0x40
db 0xE8, 0x04, 0x00                     ; 57F8 call 0x57ff
db 0x88, 0x07                           ; 57FB mov [bx],al
db 0x5B                                 ; 57FD pop bx
db 0xC3                                 ; 57FE ret
db 0x52                                 ; 57FF push dx
db 0xBA, 0x01, 0x02                     ; 5800 mov dx,0x201
db 0xEC                                 ; 5803 in al,dx
db 0x22, 0xC4                           ; 5804 and al,ah
db 0xFE, 0xC8                           ; 5806 dec al
db 0x98                                 ; 5808 cbw
db 0x8A, 0xC4                           ; 5809 mov al,ah
db 0x5A                                 ; 580B pop dx
db 0xC3                                 ; 580C ret
db 0xE8, 0x09, 0x00                     ; 580D call 0x5819
db 0xB8, 0xD3, 0x05                     ; 5810 mov ax,0x5d3
db 0xBA, 0x04, 0x00                     ; 5813 mov dx,0x4
db 0x52                                 ; 5816 push dx
db 0xEB, 0x38                           ; 5817 jmp short 0x5851
db 0x8B, 0x16, 0x66, 0x00               ; 5819 mov dx,[0x66]
db 0x0A, 0xF2                           ; 581D or dh,dl
db 0x74, 0x07                           ; 581F jz 0x5828
db 0xC6, 0x06, 0x65, 0x00, 0xFF         ; 5821 mov byte [0x65],0xff
db 0xEB, 0xF1                           ; 5826 jmp short 0x5819
db 0xC3                                 ; 5828 ret
db 0xE8, 0xDE, 0xC6                     ; 5829 call 0x1f0a
db 0x83, 0xFA, 0x25                     ; 582C cmp dx,byte +0x25
db 0x72, 0x12                           ; 582F jc 0x5843
db 0x52                                 ; 5831 push dx
db 0xE8, 0xBF, 0xD5                     ; 5832 call 0x2df4
db 0x2C, 0xE8                           ; 5835 sub al,0xe8
db 0x71, 0xCA                           ; 5837 jno 0x5803
db 0x59                                 ; 5839 pop cx
db 0x52                                 ; 583A push dx
db 0x0B, 0xD2                           ; 583B or dx,dx
db 0x75, 0x07                           ; 583D jnz 0x5846
db 0x5A                                 ; 583F pop dx
db 0xE9, 0x3B, 0xFF                     ; 5840 jmp 0x577e
db 0xE9, 0x13, 0xB8                     ; 5843 jmp 0x1059
db 0xE8, 0xD0, 0xFF                     ; 5846 call 0x5819
db 0xBA, 0x12, 0x00                     ; 5849 mov dx,0x12
db 0xB8, 0xDC, 0x34                     ; 584C mov ax,0x34dc
db 0xF7, 0xF1                           ; 584F div cx
db 0xF6, 0x06, 0x65, 0x00, 0xFF         ; 5851 test byte [0x65],0xff
db 0x75, 0x08                           ; 5856 jnz 0x5860
db 0x50                                 ; 5858 push ax
db 0xBA, 0x43, 0x00                     ; 5859 mov dx,0x43
db 0xB0, 0xB6                           ; 585C mov al,0xb6
db 0xEE                                 ; 585E out dx,al
db 0x58                                 ; 585F pop ax
db 0xBA, 0x42, 0x00                     ; 5860 mov dx,0x42
db 0xEE                                 ; 5863 out dx,al
db 0x8A, 0xC4                           ; 5864 mov al,ah
db 0xEE                                 ; 5866 out dx,al
db 0x75, 0x07                           ; 5867 jnz 0x5870
db 0xBA, 0x61, 0x00                     ; 5869 mov dx,0x61
db 0xEC                                 ; 586C in al,dx
db 0x0C, 0x03                           ; 586D or al,0x3
db 0xEE                                 ; 586F out dx,al
db 0x5A                                 ; 5870 pop dx
db 0x89, 0x16, 0x66, 0x00               ; 5871 mov [0x66],dx
db 0xC6, 0x06, 0x65, 0x00, 0x00         ; 5875 mov byte [0x65],0x0
db 0xC3                                 ; 587A ret
db 0x0A, 0x5A, 0x4D                     ; 587B or bl,[bp+si+0x4d]
db 0x41                                 ; 587E inc cx
db 0x59                                 ; 587F pop cx
db 0x10, 0x59, 0x10                     ; 5880 adc [bx+di+0x10],bl
db 0x19, 0x5A, 0x59                     ; 5883 sbb [bp+si+0x59],bx
db 0x10, 0x59, 0x10                     ; 5886 adc [bx+di+0x10],bl
db 0x59                                 ; 5889 pop cx
db 0x10, 0x59, 0x10                     ; 588A adc [bx+di+0x10],bl
db 0x2D, 0x5A, 0x59                     ; 588D sub ax,0x595a
db 0x10, 0x34                           ; 5890 adc [si],dh
db 0x5A                                 ; 5892 pop dx
db 0x4D                                 ; 5893 dec bp
db 0x41                                 ; 5894 inc cx
db 0x59                                 ; 5895 pop cx
db 0x10, 0x45, 0x5A                     ; 5896 adc [di+0x5a],al
db 0x59                                 ; 5899 pop cx
db 0x10, 0x59, 0x10                     ; 589A adc [bx+di+0x10],bl
db 0x59                                 ; 589D pop cx
db 0x10, 0x59, 0x10                     ; 589E adc [bx+di+0x10],bl
db 0x59                                 ; 58A1 pop cx
db 0x10, 0x59, 0x10                     ; 58A2 adc [bx+di+0x10],bl
db 0x5D                                 ; 58A5 pop bp
db 0x5A                                 ; 58A6 pop dx
db 0x63, 0x5A, 0x4D                     ; 58A7 arpl [bp+si+0x4d],bx
db 0x41                                 ; 58AA inc cx
db 0x59                                 ; 58AB pop cx
db 0x10, 0x7C, 0x5A                     ; 58AC adc [si+0x5a],bh
db 0x59                                 ; 58AF pop cx
db 0x10, 0x59, 0x10                     ; 58B0 adc [bx+di+0x10],bl
db 0x59                                 ; 58B3 pop cx
db 0x10, 0x59, 0x10                     ; 58B4 adc [bx+di+0x10],bl
db 0x59                                 ; 58B7 pop cx
db 0x10, 0x59, 0x10                     ; 58B8 adc [bx+di+0x10],bl
db 0xAA                                 ; 58BB stosb
db 0x5A                                 ; 58BC pop dx
db 0xB1, 0x5A                           ; 58BD mov cl,0x5a
db 0x6E                                 ; 58BF outsb
db 0x5B                                 ; 58C0 pop bx
db 0x59                                 ; 58C1 pop cx
db 0x10, 0xAB, 0x5B, 0xF0               ; 58C2 adc [bp+di+0xf05b],ch
db 0x5B                                 ; 58C6 pop bx
db 0x59                                 ; 58C7 pop cx
db 0x10, 0x59, 0x10                     ; 58C8 adc [bx+di+0x10],bl
db 0x6C                                 ; 58CB insb
db 0x5C                                 ; 58CC pop sp
db 0x59                                 ; 58CD pop cx
db 0x10, 0x76, 0x5C                     ; 58CE adc [bp+0x5c],dh
db 0x59                                 ; 58D1 pop cx
db 0x10, 0x22                           ; 58D2 adc [bp+si],ah
db 0xC2, 0x75, 0x22                     ; 58D4 ret 0x2275
db 0x9C                                 ; 58D7 pushf
db 0x50                                 ; 58D8 push ax
db 0x53                                 ; 58D9 push bx
db 0x89, 0x1E, 0xE9, 0x04               ; 58DA mov [0x4e9],bx
db 0x88, 0x17                           ; 58DE mov [bx],dl
db 0x83, 0xC3, 0x2D                     ; 58E0 add bx,byte +0x2d
db 0xC6, 0x07, 0x00                     ; 58E3 mov byte [bx],0x0
db 0x43                                 ; 58E6 inc bx
db 0x43                                 ; 58E7 inc bx
db 0x88, 0x2F                           ; 58E8 mov [bx],ch
db 0x43                                 ; 58EA inc bx
db 0xC6, 0x07, 0x00                     ; 58EB mov byte [bx],0x0
db 0x43                                 ; 58EE inc bx
db 0x88, 0x0F                           ; 58EF mov [bx],cl
db 0x43                                 ; 58F1 inc bx
db 0xC6, 0x07, 0x00                     ; 58F2 mov byte [bx],0x0
db 0x5B                                 ; 58F5 pop bx
db 0x58                                 ; 58F6 pop ax
db 0x9D                                 ; 58F7 popf
db 0xC3                                 ; 58F8 ret
db 0xE9, 0x98, 0xAE                     ; 58F9 jmp 0x794
db 0x58                                 ; 58FC pop ax
db 0x5B                                 ; 58FD pop bx
db 0xC3                                 ; 58FE ret
db 0x80, 0xFA, 0x80                     ; 58FF cmp dl,0x80
db 0x75, 0x02                           ; 5902 jnz 0x5906
db 0xB2, 0x02                           ; 5904 mov dl,0x2
db 0xC3                                 ; 5906 ret
db 0x58                                 ; 5907 pop ax
db 0x86, 0xC4                           ; 5908 xchg al,ah
db 0x9E                                 ; 590A sahf
db 0x59                                 ; 590B pop cx
db 0x5A                                 ; 590C pop dx
db 0x5B                                 ; 590D pop bx
db 0xC3                                 ; 590E ret
db 0x5A                                 ; 590F pop dx
db 0x5B                                 ; 5910 pop bx
db 0x59                                 ; 5911 pop cx
db 0xC3                                 ; 5912 ret
db 0x83, 0xC3, 0x2E                     ; 5913 add bx,byte +0x2e
db 0x8A, 0x07                           ; 5916 mov al,[bx]
db 0xF6, 0xD0                           ; 5918 not al
db 0xC3                                 ; 591A ret
db 0x8B, 0x1E, 0xE9, 0x04               ; 591B mov bx,[0x4e9]
db 0x83, 0xC3, 0x2B                     ; 591F add bx,byte +0x2b
db 0xC3                                 ; 5922 ret
db 0x8B, 0x1E, 0xE9, 0x04               ; 5923 mov bx,[0x4e9]
db 0x83, 0xC3, 0x32                     ; 5927 add bx,byte +0x32
db 0xC3                                 ; 592A ret
db 0x8B, 0x1E, 0xE9, 0x04               ; 592B mov bx,[0x4e9]
db 0x8A, 0x87, 0x2F, 0x00               ; 592F mov al,[bx+0x2f]
db 0xC3                                 ; 5933 ret
db 0x56                                 ; 5934 push si
db 0x57                                 ; 5935 push di
db 0x51                                 ; 5936 push cx
db 0xC6, 0x06, 0x3F, 0x05, 0xA5         ; 5937 mov byte [0x53f],0xa5
db 0xBE, 0xF0, 0x04                     ; 593C mov si,0x4f0
db 0xBF, 0x40, 0x05                     ; 593F mov di,0x540
db 0xB9, 0x08, 0x00                     ; 5942 mov cx,0x8
db 0xFC                                 ; 5945 cld
db 0xA4                                 ; 5946 movsb
db 0xE2, 0xFD                           ; 5947 loop 0x5946
db 0x59                                 ; 5949 pop cx
db 0x5F                                 ; 594A pop di
db 0x5E                                 ; 594B pop si
db 0xC3                                 ; 594C ret
db 0x53                                 ; 594D push bx
db 0x51                                 ; 594E push cx
db 0xBB, 0x40, 0x05                     ; 594F mov bx,0x540
db 0xB1, 0x08                           ; 5952 mov cl,0x8
db 0x80, 0x3F, 0x20                     ; 5954 cmp byte [bx],0x20
db 0x75, 0x0D                           ; 5957 jnz 0x5966
db 0x43                                 ; 5959 inc bx
db 0xFE, 0xC9                           ; 595A dec cl
db 0x75, 0xF6                           ; 595C jnz 0x5954
db 0xBE, 0x5C, 0x05                     ; 595E mov si,0x55c
db 0xBF, 0x48, 0x05                     ; 5961 mov di,0x548
db 0xEB, 0x10                           ; 5964 jmp short 0x5976
db 0xBE, 0x54, 0x05                     ; 5966 mov si,0x554
db 0xBF, 0x40, 0x05                     ; 5969 mov di,0x540
db 0xB1, 0x08                           ; 596C mov cl,0x8
db 0xFC                                 ; 596E cld
db 0xA6                                 ; 596F cmpsb
db 0x75, 0x27                           ; 5970 jnz 0x5999
db 0xFE, 0xC9                           ; 5972 dec cl
db 0x75, 0xF9                           ; 5974 jnz 0x596f
db 0x8A, 0x05                           ; 5976 mov al,[di]
db 0x3A, 0x04                           ; 5978 cmp al,[si]
db 0x74, 0x09                           ; 597A jz 0x5985
db 0x0A, 0xC0                           ; 597C or al,al
db 0x75, 0x19                           ; 597E jnz 0x5999
db 0xF6, 0x04, 0x01                     ; 5980 test byte [si],0x1
db 0x75, 0x14                           ; 5983 jnz 0x5999
db 0x8A, 0x04                           ; 5985 mov al,[si]
db 0x8B, 0x1E, 0xE9, 0x04               ; 5987 mov bx,[0x4e9]
db 0x88, 0x87, 0x31, 0x00               ; 598B mov [bx+0x31],al
db 0xBB, 0xF4, 0x59                     ; 598F mov bx,0x59f4
db 0xE8, 0x0E, 0x00                     ; 5992 call 0x59a3
db 0x32, 0xC0                           ; 5995 xor al,al
db 0xEB, 0x07                           ; 5997 jmp short 0x59a0
db 0xBB, 0xFE, 0x59                     ; 5999 mov bx,0x59fe
db 0xE8, 0x04, 0x00                     ; 599C call 0x59a3
db 0xF9                                 ; 599F stc
db 0x59                                 ; 59A0 pop cx
db 0x5B                                 ; 59A1 pop bx
db 0xC3                                 ; 59A2 ret
db 0x53                                 ; 59A3 push bx
db 0x8B, 0x1E, 0x2E, 0x00               ; 59A4 mov bx,[0x2e]
db 0x43                                 ; 59A8 inc bx
db 0x0B, 0xDB                           ; 59A9 or bx,bx
db 0x74, 0x02                           ; 59AB jz 0x59af
db 0x5B                                 ; 59AD pop bx
db 0xC3                                 ; 59AE ret
db 0xBB, 0x53, 0x05                     ; 59AF mov bx,0x553
db 0x53                                 ; 59B2 push bx
db 0x43                                 ; 59B3 inc bx
db 0xB1, 0x08                           ; 59B4 mov cl,0x8
db 0x8A, 0x07                           ; 59B6 mov al,[bx]
db 0xE8, 0xD5, 0xF4                     ; 59B8 call 0x4e90
db 0x43                                 ; 59BB inc bx
db 0xFE, 0xC9                           ; 59BC dec cl
db 0x75, 0xF6                           ; 59BE jnz 0x59b6
db 0xB0, 0x2E                           ; 59C0 mov al,0x2e
db 0xE8, 0xCB, 0xF4                     ; 59C2 call 0x4e90
db 0x5B                                 ; 59C5 pop bx
db 0x83, 0xC3, 0x09                     ; 59C6 add bx,byte +0x9
db 0xB0, 0x44                           ; 59C9 mov al,0x44
db 0xF6, 0x07, 0xE1                     ; 59CB test byte [bx],0xe1
db 0x74, 0x17                           ; 59CE jz 0x59e7
db 0xB0, 0x50                           ; 59D0 mov al,0x50
db 0xF6, 0x07, 0x20                     ; 59D2 test byte [bx],0x20
db 0x75, 0x10                           ; 59D5 jnz 0x59e7
db 0xB0, 0x42                           ; 59D7 mov al,0x42
db 0xF6, 0x07, 0x80                     ; 59D9 test byte [bx],0x80
db 0x75, 0x09                           ; 59DC jnz 0x59e7
db 0xB0, 0x41                           ; 59DE mov al,0x41
db 0xF6, 0x07, 0x40                     ; 59E0 test byte [bx],0x40
db 0x75, 0x02                           ; 59E3 jnz 0x59e7
db 0xB0, 0x4D                           ; 59E5 mov al,0x4d
db 0x5B                                 ; 59E7 pop bx
db 0xE8, 0xA5, 0xF4                     ; 59E8 call 0x4e90
db 0x2E, 0x8A, 0x07                     ; 59EB mov al,[cs:bx]
db 0x43                                 ; 59EE inc bx
db 0x0A, 0xC0                           ; 59EF or al,al
db 0x75, 0xF5                           ; 59F1 jnz 0x59e8
db 0xC3                                 ; 59F3 ret
db 0x20, 0x46, 0x6F                     ; 59F4 and [bp+0x6f],al
db 0x75, 0x6E                           ; 59F7 jnz 0x5a67
db 0x64, 0x2E, 0xFF, 0x0D               ; 59F9 dec word [cs:di]
db 0x00, 0x20                           ; 59FD add [bx+si],ah
db 0x53                                 ; 59FF push bx
db 0x6B, 0x69, 0x70, 0x70               ; 5A00 imul bp,[bx+di+0x70],byte +0x70
db 0x65, 0x64, 0x2E, 0xFF, 0x0D         ; 5A04 dec word [cs:di]
db 0x00, 0xB9, 0x00, 0x00               ; 5A09 add [bx+di+0x0],bh
db 0x88, 0x0E, 0x52, 0x05               ; 5A0D mov [0x552],cl
db 0xB0, 0xEA                           ; 5A11 mov al,0xea
db 0xE8, 0xBD, 0xFE                     ; 5A13 call 0x58d3
db 0xE9, 0xE3, 0xFE                     ; 5A16 jmp 0x58fc
db 0xBB, 0x52, 0x05                     ; 5A19 mov bx,0x552
db 0x8A, 0x07                           ; 5A1C mov al,[bx]
db 0xC6, 0x07, 0x00                     ; 5A1E mov byte [bx],0x0
db 0x0A, 0xC0                           ; 5A21 or al,al
db 0x75, 0x05                           ; 5A23 jnz 0x5a2a
db 0xE8, 0x46, 0xF3                     ; 5A25 call 0x4d6e
db 0x0A, 0xC0                           ; 5A28 or al,al
db 0xE9, 0xE2, 0xFE                     ; 5A2A jmp 0x590f
db 0x88, 0x0E, 0x52, 0x05               ; 5A2D mov [0x552],cl
db 0xE9, 0xA3, 0xEB                     ; 5A31 jmp 0x45d7
db 0xE8, 0xC8, 0xFE                     ; 5A34 call 0x58ff
db 0x8A, 0x2E, 0x29, 0x00               ; 5A37 mov ch,[0x29]
db 0xB1, 0x00                           ; 5A3B mov cl,0x0
db 0xB0, 0xED                           ; 5A3D mov al,0xed
db 0xE8, 0x91, 0xFE                     ; 5A3F call 0x58d3
db 0xE9, 0xB7, 0xFE                     ; 5A42 jmp 0x58fc
db 0x58                                 ; 5A45 pop ax
db 0x50                                 ; 5A46 push ax
db 0x86, 0xC4                           ; 5A47 xchg al,ah
db 0xE8, 0x44, 0xF4                     ; 5A49 call 0x4e90
db 0x8A, 0x0E, 0x57, 0x00               ; 5A4C mov cl,[0x57]
db 0xFE, 0xC9                           ; 5A50 dec cl
db 0x8B, 0x1E, 0xE9, 0x04               ; 5A52 mov bx,[0x4e9]
db 0x88, 0x8F, 0x32, 0x00               ; 5A56 mov [bx+0x32],cl
db 0xE9, 0xAA, 0xFE                     ; 5A5A jmp 0x5907
db 0x58                                 ; 5A5D pop ax
db 0x86, 0xC4                           ; 5A5E xchg al,ah
db 0xE9, 0x49, 0xF8                     ; 5A60 jmp 0x52ac
db 0x8A, 0x2E, 0x62, 0x00               ; 5A63 mov ch,[0x62]
db 0xB1, 0x00                           ; 5A67 mov cl,0x0
db 0xE8, 0x93, 0xFE                     ; 5A69 call 0x58ff
db 0xB0, 0x6D                           ; 5A6C mov al,0x6d
db 0xE8, 0x62, 0xFE                     ; 5A6E call 0x58d3
db 0xE8, 0xAF, 0xFE                     ; 5A71 call 0x5923
db 0xA0, 0x63, 0x00                     ; 5A74 mov al,[0x63]
db 0x88, 0x07                           ; 5A77 mov [bx],al
db 0xE9, 0x80, 0xFE                     ; 5A79 jmp 0x58fc
db 0x58                                 ; 5A7C pop ax
db 0x50                                 ; 5A7D push ax
db 0x86, 0xC4                           ; 5A7E xchg al,ah
db 0xE8, 0x03, 0x00                     ; 5A80 call 0x5a86
db 0xE9, 0x81, 0xFE                     ; 5A83 jmp 0x5907
db 0xE8, 0x6A, 0xF5                     ; 5A86 call 0x4ff3
db 0xBB, 0x63, 0x00                     ; 5A89 mov bx,0x63
db 0x3C, 0x0D                           ; 5A8C cmp al,0xd
db 0x75, 0x03                           ; 5A8E jnz 0x5a93
db 0xE9, 0xE4, 0x04                     ; 5A90 jmp 0x5f77
db 0x3C, 0x20                           ; 5A93 cmp al,0x20
db 0x73, 0x01                           ; 5A95 jnc 0x5a98
db 0xC3                                 ; 5A97 ret
db 0xFE, 0x07                           ; 5A98 inc byte [bx]
db 0x53                                 ; 5A9A push bx
db 0xE8, 0x8D, 0xFE                     ; 5A9B call 0x592b
db 0x5B                                 ; 5A9E pop bx
db 0xFE, 0xC0                           ; 5A9F inc al
db 0x74, 0xF4                           ; 5AA1 jz 0x5a97
db 0xFE, 0xC8                           ; 5AA3 dec al
db 0x38, 0x07                           ; 5AA5 cmp [bx],al
db 0xE9, 0xC5, 0x04                     ; 5AA7 jmp 0x5f6f
db 0x58                                 ; 5AAA pop ax
db 0x86, 0xC4                           ; 5AAB xchg al,ah
db 0xA2, 0x62, 0x00                     ; 5AAD mov [0x62],al
db 0xC3                                 ; 5AB0 ret
db 0xA0, 0x61, 0x00                     ; 5AB1 mov al,[0x61]
db 0x0A, 0xC0                           ; 5AB4 or al,al
db 0x74, 0x03                           ; 5AB6 jz 0x5abb
db 0xE9, 0xE8, 0xAC                     ; 5AB8 jmp 0x7a3
db 0x80, 0xE2, 0xFB                     ; 5ABB and dl,0xfb
db 0x75, 0x02                           ; 5ABE jnz 0x5ac2
db 0xB2, 0x01                           ; 5AC0 mov dl,0x1
db 0xA2, 0x51, 0x05                     ; 5AC2 mov [0x551],al
db 0xFE, 0xC0                           ; 5AC5 inc al
db 0xA2, 0x50, 0x05                     ; 5AC7 mov [0x550],al
db 0x8A, 0xCA                           ; 5ACA mov cl,dl
db 0x80, 0xE1, 0x80                     ; 5ACC and cl,0x80
db 0x80, 0xE9, 0x01                     ; 5ACF sub cl,0x1
db 0xF5                                 ; 5AD2 cmc
db 0x1A, 0xC9                           ; 5AD3 sbb cl,cl
db 0x80, 0xE1, 0x80                     ; 5AD5 and cl,0x80
db 0xF6, 0xC2, 0x10                     ; 5AD8 test dl,0x10
db 0x74, 0x03                           ; 5ADB jz 0x5ae0
db 0x80, 0xC9, 0x20                     ; 5ADD or cl,0x20
db 0xA0, 0x60, 0x00                     ; 5AE0 mov al,[0x60]
db 0x0A, 0xC0                           ; 5AE3 or al,al
db 0x74, 0x02                           ; 5AE5 jz 0x5ae9
db 0xB1, 0x01                           ; 5AE7 mov cl,0x1
db 0x0A, 0xC9                           ; 5AE9 or cl,cl
db 0x75, 0x09                           ; 5AEB jnz 0x5af6
db 0xF6, 0x06, 0x5F, 0x00, 0xFF         ; 5AED test byte [0x5f],0xff
db 0x74, 0x02                           ; 5AF2 jz 0x5af6
db 0xB1, 0x40                           ; 5AF4 mov cl,0x40
db 0x88, 0x0E, 0x48, 0x05               ; 5AF6 mov [0x548],cl
db 0xB5, 0xFF                           ; 5AFA mov ch,0xff
db 0xB0, 0x68                           ; 5AFC mov al,0x68
db 0xE8, 0xD2, 0xFD                     ; 5AFE call 0x58d3
db 0x8A, 0x27                           ; 5B01 mov ah,[bx]
db 0xE8, 0x2E, 0xFE                     ; 5B03 call 0x5934
db 0xF6, 0xC4, 0x01                     ; 5B06 test ah,0x1
db 0x75, 0x0C                           ; 5B09 jnz 0x5b17
db 0xF6, 0xC1, 0x81                     ; 5B0B test cl,0x81
db 0x75, 0x03                           ; 5B0E jnz 0x5b13
db 0xE8, 0x2F, 0x00                     ; 5B10 call 0x5b42
db 0xB0, 0xFF                           ; 5B13 mov al,0xff
db 0xEB, 0x1F                           ; 5B15 jmp short 0x5b36
db 0xE8, 0x33, 0x00                     ; 5B17 call 0x5b4d
db 0xE8, 0x30, 0xFE                     ; 5B1A call 0x594d
db 0x72, 0xF8                           ; 5B1D jc 0x5b17
db 0x8B, 0x1E, 0xE9, 0x04               ; 5B1F mov bx,[0x4e9]
db 0xF6, 0x87, 0x31, 0x00, 0x81         ; 5B23 test byte [bx+0x31],0x81
db 0x75, 0x0A                           ; 5B28 jnz 0x5b34
db 0xE8, 0x11, 0x01                     ; 5B2A call 0x5c3e
db 0x73, 0x05                           ; 5B2D jnc 0x5b34
db 0xC6, 0x06, 0x50, 0x05, 0x00         ; 5B2F mov byte [0x550],0x0
db 0xB0, 0x01                           ; 5B34 mov al,0x1
db 0xA2, 0x61, 0x00                     ; 5B36 mov [0x61],al
db 0xE8, 0xE7, 0xFD                     ; 5B39 call 0x5923
db 0xC6, 0x07, 0x01                     ; 5B3C mov byte [bx],0x1
db 0xE9, 0xBA, 0xFD                     ; 5B3F jmp 0x58fc
db 0xBB, 0x3F, 0x05                     ; 5B42 mov bx,0x53f
db 0xB9, 0x11, 0x00                     ; 5B45 mov cx,0x11
db 0xB4, 0x03                           ; 5B48 mov ah,0x3
db 0xCD, 0x15                           ; 5B4A int 0x15
db 0xC3                                 ; 5B4C ret
db 0xBB, 0x53, 0x05                     ; 5B4D mov bx,0x553
db 0xB9, 0x11, 0x00                     ; 5B50 mov cx,0x11
db 0x53                                 ; 5B53 push bx
db 0xB4, 0x02                           ; 5B54 mov ah,0x2
db 0xCD, 0x15                           ; 5B56 int 0x15
db 0x73, 0x03                           ; 5B58 jnc 0x5b5d
db 0xE9, 0x02, 0x01                     ; 5B5A jmp 0x5c5f
db 0x5B                                 ; 5B5D pop bx
db 0xA0, 0x5E, 0x00                     ; 5B5E mov al,[0x5e]
db 0x0A, 0xC0                           ; 5B61 or al,al
db 0x75, 0x06                           ; 5B63 jnz 0x5b6b
db 0x80, 0x3F, 0xA5                     ; 5B65 cmp byte [bx],0xa5
db 0x75, 0xE6                           ; 5B68 jnz 0x5b50
db 0xC3                                 ; 5B6A ret
db 0xE9, 0x29, 0xAC                     ; 5B6B jmp 0x797
db 0xA0, 0x61, 0x00                     ; 5B6E mov al,[0x61]
db 0xFE, 0xC0                           ; 5B71 inc al
db 0x74, 0x0B                           ; 5B73 jz 0x5b80
db 0x32, 0xC0                           ; 5B75 xor al,al
db 0xA2, 0x61, 0x00                     ; 5B77 mov [0x61],al
db 0xA2, 0x60, 0x00                     ; 5B7A mov [0x60],al
db 0xE9, 0xCD, 0xE5                     ; 5B7D jmp 0x414d
db 0x8B, 0x1E, 0xE9, 0x04               ; 5B80 mov bx,[0x4e9]
db 0xF6, 0x87, 0x31, 0x00, 0x81         ; 5B84 test byte [bx+0x31],0x81
db 0x75, 0xEA                           ; 5B89 jnz 0x5b75
db 0xE8, 0x3B, 0x00                     ; 5B8B call 0x5bc9
db 0xE8, 0x1F, 0x01                     ; 5B8E call 0x5cb0
db 0xEB, 0xE2                           ; 5B91 jmp short 0x5b75
db 0x53                                 ; 5B93 push bx
db 0xBB, 0x61, 0x00                     ; 5B94 mov bx,0x61
db 0x38, 0x27                           ; 5B97 cmp [bx],ah
db 0x75, 0x0D                           ; 5B99 jnz 0x5ba8
db 0x8B, 0x1E, 0xE9, 0x04               ; 5B9B mov bx,[0x4e9]
db 0xF6, 0x87, 0x31, 0x00, 0x81         ; 5B9F test byte [bx+0x31],0x81
db 0x5B                                 ; 5BA4 pop bx
db 0x75, 0x01                           ; 5BA5 jnz 0x5ba8
db 0xC3                                 ; 5BA7 ret
db 0xE9, 0xE9, 0xAB                     ; 5BA8 jmp 0x794
db 0xB4, 0xFF                           ; 5BAB mov ah,0xff
db 0xE8, 0xE3, 0xFF                     ; 5BAD call 0x5b93
db 0x58                                 ; 5BB0 pop ax
db 0x50                                 ; 5BB1 push ax
db 0x86, 0xC4                           ; 5BB2 xchg al,ah
db 0xE8, 0x03, 0x00                     ; 5BB4 call 0x5bba
db 0xE9, 0x4D, 0xFD                     ; 5BB7 jmp 0x5907
db 0xE8, 0x26, 0x00                     ; 5BBA call 0x5be3
db 0x88, 0x07                           ; 5BBD mov [bx],al
db 0xFE, 0xC1                           ; 5BBF inc cl
db 0x74, 0x0B                           ; 5BC1 jz 0x5bce
db 0xE8, 0x5D, 0xFD                     ; 5BC3 call 0x5923
db 0x88, 0x0F                           ; 5BC6 mov [bx],cl
db 0xC3                                 ; 5BC8 ret
db 0xE8, 0x57, 0xFD                     ; 5BC9 call 0x5923
db 0x8A, 0x0F                           ; 5BCC mov cl,[bx]
db 0xBB, 0x53, 0x05                     ; 5BCE mov bx,0x553
db 0xB5, 0x00                           ; 5BD1 mov ch,0x0
db 0xFE, 0xC9                           ; 5BD3 dec cl
db 0x41                                 ; 5BD5 inc cx
db 0x88, 0x0F                           ; 5BD6 mov [bx],cl
db 0xB4, 0x03                           ; 5BD8 mov ah,0x3
db 0xCD, 0x15                           ; 5BDA int 0x15
db 0xE8, 0x44, 0xFD                     ; 5BDC call 0x5923
db 0xC6, 0x07, 0x01                     ; 5BDF mov byte [bx],0x1
db 0xC3                                 ; 5BE2 ret
db 0xE8, 0x3D, 0xFD                     ; 5BE3 call 0x5923
db 0x8A, 0x0F                           ; 5BE6 mov cl,[bx]
db 0xB5, 0x00                           ; 5BE8 mov ch,0x0
db 0xBB, 0x53, 0x05                     ; 5BEA mov bx,0x553
db 0x03, 0xD9                           ; 5BED add bx,cx
db 0xC3                                 ; 5BEF ret
db 0xB4, 0x01                           ; 5BF0 mov ah,0x1
db 0xE8, 0x9E, 0xFF                     ; 5BF2 call 0x5b93
db 0xE8, 0x03, 0x00                     ; 5BF5 call 0x5bfb
db 0xE9, 0x14, 0xFD                     ; 5BF8 jmp 0x590f
db 0xA0, 0x50, 0x05                     ; 5BFB mov al,[0x550]
db 0x2C, 0x01                           ; 5BFE sub al,0x1
db 0x73, 0x01                           ; 5C00 jnc 0x5c03
db 0xC3                                 ; 5C02 ret
db 0xBB, 0x51, 0x05                     ; 5C03 mov bx,0x551
db 0x8A, 0x07                           ; 5C06 mov al,[bx]
db 0xC6, 0x07, 0x00                     ; 5C08 mov byte [bx],0x0
db 0x0A, 0xC0                           ; 5C0B or al,al
db 0x74, 0x01                           ; 5C0D jz 0x5c10
db 0xC3                                 ; 5C0F ret
db 0xE8, 0x0A, 0x00                     ; 5C10 call 0x5c1d
db 0x73, 0x07                           ; 5C13 jnc 0x5c1c
db 0xC6, 0x06, 0x50, 0x05, 0x00         ; 5C15 mov byte [0x550],0x0
db 0x0A, 0xC0                           ; 5C1A or al,al
db 0xC3                                 ; 5C1C ret
db 0xE8, 0xC3, 0xFF                     ; 5C1D call 0x5be3
db 0x8A, 0x07                           ; 5C20 mov al,[bx]
db 0xFE, 0xC1                           ; 5C22 inc cl
db 0xE8, 0xFC, 0xFC                     ; 5C24 call 0x5923
db 0x88, 0x0F                           ; 5C27 mov [bx],cl
db 0xE8, 0xEF, 0xFC                     ; 5C29 call 0x591b
db 0x3A, 0x0F                           ; 5C2C cmp cl,[bx]
db 0x74, 0x03                           ; 5C2E jz 0x5c33
db 0x0A, 0xC0                           ; 5C30 or al,al
db 0xC3                                 ; 5C32 ret
db 0x80, 0x3F, 0x00                     ; 5C33 cmp byte [bx],0x0
db 0x75, 0xDD                           ; 5C36 jnz 0x5c15
db 0x50                                 ; 5C38 push ax
db 0xE8, 0x02, 0x00                     ; 5C39 call 0x5c3e
db 0x58                                 ; 5C3C pop ax
db 0xC3                                 ; 5C3D ret
db 0xBB, 0x53, 0x05                     ; 5C3E mov bx,0x553
db 0xB9, 0x00, 0x01                     ; 5C41 mov cx,0x100
db 0xB4, 0x02                           ; 5C44 mov ah,0x2
db 0xCD, 0x15                           ; 5C46 int 0x15
db 0x72, 0x15                           ; 5C48 jc 0x5c5f
db 0xA0, 0x53, 0x05                     ; 5C4A mov al,[0x553]
db 0xE8, 0xCB, 0xFC                     ; 5C4D call 0x591b
db 0x88, 0x07                           ; 5C50 mov [bx],al
db 0xE8, 0xCE, 0xFC                     ; 5C52 call 0x5923
db 0xC6, 0x07, 0x01                     ; 5C55 mov byte [bx],0x1
db 0xFE, 0xC8                           ; 5C58 dec al
db 0xF9                                 ; 5C5A stc
db 0x74, 0x01                           ; 5C5B jz 0x5c5e
db 0xF8                                 ; 5C5D clc
db 0xC3                                 ; 5C5E ret
db 0x80, 0xFC, 0x04                     ; 5C5F cmp ah,0x4
db 0x75, 0x05                           ; 5C62 jnz 0x5c69
db 0xB2, 0x18                           ; 5C64 mov dl,0x18
db 0xE9, 0x6F, 0xAB                     ; 5C66 jmp 0x7d8
db 0xE9, 0x25, 0xAB                     ; 5C69 jmp 0x791
db 0xA0, 0x50, 0x05                     ; 5C6C mov al,[0x550]
db 0x2C, 0x01                           ; 5C6F sub al,0x1
db 0x1A, 0xC0                           ; 5C71 sbb al,al
db 0xE9, 0x93, 0x08                     ; 5C73 jmp 0x6509
db 0x88, 0x0E, 0x51, 0x05               ; 5C76 mov [0x551],cl
db 0xE9, 0x5A, 0xE9                     ; 5C7A jmp 0x45d7
db 0xC6, 0x06, 0x5F, 0x00, 0x00         ; 5C7D mov byte [0x5f],0x0
db 0x53                                 ; 5C82 push bx
db 0x89, 0x1E, 0x4D, 0x05               ; 5C83 mov [0x54d],bx
db 0x8B, 0x16, 0x50, 0x03               ; 5C87 mov dx,[0x350]
db 0x89, 0x16, 0x4B, 0x05               ; 5C8B mov [0x54b],dx
db 0x8B, 0x0E, 0x04, 0x07               ; 5C8F mov cx,[0x704]
db 0x2B, 0xCB                           ; 5C93 sub cx,bx
db 0x89, 0x0E, 0x49, 0x05               ; 5C95 mov [0x549],cx
db 0x51                                 ; 5C99 push cx
db 0x52                                 ; 5C9A push dx
db 0xE8, 0xA4, 0xFE                     ; 5C9B call 0x5b42
db 0x5A                                 ; 5C9E pop dx
db 0x59                                 ; 5C9F pop cx
db 0x5B                                 ; 5CA0 pop bx
db 0xA0, 0x60, 0x00                     ; 5CA1 mov al,[0x60]
db 0x0A, 0xC0                           ; 5CA4 or al,al
db 0x06                                 ; 5CA6 push es
db 0x74, 0x02                           ; 5CA7 jz 0x5cab
db 0x8E, 0xC2                           ; 5CA9 mov es,dx
db 0xB4, 0x03                           ; 5CAB mov ah,0x3
db 0xCD, 0x15                           ; 5CAD int 0x15
db 0x07                                 ; 5CAF pop es
db 0xE8, 0x89, 0x00                     ; 5CB0 call 0x5d3c
db 0xBA, 0x05, 0x00                     ; 5CB3 mov dx,0x5
db 0xB9, 0x00, 0x00                     ; 5CB6 mov cx,0x0
db 0x49                                 ; 5CB9 dec cx
db 0x75, 0xFD                           ; 5CBA jnz 0x5cb9
db 0x4A                                 ; 5CBC dec dx
db 0x75, 0xFA                           ; 5CBD jnz 0x5cb9
db 0xE8, 0x76, 0x00                     ; 5CBF call 0x5d38
db 0xC3                                 ; 5CC2 ret
db 0xBE, 0x53, 0x05                     ; 5CC3 mov si,0x553
db 0x8B, 0x8C, 0x0A, 0x00               ; 5CC6 mov cx,[si+0xa]
db 0xA0, 0x60, 0x00                     ; 5CCA mov al,[0x60]
db 0x0A, 0xC0                           ; 5CCD or al,al
db 0x9C                                 ; 5CCF pushf
db 0x51                                 ; 5CD0 push cx
db 0x75, 0x0D                           ; 5CD1 jnz 0x5ce0
db 0x50                                 ; 5CD3 push ax
db 0x53                                 ; 5CD4 push bx
db 0x51                                 ; 5CD5 push cx
db 0x56                                 ; 5CD6 push si
db 0x03, 0xD9                           ; 5CD7 add bx,cx
db 0xE8, 0x16, 0xE6                     ; 5CD9 call 0x42f2
db 0x5E                                 ; 5CDC pop si
db 0x59                                 ; 5CDD pop cx
db 0x5B                                 ; 5CDE pop bx
db 0x58                                 ; 5CDF pop ax
db 0x3C, 0x01                           ; 5CE0 cmp al,0x1
db 0x75, 0x04                           ; 5CE2 jnz 0x5ce8
db 0x8B, 0x9C, 0x0E, 0x00               ; 5CE4 mov bx,[si+0xe]
db 0x06                                 ; 5CE8 push es
db 0x0A, 0xC0                           ; 5CE9 or al,al
db 0x74, 0x0E                           ; 5CEB jz 0x5cfb
db 0x8B, 0x94, 0x0C, 0x00               ; 5CED mov dx,[si+0xc]
db 0xFE, 0xC8                           ; 5CF1 dec al
db 0x74, 0x04                           ; 5CF3 jz 0x5cf9
db 0x8B, 0x16, 0x50, 0x03               ; 5CF5 mov dx,[0x350]
db 0x8E, 0xC2                           ; 5CF9 mov es,dx
db 0xB4, 0x02                           ; 5CFB mov ah,0x2
db 0xCD, 0x15                           ; 5CFD int 0x15
db 0x07                                 ; 5CFF pop es
db 0x72, 0x12                           ; 5D00 jc 0x5d14
db 0x59                                 ; 5D02 pop cx
db 0x9D                                 ; 5D03 popf
db 0x75, 0x0B                           ; 5D04 jnz 0x5d11
db 0x8B, 0x1E, 0x30, 0x00               ; 5D06 mov bx,[0x30]
db 0x03, 0xD9                           ; 5D0A add bx,cx
db 0x43                                 ; 5D0C inc bx
db 0x89, 0x1E, 0x58, 0x03               ; 5D0D mov [0x358],bx
db 0xE9, 0x80, 0xE5                     ; 5D11 jmp 0x4294
db 0x50                                 ; 5D14 push ax
db 0xE8, 0x05, 0xD0                     ; 5D15 call 0x2d1d
db 0x58                                 ; 5D18 pop ax
db 0x80, 0xFC, 0x04                     ; 5D19 cmp ah,0x4
db 0x75, 0x05                           ; 5D1C jnz 0x5d23
db 0xB2, 0x18                           ; 5D1E mov dl,0x18
db 0xE9, 0xB5, 0xAA                     ; 5D20 jmp 0x7d8
db 0xE9, 0x6B, 0xAA                     ; 5D23 jmp 0x791
db 0x4B                                 ; 5D26 dec bx
db 0xE8, 0xF3, 0xB1                     ; 5D27 call 0xf1d
db 0x75, 0x05                           ; 5D2A jnz 0x5d31
db 0xA0, 0x64, 0x00                     ; 5D2C mov al,[0x64]
db 0xEB, 0x03                           ; 5D2F jmp short 0x5d34
db 0xE8, 0xE8, 0xC1                     ; 5D31 call 0x1f1c
db 0x0A, 0xC0                           ; 5D34 or al,al
db 0x75, 0x04                           ; 5D36 jnz 0x5d3c
db 0xB0, 0x01                           ; 5D38 mov al,0x1
db 0xEB, 0x02                           ; 5D3A jmp short 0x5d3e
db 0xB0, 0x00                           ; 5D3C mov al,0x0
db 0xA2, 0x64, 0x00                     ; 5D3E mov [0x64],al
db 0x8A, 0xE0                           ; 5D41 mov ah,al
db 0xCD, 0x15                           ; 5D43 int 0x15
db 0xC3                                 ; 5D45 ret
db 0xCD, 0xDB                           ; 5D46 int 0xdb
db 0xF9                                 ; 5D48 stc
db 0xEB, 0x01                           ; 5D49 jmp short 0x5d4c
db 0xF8                                 ; 5D4B clc
db 0x8B, 0xF3                           ; 5D4C mov si,bx
db 0x9C                                 ; 5D4E pushf
db 0x8B, 0x0E, 0xA5, 0x04               ; 5D4F mov cx,[0x4a5]
db 0x8A, 0xC3                           ; 5D53 mov al,bl
db 0x32, 0xC1                           ; 5D55 xor al,cl
db 0xA2, 0xA7, 0x04                     ; 5D57 mov [0x4a7],al
db 0x8A, 0xC7                           ; 5D5A mov al,bh
db 0x32, 0xE4                           ; 5D5C xor ah,ah
db 0x8A, 0xDD                           ; 5D5E mov bl,ch
db 0x32, 0xFF                           ; 5D60 xor bh,bh
db 0x9D                                 ; 5D62 popf
db 0x73, 0x07                           ; 5D63 jnc 0x5d6c
db 0x03, 0xC3                           ; 5D65 add ax,bx
db 0x2D, 0x01, 0x01                     ; 5D67 sub ax,0x101
db 0xEB, 0x02                           ; 5D6A jmp short 0x5d6e
db 0x2B, 0xC3                           ; 5D6C sub ax,bx
db 0x0A, 0xE4                           ; 5D6E or ah,ah
db 0x78, 0x0D                           ; 5D70 js 0x5d7f
db 0x3D, 0x80, 0x00                     ; 5D72 cmp ax,0x80
db 0x72, 0x15                           ; 5D75 jc 0x5d8c
db 0x8B, 0xDE                           ; 5D77 mov bx,si
db 0x83, 0xC4, 0x02                     ; 5D79 add sp,byte +0x2
db 0xE9, 0x5D, 0x17                     ; 5D7C jmp 0x74dc
db 0x05, 0x80, 0x00                     ; 5D7F add ax,0x80
db 0x79, 0x0B                           ; 5D82 jns 0x5d8f
db 0x8B, 0xDE                           ; 5D84 mov bx,si
db 0x83, 0xC4, 0x02                     ; 5D86 add sp,byte +0x2
db 0xE9, 0xDF, 0x1D                     ; 5D89 jmp 0x7b6b
db 0x05, 0x80, 0x00                     ; 5D8C add ax,0x80
db 0xA2, 0xA6, 0x04                     ; 5D8F mov [0x4a6],al
db 0xBB, 0xA5, 0x04                     ; 5D92 mov bx,0x4a5
db 0x80, 0x0F, 0x80                     ; 5D95 or byte [bx],0x80
db 0x8B, 0xDE                           ; 5D98 mov bx,si
db 0x32, 0xFF                           ; 5D9A xor bh,bh
db 0x80, 0xCB, 0x80                     ; 5D9C or bl,0x80
db 0xC3                                 ; 5D9F ret
db 0xC6, 0x06, 0x39, 0x03, 0x80         ; 5DA0 mov byte [0x339],0x80
db 0xE8, 0xC6, 0xD9                     ; 5DA5 call 0x376e
db 0x53                                 ; 5DA8 push bx
db 0x8B, 0xDA                           ; 5DA9 mov bx,dx
db 0xE8, 0xEA, 0x06                     ; 5DAB call 0x6498
db 0xE8, 0x04, 0xC5                     ; 5DAE call 0x22b5
db 0x89, 0x1E, 0x5E, 0x04               ; 5DB1 mov [0x45e],bx
db 0xB1, 0x20                           ; 5DB5 mov cl,0x20
db 0xE8, 0x1B, 0xCF                     ; 5DB7 call 0x2cd5
db 0x5B                                 ; 5DBA pop bx
db 0xE8, 0x60, 0xB1                     ; 5DBB call 0xf1e
db 0x74, 0x17                           ; 5DBE jz 0x5dd7
db 0xE8, 0x31, 0xD0                     ; 5DC0 call 0x2df4
db 0x28, 0xE8                           ; 5DC3 sub al,ch
db 0xA7                                 ; 5DC5 cmpsw
db 0xD9, 0x52, 0x8A                     ; 5DC6 fst dword [bp+si-0x76]
db 0x07                                 ; 5DC9 pop es
db 0x3C, 0x2C                           ; 5DCA cmp al,0x2c
db 0x75, 0x05                           ; 5DCC jnz 0x5dd3
db 0xE8, 0x4C, 0xB1                     ; 5DCE call 0xf1d
db 0xEB, 0xF1                           ; 5DD1 jmp short 0x5dc4
db 0xE8, 0x1E, 0xD0                     ; 5DD3 call 0x2df4
db 0x29, 0x89, 0x1E, 0x3B               ; 5DD6 sub [bx+di+0x3b1e],cx
db 0x03, 0x0E, 0xB8, 0xE9               ; 5DDA add cx,[0xe9b8]
db 0x5D                                 ; 5DDE pop bp
db 0x50                                 ; 5DDF push ax
db 0xFF, 0x36, 0x50, 0x03               ; 5DE0 push word [0x350]
db 0xFF, 0x36, 0x5E, 0x04               ; 5DE4 push word [0x45e]
db 0xCB                                 ; 5DE8 retf
db 0x8B, 0x1E, 0x3B, 0x03               ; 5DE9 mov bx,[0x33b]
db 0xC3                                 ; 5DED ret
db 0x53                                 ; 5DEE push bx
db 0xE8, 0x0E, 0x05                     ; 5DEF call 0x6300
db 0x3C, 0x6C                           ; 5DF2 cmp al,0x6c
db 0x74, 0x0A                           ; 5DF4 jz 0x5e00
db 0x3C, 0x4C                           ; 5DF6 cmp al,0x4c
db 0x74, 0x06                           ; 5DF8 jz 0x5e00
db 0x3C, 0x71                           ; 5DFA cmp al,0x71
db 0x74, 0x02                           ; 5DFC jz 0x5e00
db 0x3C, 0x51                           ; 5DFE cmp al,0x51
db 0x5B                                 ; 5E00 pop bx
db 0xC3                                 ; 5E01 ret
db 0x26, 0x26, 0x26, 0x26, 0x26, 0x26
db 0x26, 0x26, 0x26, 0x26, 0x26, 0x26
db 0x26, 0x26, 0x26, 0x26, 0x26, 0x26
db 0x26, 0x26, 0x26, 0x26, 0x26, 0x26
db 0x26, 0x25, 0x25, 0x25, 0x24, 0x24
db 0x24, 0x23                           ; 5E20 and al,0x23
db 0x23, 0x23                           ; 5E22 and sp,[bp+di]
db 0x22, 0x22                           ; 5E24 and ah,[bp+si]
db 0x22, 0x22                           ; 5E26 and ah,[bp+si]
db 0x21, 0x21                           ; 5E28 and [bx+di],sp
db 0x21, 0x20                           ; 5E2A and [bx+si],sp
db 0x20, 0x20                           ; 5E2C and [bx+si],ah
db 0x1F                                 ; 5E2E pop ds
db 0x1F                                 ; 5E2F pop ds
db 0x1F                                 ; 5E30 pop ds
db 0x1F                                 ; 5E31 pop ds
db 0x1E                                 ; 5E32 push ds
db 0x1E                                 ; 5E33 push ds
db 0x1E                                 ; 5E34 push ds
db 0x1D, 0x1D, 0x1D                     ; 5E35 sbb ax,0x1d1d
db 0x1D, 0x1C, 0x1C                     ; 5E38 sbb ax,0x1c1c
db 0x1C, 0x1B                           ; 5E3B sbb al,0x1b
db 0x1B, 0x1B                           ; 5E3D sbb bx,[bp+di]
db 0x1A, 0x1A                           ; 5E3F sbb bl,[bp+si]
db 0x1A, 0x19                           ; 5E41 sbb bl,[bx+di]
db 0x19, 0x19                           ; 5E43 sbb [bx+di],bx
db 0x19, 0x18                           ; 5E45 sbb [bx+si],bx
db 0x18, 0x18                           ; 5E47 sbb [bx+si],bl
db 0x17                                 ; 5E49 pop ss
db 0x17                                 ; 5E4A pop ss
db 0x17                                 ; 5E4B pop ss
db 0x17                                 ; 5E4C pop ss
db 0x16                                 ; 5E4D push ss
db 0x16                                 ; 5E4E push ss
db 0x16                                 ; 5E4F push ss
db 0x16                                 ; 5E50 push ss
db 0x15, 0x15, 0x15                     ; 5E51 adc ax,0x1515
db 0x14, 0x14                           ; 5E54 adc al,0x14
db 0x14, 0x13                           ; 5E56 adc al,0x13
db 0x13, 0x13                           ; 5E58 adc dx,[bp+di]
db 0x13, 0x12                           ; 5E5A adc dx,[bp+si]
db 0x12, 0x12                           ; 5E5C adc dl,[bp+si]
db 0x11, 0x11                           ; 5E5E adc [bx+di],dx
db 0x11, 0x10                           ; 5E60 adc [bx+si],dx
db 0x10, 0x10                           ; 5E62 adc [bx+si],dl
db 0x10, 0x0F                           ; 5E64 adc [bx],cl
db 0x0F, 0x0F, 0x0E, 0x0E, 0x0E, 0x0D   ; 5E66 pi2fd mm1,[0xe0e]
db 0x0D, 0x0D, 0x0D                     ; 5E6C or ax,0xd0d
db 0x0C, 0x0C                           ; 5E6F or al,0xc
db 0x0C, 0x0B                           ; 5E71 or al,0xb
db 0x0B, 0x0B                           ; 5E73 or cx,[bp+di]
db 0x0A, 0x0A                           ; 5E75 or cl,[bp+si]
db 0x0A, 0x0A                           ; 5E77 or cl,[bp+si]
db 0x09, 0x09                           ; 5E79 or [bx+di],cx
db 0x09, 0x08                           ; 5E7B or [bx+si],cx
db 0x08, 0x08                           ; 5E7D or [bx+si],cl
db 0x07                                 ; 5E7F pop es
db 0x07                                 ; 5E80 pop es
db 0x07                                 ; 5E81 pop es
db 0x06                                 ; 5E82 push es
db 0x06                                 ; 5E83 push es
db 0x06                                 ; 5E84 push es
db 0x06                                 ; 5E85 push es
db 0x05, 0x05, 0x05                     ; 5E86 add ax,0x505
db 0x04, 0x04                           ; 5E89 add al,0x4
db 0x04, 0x03                           ; 5E8B add al,0x3
db 0x03, 0x03                           ; 5E8D add ax,[bp+di]
db 0x03, 0x02                           ; 5E8F add ax,[bp+si]
db 0x02, 0x02                           ; 5E91 add al,[bp+si]
db 0x01, 0x01                           ; 5E93 add [bx+di],ax
db 0x01, 0x00                           ; 5E95 add [bx+si],ax
db 0x00, 0x00                           ; 5E97 add [bx+si],al
db 0x00, 0xFF                           ; 5E99 add bh,bh
db 0xFF                                 ; 5E9B db 0xFF
db 0xFF                                 ; 5E9C db 0xFF
db 0xFE                                 ; 5E9D db 0xFE
db 0xFE                                 ; 5E9E db 0xFE
db 0xFE                                 ; 5E9F db 0xFE
db 0xFD                                 ; 5EA0 std
db 0xFD                                 ; 5EA1 std
db 0xFD                                 ; 5EA2 std
db 0xFD                                 ; 5EA3 std
db 0xFC                                 ; 5EA4 cld
db 0xFC                                 ; 5EA5 cld
db 0xFC                                 ; 5EA6 cld
db 0xFB                                 ; 5EA7 sti
db 0xFB                                 ; 5EA8 sti
db 0xFB                                 ; 5EA9 sti
db 0xFA                                 ; 5EAA cli
db 0xFA                                 ; 5EAB cli
db 0xFA                                 ; 5EAC cli
db 0xFA                                 ; 5EAD cli
db 0xF9                                 ; 5EAE stc
db 0xF9                                 ; 5EAF stc
db 0xF9                                 ; 5EB0 stc
db 0xF8                                 ; 5EB1 clc
db 0xF8                                 ; 5EB2 clc
db 0xF8                                 ; 5EB3 clc
db 0xF7, 0xF7                           ; 5EB4 div di
db 0xF7, 0xF7                           ; 5EB6 div di
db 0xF6, 0xF6                           ; 5EB8 div dh
db 0xF6, 0xF5                           ; 5EBA div ch
db 0xF5                                 ; 5EBC cmc
db 0xF5                                 ; 5EBD cmc
db 0xF4                                 ; 5EBE hlt
db 0xF4                                 ; 5EBF hlt
db 0xF4                                 ; 5EC0 hlt
db 0xF4                                 ; 5EC1 hlt
db 0xF3, 0xF3, 0xF3, 0xF2, 0xF2, 0xF2, 0xF1     ; 5EC2 repne int1
db 0xF1                                 ; 5EC9 int1
db 0xF1                                 ; 5ECA int1
db 0xF1                                 ; 5ECB int1
db 0xF0, 0xF0, 0xF0, 0xEF               ; 5ECC lock out dx,ax
db 0xEF                                 ; 5ED0 out dx,ax
db 0xEF                                 ; 5ED1 out dx,ax
db 0xEE                                 ; 5ED2 out dx,al
db 0xEE                                 ; 5ED3 out dx,al
db 0xEE                                 ; 5ED4 out dx,al
db 0xEE                                 ; 5ED5 out dx,al
db 0xED                                 ; 5ED6 in ax,dx
db 0xED                                 ; 5ED7 in ax,dx
db 0xED                                 ; 5ED8 in ax,dx
db 0xEC                                 ; 5ED9 in al,dx
db 0xEC                                 ; 5EDA in al,dx
db 0xEC                                 ; 5EDB in al,dx
db 0xEB, 0xEB                           ; 5EDC jmp short 0x5ec9
db 0xEB, 0xEB                           ; 5EDE jmp short 0x5ecb
db 0xEA, 0xEA, 0xEA, 0xE9, 0xE9         ; 5EE0 jmp 0xe9e9:0xeaea
db 0xE9, 0xE8, 0xE8                     ; 5EE5 jmp 0x47d0
db 0xE8, 0xE7, 0xE7                     ; 5EE8 call 0x46d2
db 0xE7, 0xE7                           ; 5EEB out 0xe7,ax
db 0xE6, 0xE6                           ; 5EED out 0xe6,al
db 0xE6, 0xE5                           ; 5EEF out 0xe5,al
db 0xE5, 0xE5                           ; 5EF1 in ax,0xe5
db 0xE4, 0xE4                           ; 5EF3 in al,0xe4
db 0xE4, 0xE4                           ; 5EF5 in al,0xe4
db 0xE3, 0xE3                           ; 5EF7 jcxz 0x5edc
db 0xE3, 0xE2                           ; 5EF9 jcxz 0x5edd
db 0xE2, 0xE2                           ; 5EFB loop 0x5edf
db 0xE1, 0xE1                           ; 5EFD loope 0x5ee0
db 0xE1, 0xE1                           ; 5EFF loope 0x5ee2
db 0xE0, 0x0B                           ; 5F01 loopne 0x5f0e
db 0xF6, 0x79, 0x02                     ; 5F03 idiv byte [bx+di+0x2]
db 0xF7, 0xDA                           ; 5F06 neg dx
db 0x2B, 0xD7                           ; 5F08 sub dx,di
db 0x70, 0x3D                           ; 5F0A jo 0x5f49
db 0x74, 0x3A                           ; 5F0C jz 0x5f48
db 0x53                                 ; 5F0E push bx
db 0xE8, 0x90, 0x1C                     ; 5F0F call 0x7ba2
db 0x9C                                 ; 5F12 pushf
db 0x73, 0x03                           ; 5F13 jnc 0x5f18
db 0xE8, 0x6A, 0x0C                     ; 5F15 call 0x6b82
db 0x0B, 0xD2                           ; 5F18 or dx,dx
db 0x78, 0x0F                           ; 5F1A js 0x5f2b
db 0x83, 0xFA, 0x27                     ; 5F1C cmp dx,byte +0x27
db 0x72, 0x1D                           ; 5F1F jc 0x5f3e
db 0x9D                                 ; 5F21 popf
db 0x73, 0x03                           ; 5F22 jnc 0x5f27
db 0xE8, 0x3D, 0x0C                     ; 5F24 call 0x6b64
db 0x5B                                 ; 5F27 pop bx
db 0xE9, 0xB1, 0x15                     ; 5F28 jmp 0x74dc
L_5F2B:
db 0x83, 0xFA, 0xDA                     ; 5F2B cmp dx,byte -0x26
db 0x7D, 0x0E                           ; 5F2E jnl 0x5f3e
db 0x83, 0xC2, 0x26                     ; 5F30 add dx,byte +0x26
db 0x83, 0xFA, 0xDA                     ; 5F33 cmp dx,byte -0x26
db 0x7C, 0x11                           ; 5F36 jl 0x5f49
db 0xE8, 0x13, 0x00                     ; 5F38 call 0x5f4e
db 0xBA, 0xDA, 0xFF                     ; 5F3B mov dx,0xffda
db 0xE8, 0x0D, 0x00                     ; 5F3E call 0x5f4e
db 0x9D                                 ; 5F41 popf
db 0x73, 0x03                           ; 5F42 jnc 0x5f47
db 0xE8, 0x1D, 0x0C                     ; 5F44 call 0x6b64
db 0x5B                                 ; 5F47 pop bx
db 0xC3                                 ; 5F48 ret

db 0xE8, 0x1F, 0x1C                     ; 5F49 call 0x7b6b
db 0xEB, 0xF3                           ; 5F4C jmp short 0x5f41
db 0x0B, 0xD2                           ; 5F4E or dx,dx
db 0x9C                                 ; 5F50 pushf
db 0x79, 0x02                           ; 5F51 jns 0x5f55
db 0xF7, 0xDA                           ; 5F53 neg dx
db 0xB9, 0x03, 0x00                     ; 5F55 mov cx,0x3
db 0xD3, 0xE2                           ; 5F58 shl dx,cl
db 0x81, 0xC2, 0x32, 0x60               ; 5F5A add dx,0x6032
db 0x87, 0xDA                           ; 5F5E xchg bx,dx
db 0xE8, 0x25, 0x1D                     ; 5F60 call 0x7c88
db 0x9D                                 ; 5F63 popf
db 0x78, 0x03                           ; 5F64 js 0x5f69
db 0xE9, 0xEC, 0x0C                     ; 5F66 jmp 0x6c55
db 0xE8, 0xEC, 0x1C                     ; 5F69 call 0x7c58
db 0xE9, 0xD4, 0x08                     ; 5F6C jmp 0x6843
db 0x72, 0x09                           ; 5F6F jc 0x5f7a
db 0xB0, 0x0D                           ; 5F71 mov al,0xd
db 0x90                                 ; 5F73 nop
db 0xE9, 0x0F, 0xFB                     ; 5F74 jmp 0x5a86
db 0xC6, 0x07, 0x00                     ; 5F77 mov byte [bx],0x0
db 0x8A, 0x07                           ; 5F7A mov al,[bx]
db 0xE8, 0xA4, 0xF9                     ; 5F7C call 0x5923
db 0x88, 0x07                           ; 5F7F mov [bx],al
db 0xC3                                 ; 5F81 ret
db 0x75, 0x06                           ; 5F82 jnz 0x5f8a
db 0xB0, 0x0A                           ; 5F84 mov al,0xa
db 0x90                                 ; 5F86 nop
db 0xE8, 0x69, 0xF0                     ; 5F87 call 0x4ff3
db 0x58                                 ; 5F8A pop ax
db 0x5A                                 ; 5F8B pop dx
db 0x5B                                 ; 5F8C pop bx
db 0x9D                                 ; 5F8D popf
db 0xC3                                 ; 5F8E ret
db 0x80, 0x3E, 0x6A, 0x00, 0x00         ; 5F8F cmp byte [0x6a],0x0
db 0x74, 0x12                           ; 5F94 jz 0x5fa8
db 0x1E                                 ; 5F96 push ds
db 0x53                                 ; 5F97 push bx
db 0xC5, 0x1E, 0x6B, 0x00               ; 5F98 lds bx,[0x6b]
db 0x80, 0x3F, 0x00                     ; 5F9C cmp byte [bx],0x0
db 0x5B                                 ; 5F9F pop bx
db 0x1F                                 ; 5FA0 pop ds
db 0x75, 0x05                           ; 5FA1 jnz 0x5fa8
db 0xC6, 0x06, 0x6A, 0x00, 0x00         ; 5FA3 mov byte [0x6a],0x0
db 0xE9, 0x72, 0xAF                     ; 5FA8 jmp 0xf1d
db 0x5F                                 ; 5FAB pop di
db 0x5E                                 ; 5FAC pop si
db 0xE9, 0xBE, 0xED                     ; 5FAD jmp 0x4d6e
db 0x75, 0x10                           ; 5FB0 jnz 0x5fc2
db 0x5B                                 ; 5FB2 pop bx
db 0x88, 0x36, 0xA7, 0x04               ; 5FB3 mov [0x4a7],dh
db 0xBB, 0x00, 0x10                     ; 5FB7 mov bx,0x1000
db 0xC6, 0x06, 0xFB, 0x02, 0x04         ; 5FBA mov byte [0x2fb],0x4
db 0xE9, 0x28, 0x15                     ; 5FBF jmp 0x74ea
db 0xC7, 0x06, 0xA5, 0x04, 0x00, 0x00   ; 5FC2 mov word [0x4a5],0x0
db 0xC3                                 ; 5FC8 ret
db 0x10, 0x5B, 0x88                     ; 5FC9 adc [bp+di-0x78],bl
db 0x36, 0xA7                           ; 5FCC ss cmpsw
db 0x04, 0xBB                           ; 5FCE add al,0xbb
db 0x00, 0x10                           ; 5FD0 add [bx+si],dl
db 0xC6, 0x06, 0xFB, 0x02, 0x04         ; 5FD2 mov byte [0x2fb],0x4
db 0xE9, 0x10, 0x15                     ; 5FD7 jmp 0x74ea
db 0xC7, 0x06, 0xA5, 0x04, 0x00, 0x00   ; 5FDA mov word [0x4a5],0x0
db 0xC3                                 ; 5FE0 ret
db 0x5C                                 ; 5FE1 pop sp
db 0xD6                                 ; 5FE2 salc
db 0xED                                 ; 5FE3 in ax,dx
db 0xBD, 0xCE, 0xFE                     ; 5FE4 mov bp,0xfece
db 0xE6, 0x5B                           ; 5FE7 out 0x5b,al
db 0x5F                                 ; 5FE9 pop di
db 0xA6                                 ; 5FEA cmpsb
db 0xB4, 0x36                           ; 5FEB mov ah,0x36
db 0x41                                 ; 5FED inc cx
db 0x5F                                 ; 5FEE pop di
db 0x70, 0x09                           ; 5FEF jo 0x5ffa
db 0x63, 0xCF                           ; 5FF1 arpl di,cx
db 0x61                                 ; 5FF3 popa
db 0x84, 0x11                           ; 5FF4 test [bx+di],dl
db 0x77, 0xCC                           ; 5FF6 ja 0x5fc4
db 0x2B, 0x66, 0x43                     ; 5FF8 sub sp,[bp+0x43]
db 0x7A, 0xE5                           ; 5FFB jpe 0x5fe2
db 0xD5, 0x94                           ; 5FFD aad 0x94
db 0xBF, 0x56, 0x69                     ; 5FFF mov di,0x6956
db 0x6A, 0x6C                           ; 6002 push byte +0x6c
db 0xAF                                 ; 6004 scasw
db 0x05, 0xBD, 0x37                     ; 6005 add ax,0x37bd
db 0x06                                 ; 6008 push es
db 0x6D                                 ; 6009 insw
db 0x85, 0x47, 0x1B                     ; 600A test [bx+0x1b],ax
db 0x47                                 ; 600D inc di
db 0xAC                                 ; 600E lodsb
db 0xC5, 0x27                           ; 600F lds sp,[bx]
db 0x70, 0x66                           ; 6011 jo 0x6079
db 0x19, 0xE2                           ; 6013 sbb dx,sp
db 0x58                                 ; 6015 pop ax
db 0x17                                 ; 6016 pop ss
db 0xB7, 0x51                           ; 6017 mov bh,0x51
db 0x73, 0xE0                           ; 6019 jnc 0x5ffb
db 0x4F                                 ; 601B dec di
db 0x8D, 0x97, 0x6E, 0x12               ; 601C lea dx,[bx+0x126e]
db 0x03, 0x77, 0xD8                     ; 6020 add si,[bx-0x28]
db 0xA3, 0x70, 0x3D                     ; 6023 mov [0x3d70],ax
db 0x0A, 0xD7                           ; 6026 or dl,bh
db 0x23, 0x7A, 0xCD                     ; 6028 and di,[bp+si-0x33]
db 0xCC                                 ; 602B int3
db 0xCC                                 ; 602C int3
db 0xCC                                 ; 602D int3
db 0xCC                                 ; 602E int3
db 0xCC                                 ; 602F int3
db 0x4C                                 ; 6030 dec sp
db 0x7D, 0x00                           ; 6031 jnl 0x6033
db 0x00, 0x00                           ; 6033 add [bx+si],al
db 0x00, 0x00                           ; 6035 add [bx+si],al
db 0x00, 0x00                           ; 6037 add [bx+si],al
db 0x81, 0x00, 0x00, 0x00               ; 6039 add word [bx+si],0x0
db 0x00, 0x00                           ; 603D add [bx+si],al
db 0x00, 0x20                           ; 603F add [bx+si],ah
db 0x84, 0x00                           ; 6041 test [bx+si],al
db 0x00, 0x00                           ; 6043 add [bx+si],al
db 0x00, 0x00                           ; 6045 add [bx+si],al
db 0x00, 0x48, 0x87                     ; 6047 add [bx+si-0x79],cl
db 0x00, 0x00                           ; 604A add [bx+si],al
db 0x00, 0x00                           ; 604C add [bx+si],al
db 0x00, 0x00                           ; 604E add [bx+si],al
db 0x7A, 0x8A                           ; 6050 jpe 0x5fdc
db 0x00, 0x00                           ; 6052 add [bx+si],al
db 0x00, 0x00                           ; 6054 add [bx+si],al
db 0x00, 0x40, 0x1C                     ; 6056 add [bx+si+0x1c],al
db 0x8E, 0x00                           ; 6059 mov es,[bx+si]
db 0x00, 0x00                           ; 605B add [bx+si],al
db 0x00, 0x00                           ; 605D add [bx+si],al
db 0x50                                 ; 605F push ax
db 0x43                                 ; 6060 inc bx
db 0x91                                 ; 6061 xchg ax,cx
db 0x00, 0x00                           ; 6062 add [bx+si],al
db 0x00, 0x00                           ; 6064 add [bx+si],al
db 0x00, 0x24                           ; 6066 add [si],ah
db 0x74, 0x94                           ; 6068 jz 0x5ffe
db 0x00, 0x00                           ; 606A add [bx+si],al
db 0x00, 0x00                           ; 606C add [bx+si],al
db 0x80, 0x96, 0x18, 0x98, 0x00         ; 606E adc byte [bp+0x9818],0x0
db 0x00, 0x00                           ; 6073 add [bx+si],al
db 0x00, 0x20                           ; 6075 add [bx+si],ah
db 0xBC, 0x3E, 0x9B                     ; 6077 mov sp,0x9b3e
db 0x00, 0x00                           ; 607A add [bx+si],al
db 0x00, 0x00                           ; 607C add [bx+si],al
db 0x28, 0x6B, 0x6E                     ; 607E sub [bp+di+0x6e],ch
db 0x9E                                 ; 6081 sahf
db 0x00, 0x00                           ; 6082 add [bx+si],al
db 0x00, 0x00                           ; 6084 add [bx+si],al
db 0xF9                                 ; 6086 stc
db 0x02, 0x15                           ; 6087 add dl,[di]
db 0xA2, 0x00, 0x00                     ; 6089 mov [0x0],al
db 0x00, 0x40, 0xB7                     ; 608C add [bx+si-0x49],al
db 0x43                                 ; 608F inc bx
db 0x3A, 0xA5, 0x00, 0x00               ; 6090 cmp ah,[di+0x0]
db 0x00, 0x10                           ; 6094 add [bx+si],dl
db 0xA5                                 ; 6096 movsw
db 0xD4, 0x68                           ; 6097 aam 0x68
db 0xA8, 0x00                           ; 6099 test al,0x0
db 0x00, 0x00                           ; 609B add [bx+si],al
db 0x2A, 0xE7                           ; 609D sub ah,bh
db 0x84, 0x11                           ; 609F test [bx+di],dl
db 0xAC                                 ; 60A1 lodsb
db 0x00, 0x00                           ; 60A2 add [bx+si],al
db 0x80, 0xF4, 0x20                     ; 60A4 xor ah,0x20
db 0xE6, 0x35                           ; 60A7 out 0x35,al
db 0xAF                                 ; 60A9 scasw
db 0x00, 0x00                           ; 60AA add [bx+si],al
db 0xA0, 0x31, 0xA9                     ; 60AC mov al,[0xa931]
db 0x5F                                 ; 60AF pop di
db 0x63, 0xB2, 0x00, 0x00               ; 60B0 arpl [bp+si+0x0],si
db 0x04, 0xBF                           ; 60B4 add al,0xbf
db 0xC9                                 ; 60B6 leave
db 0x1B, 0x0E, 0xB6, 0x00               ; 60B7 sbb cx,[0xb6]
db 0x00, 0xC5                           ; 60BB add ch,al
db 0x2E, 0xBC, 0xA2, 0x31               ; 60BD cs mov sp,0x31a2
db 0xB9, 0x00, 0x40                     ; 60C1 mov cx,0x4000
db 0x76, 0x3A                           ; 60C4 jna 0x6100
db 0x6B, 0x0B, 0x5E                     ; 60C6 imul cx,[bp+di],byte +0x5e
db 0xBC, 0x00, 0xE8                     ; 60C9 mov sp,0xe800
db 0x89, 0x04                           ; 60CC mov [si],ax
db 0x23, 0xC7                           ; 60CE and ax,di
db 0x0A, 0xC0                           ; 60D0 or al,al
db 0x00, 0x62, 0xAC                     ; 60D2 add [bp+si-0x54],ah
db 0xC5                                 ; 60D5 db 0xC5
db 0xEB, 0x78                           ; 60D6 jmp short 0x6150
db 0x2D, 0xC3, 0x80                     ; 60D8 sub ax,0x80c3
db 0x7A, 0x17                           ; 60DB jpe 0x60f4
db 0xB7, 0x26                           ; 60DD mov bh,0x26
db 0xD7                                 ; 60DF xlatb
db 0x58                                 ; 60E0 pop ax
db 0xC6                                 ; 60E1 db 0xC6
db 0x90                                 ; 60E2 nop
db 0xAC                                 ; 60E3 lodsb
db 0x6E                                 ; 60E4 outsb
db 0x32, 0x78, 0x86                     ; 60E5 xor bh,[bx+si-0x7a]
db 0x07                                 ; 60E8 pop es
db 0xCA, 0xB5, 0x57                     ; 60E9 retf 0x57b5
db 0x0A, 0x3F                           ; 60EC or bh,[bx]
db 0x16                                 ; 60EE push ss
db 0x68, 0x29, 0xCD                     ; 60EF push word 0xcd29
db 0xA2, 0xED, 0xCC                     ; 60F2 mov [0xcced],al
db 0xCE                                 ; 60F5 into
db 0x1B, 0xC2                           ; 60F6 sbb ax,dx
db 0x53                                 ; 60F8 push bx
db 0xD0, 0x85, 0x14, 0x40               ; 60F9 rol byte [di+0x4014],1
db 0x61                                 ; 60FD popa
db 0x51                                 ; 60FE push cx
db 0x59                                 ; 60FF pop cx
db 0x04, 0xD4                           ; 6100 add al,0xd4
db 0xA6                                 ; 6102 cmpsb
db 0x19, 0x90, 0xB9, 0xA5               ; 6103 sbb [bx+si+0xa5b9],dx
db 0x6F                                 ; 6107 outsw
db 0x25, 0xD7, 0x10                     ; 6108 and ax,0x10d7
db 0x20, 0xF4                           ; 610B and ah,dh
db 0x27                                 ; 610D daa
db 0x8F                                 ; 610E db 0x8F
db 0xCB                                 ; 610F retf
db 0x4E                                 ; 6110 dec si
db 0xDA, 0x0A                           ; 6111 fimul dword [bp+si]
db 0x94                                 ; 6113 xchg ax,sp
db 0xF8                                 ; 6114 clc
db 0x78, 0x39                           ; 6115 js 0x6150
db 0x3F                                 ; 6117 aas
db 0x01, 0xDE                           ; 6118 add si,bx
db 0x0C, 0xB9                           ; 611A or al,0xb9
db 0x36, 0xD7                           ; 611C ss xlatb
db 0x07                                 ; 611E pop es
db 0x8F                                 ; 611F db 0x8F
db 0x21, 0xE1                           ; 6120 and cx,sp
db 0x4F                                 ; 6122 dec di
db 0x67, 0x04, 0xCD                     ; 6123 a32 add al,0xcd
db 0xC9                                 ; 6126 leave
db 0xF2, 0x49                           ; 6127 repne dec cx
db 0xE4, 0x23                           ; 6129 in al,0x23
db 0x81, 0x45, 0x40, 0x7C, 0x6F         ; 612B add word [di+0x40],0x6f7c
db 0x7C, 0xE7                           ; 6130 jl 0x6119
db 0xB6, 0x70                           ; 6132 mov dh,0x70
db 0x2B, 0xA8, 0xAD, 0xC5               ; 6134 sub bp,[bx+si+0xc5ad]
db 0x1D, 0xEB, 0xE4                     ; 6138 sbb ax,0xe4eb
db 0x4C                                 ; 613B dec sp
db 0x36, 0x12, 0x19                     ; 613C adc bl,[ss:bx+di]
db 0x37                                 ; 613F aaa
db 0x45                                 ; 6140 inc bp
db 0xEE                                 ; 6141 out dx,al
db 0x1C, 0xE0                           ; 6142 sbb al,0xe0
db 0xC3                                 ; 6144 ret
db 0x56                                 ; 6145 push si
db 0xDF, 0x84, 0x76, 0xF1               ; 6146 fild word [si+0xf176]
db 0x12, 0x6C, 0x3A                     ; 614A adc ch,[si+0x3a]
db 0x96                                 ; 614D xchg ax,si
db 0x0B, 0x13                           ; 614E or dx,[bp+di]
db 0x1A, 0xF5                           ; 6150 sbb dh,ch
db 0x16                                 ; 6152 push ss
db 0x07                                 ; 6153 pop es
db 0xC9                                 ; 6154 leave
db 0x7B, 0xCE                           ; 6155 jpo 0x6125
db 0x97                                 ; 6157 xchg ax,di
db 0x40                                 ; 6158 inc ax
db 0xF8                                 ; 6159 clc
db 0xDC, 0x48, 0xBB                     ; 615A fmul qword [bx+si-0x45]
db 0x1A, 0xC2                           ; 615D sbb al,dl
db 0xBD, 0x70, 0xFB                     ; 615F mov bp,0xfb70
db 0x89, 0x0D                           ; 6162 mov [di],cx
db 0xB5, 0x50                           ; 6164 mov ch,0x50
db 0x99                                 ; 6166 cwd
db 0x76, 0x16                           ; 6167 jna 0x617f
db 0xFF, 0x00                           ; 6169 inc word [bx+si]
db 0x00, 0x00                           ; 616B add [bx+si],al
db 0x00, 0x00                           ; 616D add [bx+si],al
db 0x00, 0x00                           ; 616F add [bx+si],al
db 0x80, 0xF1, 0x04                     ; 6171 xor cl,0x4
db 0x35, 0x80, 0x04                     ; 6174 xor ax,0x480
db 0x9A, 0xF7, 0x19, 0x83, 0x24         ; 6177 call 0x2483:0x19f7
db 0x63, 0x43, 0x83                     ; 617C arpl [bp+di-0x7d],ax
db 0x75, 0xCD                           ; 617F jnz 0x614e
db 0x8D, 0x84, 0xA9, 0x7F               ; 6181 lea ax,[si+0x7fa9]
db 0x83, 0x82, 0x04, 0x00, 0x00         ; 6185 add word [bp+si+0x4],byte +0x0
db 0x00, 0x81, 0xE2, 0xB0               ; 618A add [bx+di+0xb0e2],al
db 0x4D                                 ; 618E dec bp
db 0x83, 0x0A, 0x72                     ; 618F or word [bp+si],byte +0x72
db 0x11, 0x83, 0xF4, 0x04               ; 6192 adc [bp+di+0x4f4],ax
db 0x35, 0x7F, 0x18                     ; 6196 xor ax,0x187f
db 0x72, 0x31                           ; 6199 jc 0x61cc
db 0x80, 0x2E, 0x65, 0x45, 0x25         ; 619B sub byte [0x4565],0x25
db 0x23, 0x21                           ; 61A0 and sp,[bx+di]
db 0x44                                 ; 61A2 inc sp
db 0x64, 0x2C, 0x30                     ; 61A3 fs sub al,0x30
db 0x00, 0x80, 0xC6, 0xA4               ; 61A6 add [bx+si+0xa4c6],al
db 0x7E, 0x8D                           ; 61AA jng 0x6139
db 0x03, 0x00                           ; 61AC add ax,[bx+si]
db 0x40                                 ; 61AE inc ax
db 0x7A, 0x10                           ; 61AF jpe 0x61c1
db 0xF3, 0x5A                           ; 61B1 rep pop dx
db 0x00, 0x00                           ; 61B3 add [bx+si],al
db 0xA0, 0x72, 0x4E                     ; 61B5 mov al,[0x4e72]
db 0x18, 0x09                           ; 61B8 sbb [bx+di],cl
db 0x00, 0x00                           ; 61BA add [bx+si],al
db 0x10, 0xA5, 0xD4, 0xE8               ; 61BC adc [di+0xe8d4],ah
db 0x00, 0x00                           ; 61C0 add [bx+si],al
db 0x00, 0xE8                           ; 61C2 add al,ch
db 0x76, 0x48                           ; 61C4 jna 0x620e
db 0x17                                 ; 61C6 pop ss
db 0x00, 0x00                           ; 61C7 add [bx+si],al
db 0x00, 0xE4                           ; 61C9 add ah,ah
db 0x0B, 0x54, 0x02                     ; 61CB or dx,[si+0x2]
db 0x00, 0x00                           ; 61CE add [bx+si],al
db 0x00, 0xCA                           ; 61D0 add dl,cl
db 0x9A, 0x3B, 0x00, 0x00, 0x00         ; 61D2 call 0x0:0x3b
db 0x00, 0xE1                           ; 61D7 add cl,ah
db 0xF5                                 ; 61D9 cmc
db 0x05, 0x00, 0x00                     ; 61DA add ax,0x0
db 0x00, 0x80, 0x96, 0x98               ; 61DD add [bx+si+0x9896],al
db 0x00, 0x00                           ; 61E1 add [bx+si],al
db 0x00, 0x00                           ; 61E3 add [bx+si],al
db 0x40                                 ; 61E5 inc ax
db 0x42                                 ; 61E6 inc dx
db 0x0F, 0x00, 0x00                     ; 61E7 sldt [bx+si]
db 0x00, 0x00                           ; 61EA add [bx+si],al
db 0x40                                 ; 61EC inc ax
db 0x42                                 ; 61ED inc dx
db 0x0F, 0xA0                           ; 61EE push fs
db 0x86, 0x01                           ; 61F0 xchg al,[bx+di]
db 0x10, 0x27                           ; 61F2 adc [bx],ah
db 0x00, 0x10                           ; 61F4 add [bx+si],dl
db 0x27                                 ; 61F6 daa
db 0xE8, 0x03, 0x64                     ; 61F7 call 0xc5fd
db 0x00, 0x0A                           ; 61FA add [bp+si],cl
db 0x00, 0x01                           ; 61FC add [bx+di],al
db 0x00, 0x00                           ; 61FE add [bx+si],al
db 0x00, 0x80, 0x90, 0xFF               ; 6200 add [bx+si+0xff90],al
db 0xFF                                 ; 6204 db 0xFF
db 0xFF                                 ; 6205 db 0xFF
db 0xFF                                 ; 6206 db 0xFF
db 0xFF                                 ; 6207 db 0xFF
db 0xFF                                 ; 6208 db 0xFF
db 0x7F, 0xFF                           ; 6209 jg 0x620a
db 0xFF                                 ; 620B db 0xFF
db 0xFF                                 ; 620C db 0xFF
db 0xFF                                 ; 620D db 0xFF
db 0xFF                                 ; 620E db 0xFF
db 0xFF                                 ; 620F db 0xFF
db 0xFF                                 ; 6210 db 0xFF
db 0xFF                                 ; 6211 db 0xFF
db 0xFF                                 ; 6212 db 0xFF
db 0x3B, 0xAA, 0x38, 0x81               ; 6213 cmp bp,[bp+si+0x8138]
db 0x07                                 ; 6217 pop es
db 0x7C, 0x88                           ; 6218 jl 0x61a2
db 0x59                                 ; 621A pop cx
db 0x74, 0xE0                           ; 621B jz 0x61fd
db 0x97                                 ; 621D xchg ax,di
db 0x26, 0x77, 0xC4                     ; 621E es ja 0x61e5
db 0x1D, 0x1E, 0x7A                     ; 6221 sbb ax,0x7a1e
db 0x5E                                 ; 6224 pop si
db 0x50                                 ; 6225 push ax
db 0x63, 0x7C, 0x1A                     ; 6226 arpl [si+0x1a],di
db 0xFE                                 ; 6229 db 0xFE
db 0x75, 0x7E                           ; 622A jnz 0x62aa
db 0x18, 0x72, 0x31                     ; 622C sbb [bp+si+0x31],dh
db 0x80, 0x00, 0x00                     ; 622F add byte [bx+si],0x0
db 0x00, 0x81, 0x05, 0xFB               ; 6232 add [bx+di+0xfb05],al
db 0xD7                                 ; 6236 xlatb
db 0x1E                                 ; 6237 push ds
db 0x86, 0x65, 0x26                     ; 6238 xchg ah,[di+0x26]
db 0x99                                 ; 623B cwd
db 0x87, 0x58, 0x34                     ; 623C xchg bx,[bx+si+0x34]
db 0x23, 0x87, 0xE1, 0x5D               ; 623F and ax,[bx+0x5de1]
db 0xA5                                 ; 6243 movsw
db 0x86, 0xDB                           ; 6244 xchg bl,bl
db 0x0F, 0x49, 0x83, 0x02, 0xD7         ; 6246 cmovns ax,[bp+di+0xd702]
db 0xB3, 0x5D                           ; 624B mov bl,0x5d
db 0x81, 0x00, 0x00, 0x80               ; 624D add word [bx+si],0x8000
db 0x81, 0x04, 0x62, 0x35               ; 6251 add word [si],0x3562
db 0x83, 0x7E, 0x50, 0x24               ; 6255 cmp word [bp+0x50],byte +0x24
db 0x4C                                 ; 6259 dec sp
db 0x7E, 0x79                           ; 625A jng 0x62d5
db 0xA9, 0xAA, 0x7F                     ; 625C test ax,0x7faa
db 0x00, 0x00                           ; 625F add [bx+si],al
db 0x00, 0x81, 0x0B, 0x44               ; 6261 add [bx+di+0x440b],al
db 0x4E                                 ; 6265 dec si
db 0x6E                                 ; 6266 outsb
db 0x83, 0xF9, 0x22                     ; 6267 cmp cx,byte +0x22
db 0x7E, 0xFD                           ; 626A jng 0x6269
db 0x43                                 ; 626C inc bx
db 0x03, 0xC3                           ; 626D add ax,bx
db 0x9E                                 ; 626F sahf
db 0x26, 0x01, 0x00                     ; 6270 add [es:bx+si],ax
db 0x00, 0x30                           ; 6273 add [bx+si],dh
db 0x31, 0x32                           ; 6275 xor [bp+si],si
db 0x33, 0x34                           ; 6277 xor si,[si]
db 0x35, 0x36, 0x37                     ; 6279 xor ax,0x3736
db 0x38, 0x39                           ; 627C cmp [bx+di],bh
db 0x41                                 ; 627E inc cx
db 0x42                                 ; 627F inc dx
db 0x43                                 ; 6280 inc bx
db 0x44                                 ; 6281 inc sp
db 0x45                                 ; 6282 inc bp
db 0x46                                 ; 6283 inc si
db 0xBA, 0x3B, 0xAA                     ; 6284 mov dx,0xaa3b
db 0xBB, 0x38, 0x81                     ; 6287 mov bx,0x8138
db 0xE8, 0x72, 0x0A                     ; 628A call 0x6cff
db 0xA0, 0xA6, 0x04                     ; 628D mov al,[0x4a6]
db 0x3C, 0x88                           ; 6290 cmp al,0x88
db 0x73, 0x3C                           ; 6292 jnc 0x62d0
db 0x3C, 0x68                           ; 6294 cmp al,0x68
db 0x72, 0x4B                           ; 6296 jc 0x62e3
db 0xFF, 0x36, 0xA3, 0x04               ; 6298 push word [0x4a3]
db 0xFF, 0x36, 0xA5, 0x04               ; 629C push word [0x4a5]
db 0xE8, 0x60, 0x10                     ; 62A0 call 0x7303
db 0x8A, 0xE2                           ; 62A3 mov ah,dl
db 0x80, 0xC4, 0x81                     ; 62A5 add ah,0x81
db 0x74, 0x23                           ; 62A8 jz 0x62cd
db 0x50                                 ; 62AA push ax
db 0xF6, 0x06, 0xA7, 0x04, 0x80         ; 62AB test byte [0x4a7],0x80
db 0xE8, 0x43, 0x10                     ; 62B0 call 0x72f6
db 0x32, 0xE4                           ; 62B3 xor ah,ah
db 0xE8, 0xE6, 0x11                     ; 62B5 call 0x749e
db 0x58                                 ; 62B8 pop ax
db 0x5B                                 ; 62B9 pop bx
db 0x5A                                 ; 62BA pop dx
db 0x50                                 ; 62BB push ax
db 0xE8, 0xA8, 0x04                     ; 62BC call 0x6767
db 0xBB, 0x17, 0x62                     ; 62BF mov bx,0x6217
db 0xE8, 0x8D, 0x12                     ; 62C2 call 0x7552
db 0x5B                                 ; 62C5 pop bx
db 0x33, 0xD2                           ; 62C6 xor dx,dx
db 0x8A, 0xDA                           ; 62C8 mov bl,dl
db 0xE9, 0x32, 0x0A                     ; 62CA jmp 0x6cff
db 0x83, 0xC4, 0x04                     ; 62CD add sp,byte +0x4
db 0x80, 0x26, 0xA5, 0x04, 0x80         ; 62D0 and byte [0x4a5],0x80
db 0x74, 0x03                           ; 62D5 jz 0x62da
db 0xE9, 0x91, 0x18                     ; 62D7 jmp 0x7b6b
db 0x32, 0xE4                           ; 62DA xor ah,ah
db 0x88, 0x26, 0xA7, 0x04               ; 62DC mov [0x4a7],ah
db 0xE9, 0xF9, 0x11                     ; 62E0 jmp 0x74dc
db 0xBF, 0xA3, 0x04                     ; 62E3 mov di,0x4a3
db 0x90                                 ; 62E6 nop
db 0x33, 0xC0                           ; 62E7 xor ax,ax
db 0xFC                                 ; 62E9 cld
db 0xAB                                 ; 62EA stosw
db 0xC7, 0x05, 0x00, 0x81               ; 62EB mov word [di],0x8100
db 0xC3                                 ; 62EF ret
db 0xE8, 0xAF, 0x18                     ; 62F0 call 0x7ba2
db 0x75, 0x03                           ; 62F3 jnz 0x62f8
db 0xE9, 0xDE, 0xA4                     ; 62F5 jmp 0x7d6
db 0xC3                                 ; 62F8 ret
db 0xFC                                 ; 62F9 cld
db 0xAB                                 ; 62FA stosw
db 0xC7, 0x05, 0x00, 0x81               ; 62FB mov word [di],0x8100
db 0xC3                                 ; 62FF ret
db 0xE9, 0x1A, 0xAC                     ; 6300 jmp 0xf1d
db 0xCD, 0xB9                           ; 6303 int 0xb9
db 0x80, 0x36, 0xA5, 0x04, 0x80         ; 6305 xor byte [0x4a5],0x80
db 0x80, 0x36, 0xB1, 0x04, 0x80         ; 630A xor byte [0x4b1],0x80
db 0xE9, 0x8A, 0x03                     ; 630F jmp 0x669c
db 0xCD, 0xBA                           ; 6312 int 0xba
db 0x87, 0xD9                           ; 6314 xchg bx,cx
db 0xE9, 0x5A, 0x04                     ; 6316 jmp 0x6773
db 0xCD, 0xBB                           ; 6319 int 0xbb
db 0x87, 0xD9                           ; 631B xchg bx,cx
db 0xE9, 0x47, 0x04                     ; 631D jmp 0x6767
db 0xCD, 0xBC                           ; 6320 int 0xbc
db 0xE8, 0x33, 0x19                     ; 6322 call 0x7c58
db 0xE9, 0x1B, 0x05                     ; 6325 jmp 0x6843
db 0x87, 0xD9                           ; 6328 xchg bx,cx
db 0xE9, 0xB1, 0x05                     ; 632A jmp 0x68de
db 0xCD, 0xBD                           ; 632D int 0xbd
db 0x89, 0x1E, 0xA3, 0x04               ; 632F mov [0x4a3],bx
db 0xE9, 0x6B, 0x08                     ; 6333 jmp 0x6ba1
db 0xCD, 0xBE                           ; 6336 int 0xbe
db 0x52                                 ; 6338 push dx
db 0x98                                 ; 6339 cbw
db 0x8B, 0xD0                           ; 633A mov dx,ax
db 0xE8, 0xEE, 0x08                     ; 633C call 0x6c2d
db 0x5A                                 ; 633F pop dx
db 0xC3                                 ; 6340 ret
db 0xCD, 0xBF                           ; 6341 int 0xbf
db 0x87, 0xD9                           ; 6343 xchg bx,cx
db 0xE9, 0xB7, 0x09                     ; 6345 jmp 0x6cff
db 0xCD, 0xC0                           ; 6348 int 0xc0
db 0x87, 0xD9                           ; 634A xchg bx,cx
db 0xE9, 0xED, 0x0D                     ; 634C jmp 0x713c
db 0x81, 0xFB, 0x00, 0x80               ; 634F cmp bx,0x8000
db 0x75, 0x13                           ; 6353 jnz 0x6368
db 0xCD, 0xC1                           ; 6355 int 0xc1
db 0xE8, 0xD3, 0x08                     ; 6357 call 0x6c2d
db 0x33, 0xD2                           ; 635A xor dx,dx
db 0xBB, 0x80, 0x90                     ; 635C mov bx,0x9080
db 0xE8, 0x05, 0x04                     ; 635F call 0x6767
db 0xE8, 0x4B, 0x1A                     ; 6362 call 0x7db0
db 0xE9, 0x45, 0x08                     ; 6365 jmp 0x6bad
db 0xF7, 0xDB                           ; 6368 neg bx
db 0x53                                 ; 636A push bx
db 0x03, 0xDA                           ; 636B add bx,dx
db 0x70, 0x04                           ; 636D jo 0x6373
db 0x58                                 ; 636F pop ax
db 0xE9, 0x99, 0x01                     ; 6370 jmp 0x650c
db 0xCD, 0xC2                           ; 6373 int 0xc2
db 0xE8, 0xB5, 0x08                     ; 6375 call 0x6c2d
db 0x5A                                 ; 6378 pop dx
db 0xFF, 0x36, 0xA3, 0x04               ; 6379 push word [0x4a3]
db 0xFF, 0x36, 0xA5, 0x04               ; 637D push word [0x4a5]
db 0xE8, 0xA9, 0x08                     ; 6381 call 0x6c2d
db 0x5B                                 ; 6384 pop bx
db 0x5A                                 ; 6385 pop dx
db 0xE9, 0xEA, 0x03                     ; 6386 jmp 0x6773
db 0x8B, 0xC3                           ; 6389 mov ax,bx
db 0x52                                 ; 638B push dx
db 0xF7, 0xEA                           ; 638C imul dx
db 0x5A                                 ; 638E pop dx
db 0x72, 0x05                           ; 638F jc 0x6396
db 0x8B, 0xD8                           ; 6391 mov bx,ax
db 0xE9, 0x76, 0x01                     ; 6393 jmp 0x650c
db 0xCD, 0xC3                           ; 6396 int 0xc3
db 0x53                                 ; 6398 push bx
db 0xE8, 0x91, 0x08                     ; 6399 call 0x6c2d
db 0x5A                                 ; 639C pop dx
db 0xFF, 0x36, 0xA3, 0x04               ; 639D push word [0x4a3]
db 0xFF, 0x36, 0xA5, 0x04               ; 63A1 push word [0x4a5]
db 0xE8, 0x85, 0x08                     ; 63A5 call 0x6c2d
db 0x5B                                 ; 63A8 pop bx
db 0x5A                                 ; 63A9 pop dx
db 0xE9, 0x52, 0x09                     ; 63AA jmp 0x6cff
db 0x0B, 0xDB                           ; 63AD or bx,bx
db 0x75, 0x0C                           ; 63AF jnz 0x63bd
db 0x88, 0x36, 0xA7, 0x04               ; 63B1 mov [0x4a7],dh
db 0xC6, 0x06, 0xFB, 0x02, 0x04         ; 63B5 mov byte [0x2fb],0x4
db 0xE9, 0x2D, 0x11                     ; 63BA jmp 0x74ea
db 0x89, 0x1E, 0xA3, 0x04               ; 63BD mov [0x4a3],bx
db 0xB8, 0x00, 0x00                     ; 63C1 mov ax,0x0
db 0xA3, 0xA5, 0x04                     ; 63C4 mov [0x4a5],ax
db 0x92                                 ; 63C7 xchg ax,dx
db 0x0B, 0xC0                           ; 63C8 or ax,ax
db 0x79, 0x03                           ; 63CA jns 0x63cf
db 0xBA, 0xFF, 0xFF                     ; 63CC mov dx,0xffff
db 0x0B, 0xDB                           ; 63CF or bx,bx
db 0x79, 0x06                           ; 63D1 jns 0x63d9
db 0xC7, 0x06, 0xA5, 0x04, 0xFF, 0xFF   ; 63D3 mov word [0x4a5],0xffff
db 0xF7, 0x3E, 0xA3, 0x04               ; 63D9 idiv word [0x4a3]
db 0x8B, 0xD8                           ; 63DD mov bx,ax
db 0xE9, 0x2A, 0x01                     ; 63DF jmp 0x650c
db 0x87, 0xD9                           ; 63E2 xchg bx,cx
db 0xE8, 0xD7, 0x18                     ; 63E4 call 0x7cbe
db 0x87, 0xD9                           ; 63E7 xchg bx,cx
db 0xC3                                 ; 63E9 ret
db 0x53                                 ; 63EA push bx
db 0xE8, 0x45, 0x1A                     ; 63EB call 0x7e33
db 0x5B                                 ; 63EE pop bx
db 0x83, 0xC3, 0x04                     ; 63EF add bx,byte +0x4
db 0xC3                                 ; 63F2 ret
db 0x8B, 0x16, 0xA3, 0x04               ; 63F3 mov dx,[0x4a3]
db 0x8B, 0x0E, 0xA5, 0x04               ; 63F7 mov cx,[0x4a5]
db 0xC3                                 ; 63FB ret
db 0x9C                                 ; 63FC pushf
db 0x53                                 ; 63FD push bx
db 0xE8, 0x19, 0x1A                     ; 63FE call 0x7e1a
db 0x5B                                 ; 6401 pop bx
db 0x83, 0xC3, 0x04                     ; 6402 add bx,byte +0x4
db 0x9D                                 ; 6405 popf
db 0xC3                                 ; 6406 ret
db 0xE8, 0xAB, 0xBE                     ; 6407 call 0x22b5
db 0x89, 0x1E, 0xA3, 0x04               ; 640A mov [0x4a3],bx
db 0xE9, 0x67, 0x0A                     ; 640E jmp 0x6e78
db 0xE8, 0xA1, 0xBE                     ; 6411 call 0x22b5
db 0x89, 0x1E, 0xA3, 0x04               ; 6414 mov [0x4a3],bx
db 0xE9, 0x65, 0x0A                     ; 6418 jmp 0x6e80
db 0xCD, 0xC4                           ; 641B int 0xc4
db 0x8B, 0x17                           ; 641D mov dx,[bx]
db 0x8B, 0x9F, 0x02, 0x00               ; 641F mov bx,[bx+0x2]
db 0xE9, 0x4D, 0x03                     ; 6423 jmp 0x6773
db 0x5E                                 ; 6426 pop si
db 0xFF, 0x36, 0xA3, 0x04               ; 6427 push word [0x4a3]
db 0xFF, 0x36, 0xA5, 0x04               ; 642B push word [0x4a5]
db 0xFF, 0xE6                           ; 642F jmp si
db 0xCD, 0xC5                           ; 6431 int 0xc5
db 0x87, 0xD9                           ; 6433 xchg bx,cx
db 0xE8, 0xA0, 0x18                     ; 6435 call 0x7cd8
db 0x87, 0xD9                           ; 6438 xchg bx,cx
db 0xC3                                 ; 643A ret
db 0xE8, 0x64, 0x17                     ; 643B call 0x7ba2
db 0x74, 0x03                           ; 643E jz 0x6443
db 0xE9, 0x93, 0xA3                     ; 6440 jmp 0x7d6
db 0xC3                                 ; 6443 ret

L_6444:
  xchg    bx, dx
  call    L_6516

  xor     al, al
  mov     ch, 0x98
  int     0xc6

  mov     bx, 0x04a6
  mov     cl, al

db 0x88, 0x2F                           ; 6454 mov [bx],ch
db 0xB5, 0x00                           ; 6456 mov ch,0x0
db 0x43                                 ; 6458 inc bx
db 0x88, 0x2F                           ; 6459 mov [bx],ch
db 0xD0, 0xD0                           ; 645B rcl al,1
db 0xCD, 0xC7                           ; 645D int 0xc7
db 0x73, 0x03                           ; 645F jnc 0x6464
db 0xE8, 0x20, 0x01                     ; 6461 call 0x6584
db 0x8A, 0xE5                           ; 6464 mov ah,ch
db 0x8A, 0xD9                           ; 6466 mov bl,cl
db 0xE9, 0x33, 0x10                     ; 6468 jmp 0x749e
db 0xE8, 0x92, 0xFE                     ; 646B call 0x6300
db 0x53                                 ; 646E push bx
db 0x33, 0xDB                           ; 646F xor bx,bx
db 0x89, 0x1E, 0xA3, 0x04               ; 6471 mov [0x4a3],bx
db 0xB7, 0x81                           ; 6475 mov bh,0x81
db 0x89, 0x1E, 0xA5, 0x04               ; 6477 mov [0x4a5],bx
db 0xC6, 0x06, 0xFB, 0x02, 0x04         ; 647B mov byte [0x2fb],0x4
db 0xE8, 0x04, 0x14                     ; 6480 call 0x7887
db 0x5B                                 ; 6483 pop bx
db 0xC6, 0x06, 0xFB, 0x02, 0x04         ; 6484 mov byte [0x2fb],0x4
  ret

db 0x8B, 0xC1                           ; 648A mov ax,cx
db 0xF7, 0xE2                           ; 648C mul dx
db 0x92                                 ; 648E xchg ax,dx
db 0x73, 0x03                           ; 648F jnc 0x6494
db 0xE9, 0x25, 0xD5                     ; 6491 jmp 0x39b9
db 0xC3                                 ; 6494 ret
db 0xBB, 0xAB, 0x04                     ; 6495 mov bx,0x4ab
db 0xBA, 0xB7, 0x64                     ; 6498 mov dx,0x64b7
db 0xE9, 0x06, 0x00                     ; 649B jmp 0x64a4
db 0xBB, 0xAB, 0x04                     ; 649E mov bx,0x4ab
db 0xBA, 0xB9, 0x64                     ; 64A1 mov dx,0x64b9
db 0x52                                 ; 64A4 push dx
db 0xBA, 0xA3, 0x04                     ; 64A5 mov dx,0x4a3
db 0xE8, 0xF7, 0x16                     ; 64A8 call 0x7ba2
db 0x72, 0x03                           ; 64AB jc 0x64b0
db 0xBA, 0x9F, 0x04                     ; 64AD mov dx,0x49f
db 0xC3                                 ; 64B0 ret
db 0x8A, 0xCD                           ; 64B1 mov cl,ch
db 0x32, 0xED                           ; 64B3 xor ch,ch
db 0xEB, 0x08                           ; 64B5 jmp short 0x64bf
db 0x87, 0xDA                           ; 64B7 xchg bx,dx
db 0xA0, 0xFB, 0x02                     ; 64B9 mov al,[0x2fb]
db 0x98                                 ; 64BC cbw
db 0x8B, 0xC8                           ; 64BD mov cx,ax
db 0xFC                                 ; 64BF cld
db 0x8B, 0xF2                           ; 64C0 mov si,dx
db 0x8B, 0xFB                           ; 64C2 mov di,bx
db 0xF3, 0xA4                           ; 64C4 rep movsb
db 0x8B, 0xD6                           ; 64C6 mov dx,si
db 0x8B, 0xDF                           ; 64C8 mov bx,di
db 0xC3                                 ; 64CA ret
db 0xE8, 0x36, 0xC8                     ; 64CB call 0x2d04
db 0x8B, 0xF1                           ; 64CE mov si,cx
db 0x8B, 0xFB                           ; 64D0 mov di,bx
db 0xFD                                 ; 64D2 std
db 0x2B, 0xCA                           ; 64D3 sub cx,dx
db 0x41                                 ; 64D5 inc cx
db 0xF3, 0xA4                           ; 64D6 rep movsb
db 0x8B, 0xDA                           ; 64D8 mov bx,dx
db 0x8B, 0xCF                           ; 64DA mov cx,di
db 0x41                                 ; 64DC inc cx
db 0xFC                                 ; 64DD cld
db 0xC3                                 ; 64DE ret
db 0x9C                                 ; 64DF pushf
db 0x49                                 ; 64E0 dec cx
db 0x9D                                 ; 64E1 popf
db 0xC3                                 ; 64E2 ret
db 0xE8, 0xBC, 0x16                     ; 64E3 call 0x7ba2
db 0x75, 0x03                           ; 64E6 jnz 0x64eb
db 0xE9, 0xEB, 0xA2                     ; 64E8 jmp 0x7d6
db 0xCD, 0xC8                           ; 64EB int 0xc8
db 0x78, 0x03                           ; 64ED js 0x64f2
db 0xE9, 0x83, 0x16                     ; 64EF jmp 0x7b75
db 0xA1, 0xA3, 0x04                     ; 64F2 mov ax,[0x4a3]
db 0x0B, 0xC0                           ; 64F5 or ax,ax
db 0x74, 0x06                           ; 64F7 jz 0x64ff
db 0xB0, 0x01                           ; 64F9 mov al,0x1
db 0x79, 0x02                           ; 64FB jns 0x64ff
db 0xB0, 0xFF                           ; 64FD mov al,0xff
db 0xC3                                 ; 64FF ret
db 0x33, 0xC0                           ; 6500 xor ax,ax
db 0x0B, 0xDB                           ; 6502 or bx,bx
db 0x75, 0xF3                           ; 6504 jnz 0x64f9
db 0xC3                                 ; 6506 ret
db 0x5B                                 ; 6507 pop bx
db 0xC3                                 ; 6508 ret
db 0x98                                 ; 6509 cbw
db 0x8B, 0xD8                           ; 650A mov bx,ax
db 0xC6, 0x06, 0xFB, 0x02, 0x02         ; 650C mov byte [0x2fb],0x2
db 0x89, 0x1E, 0xA3, 0x04               ; 6511 mov [0x4a3],bx
db 0xC3                                 ; 6515 ret

L_6516:
  mov     byte [0x02fb], 4
  ret

db 0xE8, 0xC4, 0xFF                     ; 651C call 0x64e3
db 0xE9, 0xE7, 0xFF                     ; 651F jmp 0x6509
db 0xCD, 0xC9                           ; 6522 int 0xc9
db 0x5B                                 ; 6524 pop bx
db 0x5A                                 ; 6525 pop dx
db 0xE9, 0xB5, 0x03                     ; 6526 jmp 0x68de
db 0xE8, 0x25, 0x06                     ; 6529 call 0x6b51
db 0x5B                                 ; 652C pop bx
db 0x5A                                 ; 652D pop dx
db 0xE9, 0x0B, 0x0C                     ; 652E jmp 0x713c
db 0xB9, 0x04, 0x00                     ; 6531 mov cx,0x4
db 0xE9, 0x88, 0xFF                     ; 6534 jmp 0x64bf
db 0x9C                                 ; 6537 pushf
db 0x8A, 0x17                           ; 6538 mov dl,[bx]
db 0x43                                 ; 653A inc bx
db 0x9D                                 ; 653B popf
db 0x9C                                 ; 653C pushf
db 0x8A, 0x37                           ; 653D mov dh,[bx]
db 0x43                                 ; 653F inc bx
db 0x8B, 0x0F                           ; 6540 mov cx,[bx]
db 0x43                                 ; 6542 inc bx
db 0x9D                                 ; 6543 popf
db 0x9C                                 ; 6544 pushf
db 0x43                                 ; 6545 inc bx
db 0x9D                                 ; 6546 popf
db 0xC3                                 ; 6547 ret
db 0x53                                 ; 6548 push bx
db 0xBB, 0x28, 0x07                     ; 6549 mov bx,0x728
db 0xE8, 0x06, 0x16                     ; 654C call 0x7b55
db 0x5B                                 ; 654F pop bx
db 0xB9, 0xB6, 0x26                     ; 6550 mov cx,0x26b6
db 0x51                                 ; 6553 push cx
db 0xE8, 0xB5, 0xFF                     ; 6554 call 0x650c
db 0x32, 0xC0                           ; 6557 xor al,al
db 0xCD, 0xCA                           ; 6559 int 0xca
db 0xA2, 0x31, 0x03                     ; 655B mov [0x331],al
db 0xBB, 0xB4, 0x04                     ; 655E mov bx,0x4b4
db 0xC6, 0x07, 0x20                     ; 6561 mov byte [bx],0x20
db 0x0A, 0x07                           ; 6564 or al,[bx]
db 0x43                                 ; 6566 inc bx
db 0xC6, 0x07, 0x30                     ; 6567 mov byte [bx],0x30
db 0xE9, 0x6A, 0x0B                     ; 656A jmp 0x70d7
db 0xCD, 0xCB                           ; 656D int 0xcb
db 0xA0, 0xA5, 0x04                     ; 656F mov al,[0x4a5]
db 0xEB, 0x09                           ; 6572 jmp short 0x657d
db 0xCD, 0xCC                           ; 6574 int 0xcc
db 0xE8, 0xE5, 0x17                     ; 6576 call 0x7d5e
db 0x74, 0x08                           ; 6579 jz 0x6583
db 0xF6, 0xD0                           ; 657B not al
db 0xD0, 0xE0                           ; 657D shl al,1
db 0x1A, 0xC0                           ; 657F sbb al,al
db 0x74, 0x3D                           ; 6581 jz 0x65c0
db 0xC3                                 ; 6583 ret
db 0xCD, 0xCD                           ; 6584 int 0xcd
db 0x80, 0x36, 0xA7, 0x04, 0x80         ; 6586 xor byte [0x4a7],0x80
db 0x33, 0xDB                           ; 658B xor bx,bx
db 0xF6, 0xDD                           ; 658D neg ch
db 0x8B, 0xC3                           ; 658F mov ax,bx
db 0x1B, 0xC2                           ; 6591 sbb ax,dx
db 0x8B, 0xD0                           ; 6593 mov dx,ax
db 0x8A, 0xC3                           ; 6595 mov al,bl
db 0x1A, 0xC1                           ; 6597 sbb al,cl
db 0x8A, 0xC8                           ; 6599 mov cl,al
db 0xC3                                 ; 659B ret
db 0xE8, 0x03, 0x16                     ; 659C call 0x7ba2
db 0x78, 0xFA                           ; 659F js 0x659b
db 0xCD, 0xCE                           ; 65A1 int 0xce
db 0xE8, 0xCF, 0x15                     ; 65A3 call 0x7b75
db 0x78, 0x03                           ; 65A6 js 0x65ab
db 0xE9, 0xD5, 0x18                     ; 65A8 jmp 0x7e80
db 0xE8, 0x02, 0x18                     ; 65AB call 0x7db0
db 0xE8, 0xCF, 0x18                     ; 65AE call 0x7e80
db 0xE9, 0xFC, 0x17                     ; 65B1 jmp 0x7db0
db 0x8B, 0xC3                           ; 65B4 mov ax,bx
db 0x2B, 0xC2                           ; 65B6 sub ax,dx
db 0x74, 0x0E                           ; 65B8 jz 0x65c8
db 0x70, 0x07                           ; 65BA jo 0x65c3
db 0x78, 0x07                           ; 65BC js 0x65c5
db 0x32, 0xC0                           ; 65BE xor al,al
db 0xFE, 0xC0                           ; 65C0 inc al
db 0xC3                                 ; 65C2 ret
db 0x78, 0xF9                           ; 65C3 js 0x65be
db 0xF9                                 ; 65C5 stc
db 0x1A, 0xC0                           ; 65C6 sbb al,al
db 0xC3                                 ; 65C8 ret
db 0x3B, 0xDA                           ; 65C9 cmp bx,dx
db 0x75, 0x05                           ; 65CB jnz 0x65d2
db 0x33, 0xDB                           ; 65CD xor bx,bx
db 0xE9, 0x26, 0x00                     ; 65CF jmp 0x65f8
db 0x8B, 0xC2                           ; 65D2 mov ax,dx
db 0x89, 0x1E, 0xA3, 0x04               ; 65D4 mov [0x4a3],bx
db 0x0B, 0xDB                           ; 65D8 or bx,bx
db 0xE8, 0xD3, 0xF9                     ; 65DA call 0x5fb0
db 0x90                                 ; 65DD nop
db 0x90                                 ; 65DE nop
db 0x90                                 ; 65DF nop
db 0x79, 0x06                           ; 65E0 jns 0x65e8
db 0xC7, 0x06, 0xA5, 0x04, 0xFF, 0xFF   ; 65E2 mov word [0x4a5],0xffff
db 0x0B, 0xC0                           ; 65E8 or ax,ax
db 0xBA, 0x00, 0x00                     ; 65EA mov dx,0x0
db 0x79, 0x03                           ; 65ED jns 0x65f2
db 0xBA, 0xFF, 0xFF                     ; 65EF mov dx,0xffff
db 0xF7, 0x3E, 0xA3, 0x04               ; 65F2 idiv word [0x4a3]
db 0x8B, 0xDA                           ; 65F6 mov bx,dx
db 0x89, 0x1E, 0xA3, 0x04               ; 65F8 mov [0x4a3],bx
db 0xC3                                 ; 65FC ret
db 0xAD                                 ; 65FD lodsw
db 0x3A, 0xE1                           ; 65FE cmp ah,cl
db 0x74, 0x11                           ; 6600 jz 0x6613
db 0x46                                 ; 6602 inc si
db 0x02, 0x04                           ; 6603 add al,[si]
db 0xFE, 0xC0                           ; 6605 inc al
db 0x98                                 ; 6607 cbw
db 0x03, 0xF0                           ; 6608 add si,ax
db 0x3B, 0xF5                           ; 660A cmp si,bp
db 0x75, 0xEF                           ; 660C jnz 0x65fd
db 0x8B, 0xD6                           ; 660E mov dx,si
db 0xE9, 0x39, 0xD2                     ; 6610 jmp 0x384c
db 0x3A, 0x06, 0xFB, 0x02               ; 6613 cmp al,[0x2fb]
db 0x75, 0xE9                           ; 6617 jnz 0x6602
db 0x3A, 0x2C                           ; 6619 cmp ch,[si]
db 0x75, 0xE5                           ; 661B jnz 0x6602
db 0x46                                 ; 661D inc si
db 0x8A, 0xD0                           ; 661E mov dl,al
db 0xAC                                 ; 6620 lodsb
db 0x3A, 0x06, 0x8E, 0x00               ; 6621 cmp al,[0x8e]
db 0x74, 0x04                           ; 6625 jz 0x662b
db 0x02, 0xC2                           ; 6627 add al,dl
db 0xEB, 0xDC                           ; 6629 jmp short 0x6607
db 0x0A, 0xC0                           ; 662B or al,al
db 0x74, 0x10                           ; 662D jz 0x663f
db 0x98                                 ; 662F cbw
db 0x91                                 ; 6630 xchg ax,cx
db 0xBF, 0x8F, 0x00                     ; 6631 mov di,0x8f
db 0xF3, 0xA6                           ; 6634 repe cmpsb
db 0x91                                 ; 6636 xchg ax,cx
db 0x74, 0x06                           ; 6637 jz 0x663f
db 0x03, 0xF0                           ; 6639 add si,ax
db 0x8A, 0xC2                           ; 663B mov al,dl
db 0xEB, 0xC8                           ; 663D jmp short 0x6607
db 0x8B, 0xD6                           ; 663F mov dx,si
db 0x5B                                 ; 6641 pop bx
db 0xC3                                 ; 6642 ret
db 0x8B, 0xF3                           ; 6643 mov si,bx
db 0x8B, 0x2E, 0x4B, 0x04               ; 6645 mov bp,[0x44b]
db 0xFC                                 ; 6649 cld
db 0xEB, 0xBE                           ; 664A jmp short 0x660a
db 0x8B, 0xF3                           ; 664C mov si,bx
db 0x8B, 0x2E, 0x5C, 0x03               ; 664E mov bp,[0x35c]
db 0xFC                                 ; 6652 cld
db 0xE9, 0x0D, 0x00                     ; 6653 jmp 0x6663
db 0xAD                                 ; 6656 lodsw
db 0x3A, 0xE1                           ; 6657 cmp ah,cl
db 0x74, 0x11                           ; 6659 jz 0x666c
db 0x46                                 ; 665B inc si
db 0xAC                                 ; 665C lodsb
db 0x98                                 ; 665D cbw
db 0x03, 0xF0                           ; 665E add si,ax
db 0xAD                                 ; 6660 lodsw
db 0x03, 0xF0                           ; 6661 add si,ax
db 0x3B, 0xEE                           ; 6663 cmp bp,si
db 0x75, 0xEF                           ; 6665 jnz 0x6656
db 0x8B, 0xDE                           ; 6667 mov bx,si
db 0xE9, 0x53, 0xD3                     ; 6669 jmp 0x39bf
db 0x3A, 0x06, 0xFB, 0x02               ; 666C cmp al,[0x2fb]
db 0x75, 0xE9                           ; 6670 jnz 0x665b
db 0x3A, 0x2C                           ; 6672 cmp ch,[si]
db 0x75, 0xE5                           ; 6674 jnz 0x665b
db 0x46                                 ; 6676 inc si
db 0xAC                                 ; 6677 lodsb
db 0x3A, 0x06, 0x8E, 0x00               ; 6678 cmp al,[0x8e]
db 0x75, 0xDF                           ; 667C jnz 0x665d
db 0x0A, 0xC0                           ; 667E or al,al
db 0x74, 0x0E                           ; 6680 jz 0x6690
db 0x98                                 ; 6682 cbw
db 0x91                                 ; 6683 xchg ax,cx
db 0xBF, 0x8F, 0x00                     ; 6684 mov di,0x8f
db 0xF3, 0xA6                           ; 6687 repe cmpsb
db 0x91                                 ; 6689 xchg ax,cx
db 0x74, 0x04                           ; 668A jz 0x6690
db 0x03, 0xF0                           ; 668C add si,ax
db 0xEB, 0xD0                           ; 668E jmp short 0x6660
db 0xAD                                 ; 6690 lodsw
db 0x8B, 0xD0                           ; 6691 mov dx,ax
db 0x8B, 0xDE                           ; 6693 mov bx,si
db 0xE9, 0x05, 0xD3                     ; 6695 jmp 0x399d
db 0xE8, 0x18, 0x16                     ; 6698 call 0x7cb3
db 0xC3                                 ; 669B ret
db 0xA1, 0xA5, 0x04                     ; 669C mov ax,[0x4a5]
db 0x0A, 0xE4                           ; 669F or ah,ah
db 0x74, 0xF5                           ; 66A1 jz 0x6698
db 0x80, 0x36, 0xA5, 0x04, 0x80         ; 66A3 xor byte [0x4a5],0x80
db 0xCD, 0xD7                           ; 66A8 int 0xd7
db 0xB0, 0x00                           ; 66AA mov al,0x0
db 0xA2, 0x9E, 0x04                     ; 66AC mov [0x49e],al
db 0xA2, 0xAA, 0x04                     ; 66AF mov [0x4aa],al
db 0xA0, 0xB2, 0x04                     ; 66B2 mov al,[0x4b2]
db 0x0A, 0xC0                           ; 66B5 or al,al
db 0x74, 0xE2                           ; 66B7 jz 0x669b
db 0xA1, 0xA5, 0x04                     ; 66B9 mov ax,[0x4a5]
db 0x0A, 0xE4                           ; 66BC or ah,ah
db 0x74, 0xD8                           ; 66BE jz 0x6698
db 0x8B, 0x1E, 0xB1, 0x04               ; 66C0 mov bx,[0x4b1]
db 0x80, 0x0E, 0xA5, 0x04, 0x80         ; 66C4 or byte [0x4a5],0x80
db 0x80, 0x0E, 0xB1, 0x04, 0x80         ; 66C9 or byte [0x4b1],0x80
db 0x8A, 0xCC                           ; 66CE mov cl,ah
db 0x2A, 0xCF                           ; 66D0 sub cl,bh
db 0xA2, 0xA7, 0x04                     ; 66D2 mov [0x4a7],al
db 0x74, 0x22                           ; 66D5 jz 0x66f9
db 0x73, 0x12                           ; 66D7 jnc 0x66eb
db 0x86, 0xC3                           ; 66D9 xchg al,bl
db 0xF6, 0xD9                           ; 66DB neg cl
db 0xA2, 0xA7, 0x04                     ; 66DD mov [0x4a7],al
db 0x88, 0x3E, 0xA6, 0x04               ; 66E0 mov [0x4a6],bh
db 0x50                                 ; 66E4 push ax
db 0x51                                 ; 66E5 push cx
db 0xE8, 0x6F, 0x15                     ; 66E6 call 0x7c58
db 0x59                                 ; 66E9 pop cx
db 0x58                                 ; 66EA pop ax
db 0x80, 0xF9, 0x39                     ; 66EB cmp cl,0x39
db 0x73, 0x5F                           ; 66EE jnc 0x674f
db 0x53                                 ; 66F0 push bx
db 0xF8                                 ; 66F1 clc
db 0xE8, 0x21, 0x15                     ; 66F2 call 0x7c16
db 0xA0, 0xA7, 0x04                     ; 66F5 mov al,[0x4a7]
db 0x5B                                 ; 66F8 pop bx
db 0x32, 0xC3                           ; 66F9 xor al,bl
db 0xBB, 0x9E, 0x04                     ; 66FB mov bx,0x49e
db 0xBE, 0xAA, 0x04                     ; 66FE mov si,0x4aa
db 0xB9, 0x04, 0x00                     ; 6701 mov cx,0x4
db 0xF8                                 ; 6704 clc
db 0xFC                                 ; 6705 cld
db 0x78, 0x1E                           ; 6706 js 0x6726
db 0xAD                                 ; 6708 lodsw
db 0x11, 0x07                           ; 6709 adc [bx],ax
db 0x43                                 ; 670B inc bx
db 0x43                                 ; 670C inc bx
db 0xE2, 0xF9                           ; 670D loop 0x6708
db 0x73, 0x12                           ; 670F jnc 0x6723
db 0xBB, 0xA6, 0x04                     ; 6711 mov bx,0x4a6
db 0xFE, 0x07                           ; 6714 inc byte [bx]
db 0x74, 0x34                           ; 6716 jz 0x674c
db 0x4B                                 ; 6718 dec bx
db 0x4B                                 ; 6719 dec bx
db 0xB9, 0x04, 0x00                     ; 671A mov cx,0x4
db 0xD1, 0x1F                           ; 671D rcr word [bx],1
db 0x4B                                 ; 671F dec bx
db 0x4B                                 ; 6720 dec bx
db 0xE2, 0xFA                           ; 6721 loop 0x671d
db 0xE9, 0xE9, 0x11                     ; 6723 jmp 0x790f
db 0xAD                                 ; 6726 lodsw
db 0x19, 0x07                           ; 6727 sbb [bx],ax
db 0x43                                 ; 6729 inc bx
db 0x43                                 ; 672A inc bx
db 0xE2, 0xF9                           ; 672B loop 0x6726
db 0x73, 0x1A                           ; 672D jnc 0x6749
db 0xF6, 0x97, 0x01, 0x00               ; 672F not byte [bx+0x1]
db 0xB9, 0x04, 0x00                     ; 6733 mov cx,0x4
db 0x4B                                 ; 6736 dec bx
db 0x4B                                 ; 6737 dec bx
db 0xF7, 0x17                           ; 6738 not word [bx]
db 0xE2, 0xFA                           ; 673A loop 0x6736
db 0xB9, 0x04, 0x00                     ; 673C mov cx,0x4
db 0xFF, 0x07                           ; 673F inc word [bx]
db 0x75, 0x06                           ; 6741 jnz 0x6749
db 0x43                                 ; 6743 inc bx
db 0x43                                 ; 6744 inc bx
db 0xE2, 0xF8                           ; 6745 loop 0x673f
db 0x74, 0xC8                           ; 6747 jz 0x6711
db 0xE9, 0xF4, 0x0C                     ; 6749 jmp 0x7440
db 0xE9, 0x8D, 0x0D                     ; 674C jmp 0x74dc
db 0xA0, 0xA7, 0x04                     ; 674F mov al,[0x4a7]
db 0x24, 0x80                           ; 6752 and al,0x80
db 0x80, 0x26, 0xA5, 0x04, 0x7F         ; 6754 and byte [0x4a5],0x7f
db 0x08, 0x06, 0xA5, 0x04               ; 6759 or [0x4a5],al
db 0xC3                                 ; 675D ret
db 0x89, 0x1E, 0xA5, 0x04               ; 675E mov [0x4a5],bx
db 0x89, 0x16, 0xA3, 0x04               ; 6762 mov [0x4a3],dx
db 0xC3                                 ; 6766 ret
db 0xA1, 0xA5, 0x04                     ; 6767 mov ax,[0x4a5]
db 0x0A, 0xE4                           ; 676A or ah,ah
db 0x74, 0xF0                           ; 676C jz 0x675e
db 0x80, 0x36, 0xA5, 0x04, 0x80         ; 676E xor byte [0x4a5],0x80
db 0x0A, 0xFF                           ; 6773 or bh,bh
db 0x74, 0xEF                           ; 6775 jz 0x6766
db 0xA1, 0xA5, 0x04                     ; 6777 mov ax,[0x4a5]
db 0x0A, 0xE4                           ; 677A or ah,ah
db 0x74, 0xE0                           ; 677C jz 0x675e
db 0x33, 0xC9                           ; 677E xor cx,cx
db 0x8B, 0x36, 0xA3, 0x04               ; 6780 mov si,[0x4a3]
db 0xA2, 0xA7, 0x04                     ; 6784 mov [0x4a7],al
db 0x8A, 0xCC                           ; 6787 mov cl,ah
db 0x2A, 0xCF                           ; 6789 sub cl,bh
db 0x73, 0x0D                           ; 678B jnc 0x679a
db 0xF6, 0xD9                           ; 678D neg cl
db 0x86, 0xDF                           ; 678F xchg bl,bh
db 0x89, 0x1E, 0xA6, 0x04               ; 6791 mov [0x4a6],bx
db 0x86, 0xDF                           ; 6795 xchg bl,bh
db 0x93                                 ; 6797 xchg ax,bx
db 0x87, 0xD6                           ; 6798 xchg dx,si
db 0x8A, 0xE0                           ; 679A mov ah,al
db 0x32, 0xE3                           ; 679C xor ah,bl
db 0x9C                                 ; 679E pushf
db 0xB4, 0x80                           ; 679F mov ah,0x80
db 0x0A, 0xC4                           ; 67A1 or al,ah
db 0x0A, 0xDC                           ; 67A3 or bl,ah
db 0x32, 0xE4                           ; 67A5 xor ah,ah
db 0x8A, 0xFC                           ; 67A7 mov bh,ah
db 0x0B, 0xC9                           ; 67A9 or cx,cx
db 0x74, 0x46                           ; 67AB jz 0x67f3
db 0x83, 0xF9, 0x19                     ; 67AD cmp cx,byte +0x19
db 0x72, 0x12                           ; 67B0 jc 0x67c4
db 0x9D                                 ; 67B2 popf
db 0x89, 0x36, 0xA3, 0x04               ; 67B3 mov [0x4a3],si
db 0x8A, 0x26, 0xA7, 0x04               ; 67B7 mov ah,[0x4a7]
db 0x25, 0x7F, 0x80                     ; 67BB and ax,0x807f
db 0x0A, 0xC4                           ; 67BE or al,ah
db 0xA2, 0xA5, 0x04                     ; 67C0 mov [0x4a5],al
db 0xC3                                 ; 67C3 ret
db 0x80, 0xF9, 0x08                     ; 67C4 cmp cl,0x8
db 0x72, 0x1C                           ; 67C7 jc 0x67e5
db 0xE9, 0x05, 0x03                     ; 67C9 jmp 0x6ad1
db 0x90                                 ; 67CC nop
db 0x8A, 0xF3                           ; 67CD mov dh,bl
db 0x32, 0xDB                           ; 67CF xor bl,bl
db 0x80, 0xE9, 0x08                     ; 67D1 sub cl,0x8
db 0xF6, 0xC4, 0x1F                     ; 67D4 test ah,0x1f
db 0x74, 0xD0                           ; 67D7 jz 0x67a9
db 0x80, 0xCC, 0x20                     ; 67D9 or ah,0x20
db 0xEB, 0xCB                           ; 67DC jmp short 0x67a9
db 0x80, 0xCC, 0x20                     ; 67DE or ah,0x20
db 0xE2, 0x03                           ; 67E1 loop 0x67e6
db 0xEB, 0x0E                           ; 67E3 jmp short 0x67f3
db 0xF8                                 ; 67E5 clc
db 0xD0, 0xDB                           ; 67E6 rcr bl,1
db 0xD1, 0xDA                           ; 67E8 rcr dx,1
db 0xD0, 0xDC                           ; 67EA rcr ah,1
db 0xF6, 0xC4, 0x10                     ; 67EC test ah,0x10
db 0x75, 0xED                           ; 67EF jnz 0x67de
db 0xE2, 0xF3                           ; 67F1 loop 0x67e6
db 0x9D                                 ; 67F3 popf
db 0x79, 0x25                           ; 67F4 jns 0x681b
db 0x2A, 0xCC                           ; 67F6 sub cl,ah
db 0x8A, 0xE1                           ; 67F8 mov ah,cl
db 0x1B, 0xF2                           ; 67FA sbb si,dx
db 0x8B, 0xD6                           ; 67FC mov dx,si
db 0x1A, 0xC3                           ; 67FE sbb al,bl
db 0x8A, 0xD8                           ; 6800 mov bl,al
db 0x73, 0x2F                           ; 6802 jnc 0x6833
db 0xF6, 0x16, 0xA7, 0x04               ; 6804 not byte [0x4a7]
db 0xF6, 0xD4                           ; 6808 not ah
db 0xF7, 0xD2                           ; 680A not dx
db 0xF6, 0xD3                           ; 680C not bl
db 0xFE, 0xC4                           ; 680E inc ah
db 0x75, 0x21                           ; 6810 jnz 0x6833
db 0x42                                 ; 6812 inc dx
db 0x75, 0x1E                           ; 6813 jnz 0x6833
db 0xFE, 0xC3                           ; 6815 inc bl
db 0x75, 0x1A                           ; 6817 jnz 0x6833
db 0xEB, 0x06                           ; 6819 jmp short 0x6821
db 0x03, 0xD6                           ; 681B add dx,si
db 0x12, 0xD8                           ; 681D adc bl,al
db 0x73, 0x0C                           ; 681F jnc 0x682d
db 0xFE, 0x06, 0xA6, 0x04               ; 6821 inc byte [0x4a6]
db 0x74, 0x09                           ; 6825 jz 0x6830
db 0xD0, 0xDB                           ; 6827 rcr bl,1
db 0xD1, 0xDA                           ; 6829 rcr dx,1
db 0xD0, 0xDC                           ; 682B rcr ah,1
db 0xE9, 0x24, 0x11                     ; 682D jmp 0x7954
db 0xE9, 0xA9, 0x0C                     ; 6830 jmp 0x74dc
db 0xE9, 0x68, 0x0C                     ; 6833 jmp 0x749e
db 0xE8, 0x25, 0x13                     ; 6836 call 0x7b5e
db 0xC3                                 ; 6839 ret
db 0xA0, 0xB1, 0x04                     ; 683A mov al,[0x4b1]
db 0xA2, 0xA7, 0x04                     ; 683D mov [0x4a7],al
db 0xE9, 0xA7, 0x0C                     ; 6840 jmp 0x74ea
db 0xF6, 0x06, 0xA6, 0x04, 0xFF         ; 6843 test byte [0x4a6],0xff
db 0x74, 0xF0                           ; 6848 jz 0x683a
db 0xF6, 0x06, 0xB2, 0x04, 0xFF         ; 684A test byte [0x4b2],0xff
db 0x74, 0xE5                           ; 684F jz 0x6836
db 0x8B, 0x1E, 0xB1, 0x04               ; 6851 mov bx,[0x4b1]
db 0xE8, 0xF3, 0xF4                     ; 6855 call 0x5d4b
db 0x89, 0x1E, 0xB1, 0x04               ; 6858 mov [0x4b1],bx
db 0xBB, 0xA4, 0x04                     ; 685C mov bx,0x4a4
db 0xF8                                 ; 685F clc
db 0xE8, 0xA1, 0x13                     ; 6860 call 0x7c04
db 0xBB, 0xB0, 0x04                     ; 6863 mov bx,0x4b0
db 0xF8                                 ; 6866 clc
db 0xE8, 0x9A, 0x13                     ; 6867 call 0x7c04
db 0xFF, 0x36, 0xA6, 0x04               ; 686A push word [0x4a6]
db 0xE8, 0x5E, 0x15                     ; 686E call 0x7dcf
db 0x8F, 0x06, 0xA6, 0x04               ; 6871 pop word [0x4a6]
db 0xB9, 0x40, 0x00                     ; 6875 mov cx,0x40
db 0x51                                 ; 6878 push cx
db 0xEB, 0x08                           ; 6879 jmp short 0x6883
db 0x51                                 ; 687B push cx
db 0xF8                                 ; 687C clc
db 0xBB, 0xAA, 0x04                     ; 687D mov bx,0x4aa
db 0xE8, 0x77, 0x13                     ; 6880 call 0x7bfa
db 0x8B, 0xFC                           ; 6883 mov di,sp
db 0x83, 0xEC, 0x08                     ; 6885 sub sp,byte +0x8
db 0x83, 0xEF, 0x02                     ; 6888 sub di,byte +0x2
db 0xBE, 0xB0, 0x04                     ; 688B mov si,0x4b0
db 0xB9, 0x04, 0x00                     ; 688E mov cx,0x4
db 0xFD                                 ; 6891 std
db 0xF3, 0xA5                           ; 6892 rep movsw
db 0xBE, 0x78, 0x04                     ; 6894 mov si,0x478
db 0xB9, 0x04, 0x00                     ; 6897 mov cx,0x4
db 0xBB, 0xAA, 0x04                     ; 689A mov bx,0x4aa
db 0xF8                                 ; 689D clc
db 0xFC                                 ; 689E cld
db 0xAD                                 ; 689F lodsw
db 0x19, 0x07                           ; 68A0 sbb [bx],ax
db 0x43                                 ; 68A2 inc bx
db 0x43                                 ; 68A3 inc bx
db 0xE2, 0xF9                           ; 68A4 loop 0x689f
db 0x73, 0x10                           ; 68A6 jnc 0x68b8
db 0xB9, 0x04, 0x00                     ; 68A8 mov cx,0x4
db 0x8B, 0xF4                           ; 68AB mov si,sp
db 0xBF, 0xAA, 0x04                     ; 68AD mov di,0x4aa
db 0xFC                                 ; 68B0 cld
db 0xF3, 0xA5                           ; 68B1 rep movsw
db 0x8B, 0xE6                           ; 68B3 mov sp,si
db 0xF8                                 ; 68B5 clc
db 0xEB, 0x04                           ; 68B6 jmp short 0x68bc
db 0x83, 0xC4, 0x08                     ; 68B8 add sp,byte +0x8
db 0xF9                                 ; 68BB stc
db 0xBB, 0x9E, 0x04                     ; 68BC mov bx,0x49e
db 0xE8, 0x38, 0x13                     ; 68BF call 0x7bfa
db 0x59                                 ; 68C2 pop cx
db 0xE2, 0xB6                           ; 68C3 loop 0x687b
db 0xF6, 0x06, 0xA5, 0x04, 0x80         ; 68C5 test byte [0x4a5],0x80
db 0x74, 0x09                           ; 68CA jz 0x68d5
db 0xFF, 0x06, 0xA6, 0x04               ; 68CC inc word [0x4a6]
db 0x75, 0x09                           ; 68D0 jnz 0x68db
db 0xE9, 0x07, 0x0C                     ; 68D2 jmp 0x74dc
db 0xBB, 0x9E, 0x04                     ; 68D5 mov bx,0x49e
db 0xE8, 0x1F, 0x13                     ; 68D8 call 0x7bfa
db 0xE9, 0x31, 0x10                     ; 68DB jmp 0x790f
db 0xE8, 0x94, 0x12                     ; 68DE call 0x7b75
db 0x75, 0x07                           ; 68E1 jnz 0x68ea
db 0x88, 0x1E, 0xA7, 0x04               ; 68E3 mov [0x4a7],bl
db 0xE9, 0x00, 0x0C                     ; 68E7 jmp 0x74ea
db 0x0A, 0xFF                           ; 68EA or bh,bh
db 0x75, 0x03                           ; 68EC jnz 0x68f1
db 0xE9, 0x7A, 0x12                     ; 68EE jmp 0x7b6b
db 0xE8, 0x57, 0xF4                     ; 68F1 call 0x5d4b
db 0x8B, 0xFA                           ; 68F4 mov di,dx
db 0x33, 0xD2                           ; 68F6 xor dx,dx
db 0x8A, 0xFE                           ; 68F8 mov bh,dh
db 0x8B, 0xF3                           ; 68FA mov si,bx
db 0x8A, 0xDF                           ; 68FC mov bl,bh
db 0xB9, 0x20, 0x00                     ; 68FE mov cx,0x20
db 0x55                                 ; 6901 push bp
db 0x8B, 0x2E, 0xA3, 0x04               ; 6902 mov bp,[0x4a3]
db 0xA0, 0xA5, 0x04                     ; 6906 mov al,[0x4a5]
db 0x8A, 0xE7                           ; 6909 mov ah,bh
db 0xEB, 0x05                           ; 690B jmp short 0x6912
db 0xF8                                 ; 690D clc
db 0xD1, 0xD7                           ; 690E rcl di,1
db 0xD1, 0xD6                           ; 6910 rcl si,1
db 0x56                                 ; 6912 push si
db 0x57                                 ; 6913 push di
db 0x2B, 0xFD                           ; 6914 sub di,bp
db 0x1B, 0xF0                           ; 6916 sbb si,ax
db 0x73, 0x04                           ; 6918 jnc 0x691e
db 0x5F                                 ; 691A pop di
db 0x5E                                 ; 691B pop si
db 0xEB, 0x04                           ; 691C jmp short 0x6922
db 0x83, 0xC4, 0x04                     ; 691E add sp,byte +0x4
db 0xF8                                 ; 6921 clc
db 0xF5                                 ; 6922 cmc
db 0xD1, 0xD2                           ; 6923 rcl dx,1
db 0xD1, 0xD3                           ; 6925 rcl bx,1
db 0xE2, 0xE4                           ; 6927 loop 0x690d
db 0x0B, 0xDB                           ; 6929 or bx,bx
db 0x79, 0x0A                           ; 692B jns 0x6937
db 0xFE, 0x06, 0xA6, 0x04               ; 692D inc byte [0x4a6]
db 0x75, 0x08                           ; 6931 jnz 0x693b
db 0x5D                                 ; 6933 pop bp
db 0xE9, 0xA5, 0x0B                     ; 6934 jmp 0x74dc
db 0xD1, 0xD2                           ; 6937 rcl dx,1
db 0xD1, 0xD3                           ; 6939 rcl bx,1
db 0x8A, 0xE2                           ; 693B mov ah,dl
db 0x8A, 0xD6                           ; 693D mov dl,dh
db 0x8A, 0xF3                           ; 693F mov dh,bl
db 0x8A, 0xDF                           ; 6941 mov bl,bh
db 0x5D                                 ; 6943 pop bp
db 0xE9, 0x0D, 0x10                     ; 6944 jmp 0x7954
db 0x13, 0xF9                           ; 6947 adc di,cx
db 0x53                                 ; 6949 push bx
db 0x57                                 ; 694A push di
db 0x51                                 ; 694B push cx
db 0x2C, 0x30                           ; 694C sub al,0x30
db 0x50                                 ; 694E push ax
db 0xE8, 0x50, 0x12                     ; 694F call 0x7ba2
db 0x58                                 ; 6952 pop ax
db 0x98                                 ; 6953 cbw
db 0x79, 0x1E                           ; 6954 jns 0x6974
db 0x8B, 0x1E, 0xA3, 0x04               ; 6956 mov bx,[0x4a3]
db 0x81, 0xFB, 0xCD, 0x0C               ; 695A cmp bx,0xccd
db 0x73, 0x19                           ; 695E jnc 0x6979
db 0x8B, 0xCB                           ; 6960 mov cx,bx
db 0xD1, 0xE3                           ; 6962 shl bx,1
db 0xD1, 0xE3                           ; 6964 shl bx,1
db 0x03, 0xD9                           ; 6966 add bx,cx
db 0xD1, 0xE3                           ; 6968 shl bx,1
db 0x03, 0xD8                           ; 696A add bx,ax
db 0x78, 0x0B                           ; 696C js 0x6979
db 0x89, 0x1E, 0xA3, 0x04               ; 696E mov [0x4a3],bx
db 0xEB, 0x48                           ; 6972 jmp short 0x69bc
db 0x50                                 ; 6974 push ax
db 0x72, 0x08                           ; 6975 jc 0x697f
db 0xEB, 0x33                           ; 6977 jmp short 0x69ac
db 0x50                                 ; 6979 push ax
db 0xE8, 0x24, 0x02                     ; 697A call 0x6ba1
db 0xEB, 0x14                           ; 697D jmp short 0x6993
db 0xC7, 0x06, 0x7C, 0x04, 0x00, 0x24   ; 697F mov word [0x47c],0x2400
db 0xC7, 0x06, 0x7E, 0x04, 0x74, 0x94   ; 6985 mov word [0x47e],0x9474
db 0xBB, 0x7E, 0x04                     ; 698B mov bx,0x47e
db 0xE8, 0x83, 0x13                     ; 698E call 0x7d14
db 0x79, 0x16                           ; 6991 jns 0x69a9
db 0xE8, 0x3C, 0x12                     ; 6993 call 0x7bd2
db 0x5A                                 ; 6996 pop dx
db 0xFF, 0x36, 0xA3, 0x04               ; 6997 push word [0x4a3]
db 0xFF, 0x36, 0xA5, 0x04               ; 699B push word [0x4a5]
db 0xE8, 0x8B, 0x02                     ; 699F call 0x6c2d
db 0x5B                                 ; 69A2 pop bx
db 0x5A                                 ; 69A3 pop dx
db 0xE8, 0xCC, 0xFD                     ; 69A4 call 0x6773
db 0xEB, 0x13                           ; 69A7 jmp short 0x69bc
db 0xE8, 0xE7, 0x01                     ; 69A9 call 0x6b93
db 0xE8, 0x23, 0x12                     ; 69AC call 0x7bd2
db 0xE8, 0xEE, 0x12                     ; 69AF call 0x7ca0
db 0x5A                                 ; 69B2 pop dx
db 0xE8, 0x77, 0x02                     ; 69B3 call 0x6c2d
db 0xE8, 0xDA, 0x01                     ; 69B6 call 0x6b93
db 0xE8, 0xEE, 0xFC                     ; 69B9 call 0x66aa
db 0x59                                 ; 69BC pop cx
db 0x5F                                 ; 69BD pop di
db 0x5B                                 ; 69BE pop bx
db 0xC3                                 ; 69BF ret
db 0xCD, 0xD9                           ; 69C0 int 0xd9
db 0x32, 0xC0                           ; 69C2 xor al,al
db 0xE9, 0x09, 0x00                     ; 69C4 jmp 0x69d0
db 0xCD, 0xDA                           ; 69C7 int 0xda
db 0xB0, 0x01                           ; 69C9 mov al,0x1
db 0xC6, 0x06, 0xFB, 0x02, 0x08         ; 69CB mov byte [0x2fb],0x8
db 0xC6, 0x06, 0xA8, 0x04, 0x01         ; 69D0 mov byte [0x4a8],0x1
db 0xBE, 0xB4, 0x25                     ; 69D5 mov si,0x25b4
db 0x56                                 ; 69D8 push si
db 0x33, 0xFF                           ; 69D9 xor di,di
db 0x8B, 0xCF                           ; 69DB mov cx,di
db 0x8B, 0xF7                           ; 69DD mov si,di
db 0xF7, 0xD1                           ; 69DF not cx
db 0x50                                 ; 69E1 push ax
db 0xE8, 0x86, 0x11                     ; 69E2 call 0x7b6b
db 0x58                                 ; 69E5 pop ax
db 0x0A, 0xC0                           ; 69E6 or al,al
db 0x75, 0x05                           ; 69E8 jnz 0x69ef
db 0xC6, 0x06, 0xFB, 0x02, 0x02         ; 69EA mov byte [0x2fb],0x2
db 0x8A, 0x07                           ; 69EF mov al,[bx]
db 0x3C, 0x26                           ; 69F1 cmp al,0x26
db 0x75, 0x03                           ; 69F3 jnz 0x69f8
db 0xE9, 0x07, 0xB0                     ; 69F5 jmp 0x19ff
db 0x3C, 0x2D                           ; 69F8 cmp al,0x2d
db 0x9C                                 ; 69FA pushf
db 0x74, 0x05                           ; 69FB jz 0x6a02
db 0x3C, 0x2B                           ; 69FD cmp al,0x2b
db 0x74, 0x01                           ; 69FF jz 0x6a02
db 0x4B                                 ; 6A01 dec bx
db 0xE8, 0xFB, 0xF8                     ; 6A02 call 0x6300
db 0x73, 0x06                           ; 6A05 jnc 0x6a0d
db 0xE8, 0x3D, 0xFF                     ; 6A07 call 0x6947
db 0xE9, 0xF5, 0xFF                     ; 6A0A jmp 0x6a02
db 0xBD, 0xA3, 0x61                     ; 6A0D mov bp,0x61a3
db 0x33, 0xD2                           ; 6A10 xor dx,dx
db 0x8B, 0xF2                           ; 6A12 mov si,dx
db 0x2E, 0x3A, 0x86, 0x00, 0x00         ; 6A14 cmp al,[cs:bp+0x0]
db 0x74, 0x0A                           ; 6A19 jz 0x6a25
db 0x81, 0xFD, 0x9C, 0x61               ; 6A1B cmp bp,0x619c
db 0x74, 0x24                           ; 6A1F jz 0x6a45
db 0x4D                                 ; 6A21 dec bp
db 0xE9, 0xEF, 0xFF                     ; 6A22 jmp 0x6a14
db 0x81, 0xED, 0x9C, 0x61               ; 6A25 sub bp,0x619c
db 0xD1, 0xE5                           ; 6A29 shl bp,1
db 0x2E, 0xFF, 0xA6, 0x30, 0x6A         ; 6A2B jmp near [cs:bp+0x6a30]
db 0x4B                                 ; 6A30 dec bx
db 0x6A, 0x5F                           ; 6A31 push byte +0x5f
db 0x6A, 0x5F                           ; 6A33 push byte +0x5f
db 0x6A, 0x67                           ; 6A35 push byte +0x67
db 0x6A, 0x6D                           ; 6A37 push byte +0x6d
db 0x6A, 0x73                           ; 6A39 push byte +0x73
db 0x6A, 0x40                           ; 6A3B push byte +0x40
db 0x6A, 0x40                           ; 6A3D push byte +0x40
db 0x6A

L_6A40:
  xor     al, al
  call    0x6ae1
  call    0x6a89
L_6A48:
  jmp     near L_6A78
  inc     cx
db 0x75, 0xF7                           ; 6A4C jnz 0x6a45
db 0xE8, 0x51, 0x11                     ; 6A4E call 0x7ba2
db 0x79, 0xAF                           ; 6A51 jns 0x6a02
db 0x51                                 ; 6A53 push cx
db 0x53                                 ; 6A54 push bx
db 0x57                                 ; 6A55 push di
db 0xE8, 0x48, 0x01                     ; 6A56 call 0x6ba1
db 0x5F                                 ; 6A59 pop di
db 0x5B                                 ; 6A5A pop bx
db 0x59                                 ; 6A5B pop cx
db 0xE9, 0xA3, 0xFF                     ; 6A5C jmp 0x6a02
db 0xE8, 0x8C, 0xF3                     ; 6A5F call 0x5dee
db 0x74, 0xE1                           ; 6A62 jz 0x6a45
db 0xE9, 0xDB, 0xFF                     ; 6A64 jmp 0x6a42
db 0x43                                 ; 6A67 inc bx
db 0xEB, 0xDB                           ; 6A68 jmp short 0x6a45
db 0xE9, 0x0B, 0x00                     ; 6A6A jmp 0x6a78
db 0xE8, 0xBC, 0x00                     ; 6A6D call 0x6b2c
db 0xE9, 0x05, 0x00                     ; 6A70 jmp 0x6a78
db 0x32, 0xC0                           ; 6A73 xor al,al
db 0xE8, 0xB6, 0x00                     ; 6A75 call 0x6b2e
L_6A78:
  popf
db 0x75, 0x0D                           ; 6A79 jnz 0x6a88
db 0xE8, 0x2D, 0x13                     ; 6A7B call 0x7dab
db 0xE8, 0x21, 0x11                     ; 6A7E call 0x7ba2
db 0x7A, 0x05                           ; 6A81 jpe 0x6a88
db 0x53                                 ; 6A83 push bx
db 0xE8, 0xED, 0x12                     ; 6A84 call 0x7d74
db 0x5B                                 ; 6A87 pop bx
db 0xC3                                 ; 6A88 ret
db 0xE9, 0x76, 0xF4                     ; 6A89 jmp 0x5f02
db 0xC6, 0x06, 0x55, 0x04, 0xFF         ; 6A8C mov byte [0x455],0xff
db 0xE8, 0x89, 0xA4                     ; 6A91 call 0xf1d
db 0x8B, 0xD4                           ; 6A94 mov dx,sp
db 0xE9, 0xE3, 0x08                     ; 6A96 jmp 0x737c
db 0x0A, 0xFF                           ; 6A99 or bh,bh
db 0x75, 0x05                           ; 6A9B jnz 0x6aa2
db 0xE8, 0xE8, 0x10                     ; 6A9D call 0x7b88
db 0xF9                                 ; 6AA0 stc
db 0xC3                                 ; 6AA1 ret
db 0xA0, 0xA6, 0x04                     ; 6AA2 mov al,[0x4a6]
db 0x0A, 0xC0                           ; 6AA5 or al,al
db 0x75, 0x08                           ; 6AA7 jnz 0x6ab1
db 0x8A, 0xC3                           ; 6AA9 mov al,bl
db 0xF6, 0xD0                           ; 6AAB not al
db 0xE8, 0xE4, 0x10                     ; 6AAD call 0x7b94
db 0xF9                                 ; 6AB0 stc
db 0xC3                                 ; 6AB1 ret
db 0xE8, 0xE4, 0xFF                     ; 6AB2 call 0x6a99
db 0x5D                                 ; 6AB5 pop bp
db 0x72, 0x14                           ; 6AB6 jc 0x6acc
db 0xEB, 0x10                           ; 6AB8 jmp short 0x6aca
db 0x53                                 ; 6ABA push bx
db 0x8B, 0x1F                           ; 6ABB mov bx,[bx]
db 0xE8, 0xD9, 0xFF                     ; 6ABD call 0x6a99
db 0x5B                                 ; 6AC0 pop bx
db 0x5D                                 ; 6AC1 pop bp
db 0x72, 0x08                           ; 6AC2 jc 0x6acc
db 0x53                                 ; 6AC4 push bx
db 0x57                                 ; 6AC5 push di
db 0xBF, 0xA5, 0x04                     ; 6AC6 mov di,0x4a5
db 0x90                                 ; 6AC9 nop
db 0x55                                 ; 6ACA push bp
db 0xC3                                 ; 6ACB ret
db 0xFE, 0xC0                           ; 6ACC inc al
db 0x2C, 0x01                           ; 6ACE sub al,0x1
db 0xC3                                 ; 6AD0 ret
db 0xF6, 0xC4, 0xFF                     ; 6AD1 test ah,0xff
db 0x8A, 0xE2                           ; 6AD4 mov ah,dl
db 0x74, 0x03                           ; 6AD6 jz 0x6adb
db 0x80, 0xCC, 0x20                     ; 6AD8 or ah,0x20
db 0x8A, 0xD6                           ; 6ADB mov dl,dh
db 0xE9, 0xED, 0xFC                     ; 6ADD jmp 0x67cd
db 0x10, 0x9F, 0x80, 0x3E               ; 6AE0 adc [bx+0x3e80],bl
db 0xFB                                 ; 6AE4 sti
db 0x02, 0x08                           ; 6AE5 add cl,[bx+si]
db 0x75, 0x04                           ; 6AE7 jnz 0x6aed
db 0x9E                                 ; 6AE9 sahf
db 0xE9, 0x08, 0x00                     ; 6AEA jmp 0x6af5
db 0x9E                                 ; 6AED sahf
db 0x53                                 ; 6AEE push bx
db 0x57                                 ; 6AEF push di
db 0xE8, 0x5C, 0x00                     ; 6AF0 call 0x6b4f
db 0x5F                                 ; 6AF3 pop di
db 0x5B                                 ; 6AF4 pop bx
db 0x33, 0xF6                           ; 6AF5 xor si,si
db 0x8B, 0xD6                           ; 6AF7 mov dx,si
db 0xE8, 0x04, 0xF8                     ; 6AF9 call 0x6300
db 0x72, 0x13                           ; 6AFC jc 0x6b11
db 0x3C, 0x2D                           ; 6AFE cmp al,0x2d
db 0x75, 0x04                           ; 6B00 jnz 0x6b06
db 0xF7, 0xD6                           ; 6B02 not si
db 0xEB, 0x05                           ; 6B04 jmp short 0x6b0b
db 0x3C, 0x2B                           ; 6B06 cmp al,0x2b
db 0x74, 0x01                           ; 6B08 jz 0x6b0b
db 0xC3                                 ; 6B0A ret
db 0xE8, 0xF2, 0xF7                     ; 6B0B call 0x6300
db 0x72, 0x01                           ; 6B0E jc 0x6b11
db 0xC3                                 ; 6B10 ret
db 0x81, 0xFA, 0xCC, 0x0C               ; 6B11 cmp dx,0xccc
db 0x72, 0x05                           ; 6B15 jc 0x6b1c
db 0xBA, 0xFF, 0x7F                     ; 6B17 mov dx,0x7fff
db 0xEB, 0xEF                           ; 6B1A jmp short 0x6b0b
db 0x50                                 ; 6B1C push ax
db 0xB8, 0x0A, 0x00                     ; 6B1D mov ax,0xa
db 0xF7, 0xE2                           ; 6B20 mul dx
db 0x5A                                 ; 6B22 pop dx
db 0x80, 0xEA, 0x30                     ; 6B23 sub dl,0x30
db 0x32, 0xF6                           ; 6B26 xor dh,dh
db 0x03, 0xD0                           ; 6B28 add dx,ax
db 0xEB, 0xDF                           ; 6B2A jmp short 0x6b0b
db 0x0C, 0x01                           ; 6B2C or al,0x1
db 0x53                                 ; 6B2E push bx
db 0x57                                 ; 6B2F push di
db 0x75, 0x07                           ; 6B30 jnz 0x6b39
db 0xE8, 0x1C, 0x00                     ; 6B32 call 0x6b51
db 0xEB, 0x05                           ; 6B35 jmp short 0x6b3c
db 0x90                                 ; 6B37 nop
db 0x90                                 ; 6B38 nop
db 0xE8, 0x46, 0x00                     ; 6B39 call 0x6b82
db 0x5F                                 ; 6B3C pop di
db 0x5B                                 ; 6B3D pop bx
db 0x33, 0xF6                           ; 6B3E xor si,si
db 0x8B, 0xD6                           ; 6B40 mov dx,si
db 0xE8, 0xBD, 0xF3                     ; 6B42 call 0x5f02
db 0x43                                 ; 6B45 inc bx
db 0xC3                                 ; 6B46 ret
db 0xE8, 0x58, 0x10                     ; 6B47 call 0x7ba2
db 0x78, 0xF9                           ; 6B4A js 0x6b45
db 0xE9, 0x6F, 0x9C                     ; 6B4C jmp 0x7be
db 0x74, 0x31                           ; 6B4F jz 0x6b82
db 0xE8, 0x4E, 0x10                     ; 6B51 call 0x7ba2
db 0x7B, 0x56                           ; 6B54 jpo 0x6bac
db 0x75, 0x03                           ; 6B56 jnz 0x6b5b
db 0xE9, 0x7B, 0x9C                     ; 6B58 jmp 0x7d6
db 0xCD, 0xCF                           ; 6B5B int 0xcf
db 0x79, 0x05                           ; 6B5D jns 0x6b64
db 0xE8, 0x3F, 0x00                     ; 6B5F call 0x6ba1
db 0xEB, 0x48                           ; 6B62 jmp short 0x6bac
db 0xB0, 0x04                           ; 6B64 mov al,0x4
db 0xA2, 0xFB, 0x02                     ; 6B66 mov [0x2fb],al
db 0x8A, 0x1E, 0xA5, 0x04               ; 6B69 mov bl,[0x4a5]
db 0x88, 0x1E, 0xA7, 0x04               ; 6B6D mov [0x4a7],bl
db 0x8B, 0x16, 0xA3, 0x04               ; 6B71 mov dx,[0x4a3]
db 0x8A, 0x26, 0xA2, 0x04               ; 6B75 mov ah,[0x4a2]
db 0x80, 0xCC, 0x40                     ; 6B79 or ah,0x40
db 0x80, 0xCB, 0x80                     ; 6B7C or bl,0x80
db 0xE9, 0xD5, 0x0D                     ; 6B7F jmp 0x7957
db 0xE8, 0x1D, 0x10                     ; 6B82 call 0x7ba2
db 0x73, 0x25                           ; 6B85 jnc 0x6bac
db 0x75, 0x03                           ; 6B87 jnz 0x6b8c
db 0xE9, 0x4A, 0x9C                     ; 6B89 jmp 0x7d6
db 0xCD, 0xD0                           ; 6B8C int 0xd0
db 0x79, 0x03                           ; 6B8E jns 0x6b93
db 0xE8, 0x0E, 0x00                     ; 6B90 call 0x6ba1
db 0xB0, 0x08                           ; 6B93 mov al,0x8
db 0xA2, 0xFB, 0x02                     ; 6B95 mov [0x2fb],al
db 0x33, 0xC0                           ; 6B98 xor ax,ax
db 0xA3, 0x9F, 0x04                     ; 6B9A mov [0x49f],ax
db 0xA3, 0xA1, 0x04                     ; 6B9D mov [0x4a1],ax
db 0xC3                                 ; 6BA0 ret
db 0x52                                 ; 6BA1 push dx
db 0x56                                 ; 6BA2 push si
db 0x8B, 0x16, 0xA3, 0x04               ; 6BA3 mov dx,[0x4a3]
db 0xE8, 0x83, 0x00                     ; 6BA7 call 0x6c2d
db 0x5E                                 ; 6BAA pop si
db 0x5A                                 ; 6BAB pop dx
db 0xC3                                 ; 6BAC ret
db 0xE8, 0xF2, 0x0F                     ; 6BAD call 0x7ba2
db 0x79, 0x05                           ; 6BB0 jns 0x6bb7
db 0x8B, 0x1E, 0xA3, 0x04               ; 6BB2 mov bx,[0x4a3]
db 0xC3                                 ; 6BB6 ret
db 0xCD, 0xD1                           ; 6BB7 int 0xd1
db 0x75, 0x03                           ; 6BB9 jnz 0x6bbe
db 0xE9, 0x18, 0x9C                     ; 6BBB jmp 0x7d6
db 0xA0, 0xA6, 0x04                     ; 6BBE mov al,[0x4a6]
db 0x3C, 0x90                           ; 6BC1 cmp al,0x90
db 0x72, 0x31                           ; 6BC3 jc 0x6bf6
db 0x74, 0x03                           ; 6BC5 jz 0x6bca
db 0xE9, 0x06, 0x9C                     ; 6BC7 jmp 0x7d0
db 0xA0, 0xA5, 0x04                     ; 6BCA mov al,[0x4a5]
db 0x0A, 0xC0                           ; 6BCD or al,al
db 0x78, 0x03                           ; 6BCF js 0x6bd4
db 0xE9, 0xFC, 0x9B                     ; 6BD1 jmp 0x7d0
db 0xBA, 0x00, 0x00                     ; 6BD4 mov dx,0x0
db 0xBB, 0x00, 0x80                     ; 6BD7 mov bx,0x8000
db 0xE8, 0x8A, 0xFB                     ; 6BDA call 0x6767
db 0xE8, 0xCB, 0x06                     ; 6BDD call 0x72ab
db 0xE8, 0xCD, 0x11                     ; 6BE0 call 0x7db0
db 0xBA, 0x00, 0x00                     ; 6BE3 mov dx,0x0
db 0xBB, 0x80, 0x90                     ; 6BE6 mov bx,0x9080
db 0xE8, 0xEC, 0x10                     ; 6BE9 call 0x7cd8
db 0x74, 0x03                           ; 6BEC jz 0x6bf1
db 0xE9, 0xDF, 0x9B                     ; 6BEE jmp 0x7d0
db 0xBB, 0x00, 0x80                     ; 6BF1 mov bx,0x8000
db 0xEB, 0x2D                           ; 6BF4 jmp short 0x6c23
db 0xA0, 0xA5, 0x04                     ; 6BF6 mov al,[0x4a5]
db 0x0A, 0xC0                           ; 6BF9 or al,al
db 0x9C                                 ; 6BFB pushf
db 0x79, 0x05                           ; 6BFC jns 0x6c03
db 0x24, 0x7F                           ; 6BFE and al,0x7f
db 0xA2, 0xA5, 0x04                     ; 6C00 mov [0x4a5],al
db 0xBA, 0x00, 0x00                     ; 6C03 mov dx,0x0
db 0xBB, 0x00, 0x80                     ; 6C06 mov bx,0x8000
db 0xE8, 0x67, 0xFB                     ; 6C09 call 0x6773
db 0xA0, 0xA6, 0x04                     ; 6C0C mov al,[0x4a6]
db 0x3C, 0x90                           ; 6C0F cmp al,0x90
db 0x75, 0x06                           ; 6C11 jnz 0x6c19
db 0x9D                                 ; 6C13 popf
db 0x78, 0xDB                           ; 6C14 js 0x6bf1
db 0xE9, 0xB7, 0x9B                     ; 6C16 jmp 0x7d0
db 0xE8, 0xE7, 0x06                     ; 6C19 call 0x7303
db 0x8B, 0xDA                           ; 6C1C mov bx,dx
db 0x9D                                 ; 6C1E popf
db 0x79, 0x02                           ; 6C1F jns 0x6c23
db 0xF7, 0xDB                           ; 6C21 neg bx
db 0x89, 0x1E, 0xA3, 0x04               ; 6C23 mov [0x4a3],bx
db 0xC6, 0x06, 0xFB, 0x02, 0x02         ; 6C27 mov byte [0x2fb],0x2
db 0xC3                                 ; 6C2C ret
db 0x33, 0xDB                           ; 6C2D xor bx,bx
db 0x32, 0xE4                           ; 6C2F xor ah,ah
db 0xBE, 0xA7, 0x04                     ; 6C31 mov si,0x4a7
db 0xC6, 0x84, 0xFF, 0xFF, 0x90         ; 6C34 mov byte [si+0xffff],0x90
db 0xC6, 0x04, 0x00                     ; 6C39 mov byte [si],0x0
db 0x0B, 0xD2                           ; 6C3C or dx,dx
db 0x79, 0x05                           ; 6C3E jns 0x6c45
db 0xF7, 0xDA                           ; 6C40 neg dx
db 0xC6, 0x04, 0x80                     ; 6C42 mov byte [si],0x80
db 0x8A, 0xDE                           ; 6C45 mov bl,dh
db 0x8A, 0xF2                           ; 6C47 mov dh,dl
db 0x8A, 0xD7                           ; 6C49 mov dl,bh
db 0xC6, 0x06, 0xFB, 0x02, 0x04         ; 6C4B mov byte [0x2fb],0x4
db 0xE9, 0x4B, 0x08                     ; 6C50 jmp 0x749e
db 0xCD, 0xD6                           ; 6C53 int 0xd6
db 0xA0, 0xA6, 0x04                     ; 6C55 mov al,[0x4a6]
db 0x0A, 0xC0                           ; 6C58 or al,al
db 0x74, 0x0A                           ; 6C5A jz 0x6c66
db 0xA0, 0xB2, 0x04                     ; 6C5C mov al,[0x4b2]
db 0x0A, 0xC0                           ; 6C5F or al,al
db 0x75, 0x04                           ; 6C61 jnz 0x6c67
db 0xE9, 0xF8, 0x0E                     ; 6C63 jmp 0x7b5e
db 0xC3                                 ; 6C66 ret
db 0x8B, 0x1E, 0xB1, 0x04               ; 6C67 mov bx,[0x4b1]
db 0xE8, 0xDA, 0xF0                     ; 6C6B call 0x5d48
db 0xFF, 0x36, 0xA6, 0x04               ; 6C6E push word [0x4a6]
db 0x89, 0x1E, 0xB1, 0x04               ; 6C72 mov [0x4b1],bx
db 0xE8, 0x56, 0x11                     ; 6C76 call 0x7dcf
db 0x8B, 0xF0                           ; 6C79 mov si,ax
db 0xA3, 0xA6, 0x04                     ; 6C7B mov [0x4a6],ax
db 0xBB, 0x78, 0x04                     ; 6C7E mov bx,0x478
db 0xA3, 0xB2, 0x04                     ; 6C81 mov [0x4b2],ax
db 0xBD, 0xAB, 0x04                     ; 6C84 mov bp,0x4ab
db 0x8B, 0x00                           ; 6C87 mov ax,[bx+si]
db 0x0B, 0xC0                           ; 6C89 or ax,ax
db 0x74, 0x2C                           ; 6C8B jz 0x6cb9
db 0xBF, 0x00, 0x00                     ; 6C8D mov di,0x0
db 0x8B, 0xCF                           ; 6C90 mov cx,di
db 0x8B, 0x00                           ; 6C92 mov ax,[bx+si]
db 0xF7, 0x23                           ; 6C94 mul word [bp+di]
db 0x53                                 ; 6C96 push bx
db 0x8B, 0xDE                           ; 6C97 mov bx,si
db 0x03, 0xDF                           ; 6C99 add bx,di
db 0x81, 0xC3, 0x97, 0x04               ; 6C9B add bx,0x497
db 0x03, 0x07                           ; 6C9F add ax,[bx]
db 0x73, 0x01                           ; 6CA1 jnc 0x6ca4
db 0x42                                 ; 6CA3 inc dx
db 0x03, 0xC1                           ; 6CA4 add ax,cx
db 0x73, 0x01                           ; 6CA6 jnc 0x6ca9
db 0x42                                 ; 6CA8 inc dx
db 0x89, 0x07                           ; 6CA9 mov [bx],ax
db 0x8B, 0xCA                           ; 6CAB mov cx,dx
db 0x5B                                 ; 6CAD pop bx
db 0x83, 0xFF, 0x06                     ; 6CAE cmp di,byte +0x6
db 0x74, 0x04                           ; 6CB1 jz 0x6cb7
db 0x47                                 ; 6CB3 inc di
db 0x47                                 ; 6CB4 inc di
db 0xEB, 0xDB                           ; 6CB5 jmp short 0x6c92
db 0x8B, 0xC1                           ; 6CB7 mov ax,cx
db 0x53                                 ; 6CB9 push bx
db 0xBB, 0x9F, 0x04                     ; 6CBA mov bx,0x49f
db 0x89, 0x00                           ; 6CBD mov [bx+si],ax
db 0x5B                                 ; 6CBF pop bx
db 0x83, 0xFE, 0x06                     ; 6CC0 cmp si,byte +0x6
db 0x74, 0x04                           ; 6CC3 jz 0x6cc9
db 0x46                                 ; 6CC5 inc si
db 0x46                                 ; 6CC6 inc si
db 0xEB, 0xBE                           ; 6CC7 jmp short 0x6c87
db 0xBE, 0x9D, 0x04                     ; 6CC9 mov si,0x49d
db 0xFD                                 ; 6CCC std
db 0xB9, 0x07, 0x00                     ; 6CCD mov cx,0x7
db 0xAC                                 ; 6CD0 lodsb
db 0x0A, 0xC0                           ; 6CD1 or al,al
db 0xE1, 0xFB                           ; 6CD3 loope 0x6cd0
db 0x74, 0x05                           ; 6CD5 jz 0x6cdc
db 0x80, 0x0E, 0x9E, 0x04, 0x20         ; 6CD7 or byte [0x49e],0x20
db 0xA0, 0xA5, 0x04                     ; 6CDC mov al,[0x4a5]
db 0x0A, 0xC0                           ; 6CDF or al,al
db 0x8F, 0x06, 0xA6, 0x04               ; 6CE1 pop word [0x4a6]
db 0x78, 0x0F                           ; 6CE5 js 0x6cf6
db 0xBB, 0x9E, 0x04                     ; 6CE7 mov bx,0x49e
db 0xB9, 0x04, 0x00                     ; 6CEA mov cx,0x4
db 0xD1, 0x17                           ; 6CED rcl word [bx],1
db 0x43                                 ; 6CEF inc bx
db 0x43                                 ; 6CF0 inc bx
db 0xE2, 0xFA                           ; 6CF1 loop 0x6ced
db 0xE9, 0x19, 0x0C                     ; 6CF3 jmp 0x790f
db 0xFE, 0x06, 0xA6, 0x04               ; 6CF6 inc byte [0x4a6]
db 0x75, 0xF7                           ; 6CFA jnz 0x6cf3
db 0xE9, 0xDD, 0x07                     ; 6CFC jmp 0x74dc
db 0xE8, 0x73, 0x0E                     ; 6CFF call 0x7b75
db 0x74, 0x04                           ; 6D02 jz 0x6d08
db 0x0A, 0xFF                           ; 6D04 or bh,bh
db 0x75, 0x03                           ; 6D06 jnz 0x6d0b
db 0xE9, 0x60, 0x0E                     ; 6D08 jmp 0x7b6b
db 0xE8, 0x3A, 0xF0                     ; 6D0B call 0x5d48
db 0x8B, 0x0E, 0xA5, 0x04               ; 6D0E mov cx,[0x4a5]
db 0x32, 0xED                           ; 6D12 xor ch,ch
db 0xA1, 0xA3, 0x04                     ; 6D14 mov ax,[0x4a3]
db 0x8A, 0xFD                           ; 6D17 mov bh,ch
db 0x53                                 ; 6D19 push bx
db 0x51                                 ; 6D1A push cx
db 0x52                                 ; 6D1B push dx
db 0x51                                 ; 6D1C push cx
db 0x50                                 ; 6D1D push ax
db 0xF7, 0xE2                           ; 6D1E mul dx
db 0x8B, 0xCA                           ; 6D20 mov cx,dx
db 0x58                                 ; 6D22 pop ax
db 0xF7, 0xE3                           ; 6D23 mul bx
db 0x03, 0xC8                           ; 6D25 add cx,ax
db 0x73, 0x01                           ; 6D27 jnc 0x6d2a
db 0x42                                 ; 6D29 inc dx
db 0x8B, 0xDA                           ; 6D2A mov bx,dx
db 0x5A                                 ; 6D2C pop dx
db 0x58                                 ; 6D2D pop ax
db 0xF7, 0xE2                           ; 6D2E mul dx
db 0x03, 0xC8                           ; 6D30 add cx,ax
db 0x73, 0x01                           ; 6D32 jnc 0x6d35
db 0x42                                 ; 6D34 inc dx
db 0x03, 0xDA                           ; 6D35 add bx,dx
db 0x5A                                 ; 6D37 pop dx
db 0x58                                 ; 6D38 pop ax
db 0xF6, 0xE2                           ; 6D39 mul dl
db 0x03, 0xD8                           ; 6D3B add bx,ax
db 0x73, 0x0D                           ; 6D3D jnc 0x6d4c
db 0xD1, 0xDB                           ; 6D3F rcr bx,1
db 0xD1, 0xD9                           ; 6D41 rcr cx,1
db 0xFE, 0x06, 0xA6, 0x04               ; 6D43 inc byte [0x4a6]
db 0x75, 0x03                           ; 6D47 jnz 0x6d4c
db 0xE9, 0x90, 0x07                     ; 6D49 jmp 0x74dc
db 0x0A, 0xFF                           ; 6D4C or bh,bh
db 0x79, 0x09                           ; 6D4E jns 0x6d59
db 0xFE, 0x06, 0xA6, 0x04               ; 6D50 inc byte [0x4a6]
db 0x75, 0x07                           ; 6D54 jnz 0x6d5d
db 0xE9, 0x83, 0x07                     ; 6D56 jmp 0x74dc
db 0xD1, 0xD1                           ; 6D59 rcl cx,1
db 0xD1, 0xD3                           ; 6D5B rcl bx,1
db 0x8A, 0xD5                           ; 6D5D mov dl,ch
db 0x8A, 0xF3                           ; 6D5F mov dh,bl
db 0x8A, 0xDF                           ; 6D61 mov bl,bh
db 0x8A, 0xE1                           ; 6D63 mov ah,cl
db 0xE9, 0xEC, 0x0B                     ; 6D65 jmp 0x7954
db 0xC3                                 ; 6D68 ret
db 0x53                                 ; 6D69 push bx
db 0xB0, 0x08                           ; 6D6A mov al,0x8
db 0x72, 0x02                           ; 6D6C jc 0x6d70
db 0xB0, 0x11                           ; 6D6E mov al,0x11
db 0x8A, 0xE8                           ; 6D70 mov ch,al
db 0x8A, 0xC8                           ; 6D72 mov cl,al
db 0x51                                 ; 6D74 push cx
db 0x9C                                 ; 6D75 pushf
db 0xE8, 0x48, 0x02                     ; 6D76 call 0x6fc1
db 0x0A, 0xC0                           ; 6D79 or al,al
db 0x74, 0x02                           ; 6D7B jz 0x6d7f
db 0x79, 0x0C                           ; 6D7D jns 0x6d8b
db 0x9D                                 ; 6D7F popf
db 0x59                                 ; 6D80 pop cx
db 0x50                                 ; 6D81 push ax
db 0x7B, 0x0B                           ; 6D82 jpo 0x6d8f
db 0x04, 0x10                           ; 6D84 add al,0x10
db 0x58                                 ; 6D86 pop ax
db 0x79, 0x1A                           ; 6D87 jns 0x6da3
db 0xEB, 0x09                           ; 6D89 jmp short 0x6d94
db 0x9D                                 ; 6D8B popf
db 0x59                                 ; 6D8C pop cx
db 0xEB, 0x26                           ; 6D8D jmp short 0x6db5
db 0x04, 0x07                           ; 6D8F add al,0x7
db 0x58                                 ; 6D91 pop ax
db 0x79, 0x0F                           ; 6D92 jns 0x6da3
db 0x50                                 ; 6D94 push ax
db 0xE8, 0xF6, 0x0B                     ; 6D95 call 0x798e
db 0x58                                 ; 6D98 pop ax
db 0x8A, 0xE0                           ; 6D99 mov ah,al
db 0x02, 0xE1                           ; 6D9B add ah,cl
db 0x7E, 0x16                           ; 6D9D jng 0x6db5
db 0x02, 0xE8                           ; 6D9F add ch,al
db 0xEB, 0x0C                           ; 6DA1 jmp short 0x6daf
db 0x02, 0xC5                           ; 6DA3 add al,ch
db 0xFE, 0xC5                           ; 6DA5 inc ch
db 0x3A, 0xE8                           ; 6DA7 cmp ch,al
db 0xB5, 0x03                           ; 6DA9 mov ch,0x3
db 0x72, 0x0C                           ; 6DAB jc 0x6db9
db 0x8A, 0xE8                           ; 6DAD mov ch,al
db 0xFE, 0xC5                           ; 6DAF inc ch
db 0xB0, 0x02                           ; 6DB1 mov al,0x2
db 0xEB, 0x04                           ; 6DB3 jmp short 0x6db9
db 0x02, 0xC5                           ; 6DB5 add al,ch
db 0xB5, 0x03                           ; 6DB7 mov ch,0x3
db 0xFE, 0xC8                           ; 6DB9 dec al
db 0xFE, 0xC8                           ; 6DBB dec al
db 0x5B                                 ; 6DBD pop bx
db 0x50                                 ; 6DBE push ax
db 0x9C                                 ; 6DBF pushf
db 0x32, 0xC9                           ; 6DC0 xor cl,cl
db 0xE8, 0x4D, 0x00                     ; 6DC2 call 0x6e12
db 0xC6, 0x07, 0x30                     ; 6DC5 mov byte [bx],0x30
db 0x75, 0x01                           ; 6DC8 jnz 0x6dcb
db 0x43                                 ; 6DCA inc bx
db 0xE8, 0xE8, 0x00                     ; 6DCB call 0x6eb6
db 0x4B                                 ; 6DCE dec bx
db 0x80, 0x3F, 0x30                     ; 6DCF cmp byte [bx],0x30
db 0x74, 0xFA                           ; 6DD2 jz 0x6dce
db 0x80, 0x3F, 0x2E                     ; 6DD4 cmp byte [bx],0x2e
db 0x74, 0x01                           ; 6DD7 jz 0x6dda
db 0x43                                 ; 6DD9 inc bx
db 0x9D                                 ; 6DDA popf
db 0x58                                 ; 6DDB pop ax
db 0x74, 0x2B                           ; 6DDC jz 0x6e09
db 0x9C                                 ; 6DDE pushf
db 0x50                                 ; 6DDF push ax
db 0xE8, 0xBF, 0x0D                     ; 6DE0 call 0x7ba2
db 0xB4, 0x45                           ; 6DE3 mov ah,0x45
db 0x7B, 0x02                           ; 6DE5 jpo 0x6de9
db 0xB4, 0x44                           ; 6DE7 mov ah,0x44
db 0x88, 0x27                           ; 6DE9 mov [bx],ah
db 0x43                                 ; 6DEB inc bx
db 0x58                                 ; 6DEC pop ax
db 0x9D                                 ; 6DED popf
db 0xC6, 0x07, 0x2B                     ; 6DEE mov byte [bx],0x2b
db 0x79, 0x05                           ; 6DF1 jns 0x6df8
db 0xC6, 0x07, 0x2D                     ; 6DF3 mov byte [bx],0x2d
db 0xF6, 0xD8                           ; 6DF6 neg al
db 0xB4, 0x2F                           ; 6DF8 mov ah,0x2f
db 0xFE, 0xC4                           ; 6DFA inc ah
db 0x2C, 0x0A                           ; 6DFC sub al,0xa
db 0x73, 0xFA                           ; 6DFE jnc 0x6dfa
db 0x04, 0x3A                           ; 6E00 add al,0x3a
db 0x43                                 ; 6E02 inc bx
db 0x86, 0xC4                           ; 6E03 xchg al,ah
db 0x89, 0x07                           ; 6E05 mov [bx],ax
db 0x43                                 ; 6E07 inc bx
db 0x43                                 ; 6E08 inc bx
db 0xC6, 0x07, 0x00                     ; 6E09 mov byte [bx],0x0
db 0x87, 0xD9                           ; 6E0C xchg bx,cx
db 0xBB, 0xB4, 0x04                     ; 6E0E mov bx,0x4b4
db 0xC3                                 ; 6E11 ret
db 0xFE, 0xCD                           ; 6E12 dec ch
db 0x79, 0x16                           ; 6E14 jns 0x6e2c
db 0x89, 0x1E, 0x52, 0x03               ; 6E16 mov [0x352],bx
db 0xC6, 0x07, 0x2E                     ; 6E1A mov byte [bx],0x2e
db 0x43                                 ; 6E1D inc bx
db 0xC6, 0x07, 0x30                     ; 6E1E mov byte [bx],0x30
db 0xFE, 0xC5                           ; 6E21 inc ch
db 0x75, 0xF8                           ; 6E23 jnz 0x6e1d
db 0x43                                 ; 6E25 inc bx
db 0x33, 0xC9                           ; 6E26 xor cx,cx
db 0xEB, 0x1A                           ; 6E28 jmp short 0x6e44
db 0xFE, 0xCD                           ; 6E2A dec ch
db 0x75, 0x0C                           ; 6E2C jnz 0x6e3a
db 0xC6, 0x07, 0x2E                     ; 6E2E mov byte [bx],0x2e
db 0x89, 0x1E, 0x52, 0x03               ; 6E31 mov [0x352],bx
db 0x43                                 ; 6E35 inc bx
db 0x33, 0xC9                           ; 6E36 xor cx,cx
db 0xEB, 0x0A                           ; 6E38 jmp short 0x6e44
db 0xFE, 0xC9                           ; 6E3A dec cl
db 0x75, 0x06                           ; 6E3C jnz 0x6e44
db 0xC6, 0x07, 0x2C                     ; 6E3E mov byte [bx],0x2c
db 0x43                                 ; 6E41 inc bx
db 0xB1, 0x03                           ; 6E42 mov cl,0x3
db 0x89, 0x0E, 0x81, 0x04               ; 6E44 mov [0x481],cx
db 0xC3                                 ; 6E48 ret
db 0xB4, 0x05                           ; 6E49 mov ah,0x5
db 0xBD, 0xF5, 0x61                     ; 6E4B mov bp,0x61f5
db 0xE8, 0xD9, 0xFF                     ; 6E4E call 0x6e2a
db 0x2E, 0x8B, 0x96, 0x00, 0x00         ; 6E51 mov dx,[cs:bp+0x0]
db 0x45                                 ; 6E56 inc bp
db 0x45                                 ; 6E57 inc bp
db 0x8B, 0x36, 0xA3, 0x04               ; 6E58 mov si,[0x4a3]
db 0xB0, 0x2F                           ; 6E5C mov al,0x2f
db 0xFE, 0xC0                           ; 6E5E inc al
db 0x2B, 0xF2                           ; 6E60 sub si,dx
db 0x73, 0xFA                           ; 6E62 jnc 0x6e5e
db 0x03, 0xF2                           ; 6E64 add si,dx
db 0x88, 0x07                           ; 6E66 mov [bx],al
db 0x43                                 ; 6E68 inc bx
db 0x89, 0x36, 0xA3, 0x04               ; 6E69 mov [0x4a3],si
db 0xFE, 0xCC                           ; 6E6D dec ah
db 0x75, 0xDD                           ; 6E6F jnz 0x6e4e
db 0xE8, 0xB6, 0xFF                     ; 6E71 call 0x6e2a
db 0xC6, 0x07, 0x00                     ; 6E74 mov byte [bx],0x0
db 0xC3                                 ; 6E77 ret
db 0xB9, 0x01, 0x03                     ; 6E78 mov cx,0x301
db 0xBE, 0x06, 0x00                     ; 6E7B mov si,0x6
db 0xEB, 0x06                           ; 6E7E jmp short 0x6e86
db 0xB9, 0x04, 0x04                     ; 6E80 mov cx,0x404
db 0xBE, 0x04, 0x00                     ; 6E83 mov si,0x4
db 0xBF, 0xB3, 0x04                     ; 6E86 mov di,0x4b3
db 0xFC                                 ; 6E89 cld
db 0xBB, 0x74, 0x62                     ; 6E8A mov bx,0x6274
db 0x8B, 0x16, 0xA3, 0x04               ; 6E8D mov dx,[0x4a3]
db 0x56                                 ; 6E91 push si
db 0x8A, 0xC6                           ; 6E92 mov al,dh
db 0x32, 0xE4                           ; 6E94 xor ah,ah
db 0xD3, 0xE0                           ; 6E96 shl ax,cl
db 0x86, 0xE0                           ; 6E98 xchg ah,al
db 0x2E, 0xD7                           ; 6E9A cs xlatb
db 0xAA                                 ; 6E9C stosb
db 0xD3, 0xE2                           ; 6E9D shl dx,cl
db 0x8A, 0xCD                           ; 6E9F mov cl,ch
db 0x4E                                 ; 6EA1 dec si
db 0x75, 0xEE                           ; 6EA2 jnz 0x6e92
db 0xC6, 0x05, 0x00                     ; 6EA4 mov byte [di],0x0
db 0xBB, 0xB3, 0x04                     ; 6EA7 mov bx,0x4b3
db 0x59                                 ; 6EAA pop cx
db 0xFE, 0xC9                           ; 6EAB dec cl
db 0x80, 0x3F, 0x30                     ; 6EAD cmp byte [bx],0x30
db 0x75, 0x03                           ; 6EB0 jnz 0x6eb5
db 0x43                                 ; 6EB2 inc bx
db 0xE2, 0xF8                           ; 6EB3 loop 0x6ead
db 0xC3                                 ; 6EB5 ret
db 0xE8, 0xE9, 0x0C                     ; 6EB6 call 0x7ba2
db 0x7B, 0x77                           ; 6EB9 jpo 0x6f32
db 0x51                                 ; 6EBB push cx
db 0x53                                 ; 6EBC push bx
db 0xBE, 0x9F, 0x04                     ; 6EBD mov si,0x49f
db 0xBF, 0xAB, 0x04                     ; 6EC0 mov di,0x4ab
db 0xB9, 0x04, 0x00                     ; 6EC3 mov cx,0x4
db 0xFC                                 ; 6EC6 cld
db 0xF3, 0xA5                           ; 6EC7 rep movsw
db 0xE8, 0x75, 0x03                     ; 6EC9 call 0x7241
db 0x53                                 ; 6ECC push bx
db 0xBB, 0xB1, 0x04                     ; 6ECD mov bx,0x4b1
db 0xE8, 0xFD, 0x0D                     ; 6ED0 call 0x7cd0
db 0x5B                                 ; 6ED3 pop bx
db 0xBE, 0xAB, 0x04                     ; 6ED4 mov si,0x4ab
db 0xBF, 0x9F, 0x04                     ; 6ED7 mov di,0x49f
db 0xB9, 0x04, 0x00                     ; 6EDA mov cx,0x4
db 0xFC                                 ; 6EDD cld
db 0xF3, 0xA5                           ; 6EDE rep movsw
db 0x74, 0x03                           ; 6EE0 jz 0x6ee5
db 0xE8, 0xCE, 0x0C                     ; 6EE2 call 0x7bb3
db 0x8A, 0x0E, 0xA6, 0x04               ; 6EE5 mov cl,[0x4a6]
db 0x80, 0xE9, 0xB8                     ; 6EE9 sub cl,0xb8
db 0xF6, 0xD9                           ; 6EEC neg cl
db 0xF8                                 ; 6EEE clc
db 0xE8, 0x5A, 0x03                     ; 6EEF call 0x724c
db 0x5B                                 ; 6EF2 pop bx
db 0x59                                 ; 6EF3 pop cx
db 0xBE, 0xA6, 0x61                     ; 6EF4 mov si,0x61a6
db 0xB0, 0x09                           ; 6EF7 mov al,0x9
db 0xE8, 0x2E, 0xFF                     ; 6EF9 call 0x6e2a
db 0x50                                 ; 6EFC push ax
db 0xB0, 0x2F                           ; 6EFD mov al,0x2f
db 0x50                                 ; 6EFF push ax
db 0x58                                 ; 6F00 pop ax
db 0xFE, 0xC0                           ; 6F01 inc al
db 0x50                                 ; 6F03 push ax
db 0xE8, 0x94, 0x00                     ; 6F04 call 0x6f9b
db 0x73, 0xF7                           ; 6F07 jnc 0x6f00
db 0xE8, 0xA3, 0x00                     ; 6F09 call 0x6faf
db 0x58                                 ; 6F0C pop ax
db 0xEB, 0x0B                           ; 6F0D jmp short 0x6f1a
db 0x75, 0x09                           ; 6F0F jnz 0x6f1a
db 0xC6, 0x07, 0x31                     ; 6F11 mov byte [bx],0x31
db 0x43                                 ; 6F14 inc bx
db 0xC6, 0x07, 0x30                     ; 6F15 mov byte [bx],0x30
db 0xEB, 0x02                           ; 6F18 jmp short 0x6f1c
db 0x88, 0x07                           ; 6F1A mov [bx],al
db 0x43                                 ; 6F1C inc bx
db 0x58                                 ; 6F1D pop ax
db 0xFE, 0xC8                           ; 6F1E dec al
db 0x75, 0xD7                           ; 6F20 jnz 0x6ef9
db 0x51                                 ; 6F22 push cx
db 0xBE, 0x9F, 0x04                     ; 6F23 mov si,0x49f
db 0xBF, 0xA3, 0x04                     ; 6F26 mov di,0x4a3
db 0xB9, 0x02, 0x00                     ; 6F29 mov cx,0x2
db 0xFC                                 ; 6F2C cld
db 0xF3, 0xA5                           ; 6F2D rep movsw
db 0x59                                 ; 6F2F pop cx
db 0xEB, 0x29                           ; 6F30 jmp short 0x6f5b
db 0x53                                 ; 6F32 push bx
db 0x51                                 ; 6F33 push cx
db 0xE8, 0x18, 0x0F                     ; 6F34 call 0x7e4f
db 0xE8, 0x71, 0x03                     ; 6F37 call 0x72ab
db 0x5A                                 ; 6F3A pop dx
db 0x5B                                 ; 6F3B pop bx
db 0xE8, 0x99, 0x0D                     ; 6F3C call 0x7cd8
db 0x74, 0x0B                           ; 6F3F jz 0x6f4c
db 0x89, 0x1E, 0xA5, 0x04               ; 6F41 mov [0x4a5],bx
db 0x89, 0x16, 0xA3, 0x04               ; 6F45 mov [0x4a3],dx
db 0xE8, 0x73, 0x0C                     ; 6F49 call 0x7bbf
db 0xB0, 0x01                           ; 6F4C mov al,0x1
db 0xE8, 0xB2, 0x03                     ; 6F4E call 0x7303
db 0x89, 0x1E, 0xA5, 0x04               ; 6F51 mov [0x4a5],bx
db 0x89, 0x16, 0xA3, 0x04               ; 6F55 mov [0x4a3],dx
db 0x59                                 ; 6F59 pop cx
db 0x5B                                 ; 6F5A pop bx
db 0xB0, 0x03                           ; 6F5B mov al,0x3
db 0xBA, 0xEC, 0x61                     ; 6F5D mov dx,0x61ec
db 0xE8, 0xC7, 0xFE                     ; 6F60 call 0x6e2a
db 0x50                                 ; 6F63 push ax
db 0x53                                 ; 6F64 push bx
db 0x52                                 ; 6F65 push dx
db 0xE8, 0x5E, 0x0D                     ; 6F66 call 0x7cc7
db 0x5D                                 ; 6F69 pop bp
db 0xB0, 0x2F                           ; 6F6A mov al,0x2f
db 0x50                                 ; 6F6C push ax
db 0x58                                 ; 6F6D pop ax
db 0xFE, 0xC0                           ; 6F6E inc al
db 0x50                                 ; 6F70 push ax
db 0xE8, 0x17, 0x0E                     ; 6F71 call 0x7d8b
db 0x73, 0xF7                           ; 6F74 jnc 0x6f6d
db 0x2E, 0x03, 0x96, 0x00, 0x00         ; 6F76 add dx,[cs:bp+0x0]
db 0x2E, 0x12, 0x9E, 0x02, 0x00         ; 6F7B adc bl,[cs:bp+0x2]
db 0x45                                 ; 6F80 inc bp
db 0x45                                 ; 6F81 inc bp
db 0x45                                 ; 6F82 inc bp
db 0xE8, 0x38, 0x0D                     ; 6F83 call 0x7cbe
db 0x58                                 ; 6F86 pop ax
db 0x87, 0xD5                           ; 6F87 xchg dx,bp
db 0x5B                                 ; 6F89 pop bx
db 0x88, 0x07                           ; 6F8A mov [bx],al
db 0x43                                 ; 6F8C inc bx
db 0x58                                 ; 6F8D pop ax
db 0xFE, 0xC8                           ; 6F8E dec al
db 0x75, 0xCE                           ; 6F90 jnz 0x6f60
db 0x42                                 ; 6F92 inc dx
db 0x42                                 ; 6F93 inc dx
db 0x8B, 0xEA                           ; 6F94 mov bp,dx
db 0xB4, 0x04                           ; 6F96 mov ah,0x4
db 0xE9, 0xB3, 0xFE                     ; 6F98 jmp 0x6e4e
db 0x51                                 ; 6F9B push cx
db 0x56                                 ; 6F9C push si
db 0xB9, 0x07, 0x00                     ; 6F9D mov cx,0x7
db 0xBF, 0x9F, 0x04                     ; 6FA0 mov di,0x49f
db 0xF8                                 ; 6FA3 clc
db 0xFC                                 ; 6FA4 cld
db 0x2E, 0xAC                           ; 6FA5 cs lodsb
db 0x18, 0x05                           ; 6FA7 sbb [di],al
db 0x47                                 ; 6FA9 inc di
db 0xE2, 0xF9                           ; 6FAA loop 0x6fa5
db 0x5E                                 ; 6FAC pop si
db 0x59                                 ; 6FAD pop cx
db 0xC3                                 ; 6FAE ret
db 0x51                                 ; 6FAF push cx
db 0xB9, 0x07, 0x00                     ; 6FB0 mov cx,0x7
db 0xBF, 0x9F, 0x04                     ; 6FB3 mov di,0x49f
db 0xF8                                 ; 6FB6 clc
db 0xFC                                 ; 6FB7 cld
db 0x2E, 0xAC                           ; 6FB8 cs lodsb
db 0x10, 0x05                           ; 6FBA adc [di],al
db 0x47                                 ; 6FBC inc di
db 0xE2, 0xF9                           ; 6FBD loop 0x6fb8
db 0x59                                 ; 6FBF pop cx
db 0xC3                                 ; 6FC0 ret
db 0x53                                 ; 6FC1 push bx
db 0x51                                 ; 6FC2 push cx
db 0x33, 0xFF                           ; 6FC3 xor di,di
db 0x57                                 ; 6FC5 push di
db 0xBB, 0x02, 0x5E                     ; 6FC6 mov bx,0x5e02
db 0xA0, 0xA6, 0x04                     ; 6FC9 mov al,[0x4a6]
db 0x2E, 0xD7                           ; 6FCC cs xlatb
db 0x0A, 0xC0                           ; 6FCE or al,al
db 0x74, 0x0C                           ; 6FD0 jz 0x6fde
db 0x5F                                 ; 6FD2 pop di
db 0x98                                 ; 6FD3 cbw
db 0x2B, 0xF8                           ; 6FD4 sub di,ax
db 0x57                                 ; 6FD6 push di
db 0x8B, 0xD0                           ; 6FD7 mov dx,ax
db 0xE8, 0x32, 0xEF                     ; 6FD9 call 0x5f0e
db 0xEB, 0xE8                           ; 6FDC jmp short 0x6fc6
db 0xBB, 0x66, 0x60                     ; 6FDE mov bx,0x6066
db 0xE8, 0x88, 0x0C                     ; 6FE1 call 0x7c6c
db 0xE8, 0x2D, 0x0D                     ; 6FE4 call 0x7d14
db 0x73, 0x06                           ; 6FE7 jnc 0x6fef
db 0xE8, 0xE6, 0x0B                     ; 6FE9 call 0x7bd2
db 0x5F                                 ; 6FEC pop di
db 0x4F                                 ; 6FED dec di
db 0x57                                 ; 6FEE push di
db 0xE8, 0xB0, 0x0B                     ; 6FEF call 0x7ba2
db 0x72, 0x1F                           ; 6FF2 jc 0x7013
db 0xBB, 0x7A, 0x60                     ; 6FF4 mov bx,0x607a
db 0xE8, 0x8E, 0x0C                     ; 6FF7 call 0x7c88
db 0xE8, 0x58, 0xFC                     ; 6FFA call 0x6c55
db 0x58                                 ; 6FFD pop ax
db 0x2C, 0x09                           ; 6FFE sub al,0x9
db 0x50                                 ; 7000 push ax
db 0xBB, 0xF6, 0x7F                     ; 7001 mov bx,0x7ff6
db 0xE8, 0x6D, 0x0C                     ; 7004 call 0x7c74
db 0xE8, 0x57, 0x0D                     ; 7007 call 0x7d61
db 0x76, 0x07                           ; 700A jna 0x7013
db 0xE8, 0xB9, 0x0B                     ; 700C call 0x7bc8
db 0x58                                 ; 700F pop ax
db 0xFE, 0xC0                           ; 7010 inc al
db 0x50                                 ; 7012 push ax
db 0x58                                 ; 7013 pop ax
db 0x59                                 ; 7014 pop cx
db 0x5B                                 ; 7015 pop bx
db 0x0A, 0xC0                           ; 7016 or al,al
db 0xC3                                 ; 7018 ret
db 0x58                                 ; 7019 pop ax
db 0x59                                 ; 701A pop cx
db 0x5B                                 ; 701B pop bx
db 0x0A, 0xC0                           ; 701C or al,al
db 0xC3                                 ; 701E ret
db 0xBB, 0xB4, 0x04                     ; 701F mov bx,0x4b4
db 0x8A, 0x2F                           ; 7022 mov ch,[bx]
db 0xB1, 0x20                           ; 7024 mov cl,0x20
db 0x8A, 0x26, 0x83, 0x04               ; 7026 mov ah,[0x483]
db 0xF6, 0xC4, 0x20                     ; 702A test ah,0x20
db 0x74, 0x0D                           ; 702D jz 0x703c
db 0x3A, 0xE9                           ; 702F cmp ch,cl
db 0xB1, 0x2A                           ; 7031 mov cl,0x2a
db 0x75, 0x07                           ; 7033 jnz 0x703c
db 0xF6, 0xC4, 0x04                     ; 7035 test ah,0x4
db 0x75, 0x02                           ; 7038 jnz 0x703c
db 0x8A, 0xE9                           ; 703A mov ch,cl
db 0x88, 0x0F                           ; 703C mov [bx],cl
db 0xE8, 0xBF, 0xF2                     ; 703E call 0x6300
db 0x74, 0x32                           ; 7041 jz 0x7075
db 0xBD, 0xA5, 0x61                     ; 7043 mov bp,0x61a5
db 0x2E, 0x3A, 0x86, 0x00, 0x00         ; 7046 cmp al,[cs:bp+0x0]
db 0x74, 0x09                           ; 704B jz 0x7056
db 0x81, 0xFD, 0x9C, 0x61               ; 704D cmp bp,0x619c
db 0x74, 0x26                           ; 7051 jz 0x7079
db 0x4D                                 ; 7053 dec bp
db 0xEB, 0xF0                           ; 7054 jmp short 0x7046
db 0x81, 0xED, 0x9C, 0x61               ; 7056 sub bp,0x619c
db 0xD1, 0xE5                           ; 705A shl bp,1
db 0x2E, 0xFF, 0xA6, 0x61, 0x70         ; 705C jmp near [cs:bp+0x7061]
db 0x75, 0x70                           ; 7061 jnz 0x70d3
db 0x75, 0x70                           ; 7063 jnz 0x70d5
db 0x79, 0x70                           ; 7065 jns 0x70d7
db 0x79, 0x70                           ; 7067 jns 0x70d9
db 0x79, 0x70                           ; 7069 jns 0x70db
db 0x79, 0x70                           ; 706B jns 0x70dd
db 0x75, 0x70                           ; 706D jnz 0x70df
db 0x79, 0x70                           ; 706F jns 0x70e1
db 0x3C, 0x70                           ; 7071 cmp al,0x70
db 0x3C, 0x70                           ; 7073 cmp al,0x70
db 0x4B                                 ; 7075 dec bx
db 0xC6, 0x07, 0x30                     ; 7076 mov byte [bx],0x30
db 0x8A, 0x26, 0x83, 0x04               ; 7079 mov ah,[0x483]
db 0xF6, 0xC4, 0x10                     ; 707D test ah,0x10
db 0x74, 0x04                           ; 7080 jz 0x7086
db 0x4B                                 ; 7082 dec bx
db 0xC6, 0x07, 0x24                     ; 7083 mov byte [bx],0x24
db 0xF6, 0xC4, 0x04                     ; 7086 test ah,0x4
db 0x75, 0x05                           ; 7089 jnz 0x7090
db 0x4B                                 ; 708B dec bx
db 0x88, 0x2F                           ; 708C mov [bx],ch
db 0x32, 0xED                           ; 708E xor ch,ch
db 0xC3                                 ; 7090 ret
db 0x0A, 0xC0                           ; 7091 or al,al
db 0xEB, 0x06                           ; 7093 jmp short 0x709b
db 0xC6, 0x07, 0x30                     ; 7095 mov byte [bx],0x30
db 0x43                                 ; 7098 inc bx
db 0xFE, 0xC8                           ; 7099 dec al
db 0x75, 0xF8                           ; 709B jnz 0x7095
db 0xC3                                 ; 709D ret
db 0xE8, 0x89, 0xFD                     ; 709E call 0x6e2a
db 0xC6, 0x07, 0x30                     ; 70A1 mov byte [bx],0x30
db 0x43                                 ; 70A4 inc bx
db 0xFE, 0xC8                           ; 70A5 dec al
db 0x75, 0xF5                           ; 70A7 jnz 0x709e
db 0xC3                                 ; 70A9 ret
db 0xBB, 0xB4, 0x04                     ; 70AA mov bx,0x4b4
db 0xC6, 0x07, 0x20                     ; 70AD mov byte [bx],0x20
db 0x53                                 ; 70B0 push bx
db 0xE8, 0xC1, 0x0A                     ; 70B1 call 0x7b75
db 0x5B                                 ; 70B4 pop bx
db 0x9C                                 ; 70B5 pushf
db 0x79, 0x0A                           ; 70B6 jns 0x70c2
db 0xC6, 0x07, 0x2D                     ; 70B8 mov byte [bx],0x2d
db 0x53                                 ; 70BB push bx
db 0xE8, 0xEC, 0x0C                     ; 70BC call 0x7dab
db 0x5B                                 ; 70BF pop bx
db 0x0C, 0x01                           ; 70C0 or al,0x1
db 0x43                                 ; 70C2 inc bx
db 0xC6, 0x07, 0x30                     ; 70C3 mov byte [bx],0x30
db 0x9D                                 ; 70C6 popf
db 0xC3                                 ; 70C7 ret
db 0xCD, 0xD8                           ; 70C8 int 0xd8
db 0xE8, 0xDD, 0xFF                     ; 70CA call 0x70aa
db 0x75, 0x08                           ; 70CD jnz 0x70d7
db 0x43                                 ; 70CF inc bx
db 0xC6, 0x07, 0x00                     ; 70D0 mov byte [bx],0x0
db 0xBB, 0xB4, 0x04                     ; 70D3 mov bx,0x4b4
db 0xC3                                 ; 70D6 ret
db 0xE8, 0xC8, 0x0A                     ; 70D7 call 0x7ba2
db 0x79, 0x12                           ; 70DA jns 0x70ee
db 0xB9, 0x00, 0x07                     ; 70DC mov cx,0x700
db 0x33, 0xC0                           ; 70DF xor ax,ax
db 0xA3, 0x83, 0x04                     ; 70E1 mov [0x483],ax
db 0x89, 0x0E, 0x81, 0x04               ; 70E4 mov [0x481],cx
db 0xE8, 0x5E, 0xFD                     ; 70E8 call 0x6e49
db 0xE9, 0x31, 0xFF                     ; 70EB jmp 0x701f
db 0xE9, 0x78, 0xFC                     ; 70EE jmp 0x6d69
db 0xE8, 0x81, 0x0A                     ; 70F1 call 0x7b75
db 0x79, 0x03                           ; 70F4 jns 0x70f9
db 0xE9, 0x60, 0x9F                     ; 70F6 jmp 0x1059
db 0x75, 0x01                           ; 70F9 jnz 0x70fc
db 0xC3                                 ; 70FB ret
db 0xA0, 0xA6, 0x04                     ; 70FC mov al,[0x4a6]
db 0xD0, 0xE8                           ; 70FF shr al,1
db 0x50                                 ; 7101 push ax
db 0xC6, 0x06, 0xA6, 0x04, 0x40         ; 7102 mov byte [0x4a6],0x40
db 0xD0, 0x16, 0xA6, 0x04               ; 7107 rcl byte [0x4a6],1
db 0xBB, 0xAB, 0x04                     ; 710B mov bx,0x4ab
db 0xE8, 0x09, 0x0D                     ; 710E call 0x7e1a
db 0xB9, 0x04, 0x00                     ; 7111 mov cx,0x4
db 0x51                                 ; 7114 push cx
db 0xE8, 0x37, 0x0D                     ; 7115 call 0x7e4f
db 0x8B, 0x16, 0xAB, 0x04               ; 7118 mov dx,[0x4ab]
db 0x8B, 0x1E, 0xAD, 0x04               ; 711C mov bx,[0x4ad]
db 0xE8, 0xBB, 0xF7                     ; 7120 call 0x68de
db 0x5A                                 ; 7123 pop dx
db 0x5B                                 ; 7124 pop bx
db 0xE8, 0x4B, 0xF6                     ; 7125 call 0x6773
db 0xFE, 0x0E, 0xA6, 0x04               ; 7128 dec byte [0x4a6]
db 0x59                                 ; 712C pop cx
db 0x74, 0x0A                           ; 712D jz 0x7139
db 0xE2, 0xE3                           ; 712F loop 0x7114
db 0x58                                 ; 7131 pop ax
db 0x04, 0xC0                           ; 7132 add al,0xc0
db 0x00, 0x06, 0xA6, 0x04               ; 7134 add [0x4a6],al
db 0xC3                                 ; 7138 ret
db 0xE9, 0x2F, 0x0A                     ; 7139 jmp 0x7b6b
db 0xBF, 0xBE, 0x25                     ; 713C mov di,0x25be
db 0x57                                 ; 713F push di
db 0xBF, 0xA8, 0x04                     ; 7140 mov di,0x4a8
db 0xC6, 0x05, 0x01                     ; 7143 mov byte [di],0x1
db 0xE8, 0x2C, 0x0A                     ; 7146 call 0x7b75
db 0x75, 0x03                           ; 7149 jnz 0x714e
db 0xE9, 0x36, 0xF1                     ; 714B jmp 0x6284
db 0x79, 0x07                           ; 714E jns 0x7157
db 0x0A, 0xFF                           ; 7150 or bh,bh
db 0x75, 0x0A                           ; 7152 jnz 0x715e
db 0xE9, 0x93, 0x03                     ; 7154 jmp 0x74ea
db 0x0A, 0xFF                           ; 7157 or bh,bh
db 0x75, 0x03                           ; 7159 jnz 0x715e
db 0xE9, 0x0D, 0x0A                     ; 715B jmp 0x7b6b
db 0x0A, 0xDB                           ; 715E or bl,bl
db 0x79, 0x26                           ; 7160 jns 0x7188
db 0x80, 0x3E, 0xA6, 0x04, 0x99         ; 7162 cmp byte [0x4a6],0x99
db 0x72, 0x03                           ; 7167 jc 0x716c
db 0xE9, 0xED, 0x9E                     ; 7169 jmp 0x1059
db 0x52                                 ; 716C push dx
db 0x53                                 ; 716D push bx
db 0xFF, 0x36, 0xA3, 0x04               ; 716E push word [0x4a3]
db 0xFF, 0x36, 0xA5, 0x04               ; 7172 push word [0x4a5]
db 0xE8, 0x32, 0x01                     ; 7176 call 0x72ab
db 0x5B                                 ; 7179 pop bx
db 0x5A                                 ; 717A pop dx
db 0xE8, 0x5A, 0x0B                     ; 717B call 0x7cd8
db 0xE8, 0x3D, 0x0B                     ; 717E call 0x7cbe
db 0x5B                                 ; 7181 pop bx
db 0x5A                                 ; 7182 pop dx
db 0x74, 0x03                           ; 7183 jz 0x7188
db 0xE9, 0xD1, 0x9E                     ; 7185 jmp 0x1059
db 0xA0, 0xA5, 0x04                     ; 7188 mov al,[0x4a5]
db 0x0A, 0xC0                           ; 718B or al,al
db 0x79, 0x09                           ; 718D jns 0x7198
db 0xBF, 0xC4, 0x71                     ; 718F mov di,0x71c4
db 0x57                                 ; 7192 push di
db 0x24, 0x7F                           ; 7193 and al,0x7f
db 0xA2, 0xA5, 0x04                     ; 7195 mov [0x4a5],al
db 0x53                                 ; 7198 push bx
db 0x52                                 ; 7199 push dx
db 0x80, 0xCB, 0x7F                     ; 719A or bl,0x7f
db 0x9C                                 ; 719D pushf
db 0xFF, 0x36, 0xA5, 0x04               ; 719E push word [0x4a5]
db 0xFF, 0x36, 0xA3, 0x04               ; 71A2 push word [0x4a3]
db 0xE8, 0x02, 0x01                     ; 71A6 call 0x72ab
db 0x5A                                 ; 71A9 pop dx
db 0x5B                                 ; 71AA pop bx
db 0xE8, 0x2A, 0x0B                     ; 71AB call 0x7cd8
db 0x75, 0x1C                           ; 71AE jnz 0x71cc
db 0x52                                 ; 71B0 push dx
db 0x53                                 ; 71B1 push bx
db 0x33, 0xD2                           ; 71B2 xor dx,dx
db 0xBB, 0x00, 0x90                     ; 71B4 mov bx,0x9000
db 0xE8, 0x1E, 0x0B                     ; 71B7 call 0x7cd8
db 0x5B                                 ; 71BA pop bx
db 0x5A                                 ; 71BB pop dx
db 0x79, 0x0E                           ; 71BC jns 0x71cc
db 0x9D                                 ; 71BE popf
db 0x5A                                 ; 71BF pop dx
db 0x5B                                 ; 71C0 pop bx
db 0xEB, 0x3C                           ; 71C1 jmp short 0x71ff
db 0x90                                 ; 71C3 nop
db 0x33, 0xD2                           ; 71C4 xor dx,dx
db 0xBB, 0x00, 0x81                     ; 71C6 mov bx,0x8100
db 0xE9, 0x12, 0xF7                     ; 71C9 jmp 0x68de
db 0x9D                                 ; 71CC popf
db 0x79, 0x0E                           ; 71CD jns 0x71dd
db 0x53                                 ; 71CF push bx
db 0x52                                 ; 71D0 push dx
db 0xE8, 0x2F, 0x01                     ; 71D1 call 0x7303
db 0x8A, 0xC2                           ; 71D4 mov al,dl
db 0xE8, 0xC5, 0x02                     ; 71D6 call 0x749e
db 0x5A                                 ; 71D9 pop dx
db 0x5B                                 ; 71DA pop bx
db 0xD0, 0xD8                           ; 71DB rcr al,1
db 0x8F, 0x06, 0xA3, 0x04               ; 71DD pop word [0x4a3]
db 0x8F, 0x06, 0xA5, 0x04               ; 71E1 pop word [0x4a5]
db 0x9F                                 ; 71E5 lahf
db 0x80, 0x26, 0xA5, 0x04, 0x7F         ; 71E6 and byte [0x4a5],0x7f
db 0x9E                                 ; 71EB sahf
db 0x73, 0x04                           ; 71EC jnc 0x71f2
db 0xBF, 0xB0, 0x7D                     ; 71EE mov di,0x7db0
db 0x57                                 ; 71F1 push di
db 0x53                                 ; 71F2 push bx
db 0x52                                 ; 71F3 push dx
db 0xE8, 0x15, 0x01                     ; 71F4 call 0x730c
db 0x5A                                 ; 71F7 pop dx
db 0x5B                                 ; 71F8 pop bx
db 0xE8, 0x03, 0xFB                     ; 71F9 call 0x6cff
db 0xE9, 0x85, 0xF0                     ; 71FC jmp 0x6284
db 0x53                                 ; 71FF push bx
db 0x52                                 ; 7200 push dx
db 0xE8, 0xFF, 0x00                     ; 7201 call 0x7303
db 0x89, 0x16, 0xB2, 0x04               ; 7204 mov [0x4b2],dx
db 0xC7, 0x06, 0xA3, 0x04, 0x00, 0x00   ; 7208 mov word [0x4a3],0x0
db 0xC7, 0x06, 0xA5, 0x04, 0x00, 0x81   ; 720E mov word [0x4a5],0x8100
db 0xD1, 0x2E, 0xB2, 0x04               ; 7214 shr word [0x4b2],1
db 0x73, 0x07                           ; 7218 jnc 0x7221
db 0x5A                                 ; 721A pop dx
db 0x5B                                 ; 721B pop bx
db 0x53                                 ; 721C push bx
db 0x52                                 ; 721D push dx
db 0xE8, 0xDE, 0xFA                     ; 721E call 0x6cff
db 0xF7, 0x06, 0xB2, 0x04, 0xFF, 0xFF   ; 7221 test word [0x4b2],0xffff
db 0x74, 0x15                           ; 7227 jz 0x723e
db 0x5A                                 ; 7229 pop dx
db 0x5B                                 ; 722A pop bx
db 0xE8, 0x21, 0x0C                     ; 722B call 0x7e4f
db 0xE8, 0x8D, 0x0A                     ; 722E call 0x7cbe
db 0xE8, 0xCB, 0xFA                     ; 7231 call 0x6cff
db 0x5A                                 ; 7234 pop dx
db 0x5B                                 ; 7235 pop bx
db 0xE8, 0x16, 0x0C                     ; 7236 call 0x7e4f
db 0xE8, 0x82, 0x0A                     ; 7239 call 0x7cbe
db 0xEB, 0xD6                           ; 723C jmp short 0x7214
db 0x5A                                 ; 723E pop dx
db 0x5B                                 ; 723F pop bx
db 0xC3                                 ; 7240 ret
db 0x8A, 0x0E, 0xA6, 0x04               ; 7241 mov cl,[0x4a6]
db 0x80, 0xE9, 0xB8                     ; 7245 sub cl,0xb8
db 0x73, 0x39                           ; 7248 jnc 0x7283
db 0xF6, 0xD9                           ; 724A neg cl
db 0x9C                                 ; 724C pushf
db 0xBB, 0xA4, 0x04                     ; 724D mov bx,0x4a4
db 0x8A, 0x87, 0x01, 0x00               ; 7250 mov al,[bx+0x1]
db 0x88, 0x87, 0x03, 0x00               ; 7254 mov [bx+0x3],al
db 0x0A, 0xC0                           ; 7258 or al,al
db 0x9C                                 ; 725A pushf
db 0x0C, 0x80                           ; 725B or al,0x80
db 0x88, 0x87, 0x01, 0x00               ; 725D mov [bx+0x1],al
db 0xC6, 0x87, 0x02, 0x00, 0xB8         ; 7261 mov byte [bx+0x2],0xb8
db 0x9D                                 ; 7266 popf
db 0x9C                                 ; 7267 pushf
db 0x79, 0x03                           ; 7268 jns 0x726d
db 0xE8, 0x22, 0x00                     ; 726A call 0x728f
db 0x32, 0xED                           ; 726D xor ch,ch
db 0xE8, 0x12, 0x00                     ; 726F call 0x7284
db 0x9D                                 ; 7272 popf
db 0x79, 0x03                           ; 7273 jns 0x7278
db 0xE8, 0x26, 0x00                     ; 7275 call 0x729e
db 0xC6, 0x06, 0x9E, 0x04, 0x00         ; 7278 mov byte [0x49e],0x0
db 0x9D                                 ; 727D popf
db 0x73, 0x03                           ; 727E jnc 0x7283
db 0xE9, 0xBD, 0x01                     ; 7280 jmp 0x7440
db 0xC3                                 ; 7283 ret
db 0x51                                 ; 7284 push cx
db 0x53                                 ; 7285 push bx
db 0xF8                                 ; 7286 clc
db 0xE8, 0x7A, 0x09                     ; 7287 call 0x7c04
db 0x5B                                 ; 728A pop bx
db 0x59                                 ; 728B pop cx
db 0xE2, 0xF6                           ; 728C loop 0x7284
db 0xC3                                 ; 728E ret
db 0x53                                 ; 728F push bx
db 0xBB, 0x9F, 0x04                     ; 7290 mov bx,0x49f
db 0x83, 0x2F, 0x01                     ; 7293 sub word [bx],byte +0x1
db 0x73, 0x04                           ; 7296 jnc 0x729c
db 0x43                                 ; 7298 inc bx
db 0x43                                 ; 7299 inc bx
db 0xEB, 0xF7                           ; 729A jmp short 0x7293
db 0x5B                                 ; 729C pop bx
db 0xC3                                 ; 729D ret
db 0x53                                 ; 729E push bx
db 0xBB, 0x9F, 0x04                     ; 729F mov bx,0x49f
db 0xFE, 0x07                           ; 72A2 inc byte [bx]
db 0x75, 0x03                           ; 72A4 jnz 0x72a9
db 0x43                                 ; 72A6 inc bx
db 0xEB, 0xF9                           ; 72A7 jmp short 0x72a2
db 0x5B                                 ; 72A9 pop bx
db 0xC3                                 ; 72AA ret
db 0x8A, 0x0E, 0xA6, 0x04               ; 72AB mov cl,[0x4a6]
db 0x80, 0xE9, 0x98                     ; 72AF sub cl,0x98
db 0x73, 0x41                           ; 72B2 jnc 0x72f5
db 0xF6, 0xD9                           ; 72B4 neg cl
db 0x9C                                 ; 72B6 pushf
db 0x8B, 0x16, 0xA3, 0x04               ; 72B7 mov dx,[0x4a3]
db 0x8B, 0x1E, 0xA5, 0x04               ; 72BB mov bx,[0x4a5]
db 0x0A, 0xDB                           ; 72BF or bl,bl
db 0x9C                                 ; 72C1 pushf
db 0x88, 0x1E, 0xA7, 0x04               ; 72C2 mov [0x4a7],bl
db 0xC6, 0x06, 0xA6, 0x04, 0x98         ; 72C6 mov byte [0x4a6],0x98
db 0x80, 0xCB, 0x80                     ; 72CB or bl,0x80
db 0x9D                                 ; 72CE popf
db 0x9C                                 ; 72CF pushf
db 0x79, 0x06                           ; 72D0 jns 0x72d8
db 0x83, 0xEA, 0x01                     ; 72D2 sub dx,byte +0x1
db 0x80, 0xDB, 0x00                     ; 72D5 sbb bl,0x0
db 0x32, 0xED                           ; 72D8 xor ch,ch
db 0x0A, 0xC9                           ; 72DA or cl,cl
db 0x74, 0x06                           ; 72DC jz 0x72e4
db 0xD0, 0xEB                           ; 72DE shr bl,1
db 0xD1, 0xDA                           ; 72E0 rcr dx,1
db 0xE2, 0xFA                           ; 72E2 loop 0x72de
db 0x9D                                 ; 72E4 popf
db 0x9F                                 ; 72E5 lahf
db 0x79, 0x05                           ; 72E6 jns 0x72ed
db 0x42                                 ; 72E8 inc dx
db 0x75, 0x02                           ; 72E9 jnz 0x72ed
db 0xFE, 0xC3                           ; 72EB inc bl
db 0x9D                                 ; 72ED popf
db 0x73, 0x05                           ; 72EE jnc 0x72f5
db 0x32, 0xE4                           ; 72F0 xor ah,ah
db 0xE9, 0xA9, 0x01                     ; 72F2 jmp 0x749e
db 0x9E                                 ; 72F5 sahf
db 0x79, 0x0A                           ; 72F6 jns 0x7302
db 0xF7, 0xD2                           ; 72F8 not dx
db 0xF6, 0xD3                           ; 72FA not bl
db 0x83, 0xC2, 0x01                     ; 72FC add dx,byte +0x1
db 0x80, 0xD3, 0x00                     ; 72FF adc bl,0x0
db 0xC3                                 ; 7302 ret
db 0xB1, 0x98                           ; 7303 mov cl,0x98
db 0x2A, 0x0E, 0xA6, 0x04               ; 7305 sub cl,[0x4a6]
db 0xF8                                 ; 7309 clc
db 0xEB, 0xAA                           ; 730A jmp short 0x72b6
db 0xE8, 0x66, 0x08                     ; 730C call 0x7b75
db 0x7E, 0x51                           ; 730F jng 0x7362
db 0xBA, 0x00, 0x00                     ; 7311 mov dx,0x0
db 0xBB, 0x00, 0x81                     ; 7314 mov bx,0x8100
db 0xE8, 0xBE, 0x09                     ; 7317 call 0x7cd8
db 0x75, 0x09                           ; 731A jnz 0x7325
db 0x89, 0x16, 0xA3, 0x04               ; 731C mov [0x4a3],dx
db 0x89, 0x16, 0xA5, 0x04               ; 7320 mov [0x4a5],dx
db 0xC3                                 ; 7324 ret
db 0xA0, 0xA6, 0x04                     ; 7325 mov al,[0x4a6]
db 0x2C, 0x80                           ; 7328 sub al,0x80
db 0x98                                 ; 732A cbw
db 0x50                                 ; 732B push ax
db 0xC6, 0x06, 0xA6, 0x04, 0x80         ; 732C mov byte [0x4a6],0x80
db 0xE8, 0x1B, 0x0B                     ; 7331 call 0x7e4f
db 0xBB, 0x76, 0x61                     ; 7334 mov bx,0x6176
db 0xE8, 0x18, 0x02                     ; 7337 call 0x7552
db 0x5A                                 ; 733A pop dx
db 0x5B                                 ; 733B pop bx
db 0xE8, 0x10, 0x0B                     ; 733C call 0x7e4f
db 0xE8, 0x7C, 0x09                     ; 733F call 0x7cbe
db 0xBB, 0x87, 0x61                     ; 7342 mov bx,0x6187
db 0xE8, 0x0A, 0x02                     ; 7345 call 0x7552
db 0x5A                                 ; 7348 pop dx
db 0x5B                                 ; 7349 pop bx
db 0xE8, 0x91, 0xF5                     ; 734A call 0x68de
db 0x5A                                 ; 734D pop dx
db 0xE8, 0xFE, 0x0A                     ; 734E call 0x7e4f
db 0xE8, 0xD9, 0xF8                     ; 7351 call 0x6c2d
db 0x5A                                 ; 7354 pop dx
db 0x5B                                 ; 7355 pop bx
db 0xE8, 0x1A, 0xF4                     ; 7356 call 0x6773
db 0xBB, 0x31, 0x80                     ; 7359 mov bx,0x8031
db 0xBA, 0x18, 0x72                     ; 735C mov dx,0x7218
db 0xE9, 0x9D, 0xF9                     ; 735F jmp 0x6cff
db 0xE9, 0xF4, 0x9C                     ; 7362 jmp 0x1059
db 0xE9, 0x5C, 0x94                     ; 7365 jmp 0x7c4
db 0x9F                                 ; 7368 lahf
db 0x86, 0xE0                           ; 7369 xchg ah,al
db 0x50                                 ; 736B push ax
db 0xB0, 0x01                           ; 736C mov al,0x1
db 0xEB, 0x02                           ; 736E jmp short 0x7372
db 0x32, 0xC0                           ; 7370 xor al,al
db 0xA2, 0x55, 0x04                     ; 7372 mov [0x455],al
db 0x58                                 ; 7375 pop ax
db 0x86, 0xC4                           ; 7376 xchg al,ah
db 0x9E                                 ; 7378 sahf
db 0xBA, 0x00, 0x00                     ; 7379 mov dx,0x0
db 0x89, 0x1E, 0x53, 0x04               ; 737C mov [0x453],bx
db 0x74, 0x03                           ; 7380 jz 0x7385
db 0xE8, 0xE9, 0xC3                     ; 7382 call 0x376e
db 0x89, 0x1E, 0x3B, 0x03               ; 7385 mov [0x33b],bx
db 0xE8, 0xAC, 0x93                     ; 7389 call 0x738
db 0x75, 0xD7                           ; 738C jnz 0x7365
db 0x8B, 0xE3                           ; 738E mov sp,bx
db 0x8B, 0x36, 0x53, 0x04               ; 7390 mov si,[0x453]
db 0x39, 0x37                           ; 7394 cmp [bx],si
db 0x75, 0xCD                           ; 7396 jnz 0x7365
db 0x52                                 ; 7398 push dx
db 0x8A, 0xA7, 0x02, 0x00               ; 7399 mov ah,[bx+0x2]
db 0x50                                 ; 739D push ax
db 0x52                                 ; 739E push dx
db 0x83, 0xC3, 0x04                     ; 739F add bx,byte +0x4
db 0xF6, 0x87, 0xFF, 0xFF, 0x80         ; 73A2 test byte [bx+0xffff],0x80
db 0x78, 0x41                           ; 73A7 js 0x73ea
db 0xB9, 0x02, 0x00                     ; 73A9 mov cx,0x2
db 0xFC                                 ; 73AC cld
db 0x8B, 0xF3                           ; 73AD mov si,bx
db 0xBF, 0xA3, 0x04                     ; 73AF mov di,0x4a3
db 0xF3, 0xA5                           ; 73B2 rep movsw
db 0x5B                                 ; 73B4 pop bx
db 0x56                                 ; 73B5 push si
db 0x53                                 ; 73B6 push bx
db 0xF6, 0x06, 0x55, 0x04, 0xFF         ; 73B7 test byte [0x455],0xff
db 0x75, 0x0F                           ; 73BC jnz 0x73cd
db 0xBE, 0x56, 0x04                     ; 73BE mov si,0x456
db 0x83, 0xEF, 0x04                     ; 73C1 sub di,byte +0x4
db 0xB9, 0x02, 0x00                     ; 73C4 mov cx,0x2
db 0xF3, 0xA5                           ; 73C7 rep movsw
db 0x32, 0xC0                           ; 73C9 xor al,al
db 0x74, 0x03                           ; 73CB jz 0x73d0
db 0xE8, 0x4B, 0xF0                     ; 73CD call 0x641b
db 0x5F                                 ; 73D0 pop di
db 0xBE, 0xA3, 0x04                     ; 73D1 mov si,0x4a3
db 0xB9, 0x02, 0x00                     ; 73D4 mov cx,0x2
db 0xFC                                 ; 73D7 cld
db 0xF3, 0xA5                           ; 73D8 rep movsw
db 0x5E                                 ; 73DA pop si
db 0x8B, 0x14                           ; 73DB mov dx,[si]
db 0x8B, 0x8C, 0x02, 0x00               ; 73DD mov cx,[si+0x2]
db 0x83, 0xC6, 0x04                     ; 73E1 add si,byte +0x4
db 0x56                                 ; 73E4 push si
db 0xE8, 0x49, 0xF0                     ; 73E5 call 0x6431
db 0xEB, 0x27                           ; 73E8 jmp short 0x7411
db 0x83, 0xC3, 0x04                     ; 73EA add bx,byte +0x4
db 0x8B, 0x0F                           ; 73ED mov cx,[bx]
db 0x43                                 ; 73EF inc bx
db 0x43                                 ; 73F0 inc bx
db 0x5E                                 ; 73F1 pop si
db 0x8B, 0x14                           ; 73F2 mov dx,[si]
db 0xF6, 0x06, 0x55, 0x04, 0xFF         ; 73F4 test byte [0x455],0xff
db 0x75, 0x06                           ; 73F9 jnz 0x7401
db 0x8B, 0x16, 0x56, 0x04               ; 73FB mov dx,[0x456]
db 0xEB, 0x04                           ; 73FF jmp short 0x7405
db 0x03, 0xD1                           ; 7401 add dx,cx
db 0x70, 0x35                           ; 7403 jo 0x743a
db 0x89, 0x14                           ; 7405 mov [si],dx
db 0x52                                 ; 7407 push dx
db 0x8B, 0x17                           ; 7408 mov dx,[bx]
db 0x43                                 ; 740A inc bx
db 0x43                                 ; 740B inc bx
db 0x58                                 ; 740C pop ax
db 0x53                                 ; 740D push bx
db 0xE8, 0xA5, 0xF1                     ; 740E call 0x65b6
db 0x5B                                 ; 7411 pop bx
db 0x59                                 ; 7412 pop cx
db 0x2A, 0xC5                           ; 7413 sub al,ch
db 0xE8, 0x1F, 0xF1                     ; 7415 call 0x6537
db 0x74, 0x0B                           ; 7418 jz 0x7425
db 0x89, 0x16, 0x2E, 0x00               ; 741A mov [0x2e],dx
db 0x8B, 0xD1                           ; 741E mov dx,cx
db 0x87, 0xD3                           ; 7420 xchg dx,bx
db 0xE9, 0x7C, 0x9A                     ; 7422 jmp 0xea1
db 0x8B, 0xE3                           ; 7425 mov sp,bx
db 0x89, 0x1E, 0x45, 0x03               ; 7427 mov [0x345],bx
db 0x8B, 0x1E, 0x3B, 0x03               ; 742B mov bx,[0x33b]
db 0x80, 0x3F, 0x2C                     ; 742F cmp byte [bx],0x2c
db 0x75, 0x09                           ; 7432 jnz 0x743d
db 0xE8, 0x55, 0xF6                     ; 7434 call 0x6a8c
db 0xE8, 0x42, 0xFF                     ; 7437 call 0x737c
db 0xE9, 0x93, 0x93                     ; 743A jmp 0x7d0
db 0xE9, 0xA8, 0x9A                     ; 743D jmp 0xee8
db 0x51                                 ; 7440 push cx
db 0x53                                 ; 7441 push bx
db 0x56                                 ; 7442 push si
db 0x57                                 ; 7443 push di
db 0x52                                 ; 7444 push dx
db 0xB2, 0x39                           ; 7445 mov dl,0x39
db 0xBB, 0x9E, 0x04                     ; 7447 mov bx,0x49e
db 0xBF, 0xA5, 0x04                     ; 744A mov di,0x4a5
db 0xBE, 0xA6, 0x04                     ; 744D mov si,0x4a6
db 0xEB, 0x19                           ; 7450 jmp short 0x746b
db 0x53                                 ; 7452 push bx
db 0xB9, 0x04, 0x00                     ; 7453 mov cx,0x4
db 0xF8                                 ; 7456 clc
db 0xD1, 0x17                           ; 7457 rcl word [bx],1
db 0x43                                 ; 7459 inc bx
db 0x43                                 ; 745A inc bx
db 0xE2, 0xFA                           ; 745B loop 0x7457
db 0x5B                                 ; 745D pop bx
db 0xF6, 0x07, 0x40                     ; 745E test byte [bx],0x40
db 0x75, 0x29                           ; 7461 jnz 0x748c
db 0xFE, 0x0C                           ; 7463 dec byte [si]
db 0x74, 0x2A                           ; 7465 jz 0x7491
db 0xFE, 0xCA                           ; 7467 dec dl
db 0x74, 0x26                           ; 7469 jz 0x7491
db 0xF6, 0x05, 0xFF                     ; 746B test byte [di],0xff
db 0x78, 0x21                           ; 746E js 0x7491
db 0x75, 0xE0                           ; 7470 jnz 0x7452
db 0x80, 0x2C, 0x08                     ; 7472 sub byte [si],0x8
db 0x76, 0x1A                           ; 7475 jna 0x7491
db 0x80, 0xEA, 0x08                     ; 7477 sub dl,0x8
db 0x76, 0x15                           ; 747A jna 0x7491
db 0xBE, 0xA4, 0x04                     ; 747C mov si,0x4a4
db 0xB9, 0x07, 0x00                     ; 747F mov cx,0x7
db 0xFD                                 ; 7482 std
db 0xF3, 0xA4                           ; 7483 rep movsb
db 0x80, 0x26, 0x9E, 0x04, 0x20         ; 7485 and byte [0x49e],0x20
db 0xEB, 0xBE                           ; 748A jmp short 0x744a
db 0x80, 0x0F, 0x20                     ; 748C or byte [bx],0x20
db 0xEB, 0xD2                           ; 748F jmp short 0x7463
db 0x5A                                 ; 7491 pop dx
db 0x5F                                 ; 7492 pop di
db 0x5E                                 ; 7493 pop si
db 0x5B                                 ; 7494 pop bx
db 0x59                                 ; 7495 pop cx
db 0x76, 0x03                           ; 7496 jna 0x749b
db 0xE9, 0x74, 0x04                     ; 7498 jmp 0x790f
db 0xE9, 0xC0, 0x06                     ; 749B jmp 0x7b5e

L_749E:
db 0x8A, 0x3E, 0xA6, 0x04               ; 749E mov bh,[0x4a6]
db 0xB9, 0x04, 0x00                     ; 74A2 mov cx,0x4
db 0x0A, 0xDB                           ; 74A5 or bl,bl
db 0x78, 0x21                           ; 74A7 js 0x74ca
db 0x75, 0x11                           ; 74A9 jnz 0x74bc
db 0x80, 0xEF, 0x08                     ; 74AB sub bh,0x8
db 0x72, 0x17                           ; 74AE jc 0x74c7
db 0x8A, 0xDE                           ; 74B0 mov bl,dh
db 0x8A, 0xF2                           ; 74B2 mov dh,dl
db 0x8A, 0xD4                           ; 74B4 mov dl,ah
db 0x32, 0xE4                           ; 74B6 xor ah,ah
db 0xE2, 0xEB                           ; 74B8 loop 0x74a5
db 0x74, 0x0B                           ; 74BA jz 0x74c7
db 0xF8                                 ; 74BC clc
db 0xD0, 0xD4                           ; 74BD rcl ah,1
db 0xD1, 0xD2                           ; 74BF rcl dx,1
db 0xD0, 0xD3                           ; 74C1 rcl bl,1
db 0xFE, 0xCF                           ; 74C3 dec bh
db 0x75, 0xDE                           ; 74C5 jnz 0x74a5
db 0xE9, 0xA1, 0x06                     ; 74C7 jmp 0x7b6b

db 0x88, 0x3E, 0xA6, 0x04               ; 74CA mov [0x4a6],bh
db 0xE9, 0x83, 0x04                     ; 74CE jmp 0x7954
db 0xCC                                 ; 74D1 int3
db 0x20, 0xEB                           ; 74D2 and bl,ch
db 0xF4                                 ; 74D4 hlt
db 0x88, 0x3E, 0xA6, 0x04               ; 74D5 mov [0x4a6],bh
db 0xE9, 0x78, 0x04                     ; 74D9 jmp 0x7954

L_74DC:
  push    bx
  call    L_74E2
  pop     bx
  ret

L_74E2:
db 0xE8, 0x2D, 0x00                     ; 74E2 call 0x7512
  mov     bx, err_overflow              ; 040A
  jmp     short L_74F6
db 0x53                                 ; 74EA push bx
db 0xE8, 0x02, 0x00                     ; 74EB call 0x74f0
db 0x5B                                 ; 74EE pop bx
db 0xC3                                 ; 74EF ret
db 0xE8, 0x1F, 0x00                     ; 74F0 call 0x7512
db 0xBB, 0x63, 0x04                     ; 74F3 mov bx,0x463

L_74F6:
  cmp     byte [0x04a8], 1
  js      L_7504
  jne     L_7511

  mov     byte [0x04a8], 2

L_7504:
db 0xE8, 0x81, 0xAF                     ; 7504 call 0x2488
db 0xB0, 0x0D                           ; 7507 mov al,0xd
db 0xE8, 0x89, 0xAF                     ; 7509 call 0x2495
db 0xB0, 0x0A                           ; 750C mov al,0xa
db 0xE8, 0x84, 0xAF                     ; 750E call 0x2495
L_7511:
  ret

db 0xFC                                 ; 7512 cld
db 0x0A, 0xFF                           ; 7513 or bh,bh
db 0xBE, 0x03, 0x62                     ; 7515 mov si,0x6203
db 0x74, 0x0A                           ; 7518 jz 0x7524
db 0xF6, 0x06, 0xA7, 0x04, 0x80         ; 751A test byte [0x4a7],0x80
db 0x79, 0x03                           ; 751F jns 0x7524
db 0xBE, 0x0B, 0x62                     ; 7521 mov si,0x620b
db 0xE8, 0x7B, 0x06                     ; 7524 call 0x7ba2
db 0x72, 0x08                           ; 7527 jc 0x7531
db 0xBF, 0x9F, 0x04                     ; 7529 mov di,0x49f
db 0xB9, 0x04, 0x00                     ; 752C mov cx,0x4
db 0xEB, 0x09                           ; 752F jmp short 0x753a
db 0x83, 0xC6, 0x04                     ; 7531 add si,byte +0x4
db 0xBF, 0xA3, 0x04                     ; 7534 mov di,0x4a3
db 0xB9, 0x02, 0x00                     ; 7537 mov cx,0x2
db 0x2E, 0xA5                           ; 753A cs movsw
db 0xE2, 0xFC                           ; 753C loop 0x753a
db 0xC3                                 ; 753E ret
db 0xE8, 0x0D, 0x09                     ; 753F call 0x7e4f
db 0x53                                 ; 7542 push bx
db 0xE8, 0x81, 0x07                     ; 7543 call 0x7cc7
db 0xE8, 0xB6, 0xF7                     ; 7546 call 0x6cff
db 0x5B                                 ; 7549 pop bx
db 0xE8, 0x05, 0x00                     ; 754A call 0x7552
db 0x5A                                 ; 754D pop dx
db 0x5B                                 ; 754E pop bx
db 0xE9, 0xAD, 0xF7                     ; 754F jmp 0x6cff
db 0x2E, 0x8A, 0x07                     ; 7552 mov al,[cs:bx]
db 0x98                                 ; 7555 cbw
db 0xE8, 0xF6, 0x08                     ; 7556 call 0x7e4f
db 0x50                                 ; 7559 push ax
db 0x43                                 ; 755A inc bx
db 0x2E, 0x8B, 0x07                     ; 755B mov ax,[cs:bx]
db 0xA3, 0xA3, 0x04                     ; 755E mov [0x4a3],ax
db 0x83, 0xC3, 0x02                     ; 7561 add bx,byte +0x2
db 0x2E, 0x8B, 0x07                     ; 7564 mov ax,[cs:bx]
db 0xA3, 0xA5, 0x04                     ; 7567 mov [0x4a5],ax
db 0x83, 0xC3, 0x02                     ; 756A add bx,byte +0x2
db 0x58                                 ; 756D pop ax
db 0x5A                                 ; 756E pop dx
db 0x59                                 ; 756F pop cx
db 0x48                                 ; 7570 dec ax
db 0x74, 0x1C                           ; 7571 jz 0x758f
db 0x51                                 ; 7573 push cx
db 0x52                                 ; 7574 push dx
db 0x50                                 ; 7575 push ax
db 0x53                                 ; 7576 push bx
db 0x87, 0xD9                           ; 7577 xchg bx,cx
db 0xE8, 0x83, 0xF7                     ; 7579 call 0x6cff
db 0x5B                                 ; 757C pop bx
db 0x53                                 ; 757D push bx
db 0x2E, 0x8B, 0x17                     ; 757E mov dx,[cs:bx]
db 0x2E, 0x8B, 0x9F, 0x02, 0x00         ; 7581 mov bx,[cs:bx+0x2]
db 0xE8, 0xEA, 0xF1                     ; 7586 call 0x6773
db 0x5B                                 ; 7589 pop bx
db 0x83, 0xC3, 0x04                     ; 758A add bx,byte +0x4
db 0xEB, 0xDE                           ; 758D jmp short 0x756d
db 0xC3                                 ; 758F ret
db 0x53                                 ; 7590 push bx
db 0xD0, 0xE8                           ; 7591 shr al,1
db 0x73, 0x03                           ; 7593 jnc 0x7598
db 0xE9, 0x09, 0x01                     ; 7595 jmp 0x76a1
db 0xBB, 0xB2, 0x60                     ; 7598 mov bx,0x60b2
db 0xE8, 0xD6, 0x06                     ; 759B call 0x7c74
db 0xE8, 0x2F, 0x07                     ; 759E call 0x7cd0
db 0x72, 0x09                           ; 75A1 jc 0x75ac
db 0x5B                                 ; 75A3 pop bx
db 0xE8, 0x23, 0xFB                     ; 75A4 call 0x70ca
db 0x4B                                 ; 75A7 dec bx
db 0xC6, 0x07, 0x25                     ; 75A8 mov byte [bx],0x25
db 0xC3                                 ; 75AB ret
db 0xE8, 0xF3, 0x05                     ; 75AC call 0x7ba2
db 0xB5, 0x10                           ; 75AF mov ch,0x10
db 0x73, 0x02                           ; 75B1 jnc 0x75b5
db 0xB5, 0x07                           ; 75B3 mov ch,0x7
db 0xE8, 0xBD, 0x05                     ; 75B5 call 0x7b75
db 0x74, 0x03                           ; 75B8 jz 0x75bd
db 0xE8, 0x04, 0xFA                     ; 75BA call 0x6fc1
db 0x5B                                 ; 75BD pop bx
db 0x78, 0x3F                           ; 75BE js 0x75ff
db 0x8A, 0xD0                           ; 75C0 mov dl,al
db 0x02, 0xC5                           ; 75C2 add al,ch
db 0x2A, 0x06, 0x82, 0x04               ; 75C4 sub al,[0x482]
db 0x79, 0x05                           ; 75C8 jns 0x75cf
db 0xF6, 0xD8                           ; 75CA neg al
db 0xE8, 0xC2, 0xFA                     ; 75CC call 0x7091
db 0x32, 0xC9                           ; 75CF xor cl,cl
db 0xE8, 0xB1, 0x00                     ; 75D1 call 0x7685
db 0xFF, 0x36, 0x81, 0x04               ; 75D4 push word [0x481]
db 0x52                                 ; 75D8 push dx
db 0xE8, 0xDA, 0xF8                     ; 75D9 call 0x6eb6
db 0x5A                                 ; 75DC pop dx
db 0x8F, 0x06, 0x81, 0x04               ; 75DD pop word [0x481]
db 0xFF, 0x36, 0x81, 0x04               ; 75E1 push word [0x481]
db 0x32, 0xC0                           ; 75E5 xor al,al
db 0x0A, 0xC2                           ; 75E7 or al,dl
db 0x74, 0x06                           ; 75E9 jz 0x75f1
db 0xE8, 0xB3, 0xFA                     ; 75EB call 0x70a1
db 0xE8, 0x39, 0xF8                     ; 75EE call 0x6e2a
db 0x8F, 0x06, 0x81, 0x04               ; 75F1 pop word [0x481]
db 0xFF, 0x36, 0x81, 0x04               ; 75F5 push word [0x481]
db 0xA0, 0x81, 0x04                     ; 75F9 mov al,[0x481]
db 0xE9, 0x72, 0x02                     ; 75FC jmp 0x7871
db 0x8A, 0xD0                           ; 75FF mov dl,al
db 0xA0, 0x81, 0x04                     ; 7601 mov al,[0x481]
db 0x0A, 0xC0                           ; 7604 or al,al
db 0x74, 0x02                           ; 7606 jz 0x760a
db 0xFE, 0xC8                           ; 7608 dec al
db 0x8A, 0xF0                           ; 760A mov dh,al
db 0x02, 0xC2                           ; 760C add al,dl
db 0x8A, 0xC8                           ; 760E mov cl,al
db 0x78, 0x04                           ; 7610 js 0x7616
db 0x32, 0xC0                           ; 7612 xor al,al
db 0x8A, 0xC8                           ; 7614 mov cl,al
db 0x79, 0x11                           ; 7616 jns 0x7629
db 0x50                                 ; 7618 push ax
db 0x51                                 ; 7619 push cx
db 0x52                                 ; 761A push dx
db 0x53                                 ; 761B push bx
db 0xE8, 0xA9, 0x05                     ; 761C call 0x7bc8
db 0x5B                                 ; 761F pop bx
db 0x5A                                 ; 7620 pop dx
db 0x59                                 ; 7621 pop cx
db 0x58                                 ; 7622 pop ax
db 0xFE, 0xC0                           ; 7623 inc al
db 0x78, 0xF1                           ; 7625 js 0x7618
db 0x8A, 0xE1                           ; 7627 mov ah,cl
db 0x8A, 0xC2                           ; 7629 mov al,dl
db 0x2A, 0xC1                           ; 762B sub al,cl
db 0x02, 0xC5                           ; 762D add al,ch
db 0x79, 0x17                           ; 762F jns 0x7648
db 0xA0, 0x82, 0x04                     ; 7631 mov al,[0x482]
db 0xE8, 0x5A, 0xFA                     ; 7634 call 0x7091
db 0xC6, 0x07, 0x2E                     ; 7637 mov byte [bx],0x2e
db 0x89, 0x1E, 0x52, 0x03               ; 763A mov [0x352],bx
db 0x43                                 ; 763E inc bx
db 0x32, 0xC9                           ; 763F xor cl,cl
db 0x8A, 0xC6                           ; 7641 mov al,dh
db 0x2A, 0xC5                           ; 7643 sub al,ch
db 0xE9, 0xA1, 0x09                     ; 7645 jmp 0x7fe9
db 0xA0, 0x82, 0x04                     ; 7648 mov al,[0x482]
db 0x52                                 ; 764B push dx
db 0xFF, 0x36, 0x81, 0x04               ; 764C push word [0x481]
db 0x2A, 0xC5                           ; 7650 sub al,ch
db 0x2A, 0xC2                           ; 7652 sub al,dl
db 0x02, 0xC1                           ; 7654 add al,cl
db 0x78, 0x03                           ; 7656 js 0x765b
db 0xE8, 0x36, 0xFA                     ; 7658 call 0x7091
db 0xE8, 0x27, 0x00                     ; 765B call 0x7685
db 0xFF, 0x36, 0x81, 0x04               ; 765E push word [0x481]
db 0xE8, 0x51, 0xF8                     ; 7662 call 0x6eb6
db 0xA0, 0x82, 0x04                     ; 7665 mov al,[0x482]
db 0x8F, 0x06, 0x81, 0x04               ; 7668 pop word [0x481]
db 0x0A, 0xC0                           ; 766C or al,al
db 0x58                                 ; 766E pop ax
db 0x5A                                 ; 766F pop dx
db 0x75, 0x07                           ; 7670 jnz 0x7679
db 0x8B, 0x1E, 0x52, 0x03               ; 7672 mov bx,[0x352]
db 0xE9, 0x67, 0x01                     ; 7676 jmp 0x77e0
db 0x02, 0xC2                           ; 7679 add al,dl
db 0xFE, 0xC8                           ; 767B dec al
db 0x78, 0x03                           ; 767D js 0x7682
db 0xE8, 0x0F, 0xFA                     ; 767F call 0x7091
db 0xE9, 0x5B, 0x01                     ; 7682 jmp 0x77e0
db 0x8A, 0xC5                           ; 7685 mov al,ch
db 0x02, 0xC2                           ; 7687 add al,dl
db 0x2A, 0xC1                           ; 7689 sub al,cl
db 0xFE, 0xC0                           ; 768B inc al
db 0x8A, 0xE8                           ; 768D mov ch,al
db 0x2C, 0x03                           ; 768F sub al,0x3
db 0x7F, 0xFC                           ; 7691 jg 0x768f
db 0x04, 0x03                           ; 7693 add al,0x3
db 0x8A, 0xC8                           ; 7695 mov cl,al
db 0xA0, 0x83, 0x04                     ; 7697 mov al,[0x483]
db 0x24, 0x40                           ; 769A and al,0x40
db 0x75, 0x02                           ; 769C jnz 0x76a0
db 0x8A, 0xC8                           ; 769E mov cl,al
db 0xC3                                 ; 76A0 ret
db 0xE8, 0xFE, 0x04                     ; 76A1 call 0x7ba2
db 0xB4, 0x07                           ; 76A4 mov ah,0x7
db 0x72, 0x02                           ; 76A6 jc 0x76aa
db 0xB4, 0x10                           ; 76A8 mov ah,0x10
db 0xE8, 0xC8, 0x04                     ; 76AA call 0x7b75
db 0x5B                                 ; 76AD pop bx
db 0xF9                                 ; 76AE stc
db 0x74, 0x09                           ; 76AF jz 0x76ba
db 0x53                                 ; 76B1 push bx
db 0x50                                 ; 76B2 push ax
db 0xE8, 0x0B, 0xF9                     ; 76B3 call 0x6fc1
db 0x5A                                 ; 76B6 pop dx
db 0x5B                                 ; 76B7 pop bx
db 0x8A, 0xE6                           ; 76B8 mov ah,dh
db 0x9C                                 ; 76BA pushf
db 0x50                                 ; 76BB push ax
db 0x8B, 0x16, 0x81, 0x04               ; 76BC mov dx,[0x481]
db 0x0A, 0xF6                           ; 76C0 or dh,dh
db 0x9C                                 ; 76C2 pushf
db 0x0A, 0xD2                           ; 76C3 or dl,dl
db 0x74, 0x02                           ; 76C5 jz 0x76c9
db 0xFE, 0xCA                           ; 76C7 dec dl
db 0x02, 0xF2                           ; 76C9 add dh,dl
db 0x9D                                 ; 76CB popf
db 0x74, 0x09                           ; 76CC jz 0x76d7
db 0xF6, 0x06, 0x83, 0x04, 0x04         ; 76CE test byte [0x483],0x4
db 0x75, 0x02                           ; 76D3 jnz 0x76d7
db 0xFE, 0xCE                           ; 76D5 dec dh
db 0x2A, 0xF4                           ; 76D7 sub dh,ah
db 0x8A, 0xE6                           ; 76D9 mov ah,dh
db 0x50                                 ; 76DB push ax
db 0x78, 0x03                           ; 76DC js 0x76e1
db 0xE9, 0x4E, 0x00                     ; 76DE jmp 0x772f
db 0x53                                 ; 76E1 push bx
db 0x50                                 ; 76E2 push ax
db 0x50                                 ; 76E3 push ax
db 0xE8, 0xE1, 0x04                     ; 76E4 call 0x7bc8
db 0x58                                 ; 76E7 pop ax
db 0xFE, 0xC4                           ; 76E8 inc ah
db 0x75, 0xF7                           ; 76EA jnz 0x76e3
db 0xE8, 0xBF, 0x04                     ; 76EC call 0x7bae
db 0xE8, 0x8E, 0x07                     ; 76EF call 0x7e80
db 0x58                                 ; 76F2 pop ax
db 0x50                                 ; 76F3 push ax
db 0xB9, 0x03, 0x00                     ; 76F4 mov cx,0x3
db 0xD2, 0xE4                           ; 76F7 shl ah,cl
db 0xE8, 0xA6, 0x04                     ; 76F9 call 0x7ba2
db 0x72, 0x10                           ; 76FC jc 0x770e
db 0x8A, 0xC4                           ; 76FE mov al,ah
db 0x98                                 ; 7700 cbw
db 0xBB, 0xB2, 0x60                     ; 7701 mov bx,0x60b2
db 0x03, 0xD8                           ; 7704 add bx,ax
db 0xE8, 0x6B, 0x05                     ; 7706 call 0x7c74
db 0xE8, 0x55, 0x06                     ; 7709 call 0x7d61
db 0xEB, 0x0E                           ; 770C jmp short 0x771c
db 0xBB, 0x6E, 0x60                     ; 770E mov bx,0x606e
db 0x8A, 0xC4                           ; 7711 mov al,ah
db 0x98                                 ; 7713 cbw
db 0x03, 0xD8                           ; 7714 add bx,ax
db 0xE8, 0x53, 0x05                     ; 7716 call 0x7c6c
db 0xE8, 0xF8, 0x05                     ; 7719 call 0x7d14
db 0x58                                 ; 771C pop ax
db 0x5B                                 ; 771D pop bx
db 0x78, 0x11                           ; 771E js 0x7731
db 0x58                                 ; 7720 pop ax
db 0x59                                 ; 7721 pop cx
db 0xFE, 0xC1                           ; 7722 inc cl
db 0x51                                 ; 7724 push cx
db 0x50                                 ; 7725 push ax
db 0x53                                 ; 7726 push bx
db 0x50                                 ; 7727 push ax
db 0xE8, 0x9D, 0x04                     ; 7728 call 0x7bc8
db 0x58                                 ; 772B pop ax
db 0x5B                                 ; 772C pop bx
db 0xEB, 0x02                           ; 772D jmp short 0x7731
db 0x32, 0xE4                           ; 772F xor ah,ah
db 0xF6, 0xDC                           ; 7731 neg ah
db 0xA0, 0x82, 0x04                     ; 7733 mov al,[0x482]
db 0x02, 0xE0                           ; 7736 add ah,al
db 0xFE, 0xC4                           ; 7738 inc ah
db 0x0A, 0xC0                           ; 773A or al,al
db 0x74, 0x09                           ; 773C jz 0x7747
db 0xF6, 0x06, 0x83, 0x04, 0x04         ; 773E test byte [0x483],0x4
db 0x75, 0x02                           ; 7743 jnz 0x7747
db 0xFE, 0xCC                           ; 7745 dec ah
db 0x8A, 0xEC                           ; 7747 mov ch,ah
db 0x32, 0xC9                           ; 7749 xor cl,cl
db 0x58                                 ; 774B pop ax
db 0xFF, 0x36, 0x81, 0x04               ; 774C push word [0x481]
db 0x50                                 ; 7750 push ax
db 0x88, 0x2E, 0x82, 0x04               ; 7751 mov [0x482],ch
db 0xE8, 0x5E, 0xF7                     ; 7755 call 0x6eb6
db 0x58                                 ; 7758 pop ax
db 0x0A, 0xE4                           ; 7759 or ah,ah
db 0x7E, 0x05                           ; 775B jng 0x7762
db 0x8A, 0xC4                           ; 775D mov al,ah
db 0xE8, 0x3F, 0xF9                     ; 775F call 0x70a1
db 0x58                                 ; 7762 pop ax
db 0xA3, 0x81, 0x04                     ; 7763 mov [0x481],ax
db 0x0A, 0xC0                           ; 7766 or al,al
db 0x75, 0x0C                           ; 7768 jnz 0x7776
db 0x4B                                 ; 776A dec bx
db 0x8A, 0x07                           ; 776B mov al,[bx]
db 0x3C, 0x2E                           ; 776D cmp al,0x2e
db 0x74, 0x01                           ; 776F jz 0x7772
db 0x43                                 ; 7771 inc bx
db 0x89, 0x1E, 0x52, 0x03               ; 7772 mov [0x352],bx
db 0x58                                 ; 7776 pop ax
db 0x9D                                 ; 7777 popf
db 0x72, 0x15                           ; 7778 jc 0x778f
db 0x02, 0xC4                           ; 777A add al,ah
db 0x8A, 0x26, 0x82, 0x04               ; 777C mov ah,[0x482]
db 0x2A, 0xC4                           ; 7780 sub al,ah
db 0x0A, 0xE4                           ; 7782 or ah,ah
db 0x74, 0x09                           ; 7784 jz 0x778f
db 0xF6, 0x06, 0x83, 0x04, 0x04         ; 7786 test byte [0x483],0x4
db 0x75, 0x02                           ; 778B jnz 0x778f
db 0xFE, 0xC0                           ; 778D inc al
db 0x0A, 0xC0                           ; 778F or al,al
db 0xE8, 0x4A, 0xF6                     ; 7791 call 0x6dde
db 0x8B, 0xD9                           ; 7794 mov bx,cx
db 0xE9, 0x47, 0x00                     ; 7796 jmp 0x77e0
db 0x8A, 0xE0                           ; 7799 mov ah,al
db 0xF6, 0xC4, 0x40                     ; 779B test ah,0x40
db 0xB4, 0x03                           ; 779E mov ah,0x3
db 0x75, 0x02                           ; 77A0 jnz 0x77a4
db 0x32, 0xE4                           ; 77A2 xor ah,ah
db 0xA3, 0x83, 0x04                     ; 77A4 mov [0x483],ax
db 0x89, 0x0E, 0x81, 0x04               ; 77A7 mov [0x481],cx
db 0x8A, 0xE0                           ; 77AB mov ah,al
db 0xBB, 0xB4, 0x04                     ; 77AD mov bx,0x4b4
db 0xC6, 0x07, 0x20                     ; 77B0 mov byte [bx],0x20
db 0xF6, 0xC4, 0x08                     ; 77B3 test ah,0x8
db 0x74, 0x03                           ; 77B6 jz 0x77bb
db 0xC6, 0x07, 0x2B                     ; 77B8 mov byte [bx],0x2b
db 0x53                                 ; 77BB push bx
db 0xE8, 0xB6, 0x03                     ; 77BC call 0x7b75
db 0x5B                                 ; 77BF pop bx
db 0x79, 0x08                           ; 77C0 jns 0x77ca
db 0xC6, 0x07, 0x2D                     ; 77C2 mov byte [bx],0x2d
db 0x53                                 ; 77C5 push bx
db 0xE8, 0xE2, 0x05                     ; 77C6 call 0x7dab
db 0x5B                                 ; 77C9 pop bx
db 0x43                                 ; 77CA inc bx
db 0xC6, 0x07, 0x30                     ; 77CB mov byte [bx],0x30
db 0xE8, 0xD1, 0x03                     ; 77CE call 0x7ba2
db 0xA1, 0x83, 0x04                     ; 77D1 mov ax,[0x483]
db 0x8B, 0x0E, 0x81, 0x04               ; 77D4 mov cx,[0x481]
db 0x78, 0x03                           ; 77D8 js 0x77dd
db 0xE9, 0xB3, 0xFD                     ; 77DA jmp 0x7590
db 0xE9, 0x68, 0x00                     ; 77DD jmp 0x7848
db 0x53                                 ; 77E0 push bx
db 0xE8, 0x3B, 0xF8                     ; 77E1 call 0x701f
db 0x5B                                 ; 77E4 pop bx
db 0x74, 0x03                           ; 77E5 jz 0x77ea
db 0x88, 0x2F                           ; 77E7 mov [bx],ch
db 0x43                                 ; 77E9 inc bx
db 0xC6, 0x07, 0x00                     ; 77EA mov byte [bx],0x0
db 0xBB, 0xB3, 0x04                     ; 77ED mov bx,0x4b3
db 0x43                                 ; 77F0 inc bx
db 0x8B, 0x3E, 0x52, 0x03               ; 77F1 mov di,[0x352]
db 0x8B, 0x16, 0x81, 0x04               ; 77F5 mov dx,[0x481]
db 0xA0, 0x82, 0x04                     ; 77F9 mov al,[0x482]
db 0x32, 0xE4                           ; 77FC xor ah,ah
db 0x2B, 0xFB                           ; 77FE sub di,bx
db 0x2B, 0xF8                           ; 7800 sub di,ax
db 0x74, 0x43                           ; 7802 jz 0x7847
db 0x8A, 0x07                           ; 7804 mov al,[bx]
db 0x3C, 0x20                           ; 7806 cmp al,0x20
db 0x74, 0xE6                           ; 7808 jz 0x77f0
db 0x3C, 0x2A                           ; 780A cmp al,0x2a
db 0x74, 0xE2                           ; 780C jz 0x77f0
db 0xB4, 0x01                           ; 780E mov ah,0x1
db 0x4B                                 ; 7810 dec bx
db 0x53                                 ; 7811 push bx
db 0x50                                 ; 7812 push ax
db 0xE8, 0xEA, 0xEA                     ; 7813 call 0x6300
db 0x32, 0xE4                           ; 7816 xor ah,ah
db 0x3C, 0x2D                           ; 7818 cmp al,0x2d
db 0x74, 0xF6                           ; 781A jz 0x7812
db 0x3C, 0x2B                           ; 781C cmp al,0x2b
db 0x74, 0xF2                           ; 781E jz 0x7812
db 0x3C, 0x24                           ; 7820 cmp al,0x24
db 0x74, 0xEE                           ; 7822 jz 0x7812
db 0x3C, 0x30                           ; 7824 cmp al,0x30
db 0x75, 0x16                           ; 7826 jnz 0x783e
db 0x43                                 ; 7828 inc bx
db 0xE8, 0xD4, 0xEA                     ; 7829 call 0x6300
db 0x73, 0x10                           ; 782C jnc 0x783e
db 0x4B                                 ; 782E dec bx
db 0xEB, 0x03                           ; 782F jmp short 0x7834
db 0x4B                                 ; 7831 dec bx
db 0x88, 0x07                           ; 7832 mov [bx],al
db 0x58                                 ; 7834 pop ax
db 0x0A, 0xE4                           ; 7835 or ah,ah
db 0x74, 0xF8                           ; 7837 jz 0x7831
db 0x83, 0xC4, 0x02                     ; 7839 add sp,byte +0x2
db 0xEB, 0xB3                           ; 783C jmp short 0x77f1
db 0x58                                 ; 783E pop ax
db 0x0A, 0xE4                           ; 783F or ah,ah
db 0x74, 0xFB                           ; 7841 jz 0x783e
db 0x5B                                 ; 7843 pop bx
db 0xC6, 0x07, 0x25                     ; 7844 mov byte [bx],0x25
db 0xC3                                 ; 7847 ret
db 0xA1, 0x83, 0x04                     ; 7848 mov ax,[0x483]
db 0x8A, 0xCC                           ; 784B mov cl,ah
db 0xB5, 0x06                           ; 784D mov ch,0x6
db 0xD0, 0xE8                           ; 784F shr al,1
db 0x8B, 0x16, 0x81, 0x04               ; 7851 mov dx,[0x481]
db 0x73, 0x0B                           ; 7855 jnc 0x7862
db 0x53                                 ; 7857 push bx
db 0x52                                 ; 7858 push dx
db 0xE8, 0x45, 0xF3                     ; 7859 call 0x6ba1
db 0x32, 0xC0                           ; 785C xor al,al
db 0x5A                                 ; 785E pop dx
db 0xE9, 0x3F, 0xFE                     ; 785F jmp 0x76a1
db 0x8A, 0xC6                           ; 7862 mov al,dh
db 0x2C, 0x05                           ; 7864 sub al,0x5
db 0x78, 0x03                           ; 7866 js 0x786b
db 0xE8, 0x26, 0xF8                     ; 7868 call 0x7091
db 0x52                                 ; 786B push dx
db 0xE8, 0xDA, 0xF5                     ; 786C call 0x6e49
db 0x58                                 ; 786F pop ax
db 0x50                                 ; 7870 push ax
db 0x0A, 0xC0                           ; 7871 or al,al
db 0x75, 0x01                           ; 7873 jnz 0x7876
db 0x4B                                 ; 7875 dec bx
db 0xFE, 0xC8                           ; 7876 dec al
db 0x78, 0x06                           ; 7878 js 0x7880
db 0xE8, 0x14, 0xF8                     ; 787A call 0x7091
db 0xC6, 0x07, 0x00                     ; 787D mov byte [bx],0x0
db 0x8F, 0x06, 0x81, 0x04               ; 7880 pop word [0x481]
db 0xE9, 0x59, 0xFF                     ; 7884 jmp 0x77e0
db 0xE8, 0xEB, 0x02                     ; 7887 call 0x7b75
db 0x74, 0x6D                           ; 788A jz 0x78f9
db 0x79, 0x0C                           ; 788C jns 0x789a
db 0xA1, 0xA3, 0x04                     ; 788E mov ax,[0x4a3]
db 0xA3, 0x0B, 0x00                     ; 7891 mov [0xb],ax
db 0xA0, 0xA5, 0x04                     ; 7894 mov al,[0x4a5]
db 0xA2, 0x0D, 0x00                     ; 7897 mov [0xd],al
db 0xA1, 0x0B, 0x00                     ; 789A mov ax,[0xb]
db 0x2E, 0xF7, 0x26, 0x6B, 0x62         ; 789D mul word [cs:0x626b]
db 0x8B, 0xF8                           ; 78A2 mov di,ax
db 0x8A, 0xCA                           ; 78A4 mov cl,dl
db 0x2E, 0xA0, 0x6D, 0x62               ; 78A6 mov al,[cs:0x626d]
db 0xF6, 0x26, 0x0B, 0x00               ; 78AA mul byte [0xb]
db 0x02, 0xC8                           ; 78AE add cl,al
db 0x2E, 0xA0, 0x0D, 0x00               ; 78B0 mov al,[cs:0xd]
db 0x2E, 0xF6, 0x26, 0x6B, 0x62         ; 78B4 mul byte [cs:0x626b]
db 0x02, 0xC8                           ; 78B9 add cl,al
db 0x32, 0xC0                           ; 78BB xor al,al
db 0x2E, 0x8B, 0x16, 0x6E, 0x62         ; 78BD mov dx,[cs:0x626e]
db 0x03, 0xD7                           ; 78C2 add dx,di
db 0x2E, 0x8A, 0x1E, 0x70, 0x62         ; 78C4 mov bl,[cs:0x6270]
db 0x12, 0xD9                           ; 78C9 adc bl,cl
db 0xA2, 0xA7, 0x04                     ; 78CB mov [0x4a7],al
db 0xB0, 0x80                           ; 78CE mov al,0x80
db 0xA2, 0xA6, 0x04                     ; 78D0 mov [0x4a6],al
db 0x89, 0x16, 0x0B, 0x00               ; 78D3 mov [0xb],dx
db 0x88, 0x1E, 0x0D, 0x00               ; 78D7 mov [0xd],bl
db 0xB0, 0x04                           ; 78DB mov al,0x4
db 0xA2, 0xFB, 0x02                     ; 78DD mov [0x2fb],al
db 0xE9, 0xBB, 0xFB                     ; 78E0 jmp 0x749e
db 0x00, 0x00                           ; 78E3 add [bx+si],al
db 0x00, 0xBB, 0xB3, 0x04               ; 78E5 add [bp+di+0x4b3],bh
db 0xB9, 0x20, 0x00                     ; 78E9 mov cx,0x20
db 0x03, 0x07                           ; 78EC add ax,[bx]
db 0x43                                 ; 78EE inc bx
db 0x43                                 ; 78EF inc bx
db 0xE2, 0xFA                           ; 78F0 loop 0x78ec
db 0x24, 0xFE                           ; 78F2 and al,0xfe
db 0xA3, 0x0B, 0x00                     ; 78F4 mov [0xb],ax
db 0xEB, 0xA1                           ; 78F7 jmp short 0x789a
db 0x8B, 0x16, 0x0B, 0x00               ; 78F9 mov dx,[0xb]
db 0x8A, 0x1E, 0x0D, 0x00               ; 78FD mov bl,[0xd]
db 0x33, 0xC0                           ; 7901 xor ax,ax
db 0xB0, 0x80                           ; 7903 mov al,0x80
db 0xA2, 0xA6, 0x04                     ; 7905 mov [0x4a6],al
db 0x88, 0x26, 0xA7, 0x04               ; 7908 mov [0x4a7],ah
db 0xE9, 0x8F, 0xFB                     ; 790C jmp 0x749e
db 0x53                                 ; 790F push bx
db 0x51                                 ; 7910 push cx
db 0xBB, 0x9E, 0x04                     ; 7911 mov bx,0x49e
db 0x81, 0x07, 0x80, 0x00               ; 7914 add word [bx],0x80
db 0xB9, 0x03, 0x00                     ; 7918 mov cx,0x3
db 0x73, 0x0E                           ; 791B jnc 0x792b
db 0x43                                 ; 791D inc bx
db 0x43                                 ; 791E inc bx
db 0xFF, 0x07                           ; 791F inc word [bx]
db 0x75, 0x08                           ; 7921 jnz 0x792b
db 0xE2, 0xF8                           ; 7923 loop 0x791d
db 0xFE, 0x06, 0xA6, 0x04               ; 7925 inc byte [0x4a6]
db 0xD1, 0x1F                           ; 7929 rcr word [bx],1
db 0x59                                 ; 792B pop cx
db 0x74, 0x20                           ; 792C jz 0x794e
db 0xF6, 0x06, 0x9E, 0x04, 0xFF         ; 792E test byte [0x49e],0xff
db 0x75, 0x05                           ; 7933 jnz 0x793a
db 0x80, 0x26, 0x9F, 0x04, 0xFE         ; 7935 and byte [0x49f],0xfe
db 0xBB, 0xA5, 0x04                     ; 793A mov bx,0x4a5
db 0x8A, 0x07                           ; 793D mov al,[bx]
db 0x8A, 0xA7, 0x02, 0x00               ; 793F mov ah,[bx+0x2]
db 0x24, 0x7F                           ; 7943 and al,0x7f
db 0x80, 0xE4, 0x80                     ; 7945 and ah,0x80
db 0x0A, 0xE0                           ; 7948 or ah,al
db 0x88, 0x27                           ; 794A mov [bx],ah
db 0x5B                                 ; 794C pop bx
db 0xC3                                 ; 794D ret
db 0x90                                 ; 794E nop
db 0x90                                 ; 794F nop
db 0x90                                 ; 7950 nop
db 0xE9, 0x88, 0xFB                     ; 7951 jmp 0x74dc
db 0x80, 0xE4, 0xE0                     ; 7954 and ah,0xe0
db 0x80, 0xC4, 0x80                     ; 7957 add ah,0x80
db 0x73, 0x1C                           ; 795A jnc 0x7978
db 0x9C                                 ; 795C pushf
db 0x42                                 ; 795D inc dx
db 0x75, 0x12                           ; 795E jnz 0x7972
db 0x9D                                 ; 7960 popf
db 0xFE, 0xC3                           ; 7961 inc bl
db 0x75, 0x13                           ; 7963 jnz 0x7978
db 0xF9                                 ; 7965 stc
db 0xD0, 0xDB                           ; 7966 rcr bl,1
db 0xFE, 0x06, 0xA6, 0x04               ; 7968 inc byte [0x4a6]
db 0x75, 0x0A                           ; 796C jnz 0x7978
db 0x90                                 ; 796E nop
db 0xE9, 0x6A, 0xFB                     ; 796F jmp 0x74dc
db 0x9D                                 ; 7972 popf
db 0x75, 0x03                           ; 7973 jnz 0x7978
db 0x80, 0xE2, 0xFE                     ; 7975 and dl,0xfe
db 0x56                                 ; 7978 push si
db 0xBE, 0xA3, 0x04                     ; 7979 mov si,0x4a3
db 0x89, 0x14                           ; 797C mov [si],dx
db 0x46                                 ; 797E inc si
db 0x46                                 ; 797F inc si
db 0x8A, 0x3E, 0xA7, 0x04               ; 7980 mov bh,[0x4a7]
db 0x81, 0xE3, 0x7F, 0x80               ; 7984 and bx,0x807f
db 0x0A, 0xDF                           ; 7988 or bl,bh
db 0x88, 0x1C                           ; 798A mov [si],bl
db 0x5E                                 ; 798C pop si
db 0xC3                                 ; 798D ret
db 0x8B, 0xF1                           ; 798E mov si,cx
db 0xE8, 0xB4, 0x04                     ; 7990 call 0x7e47
db 0x8B, 0xCE                           ; 7993 mov cx,si
db 0x51                                 ; 7995 push cx
db 0xE8, 0x09, 0x02                     ; 7996 call 0x7ba2
db 0x72, 0x09                           ; 7999 jc 0x79a4
db 0x80, 0x3E, 0xA6, 0x04, 0xB8         ; 799B cmp byte [0x4a6],0xb8
db 0x79, 0x0F                           ; 79A0 jns 0x79b1
db 0xEB, 0x07                           ; 79A2 jmp short 0x79ab
db 0x80, 0x3E, 0xA6, 0x04, 0x98         ; 79A4 cmp byte [0x4a6],0x98
db 0x79, 0x06                           ; 79A9 jns 0x79b1
db 0xE8, 0x00, 0x02                     ; 79AB call 0x7bae
db 0xE8, 0xCF, 0x04                     ; 79AE call 0x7e80
db 0xBB, 0x86, 0x04                     ; 79B1 mov bx,0x486
db 0xE8, 0x51, 0x04                     ; 79B4 call 0x7e08
db 0x59                                 ; 79B7 pop cx
db 0x51                                 ; 79B8 push cx
db 0xBF, 0x8E, 0x04                     ; 79B9 mov di,0x48e
db 0xBB, 0x86, 0x04                     ; 79BC mov bx,0x486
db 0xE8, 0x35, 0x04                     ; 79BF call 0x7df7
db 0xBB, 0x86, 0x04                     ; 79C2 mov bx,0x486
db 0xE8, 0x5D, 0x04                     ; 79C5 call 0x7e25
db 0xE8, 0xFD, 0x01                     ; 79C8 call 0x7bc8
db 0xE8, 0xB2, 0x04                     ; 79CB call 0x7e80
db 0xBB, 0x86, 0x04                     ; 79CE mov bx,0x486
db 0xE8, 0x34, 0x04                     ; 79D1 call 0x7e08
db 0xE8, 0xFB, 0x01                     ; 79D4 call 0x7bd2
db 0xBB, 0x94, 0x04                     ; 79D7 mov bx,0x494
db 0xE8, 0xC5, 0x01                     ; 79DA call 0x7ba2
db 0x73, 0x03                           ; 79DD jnc 0x79e2
db 0x83, 0xEB, 0x04                     ; 79DF sub bx,byte +0x4
db 0xE8, 0x57, 0x04                     ; 79E2 call 0x7e3c
db 0x59                                 ; 79E5 pop cx
db 0x75, 0x04                           ; 79E6 jnz 0x79ec
db 0xFE, 0xC1                           ; 79E8 inc cl
db 0xEB, 0xCC                           ; 79EA jmp short 0x79b8
db 0x8B, 0xE9                           ; 79EC mov bp,cx
db 0xE8, 0x75, 0x04                     ; 79EE call 0x7e66
db 0x8B, 0xCD                           ; 79F1 mov cx,bp
db 0xC3                                 ; 79F3 ret
db 0x80, 0x26, 0xA5, 0x04, 0x7F         ; 79F4 and byte [0x4a5],0x7f
db 0xE8, 0x86, 0x00                     ; 79F9 call 0x7a82
db 0xE8, 0xA5, 0x00                     ; 79FC call 0x7aa4
db 0xC6, 0x06, 0xB2, 0x04, 0x7F         ; 79FF mov byte [0x4b2],0x7f
db 0xE8, 0xA3, 0xEC                     ; 7A04 call 0x66aa
db 0xE8, 0x84, 0x00                     ; 7A07 call 0x7a8e
db 0xEB, 0x1B                           ; 7A0A jmp short 0x7a27
db 0x65, 0xED                           ; 7A0C gs in ax,dx
db 0xA1, 0xA5, 0x04                     ; 7A0E mov ax,[0x4a5]
db 0x80, 0xFC, 0x77                     ; 7A11 cmp ah,0x77
db 0x73, 0x01                           ; 7A14 jnc 0x7a17
db 0xC3                                 ; 7A16 ret
db 0x0A, 0xC0                           ; 7A17 or al,al
db 0x79, 0x09                           ; 7A19 jns 0x7a24
db 0x24, 0x7F                           ; 7A1B and al,0x7f
db 0xA2, 0xA5, 0x04                     ; 7A1D mov [0x4a5],al
db 0xB8, 0xB0, 0x7D                     ; 7A20 mov ax,0x7db0
db 0x50                                 ; 7A23 push ax
db 0xE8, 0x5B, 0x00                     ; 7A24 call 0x7a82
db 0xA0, 0xA6, 0x04                     ; 7A27 mov al,[0x4a6]
db 0x0A, 0xC0                           ; 7A2A or al,al
db 0x74, 0x05                           ; 7A2C jz 0x7a33
db 0x80, 0x06, 0xA6, 0x04, 0x02         ; 7A2E add byte [0x4a6],0x2
db 0xE8, 0x61, 0x00                     ; 7A33 call 0x7a97
db 0xA1, 0xB1, 0x04                     ; 7A36 mov ax,[0x4b1]
db 0x80, 0xFC, 0x82                     ; 7A39 cmp ah,0x82
db 0x9C                                 ; 7A3C pushf
db 0xF6, 0xC4, 0x01                     ; 7A3D test ah,0x1
db 0x75, 0x02                           ; 7A40 jnz 0x7a44
db 0xA8, 0x40                           ; 7A42 test al,0x40
db 0x9C                                 ; 7A44 pushf
db 0xE8, 0x49, 0x00                     ; 7A45 call 0x7a91
db 0x9D                                 ; 7A48 popf
db 0x74, 0x09                           ; 7A49 jz 0x7a54
db 0xBB, 0x32, 0x60                     ; 7A4B mov bx,0x6032
db 0xE8, 0x37, 0x02                     ; 7A4E call 0x7c88
db 0xE8, 0x48, 0xEC                     ; 7A51 call 0x669c
db 0x80, 0x2E, 0xA6, 0x04, 0x02         ; 7A54 sub byte [0x4a6],0x2
db 0x73, 0x03                           ; 7A59 jnc 0x7a5e
db 0xE8, 0x00, 0x01                     ; 7A5B call 0x7b5e
db 0xE8, 0x03, 0xF1                     ; 7A5E call 0x6b64
db 0xA0, 0xA6, 0x04                     ; 7A61 mov al,[0x4a6]
db 0x3C, 0x74                           ; 7A64 cmp al,0x74
db 0x73, 0x0B                           ; 7A66 jnc 0x7a73
db 0xBA, 0xDB, 0x0F                     ; 7A68 mov dx,0xfdb
db 0xBB, 0x49, 0x83                     ; 7A6B mov bx,0x8349
db 0xE8, 0x8E, 0xF2                     ; 7A6E call 0x6cff
db 0xEB, 0x06                           ; 7A71 jmp short 0x7a79
db 0xBB, 0x34, 0x62                     ; 7A73 mov bx,0x6234
db 0xE8, 0xC6, 0xFA                     ; 7A76 call 0x753f
db 0x9D                                 ; 7A79 popf
db 0x75, 0x05                           ; 7A7A jnz 0x7a81
db 0x80, 0x36, 0xA5, 0x04, 0x80         ; 7A7C xor byte [0x4a5],0x80
db 0xC3                                 ; 7A81 ret
db 0xBB, 0x63, 0x62                     ; 7A82 mov bx,0x6263
db 0xE8, 0x00, 0x02                     ; 7A85 call 0x7c88
db 0xE8, 0x08, 0xF1                     ; 7A88 call 0x6b93
db 0xE8, 0xC7, 0xF1                     ; 7A8B call 0x6c55
db 0xE8, 0x06, 0x00                     ; 7A8E call 0x7a97
db 0xE8, 0x08, 0xEC                     ; 7A91 call 0x669c
db 0xE9, 0x19, 0x03                     ; 7A94 jmp 0x7db0
db 0xE8, 0xAD, 0x03                     ; 7A97 call 0x7e47
db 0xE8, 0xA4, 0xF7                     ; 7A9A call 0x7241
db 0xE8, 0x00, 0x02                     ; 7A9D call 0x7ca0
db 0xE8, 0xC3, 0x03                     ; 7AA0 call 0x7e66
db 0xC3                                 ; 7AA3 ret
db 0xBB, 0x32, 0x60                     ; 7AA4 mov bx,0x6032
db 0xE9, 0xDE, 0x01                     ; 7AA7 jmp 0x7c88
db 0xB8, 0xF0, 0xC3                     ; 7AAA mov ax,0xc3f0
db 0xFF, 0x36, 0xA5, 0x04               ; 7AAD push word [0x4a5]
db 0xFF, 0x36, 0xA3, 0x04               ; 7AB1 push word [0x4a3]
db 0xE8, 0x56, 0xFF                     ; 7AB5 call 0x7a0e
db 0x5A                                 ; 7AB8 pop dx
db 0x5B                                 ; 7AB9 pop bx
db 0xFF, 0x36, 0xA3, 0x04               ; 7ABA push word [0x4a3]
db 0xFF, 0x36, 0xA5, 0x04               ; 7ABE push word [0x4a5]
db 0xE8, 0xF9, 0x01                     ; 7AC2 call 0x7cbe
db 0xE8, 0x2C, 0xFF                     ; 7AC5 call 0x79f4
db 0x5B                                 ; 7AC8 pop bx
db 0x5A                                 ; 7AC9 pop dx
db 0xE9, 0x11, 0xEE                     ; 7ACA jmp 0x68de
db 0xA1, 0xA5, 0x04                     ; 7ACD mov ax,[0x4a5]
db 0x0A, 0xC0                           ; 7AD0 or al,al
db 0x79, 0x09                           ; 7AD2 jns 0x7add
db 0xBF, 0xB0, 0x7D                     ; 7AD4 mov di,0x7db0
db 0x57                                 ; 7AD7 push di
db 0x24, 0x7F                           ; 7AD8 and al,0x7f
db 0xA2, 0xA5, 0x04                     ; 7ADA mov [0x4a5],al
db 0x80, 0xFC, 0x81                     ; 7ADD cmp ah,0x81
db 0x72, 0x0C                           ; 7AE0 jc 0x7aee
db 0xBF, 0x39, 0x7B                     ; 7AE2 mov di,0x7b39
db 0x57                                 ; 7AE5 push di
db 0x33, 0xD2                           ; 7AE6 xor dx,dx
db 0xBB, 0x00, 0x81                     ; 7AE8 mov bx,0x8100
db 0xE8, 0xF0, 0xED                     ; 7AEB call 0x68de
db 0xBA, 0xA2, 0x30                     ; 7AEE mov dx,0x30a2
db 0xBB, 0x09, 0x7F                     ; 7AF1 mov bx,0x7f09
db 0xE8, 0xE1, 0x01                     ; 7AF4 call 0x7cd8
db 0x78, 0x3A                           ; 7AF7 js 0x7b33
db 0xBF, 0x42, 0x7B                     ; 7AF9 mov di,0x7b42
db 0x57                                 ; 7AFC push di
db 0xFF, 0x36, 0xA3, 0x04               ; 7AFD push word [0x4a3]
db 0xFF, 0x36, 0xA5, 0x04               ; 7B01 push word [0x4a5]
db 0xBA, 0xD7, 0xB3                     ; 7B05 mov dx,0xb3d7
db 0xBB, 0x5D, 0x81                     ; 7B08 mov bx,0x815d
db 0xE8, 0x65, 0xEC                     ; 7B0B call 0x6773
db 0x5B                                 ; 7B0E pop bx
db 0x5A                                 ; 7B0F pop dx
db 0xFF, 0x36, 0xA3, 0x04               ; 7B10 push word [0x4a3]
db 0xFF, 0x36, 0xA5, 0x04               ; 7B14 push word [0x4a5]
db 0xE8, 0xA3, 0x01                     ; 7B18 call 0x7cbe
db 0xBB, 0x49, 0x62                     ; 7B1B mov bx,0x6249
db 0xE8, 0x31, 0xFA                     ; 7B1E call 0x7552
db 0x5B                                 ; 7B21 pop bx
db 0x5A                                 ; 7B22 pop dx
db 0xFF, 0x36, 0xA3, 0x04               ; 7B23 push word [0x4a3]
db 0xFF, 0x36, 0xA5, 0x04               ; 7B27 push word [0x4a5]
db 0xE8, 0x90, 0x01                     ; 7B2B call 0x7cbe
db 0x5B                                 ; 7B2E pop bx
db 0x5A                                 ; 7B2F pop dx
db 0xE8, 0xAB, 0xED                     ; 7B30 call 0x68de
db 0xBB, 0x52, 0x62                     ; 7B33 mov bx,0x6252
db 0xE9, 0x06, 0xFA                     ; 7B36 jmp 0x753f
db 0xBA, 0xDB, 0x0F                     ; 7B39 mov dx,0xfdb
db 0xBB, 0x49, 0x81                     ; 7B3C mov bx,0x8149
db 0xE9, 0x25, 0xEC                     ; 7B3F jmp 0x6767
db 0xBA, 0x92, 0x0A                     ; 7B42 mov dx,0xa92
db 0xBB, 0x06, 0x80                     ; 7B45 mov bx,0x8006
db 0xE9, 0x28, 0xEC                     ; 7B48 jmp 0x6773

; print a string ?
L_7B4B:
  call    L_2BA5

  cmp     al, 0x0d
  jne     L_7B55

  call    L_2C78

L_7B55:
  mov     al, [cs:bx]
  inc     bx
  or      al, al
  jnz     L_7B4B
  ret

L_7B5E:
db 0xBF, 0x9F, 0x04                     ; 7B5E mov di,0x49f
db 0xB9, 0x04, 0x00                     ; 7B61 mov cx,0x4
db 0xB8, 0x00, 0x00                     ; 7B64 mov ax,0x0
db 0xFC                                 ; 7B67 cld
db 0xF3, 0xAB                           ; 7B68 rep stosw
  ret

L_7B6B:
  mov     ax, 0
  mov     [0x4a3], ax
  mov     [0x4a5], ax
  ret

db 0xE8, 0x78, 0xE7                     ; 7B75 call 0x62f0
db 0x79, 0x0E                           ; 7B78 jns 0x7b88
db 0xA1, 0xA3, 0x04                     ; 7B7A mov ax,[0x4a3]
db 0x0B, 0xC0                           ; 7B7D or ax,ax
db 0x74, 0x20                           ; 7B7F jz 0x7ba1
db 0xB0, 0x01                           ; 7B81 mov al,0x1
db 0x79, 0x1C                           ; 7B83 jns 0x7ba1
db 0xF6, 0xD8                           ; 7B85 neg al
db 0xC3                                 ; 7B87 ret
db 0xCD, 0xD4                           ; 7B88 int 0xd4
db 0xA0, 0xA6, 0x04                     ; 7B8A mov al,[0x4a6]
db 0x0A, 0xC0                           ; 7B8D or al,al
db 0x74, 0x10                           ; 7B8F jz 0x7ba1
db 0xA0, 0xA5, 0x04                     ; 7B91 mov al,[0x4a5]
db 0x0A, 0xC0                           ; 7B94 or al,al
db 0x74, 0x07                           ; 7B96 jz 0x7b9f
db 0xB0, 0x01                           ; 7B98 mov al,0x1
db 0x79, 0x05                           ; 7B9A jns 0x7ba1
db 0xF6, 0xD8                           ; 7B9C neg al
db 0xC3                                 ; 7B9E ret
db 0x0C, 0x01                           ; 7B9F or al,0x1
db 0xC3                                 ; 7BA1 ret
db 0xA0, 0xFB, 0x02                     ; 7BA2 mov al,[0x2fb]
db 0x3C, 0x08                           ; 7BA5 cmp al,0x8
db 0xFE, 0xC8                           ; 7BA7 dec al
db 0xFE, 0xC8                           ; 7BA9 dec al
db 0xFE, 0xC8                           ; 7BAB dec al
db 0xC3                                 ; 7BAD ret
db 0xE8, 0xF1, 0xFF                     ; 7BAE call 0x7ba2
db 0x72, 0x0C                           ; 7BB1 jc 0x7bbf
db 0x53                                 ; 7BB3 push bx
db 0xBB, 0x6A, 0x61                     ; 7BB4 mov bx,0x616a
db 0xE8, 0xCE, 0x00                     ; 7BB7 call 0x7c88
db 0xE8, 0xED, 0xEA                     ; 7BBA call 0x66aa
db 0x5B                                 ; 7BBD pop bx
db 0xC3                                 ; 7BBE ret
db 0x33, 0xD2                           ; 7BBF xor dx,dx
db 0xBB, 0x00, 0x80                     ; 7BC1 mov bx,0x8000
db 0xE8, 0xAC, 0xEB                     ; 7BC4 call 0x6773
db 0xC3                                 ; 7BC7 ret
db 0xE8, 0xD7, 0xFF                     ; 7BC8 call 0x7ba2
db 0xBB, 0x2A, 0x60                     ; 7BCB mov bx,0x602a
db 0x72, 0x11                           ; 7BCE jc 0x7be1
db 0xEB, 0x08                           ; 7BD0 jmp short 0x7bda
db 0xE8, 0xCD, 0xFF                     ; 7BD2 call 0x7ba2
db 0xBB, 0x3A, 0x60                     ; 7BD5 mov bx,0x603a
db 0x72, 0x07                           ; 7BD8 jc 0x7be1
db 0xE8, 0xAB, 0x00                     ; 7BDA call 0x7c88
db 0xE8, 0x75, 0xF0                     ; 7BDD call 0x6c55
db 0xC3                                 ; 7BE0 ret
db 0xFF, 0x36, 0xA5, 0x04               ; 7BE1 push word [0x4a5]
db 0xFF, 0x36, 0xA3, 0x04               ; 7BE5 push word [0x4a3]
db 0xC6, 0x06, 0xFB, 0x02, 0x08         ; 7BE9 mov byte [0x2fb],0x8
db 0xE8, 0x9C, 0x00                     ; 7BEE call 0x7c8d
db 0xE8, 0x70, 0xEF                     ; 7BF1 call 0x6b64
db 0x5A                                 ; 7BF4 pop dx
db 0x5B                                 ; 7BF5 pop bx
db 0xE8, 0x06, 0xF1                     ; 7BF6 call 0x6cff
db 0xC3                                 ; 7BF9 ret
db 0xB9, 0x04, 0x00                     ; 7BFA mov cx,0x4
db 0xD1, 0x17                           ; 7BFD rcl word [bx],1
db 0x43                                 ; 7BFF inc bx
db 0x43                                 ; 7C00 inc bx
db 0xE2, 0xFA                           ; 7C01 loop 0x7bfd
db 0xC3                                 ; 7C03 ret
db 0xB9, 0x04, 0x00                     ; 7C04 mov cx,0x4
db 0xD1, 0x1F                           ; 7C07 rcr word [bx],1
db 0x4B                                 ; 7C09 dec bx
db 0x4B                                 ; 7C0A dec bx
db 0xE2, 0xFA                           ; 7C0B loop 0x7c07
db 0xC3                                 ; 7C0D ret
db 0x80, 0x8F, 0x02, 0x00, 0x20         ; 7C0E or byte [bx+0x2],0x20
db 0xE2, 0x01                           ; 7C13 loop 0x7c16
db 0xC3                                 ; 7C15 ret
db 0xBB, 0xB0, 0x04                     ; 7C16 mov bx,0x4b0
db 0x80, 0xF9, 0x08                     ; 7C19 cmp cl,0x8
db 0x72, 0x26                           ; 7C1C jc 0x7c44
db 0x51                                 ; 7C1E push cx
db 0xB9, 0x07, 0x00                     ; 7C1F mov cx,0x7
db 0xBB, 0xAA, 0x04                     ; 7C22 mov bx,0x4aa
db 0x8A, 0x27                           ; 7C25 mov ah,[bx]
db 0x8A, 0x87, 0x01, 0x00               ; 7C27 mov al,[bx+0x1]
db 0x88, 0x07                           ; 7C2B mov [bx],al
db 0x43                                 ; 7C2D inc bx
db 0xE2, 0xF7                           ; 7C2E loop 0x7c27
db 0x32, 0xC0                           ; 7C30 xor al,al
db 0x88, 0x07                           ; 7C32 mov [bx],al
db 0x59                                 ; 7C34 pop cx
db 0x80, 0xE9, 0x08                     ; 7C35 sub cl,0x8
db 0x80, 0xE4, 0x20                     ; 7C38 and ah,0x20
db 0x74, 0xD9                           ; 7C3B jz 0x7c16
db 0x08, 0x26, 0xAA, 0x04               ; 7C3D or [0x4aa],ah
db 0xE9, 0xD2, 0xFF                     ; 7C41 jmp 0x7c16
db 0x0A, 0xC9                           ; 7C44 or cl,cl
db 0x74, 0x0F                           ; 7C46 jz 0x7c57
db 0x51                                 ; 7C48 push cx
db 0xF8                                 ; 7C49 clc
db 0xE8, 0xB7, 0xFF                     ; 7C4A call 0x7c04
db 0x59                                 ; 7C4D pop cx
db 0xF6, 0x87, 0x02, 0x00, 0x10         ; 7C4E test byte [bx+0x2],0x10
db 0x75, 0xB9                           ; 7C53 jnz 0x7c0e
db 0xE2, 0xBF                           ; 7C55 loop 0x7c16
db 0xC3                                 ; 7C57 ret
db 0xBE, 0x9F, 0x04                     ; 7C58 mov si,0x49f
db 0xBF, 0xAB, 0x04                     ; 7C5B mov di,0x4ab
db 0xFC                                 ; 7C5E cld
db 0xB9, 0x04, 0x00                     ; 7C5F mov cx,0x4
db 0x8B, 0x05                           ; 7C62 mov ax,[di]
db 0xA5                                 ; 7C64 movsw
db 0x89, 0x84, 0xFE, 0xFF               ; 7C65 mov [si+0xfffe],ax
db 0xE2, 0xF7                           ; 7C69 loop 0x7c62
db 0xC3                                 ; 7C6B ret
db 0xBF, 0x7C, 0x04                     ; 7C6C mov di,0x47c
db 0xB9, 0x02, 0x00                     ; 7C6F mov cx,0x2
db 0xEB, 0x06                           ; 7C72 jmp short 0x7c7a
db 0xBF, 0x78, 0x04                     ; 7C74 mov di,0x478
db 0xB9, 0x04, 0x00                     ; 7C77 mov cx,0x4
db 0xFC                                 ; 7C7A cld
db 0x2E, 0x8B, 0x07                     ; 7C7B mov ax,[cs:bx]
db 0xAB                                 ; 7C7E stosw
db 0x43                                 ; 7C7F inc bx
db 0x43                                 ; 7C80 inc bx
db 0xE2, 0xF8                           ; 7C81 loop 0x7c7b
db 0x8B, 0xDF                           ; 7C83 mov bx,di
db 0x4B                                 ; 7C85 dec bx
db 0x4B                                 ; 7C86 dec bx
db 0xC3                                 ; 7C87 ret
db 0xBF, 0xAB, 0x04                     ; 7C88 mov di,0x4ab
db 0xEB, 0xEA                           ; 7C8B jmp short 0x7c77
db 0xBF, 0x9F, 0x04                     ; 7C8D mov di,0x49f
db 0xEB, 0xE5                           ; 7C90 jmp short 0x7c77
db 0xBF, 0xAB, 0x04                     ; 7C92 mov di,0x4ab
db 0xB9, 0x04, 0x00                     ; 7C95 mov cx,0x4
db 0x87, 0xDE                           ; 7C98 xchg bx,si
db 0xFC                                 ; 7C9A cld
db 0xF3, 0xA5                           ; 7C9B rep movsw
db 0x87, 0xDE                           ; 7C9D xchg bx,si
db 0xC3                                 ; 7C9F ret
db 0x51                                 ; 7CA0 push cx
db 0x53                                 ; 7CA1 push bx
db 0x57                                 ; 7CA2 push di
db 0xBB, 0x9F, 0x04                     ; 7CA3 mov bx,0x49f
db 0xBF, 0xAB, 0x04                     ; 7CA6 mov di,0x4ab
db 0xB9, 0x04, 0x00                     ; 7CA9 mov cx,0x4
db 0xE8, 0xE9, 0xFF                     ; 7CAC call 0x7c98
db 0x5F                                 ; 7CAF pop di
db 0x5B                                 ; 7CB0 pop bx
db 0x59                                 ; 7CB1 pop cx
db 0xC3                                 ; 7CB2 ret
db 0x51                                 ; 7CB3 push cx
db 0x53                                 ; 7CB4 push bx
db 0x57                                 ; 7CB5 push di
db 0xBB, 0xAB, 0x04                     ; 7CB6 mov bx,0x4ab
db 0xBF, 0x9F, 0x04                     ; 7CB9 mov di,0x49f
db 0xEB, 0xEB                           ; 7CBC jmp short 0x7ca9
db 0x89, 0x16, 0xA3, 0x04               ; 7CBE mov [0x4a3],dx
db 0x89, 0x1E, 0xA5, 0x04               ; 7CC2 mov [0x4a5],bx
db 0xC3                                 ; 7CC6 ret
db 0x8B, 0x16, 0xA3, 0x04               ; 7CC7 mov dx,[0x4a3]
db 0x8B, 0x1E, 0xA5, 0x04               ; 7CCB mov bx,[0x4a5]
db 0xC3                                 ; 7CCF ret
db 0xE8, 0xCF, 0xFE                     ; 7CD0 call 0x7ba2
db 0x72, 0x3F                           ; 7CD3 jc 0x7d14
db 0xE9, 0x89, 0x00                     ; 7CD5 jmp 0x7d61
db 0xE8, 0xD7, 0xED                     ; 7CD8 call 0x6ab2
db 0x53                                 ; 7CDB push bx
db 0x57                                 ; 7CDC push di
db 0x8A, 0xC3                           ; 7CDD mov al,bl
db 0x32, 0x06, 0xA5, 0x04               ; 7CDF xor al,[0x4a5]
db 0x78, 0x3C                           ; 7CE3 js 0x7d21
db 0x0A, 0xDB                           ; 7CE5 or bl,bl
db 0x78, 0x10                           ; 7CE7 js 0x7cf9
db 0xA1, 0xA5, 0x04                     ; 7CE9 mov ax,[0x4a5]
db 0x2B, 0xC3                           ; 7CEC sub ax,bx
db 0x72, 0x3F                           ; 7CEE jc 0x7d2f
db 0x75, 0x37                           ; 7CF0 jnz 0x7d29
db 0xA1, 0xA3, 0x04                     ; 7CF2 mov ax,[0x4a3]
db 0x2B, 0xC2                           ; 7CF5 sub ax,dx
db 0xEB, 0x10                           ; 7CF7 jmp short 0x7d09
db 0x8B, 0xC3                           ; 7CF9 mov ax,bx
db 0x2B, 0x06, 0xA5, 0x04               ; 7CFB sub ax,[0x4a5]
db 0x72, 0x2E                           ; 7CFF jc 0x7d2f
db 0x75, 0x26                           ; 7D01 jnz 0x7d29
db 0x8B, 0xC2                           ; 7D03 mov ax,dx
db 0x2B, 0x06, 0xA3, 0x04               ; 7D05 sub ax,[0x4a3]
db 0x72, 0x24                           ; 7D09 jc 0x7d2f
db 0x75, 0x1C                           ; 7D0B jnz 0x7d29
db 0x32, 0xC0                           ; 7D0D xor al,al
db 0xEB, 0x4A                           ; 7D0F jmp short 0x7d5b
db 0xC0, 0xEB, 0x47                     ; 7D11 shr bl,0x47
db 0xE8, 0xA3, 0xED                     ; 7D14 call 0x6aba
db 0x90                                 ; 7D17 nop
db 0x90                                 ; 7D18 nop
db 0x8B, 0x07                           ; 7D19 mov ax,[bx]
db 0x32, 0x06, 0xA5, 0x04               ; 7D1B xor al,[0x4a5]
db 0x79, 0x13                           ; 7D1F jns 0x7d34
db 0x8A, 0x26, 0xA5, 0x04               ; 7D21 mov ah,[0x4a5]
db 0x0A, 0xE4                           ; 7D25 or ah,ah
db 0x78, 0x06                           ; 7D27 js 0x7d2f
db 0xB0, 0x01                           ; 7D29 mov al,0x1
db 0x0A, 0xC0                           ; 7D2B or al,al
db 0xEB, 0x2C                           ; 7D2D jmp short 0x7d5b
db 0xB0, 0xFF                           ; 7D2F mov al,0xff
db 0xF9                                 ; 7D31 stc
db 0xEB, 0x27                           ; 7D32 jmp short 0x7d5b
db 0x51                                 ; 7D34 push cx
db 0xB9, 0x02, 0x00                     ; 7D35 mov cx,0x2
db 0x87, 0xDE                           ; 7D38 xchg bx,si
db 0xA0, 0xA5, 0x04                     ; 7D3A mov al,[0x4a5]
db 0x0A, 0xC0                           ; 7D3D or al,al
db 0x79, 0x02                           ; 7D3F jns 0x7d43
db 0x87, 0xF7                           ; 7D41 xchg si,di
db 0xFD                                 ; 7D43 std
db 0xA7                                 ; 7D44 cmpsw
db 0x75, 0x06                           ; 7D45 jnz 0x7d4d
db 0xE2, 0xFB                           ; 7D47 loop 0x7d44
db 0xB0, 0x00                           ; 7D49 mov al,0x0
db 0xEB, 0x0D                           ; 7D4B jmp short 0x7d5a
db 0x73, 0x06                           ; 7D4D jnc 0x7d55
db 0xB0, 0x01                           ; 7D4F mov al,0x1
db 0x0A, 0xC0                           ; 7D51 or al,al
db 0xEB, 0x05                           ; 7D53 jmp short 0x7d5a
db 0xB0, 0xFF                           ; 7D55 mov al,0xff
db 0x0A, 0xC0                           ; 7D57 or al,al
db 0xF9                                 ; 7D59 stc
db 0x59                                 ; 7D5A pop cx
db 0x5F                                 ; 7D5B pop di
db 0x5B                                 ; 7D5C pop bx
db 0xC3                                 ; 7D5D ret
db 0xBB, 0xB1, 0x04                     ; 7D5E mov bx,0x4b1
db 0xE8, 0x56, 0xED                     ; 7D61 call 0x6aba
db 0x90                                 ; 7D64 nop
db 0x90                                 ; 7D65 nop
db 0x8A, 0x05                           ; 7D66 mov al,[di]
db 0x32, 0x07                           ; 7D68 xor al,[bx]
db 0x79, 0x02                           ; 7D6A jns 0x7d6e
db 0xEB, 0xB3                           ; 7D6C jmp short 0x7d21
db 0x51                                 ; 7D6E push cx
db 0xB9, 0x04, 0x00                     ; 7D6F mov cx,0x4
db 0xEB, 0xC4                           ; 7D72 jmp short 0x7d38
db 0xBB, 0xFF, 0x61                     ; 7D74 mov bx,0x61ff
db 0xE8, 0xF2, 0xFE                     ; 7D77 call 0x7c6c
db 0xE8, 0x97, 0xFF                     ; 7D7A call 0x7d14
db 0x75, 0x0B                           ; 7D7D jnz 0x7d8a
db 0xC6, 0x06, 0xFB, 0x02, 0x02         ; 7D7F mov byte [0x2fb],0x2
db 0xC7, 0x06, 0xA3, 0x04, 0x00, 0x80   ; 7D84 mov word [0x4a3],0x8000
db 0xC3                                 ; 7D8A ret
db 0x2E, 0x2B, 0x96, 0x00, 0x00         ; 7D8B sub dx,[cs:bp+0x0]
db 0x2E, 0x1A, 0x9E, 0x02, 0x00         ; 7D90 sbb bl,[cs:bp+0x2]
db 0xC3                                 ; 7D95 ret
db 0xE8, 0x09, 0xFE                     ; 7D96 call 0x7ba2
db 0x78, 0x08                           ; 7D99 js 0x7da3
db 0xA0, 0xA5, 0x04                     ; 7D9B mov al,[0x4a5]
db 0x0A, 0xC0                           ; 7D9E or al,al
db 0x78, 0x0E                           ; 7DA0 js 0x7db0
db 0xC3                                 ; 7DA2 ret
db 0xA1, 0xA3, 0x04                     ; 7DA3 mov ax,[0x4a3]
db 0x0B, 0xC0                           ; 7DA6 or ax,ax
db 0x78, 0x11                           ; 7DA8 js 0x7dbb
db 0xC3                                 ; 7DAA ret
db 0xE8, 0xF4, 0xFD                     ; 7DAB call 0x7ba2
db 0x78, 0x08                           ; 7DAE js 0x7db8
db 0xCD, 0xD2                           ; 7DB0 int 0xd2
db 0x80, 0x36, 0xA5, 0x04, 0x80         ; 7DB2 xor byte [0x4a5],0x80
db 0xC3                                 ; 7DB7 ret
db 0xA1, 0xA3, 0x04                     ; 7DB8 mov ax,[0x4a3]
db 0x3D, 0x00, 0x80                     ; 7DBB cmp ax,0x8000
db 0x75, 0x0A                           ; 7DBE jnz 0x7dca
db 0xCD, 0xD3                           ; 7DC0 int 0xd3
db 0x53                                 ; 7DC2 push bx
db 0xE8, 0xDB, 0xED                     ; 7DC3 call 0x6ba1
db 0x5B                                 ; 7DC6 pop bx
db 0xE9, 0xE6, 0xFF                     ; 7DC7 jmp 0x7db0
db 0xF7, 0x1E, 0xA3, 0x04               ; 7DCA neg word [0x4a3]
db 0xC3                                 ; 7DCE ret
db 0xBB, 0x79, 0x04                     ; 7DCF mov bx,0x479
db 0xE8, 0x33, 0x00                     ; 7DD2 call 0x7e08
db 0xBF, 0x97, 0x04                     ; 7DD5 mov di,0x497
db 0xB9, 0x08, 0x00                     ; 7DD8 mov cx,0x8
db 0xB8, 0x00, 0x00                     ; 7DDB mov ax,0x0
db 0xFC                                 ; 7DDE cld
db 0xF3, 0xAB                           ; 7DDF rep stosw
db 0xA2, 0x78, 0x04                     ; 7DE1 mov [0x478],al
db 0xA2, 0xAA, 0x04                     ; 7DE4 mov [0x4aa],al
db 0xC3                                 ; 7DE7 ret
db 0xE8, 0xB7, 0xFD                     ; 7DE8 call 0x7ba2
db 0x72, 0x03                           ; 7DEB jc 0x7df0
db 0xE9, 0xA2, 0xFE                     ; 7DED jmp 0x7c92
db 0x8B, 0x17                           ; 7DF0 mov dx,[bx]
db 0x8B, 0x9F, 0x02, 0x00               ; 7DF2 mov bx,[bx+0x2]
db 0xC3                                 ; 7DF6 ret
db 0xB9, 0x04, 0x00                     ; 7DF7 mov cx,0x4
db 0xE8, 0xA5, 0xFD                     ; 7DFA call 0x7ba2
db 0x72, 0x03                           ; 7DFD jc 0x7e02
db 0xE9, 0x96, 0xFE                     ; 7DFF jmp 0x7c98
db 0xB9, 0x02, 0x00                     ; 7E02 mov cx,0x2
db 0xE9, 0x90, 0xFE                     ; 7E05 jmp 0x7c98
db 0xB9, 0x04, 0x00                     ; 7E08 mov cx,0x4
db 0x87, 0xFB                           ; 7E0B xchg di,bx
db 0xBB, 0x9F, 0x04                     ; 7E0D mov bx,0x49f
db 0xE8, 0x8F, 0xFD                     ; 7E10 call 0x7ba2
db 0x72, 0x03                           ; 7E13 jc 0x7e18
db 0xE9, 0x80, 0xFE                     ; 7E15 jmp 0x7c98
db 0x87, 0xDF                           ; 7E18 xchg bx,di
db 0xB9, 0x02, 0x00                     ; 7E1A mov cx,0x2
db 0xBF, 0xA3, 0x04                     ; 7E1D mov di,0x4a3
db 0x87, 0xFB                           ; 7E20 xchg di,bx
db 0xE9, 0x73, 0xFE                     ; 7E22 jmp 0x7c98
db 0xB9, 0x04, 0x00                     ; 7E25 mov cx,0x4
db 0xBF, 0x9F, 0x04                     ; 7E28 mov di,0x49f
db 0xE8, 0x74, 0xFD                     ; 7E2B call 0x7ba2
db 0x72, 0x03                           ; 7E2E jc 0x7e33
db 0xE9, 0x65, 0xFE                     ; 7E30 jmp 0x7c98
db 0xB9, 0x02, 0x00                     ; 7E33 mov cx,0x2
db 0xBF, 0xA3, 0x04                     ; 7E36 mov di,0x4a3
db 0xE9, 0x5C, 0xFE                     ; 7E39 jmp 0x7c98
db 0xE8, 0x63, 0xFD                     ; 7E3C call 0x7ba2
db 0x72, 0x03                           ; 7E3F jc 0x7e44
db 0xE9, 0x1D, 0xFF                     ; 7E41 jmp 0x7d61
db 0xE9, 0xCD, 0xFE                     ; 7E44 jmp 0x7d14
db 0xE8, 0x58, 0xFD                     ; 7E47 call 0x7ba2
db 0xB9, 0x04, 0x00                     ; 7E4A mov cx,0x4
db 0x73, 0x03                           ; 7E4D jnc 0x7e52
db 0xB9, 0x02, 0x00                     ; 7E4F mov cx,0x2
db 0x5D                                 ; 7E52 pop bp
db 0xBF, 0xA5, 0x04                     ; 7E53 mov di,0x4a5
db 0xFF, 0x35                           ; 7E56 push word [di]
db 0x4F                                 ; 7E58 dec di
db 0x4F                                 ; 7E59 dec di
db 0xE2, 0xFA                           ; 7E5A loop 0x7e56
db 0x55                                 ; 7E5C push bp
db 0xC3                                 ; 7E5D ret
db 0xBF, 0xAB, 0x04                     ; 7E5E mov di,0x4ab
db 0xB9, 0x04, 0x00                     ; 7E61 mov cx,0x4
db 0xEB, 0x11                           ; 7E64 jmp short 0x7e77
db 0xE8, 0x39, 0xFD                     ; 7E66 call 0x7ba2
db 0xBF, 0x9F, 0x04                     ; 7E69 mov di,0x49f
db 0xB9, 0x04, 0x00                     ; 7E6C mov cx,0x4
db 0x73, 0x06                           ; 7E6F jnc 0x7e77
db 0xBF, 0xA3, 0x04                     ; 7E71 mov di,0x4a3
db 0xB9, 0x02, 0x00                     ; 7E74 mov cx,0x2
db 0x58                                 ; 7E77 pop ax
db 0x8F, 0x05                           ; 7E78 pop word [di]
db 0x47                                 ; 7E7A inc di
db 0x47                                 ; 7E7B inc di
db 0xE2, 0xFA                           ; 7E7C loop 0x7e78
db 0x50                                 ; 7E7E push ax
db 0xC3                                 ; 7E7F ret
db 0xE8, 0x1F, 0xFD                     ; 7E80 call 0x7ba2
db 0x79, 0x01                           ; 7E83 jns 0x7e86
db 0xC3                                 ; 7E85 ret
db 0xCD, 0xD5                           ; 7E86 int 0xd5
db 0x72, 0x03                           ; 7E88 jc 0x7e8d
db 0xE9, 0xB4, 0xF3                     ; 7E8A jmp 0x7241
db 0xE9, 0x1B, 0xF4                     ; 7E8D jmp 0x72ab
db 0x00, 0x00                           ; 7E90 add [bx+si],al

L_7E92:
  cli

  mov     dx, 0x0060
  mov     ds, dx
  mov     es, dx
  mov     ss, dx

  xor     al, al
  mov     [0x464], al

  mov     ch, L_072B - L_069A ;0x91
  mov     bx, 0
  mov     dx, L_069A

L_7EA9
  mov     si, dx
  cs      lodsb
  mov     [bx], al

  inc     bx
  inc     dx
  dec     ch
  jnz     L_7EA9

  mov     sp, 0x070e

  int     0x12

  cli

  mov     bx, 64
  mul     bx                            ; convert to segment address

  mov     bx, ds
  sub     ax, bx

  ; AX contains RAM size after segment 0060 in paragraphs.

  mov     bx, 0

  test    ah, 0xf0
  jnz     L_7ED2                        ; We have at least 64K at 0060

  mov     cl, 4
  shl     ax, cl                        ; paragraphs to bytes
  mov     bx, ax

L_7ED2:
  dec     bx
  mov     [0x2c], bx

  mov     sp, bx

  jmp     L_4BFE

L_7EDC:
db 0xB0, 0x2C                           ; 7EDC mov al,0x2c
db 0xA2, 0xF6, 0x01                     ; 7EDE mov [0x1f6],al
db 0xBB, 0xB7, 0x00                     ; 7EE1 mov bx,0xb7
db 0xC6, 0x07, 0x3A                     ; 7EE4 mov byte [bx],0x3a
db 0x32, 0xC0                           ; 7EE7 xor al,al
db 0xA2, 0xF9, 0x02                     ; 7EE9 mov [0x2f9],al
db 0xA2, 0x06, 0x00                     ; 7EEC mov [0x6],al
db 0xA2, 0x6B, 0x04                     ; 7EEF mov [0x46b],al
db 0xA2, 0x65, 0x04                     ; 7EF2 mov [0x465],al
db 0xA2, 0x28, 0x00                     ; 7EF5 mov [0x28],al
db 0xBB, 0x0E, 0x03                     ; 7EF8 mov bx,0x30e
db 0x89, 0x1E, 0x0C, 0x03               ; 7EFB mov [0x30c],bx
db 0xBB, 0x7A, 0x03                     ; 7EFF mov bx,0x37a
db 0x89, 0x1E, 0xE2, 0x03               ; 7F02 mov [0x3e2],bx
db 0x8B, 0x1E, 0x2C, 0x00               ; 7F06 mov bx,[0x2c]
db 0x4B                                 ; 7F0A dec bx
db 0x89, 0x1E, 0x0A, 0x03               ; 7F0B mov [0x30a],bx
db 0x4B                                 ; 7F0F dec bx
db 0x53                                 ; 7F10 push bx
db 0xBB, 0x0E, 0x07                     ; 7F11 mov bx,0x70e
db 0xB0, 0x04                           ; 7F14 mov al,0x4
db 0xA2, 0xDF, 0x04                     ; 7F16 mov [0x4df],al
db 0x53                                 ; 7F19 push bx
db 0x89, 0x1E, 0xE0, 0x04               ; 7F1A mov [0x4e0],bx
db 0xA0, 0xDF, 0x04                     ; 7F1E mov al,[0x4df]
db 0xFE, 0xC0                           ; 7F21 inc al
db 0x02, 0xC0                           ; 7F23 add al,al
db 0x8A, 0xD0                           ; 7F25 mov dl,al
db 0xB6, 0x00                           ; 7F27 mov dh,0x0
db 0x03, 0xDA                           ; 7F29 add bx,dx
db 0x5A                                 ; 7F2B pop dx
db 0x87, 0xDA                           ; 7F2C xchg bx,dx
db 0x8B, 0x1E, 0xE0, 0x04               ; 7F2E mov bx,[0x4e0]
db 0x88, 0x17                           ; 7F32 mov [bx],dl
db 0x43                                 ; 7F34 inc bx
db 0x88, 0x37                           ; 7F35 mov [bx],dh
db 0x43                                 ; 7F37 inc bx
db 0xA0, 0xDF, 0x04                     ; 7F38 mov al,[0x4df]
db 0xB9, 0x34, 0x00                     ; 7F3B mov cx,0x34
db 0x0A, 0xC0                           ; 7F3E or al,al
db 0x74, 0x0E                           ; 7F40 jz 0x7f50
db 0x87, 0xDA                           ; 7F42 xchg bx,dx
db 0x03, 0xD9                           ; 7F44 add bx,cx
db 0x87, 0xDA                           ; 7F46 xchg bx,dx
db 0x89, 0x17                           ; 7F48 mov [bx],dx
db 0x43                                 ; 7F4A inc bx
db 0x43                                 ; 7F4B inc bx
db 0xFE, 0xC8                           ; 7F4C dec al
db 0x75, 0xF2                           ; 7F4E jnz 0x7f42
db 0x87, 0xDA                           ; 7F50 xchg bx,dx
db 0x03, 0xD9                           ; 7F52 add bx,cx
db 0x43                                 ; 7F54 inc bx
db 0x53                                 ; 7F55 push bx
db 0xFE, 0xC8                           ; 7F56 dec al
db 0xA2, 0x36, 0x05                     ; 7F58 mov [0x536],al
db 0x8B, 0x1E, 0xE0, 0x04               ; 7F5B mov bx,[0x4e0]
db 0x8B, 0x17                           ; 7F5F mov dx,[bx]
db 0xBB, 0x33, 0x00                     ; 7F61 mov bx,0x33
db 0x03, 0xDA                           ; 7F64 add bx,dx
db 0x89, 0x1E, 0xE4, 0x04               ; 7F66 mov [0x4e4],bx
db 0x5B                                 ; 7F6A pop bx
db 0x43                                 ; 7F6B inc bx
db 0x89, 0x1E, 0x30, 0x00               ; 7F6C mov [0x30],bx
db 0x89, 0x1E, 0x45, 0x03               ; 7F70 mov [0x345],bx
db 0x5A                                 ; 7F74 pop dx
db 0x8A, 0xC2                           ; 7F75 mov al,dl
db 0x24, 0xFE                           ; 7F77 and al,0xfe
db 0x8A, 0xD0                           ; 7F79 mov dl,al
db 0x8A, 0xC2                           ; 7F7B mov al,dl
db 0x2A, 0xC3                           ; 7F7D sub al,bl
db 0x8A, 0xD8                           ; 7F7F mov bl,al
db 0x8A, 0xC6                           ; 7F81 mov al,dh
db 0x1A, 0xC7                           ; 7F83 sbb al,bh
db 0x8A, 0xF8                           ; 7F85 mov bh,al
db 0x73, 0x03                           ; 7F87 jnc 0x7f8c
db 0xE9, 0x68, 0xAD                     ; 7F89 jmp 0x2cf4
db 0xB1, 0x03                           ; 7F8C mov cl,0x3
db 0xD3, 0xEB                           ; 7F8E shr bx,cl
db 0x8A, 0xC7                           ; 7F90 mov al,bh
db 0x3C, 0x02                           ; 7F92 cmp al,0x2
db 0x72, 0x03                           ; 7F94 jc 0x7f99
db 0xBB, 0x00, 0x02                     ; 7F96 mov bx,0x200
db 0x8A, 0xC2                           ; 7F99 mov al,dl
db 0x2A, 0xC3                           ; 7F9B sub al,bl
db 0x8A, 0xD8                           ; 7F9D mov bl,al
db 0x8A, 0xC6                           ; 7F9F mov al,dh
db 0x1A, 0xC7                           ; 7FA1 sbb al,bh
db 0x8A, 0xF8                           ; 7FA3 mov bh,al
db 0x73, 0x03                           ; 7FA5 jnc 0x7faa
db 0xE9, 0x4A, 0xAD                     ; 7FA7 jmp 0x2cf4
db 0x89, 0x1E, 0x0A, 0x03               ; 7FAA mov [0x30a],bx
db 0x87, 0xDA                           ; 7FAE xchg bx,dx
db 0x89, 0x1E, 0x2C, 0x00               ; 7FB0 mov [0x2c],bx
db 0x89, 0x1E, 0x2F, 0x03               ; 7FB4 mov [0x32f],bx
db 0x8B, 0xE3                           ; 7FB8 mov sp,bx
db 0x89, 0x1E, 0x45, 0x03               ; 7FBA mov [0x345],bx
db 0x8B, 0x1E, 0x30, 0x00               ; 7FBE mov bx,[0x30]
db 0x87, 0xDA                           ; 7FC2 xchg bx,dx
db 0xE8, 0x3D, 0xAD                     ; 7FC4 call 0x2d04
db 0x2B, 0xDA                           ; 7FC7 sub bx,dx
db 0x4B                                 ; 7FC9 dec bx
db 0x4B                                 ; 7FCA dec bx
db 0x53                                 ; 7FCB push bx
db 0x5B                                 ; 7FCC pop bx
db 0xE8, 0x80, 0xE5                     ; 7FCD call 0x6550
db 0xBB, 0xDC, 0x7F                     ; 7FD0 mov bx,0x7fdc
db 0xE8, 0x7F, 0xFB                     ; 7FD3 call 0x7b55
db 0xE8, 0x98, 0xAC                     ; 7FD6 call 0x2c71
db 0xE9, 0x8F, 0xC3                     ; 7FD9 jmp 0x436b
db 0x20, 0x42, 0x79                     ; 7FDC and [bp+si+0x79],al
db 0x74, 0x65                           ; 7FDF jz 0x8046
db 0x73, 0x20                           ; 7FE1 jnc 0x8003
db 0x66, 0x72, 0x65                     ; 7FE3 o32 jc 0x804b
db 0x65, 0x00, 0x14                     ; 7FE6 add [gs:si],dl
db 0xE8, 0xA5, 0xF0                     ; 7FE9 call 0x7091
db 0x33, 0xC9                           ; 7FEC xor cx,cx
db 0x52                                 ; 7FEE push dx
db 0xFF, 0x36, 0x81, 0x04               ; 7FEF push word [0x481]
db 0xE9, 0x68, 0xF6                     ; 7FF3 jmp 0x765e
db 0xFD                                 ; 7FF6 std
db 0xFF, 0x03                           ; 7FF7 inc word [bp+di]
db 0xBF, 0xC9, 0x1B                     ; 7FF9 mov di,0x1bc9
db 0x0E                                 ; 7FFC push cs
db 0xB6, 0x00                           ; 7FFD mov dh,0x0
db 0x00
