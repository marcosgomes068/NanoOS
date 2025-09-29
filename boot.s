.set ALIGN,    1<<0             // Alinha módulos carregados em 4KB
.set MEMINFO,  1<<1             // Fornece informações de memória
.set FLAGS,    ALIGN | MEMINFO  // Combina as flags acima
.set MAGIC,    0x1BADB002       // Número mágico do Multiboot
.set CHECKSUM, -(MAGIC + FLAGS) // Checksum para validar o header

// Seção do header Multiboot - deve estar no início do kernel
.section .multiboot
.align 4                        // Alinha em 4 bytes
.long MAGIC                     // Coloca o número mágico
.long FLAGS                     // Coloca as flags
.long CHECKSUM                  // Coloca o checksum

// Seção BSS - área de dados não inicializados
.section .bss
.align 16                       // Alinha a stack em 16 bytes
stack_bottom:                   // Início da stack
.skip 16384                     // Reserva 16KB para a stack (16 * 1024 bytes)
stack_top:                      // Topo da stack (cresce para baixo)

// Seção de código executável
.section .text
.global _start                  // Torna _start visível para o linker
.type _start, @function         // Define _start como uma função
_start:
    mov $stack_top, %esp        // Configura o ponteiro da stack (ESP)
    call kernel_main            // Chama a função principal do kernel em C
    cli                         // Desabilita interrupções (Clear Interrupt)
1:  hlt                         // Para o processador (Halt)
    jmp 1b                      // Loop infinito caso o processador acorde

.size _start, . - _start        // Define o tamanho da função _start
