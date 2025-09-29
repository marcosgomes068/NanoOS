NanoOS - Mini Kernel 32-bit

Um kernel minimalista em 32-bit x86.

Recursos

Bootloader Multiboot (compatível com GRUB)

Driver VGA em modo texto

GDT (Global Descriptor Table)

Mini terminal interativo

Estrutura modular para expansão

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