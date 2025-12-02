#include <Wire.h>
#define SLAVE_ADDR 0x10

void setI2CSpeedPreset(uint8_t mode)
{
    if (mode == 0)
    {
        // 25 kHz: Prescaler = 4, TWBR = 78; Formula: ((16000000/SCL)-16)/(2 x prescaler value) (Datasheet 21.5.2)
        TWSR = (TWSR & ~((1 << TWPS1) | (1 << TWPS0))) | (1 << TWPS0);
        TWBR = 78;
    }
    else if (mode == 1)
    {
        // 100 kHz: Prescaler = 1, TWBR = 72; Formula: ((16000000/SCL)-16)/(2 x prescaler value) (Datasheet 21.5.2)
        TWSR &= ~((1 << TWPS1) | (1 << TWPS0));
        TWBR = 72;
    }
    else if (mode == 2)
    {
        // 400 kHz: Prescaler = 1, TWBR = 12; Formula: ((16000000/SCL)-16)/(2 x prescaler value) (Datasheet 21.5.2)
        TWSR &= ~((1 << TWPS1) | (1 << TWPS0));
        TWBR = 12;
    }
}

void setup()
{
    Wire.begin(); // Initialize I2C hardware in master mode
    setI2CSpeedPreset(1);  // Select speed preset (0=25k, 1=100k, 2=400k)
}

void loop()
{
    Wire.beginTransmission(SLAVE_ADDR); // Send START + SLA+W to slave
    Wire.write(0x55);  // Write one byte to slave
    Wire.endTransmission(); // Send STOP and finish I2C transaction
}