#ifndef DISK_H
#define DISK_H

#include <stdint.h>

// ============================================================================
// DRIVER DE DISCO (ATA/IDE BÁSICO)
// ============================================================================

// Portas ATA Primary
#define ATA_PRIMARY_DATA        0x1F0
#define ATA_PRIMARY_ERROR       0x1F1
#define ATA_PRIMARY_SECCOUNT    0x1F2
#define ATA_PRIMARY_SECNUM      0x1F3
#define ATA_PRIMARY_CYLLOW      0x1F4
#define ATA_PRIMARY_CYLHIGH     0x1F5
#define ATA_PRIMARY_DRVHEAD     0x1F6
#define ATA_PRIMARY_STATUS      0x1F7
#define ATA_PRIMARY_COMMAND     0x1F7

// Comandos ATA
#define ATA_CMD_READ_SECTORS    0x20
#define ATA_CMD_WRITE_SECTORS   0x30
#define ATA_CMD_IDENTIFY        0xEC

// Status bits
#define ATA_STATUS_BSY          0x80  // Busy
#define ATA_STATUS_DRDY         0x40  // Drive ready
#define ATA_STATUS_DRQ          0x08  // Data request
#define ATA_STATUS_ERR          0x01  // Error

// Configurações
#define DISK_SECTOR_SIZE        512
#define DISK_TIMEOUT            10000

// ============================================================================
// FUNÇÕES DO DRIVER DE DISCO
// ============================================================================

// Inicialização
int disk_init(void);
int disk_identify(void);

// Operações básicas
int disk_read_sector(uint32_t lba, void* buffer);
int disk_write_sector(uint32_t lba, const void* buffer);
int disk_read_sectors(uint32_t lba, uint32_t count, void* buffer);
int disk_write_sectors(uint32_t lba, uint32_t count, const void* buffer);

// Utilitários
void disk_wait_ready(void);
int disk_wait_drq(void);
void disk_print_info(void);

#endif // DISK_H