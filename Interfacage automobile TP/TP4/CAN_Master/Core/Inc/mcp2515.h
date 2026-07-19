#ifndef INC_MCP2515_H_
#define INC_MCP2515_H_

#include "main.h"
#include <stdint.h>

/* === Registres MCP2515 === */
#define MCP_CANCTRL    0x0F
#define MCP_CANSTAT    0x0E
#define MCP_CNF1       0x2A
#define MCP_CNF2       0x29
#define MCP_CNF3       0x28
#define MCP_TXB0CTRL   0x30
#define MCP_TXB0SIDH   0x31
#define MCP_TXB0SIDL   0x32
#define MCP_TXB0DLC    0x35
#define MCP_TXB0D0     0x36
#define MCP_RXB0CTRL   0x60
#define MCP_RXB0SIDH   0x61
#define MCP_RXB0SIDL   0x62
#define MCP_RXB0DLC    0x65
#define MCP_RXB0D0     0x66
#define MCP_CANINTE    0x2B
#define MCP_CANINTF    0x2C

/* === Commandes SPI === */
#define MCP_RESET       0xC0
#define MCP_READ        0x03
#define MCP_WRITE       0x02
#define MCP_RTS_TX0     0x81
#define MCP_READ_STATUS 0xA0
#define MCP_BITMOD      0x05

/* === Modes du contrôleur CAN === */
#define MODE_NORMAL    0x00
#define MODE_CONFIG    0x80
#define MODE_LOOPBACK  0x40

/* === Structure d'une trame CAN === */
typedef struct {
    uint16_t id;        // identifiant standard 11 bits
    uint8_t  dlc;       // longueur des données (0 à 8)
    uint8_t  data[8];   // données utiles
} CAN_Frame_t;

/* === API publique === */
void    MCP2515_Reset(void);
void    MCP2515_Init(void);
uint8_t MCP2515_SendFrame(CAN_Frame_t *frame);
uint8_t MCP2515_ReadFrame(CAN_Frame_t *frame);
uint8_t MCP2515_CheckReceive(void);

#endif /* INC_MCP2515_H_ */
