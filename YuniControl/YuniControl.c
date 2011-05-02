#define DEVICE_ADDRESS 0x01
#define MASTER_ADDRESS 0xEF

bool checkForStart = true;
void StartMatch();

#include "yunimin3.h"

#define PING 1

enum states
{
    STATE_BUTTON      = 0x01,
    STATE_RECORD      = 0x02,
    STATE_PLAY        = 0x04,
    STATE_ERASING     = 0x08,
    STATE_CORRECTION2 = 0x10,
    STATE_COLLISION   = 0x20,
    STATE_LOCKED      = 0x40,
    STATE_PAUSED      = 0x80,
};

#define RANGE_THRESHOLD  35

volatile uint8_t state = 0;
bool sendEmergency = false;
bool emergencySent = false;
#ifdef PING
 uint32_t lastTime = getTickCount();
 uint32_t thisTime = 0;
 #define PING_TIME ((700000) * JUNIOR_WAIT_MUL / JUNIOR_WAIT_DIV)
 uint32_t pingTimer = PING_TIME;
 uint32_t diff = 0;

 #define RANGE_TIME ((70000) * JUNIOR_WAIT_MUL / JUNIOR_WAIT_DIV)
 uint32_t rangeTimer = RANGE_TIME;
 bool checkRangeNow = false;

 bool doReel = false;
 uint32_t reelStopTimer = ((1500000) * JUNIOR_WAIT_MUL / JUNIOR_WAIT_DIV);
#endif


#include "func.h"
#include "movement.h"
#include "packets.h"

void run()
{
    state = 0;
    setLeftServo(-312);
    setRightServo(690);
    clearLed();

    while(true)
    {
        if(moveflags == MOVE_FORWARD || moveflags == MOVE_BACKWARD)
            MovementCorrection();

        if(readPacket())
            handlePacket(&pkt);
#ifdef PING
        if((state & STATE_PAUSED) || (state & STATE_LOCKED))
            continue;
            
        if(!(PINB & (1<<0)))
            discButton();
            
        thisTime = getTickCount();
        diff = (thisTime - lastTime);
        if(pingTimer <= diff)
            conLost();
        else pingTimer -= diff;

        if(checkRangeNow)
        {
            if(rangeTimer <= diff)
            {
                bool collision = checkRange();
                if(collision && !(state & STATE_COLLISION))
                {
                    state |= STATE_COLLISION;
                    setMotorPower(0, 0);
                    Packet col(CMSG_RANGE_BLOCK);
                    sendPacket(&col);
                }
                else if(!collision && (state & STATE_COLLISION))
                {
                    state &= ~(STATE_COLLISION);
                    SetMovementByFlags();
                    Packet col(CMSG_RANGE_BLOCK_GONE);
                    sendPacket(&col);
                }
                rangeTimer = RANGE_TIME;
                checkRangeNow = false;
            }
            else rangeTimer -= diff;
        }

        if(doReel)
        {
            if(reelStopTimer <= diff)
            {
               doReel = false;
               clearLed();
               reelStopTimer = ((1500000) * JUNIOR_WAIT_MUL / JUNIOR_WAIT_DIV);
            }
            else reelStopTimer -= diff;
        }
   
        lastTime = thisTime;
#endif
    }
}
