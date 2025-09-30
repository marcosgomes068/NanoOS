; ******************************************************* ;
;                           reset_drive.asm                            ;
; ******************************************************* ;

init_disk_successfully db "[+] Discos reinicializados e em funcionamento", 0xD, 0xA, 0x0
; Msg de sucesso
INIT_DISK_SUCCESSFULLY_LEN equ $ - init_disk_successfully ; Tamanho da msg de sucesso
init_disk_exception db "[-] Não foi possível inicializar o(s) disco(s)", 0xD, 0xA, 0x0    
; Msg de erro
INIT_DISK_EXCEPTION_LEN equ $ - init_disk_exception ; Tamanho da msg de erro

%include "utils/interruptions/write.asm" ; função para escrever na bios

reset_drive:
    mov ah, ZERO ; recarrega os discos para garantir estado conhecido
    mov dl, 0x80 ; 1000 0000 = seleciona todos os dispositivos 
    int 0x13 ; interrupção para discos
    jc .error ; Se der erro mostra a msg (carry flag = 1)
    jmp .done ; Pronto

.error:
    ; Imprime a msg de erro
    mov si, init_disk_exception 
    mov cx, INIT_DISK_EXCEPTION_LEN
    call write_char

.done:
    mov si, init_disk_successfully ; Move o início da msg para si
    mov cx, INIT_DISK_SUCCESSFULLY_LEN ; Tamanho da msg
    call write_char ; Imprime msg de sucesso
    ret