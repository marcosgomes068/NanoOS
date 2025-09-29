; ******************************************************* ;
;                                  Write.asm                               ;
; ******************************************************* ;
write_char:
    cmp cx, 0
    je .done
    
.loop:
    lodsb
    mov ah, 0xE
    int 0x10
    loop .loop

.done:
    ret