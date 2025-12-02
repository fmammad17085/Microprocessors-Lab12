#include <Wire.h>
#define SLAVE_ADDR 0x10

void configureI2CSpeed(uint32_t freq)
{

    // SPECIAL CASE: 25 kHz
    if (freq == 25000)
    {

        TWSR = (TWSR & ~((1 << TWPS1) | (1 << TWPS0))) | (1 << TWPS0); // Set prescaler = 4; For 25 kHz, the formula gives TWBR > 255, so we use prescaler = 4, TWBR = 78  which produces ~25 kHz.

        TWBR = 78; // Set TWBR = 78; Formula: ((16000000/SCL)-16)/(2 x prescaler value) (Datasheet 21.5.2)

        Wire.begin();    // Initialize Wire (this resets TWBR/TWSR)

        // Reapply settings
        TWSR = (TWSR & ~((1 << TWPS1) | (1 << TWPS0))) | (1 << TWPS0); 
        TWBR = 78;

        Serial.println("Configured freq = 25000 Hz, Prescaler=4, TWBR=78");
        return;
    }

    // NORMAL CASE: 100 kHz or 400 kHz
    uint32_t twbr_val = (F_CPU / freq - 16) / 2; // Datasheet (21.5.2)
    if (twbr_val > 255) twbr_val = 255;  // Limit TWBR to 8-bit maximum

    TWSR &= ~((1 << TWPS1) | (1 << TWPS0));  // Set prescaler = 1

    TWBR = (uint8_t)twbr_val; // Set TWBR (12 for 400 kHz and 72 for 100kHz); Formula: ((16000000/SCL)-16)/(2 x prescaler value) (Datasheet 21.5.2)

    Wire.begin(); // Wire.begin overwrites TWBR, so call then reapply

    // Reapply settings
    TWSR &= ~((1 << TWPS1) | (1 << TWPS0));
    TWBR = (uint8_t)twbr_val;

    Serial.print("Configured freq = ");
    Serial.print(freq);
    Serial.print(" Hz, TWBR = ");
    Serial.println((uint8_t)twbr_val);
}

// Perform a write + read exchange with the slave
void exchange_once()
{
    // MASTER → SLAVE
    Wire.beginTransmission(SLAVE_ADDR); // Send START + SLA+W
    Wire.write(0x55);  // Send one byte to slave
    Wire.endTransmission(); // Send STOP

    // SLAVE → MASTER
    Wire.requestFrom(SLAVE_ADDR, 1); // Request 1 byte (SLA+R)
    while (Wire.available() == 0); // Wait for incoming byte
    uint8_t response = Wire.read(); // Read byte from slave
}

// Measure total time for 50 exchanges
uint32_t run_test()
{
    uint32_t t0 = micros(); // Capture start time
    for (int i = 0; i < 50; i++) // Run 50 write+read operations
        exchange_once();
    uint32_t t1 = micros(); // Capture end time
    return (t1 - t0);  // Total time in microseconds
}

void setup()
{
    Serial.begin(9600);
    delay(500);

    // TEST 1 — Default Wire speed (~100 kHz)
    Wire.begin(); // Initialize I2C default mode
    uint32_t t_default = run_test(); // Run timing test
    Serial.print("Default (~100 kHz) time (us): ");
    Serial.println(t_default);

    delay(1000); // Delay

    // TEST 2 — 25 kHz (Slow Mode)
    // Uses Prescaler = 4, TWBR = 78 (special case)
    configureI2CSpeed(25000);
    uint32_t t25 = run_test();
    Serial.print("25 kHz time (us): ");
    Serial.println(t25);

    delay(1000);

    // TEST 3 — 100 kHz (Standard Mode)
    // TWBR = 72, Prescaler = 1
    configureI2CSpeed(100000);
    uint32_t t100 = run_test();
    Serial.print("100 kHz time (us): ");
    Serial.println(t100);

    delay(1000);

    // TEST 4 — 400 kHz (Fast Mode)
    // TWBR = 12, Prescaler = 1
    // Actual speed ~270–320 kHz due to Uno limitations
    configureI2CSpeed(400000);
    uint32_t t400 = run_test();
    Serial.print("400 kHz time (us): ");
    Serial.println(t400);

    Serial.println("TEST COMPLETE");
}

void loop() {} // Nothing