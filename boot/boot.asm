%DEFINE BASE 0x7C00 ; Inicio do bootloader
%DEFINE SEGMENT_END 0x7E00 ; Fim do bootloader (512 bytes)
%DEFINE VIDEO_FUNC 0x10 ; Funções de vídeo da bios
%DEFINE SIZE 512 ; Tamanho do bootloader
%DEFINE BOOT_SIGN 00AA55h ; assinatura do Bootloader
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
    mov ax, SEGMENT_END ; copia o fim do bl para ax
    mov ss, ax ; seta a stack como fim do bl (512 bytes)
    mov sp, 0x2000 ; Topo da stack iniciando em 2000 e terminando em0
    cld ; Limpa a direction flag 
    cli ; Ignora interrupções da externas


;******************************************************************;
;                       Limpa a tela padrão da BIOS                          ;
;******************************************************************;

.clear_screen:
    push bp ; Salva o base pointer na stack
    mov bp, sp ; Base pointer aponta para a stack (Cria um stackframe)
    pusha ; Salva todos os registradores no stack frame 

    mov ah, 0x06 ; função de scrool down da bios
    mov al, ZERO ; Quantas linhas limpar (0x0 = clear)
    mov bh, 0x17 ; Background color e color (1 = backgroun azul | 7 = color cinza claro)
    mov cx, ZERO ; 
    mov dh, 0x18 ; linhas de no máximo 24 caracteres
    mov dl, 0x4f ; colunas de até 79 caracteres
    int VIDEO_FUNC ; Chama a interrupção de vídeo da bios

    popa ; Recupera os registradores do início da função
    mov sp, bp ; Restaura o stack pointer para o topo do stack frame
    pop bp ; Recupera o Base pointer  global

;*******************************************************************;
;            Define o contador e o ponteiro da mensagem           ;
;*******************************************************************;

start:
    ; call init_state ; Inicializa registros 
    ; call .clear_screen
    mov si, msg  ; aponta para o início da msg (byte do "H")
    mov cx, MSG_LEN ; cx = msg len
    call write_char

;*******************************************************************;
;                 Itera sobre o ponteiro e escreve na tela                ;
;*******************************************************************;

%include "../utils/msg/write.asm"

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