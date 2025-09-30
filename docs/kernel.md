# Kernel - Documentação

## Arquivo Principal
`src/kernel/kernel.c` - Core do sistema operacional

## Principais Componentes

### Terminal VGA
- **Localização**: Memória VGA em 0xB8000
- **Resolução**: 80x25 caracteres
- **Cores**: 4 bits fundo + 4 bits texto
- **Funções**: `terminal_init()`, `terminal_putchar()`, `terminal_print()`

### Sistema de Teclado
- **Porta**: 0x60 (dados) e 0x64 (status)
- **Mapeamento**: Scancode para ASCII
- **Layout**: Teclado US padrão
- **Funcionalidades**: Enter, Backspace, caracteres alfanuméricos

### Timer (PIT)
- **Frequência**: 100 Hz (10ms por tick)
- **Porta de comando**: 0x43
- **Porta de dados**: 0x40
- **Uso**: Medição de tempo de sistema

### GDT (Global Descriptor Table)
- **Entradas**: NULL, Código (Ring 0), Dados (Ring 0)
- **Tamanho**: Cobertura completa de 4GB
- **Função**: Segmentação de memória

### IDT (Interrupt Descriptor Table)
- **Tamanho**: 256 entradas
- **IRQs configuradas**: Timer (0x20), Teclado (0x21)
- **PIC**: Remapeado para evitar conflitos

### Funções I/O
- `inb(port)` - Lê byte de porta
- `outb(port, value)` - Escreve byte em porta
- Implementadas com inline assembly Intel

### Handlers de Interrupção
- `timer_handler()` - Incrementa contador de ticks
- `keyboard_handler()` - Processa entrada de teclado
- `handle_keypress()` - Converte scancode e processa comandos

### Loop Principal
- Executa `hlt` para economizar energia
- Desperta apenas com interrupções
- Mantém sistema responsivo