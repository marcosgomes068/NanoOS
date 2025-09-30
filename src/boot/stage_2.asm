; ******************************************************* ;
;                               stage_2.asm                              ;
; ******************************************************* ;

org 0x10000
msg db "aaaa", 0
msg_len equ $ - msg

bits 16

%include "utils/interruptions/write.asm"
mov ax, 0x1000
mov ds, ax
mov si, msg
mov cx, msg_len
call write_char


