// ============================================================================
// NanoOS - Driver de Disco ATA/IDE
// Implementação básica para leitura/escrita de setores
// ============================================================================

#include "../../include/disk.h"
#include "../../include/kernel.h"
#include <stdint.h>

// ============================================================================
// VARIÁVEIS GLOBAIS
// ============================================================================

static int disk_available = 0;

// ============================================================================
// FUNÇÕES DE E/S
// ============================================================================

// Lê um byte de uma porta
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Escreve um byte em uma porta
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Lê uma word (16 bits) de uma porta
static inline uint16_t inw(uint16_t port) {
    uint16_t result;
    __asm__ volatile ("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Escreve uma word (16 bits) em uma porta
static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

// ============================================================================
// FUNÇÕES DE ESPERA
// ============================================================================

// Aguarda o disco ficar pronto
void disk_wait_ready(void) {
    while (inb(ATA_PRIMARY_STATUS) & ATA_STATUS_BSY);
}

// Aguarda dados estarem prontos para leitura/escrita
int disk_wait_drq(void) {
    uint32_t timeout = DISK_TIMEOUT;
    
    while (timeout--) {
        uint8_t status = inb(ATA_PRIMARY_STATUS);
        if (status & ATA_STATUS_DRQ) return 0;  // Sucesso
        if (status & ATA_STATUS_ERR) return -1; // Erro
    }
    
    return -1; // Timeout
}

// ============================================================================
// INICIALIZAÇÃO DO DISCO
// ============================================================================

// Inicializa o driver de disco
int disk_init(void) {
    terminal_print("Inicializando driver de disco...\n");
    
    // Seleciona drive 0 (master)
    outb(ATA_PRIMARY_DRVHEAD, 0xA0);
    disk_wait_ready();
    
    // Tenta identificar o disco
    if (disk_identify() == 0) {
        disk_available = 1;
        terminal_print("Disco detectado com sucesso!\n");
        return 0;
    } else {
        terminal_print("Nenhum disco detectado.\n");
        return -1;
    }
}

// Identifica o disco
int disk_identify(void) {
    // Seleciona drive 0
    outb(ATA_PRIMARY_DRVHEAD, 0xA0);
    
    // Zera contadores
    outb(ATA_PRIMARY_SECCOUNT, 0);
    outb(ATA_PRIMARY_SECNUM, 0);
    outb(ATA_PRIMARY_CYLLOW, 0);
    outb(ATA_PRIMARY_CYLHIGH, 0);
    
    // Envia comando IDENTIFY
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_IDENTIFY);
    
    // Verifica se há drive
    uint8_t status = inb(ATA_PRIMARY_STATUS);
    if (status == 0) return -1; // Não há drive
    
    // Aguarda até não estar busy
    disk_wait_ready();
    
    // Verifica se é ATA (não ATAPI)
    if (inb(ATA_PRIMARY_CYLLOW) != 0 || inb(ATA_PRIMARY_CYLHIGH) != 0) {
        return -1; // Não é ATA
    }
    
    // Aguarda DRQ ou erro
    if (disk_wait_drq() != 0) return -1;
    
    // Lê os dados de identificação (256 words = 512 bytes)
    uint16_t identify_data[256];
    for (int i = 0; i < 256; i++) {
        identify_data[i] = inw(ATA_PRIMARY_DATA);
    }
    
    return 0; // Sucesso
}

// ============================================================================
// OPERAÇÕES DE LEITURA/ESCRITA
// ============================================================================

// Lê um setor do disco
int disk_read_sector(uint32_t lba, void* buffer) {
    if (!disk_available) return -1;
    
    uint16_t* buf = (uint16_t*)buffer;
    
    // Aguarda disco pronto
    disk_wait_ready();
    
    // Configura LBA
    outb(ATA_PRIMARY_DRVHEAD, 0xE0 | ((lba >> 24) & 0x0F)); // LBA mode, drive 0
    outb(ATA_PRIMARY_SECCOUNT, 1);                           // 1 setor
    outb(ATA_PRIMARY_SECNUM, lba & 0xFF);                   // LBA[7:0]
    outb(ATA_PRIMARY_CYLLOW, (lba >> 8) & 0xFF);            // LBA[15:8]
    outb(ATA_PRIMARY_CYLHIGH, (lba >> 16) & 0xFF);          // LBA[23:16]
    
    // Envia comando de leitura
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_READ_SECTORS);
    
    // Aguarda dados estarem prontos
    if (disk_wait_drq() != 0) return -1;
    
    // Lê os dados (256 words = 512 bytes)
    for (int i = 0; i < 256; i++) {
        buf[i] = inw(ATA_PRIMARY_DATA);
    }
    
    return 0; // Sucesso
}

// Escreve um setor no disco
int disk_write_sector(uint32_t lba, const void* buffer) {
    if (!disk_available) return -1;
    
    const uint16_t* buf = (const uint16_t*)buffer;
    
    // Aguarda disco pronto
    disk_wait_ready();
    
    // Configura LBA
    outb(ATA_PRIMARY_DRVHEAD, 0xE0 | ((lba >> 24) & 0x0F)); // LBA mode, drive 0
    outb(ATA_PRIMARY_SECCOUNT, 1);                           // 1 setor
    outb(ATA_PRIMARY_SECNUM, lba & 0xFF);                   // LBA[7:0]
    outb(ATA_PRIMARY_CYLLOW, (lba >> 8) & 0xFF);            // LBA[15:8]
    outb(ATA_PRIMARY_CYLHIGH, (lba >> 16) & 0xFF);          // LBA[23:16]
    
    // Envia comando de escrita
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_WRITE_SECTORS);
    
    // Aguarda estar pronto para receber dados
    if (disk_wait_drq() != 0) return -1;
    
    // Escreve os dados (256 words = 512 bytes)
    for (int i = 0; i < 256; i++) {
        outw(ATA_PRIMARY_DATA, buf[i]);
    }
    
    // Aguarda conclusão da escrita
    disk_wait_ready();
    
    return 0; // Sucesso
}

// Lê múltiplos setores
int disk_read_sectors(uint32_t lba, uint32_t count, void* buffer) {
    uint8_t* buf = (uint8_t*)buffer;
    
    for (uint32_t i = 0; i < count; i++) {
        if (disk_read_sector(lba + i, buf + (i * DISK_SECTOR_SIZE)) != 0) {
            return -1;
        }
    }
    
    return 0;
}

// Escreve múltiplos setores
int disk_write_sectors(uint32_t lba, uint32_t count, const void* buffer) {
    const uint8_t* buf = (const uint8_t*)buffer;
    
    for (uint32_t i = 0; i < count; i++) {
        if (disk_write_sector(lba + i, buf + (i * DISK_SECTOR_SIZE)) != 0) {
            return -1;
        }
    }
    
    return 0;
}

// ============================================================================
// UTILITÁRIOS
// ============================================================================

// Mostra informações do disco
void disk_print_info(void) {
    if (disk_available) {
        terminal_print("Status do disco: Disponivel\n");
        terminal_print("Tamanho do setor: 512 bytes\n");
    } else {
        terminal_print("Status do disco: Nao disponivel\n");
    }
}