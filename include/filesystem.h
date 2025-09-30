#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>
#include <stddef.h>

// ============================================================================
// ESTRUTURAS DO SISTEMA DE ARQUIVOS FAT12
// ============================================================================

// Boot Sector (FAT12) - 512 bytes
typedef struct fat12_boot_sector {
    uint8_t  jump[3];                 // Jump instruction
    uint8_t  oem_name[8];            // OEM name
    uint16_t bytes_per_sector;        // Bytes per sector (512)
    uint8_t  sectors_per_cluster;     // Sectors per cluster
    uint16_t reserved_sectors;        // Reserved sectors
    uint8_t  fat_count;              // Number of FATs
    uint16_t root_entries;           // Root directory entries
    uint16_t total_sectors_16;       // Total sectors (if < 65536)
    uint8_t  media_descriptor;       // Media descriptor
    uint16_t sectors_per_fat;        // Sectors per FAT
    uint16_t sectors_per_track;      // Sectors per track
    uint16_t heads;                  // Number of heads
    uint32_t hidden_sectors;         // Hidden sectors
    uint32_t total_sectors_32;       // Total sectors (if >= 65536)
    uint8_t  drive_number;           // Drive number
    uint8_t  reserved;               // Reserved
    uint8_t  boot_signature;         // Boot signature
    uint32_t volume_serial;          // Volume serial number
    uint8_t  volume_label[11];       // Volume label
    uint8_t  filesystem_type[8];     // Filesystem type
} __attribute__((packed)) fat12_boot_sector_t;

// Directory Entry (FAT12) - 32 bytes
typedef struct fat12_dir_entry {
    uint8_t  filename[8];            // Filename (8 chars)
    uint8_t  extension[3];           // Extension (3 chars)
    uint8_t  attributes;             // File attributes
    uint8_t  reserved;               // Reserved
    uint8_t  creation_time_fine;     // Creation time (fine resolution)
    uint16_t creation_time;          // Creation time
    uint16_t creation_date;          // Creation date
    uint16_t last_access_date;       // Last access date
    uint16_t first_cluster_high;     // First cluster (high 16 bits)
    uint16_t last_write_time;        // Last write time
    uint16_t last_write_date;        // Last write date
    uint16_t first_cluster_low;      // First cluster (low 16 bits)
    uint32_t file_size;              // File size in bytes
} __attribute__((packed)) fat12_dir_entry_t;

// File attributes
#define FAT_ATTR_READ_ONLY    0x01
#define FAT_ATTR_HIDDEN       0x02
#define FAT_ATTR_SYSTEM       0x04
#define FAT_ATTR_VOLUME_LABEL 0x08
#define FAT_ATTR_DIRECTORY    0x10
#define FAT_ATTR_ARCHIVE      0x20

// FAT12 constants
#define FAT12_SECTOR_SIZE     512
#define FAT12_ROOT_ENTRIES    224
#define FAT12_CLUSTER_FREE    0x000
#define FAT12_CLUSTER_EOF     0xFF8

// ============================================================================
// ESTRUTURA DO SISTEMA DE ARQUIVOS
// ============================================================================

typedef struct filesystem {
    fat12_boot_sector_t boot_sector;
    uint8_t *fat_table;              // FAT table in memory
    uint32_t root_dir_sector;        // First sector of root directory
    uint32_t data_sector;            // First sector of data area
    uint32_t sectors_per_cluster;    // Sectors per cluster
    uint32_t bytes_per_cluster;      // Bytes per cluster
} filesystem_t;

// File handle structure
typedef struct file_handle {
    char filename[12];               // 8.3 format filename
    uint32_t size;                   // File size
    uint32_t current_cluster;        // Current cluster
    uint32_t position;               // Current position in file
    uint8_t is_open;                 // File open flag
} file_handle_t;

// ============================================================================
// FUNÇÕES DO SISTEMA DE ARQUIVOS
// ============================================================================

// Inicialização
int fs_init(void);
int fs_read_boot_sector(void);
int fs_load_fat_table(void);

// Operações de arquivo
int fs_open(const char* filename, file_handle_t* handle);
int fs_read(file_handle_t* handle, void* buffer, uint32_t size);
int fs_close(file_handle_t* handle);
int fs_list_directory(void);

// Utilitários
void fs_print_boot_info(void);
uint32_t fs_get_next_cluster(uint32_t cluster);
int fs_find_file(const char* filename, fat12_dir_entry_t* entry);

// Funções de disco (implementadas em disk.c)
int disk_read_sector(uint32_t sector, void* buffer);
int disk_write_sector(uint32_t sector, const void* buffer);

#endif // FILESYSTEM_H