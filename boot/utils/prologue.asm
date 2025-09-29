prologue: 
    push bp ; Guarda o base pointer antigo na stack
    mov bp, sp ; Copia o stack pointer para base pointer (cria o stack frame)
    pusha ; Salva o estado de todos os registradores gerais
    ret