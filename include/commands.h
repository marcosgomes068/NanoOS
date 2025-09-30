#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>
#include <stddef.h>

// ============================================================================
// DECLARAÇÕES DAS FUNÇÕES DE COMANDO
// ============================================================================

// Função principal para processar comandos
void process_command(const char* cmd);

// Comandos individuais
void cmd_help(void);
void cmd_clear(void);
void cmd_about(void);
void cmd_uptime(void);
void cmd_license(void);
void cmd_echo(const char* text);
void cmd_shutdown(void);

// Comandos do sistema de arquivos
void cmd_ls(void);
void cmd_cat(const char* filename);
void cmd_fsinfo(void);
void cmd_diskinfo(void);

// Comandos de rede
void cmd_ifconfig(void);
void cmd_ping(const char* target);
void cmd_arp(void);
void cmd_netstat(void);

// Função auxiliar para imprimir no terminal
void terminal_print(const char* str);

// Função de encerramento do sistema
void shutdown_system(void);

// Função auxiliar para conversão de números
void uint_to_str(uint32_t num, char* buffer, size_t buffer_size);

// Funções auxiliares de string (implementadas no kernel.c)
size_t strlen(const char* str);
int strcmp(const char* s1, const char* s2);

// Variável externa para o timer
extern volatile uint32_t timer_ticks;

// Constante para frequência do timer
#define TIMER_FREQUENCY 100

#endif // COMMANDS_H