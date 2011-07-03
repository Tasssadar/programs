#include "3piLibPack.h"
#include "packets.h"

void run()
{   
    display.print("3piCtrl");
    display.gotoXY(0, 1);
    display.send_data('0');
    display.gotoXY(7, 1);
    display.send_data('0');
    while(true)
    {
        
        //delay(1000);
        if(readPacket())
            handlePacket(&pkt);
    }
}
