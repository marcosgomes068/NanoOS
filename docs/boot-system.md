# Boot System - Documentação

## Arquivos

- `boot.s` - Bootloader e ponto de entrada
- `gdt_flush.s` - Carregamento da GDT
- `interrupts.s` - Handlers de interrupção em assembly

## boot.s

### Função
Ponto de entrada principal do kernel. Configura o ambiente inicial e transfere controle para o kernel em C.

### Responsabilidades
- Configuração do stack inicial
- Preparação do ambiente de execução
- Chamada da função kernel_main()

### Seções Importantes
- `.multiboot` - Header para bootloaders compatíveis
- `.text` - Código executável
- `.bss` - Dados não inicializados

## gdt_flush.s

### Função
Carrega a Global Descriptor Table (GDT) no processador.

### Operação
1. Recebe ponteiro para estrutura GDT
2. Executa instrução LGDT
3. Recarrega segmentos de código e dados
4. Atualiza registradores de segmento

## interrupts.s

### Função
Implementa handlers de interrupção em assembly de baixo nível.

### Handlers Implementados
- `irq0_handler` - Timer (IRQ 0)
- `irq1_handler` - Teclado (IRQ 1)

### Fluxo de Execução
1. Salva estado dos registradores
2. Chama função handler em C
3. Restaura estado dos registradores
4. Executa IRET para retornar