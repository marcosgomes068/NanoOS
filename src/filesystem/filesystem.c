// ============================================================================
// NanoOS - Sistema de Arquivos FAT12
// Implementação básica para leitura de arquivos em disquetes FAT12
// ============================================================================

#include "../../include/filesystem.h"
#include "../../include/disk.h"
#include "../../include/commands.h"
#include <stdint.h>

// ============================================================================
// VARIÁVEIS GLOBAIS DO SISTEMA DE ARQUIVOS
// ============================================================================

static filesystem_t fs;
static uint8_t fat_buffer[FAT12_SECTOR_SIZE * 9]; // Buffer para FAT (máx 9 setores)
static int fs_initialized = 0;

// ============================================================================
// FUNÇÕES AUXILIARES DE STRING
// ============================================================================

// Converte string para formato 8.3 (maiúsculo, preenchido com espaços)
static void str_to_fat_format(const char* input, char* output) {
    int i, j;
    
    // Limpa o buffer de saída
    for (i = 0; i < 11; i++) {
        output[i] = ' ';
    }
    output[11] = '\0';
    
    // Processa nome do arquivo
    for (i = 0, j = 0; i < 8 && input[j] && input[j] != '.'; i++, j++) {
        if (input[j] >= 'a' && input[j] <= 'z') {
            output[i] = input[j] - 'a' + 'A'; // Converte para maiúsculo
        } else {
            output[i] = input[j];
        }
    }
    
    // Procura a extensão
    while (input[j] && input[j] != '.') j++;
    if (input[j] == '.') {
        j++; // Pula o ponto
        // Processa extensão
        for (i = 8; i < 11 && input[j]; i++, j++) {
            if (input[j] >= 'a' && input[j] <= 'z') {
                output[i] = input[j] - 'a' + 'A';
            } else {
                output[i] = input[j];
            }
        }
    }
}

// Compara nomes de arquivo no formato FAT
static int fat_name_compare(const char* fat_name, const char* search_name) {
    char formatted_search[12];
    str_to_fat_format(search_name, formatted_search);
    
    for (int i = 0; i < 11; i++) {
        if (fat_name[i] != formatted_search[i]) {
            return 0; // Diferentes
        }
    }
    return 1; // Iguais
}

// ============================================================================
// INICIALIZAÇÃO DO SISTEMA DE ARQUIVOS
// ============================================================================

// Inicializa o sistema de arquivos
int fs_init(void) {
    terminal_print("Inicializando sistema de arquivos FAT12...\n");
    
    // Inicializa o driver de disco
    if (disk_init() != 0) {
        terminal_print("Erro: Nao foi possivel inicializar o disco.\n");
        return -1;
    }
    
    // Lê o boot sector
    if (fs_read_boot_sector() != 0) {
        terminal_print("Erro: Nao foi possivel ler o boot sector.\n");
        return -1;
    }
    
    // Carrega a tabela FAT
    if (fs_load_fat_table() != 0) {
        terminal_print("Erro: Nao foi possivel carregar a tabela FAT.\n");
        return -1;
    }
    
    fs_initialized = 1;
    terminal_print("Sistema de arquivos inicializado com sucesso!\n");
    return 0;
}

// Lê o boot sector do disco
int fs_read_boot_sector(void) {
    // Lê o setor 0 (boot sector)
    if (disk_read_sector(0, &fs.boot_sector) != 0) {
        return -1;
    }
    
    // Calcula informações do sistema de arquivos
    fs.sectors_per_cluster = fs.boot_sector.sectors_per_cluster;
    fs.bytes_per_cluster = fs.boot_sector.bytes_per_sector * fs.sectors_per_cluster;
    
    // Calcula setor inicial do diretório raiz
    fs.root_dir_sector = fs.boot_sector.reserved_sectors + 
                        (fs.boot_sector.fat_count * fs.boot_sector.sectors_per_fat);
    
    // Calcula setor inicial da área de dados
    uint32_t root_dir_sectors = (fs.boot_sector.root_entries * 32 + 
                                fs.boot_sector.bytes_per_sector - 1) / 
                               fs.boot_sector.bytes_per_sector;
    fs.data_sector = fs.root_dir_sector + root_dir_sectors;
    
    return 0;
}

// Carrega a tabela FAT na memória
int fs_load_fat_table(void) {
    uint32_t fat_start_sector = fs.boot_sector.reserved_sectors;
    uint32_t fat_sectors = fs.boot_sector.sectors_per_fat;
    
    // Lê todos os setores da primeira FAT
    for (uint32_t i = 0; i < fat_sectors && i < 9; i++) {
        if (disk_read_sector(fat_start_sector + i, 
                           fat_buffer + (i * FAT12_SECTOR_SIZE)) != 0) {
            return -1;
        }
    }
    
    fs.fat_table = fat_buffer;
    return 0;
}

// ============================================================================
// OPERAÇÕES DE ARQUIVO
// ============================================================================

// Procura um arquivo no diretório raiz
int fs_find_file(const char* filename, fat12_dir_entry_t* entry) {
    if (!fs_initialized) return -1;
    
    uint8_t sector_buffer[FAT12_SECTOR_SIZE];
    uint32_t root_sectors = (fs.boot_sector.root_entries * 32 + 
                            FAT12_SECTOR_SIZE - 1) / FAT12_SECTOR_SIZE;
    
    // Percorre todos os setores do diretório raiz
    for (uint32_t sector = 0; sector < root_sectors; sector++) {
        if (disk_read_sector(fs.root_dir_sector + sector, sector_buffer) != 0) {
            return -1;
        }
        
        // Percorre todas as entradas do setor
        fat12_dir_entry_t* entries = (fat12_dir_entry_t*)sector_buffer;
        uint32_t entries_per_sector = FAT12_SECTOR_SIZE / sizeof(fat12_dir_entry_t);
        
        for (uint32_t i = 0; i < entries_per_sector; i++) {
            // Verifica se a entrada é válida
            if (entries[i].filename[0] == 0x00) {
                return -1; // Fim das entradas
            }
            
            if (entries[i].filename[0] == 0xE5) {
                continue; // Entrada deletada
            }
            
            // Ignora entradas de volume label e diretórios
            if (entries[i].attributes & (FAT_ATTR_VOLUME_LABEL | FAT_ATTR_DIRECTORY)) {
                continue;
            }
            
            // Compara nomes
            if (fat_name_compare((char*)entries[i].filename, filename)) {
                *entry = entries[i];
                return 0; // Arquivo encontrado
            }
        }
    }
    
    return -1; // Arquivo não encontrado
}

// Obtém o próximo cluster de um arquivo
uint32_t fs_get_next_cluster(uint32_t cluster) {
    if (!fs_initialized || cluster < 2) return FAT12_CLUSTER_EOF;
    
    // Calcula posição na FAT (FAT12 = 1.5 bytes per entry)
    uint32_t fat_offset = cluster + (cluster / 2); // cluster * 1.5
    uint16_t fat_value = *(uint16_t*)(fs.fat_table + fat_offset);
    
    if (cluster & 1) {
        fat_value = fat_value >> 4; // Cluster ímpar - usa bits superiores
    } else {
        fat_value = fat_value & 0x0FFF; // Cluster par - usa bits inferiores
    }
    
    if (fat_value >= 0xFF8) {
        return FAT12_CLUSTER_EOF; // Fim do arquivo
    }
    
    return fat_value;
}

// Abre um arquivo para leitura
int fs_open(const char* filename, file_handle_t* handle) {
    if (!fs_initialized || !filename || !handle) return -1;
    
    fat12_dir_entry_t entry;
    if (fs_find_file(filename, &entry) != 0) {
        return -1; // Arquivo não encontrado
    }
    
    // Inicializa o handle
    str_to_fat_format(filename, handle->filename);
    handle->size = entry.file_size;
    handle->current_cluster = entry.first_cluster_low;
    handle->position = 0;
    handle->is_open = 1;
    
    return 0;
}

// Lê dados de um arquivo
int fs_read(file_handle_t* handle, void* buffer, uint32_t size) {
    if (!fs_initialized || !handle || !handle->is_open || !buffer) return -1;
    
    uint32_t bytes_read = 0;
    uint8_t* buf = (uint8_t*)buffer;
    uint8_t cluster_buffer[FAT12_SECTOR_SIZE];
    
    while (bytes_read < size && handle->position < handle->size && 
           handle->current_cluster < FAT12_CLUSTER_EOF) {
        
        // Calcula setor do cluster atual
        uint32_t cluster_sector = fs.data_sector + 
                                 (handle->current_cluster - 2) * fs.sectors_per_cluster;
        
        // Lê o cluster
        if (disk_read_sector(cluster_sector, cluster_buffer) != 0) {
            return -1;
        }
        
        // Calcula quantos bytes copiar
        uint32_t cluster_offset = handle->position % fs.boot_sector.bytes_per_sector;
        uint32_t bytes_in_cluster = fs.boot_sector.bytes_per_sector - cluster_offset;
        uint32_t bytes_to_copy = size - bytes_read;
        
        if (bytes_to_copy > bytes_in_cluster) {
            bytes_to_copy = bytes_in_cluster;
        }
        
        if (handle->position + bytes_to_copy > handle->size) {
            bytes_to_copy = handle->size - handle->position;
        }
        
        // Copia dados
        for (uint32_t i = 0; i < bytes_to_copy; i++) {
            buf[bytes_read + i] = cluster_buffer[cluster_offset + i];
        }
        
        bytes_read += bytes_to_copy;
        handle->position += bytes_to_copy;
        
        // Se chegou ao fim do cluster, vai para o próximo
        if ((handle->position % fs.boot_sector.bytes_per_sector) == 0) {
            handle->current_cluster = fs_get_next_cluster(handle->current_cluster);
        }
    }
    
    return bytes_read;
}

// Fecha um arquivo
int fs_close(file_handle_t* handle) {
    if (!handle) return -1;
    
    handle->is_open = 0;
    return 0;
}

// Lista arquivos do diretório raiz
int fs_list_directory(void) {
    if (!fs_initialized) return -1;
    
    uint8_t sector_buffer[FAT12_SECTOR_SIZE];
    uint32_t root_sectors = (fs.boot_sector.root_entries * 32 + 
                            FAT12_SECTOR_SIZE - 1) / FAT12_SECTOR_SIZE;
    int file_count = 0;
    
    terminal_print("\nArquivos no diretorio raiz:\n");
    terminal_print("Nome           Tamanho\n");
    terminal_print("------------------------\n");
    
    // Percorre todos os setores do diretório raiz
    for (uint32_t sector = 0; sector < root_sectors; sector++) {
        if (disk_read_sector(fs.root_dir_sector + sector, sector_buffer) != 0) {
            return -1;
        }
        
        fat12_dir_entry_t* entries = (fat12_dir_entry_t*)sector_buffer;
        uint32_t entries_per_sector = FAT12_SECTOR_SIZE / sizeof(fat12_dir_entry_t);
        
        for (uint32_t i = 0; i < entries_per_sector; i++) {
            if (entries[i].filename[0] == 0x00) {
                goto end_listing; // Fim das entradas
            }
            
            if (entries[i].filename[0] == 0xE5) {
                continue; // Entrada deletada
            }
            
            // Ignora volume labels
            if (entries[i].attributes & FAT_ATTR_VOLUME_LABEL) {
                continue;
            }
            
            // Mostra nome do arquivo
            char name[12] = {0};
            for (int j = 0; j < 8; j++) {
                if (entries[i].filename[j] != ' ') {
                    name[j] = entries[i].filename[j];
                } else {
                    break;
                }
            }
            
            // Adiciona extensão se houver
            if (entries[i].extension[0] != ' ') {
                int len = strlen(name);
                name[len] = '.';
                for (int j = 0; j < 3; j++) {
                    if (entries[i].extension[j] != ' ') {
                        name[len + 1 + j] = entries[i].extension[j];
                    } else {
                        break;
                    }
                }
            }
            
            // Mostra informações
            terminal_print(name);
            
            // Preenche com espaços
            int name_len = strlen(name);
            for (int j = name_len; j < 14; j++) {
                terminal_print(" ");
            }
            
            // Mostra tamanho
            char size_str[16];
            uint_to_str(entries[i].file_size, size_str, sizeof(size_str));
            terminal_print(size_str);
            terminal_print(" bytes");
            
            if (entries[i].attributes & FAT_ATTR_DIRECTORY) {
                terminal_print(" <DIR>");
            }
            
            terminal_print("\n");
            file_count++;
        }
    }
    
end_listing:
    char count_str[16];
    uint_to_str(file_count, count_str, sizeof(count_str));
    terminal_print("\nTotal: ");
    terminal_print(count_str);
    terminal_print(" arquivo(s)\n");
    
    return file_count;
}

// ============================================================================
// UTILITÁRIOS
// ============================================================================

// Mostra informações do boot sector
void fs_print_boot_info(void) {
    if (!fs_initialized) {
        terminal_print("Sistema de arquivos nao inicializado.\n");
        return;
    }
    
    terminal_print("\nInformacoes do sistema de arquivos:\n");
    
    char buffer[32];
    
    terminal_print("Bytes por setor: ");
    uint_to_str(fs.boot_sector.bytes_per_sector, buffer, sizeof(buffer));
    terminal_print(buffer);
    terminal_print("\n");
    
    terminal_print("Setores por cluster: ");
    uint_to_str(fs.boot_sector.sectors_per_cluster, buffer, sizeof(buffer));
    terminal_print(buffer);
    terminal_print("\n");
    
    terminal_print("Numero de FATs: ");
    uint_to_str(fs.boot_sector.fat_count, buffer, sizeof(buffer));
    terminal_print(buffer);
    terminal_print("\n");
    
    terminal_print("Entradas do diretorio raiz: ");
    uint_to_str(fs.boot_sector.root_entries, buffer, sizeof(buffer));
    terminal_print(buffer);
    terminal_print("\n");
    
    terminal_print("Setores por FAT: ");
    uint_to_str(fs.boot_sector.sectors_per_fat, buffer, sizeof(buffer));
    terminal_print(buffer);
    terminal_print("\n");
}