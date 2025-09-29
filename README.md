NanoOS - Mini Kernel 32-bit

Um kernel minimalista em 32-bit x86.

Recursos

Bootloader Multiboot (compatível com GRUB)

Driver VGA em modo texto

GDT (Global Descriptor Table)

Mini terminal interativo

Estrutura modular para expansão

lista do que o NanoOS já tem:

[1] Bootloader Multiboot compatível (GRUB)
[2] Configuração de stack (pilha de 16KB)
[3] GDT (Global Descriptor Table) funcional
[4] Terminal VGA em modo texto
[5] Mini terminal interativo com comandos básicos (help, clear, about)
[6] Estrutura modular separada (boot, GDT, terminal, linker)
[7] Makefile para build e execução no QEMU
[8] Script de linker customizado

Compilar
make

Executar
make run


Requer: gcc, binutils, qemu-system-i386

Comandos do Terminal

help - Ajuda

clear - Limpa a tela

about - Info do kernel

Estrutura
.
├── boot.s
├── kernel.c
├── gdt_flush.s
├── linker.ld
└── Makefile

Próximos Passos

IDT e interrupções

Driver de teclado

Gerenciamento de memória

Multitasking básico