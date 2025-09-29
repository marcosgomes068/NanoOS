// Função para carregar e ativar a GDT (Global Descriptor Table)
// A GDT define os segmentos de memória e seus privilégios

.global gdt_flush              // Torna a função visível para o código C
.type gdt_flush, @function     // Define como função
gdt_flush:
    mov 4(%esp), %eax          // Pega o ponteiro para GDT da pilha (1º parâmetro)
    lgdt (%eax)                // Carrega a GDT no processador
    
    // Atualiza todos os registradores de segmento para o novo segmento de dados
    mov $0x10, %ax             // 0x10 = seletor do segmento de dados na GDT
    mov %ax, %ds               // Segmento de dados
    mov %ax, %es               // Segmento extra
    mov %ax, %fs               // Segmento F
    mov %ax, %gs               // Segmento G
    mov %ax, %ss               // Segmento de pilha
    
    // Far jump para atualizar o segmento de código (CS)
    jmp $0x08, $.flush         // 0x08 = seletor do segmento de código na GDT
.flush:
    ret                        // Retorna para o código C
