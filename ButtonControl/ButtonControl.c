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
    }
    
    cli();
    clean();
}


