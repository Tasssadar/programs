#include "3piLibPack.h"
#include "packets.h"

void run()
{
    while(true)
    {
        if(readPacket())
            handlePacket(&pkt);
    }
}
