# Hardware - Documentação

## Recursos de Hardware Utilizados

### Terminal VGA
- **Endereço**: 0xB8000 (memória VGA)
- **Formato**: Text mode 80x25
- **Estrutura**: 2 bytes por caractere (ASCII + atributo cor)
- **Cores**: 4 bits fundo + 4 bits frente

### Teclado PS/2
- **Porta de dados**: 0x60
- **Porta de status**: 0x64  
- **IRQ**: 1
- **Protocolo**: Scancode Set 1
- **Mapeamento**: Layout US QWERTY

### Timer (PIT - Programmable Interval Timer)
- **Porta de comando**: 0x43
- **Porta de dados**: 0x40
- **Frequência base**: 1.193.180 Hz
- **Configuração**: 100 Hz (10ms por tick)
- **IRQ**: 0

### PIC (Programmable Interrupt Controller)
- **PIC1**: 0x20 (comando), 0x21 (dados)
- **PIC2**: 0xA0 (comando), 0xA1 (dados)  
- **Remapeamento**: IRQs 0-15 para INT 0x20-0x2F
- **Configuração**: Modo cascata

### Processador x86
- **Arquitetura**: 32 bits (i386)
- **Segmentação**: GDT com 3 entradas
- **Interrupções**: IDT com 256 entradas
- **Modo**: Kernel mode (Ring 0)

## Configurações de Hardware

### GDT (Global Descriptor Table)
1. **Entrada 0**: NULL (obrigatório)
2. **Entrada 1**: Segmento código (0-4GB, Ring 0)
3. **Entrada 2**: Segmento dados (0-4GB, Ring 0)

### IDT (Interrupt Descriptor Table)
- **Timer**: INT 0x20 (IRQ 0)
- **Teclado**: INT 0x21 (IRQ 1)
- **Outras**: Não utilizadas (zeradas)

### Memória
- **Stack**: Configurado pelo bootloader
- **Heap**: Não implementado (alocação estática)
- **VGA**: Mapeamento direto para 0xB8000

## Limitações Atuais
- Apenas modo texto VGA
- Sem suporte a mouse
- Sem gerenciamento de memória dinâmica
- Apenas layout de teclado US
- Sem suporte a multitasking