; ******************************************************* ;
;                                 boot.asm                                 ;
; ******************************************************* ;

%DEFINE BASE 0x7C00 ; Inicio do bootloader
%DEFINE SEGMENT_END 0x7E00 ; Fim do bootloader (512 bytes)
%DEFINE VIDEO_FUNC 0x10 ; Funções de vídeo da bios
%DEFINE SIZE 512 ; Tamanho do bootloader
%DEFINE BOOT_SIGN 0xAA55 ; assinatura do Bootloader
%DEFINE ZERO 0x0 ; zero

ORG BASE ; Endereço inicial do boot
BITS 16 ; real-mode CPU


msg db "Hello, World!", ZERO ;  Hello world + null terminator
MSG_LEN equ  $ - msg ; Tamanho da msg (endereço atual - endereço da msg)

;*******************************************************************;
;             Inicializa todos os registradores corretamente          ;
;*******************************************************************;


init_state:
    xor ax, ax ; Zera ax
    mov si, ax ; Zera si
    mov cx, ax ; Zera cx
    mov ss, ax ; Zera ss
    mov sp, 0x7C00 ; Topo da stack iniciando dentro do bootloader
    cld ; Limpa a direction flag 
    cli ; Ignora interrupções da externas



;*******************************************************************;
;            Define o contador e o ponteiro da mensagem           ;
;*******************************************************************;

start:
    call reset_drive
    call read_sectors
;******************************************************************;
;                            Reinicializa os discos                                ;
;******************************************************************;
 
%include "reset_drive.asm"

;*************************************************************;
;                                   Lê os setores                               ;
;*************************************************************;

%include "read_sectors.asm"

;************************************************************************;
;                       Pausa a CPU até a próxima interrupção                ;
;************************************************************************;

.halt:
    sti ; Libera interrupções para não travar a CPU
    hlt ;pausa a CPU até a próxima interrupção 
    jmp .halt  ; Entra em um loop infinito


;*******************************************************************;
;                      Substitui os bytes não usados por 0                 ;
;*******************************************************************;

times (SIZE-2)-($-$$) db ZERO ; Calcula os bytes que o bootloader usou e zera o restante (512-2)=510 | 510-(endereço atual-endereço do segmento (ORG)) 
dw BOOT_SIGN ; Assinatura dos 2 últimos bytes 