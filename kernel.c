#include <stdint.h>
#include <stddef.h>

#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

typedef struct {
    char character;
    uint8_t color;
} vga_char;

static vga_char* vga_buffer = (vga_char*)VGA_MEMORY;
static size_t terminal_row = 0;
static size_t terminal_col = 0;
static uint8_t terminal_color = 0x0F;

char input_buffer[256];
size_t input_pos = 0;

void terminal_init(void);
void terminal_putchar(char c);
void terminal_print(const char* str);
void gdt_init(void);
void idt_init(void);
void keyboard_init(void);
void process_command(const char* cmd);

void terminal_init(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index].character = ' ';
            vga_buffer[index].color = terminal_color;
        }
    }
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_col = 0;
        terminal_row++;
    } else if (c == '\b') {
        if (terminal_col > 0) {
            terminal_col--;
            const size_t index = terminal_row * VGA_WIDTH + terminal_col;
            vga_buffer[index].character = ' ';
            vga_buffer[index].color = terminal_color;
        }
    } else {
        const size_t index = terminal_row * VGA_WIDTH + terminal_col;
        vga_buffer[index].character = c;
        vga_buffer[index].color = terminal_color;
        terminal_col++;
        if (terminal_col >= VGA_WIDTH) {
            terminal_col = 0;
            terminal_row++;
        }
    }
    if (terminal_row >= VGA_HEIGHT) {
        terminal_row = 0;
    }
}

void terminal_print(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        terminal_putchar(str[i]);
    }
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void process_command(const char* cmd) {
    if (strcmp(cmd, "help") == 0) {
        terminal_print("\nComandos disponiveis:\n");
        terminal_print("  help  - Mostra esta ajuda\n");
        terminal_print("  clear - Limpa a tela\n");
        terminal_print("  about - Informacoes do kernel\n");
    } else if (strcmp(cmd, "clear") == 0) {
        terminal_init();
        terminal_row = 0;
        terminal_col = 0;
    } else if (strcmp(cmd, "about") == 0) {
        terminal_print("\nNanoOS v0.1\n");
        terminal_print("Mini kernel 32-bit educacional\n");
    } else if (strlen(cmd) > 0) {
        terminal_print("\nComando desconhecido: ");
        terminal_print(cmd);
        terminal_print("\n");
    }
}

// GDT
typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) gdt_entry;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr;

gdt_entry gdt[3];
gdt_ptr gp;

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;
    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access = access;
}

extern void gdt_flush(uint32_t);

void gdt_init(void) {
    gp.limit = (sizeof(gdt_entry) * 3) - 1;
    gp.base = (uint32_t)&gdt;
    
    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    
    gdt_flush((uint32_t)&gp);
}

// IDT Stub (basico)
void idt_init(void) {
    // TODO: Implementar IDT completo
}

void keyboard_init(void) {
    // TODO: Implementar teclado
}

void kernel_main(void) {
    terminal_init();
    gdt_init();
    idt_init();
    
    terminal_print("Bem-vindo ao NanoOS!\n\n");
    terminal_print("Digite 'help' para ver comandos\n\n");
    terminal_print("> ");
    
    while(1) {
        __asm__ volatile ("hlt");
    }
}
