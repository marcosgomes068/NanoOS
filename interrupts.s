# Handlers de interrupção para NanoOS
# Este arquivo contém os stubs assembly que chamam as funções C

.section .text

# Carrega a IDT no processador
.global idt_load
.type idt_load, @function
idt_load:
    mov 4(%esp), %eax       # Pega o ponteiro para IDT da pilha
    lidt (%eax)             # Carrega a IDT
    ret                     # Retorna

# Handler de interrupção do timer (IRQ 0)
.global irq0_handler
.type irq0_handler, @function
irq0_handler:
    pusha                   # Salva todos os registradores
    call timer_handler      # Chama a função C do timer
    popa                    # Restaura todos os registradores
    iret                    # Retorna da interrupção

# Handler de interrupção do teclado (IRQ 1)
.global irq1_handler
.type irq1_handler, @function
irq1_handler:
    pusha                   # Salva todos os registradores
    call keyboard_handler   # Chama a função C do teclado
    popa                    # Restaura todos os registradores
    iret                    # Retorna da interrupção