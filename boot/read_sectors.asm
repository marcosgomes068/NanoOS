; ********************************************************* ;
;                           read_sectors.asm                            ;
; ********************************************************* ;
error_reading_hdd db "[-] HDD nao encontrado, procurando floppy..."
ERROR_READING_HDD_LEN equ $ - error_reading_hdd
success_reading_hdd db "[+] HDD encontrado..."
SUCCESS_READING_HDD_LEN equ $ - success_reading_hdd
success_reading_floppy db "[+] HDD encontrado..."
SUCCESS_READING_floppy_LEN equ $ - success_reading_hdd

buffer:
    times 100 db 0 ; buffer de 512 bytes

dap:
    
    db 0x10 ; Tamanho do dap
    db 0x0 ; reservado
    dw 1 ; Setores a ler
    dw buffer - BASE; Offset
    dw BASE ; Segmento
    dq 0 ; LBA 0

read_sectors:
    ; Tenta ler o HDD principal
    xor ax, ax
    mov ds, ax
    mov si, dap
    mov ah, 0x42
    mov dl, 0x80
    int 0x13
    jc .search_floppy
    jmp .success

.search_floppy:
    mov dl,0x00 
    int 0x13
    jc .error
    jmp .success

.error:
    mov si, error_reading_hdd
    mov cx, ERROR_READING_HDD_LEN
    call write_char

.success:
    mov si, success_reading_hdd
    mov cx, SUCCESS_READING_HDD_LEN
    call write_char

