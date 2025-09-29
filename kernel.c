// Includes necessários para tipos de dados
#include <stdint.h>
#include <stddef.h>

// Configurações do terminal VGA
#define VGA_MEMORY 0xB8000    // Endereço da memória VGA
#define VGA_WIDTH 80          // Largura em caracteres
#define VGA_HEIGHT 25         // Altura em caracteres

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

// Buffer para entrada de comandos (não usado ainda)
char input_buffer[256];                               // Buffer de entrada
size_t input_pos = 0;                                // Posição no buffer

// Declarações das funções
void terminal_init(void);                    // Inicializa o terminal VGA
void terminal_putchar(char c);               // Escreve um caractere na tela
void terminal_print(const char* str);        // Escreve uma string na tela
void gdt_init(void);                         // Inicializa a GDT
void idt_init(void);                         // Inicializa a IDT (stub)
void keyboard_init(void);                    // Inicializa o teclado (stub)
void process_command(const char* cmd);       // Processa comandos do usuário

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
        terminal_print("Mini kernel 32-bit educacional\n");
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

// === STUBS PARA EXPANSÃO FUTURA ===

// IDT Stub (básico) - Interrupt Descriptor Table
void idt_init(void) {
    // TODO: Implementar IDT completo para gerenciar interrupções
    // Necessário para teclado, timer, etc.
}

// Inicialização do teclado (stub)
void keyboard_init(void) {
    // TODO: Implementar driver de teclado PS/2
    // Configurar porta 0x60/0x64 e interrupts
}

// === FUNÇÃO PRINCIPAL DO KERNEL ===
void kernel_main(void) {
    // Inicialização do sistema
    terminal_init();                          // Inicializa o terminal VGA
    gdt_init();                              // Configura a GDT
    idt_init();                              // Stub da IDT
    
    // Mensagem de boas-vindas
    terminal_print("Bem-vindo ao NanoOS!\n\n");
    terminal_print("Digite 'help' para ver comandos\n\n");
    terminal_print("> ");
    
    // Loop principal do kernel
    while(1) {
        __asm__ volatile ("hlt");            // Pausa o processador até próxima interrupção
    }
}
