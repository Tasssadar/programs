#include "3piLibPack.h"
#include "packets.h"

void run()
{   
    display.print("3piCtrl");
    display.printToXY("0", 0, 1);
    display.printToXY("0", 7, 1);
    while(true)
    {
        if(readPacket())
            handlePacket(&pkt);
    }
}
