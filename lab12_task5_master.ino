// Sends 1 byte to slave (0x01 = pressed, 0x00 = released)
// Receives 1 byte back from slave and updates LED

// MESSAGE DESIGN:
// We send exactly one byte because I2C transmits 8-bit frames.
// The button state is binary, so 0x01 or 0x00 is the simplest representation.

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#define SLAVE_ADDR 0x10      // 7-bit address (0x10 is arbitrary)
#define BUTTON_A   PD2
#define LED_A      PD3


void TWI_MasterInit(void)
{
    // Enable TWI module (PRTWI = 0)
    PRR &= ~(1 << PRTWI);  

    // Prescaler = 1
    TWSR &= ~((1 << TWPS1) | (1 << TWPS0)); 

    // TWBR formula 21.5.2 → 100 kHz
    TWBR = 72; 

    // Enable TWI hardware
    TWCR = (1 << TWEN); 
}


// SEND START CONDITION 
void TWI_Start(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));  
}

void TWI_Stop(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void TWI_Write(uint8_t data)
{
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT))); 
}

uint8_t TWI_Read_ACK(void)
{
    TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

uint8_t TWI_Read_NACK(void)
{
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}


// IO setup
void io_init(void)
{
    DDRD &= ~(1 << BUTTON_A);    // button input
    PORTD |= (1 << BUTTON_A);    // pull-up

    DDRD |= (1 << LED_A);        // LED output
}


// MAIN LOOP
int main(void)
{
    io_init();
    TWI_MasterInit();

    // ENABLE INTERNAL PULL-UPS ON SDA/SCL   <<<<<<<< ADDED
    PORTC |= (1 << PC4) | (1 << PC5);

    uint8_t button_state = 0;
    uint8_t slave_reply  = 0;

    static uint8_t prev_button_state = 0;
    uint8_t toggle_event = 0;


    while (1)
    {
        // Active-low button
        button_state = !(PIND & (1 << BUTTON_A));

        toggle_event = 0;
        if (button_state == 1 && prev_button_state == 0)
        {
            toggle_event = 1;   // rising edge → ONE press event
        }
        prev_button_state = button_state;


        // SEND TO SLAVE 
        TWI_Start();
        TWI_Write((SLAVE_ADDR << 1) | 0);     // SLA+W
        TWI_Write(toggle_event ? 0x01 : 0x00);
        TWI_Stop();

        _delay_ms(5);

        // READ FROM SLAVE
        TWI_Start();
        TWI_Write((SLAVE_ADDR << 1) | 1);     // SLA+R
        slave_reply = TWI_Read_NACK();
        TWI_Stop();

        // UPDATE LED_A
        static uint8_t led_a_state = 0;

        if (slave_reply == 0x01)
        {
            led_a_state ^= 1;

            if (led_a_state)
                PORTD |= (1 << LED_A);
            else
                PORTD &= ~(1 << LED_A);
        }

        _delay_ms(100);
    }
}
