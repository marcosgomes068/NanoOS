// ============================================================================
// NanoOS v0.2 - Mini Kernel 32-bit
// Kernel básico com suporte a terminal VGA, teclado e timer
// ============================================================================

// Includes necessários para tipos de dados
#include <stdint.h>
#include <stddef.h>

// ============================================================================
// CONFIGURAÇÕES DE HARDWARE
// ============================================================================

// Configurações do terminal VGA
#define VGA_MEMORY 0xB8000    // Endereço da memória VGA
#define VGA_WIDTH 80          // Largura em caracteres
#define VGA_HEIGHT 25         // Altura em caracteres

// Configurações do teclado
#define KEYBOARD_DATA_PORT 0x60     // Porta de dados do teclado
#define KEYBOARD_STATUS_PORT 0x64   // Porta de status do teclado
#define KEYBOARD_IRQ 1              // IRQ do teclado

// Portas para PIC (Programmable Interrupt Controller)
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

// Configurações do Timer (PIT)
#define TIMER_COMMAND_PORT 0x43
#define TIMER_DATA_PORT 0x40
#define TIMER_IRQ 0
#define TIMER_FREQUENCY 100         // 100 Hz (10ms por tick)
#define PIT_BASE_FREQUENCY 1193180  // Frequência base do PIT

// IDT (Interrupt Descriptor Table)
#define IDT_SIZE 256

// ============================================================================
// ESTRUTURAS DE DADOS
// ============================================================================

// Estrutura para um caractere na tela VGA
typedef struct {
    char character;           // O caractere ASCII
    uint8_t color;           // Cor (4 bits fundo + 4 bits frente)
} __attribute__((packed)) vga_char;

// Entrada da GDT (Global Descriptor Table) - 8 bytes
typedef struct {
    uint16_t limit_low;      // Limite inferior (bits 0-15)
    uint16_t base_low;       // Base inferior (bits 0-15) 
    uint8_t base_middle;     // Base média (bits 16-23)
    uint8_t access;          // Flags de acesso (presente, privilégio, tipo)
    uint8_t granularity;     // Granularidade e limite superior
    uint8_t base_high;       // Base superior (bits 24-31)
} __attribute__((packed)) gdt_entry;

// Ponteiro para a GDT
typedef struct {
    uint16_t limit;          // Tamanho da GDT - 1
    uint32_t base;           // Endereço base da GDT
} __attribute__((packed)) gdt_ptr;

// Entrada da IDT (Interrupt Descriptor Table) - 8 bytes
typedef struct {
    uint16_t base_low;        // Bits 0-15 do endereço do handler
    uint16_t sel;             // Seletor de segmento de código
    uint8_t always0;          // Sempre 0
    uint8_t flags;            // Flags (presente, DPL, tipo)
    uint16_t base_high;       // Bits 16-31 do endereço do handler
} __attribute__((packed)) idt_entry;

// Ponteiro para a IDT
typedef struct {
    uint16_t limit;           // Tamanho da IDT - 1
    uint32_t base;            // Endereço base da IDT
} __attribute__((packed)) idt_ptr;

// ============================================================================
// VARIÁVEIS GLOBAIS
// ============================================================================

// Terminal VGA
static vga_char* vga_buffer = (vga_char*)VGA_MEMORY;  // Ponteiro para memória VGA
static size_t terminal_row = 0;                       // Linha atual do cursor
static size_t terminal_col = 0;                       // Coluna atual do cursor  
static uint8_t terminal_color = 0x0F;                 // Cor padrão (branco no preto)

// Teclado e entrada
static char input_buffer[256];        // Buffer para comandos digitados
static size_t input_pos = 0;         // Posição atual no buffer

// Timer
static volatile uint32_t timer_ticks = 0;     // Contador do timer (volatile para ISR)

// GDT e IDT
static gdt_entry gdt[3];             // Array da GDT: NULL, Código, Dados
static gdt_ptr gp;                   // Ponteiro para a GDT
static idt_entry idt_table[IDT_SIZE]; // Tabela IDT
static idt_ptr idtp;                 // Ponteiro para a IDT

// ============================================================================
// MAPA DE TECLADO US (Scancode para ASCII)
// ============================================================================

static const char keyboard_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',    // 0-9
    '9', '0', '-', '=', '\b',                          // 10-14
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', // 15-24
    'p', '[', ']', '\n',                               // 25-28
    0,                                                 // 29 - Ctrl
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', // 30-39
    '\'', '`', 0,                                      // 40-42 (42 = Shift)
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',               // 43-49
    'm', ',', '.', '/', 0,                             // 50-54 (54 = Shift)
    '*',                                               // 55
    0,                                                 // 56 - Alt
    ' ',                                               // 57 - Espaço
    0,                                                 // 58 - Caps lock
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                     // 59-68 - F1 a F10
    0,                                                 // 69 - Num lock
    0,                                                 // 70 - Scroll Lock
    0,                                                 // 71 - Home
    0,                                                 // 72 - Up Arrow
    0,                                                 // 73 - Page Up
    '-',                                               // 74
    0,                                                 // 75 - Left Arrow
    0,                                                 // 76
    0,                                                 // 77 - Right Arrow
    '+',                                               // 78
    0,                                                 // 79 - End
    0,                                                 // 80 - Down Arrow
    0,                                                 // 81 - Page Down
    0,                                                 // 82 - Insert
    0,                                                 // 83 - Delete
    0, 0, 0,                                           // 84-86
    0,                                                 // 87 - F11
    0,                                                 // 88 - F12
    0                                                  // 89+ - Indefinido
};

// ============================================================================
// FUNÇÕES DE E/S DE PORTAS (I/O)
// ============================================================================

// Lê um byte de uma porta de I/O
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Escreve um byte em uma porta de I/O
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

// ============================================================================
// FUNÇÕES AUXILIARES DE STRING
// ============================================================================

// Calcula o comprimento de uma string
size_t strlen(const char* str) {
    if (!str) return 0;  // Proteção contra ponteiro nulo
    
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

// Compara duas strings (retorna 0 se iguais)
int strcmp(const char* s1, const char* s2) {
    if (!s1 || !s2) return (s1 == s2) ? 0 : (s1 ? 1 : -1);  // Proteção
    
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// Converte número para string (helper interno)
static void uint_to_str(uint32_t num, char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 2) return;  // Proteção
    
    int i = 0;
    
    // Caso especial: zero
    if (num == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    
    // Extrai dígitos (ordem reversa)
    while (num > 0 && i < (int)buffer_size - 1) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    // Inverte a string
    for (int j = 0; j < i / 2; j++) {
        char swap = buffer[j];
        buffer[j] = buffer[i - 1 - j];
        buffer[i - 1 - j] = swap;
    }
    
    buffer[i] = '\0';
}

// ============================================================================
// FUNÇÕES DO TERMINAL VGA
// ============================================================================

// Inicializa o terminal VGA limpando a tela
void terminal_init(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index].character = ' ';
            vga_buffer[index].color = terminal_color;
        }
    }
    terminal_row = 0;
    terminal_col = 0;
}

// Escreve um caractere na posição atual do cursor
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
    
    // Wrap vertical (volta ao topo quando chega no fim)
    if (terminal_row >= VGA_HEIGHT) {
        terminal_row = 0;
    }
}

// Escreve uma string completa na tela
void terminal_print(const char* str) {
    if (!str) return;  // Proteção contra ponteiro nulo
    
    for (size_t i = 0; str[i] != '\0'; i++) {
        terminal_putchar(str[i]);
    }
}

// ============================================================================
// FUNÇÕES DO PIC (PROGRAMMABLE INTERRUPT CONTROLLER)
// ============================================================================

// Reconfigura o PIC para evitar conflitos com exceções da CPU
void pic_remap(uint8_t offset1, uint8_t offset2) {
    uint8_t a1 = inb(PIC1_DATA);  // Salva máscaras atuais
    uint8_t a2 = inb(PIC2_DATA);
    
    // Inicia sequência de inicialização em modo cascata
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);
    
    // Define vetores de offset
    outb(PIC1_DATA, offset1);     // PIC1 offset
    outb(PIC2_DATA, offset2);     // PIC2 offset
    
    // Configura cascata (PIC2 conectado ao IRQ2 do PIC1)
    outb(PIC1_DATA, 4);
    outb(PIC2_DATA, 2);
    
    // Modo 8086/88 (MCS-80/85)
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    
    // Restaura máscaras
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

// Envia sinal de End of Interrupt (EOI) para o PIC
void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, 0x20);  // EOI para PIC2 (slave)
    }
    outb(PIC1_COMMAND, 0x20);      // EOI para PIC1 (master)
}

// ============================================================================
// FUNÇÕES DA GDT (GLOBAL DESCRIPTOR TABLE)
// ============================================================================

// Configura uma entrada específica na GDT
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    if (num < 0 || num >= 3) return;  // Proteção de índice
    
    // Configura endereço base (32 bits divididos em 3 campos)
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;
    
    // Configura limite (20 bits divididos em 2 campos)
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;
    gdt[num].granularity |= gran & 0xF0;
    
    // Configura flags de acesso
    gdt[num].access = access;
}

// Função externa em assembly para carregar a GDT
extern void gdt_flush(uint32_t);

// Inicializa a GDT com segmentos básicos
void gdt_init(void) {
    // Configura o ponteiro da GDT
    gp.limit = (sizeof(gdt_entry) * 3) - 1;
    gp.base = (uint32_t)&gdt;
    
    // Entrada 0: NULL (obrigatória pela arquitetura x86)
    gdt_set_gate(0, 0, 0, 0, 0);
    
    // Entrada 1: Segmento de código (4GB, executável, ring 0)
    // Access: 0x9A = 10011010b = Present, Ring 0, Code, Executable, Readable
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    
    // Entrada 2: Segmento de dados (4GB, leitura/escrita, ring 0)
    // Access: 0x92 = 10010010b = Present, Ring 0, Data, Writable
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    
    // Carrega a GDT no processador
    gdt_flush((uint32_t)&gp);
}

// ============================================================================
// FUNÇÕES DA IDT (INTERRUPT DESCRIPTOR TABLE)
// ============================================================================

// Configura uma entrada na IDT
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_table[num].base_low = base & 0xFFFF;
    idt_table[num].base_high = (base >> 16) & 0xFFFF;
    idt_table[num].sel = sel;
    idt_table[num].always0 = 0;
    idt_table[num].flags = flags;
}

// Função externa em assembly para carregar a IDT
extern void idt_load(uint32_t);

// Handlers externos de interrupção (definidos em assembly)
extern void irq0_handler(void);  // Timer
extern void irq1_handler(void);  // Teclado

// Inicializa a IDT completa
void idt_init(void) {
    // Configura o ponteiro da IDT
    idtp.limit = (sizeof(idt_entry) * IDT_SIZE) - 1;
    idtp.base = (uint32_t)&idt_table;
    
    // Limpa toda a IDT
    for (int i = 0; i < IDT_SIZE; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    
    // Remapeia o PIC (IRQs 0-15 → INT 0x20-0x2F)
    pic_remap(0x20, 0x28);
    
    // Configura handlers de IRQs
    // Flags 0x8E = 10001110b = Present, Ring 0, 32-bit Interrupt Gate
    idt_set_gate(0x20, (uint32_t)irq0_handler, 0x08, 0x8E);  // Timer (IRQ 0)
    idt_set_gate(0x21, (uint32_t)irq1_handler, 0x08, 0x8E);  // Teclado (IRQ 1)
    
    // Habilita IRQ 0 (timer) e IRQ 1 (teclado) no PIC
    // 0xFC = 11111100b (apenas bits 0 e 1 em 0 = habilitados)
    outb(PIC1_DATA, 0xFC);
    outb(PIC2_DATA, 0xFF);  // Desabilita todas as IRQs do PIC2
    
    // Carrega a IDT no processador
    idt_load((uint32_t)&idtp);
    
    // Habilita interrupções globalmente
    __asm__ volatile ("sti");
}

// ============================================================================
// HANDLERS DE INTERRUPÇÃO (ISR)
// ============================================================================

// Handler do timer (IRQ 0) - chamado 100 vezes por segundo
void timer_handler(void) {
    timer_ticks++;
    pic_send_eoi(0);
}

// Handler do teclado (IRQ 1) - chamado quando uma tecla é pressionada/solta
void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    handle_keypress(scancode);
    pic_send_eoi(1);
}

// ============================================================================
// FUNÇÕES DO TECLADO
// ============================================================================

// Processa uma tecla pressionada
void handle_keypress(uint8_t scancode) {
    // Ignora teclas liberadas (bit 7 = 1)
    if (scancode & 0x80) return;
    
    // Converte scancode para ASCII
    char key = keyboard_map[scancode];
    if (key == 0) return;  // Tecla não mapeada
    
    if (key == '\n') {
        // Enter - processa comando
        terminal_putchar('\n');
        
        if (input_pos < sizeof(input_buffer)) {
            input_buffer[input_pos] = '\0';
        } else {
            input_buffer[sizeof(input_buffer) - 1] = '\0';
        }
        
        process_command(input_buffer);
        input_pos = 0;
        terminal_print("> ");
    } else if (key == '\b') {
        // Backspace
        if (input_pos > 0) {
            input_pos--;
            terminal_putchar('\b');
        }
    } else if (input_pos < sizeof(input_buffer) - 1) {
        // Caractere normal
        input_buffer[input_pos++] = key;
        terminal_putchar(key);
    }
}

// ============================================================================
// PROCESSAMENTO DE COMANDOS
// ============================================================================

// Processa comandos digitados pelo usuário
void process_command(const char* cmd) {
    if (!cmd) return;  // Proteção
    
    if (strcmp(cmd, "help") == 0) {
        terminal_print("\nComandos disponiveis:\n");
        terminal_print("  help   - Mostra esta ajuda\n");
        terminal_print("  clear  - Limpa a tela\n");
        terminal_print("  about  - Informacoes do kernel\n");
        terminal_print("  uptime - Tempo ligado (em ticks)\n");
        terminal_print("  echo   - Repete o texto digitado\n");
        
    } else if (strcmp(cmd, "clear") == 0) {
        terminal_init();
        
    } else if (strcmp(cmd, "about") == 0) {
        terminal_print("\nNanoOS v0.2\n");
        terminal_print("Mini kernel 32-bit com:\n");
        terminal_print("- Teclado funcional\n");
        terminal_print("- Timer/PIT integrado\n");
        terminal_print("- IDT completa\n");
        terminal_print("- Tratamento de interrupcoes\n");
        
    } else if (strcmp(cmd, "uptime") == 0) {
        char buffer[32];
        
        terminal_print("\nTicks desde boot: ");
        uint_to_str(timer_ticks, buffer, sizeof(buffer));
        terminal_print(buffer);
        
        terminal_print(" (aprox. ");
        uint_to_str(timer_ticks / TIMER_FREQUENCY, buffer, sizeof(buffer));
        terminal_print(buffer);
        terminal_print("s)\n");
        
    } else if (strlen(cmd) > 5 && cmd[0] == 'e' && cmd[1] == 'c' && 
               cmd[2] == 'h' && cmd[3] == 'o' && cmd[4] == ' ') {
        // Comando echo - repete o texto após "echo "
        terminal_print("\n");
        terminal_print(cmd + 5);
        terminal_print("\n");
        
    } else if (strlen(cmd) > 0) {
        terminal_print("\nComando desconhecido: ");
        terminal_print(cmd);
        terminal_print("\nDigite 'help' para ver comandos disponiveis\n");
    }
}

// ============================================================================
// INICIALIZAÇÃO DO TIMER
// ============================================================================

// Inicializa o timer (PIT) para gerar interrupções periódicas
void timer_init(void) {
    // Calcula divisor para a frequência desejada
    uint32_t divisor = PIT_BASE_FREQUENCY / TIMER_FREQUENCY;
    
    // Comando: Canal 0, Modo 3 (square wave), Binário
    outb(TIMER_COMMAND_PORT, 0x36);
    
    // Envia divisor (low byte primeiro, depois high byte)
    outb(TIMER_DATA_PORT, divisor & 0xFF);
    outb(TIMER_DATA_PORT, (divisor >> 8) & 0xFF);
}

// Stub de inicialização do teclado (a inicialização real está na idt_init)
void keyboard_init(void) {
    // A configuração real do teclado acontece em idt_init()
    // Esta função existe apenas para compatibilidade
}

// ============================================================================
// FUNÇÃO PRINCIPAL DO KERNEL
// ============================================================================

void kernel_main(void) {
    // Inicialização do sistema em ordem
    terminal_init();     // 1. Inicializa o terminal VGA
    gdt_init();         // 2. Configura a GDT (segmentação)
    timer_init();       // 3. Inicializa o timer (PIT)
    idt_init();         // 4. Configura IDT e habilita interrupções
    keyboard_init();    // 5. Stub de inicialização do teclado
    
    // Mensagem de boas-vindas
    terminal_print("Bem-vindo ao NanoOS!\n\n");
    terminal_print("Digite 'help' para ver comandos\n\n");
    terminal_print("> ");
    
    // Loop principal do kernel
    // O processador fica em halt até receber uma interrupção
    while (1) {
        __asm__ volatile ("hlt");
    }
}