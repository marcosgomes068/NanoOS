#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>

// ============================================================================
// CONFIGURAÇÕES DE HARDWARE
// ============================================================================

// Configurações do terminal VGA
#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// Configurações do teclado
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_IRQ 1

// Portas para PIC
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

// Configurações do Timer (PIT)
#define TIMER_COMMAND_PORT 0x43
#define TIMER_DATA_PORT 0x40
#define TIMER_IRQ 0
#define TIMER_FREQUENCY 100
#define PIT_BASE_FREQUENCY 1193180

// IDT
#define IDT_SIZE 256

// ============================================================================
// ESTRUTURAS DE DADOS
// ============================================================================

// Estrutura VGA
typedef struct {
    char character;
    uint8_t color;
} __attribute__((packed)) vga_char;

// Estruturas GDT
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

// Estruturas IDT
typedef struct {
    uint16_t base_low;
    uint16_t sel;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed)) idt_entry;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr;

// ============================================================================
// DECLARAÇÕES DE FUNÇÕES
// ============================================================================

// Funções de I/O
uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t value);

// Funções do terminal
void terminal_init(void);
void terminal_print(const char* str);

// Funções de sistema
void timer_init(void);
void keyboard_init(void);
void idt_init(void);

// Funções de tratamento
void handle_keypress(uint8_t scancode);
void timer_handler(void);
void keyboard_handler(void);

// Funções auxiliares
size_t strlen(const char* str);
int strcmp(const char* s1, const char* s2);
void uint_to_str(uint32_t num, char* buffer, size_t buffer_size);

// Função principal do kernel
void kernel_main(void);

// Variáveis globais externas
extern volatile uint32_t timer_ticks;

#endif // KERNEL_H