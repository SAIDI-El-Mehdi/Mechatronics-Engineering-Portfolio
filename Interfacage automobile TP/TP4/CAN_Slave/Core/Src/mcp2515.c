#include "mcp2515.h"

extern SPI_HandleTypeDef hspi1;

#define CS_LOW()   HAL_GPIO_WritePin(MCP_CS_GPIO_Port, MCP_CS_Pin, GPIO_PIN_RESET)
#define CS_HIGH()  HAL_GPIO_WritePin(MCP_CS_GPIO_Port, MCP_CS_Pin, GPIO_PIN_SET)

/* === Fonctions internes bas niveau SPI === */
static void mcp_write_reg(uint8_t addr, uint8_t value)
{
    uint8_t buf[3] = { MCP_WRITE, addr, value };
    CS_LOW();
    HAL_SPI_Transmit(&hspi1, buf, 3, 100);
    CS_HIGH();
}

static uint8_t mcp_read_reg(uint8_t addr)
{
    uint8_t tx[2] = { MCP_READ, addr };
    uint8_t rx = 0;
    CS_LOW();
    HAL_SPI_Transmit(&hspi1, tx, 2, 100);
    HAL_SPI_Receive(&hspi1, &rx, 1, 100);
    CS_HIGH();
    return rx;
}

static void mcp_bit_modify(uint8_t addr, uint8_t mask, uint8_t value)
{
    uint8_t buf[4] = { MCP_BITMOD, addr, mask, value };
    CS_LOW();
    HAL_SPI_Transmit(&hspi1, buf, 4, 100);
    CS_HIGH();
}

/* === Reset du MCP2515 === */
void MCP2515_Reset(void)
{
    uint8_t cmd = MCP_RESET;
    CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    CS_HIGH();
    HAL_Delay(10);
}

/* === Initialisation : 500 kbps avec quartz 8 MHz === */
void MCP2515_Init(void)
{
    MCP2515_Reset();

    // Passer en mode configuration
    mcp_write_reg(MCP_CANCTRL, MODE_CONFIG);
    HAL_Delay(10);

    // Bit timing pour 500 kbps avec quartz 8 MHz
    mcp_write_reg(MCP_CNF1, 0x00);
    mcp_write_reg(MCP_CNF2, 0x90);
    mcp_write_reg(MCP_CNF3, 0x02);

    // Activer interruption sur réception RX0
    mcp_write_reg(MCP_CANINTE, 0x01);

    // RXB0 : recevoir tous les messages (pas de filtre)
    mcp_write_reg(MCP_RXB0CTRL, 0x60);

    // Passer en mode normal (CAN bus réel)
    mcp_write_reg(MCP_CANCTRL, MODE_NORMAL);
    HAL_Delay(10);
}

/* === Envoi d'une trame CAN === */
uint8_t MCP2515_SendFrame(CAN_Frame_t *frame)
{
    // Identifiant standard sur 11 bits
    mcp_write_reg(MCP_TXB0SIDH, (uint8_t)(frame->id >> 3));
    mcp_write_reg(MCP_TXB0SIDL, (uint8_t)((frame->id & 0x07) << 5));

    // Longueur des données (DLC)
    mcp_write_reg(MCP_TXB0DLC, frame->dlc & 0x0F);

    // Données utiles
    for (uint8_t i = 0; i < frame->dlc; i++) {
        mcp_write_reg(MCP_TXB0D0 + i, frame->data[i]);
    }

    // Demande de transmission (Request To Send sur TX0)
    uint8_t cmd = MCP_RTS_TX0;
    CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    CS_HIGH();

    return 1;
}

/* === Vérifie si un message est reçu (flag RX0IF) === */
uint8_t MCP2515_CheckReceive(void)
{
    uint8_t flags = mcp_read_reg(MCP_CANINTF);
    return (flags & 0x01);
}

/* === Lecture d'une trame reçue === */
uint8_t MCP2515_ReadFrame(CAN_Frame_t *frame)
{
    uint8_t sidh = mcp_read_reg(MCP_RXB0SIDH);
    uint8_t sidl = mcp_read_reg(MCP_RXB0SIDL);
    frame->id = ((uint16_t)sidh << 3) | (sidl >> 5);

    frame->dlc = mcp_read_reg(MCP_RXB0DLC) & 0x0F;
    if (frame->dlc > 8) frame->dlc = 8;

    for (uint8_t i = 0; i < frame->dlc; i++) {
        frame->data[i] = mcp_read_reg(MCP_RXB0D0 + i);
    }

    // Effacer le flag RX0IF pour la prochaine réception
    mcp_bit_modify(MCP_CANINTF, 0x01, 0x00);
    return 1;
}
