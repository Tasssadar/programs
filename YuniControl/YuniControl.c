#include "yunimin3.h"

enum states
{
    STATE_CORRECTION  = 0x01,
    STATE_RECORD      = 0x02,
    STATE_PLAY        = 0x04,
    STATE_ERASING     = 0x08,
    STATE_CORRECTION2 = 0x10,
    STATE_COLLISION   = 0x20,
    STATE_LOCKED      = 0x40,
};

uint8_t state = 0;
bool sendEmergency = false;
bool emergencySent = false;

#include "func.h"
#include "movement.h"
#include "packets.h"

void run()
{
    state = STATE_CORRECTION;
    while(true)
    {
        if((moveflags == MOVE_FORWARD || moveflags == MOVE_BACKWARD) && (state & STATE_CORRECTION))
            MovementCorrection();

        if(!readPacket())
           continue;

        handlePacket(&pkt);
    }
}
