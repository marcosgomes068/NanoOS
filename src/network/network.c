// ============================================================================
// NanoOS - Driver de Rede RTL8139
// Driver básico para placa de rede Ethernet RTL8139
// ============================================================================

#include "../../include/network.h"
#include "../../include/kernel.h"
#include <stdint.h>
#include <stddef.h>

// ============================================================================
// VARIÁVEIS GLOBAIS
// ============================================================================

static rtl8139_device_t rtl8139;
static network_interface_t net_interface;
static arp_entry_t arp_table[ARP_TABLE_SIZE];
static uint8_t rx_buffer[8192 + 16 + 1500]; // Buffer de recepção
static uint8_t tx_buffers[4][1536];         // 4 buffers de transmissão

// ============================================================================
// FUNÇÕES AUXILIARES
// ============================================================================

// Leitura/Escrita de registradores
static inline uint8_t rtl8139_read8(uint16_t reg) {
    return inb(rtl8139.io_base + reg);
}

static inline uint16_t rtl8139_read16(uint16_t reg) {
    return inw(rtl8139.io_base + reg);
}

static inline uint32_t rtl8139_read32(uint16_t reg) {
    return inl(rtl8139.io_base + reg);
}

static inline void rtl8139_write8(uint16_t reg, uint8_t val) {
    outb(rtl8139.io_base + reg, val);
}

static inline void rtl8139_write16(uint16_t reg, uint16_t val) {
    outw(rtl8139.io_base + reg, val);
}

static inline void rtl8139_write32(uint16_t reg, uint32_t val) {
    outl(rtl8139.io_base + reg, val);
}

// ============================================================================
// INICIALIZAÇÃO DO DRIVER
// ============================================================================

void network_init(void) {
    terminal_print("Inicializando subsistema de rede...\n");
    
    // Inicializar tabela ARP
    arp_init();
    
    // Tentar inicializar RTL8139
    if (rtl8139_init() == 0) {
        terminal_print("Placa de rede RTL8139 detectada e inicializada\n");
        network_interface_init();
    } else {
        terminal_print("Nenhuma placa de rede compatível encontrada\n");
        terminal_print("Modo simulado de rede ativado para demonstração\n");
        
        // Configurar interface simulada
        net_interface.enabled = 1;
        string_copy("eth0", net_interface.name);
        
        // MAC simulado: 52:54:00:12:34:56
        net_interface.mac_address.addr[0] = 0x52;
        net_interface.mac_address.addr[1] = 0x54;
        net_interface.mac_address.addr[2] = 0x00;
        net_interface.mac_address.addr[3] = 0x12;
        net_interface.mac_address.addr[4] = 0x34;
        net_interface.mac_address.addr[5] = 0x56;
        
        // IP simulado: 192.168.1.100
        net_interface.ip_address.addr[0] = 192;
        net_interface.ip_address.addr[1] = 168;
        net_interface.ip_address.addr[2] = 1;
        net_interface.ip_address.addr[3] = 100;
        
        // Máscara: 255.255.255.0
        net_interface.subnet_mask.addr[0] = 255;
        net_interface.subnet_mask.addr[1] = 255;
        net_interface.subnet_mask.addr[2] = 255;
        net_interface.subnet_mask.addr[3] = 0;
        
        // Gateway: 192.168.1.1
        net_interface.gateway.addr[0] = 192;
        net_interface.gateway.addr[1] = 168;
        net_interface.gateway.addr[2] = 1;
        net_interface.gateway.addr[3] = 1;
    }
}

int rtl8139_init(void) {
    // Simular detecção de hardware (em um OS real, seria via PCI)
    // Para demonstração, assumimos que não há hardware
    return -1;
}

void network_interface_init(void) {
    net_interface.enabled = 1;
    string_copy("eth0", net_interface.name);
    terminal_print("Interface de rede eth0 configurada\n");
}

// ============================================================================
// TABELA ARP
// ============================================================================

void arp_init(void) {
    for (int i = 0; i < ARP_TABLE_SIZE; i++) {
        arp_table[i].valid = 0;
    }
}

int arp_lookup(const ip_addr_t* ip, mac_addr_t* mac) {
    for (int i = 0; i < ARP_TABLE_SIZE; i++) {
        if (arp_table[i].valid) {
            if (memory_compare(&arp_table[i].ip, ip, sizeof(ip_addr_t)) == 0) {
                memory_copy(mac, &arp_table[i].mac, sizeof(mac_addr_t));
                return 0;
            }
        }
    }
    return -1;
}

void arp_add_entry(const ip_addr_t* ip, const mac_addr_t* mac) {
    // Procurar slot vazio ou substituir o mais antigo
    for (int i = 0; i < ARP_TABLE_SIZE; i++) {
        if (!arp_table[i].valid) {
            memory_copy(&arp_table[i].ip, ip, sizeof(ip_addr_t));
            memory_copy(&arp_table[i].mac, mac, sizeof(mac_addr_t));
            arp_table[i].valid = 1;
            return;
        }
    }
    
    // Se não há slots vazios, substitui o primeiro
    memory_copy(&arp_table[0].ip, ip, sizeof(ip_addr_t));
    memory_copy(&arp_table[0].mac, mac, sizeof(mac_addr_t));
    arp_table[0].valid = 1;
}

void arp_request(const ip_addr_t* ip) {
    terminal_print("Enviando requisicao ARP para ");
    char ip_str[16];
    ip_to_string(ip, ip_str);
    terminal_print(ip_str);
    terminal_print("\n");
    
    // Em um OS real, enviaria packet ARP
    // Para demonstração, simular resposta
    mac_addr_t fake_mac = {{0x52, 0x54, 0x00, 0x12, 0x34, 0x57}};
    arp_add_entry(ip, &fake_mac);
}

// ============================================================================
// FUNÇÕES DE TRANSMISSÃO
// ============================================================================

int eth_send_frame(const uint8_t* data, size_t len, const mac_addr_t* dst_mac, uint16_t type) {
    if (!net_interface.enabled) {
        return -1;
    }
    
    terminal_print("Enviando frame Ethernet (");
    terminal_print_dec(len);
    terminal_print(" bytes)\n");
    
    // Em um OS real, montaria o frame e enviaria via hardware
    return 0;
}

int ip_send(const uint8_t* data, size_t len, const ip_addr_t* dst_ip, uint8_t protocol) {
    if (!net_interface.enabled) {
        return -1;
    }
    
    terminal_print("Enviando pacote IP para ");
    char ip_str[16];
    ip_to_string(dst_ip, ip_str);
    terminal_print(ip_str);
    terminal_print("\n");
    
    // Verificar se destino está na mesma rede
    mac_addr_t dst_mac;
    if (arp_lookup(dst_ip, &dst_mac) != 0) {
        // Não encontrado na tabela ARP, fazer requisição
        arp_request(dst_ip);
        return -1; // Tentar novamente depois
    }
    
    // Em um OS real, montaria cabeçalho IP e enviaria
    return eth_send_frame(data, len + sizeof(ip_header_t), &dst_mac, ETH_TYPE_IP);
}

int ping_send(const ip_addr_t* dst_ip, uint16_t id, uint16_t seq) {
    terminal_print("PING ");
    char ip_str[16];
    ip_to_string(dst_ip, ip_str);
    terminal_print(ip_str);
    terminal_print(" (");
    terminal_print_dec(id);
    terminal_print(":");
    terminal_print_dec(seq);
    terminal_print(")\n");
    
    // Simular ping bem-sucedido
    terminal_print("64 bytes de ");
    terminal_print(ip_str);
    terminal_print(": icmp_seq=");
    terminal_print_dec(seq);
    terminal_print(" ttl=64 tempo=1ms\n");
    
    return 0;
}

// ============================================================================
// PROCESSAMENTO DE PACOTES
// ============================================================================

void network_process_packets(void) {
    // Em um OS real, verificaria buffer de recepção
    // Para demonstração, não faz nada
}

void eth_receive_frame(void) {
    // Processamento de frames recebidos
}

void ip_receive(const uint8_t* packet, size_t len) {
    // Processamento de pacotes IP
}

void icmp_reply(const ip_addr_t* src_ip, const uint8_t* data, size_t len) {
    // Resposta a ping
}

// ============================================================================
// FUNÇÕES UTILITÁRIAS
// ============================================================================

void mac_to_string(const mac_addr_t* mac, char* str) {
    static const char hex[] = "0123456789ABCDEF";
    for (int i = 0; i < 6; i++) {
        str[i*3] = hex[mac->addr[i] >> 4];
        str[i*3+1] = hex[mac->addr[i] & 0xF];
        if (i < 5) str[i*3+2] = ':';
    }
    str[17] = '\0';
}

void ip_to_string(const ip_addr_t* ip, char* str) {
    int pos = 0;
    for (int i = 0; i < 4; i++) {
        uint8_t octet = ip->addr[i];
        if (octet >= 100) {
            str[pos++] = '0' + (octet / 100);
            octet %= 100;
        }
        if (octet >= 10 || pos > 0) {
            str[pos++] = '0' + (octet / 10);
            octet %= 10;
        }
        str[pos++] = '0' + octet;
        if (i < 3) str[pos++] = '.';
    }
    str[pos] = '\0';
}

int string_to_ip(const char* str, ip_addr_t* ip) {
    int octet = 0, count = 0, pos = 0;
    
    for (int i = 0; str[i] != '\0' && count < 4; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            octet = octet * 10 + (str[i] - '0');
        } else if (str[i] == '.' || str[i] == '\0') {
            if (octet > 255) return -1;
            ip->addr[count++] = octet;
            octet = 0;
        } else {
            return -1;
        }
    }
    
    if (count == 3 && octet <= 255) {
        ip->addr[3] = octet;
        return 0;
    }
    
    return -1;
}

uint16_t calculate_checksum(const void* data, size_t len) {
    const uint16_t* ptr = (const uint16_t*)data;
    uint32_t sum = 0;
    
    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }
    
    if (len > 0) {
        sum += *(uint8_t*)ptr;
    }
    
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

// ============================================================================
// COMANDOS DE REDE
// ============================================================================

void cmd_ifconfig(void) {
    terminal_print("\nConfiguracoes de rede:\n");
    terminal_print("Interface: ");
    terminal_print(net_interface.name);
    terminal_print("\n");
    
    if (net_interface.enabled) {
        terminal_print("  Status: UP\n");
        
        char mac_str[18];
        mac_to_string(&net_interface.mac_address, mac_str);
        terminal_print("  MAC: ");
        terminal_print(mac_str);
        terminal_print("\n");
        
        char ip_str[16];
        ip_to_string(&net_interface.ip_address, ip_str);
        terminal_print("  IP: ");
        terminal_print(ip_str);
        terminal_print("\n");
        
        ip_to_string(&net_interface.subnet_mask, ip_str);
        terminal_print("  Mask: ");
        terminal_print(ip_str);
        terminal_print("\n");
        
        ip_to_string(&net_interface.gateway, ip_str);
        terminal_print("  Gateway: ");
        terminal_print(ip_str);
        terminal_print("\n");
    } else {
        terminal_print("  Status: DOWN\n");
    }
}

void cmd_ping(const char* target) {
    if (!net_interface.enabled) {
        terminal_print("Interface de rede nao disponivel\n");
        return;
    }
    
    ip_addr_t dst_ip;
    if (string_to_ip(target, &dst_ip) != 0) {
        terminal_print("Endereco IP invalido: ");
        terminal_print(target);
        terminal_print("\n");
        return;
    }
    
    terminal_print("PING ");
    terminal_print(target);
    terminal_print(" (");
    terminal_print(target);
    terminal_print("): 56 bytes de dados\n");
    
    // Simular 4 pings
    for (int i = 1; i <= 4; i++) {
        ping_send(&dst_ip, 1234, i);
        // Em um OS real, haveria delay aqui
    }
    
    terminal_print("\n--- ");
    terminal_print(target);
    terminal_print(" estatisticas de ping ---\n");
    terminal_print("4 pacotes transmitidos, 4 recebidos, 0% perda de pacotes\n");
    terminal_print("tempo round-trip min/avg/max/stddev = 1.0/1.0/1.0/0.0 ms\n");
}

void cmd_arp(void) {
    terminal_print("\nTabela ARP:\n");
    terminal_print("IP Address       HW Address         Type\n");
    terminal_print("-------------------------------------------\n");
    
    int count = 0;
    for (int i = 0; i < ARP_TABLE_SIZE; i++) {
        if (arp_table[i].valid) {
            char ip_str[16], mac_str[18];
            ip_to_string(&arp_table[i].ip, ip_str);
            mac_to_string(&arp_table[i].mac, mac_str);
            
            terminal_print(ip_str);
            // Padding para alinhar
            int ip_len = string_length(ip_str);
            for (int j = ip_len; j < 17; j++) {
                terminal_print(" ");
            }
            terminal_print(mac_str);
            terminal_print("  dynamic\n");
            count++;
        }
    }
    
    if (count == 0) {
        terminal_print("Nenhuma entrada encontrada\n");
    } else {
        terminal_print("\nTotal: ");
        terminal_print_dec(count);
        terminal_print(" entradas\n");
    }
}

void cmd_netstat(void) {
    terminal_print("\nEstatisticas de rede:\n");
    terminal_print("Conexoes ativas: 0\n");
    terminal_print("Interfaces ativas: ");
    terminal_print_dec(net_interface.enabled ? 1 : 0);
    terminal_print("\n");
    terminal_print("Pacotes enviados: N/A (modo demonstracao)\n");
    terminal_print("Pacotes recebidos: N/A (modo demonstracao)\n");
    terminal_print("Erros: 0\n");
}