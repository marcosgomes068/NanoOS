#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>
#include <stddef.h>

// ============================================================================
// DEFINIÇÕES DE REDE - NanoOS
// ============================================================================

// Tamanhos padrão
#define ETH_FRAME_SIZE      1518
#define ETH_HEADER_SIZE     14
#define ETH_ADDR_LEN        6
#define IP_ADDR_LEN         4
#define ARP_TABLE_SIZE      32

// Tipos Ethernet
#define ETH_TYPE_IP         0x0800
#define ETH_TYPE_ARP        0x0806

// Protocolos IP
#define IP_PROTO_ICMP       1
#define IP_PROTO_TCP        6
#define IP_PROTO_UDP        17

// ============================================================================
// ESTRUTURAS DE DADOS
// ============================================================================

// Endereço MAC
typedef struct {
    uint8_t addr[ETH_ADDR_LEN];
} __attribute__((packed)) mac_addr_t;

// Endereço IP
typedef struct {
    uint8_t addr[IP_ADDR_LEN];
} __attribute__((packed)) ip_addr_t;

// Cabeçalho Ethernet
typedef struct {
    mac_addr_t dst_mac;      // MAC destino
    mac_addr_t src_mac;      // MAC origem
    uint16_t type;           // Tipo do protocolo
} __attribute__((packed)) eth_header_t;

// Cabeçalho IP
typedef struct {
    uint8_t version_ihl;     // Versão (4 bits) + IHL (4 bits)
    uint8_t tos;             // Type of Service
    uint16_t length;         // Tamanho total
    uint16_t id;             // Identificação
    uint16_t flags_offset;   // Flags (3 bits) + Fragment Offset (13 bits)
    uint8_t ttl;             // Time to Live
    uint8_t protocol;        // Protocolo
    uint16_t checksum;       // Checksum
    ip_addr_t src_ip;        // IP origem
    ip_addr_t dst_ip;        // IP destino
} __attribute__((packed)) ip_header_t;

// Entrada da tabela ARP
typedef struct {
    ip_addr_t ip;
    mac_addr_t mac;
    uint8_t valid;
} arp_entry_t;

// Interface de rede
typedef struct {
    mac_addr_t mac_address;
    ip_addr_t ip_address;
    ip_addr_t subnet_mask;
    ip_addr_t gateway;
    uint8_t enabled;
    char name[16];
} network_interface_t;

// ============================================================================
// DRIVER ETHERNET (RTL8139)
// ============================================================================

// Registradores RTL8139
#define RTL8139_IDR0        0x00    // ID Registers
#define RTL8139_IDR4        0x04
#define RTL8139_MAR0        0x08    // Multicast Registers
#define RTL8139_MAR4        0x0C
#define RTL8139_TSAD0       0x20    // Transmit Start Address
#define RTL8139_TSAD1       0x24
#define RTL8139_TSAD2       0x28
#define RTL8139_TSAD3       0x2C
#define RTL8139_TSD0        0x10    // Transmit Status
#define RTL8139_TSD1        0x14
#define RTL8139_TSD2        0x18
#define RTL8139_TSD3        0x1C
#define RTL8139_RBSTART     0x30    // Receive Buffer Start
#define RTL8139_ERBCR       0x34    // Early RX Byte Count
#define RTL8139_ERSR        0x36    // Early RX Status
#define RTL8139_CMD         0x37    // Command Register
#define RTL8139_CAPR        0x38    // Current Address of Packet Read
#define RTL8139_CBR         0x3A    // Current Buffer Address
#define RTL8139_IMR         0x3C    // Interrupt Mask
#define RTL8139_ISR         0x3E    // Interrupt Status
#define RTL8139_TCR         0x40    // Transmit Configuration
#define RTL8139_RCR         0x44    // Receive Configuration
#define RTL8139_TCTR        0x48    // Timer Count
#define RTL8139_MPC         0x4C    // Missed Packet Counter
#define RTL8139_9346CR      0x50    // EEPROM Command
#define RTL8139_CONFIG0     0x51    // Configuration Register 0
#define RTL8139_CONFIG1     0x52    // Configuration Register 1
#define RTL8139_MSR         0x58    // Media Status Register
#define RTL8139_CONFIG4     0x5A    // Configuration Register 4
#define RTL8139_MULINT      0x5C    // Multiple Interrupt Select
#define RTL8139_BMCR        0x62    // Basic Mode Control Register
#define RTL8139_BMSR        0x64    // Basic Mode Status Register

// Comandos
#define RTL8139_CMD_RST     0x10    // Reset
#define RTL8139_CMD_RE      0x08    // Receiver Enable
#define RTL8139_CMD_TE      0x04    // Transmitter Enable
#define RTL8139_CMD_BUFE    0x01    // Buffer Empty

// Estado do driver
typedef struct {
    uint16_t io_base;           // Endereço base I/O
    uint32_t tx_buffer[4];      // Buffers de transmissão
    uint32_t rx_buffer;         // Buffer de recepção
    uint8_t tx_cur;             // Índice atual TX
    uint16_t rx_cur;            // Índice atual RX
    network_interface_t interface;
} rtl8139_device_t;

// ============================================================================
// FUNÇÕES PÚBLICAS
// ============================================================================

// Inicialização
void network_init(void);
int rtl8139_init(void);
void network_interface_init(void);

// Transmissão/Recepção
int eth_send_frame(const uint8_t* data, size_t len, const mac_addr_t* dst_mac, uint16_t type);
void eth_receive_frame(void);
void network_process_packets(void);

// Utilidades
void mac_to_string(const mac_addr_t* mac, char* str);
void ip_to_string(const ip_addr_t* ip, char* str);
int string_to_ip(const char* str, ip_addr_t* ip);
uint16_t calculate_checksum(const void* data, size_t len);

// ARP
void arp_init(void);
int arp_lookup(const ip_addr_t* ip, mac_addr_t* mac);
void arp_add_entry(const ip_addr_t* ip, const mac_addr_t* mac);
void arp_request(const ip_addr_t* ip);

// IP
int ip_send(const uint8_t* data, size_t len, const ip_addr_t* dst_ip, uint8_t protocol);
void ip_receive(const uint8_t* packet, size_t len);

// ICMP (Ping)
void icmp_reply(const ip_addr_t* src_ip, const uint8_t* data, size_t len);
int ping_send(const ip_addr_t* dst_ip, uint16_t id, uint16_t seq);

// Comandos de rede
void cmd_ifconfig(void);
void cmd_ping(const char* target);
void cmd_arp(void);
void cmd_netstat(void);

#endif // NETWORK_H