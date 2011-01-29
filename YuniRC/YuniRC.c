//#define SET_FINDER_ADR 1
//#define FINDERS_DEBUG 1
//#define EEPROM_PROTECTED 1
#define RS232_CONNECTED 1

#include "yunimin3.h"
#include "enums.h"

uint8_t moveflags;
uint8_t speed;
left_encoder le_cor;
right_encoder re_cor;
left_encoder encoder_play_l;
right_encoder encoder_play_r;
uint8_t state;
uint16_t lastAdress;
uint8_t recordIter;
Record lastRec;
uint32_t startTime;
uint8_t memBegin;

#include "func.h"

void CheckProcedure();

bool SetMovement(uint8_t key[])
{
    // Set Movement Flags
    bool down = (key[1] == uint8_t('d'));
    bool down_only = false;
    // only down keys
    if(down)
    {
        switch(char(key[0]))
        {
            //speed (1 2 3 on keyboard Oo)
            case 'a': speed = 50;  break;
            case 'b': speed = 100; break;
            case 'c': speed = 127; break; 
            case 'P':
                le_cor.stop();
                re_cor.stop();
                moveflags = MOVE_NONE;
                setMotorPower(0, 0);
                state &= ~(STATE_COLLISION);
                if(!(state & STATE_PLAY))
                {  
#ifdef RS232_CONNECTED
                    rs232.send("Playing..\r\n");   
#endif
                    recordIter = 0; 
                    lastAdress = 0;
                    state |= STATE_PLAY;
                    state &= ~(STATE_RECORD);
                }
                else
                {
#ifdef RS232_CONNECTED
                    rs232.send("Playback stopped\r\n");
#endif
                    state &= ~(STATE_PLAY);
                }
                break;
#ifdef FINDERS_DEBUG
            case ' ':
            {
                uint8_t adr = FINDER_FRONT1;
                rs232.send("\r\n");
                for(uint8_t i = 0; i < 6; ++i, adr +=2)
                {
                    rs232.sendHexByte(adr);
                    rs232.send(" ");
                    rs232.dumpNumber(ReadRange(adr));
                    rs232.sendNumber(GetMinRange(adr));
                    rs232.send("\r\n");
    
                }
                rs232.dumpNumber(memBegin);
                rs232.dumpNumber(getSensorValue(4));
                break;
            }
#endif
#ifdef SET_FINDER_ADR
            case 'H':
            {
                // range finders adress changing
                uint8_t data[4] = { 0xA0, 0xAA, 0xA5, 0xEA };
                for(uint8_t i = 0; i < 4; ++i)
                { 
                    uint8_t dataS [2] = { 0, data[i] };
                    i2c.write(0xE0, &dataS[0], 2);
                    if(i2c.get_result().result != 2)
                        rs232.dumpNumber(6);
                    rs232.dumpNumber(i);
                }
                rs232.send("Adress changed\r\n");
                break;
            }
#endif
            case 'I':
                setLeftServo(127);
                setRightServo(127);
                break;
            case 'O':
                setLeftServo(-128);
                setRightServo(-128);
                break;
            case 'K':
                CheckProcedure();
                break;

       }
    }
    // Movement
    switch(char(key[0]))
    {
        case 'W':
            if(!(moveflags & MOVE_BACKWARD))
            {
                if(down) moveflags |= MOVE_FORWARD;
                else moveflags &= ~(MOVE_FORWARD);
            }
            break;
        case 'S':
            if(!(moveflags & MOVE_FORWARD))
            {
                if(down) moveflags |= MOVE_BACKWARD;
                else moveflags &= ~(MOVE_BACKWARD);
            }
            break;
        case 'A':
            if(!(moveflags & MOVE_RIGHT))
            {
                if(down) moveflags |= MOVE_LEFT;
                else moveflags &= ~(MOVE_LEFT);
            } 
            break;
        case 'D':
            if(!(moveflags & MOVE_LEFT))
            {
                if(down) moveflags |= MOVE_RIGHT;
                else moveflags &= ~(MOVE_RIGHT);
            }
            break;
        default:
            down_only = true;
            break;
    }
    SetMovementByFlags();
    return down_only;
}

void CheckProcedure()
{
    uint8_t key[16][3] = 
    {
        {'W', 'd', 0},
        {'D', 'd', 5},
        {'W', 'u', 0},

        {'S', 'd', 10},
        {'S', 'u', 0},

        {'W', 'd', 5},
        {'D', 'u', 0},

        {'A', 'd', 5},
        {'W', 'u', 0},

        {'S', 'd', 10},
        {'S', 'u', 0},

        {'W', 'd', 5},
        {'A', 'u', 0},
        {'W', 'u', 0},

        {'I', 'd', 5},
        {'O', 'd', 0},
    };
    for(uint8_t i = 0; i < 16; ++i)
    {
        SetMovement(key[i]);
        wait(key[i][2]*100000);
    }
    uint8_t adr = FINDER_FRONT1;
    for(uint8_t i = 0; i < 6; ++i, adr +=2)
        ReadRange(adr);
}

void MovementCorrection()
{
    int8_t speed_cor = moveflags == MOVE_FORWARD ? speed : -speed;
    int8_t speed_cor_low = moveflags == MOVE_FORWARD ? speed-1 : -(speed-1);
    int r = re_cor.get();
    int l = le_cor.get();
    if(!(state & STATE_CORRECTION2) && fabs(r - l) >= 5)
    {
        if((l > r && moveflags == MOVE_FORWARD) || (l < r && moveflags == MOVE_BACKWARD))
            setMotorPower(speed_cor_low, speed_cor);
        else setMotorPower(speed_cor, speed_cor_low);
        state |= STATE_CORRECTION2;
    }
    else if((state & STATE_CORRECTION2) && fabs(r - l) >= 5)
    {
        setMotorPower(speed_cor, speed_cor);
        state &= ~(STATE_CORRECTION2);
    }
    else return;
    re_cor.clear();
    le_cor.clear();
    
}

void checkCollision(uint32_t *nextPlayBase, uint32_t *nextPlay)
{
    if(moveflags != MOVE_FORWARD && moveflags != MOVE_BACKWARD)
        return;

    bool found = false;
    uint8_t adr = (moveflags == MOVE_FORWARD) ? FINDER_FRONT1 : FINDER_BACK1;

    for(uint8_t i = 0; i < 3 && !found; ++i, adr +=2)
    {
        if(ReadRange(adr) <= COLISION_TRESHOLD)
            found = true;
    }

    if(found && !(state & STATE_COLLISION))
    {
        state |= STATE_COLLISION;
        if(lastRec.end_event == EVENT_TIME)
            *nextPlay -= (getTickCount() - *nextPlayBase);
        setMotorPower(0, 0);
    }
    else if(!found && (state & STATE_COLLISION))
    {
        state &= ~(STATE_COLLISION);
        wait(800000);
        SetMovementByFlags();

        if(lastRec.end_event == EVENT_TIME)
            *nextPlayBase = getTickCount();
    }
}
/*
void CalibrateUltraSound()
{
    uint8_t adr = FINDER_FRONT1;
    for(uint8_t i = 0; i < 6; ++i, adr +=2)
        ReadRange(adr);
}*/

void run()
{
    uint8_t key[2];
    uint8_t key_itr = 0;
    while(key_itr < 2)
        key[++key_itr] = 0;
    key_itr = 0;
    moveflags = 0;
    recordIter = 0;
    speed = 127;
    uint32_t nextPlay = 0;
    uint32_t nextPlayBase = 0;
    state = (STATE_CORRECTION); // TODO: | STATE_PLAY);
    encoder_play_l.stop();
    encoder_play_r.stop();
    startTime = 0;
    uint32_t rangeCheckBase = getTickCount();
    const uint32_t rangeDelay = (300000) * JUNIOR_WAIT_MUL / JUNIOR_WAIT_DIV;

    memBegin = (getSensorValue(4) == 511) ? MEM_PART2 : MEM_PART1; // TODO: if something then part 2

    setLeftServo(-127);
    setRightServo(-127);

    /*for(uint8_t z = 0; z < 7; ++z)
        CalibrateUltraSound();*/

    char ch;
    while(true)
    {
        if(state & STATE_ERASING)
            continue;

        // Move correction
        if((state & STATE_CORRECTION) && (moveflags == MOVE_FORWARD || moveflags == MOVE_BACKWARD))
            MovementCorrection();
        if((state & STATE_PLAY) && getTickCount() - rangeCheckBase >= rangeDelay)
        {
            rangeCheckBase = getTickCount();
            checkCollision(&nextPlayBase, &nextPlay);
        }

        if((state & STATE_PLAY) && !(state & STATE_COLLISION) &&
            (lastAdress == 0 || EventHappened(&lastRec, &nextPlayBase, &nextPlay)))
        {
            encoder_play_l.stop();
            encoder_play_r.stop();
            read_mem(&lastRec, lastAdress);
            lastAdress += REC_SIZE;
            if((lastRec.key[0] == 0 && lastRec.key[1] == 0 && lastRec.getBigNum() == 0) ||
               lastAdress > 255)
            {
                state &= ~(STATE_PLAY);
#ifdef RS232_CONNECTED
                rs232.send("Playback finished\r\n");
#endif
                setMotorPower(0, 0);
                le_cor.stop();
                re_cor.stop();
                moveflags = MOVE_NONE;
                continue;
            }
            SetMovement(lastRec.key);
            nextPlay = 0;
            nextPlayBase = 0;
            if(lastRec.end_event == EVENT_TIME)
            {
                nextPlayBase = getTickCount();
                nextPlay = (uint32_t(lastRec.getBigNum())*10000) * JUNIOR_WAIT_MUL / JUNIOR_WAIT_DIV;
            }
            //Uncomment to set messure delay
            /*else if(lastRec.end_event == EVENT_RANGE_MIDDLE_HIGHER || lastRec.end_event == EVENT_RANGE_MIDDLE_LOWER)
            {
                nextPlayBase = getTickCount();
                nextPlay = (50000) * JUNIOR_WAIT_MUL / JUNIOR_WAIT_DIV;
            }*/
            else if(lastRec.end_event == EVENT_DISTANCE || lastRec.end_event == EVENT_DISTANCE_LEFT || lastRec.end_event == EVENT_DISTANCE_RIGHT)
            {
                encoder_play_r.clear();
                encoder_play_l.clear();
                encoder_play_l.start();
                encoder_play_r.start();
            }
            ++recordIter;
        }
        //Read command
#ifdef RS232_CONNECTED
        if(!rs232.peek(ch))
            continue;

        key[key_itr] = uint8_t(ch);
        ++key_itr;
    
        //key recieved
        if(key_itr >= 2)
        {
            key_itr = 0;
            SetMovement(key);
        }
 #ifndef EEPROM_PROTECTED
        // EEPROM Flash mode
        else if(ch == 0x1C)
        {
            while(key_itr < 2)
                key[++key_itr] = '0';
            key_itr = 0;
            erase_eeprom();
            rs232.sendCharacter(0x1D);
            for(lastAdress = 0; true; )
            {
                if(!rs232.peek(ch))
                    continue;
                if(ch == 0x1E && (lastAdress%5 == 0 || lastAdress == 0))
                    break;
                else if(ch == 0x16 && (lastAdress%5 == 0 || lastAdress == 0))
                {
                    lastAdress = MEM_PART2;
                    rs232.sendCharacter(0x1F);
                    continue;
                }
                write_byte(lastAdress, uint8_t(ch));
                ++lastAdress;
                if(lastAdress%5 == 0)
                  rs232.sendCharacter(0x1F);
            }
            lastAdress = 0;
    
        }
 #else
        else if(ch == 0x1C)
            continue;
 #endif
        // EEPROM read mode
        else if(ch == 0x16)
        {
            while(key_itr < 2)
                key[++key_itr] = '0';
            key_itr = 0;
            rs232.sendCharacter(0x17);
            for(lastAdress = 0; lastAdress < 512; ++lastAdress)
            {
                rs232.wait();
                rs232.sendCharacter(read_byte(lastAdress));
            }
            rs232.sendCharacter(0x18);
            lastAdress = 0;
        }
#endif
    }
}

