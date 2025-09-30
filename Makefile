ASM = nasm
QEMU = qemu-system-i386
OUT_DIR = out
SRC_DIR = src/boot

BOOT_BIN   = $(OUT_DIR)/boot.bin
STAGE2_BIN = $(OUT_DIR)/stage_2.bin
DEVICE     = $(OUT_DIR)/device.img

all: run

# Compila o bootloader
$(BOOT_BIN): $(SRC_DIR)/boot.asm
	$(ASM) -f bin $< -o $@ -I $(SRC_DIR)

# Compila o segundo estágio
$(STAGE2_BIN): $(SRC_DIR)/stage_2.asm
	$(ASM) -f bin $< -o $@ -I $(SRC_DIR)

# Cria a imagem final e injeta boot + stage 2
$(DEVICE): $(BOOT_BIN) $(STAGE2_BIN)
	cp $(BOOT_BIN) $(DEVICE)
	dd if=$(STAGE2_BIN) of=$(DEVICE) bs=512 seek=1 conv=notrunc status=none

# Roda no QEMU
run: $(DEVICE)
	$(QEMU) -drive format=raw,file=$(DEVICE)

clean:
	rm -f $(BOOT_BIN) $(STAGE2_BIN) $(DEVICE)
