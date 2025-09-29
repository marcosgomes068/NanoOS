# Handlers de interrupção para o NanoOS

.section .text

# Função para carregar a IDT
.global idt_load
.type idt_load, @function
idt_load:
    mov 4(%esp), %eax        # Pega o ponteiro para IDT da pilha
    lidt (%eax)              # Carrega a IDT no processador
    ret                      # Retorna

# Handler de interrupção do teclado
.global keyboard_interrupt_handler
.type keyboard_interrupt_handler, @function
keyboard_interrupt_handler:
    pusha                    # Salva todos os registradores
    pushf                    # Salva as flags
    
    call keyboard_handler    # Chama a função C do handler
    
    popf                     # Restaura as flags
    popa                     # Restaura todos os registradores
    iret                     # Retorna da interrupção