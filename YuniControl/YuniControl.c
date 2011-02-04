#include "yunimin3.h"

enum states
{
    STATE_CORRECTION  = 0x01,
    STATE_RECORD      = 0x02,
    STATE_PLAY        = 0x04,
    STATE_ERASING     = 0x08,
    STATE_CORRECTION2 = 0x10,
    STATE_COLLISION   = 0x20
};

uint8_t state = 0;

#include "func.h"
#include "movement.h"
#include "packets.h"

void run()
{
    char c;
    state |= STATE_CORRECTION;
    while(true)
    {
        if((state & STATE_CORRECTION) && (moveflags == MOVE_FORWARD || moveflags == MOVE_BACKWARD))
            MovementCorrection();
    
        if(!readPacket())
           continue;

        handlePacket(&pkt);
    }
}
