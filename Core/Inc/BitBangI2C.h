#ifndef BITBANG_I2C_H
#define BITBANG_I2C_H

#include "main.h" // GPIO_TypeDef, TIM handle types ke liye

// ---- Pin Definitions (apne actual pins se match karo) ----
// ---- Pin Definitions ----
#define I2C_SDA_PORT GPIOG
#define I2C_SDA_PIN GPIO_PIN_6
#define I2C_SCL_PORT GPIOG
#define I2C_SCL_PIN GPIO_PIN_5
#define TCA9548A_ADDR 0x70
#define TCS34727_ADDR 0x29
// ---- Function Prototypes ----
void BitBangI2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_WriteBit(uint8_t bit);
uint8_t I2C_ReadBit(void);
uint8_t I2C_WriteByte(uint8_t byte); // returns ACK (0) / NACK (1)
uint8_t I2C_ReadByte(uint8_t ack);   // ack=1 to send ACK after read, 0 for NACK
uint8_t I2C_ReadSDA(void);
uint16_t TCS34727_Read16(uint8_t reg);
void I2C_Scan(void);
void delay_us(uint32_t us);
void select_mux_channel(uint8_t channel);
void TCS34727_ReadRGBC(uint16_t *c, uint16_t *r, uint16_t *g, uint16_t *b);
void TCA9548A_SelectChannel(uint8_t channel);
void TCS34727_Enable(void);

#endif