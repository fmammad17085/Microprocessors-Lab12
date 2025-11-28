#include <Wire.h>
#define SLAVE_ADDR 0x10

// --------------------------------------------------------
// PRESET I2C SPEEDS
// mode = 0 → 25 kHz
// mode = 1 → 100 kHz
// mode = 2 → 400 kHz
// --------------------------------------------------------
void setI2CSpeedPreset(uint8_t mode)
{
    if (mode == 0)
    {
        // 25 kHz: Prescaler = 4, TWBR = 78
        TWSR = (TWSR & ~((1 << TWPS1) | (1 << TWPS0))) | (1 << TWPS0);
        TWBR = 78;
    }
    else if (mode == 1)
    {
        // 100 kHz: Prescaler = 1, TWBR = 72
        TWSR &= ~((1 << TWPS1) | (1 << TWPS0));
        TWBR = 72;
    }
    else if (mode == 2)
    {
        // 400 kHz: Prescaler = 1, TWBR = 12
        TWSR &= ~((1 << TWPS1) | (1 << TWPS0));
        TWBR = 12;
    }
}

void setup()
{
    Wire.begin();       // MUST come first
    setI2CSpeedPreset(2);    // <<< CHANGE 0→1→2 to test 25k/100k/400k
}

void loop()
{
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(0x55);
    Wire.endTransmission();
}
