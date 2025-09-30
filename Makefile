# Compiladores e ferramentas
CC = gcc          # Compilador C
AS = as           # Assembler
LD = ld           # Linker

# Flags para compilação 32-bit independente
CFLAGS = -m32 -ffreestanding -nostdlib -fno-pie -O2 -Wall -Wextra -masm=intel
# -m32: compila para 32-bit
# -ffreestanding: não assume bibliotecas padrão
# -nostdlib: não linka bibliotecas padrão
# -fno-pie: desabilita Position Independent Executable
# -O2: otimização nível 2
# -Wall -Wextra: warnings extras
# -masm=intel: usa sintaxe Intel para assembly inline

# Flags do assembler para sintaxe Intel
ASFLAGS = --32 -msyntax=intel -mnaked-reg
# --32: modo 32-bit
# -msyntax=intel: usa sintaxe Intel ao invés de AT&T
# -mnaked-reg: permite registros sem % (sintaxe Intel)

# Flags do linker
LDFLAGS = -m elf_i386 -T linker.ld
# -m elf_i386: formato ELF 32-bit
# -T linker.ld: usa script de linker customizado

# Lista de arquivos objeto a serem compilados
OBJS = boot.o kernel.o gdt_flush.o interrupts.o

# Target padrão: compila o kernel
all: kernel.bin

# Linka todos os objetos em um binário final
kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

# Compila o código C do kernel
kernel.o: kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compila o bootstrap em assembly
boot.o: boot.s
	$(AS) $(ASFLAGS) $< -o $@

# Compila o flush da GDT em assembly  
gdt_flush.o: gdt_flush.s
	$(AS) $(ASFLAGS) $< -o $@

# Compila os handlers de interrupção em assembly
interrupts.o: interrupts.s
	$(AS) $(ASFLAGS) $< -o $@

# Remove arquivos compilados
clean:
	rm -f *.o kernel.bin

# Executa o kernel no QEMU
run: kernel.bin
	qemu-system-i386 -kernel kernel.bin

# Targets que não geram arquivos
.PHONY: all clean run
