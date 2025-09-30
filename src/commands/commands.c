// ============================================================================
// NanoOS - Sistema de Comandos
// Implementação dos comandos disponíveis no terminal
// ============================================================================

#include "../../include/commands.h"
#include <stdint.h>
#include <stddef.h>

// Declarações externas necessárias
extern void terminal_init(void);

// ============================================================================
// IMPLEMENTAÇÕES DOS COMANDOS
// ============================================================================

// Comando: help - Mostra lista de comandos disponíveis
void cmd_help(void) {
    terminal_print("\nComandos disponiveis:\n");
    terminal_print("  help    - Mostra esta ajuda\n");
    terminal_print("  clear   - Limpa a tela\n");
    terminal_print("  about   - Informacoes do kernel\n");
    terminal_print("  uptime  - Tempo ligado (em ticks)\n");
    terminal_print("  echo    - Repete o texto digitado\n");
    terminal_print("  license - Mostra licenca e desenvolvedores\n");
}

// Comando: clear - Limpa a tela
void cmd_clear(void) {
    terminal_init();
}

// Comando: about - Informações do kernel
void cmd_about(void) {
    terminal_print("\nNanoOS v0.2\n");
    terminal_print("Mini kernel 32-bit com:\n");
    terminal_print("- Teclado funcional\n");
    terminal_print("- Timer/PIT integrado\n");
    terminal_print("- IDT completa\n");
    terminal_print("- Tratamento de interrupcoes\n");
}

// Comando: uptime - Mostra tempo de execução
void cmd_uptime(void) {
    char buffer[32];
    
    terminal_print("\nTicks desde boot: ");
    uint_to_str(timer_ticks, buffer, sizeof(buffer));
    terminal_print(buffer);
    
    terminal_print(" (aprox. ");
    uint_to_str(timer_ticks / TIMER_FREQUENCY, buffer, sizeof(buffer));
    terminal_print(buffer);
    terminal_print("s)\n");
}

// Comando: license - Mostra licença e desenvolvedores
void cmd_license(void) {
    terminal_print("\nNanoOS - Licenca MIT\n");
    terminal_print("Copyright (c) 2025 marcosgomes068 / pietr0davila\n");
    terminal_print("\nDesenvolvedores:\n");
    terminal_print("- Marcos Gomes: https://github.com/marcosgomes068\n");
    terminal_print("- Pietro Davila: https://github.com/pietr0davila\n");
    terminal_print("\nSoftware livre sob licenca MIT.\n");
    terminal_print("Veja o arquivo LICENSE para detalhes completos.\n");
}

// Comando: echo - Repete texto
void cmd_echo(const char* text) {
    terminal_print("\n");
    terminal_print(text);
    terminal_print("\n");
}

// ============================================================================
// PROCESSADOR PRINCIPAL DE COMANDOS
// ============================================================================

// Processa comandos digitados pelo usuário
void process_command(const char* cmd) {
    if (!cmd) return;  // Proteção
    
    if (strcmp(cmd, "help") == 0) {
        cmd_help();
        
    } else if (strcmp(cmd, "clear") == 0) {
        cmd_clear();
        
    } else if (strcmp(cmd, "about") == 0) {
        cmd_about();
        
    } else if (strcmp(cmd, "uptime") == 0) {
        cmd_uptime();
        
    } else if (strcmp(cmd, "license") == 0) {
        cmd_license();
        
    } else if (strlen(cmd) > 5 && cmd[0] == 'e' && cmd[1] == 'c' && 
               cmd[2] == 'h' && cmd[3] == 'o' && cmd[4] == ' ') {
        // Comando echo - repete o texto após "echo "
        cmd_echo(cmd + 5);
        
    } else if (strlen(cmd) > 0) {
        terminal_print("\nComando desconhecido: ");
        terminal_print(cmd);
        terminal_print("\nDigite 'help' para ver comandos disponiveis\n");
    }
}