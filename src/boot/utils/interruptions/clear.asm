    push bp ; Salva o base pointer na stack
    mov bp, sp ; Base pointer aponta para a stack (Cria um stackframe)
    pusha ; Salva todos os registradores no stack frame 

    mov ah, 0x06 ; função de scrool down da bios
    mov al, 0x0 ; Quantas linhas limpar (0x0 = clear)
    mov cl, 0x5 ; 
    mov cl, 0x0 ; 
    mov dh, 0x18 ; linhas de no máximo 24 caracteres
    mov dl, 0x4f ; colunas de até 79 caracteres
    int 0x10 ; Chama a interrupção de vídeo da bios

    popa ; Recupera os registradores do início da função
    mov sp, bp ; Restaura o stack pointer para o topo do stack frame
    pop bp ; Recupera o Base pointer  global
