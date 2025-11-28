#include <Wire.h>

#define SLAVE_ADDR 0x10

//------------------------------------------------------------
//  Apply custom I2C bus speed AFTER Wire.begin()
//  SCL = F_CPU / (16 + 2 * TWBR * 4^Prescaler)
//  Prescaler = 1 (TWPS1 = TWPS0 = 0)
//------------------------------------------------------------
void setI2CSpeed(uint32_t freq)
{
    // Prescaler = 1
    TWSR &= ~((1 << TWPS1) | (1 << TWPS0));

    // Compute TWBR value
    uint32_t twbr_val = ((F_CPU / freq) - 16) / 2;
    TWBR = (uint8_t)twbr_val;
}

//------------------------------------------------------------
//  Perform one request/response exchange
//------------------------------------------------------------
void exchange_once()
{
    // MASTER → SLAVE
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(0x55);          // send any byte
    Wire.endTransmission();

    // SLAVE → MASTER
    Wire.requestFrom(SLAVE_ADDR, 1);
    while (Wire.available() == 0);
    uint8_t x = Wire.read();
}


//  Run 50-message test and return elapsed time in microseconds

uint32_t run_test()
{
    uint32_t t0 = micros();
    for (int i = 0; i < 50; i++)
        exchange_once();
    uint32_t t1 = micros();
    return (t1 - t0);
}


//  MAIN  

void setup()
{
    Serial.begin(9600);

    // DEFAULT SPEED TEST (Wire's default ~100 kHz)

    Wire.begin();
    uint32_t t_default = run_test();
    Serial.print("Default Wire speed time (us): ");
    Serial.println(t_default);

    delay(1000);


    // 25 kHz TEST
    Wire.begin();          // reinitialize Wire library
    setI2CSpeed(25000);    // apply custom clock
    uint32_t t_25 = run_test();
    Serial.print("25 kHz time (us): ");
    Serial.println(t_25);

    delay(1000);



    // 100 kHz TEST

    Wire.begin();
    setI2CSpeed(100000);
    uint32_t t_100 = run_test();
    Serial.print("100 kHz time (us): ");
    Serial.println(t_100);

    delay(1000);


    // 400 kHz TEST

    Wire.begin();
    setI2CSpeed(400000);
    uint32_t t_400 = run_test();
    Serial.print("400 kHz time (us): ");
    Serial.println(t_400);

    Serial.println("\n--- TEST COMPLETE ---");
}

void loop() {}
