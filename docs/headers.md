# Arquivos de Cabeçalho - Documentação

## Localização
`include/` - Diretório de headers do projeto

## Arquivos

### kernel.h
- **Função**: Declarações principais do kernel
- **Conteúdo**: Estruturas de dados, constantes de hardware
- **Uso**: Incluso por módulos que precisam de definições core

### commands.h
- **Função**: Interface do sistema de comandos
- **Declarações**: 
  - Protótipos de funções de comando
  - Declarações de funções auxiliares
  - Variáveis externas (timer_ticks)
  - Constantes (TIMER_FREQUENCY)

## Estruturas Definidas

### Kernel Structures
- `vga_char` - Caractere VGA com cor
- `gdt_entry` - Entrada da Global Descriptor Table
- `gdt_ptr` - Ponteiro para GDT
- `idt_entry` - Entrada da Interrupt Descriptor Table  
- `idt_ptr` - Ponteiro para IDT

### Funções Exportadas

#### Terminal
- `terminal_init()` - Inicialização
- `terminal_putchar()` - Output de caractere
- `terminal_print()` - Output de string

#### Sistema
- `strlen()` - Comprimento de string
- `strcmp()` - Comparação de strings
- `uint_to_str()` - Conversão número para string

#### Comandos
- `process_command()` - Processador principal
- `cmd_*()` - Funções individuais de comando

## Dependências
- Headers utilizam tipos padrão C (stdint.h, stddef.h)
- Declarações extern para variáveis globais
- Macros para constantes de hardware

## Organização
- Separação clara entre interface pública e implementação
- Headers incluem apenas declarações necessárias
- Evita dependências circulares