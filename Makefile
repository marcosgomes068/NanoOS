# Compiladores e ferramentas
CC = gcc          # Compilador C
AS = as           # Assembler
LD = ld           # Linker

# Diretórios
SRC_DIR = src
BUILD_DIR = build
BOOT_DIR = $(SRC_DIR)/boot
KERNEL_DIR = $(SRC_DIR)/kernel
COMMANDS_DIR = $(SRC_DIR)/commands
INCLUDE_DIR = include

# Flags para compilação 32-bit independente
CFLAGS = -m32 -ffreestanding -nostdlib -fno-pie -O2 -Wall -Wextra -masm=intel -I$(INCLUDE_DIR)
# -m32: compila para 32-bit
# -ffreestanding: não assume bibliotecas padrão
# -nostdlib: não linka bibliotecas padrão
# -fno-pie: desabilita Position Independent Executable
# -O2: otimização nível 2
# -Wall -Wextra: warnings extras
# -masm=intel: usa sintaxe Intel para assembly inline
# -I$(INCLUDE_DIR): adiciona diretório de headers

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
OBJS = $(BUILD_DIR)/boot.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/gdt_flush.o $(BUILD_DIR)/interrupts.o $(BUILD_DIR)/commands.o

# Target padrão: compila o kernel
all: $(BUILD_DIR) kernel.bin

# Cria diretório build se não existir
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Linka todos os objetos em um binário final
kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

# Compila o código C do kernel
$(BUILD_DIR)/kernel.o: $(KERNEL_DIR)/kernel.c $(INCLUDE_DIR)/kernel.h
	$(CC) $(CFLAGS) -c $< -o $@

# Compila o sistema de comandos
$(BUILD_DIR)/commands.o: $(COMMANDS_DIR)/commands.c $(INCLUDE_DIR)/commands.h
	$(CC) $(CFLAGS) -c $< -o $@

# Compila o sistema de arquivos
$(BUILD_DIR)/filesystem.o: $(SRC_DIR)/filesystem/filesystem.c $(INCLUDE_DIR)/filesystem.h
	$(CC) $(CFLAGS) -c $< -o $@

# Compila o driver de disco
$(BUILD_DIR)/disk.o: $(SRC_DIR)/filesystem/disk.c $(INCLUDE_DIR)/disk.h
	$(CC) $(CFLAGS) -c $< -o $@

# Compila o bootstrap em assembly
$(BUILD_DIR)/boot.o: $(BOOT_DIR)/boot.s
	$(AS) $(ASFLAGS) $< -o $@

# Compila o flush da GDT em assembly  
$(BUILD_DIR)/gdt_flush.o: $(BOOT_DIR)/gdt_flush.s
	$(AS) $(ASFLAGS) $< -o $@

# Compila os handlers de interrupção em assembly
$(BUILD_DIR)/interrupts.o: $(BOOT_DIR)/interrupts.s
	$(AS) $(ASFLAGS) $< -o $@

# Remove arquivos compilados
clean:
	rm -rf $(BUILD_DIR) kernel.bin

# Executa o kernel no QEMU
run: kernel.bin
	qemu-system-i386 -kernel kernel.bin

# Targets que não geram arquivos
.PHONY: all clean run
