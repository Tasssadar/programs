#define DEVICE_ADDRESS 0x02
#define MASTER_ADDRESS 0xEF

#include "base.h"
#include "usart.h"
#include "packets.h"
#include "buttons.h"

inline void init()
{
    init_rs232();
    init_buttons();
    DDRD |= (1<<PD6)|(1<<PD7);
    PORTD |= (1<<PD6);
}

inline void clean()
{
    clean_rs232();
    clean_buttons();
}

int main()
{
    sei();
    init();
    
    while(true)
    {
        if(readPacket())
            handlePacket(&pkt);
        _delay_ms(10);
        if(led)
        {
            if(ledCounter >= 10)
                PORTD &= ~(1<<PD7);
            else
                ++ledCounter;
        }
    }
    
    cli();
    clean();
}


