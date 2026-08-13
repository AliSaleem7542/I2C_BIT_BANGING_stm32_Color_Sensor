#include "BitBangI2C.h"
#include "stdio.h"

extern TIM_HandleTypeDef htim2;

// ---------------- Microsecond delay using TIM2 ----------------
void delay_us(uint32_t us)
{
    uint32_t start = __HAL_TIM_GET_COUNTER(&htim2);
    while ((__HAL_TIM_GET_COUNTER(&htim2) - start) < us)
        ;
}

// ---------------- Low-level pin control ----------------
// SDA ko output-low ya input(release/high via pull-up) mode mein switch karna zaroori hai
// kyunke open-drain mode mein "High" bhejne ka matlab hai pin ko release karna (float),
// pull-up resistor usse High kar dega.

static void SDA_HIGH(void)
{
    HAL_GPIO_WritePin(I2C_SDA_PORT, I2C_SDA_PIN, GPIO_PIN_SET);
}
static void SDA_LOW(void)
{
    HAL_GPIO_WritePin(I2C_SDA_PORT, I2C_SDA_PIN, GPIO_PIN_RESET);
}
static void SCL_HIGH(void)
{
    HAL_GPIO_WritePin(I2C_SCL_PORT, I2C_SCL_PIN, GPIO_PIN_SET);
}
static void SCL_LOW(void)
{
    HAL_GPIO_WritePin(I2C_SCL_PORT, I2C_SCL_PIN, GPIO_PIN_RESET);
}
static uint8_t SDA_READ(void)
{
    return HAL_GPIO_ReadPin(I2C_SDA_PORT, I2C_SDA_PIN);
}

// ---------------- Init ----------------
void BitBangI2C_Init(void)
{
    // GPIO already CubeMX se Open-Drain configure hua hai (.ioc mein)
    // Idle state: dono lines High (released)
    SDA_HIGH();
    SCL_HIGH();
    delay_us(5);
}

// ---------------- Start Condition ----------------
// SCL High rehte hue SDA High->Low
void I2C_Start(void)
{
    SDA_HIGH();
    SCL_HIGH();
    delay_us(5);
    SDA_LOW();
    delay_us(5);
    SCL_LOW();
    delay_us(5);
}

// ---------------- Stop Condition ----------------
// SCL High rehte hue SDA Low->High
void I2C_Stop(void)
{
    SDA_LOW();
    delay_us(5);
    SCL_HIGH();
    delay_us(5);
    SDA_HIGH();
    delay_us(5);
}

// ---------------- Single Bit Write ----------------
void I2C_WriteBit(uint8_t bit)
{
    if (bit)
        SDA_HIGH();
    else
        SDA_LOW();
    delay_us(3); // setup time - SDA stable hone dena
    SCL_HIGH();
    delay_us(5); // yahan slave SDA ko sample karega
    SCL_LOW();
    delay_us(3);
}

// ---------------- Single Bit Read ----------------
uint8_t I2C_ReadBit(void)
{
    // SDA_HIGH();          // SDA release karo (open-drain, pull-up High kar dega)
    // delay_us(3);
    // SCL_HIGH();
    // delay_us(50);
    // uint8_t bit = SDA_READ();
    // SCL_LOW();
    // delay_us(3);
    // return bit;

    SDA_HIGH(); // release SDA
    delay_us(5);

    // printf("Before SCL: SDA=%d\r\n", SDA_READ());

    SCL_HIGH();
    delay_us(20);

    uint8_t bit = SDA_READ();

    // printf("During SCL: SDA=%d\r\n", bit);

    SCL_LOW();
    delay_us(5);

    return bit;
}

// ---------------- Write Full Byte (MSB first), return ACK/NACK ----------------
uint8_t I2C_WriteByte(uint8_t byte)
{
    for (int i = 7; i >= 0; i--)
    {
        I2C_WriteBit((byte >> i) & 0x01);
    }
    // 9th clock = ACK bit check
    uint8_t ack = I2C_ReadBit(); // 0 = ACK (slave responded), 1 = NACK
    return ack;
}

// ---------------- Read Full Byte, send ACK/NACK after ----------------
uint8_t I2C_ReadByte(uint8_t ack)
{
    uint8_t byte = 0;
    for (int i = 7; i >= 0; i--)
    {
        byte |= (I2C_ReadBit() << i);
    }
    I2C_WriteBit(ack ? 0 : 1); // 0 = ACK bhejna hai, 1 = NACK (last byte pe)
    return byte;
    // uint8_t byte = 0;

    // for (int i = 7; i >= 0; i--)
    // {
    //     uint8_t bit = I2C_ReadBit();

    //     printf("bit%d = %d\r\n", i, bit);

    //     byte |= (bit << i);
    // }

    // I2C_WriteBit(ack ? 0 : 1);

    // return byte;
}

// ---------------- TCA9548A Mux Channel Select ----------------
void select_mux_channel(uint8_t channel)
{
    I2C_Start();
    I2C_WriteByte(0x70 << 1);
    I2C_WriteByte(1 << channel);
    I2C_Stop();
}

uint8_t I2C_ReadSDA(void)
{
    SDA_HIGH();
    delay_us(10);
    return SDA_READ();
}
void I2C_Scan(void)
{
    printf("I2C SCAN START\r\n");

    for (uint8_t addr = 1; addr < 127; addr++)
    {
        I2C_Start();

        uint8_t ack = I2C_WriteByte(addr << 1);

        I2C_Stop();

        if (ack == 0)
        {
            printf("FOUND: 0x%02X\r\n", addr);
        }

        delay_us(100);
    }

    printf("I2C SCAN END\r\n");
}

uint16_t TCS34727_Read16(uint8_t reg)
{
    uint8_t low, high;

    I2C_Start();

    // Sensor address + WRITE
    I2C_WriteByte(0x29 << 1);

    // Command bit + register address
    I2C_WriteByte(0x80 | reg);

    // Repeated START
    I2C_Start();

    // Sensor address + READ
    I2C_WriteByte((0x29 << 1) | 1);

    // Read LOW byte, send ACK
    low = I2C_ReadByte(1);

    // Read HIGH byte, send NACK
    high = I2C_ReadByte(0);

    I2C_Stop();

    return ((uint16_t)high << 8) | low;
}
void TCS34727_ReadRGBC(uint16_t *c,
                       uint16_t *r,
                       uint16_t *g,
                       uint16_t *b)
{
    /*
     * TCS34727 registers:
     *
     * C: 0x14 / 0x15
     * R: 0x16 / 0x17
     * G: 0x18 / 0x19
     * B: 0x1A / 0x1B
     */

    *c = TCS34727_Read16(0x14);
    *r = TCS34727_Read16(0x16);
    *g = TCS34727_Read16(0x18);
    *b = TCS34727_Read16(0x1A);
}
void TCA9548A_SelectChannel(uint8_t channel)
{
    if (channel > 7)
        return;

    I2C_Start();

    I2C_WriteByte(TCA9548A_ADDR << 1); // MUX write address
    I2C_WriteByte(1 << channel);       // Select channel

    I2C_Stop();

    HAL_Delay(2);
}

void TCS34727_Enable(void)
{
    I2C_Start();

    I2C_WriteByte(TCS34727_ADDR << 1); // WRITE
    I2C_WriteByte(0x80);               // ENABLE register
    I2C_WriteByte(0x03);               // PON + AEN

    I2C_Stop();

    HAL_Delay(10);
}