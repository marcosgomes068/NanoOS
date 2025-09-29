CC = gcc
AS = as
LD = ld
CFLAGS = -m32 -ffreestanding -nostdlib -fno-pie -O2 -Wall -Wextra
LDFLAGS = -m elf_i386 -T linker.ld

OBJS = boot.o kernel.o gdt_flush.o

all: kernel.bin

kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

boot.o: boot.s
	$(AS) --32 $< -o $@

gdt_flush.o: gdt_flush.s
	$(AS) --32 $< -o $@

clean:
	rm -f *.o kernel.bin

run: kernel.bin
	qemu-system-i386 -kernel kernel.bin

.PHONY: all clean run
