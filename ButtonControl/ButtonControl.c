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
    DDRD = 0xFF;
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
       //char ch;
       //if(rs232.peek(ch))
            //rs232.sendCharacter(ch);
        if(readPacket())
            handlePacket(&pkt);
    }
    
    cli();
    clean();
}


