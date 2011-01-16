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
stopwatch recordTime;
Record lastRec;
uint32_t startTime;

#include "func.h"

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
            /*case 'Q':   // on/off correction
                rs232.send("Engine correction is ");
                if(state & STATE_CORRECTION)
                {
                    state &= ~(STATE_CORRECTION);
                    rs232.send("disabled \r\n");
                }
                else
                {
                    state |= STATE_CORRECTION;
                    rs232.send("enabled \r\n");
                }
                break;*/
           /* case 'C':
                rs232.wait();
                setMotorPower(0, 0);
                le_cor.stop();
                re_cor.stop();
                moveflags = MOVE_NONE;
                if(!(state & STATE_RECORD))
                {
                    state |= STATE_RECORD;
                    recordIter = 0;
                    rs232.send("Erasing EEPROM...");
                    state |= STATE_ERASING;
                    erase_eeprom();
                    state &= ~(STATE_ERASING);
                    rs232.send("done\r\n");
                    lastAdress = 0;
                }
                else
                {
                    lastRec.end_event = EVENT_TIME;
                    lastRec.setBigNum(recordTime.getTime()/10000);
                    write_mem(&lastRec, lastAdress);                   
                    recordTime.stop();
                    recordTime.clear();
                    recordIter = 0;
                    state &= ~(STATE_RECORD);
                }
                rs232.send("Trace recording is ");
                if(state & STATE_RECORD){ rs232.send("enabled \r\n");}
                else {rs232.send("disabled \r\n");}
                break; */
            case 'P':
                le_cor.stop();
                re_cor.stop();
                moveflags = MOVE_NONE;
                setMotorPower(0, 0);

                if(!(state & STATE_PLAY))
                {  
                    recordTime.stop();
                    recordTime.clear();
                    rs232.send("Playing..\r\n");   
                    recordIter = 0; 
                    lastAdress = 0;              
                    state |= STATE_PLAY;
                    state &= ~(STATE_RECORD);
                }
                else
                {
                    rs232.send("Playback stopped\r\n");
                    state &= ~(STATE_PLAY);
                }
                break;
         /*   case 'O':
                if(state & STATE_RECORD)
                   break;
                rs232.send("Playback ");
                if(state & STATE_PLAY)
                {
                    rs232.send("unpaused\r\n");
                    state &= ~(STATE_PLAY);
                }
                else
                {
                    setMotorPower(0, 0);
                    le_cor.stop();
                    re_cor.stop();
                    moveflags = MOVE_NONE;
                    rs232.send("paused\r\n");
                    state |= STATE_PLAY;
                }
                break; */
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
   
    //Set motors
    if(moveflags & MOVE_FORWARD)
    {
        if(moveflags & MOVE_LEFT)
            setMotorPower(speed-TURN_VALUE, speed);
        else if(moveflags & MOVE_RIGHT)
            setMotorPower(speed, speed-TURN_VALUE);
        else
        {
            le_cor.start();
            re_cor.start();
            le_cor.clear();
            re_cor.clear();
            setMotorPower(speed, speed);
            state &= ~(STATE_CORRECTION2);
        }
		startTime = getTickCount();
    }
    else if(moveflags & MOVE_BACKWARD)
    {
        if(moveflags & MOVE_LEFT)
            setMotorPower(-(speed-TURN_VALUE), -speed);
        else if(moveflags & MOVE_RIGHT)
            setMotorPower(-speed, -(speed-TURN_VALUE));
        else
        {
            state &= ~(STATE_CORRECTION2);
            le_cor.start();
            re_cor.start();
            le_cor.clear();
            re_cor.clear();
            setMotorPower(-speed, -speed);
        }
		startTime = getTickCount();
    }
    else if(moveflags & MOVE_LEFT)
	{
        setMotorPower(-speed, speed);
		startTime = getTickCount();
	}
    else if(moveflags & MOVE_RIGHT)
	{
        setMotorPower(speed, -speed);
		startTime = getTickCount();
	}
    else
    {
		startTime = getTickCount();
        setMotorPower(0, 0);
        le_cor.stop();
        re_cor.stop();
        state &= ~(STATE_CORRECTION2);
    }
    return down_only;
}

void MovementCorrection()
{
    int8_t speed_cor = moveflags == MOVE_FORWARD ? speed : -speed;
    int8_t speed_cor_low = moveflags == MOVE_FORWARD ? speed-1 : (-speed)+1;
    int r = re_cor.get();
    int l = le_cor.get();
    if(!(state & STATE_CORRECTION2) && fabs(r - l) >= 10)
    {
        if((l > r && moveflags == MOVE_FORWARD) || (l < r && moveflags == MOVE_BACKWARD))
            setMotorPower(speed_cor_low, speed_cor);
        else setMotorPower(speed_cor, speed_cor_low);
        state |= STATE_CORRECTION2;
    }
    else if((state & STATE_CORRECTION2) && fabs(r - l) >= 10)
    {
        setMotorPower(speed_cor, speed_cor);
        state &= ~(STATE_CORRECTION2);
    }
    else return;
    re_cor.clear();
    le_cor.clear();
    
}

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
    recordTime.stop();
    recordTime.clear();
    uint32_t nextPlay = 0;
    uint32_t nextPlayBase = 0;
    state = STATE_CORRECTION;
    encoder_play_l.stop();
	encoder_play_r.stop();
	startTime = 0;

    char ch;
    while(true)
    {
        if(state & STATE_ERASING)
            continue;

        // Move correction
        if((state & STATE_CORRECTION) && (moveflags == MOVE_FORWARD || moveflags == MOVE_BACKWARD))
            MovementCorrection();

        if((state & STATE_PLAY) && (lastAdress == 0 || EventHappened(&lastRec, &nextPlayBase, &nextPlay)))
        {
            encoder_play_l.stop();
            encoder_play_r.stop();
            read_mem(&lastRec, lastAdress);
            lastAdress += REC_SIZE;
            if((lastRec.key[0] == 0 && lastRec.key[1] == 0 && lastRec.getBigNum() == 0) ||
               lastAdress > 512)
            {
                state &= ~(STATE_PLAY);
                rs232.send("Playback finished\r\n");
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
                if(ch == 0x1E && lastAdress%5 == 0)
                    break;
                write_byte(lastAdress, uint8_t(ch));
                ++lastAdress;
                rs232.sendCharacter(0x1F);
            }
            lastAdress = 0;
             
        }
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
    }
}

