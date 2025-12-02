#include <Wire.h>

#define SLAVE_ADDR 0x10

volatile uint8_t last_received = 0;

void receiveEvent(int howMany)
{
    if (howMany >= 1)
        last_received = Wire.read();   // store received byte
}

void requestEvent()
{
    Wire.write(last_received);         // send it back
}

void setup()
{
    Wire.begin(SLAVE_ADDR);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);
}

void loop()
{
    // nothing
}