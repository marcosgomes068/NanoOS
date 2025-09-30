# Função para carregar e ativar a GDT (Global Descriptor Table)
# A GDT define os segmentos de memória e seus privilégios

.global gdt_flush              # Torna a função visível para o código C
.type gdt_flush, @function     # Define como função
gdt_flush:
    mov eax, [esp + 4]         # Pega o ponteiro para GDT da pilha (1º parâmetro)
    lgdt [eax]                 # Carrega a GDT no processador
    
    # Atualiza todos os registradores de segmento para o novo segmento de dados
    mov ax, 0x10               # 0x10 = seletor do segmento de dados na GDT
    mov ds, ax                 # Segmento de dados
    mov es, ax                 # Segmento extra
    mov fs, ax                 # Segmento F
    mov gs, ax                 # Segmento G
    mov ss, ax                 # Segmento de pilha
    
    # Far jump para atualizar o segmento de código (CS)
    ljmp 0x08, offset .flush  # 0x08 = seletor do segmento de código na GDT
.flush:
    ret                        # Retorna para o código C

.section .note.GNU-stack,"",@progbits
