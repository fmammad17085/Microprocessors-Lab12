
// BOARD B – I2C SLAVE 
// Receives 1 byte from master - updates LED_B
// Sends 1 byte back - state of BUTTON_B
// MESSAGE DESIGN identical to master (0x01 / 0x00)


#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#define SLAVE_ADDR 0x10
#define BUTTON_B   PD2
#define LED_B      PD3

void TWI_SlaveInit(uint8_t address)
{
    // Enable TWI clock
    PRR &= ~(1 << PRTWI); // Datasheet 9.11.3, PRR controls which modules are turned off. 0 = ENABLE T

    // Set 7-bit address (shifted left)
    TWAR = (address << 1); // Table 21.9.5, TWI Adress Regsiter, stores slave's 7-bit address, 0 is "general call"

    // Enable ACK + TWI
    TWCR = (1 << TWEA) | (1 << TWEN); // TWI Control Register, main bits:

    //TWEN — enable TWI hardware
    //WINT — interrupt flag (set when an operation completes)
    //TWSTA — send START condition
    //TWSTO — send STOP condition
    //TWEA — send ACK after reading
    //TWINT = 1 - “start this operation now”
}

    //IO Setup
void io_init(void)
{
    DDRD &= ~(1 << BUTTON_B); // input
    PORTD |= (1 << BUTTON_B); // pullup

    DDRD |= (1 << LED_B);
}


// MAIN LOOP (wait for commands from master)

int main(void)
{
    io_init();
    PORTC |= (1 << PC4) | (1 << PC5);
    TWI_SlaveInit(SLAVE_ADDR);

    uint8_t received = 0;
    uint8_t button_b_state = 0;

    static uint8_t led_b_state = 0;   // NEW toggle state
    static uint8_t prev_button_b_state = 0;  // NEW for reply toggle
    uint8_t toggle_event_b = 0;  // NEW


    while (1)
    {
        // Wait until an I2C event completes
        while (!(TWCR & (1 << TWINT))); 

        // Read status (mask lower 3 bits)
        uint8_t status = TWSR & 0xF8; // TWI Status Register

        //0x60 – SLA+W received: slave addressed for writing
        //0x80 – Data byte received from master
        //0xA8 – SLA+R received: master requests data
        //0xC0 – Data sent, master ended read



        // MASTER to SLAVE WRITE (SLA+W) Table 21-5. Status Codes for Slave Receiver Mode
        if (status == 0x60 || status == 0x68)
        {
            // Own address + Write received, ACK returned
            TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN); // TWI Control Register, Start operation now, send ACK after reading, enable TWI
        }


        // DATA RECEIVED FROM MASTER, master sent 1 byte after SLA+W, read from TWDR, ACK it
        else if (status == 0x80)
        {
            received = TWDR;   // Read received byte, TWI Data Register

        if (received == 0x01)
        {
            led_b_state ^= 1;  // toggle

            if (led_b_state)
                PORTD |= (1 << LED_B);
            else
                PORTD &= ~(1 << LED_B);
}


            TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
        }


        // MASTER from SLAVE READ (SLA+R) Table 21.6 Status Codes for Slave Transmitter Mode
        else if (status == 0xA8 || status == 0xB0)
        {
            // Master requested data, so prepare reply
        button_b_state = !(PIND & (1 << BUTTON_B)); // active-low

        toggle_event_b = 0;
        if (button_b_state == 1 && prev_button_b_state == 0)
        {
            toggle_event_b = 1;   // rising edge
        }
        prev_button_b_state = button_b_state;

        TWDR = toggle_event_b ? 0x01 : 0x00;  // send TOGGLE event


            TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
        }


        // MASTER DONE READING 
        else if (status == 0xC0 || status == 0xC8)
        {
            // Data sent, master returned NACK or ACK (end of read)
            TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
        }

        else
        {
            TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
        }
    }
}