// Includes necessários para tipos de dados
#include <stdint.h>
#include <stddef.h>

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

// Estrutura para um caractere na tela VGA
typedef struct {
    char character;           // O caractere ASCII
    uint8_t color;           // Cor (4 bits fundo + 4 bits frente)
} vga_char;

// Variáveis globais do terminal
static vga_char* vga_buffer = (vga_char*)VGA_MEMORY;  // Ponteiro para memória VGA
static size_t terminal_row = 0;                       // Linha atual do cursor
static size_t terminal_col = 0;                       // Coluna atual do cursor  
static uint8_t terminal_color = 0x0F;                 // Cor padrão (branco no preto)

// Buffer para entrada de comandos
char input_buffer[256];                               // Buffer de entrada
size_t input_pos = 0;                                // Posição no buffer

// Mapa de teclas US (scancode para ASCII)
static const char keyboard_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

// Declarações das funções
void terminal_init(void);                    // Inicializa o terminal VGA
void terminal_putchar(char c);               // Escreve um caractere na tela
void terminal_print(const char* str);        // Escreve uma string na tela
void gdt_init(void);                         // Inicializa a GDT
void idt_init(void);                         // Inicializa a IDT
void keyboard_init(void);                    // Inicializa o teclado
void process_command(const char* cmd);       // Processa comandos do usuário

// Funções de E/S de portas
uint8_t inb(uint16_t port);                  // Lê um byte de uma porta
void outb(uint16_t port, uint8_t value);     // Escreve um byte em uma porta

// Funções do PIC e teclado
void pic_init(void);                         // Inicializa o PIC
void keyboard_handler(void);                 // Handler de interrupção do teclado
void handle_keypress(uint8_t scancode);      // Processa tecla pressionada

// Inicializa o terminal VGA limpando a tela
void terminal_init(void) {
    // Percorre toda a tela (25 linhas x 80 colunas)
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;  // Calcula posição linear
            vga_buffer[index].character = ' ';        // Espaço em branco
            vga_buffer[index].color = terminal_color; // Cor padrão
        }
    }
}

// Escreve um caractere na posição atual do cursor
void terminal_putchar(char c) {
    if (c == '\n') {                          // Nova linha
        terminal_col = 0;                     // Volta para o início da linha
        terminal_row++;                       // Próxima linha
    } else if (c == '\b') {                   // Backspace
        if (terminal_col > 0) {               // Se não está no início da linha
            terminal_col--;                   // Volta uma posição
            const size_t index = terminal_row * VGA_WIDTH + terminal_col;
            vga_buffer[index].character = ' '; // Apaga o caractere
            vga_buffer[index].color = terminal_color;
        }
    } else {                                  // Caractere normal
        const size_t index = terminal_row * VGA_WIDTH + terminal_col;
        vga_buffer[index].character = c;      // Escreve o caractere
        vga_buffer[index].color = terminal_color;
        terminal_col++;                       // Avança o cursor
        if (terminal_col >= VGA_WIDTH) {      // Se chegou no fim da linha
            terminal_col = 0;                 // Vai para o início
            terminal_row++;                   // Próxima linha
        }
    }
    if (terminal_row >= VGA_HEIGHT) {         // Se passou do fim da tela
        terminal_row = 0;                     // Volta para o topo (wrap)
    }
}

// Escreve uma string completa na tela
void terminal_print(const char* str) {
    // Percorre cada caractere da string até o terminador nulo
    for (size_t i = 0; str[i] != '\0'; i++) {
        terminal_putchar(str[i]);             // Escreve caractere por caractere
    }
}

// Calcula o comprimento de uma string (implementação própria)
size_t strlen(const char* str) {
    size_t len = 0;                          // Contador de caracteres
    while (str[len]) len++;                  // Conta até encontrar '\0'
    return len;                              // Retorna o tamanho
}

// Compara duas strings (implementação própria)
int strcmp(const char* s1, const char* s2) {
    // Percorre as strings enquanto são iguais e não chegam ao fim
    while (*s1 && (*s1 == *s2)) {
        s1++;                                // Próximo caractere de s1
        s2++;                                // Próximo caractere de s2
    }
    // Retorna a diferença entre os caracteres atuais
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// === FUNÇÕES DE E/S DE PORTAS ===

// Lê um byte de uma porta de I/O
uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Escreve um byte em uma porta de I/O
void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

// === FUNÇÕES DO TECLADO ===

// Inicializa o PIC (Programmable Interrupt Controller)
void pic_init(void) {
    // Reinicia ambos os PICs
    outb(PIC1_COMMAND, 0x11);  // Inicia sequência de inicialização do PIC1
    outb(PIC2_COMMAND, 0x11);  // Inicia sequência de inicialização do PIC2
    
    // Define offset das IRQs
    outb(PIC1_DATA, 0x20);     // PIC1 offset: IRQ 0-7 → INT 0x20-0x27
    outb(PIC2_DATA, 0x28);     // PIC2 offset: IRQ 8-15 → INT 0x28-0x2F
    
    // Conecta PIC1 e PIC2
    outb(PIC1_DATA, 4);        // PIC2 está conectado ao IRQ2 do PIC1
    outb(PIC2_DATA, 2);        // Cascata do PIC2
    
    // Modo 8086
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    
    // Desabilita todas as IRQs exceto teclado (IRQ1)
    outb(PIC1_DATA, 0xFD);     // 11111101 - só IRQ1 habilitado
    outb(PIC2_DATA, 0xFF);     // 11111111 - todas desabilitadas
}

// Handler de interrupção do teclado
void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);  // Lê o scancode
    handle_keypress(scancode);                   // Processa a tecla
    outb(PIC1_COMMAND, 0x20);                   // Sinaliza fim da interrupção
}

// Processa uma tecla pressionada
void handle_keypress(uint8_t scancode) {
    // Ignora teclas liberadas (bit 7 = 1)
    if (scancode & 0x80) return;
    
    char key = keyboard_map[scancode];
    if (key == 0) return;  // Tecla não mapeada
    
    if (key == '\n') {                          // Enter - processa comando
        terminal_putchar('\n');
        input_buffer[input_pos] = '\0';         // Termina a string
        process_command(input_buffer);          // Executa o comando
        input_pos = 0;                          // Reseta buffer
        terminal_print("> ");                   // Mostra prompt novamente
    } else if (key == '\b') {                   // Backspace
        if (input_pos > 0) {
            input_pos--;
            terminal_putchar('\b');
        }
    } else if (input_pos < sizeof(input_buffer) - 1) {  // Caractere normal
        input_buffer[input_pos++] = key;
        terminal_putchar(key);
    }
}

// Processa comandos digitados pelo usuário
void process_command(const char* cmd) {
    if (strcmp(cmd, "help") == 0) {          // Comando de ajuda
        terminal_print("\nComandos disponiveis:\n");
        terminal_print("  help  - Mostra esta ajuda\n");
        terminal_print("  clear - Limpa a tela\n");
        terminal_print("  about - Informacoes do kernel\n");
    } else if (strcmp(cmd, "clear") == 0) {  // Comando para limpar tela
        terminal_init();                     // Reinicializa o terminal
        terminal_row = 0;                    // Reseta posição do cursor
        terminal_col = 0;
    } else if (strcmp(cmd, "about") == 0) {  // Informações do sistema
        terminal_print("\nNanoOS v0.1\n");
        terminal_print("Mini kernel 32-bit com teclado funcional\n");
    } else if (strlen(cmd) > 0) {            // Comando desconhecido (não vazio)
        terminal_print("\nComando desconhecido: ");
        terminal_print(cmd);
        terminal_print("\n");
    }
}

// === SEÇÃO GDT (Global Descriptor Table) ===
// A GDT define segmentos de memória e seus privilégios

// Estrutura de uma entrada na GDT (8 bytes)
typedef struct {
    uint16_t limit_low;      // Limite inferior (bits 0-15)
    uint16_t base_low;       // Base inferior (bits 0-15) 
    uint8_t base_middle;     // Base média (bits 16-23)
    uint8_t access;          // Flags de acesso (presente, privilégio, tipo)
    uint8_t granularity;     // Granularidade e limite superior
    uint8_t base_high;       // Base superior (bits 24-31)
} __attribute__((packed)) gdt_entry;

// Estrutura do ponteiro para a GDT
typedef struct {
    uint16_t limit;          // Tamanho da GDT - 1
    uint32_t base;           // Endereço base da GDT
} __attribute__((packed)) gdt_ptr;

// Array da GDT com 3 entradas: NULL, Código, Dados
gdt_entry gdt[3];
gdt_ptr gp;                              // Ponteiro para a GDT

// Configura uma entrada específica na GDT
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    // Configura o endereço base (dividido em 3 partes de 8 bits cada)
    gdt[num].base_low = (base & 0xFFFF);           // Bits 0-15
    gdt[num].base_middle = (base >> 16) & 0xFF;    // Bits 16-23  
    gdt[num].base_high = (base >> 24) & 0xFF;      // Bits 24-31
    
    // Configura o limite (dividido em 2 partes)
    gdt[num].limit_low = (limit & 0xFFFF);         // Bits 0-15
    gdt[num].granularity = (limit >> 16) & 0x0F;   // Bits 16-19
    
    // Combina granularidade com as flags
    gdt[num].granularity |= gran & 0xF0;           // Flags de granularidade
    gdt[num].access = access;                      // Flags de acesso
}

// Função externa em assembly para carregar a GDT
extern void gdt_flush(uint32_t);

// Inicializa a GDT com segmentos básicos
void gdt_init(void) {
    // Configura o ponteiro da GDT
    gp.limit = (sizeof(gdt_entry) * 3) - 1;       // Tamanho total - 1
    gp.base = (uint32_t)&gdt;                     // Endereço da GDT
    
    // Configura as 3 entradas da GDT:
    gdt_set_gate(0, 0, 0, 0, 0);                  // Entrada NULL (obrigatória)
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);  // Segmento de código (4GB, executável)
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);  // Segmento de dados (4GB, leitura/escrita)
    
    // Carrega a GDT no processador
    gdt_flush((uint32_t)&gp);
}

// === SEÇÃO IDT (Interrupt Descriptor Table) ===

// Estrutura de uma entrada na IDT
typedef struct {
    uint16_t offset_low;     // Offset inferior (bits 0-15)
    uint16_t selector;       // Seletor de segmento
    uint8_t zero;           // Sempre 0
    uint8_t type_attr;      // Tipo e atributos
    uint16_t offset_high;   // Offset superior (bits 16-31)
} __attribute__((packed)) idt_entry;

// Ponteiro para a IDT
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr;

// Array da IDT e ponteiro
idt_entry idt[256];
idt_ptr idtp;

// Função externa para carregar IDT
extern void idt_load(uint32_t);

// Handler externo de interrupção do teclado
extern void keyboard_interrupt_handler(void);

// Configura uma entrada na IDT
void idt_set_gate(uint8_t num, uint32_t offset, uint16_t sel, uint8_t flags) {
    idt[num].offset_low = offset & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].type_attr = flags;
    idt[num].offset_high = (offset >> 16) & 0xFFFF;
}

// Inicializa a IDT
void idt_init(void) {
    // Configura o ponteiro da IDT
    idtp.limit = (sizeof(idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;
    
    // Limpa a IDT
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    
    // Configura a interrupção do teclado (IRQ1 → INT 0x21)
    idt_set_gate(0x21, (uint32_t)keyboard_interrupt_handler, 0x08, 0x8E);
    
    // Carrega a IDT
    idt_load((uint32_t)&idtp);
}

// Inicializa o teclado
void keyboard_init(void) {
    pic_init();     // Inicializa o PIC
    idt_init();     // Inicializa a IDT
    __asm__ volatile ("sti");  // Habilita interrupções
}

// === FUNÇÃO PRINCIPAL DO KERNEL ===
void kernel_main(void) {
    // Inicialização do sistema
    terminal_init();                          // Inicializa o terminal VGA
    gdt_init();                              // Configura a GDT
    keyboard_init();                         // Inicializa teclado e IDT
    
    // Mensagem de boas-vindas
    terminal_print("Bem-vindo ao NanoOS!\n\n");
    terminal_print("Digite 'help' para ver comandos\n\n");
    terminal_print("> ");
    
    // Loop principal do kernel
    while(1) {
        __asm__ volatile ("hlt");            // Pausa o processador até próxima interrupção
    }
}
