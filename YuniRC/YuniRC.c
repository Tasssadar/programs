#include "yunimin3.h"
#include "enums.h"

uint8_t moveflags;
uint8_t speed;
left_encoder le;
right_encoder re;
left_encoder le_cor;
right_encoder re_cor;
uint8_t state;
uint16_t lastAdress;
uint8_t recordIter;
stopwatch recordTime;
Record lastRec;

#include "func.h"

void SetMovement(uint8_t key[])
{
    // Set Movement Flags
    bool down = key[1] == uint8_t('d');
    // only down keys
    if(down)
    {
        switch(char(key[0]))
        {
            //speed (1 2 3 on keyboard Oo)
            case 'a': speed = 50;  break;
            case 'b': speed = 100; break;
            case 'c': speed = 127; break; 
            case 'R':   // reset encoders
                re.clear();
                le.clear();
                break;
            case 'Q':   // on/off correction
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
                break;
            case 'C':
                rs232.wait();
                setMotorPower(0, 0);
                le_cor.stop();
                re_cor.stop();
                moveflags = MOVE_NONE;
                if(!(state & STATE_RECORD))
                {
                    state |= STATE_RECORD;
                    recordIter = 0;
                    lastAdress = 0;
                    rs232.send("Erasing EEPROM...");
                    state |= STATE_ERASING;
                    Record rec;
                    rec.time = 0;
                    rec.key[0] = 0;
                    rec.key[1] = 0;
                    for(uint8_t i = 0; i < MEM_SIZE; ++i)
                    {
                        write_mem(&rec, lastAdress);
                        lastAdress += REC_SIZE;
                    }
                    state &= ~(STATE_ERASING);
                    rs232.send("done\r\n");
                    lastAdress = 0;
                }
                else
                {
                    lastRec.time = recordTime.getTime()/10000;
                    write_mem(&lastRec, lastAdress);                   
                    recordTime.stop();
                    recordTime.clear();
                    recordIter = 0;
                    state &= ~(STATE_RECORD);
                }
                rs232.send("Trace recording is ");
                if(state & STATE_RECORD){ rs232.send("enabled \r\n");}
                else {rs232.send("disabled \r\n");}
                break;
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
            case 'O':
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
    }
    // Sensors
    if(char(key[0]) == ' ' && down) // Space
    {
        rs232.wait();
        rs232.send("\r\nSensors: \r\n");
        rs232.sendNumber(getSensorValue(0));
        rs232.send("  ");
        rs232.sendNumber(getSensorValue(4));
        rs232.send("  ");
        rs232.sendNumber(getSensorValue(5));
        rs232.send("  ");
        rs232.sendNumber(getSensorValue(1));
        rs232.wait();
        rs232.send("\r\n");
        rs232.sendNumber(getSensorValue(2));
        rs232.send("                    ");
        rs232.sendNumber(getSensorValue(3));
        rs232.wait();
        rs232.send("\r\nEncoders: \r\n L: ");
        rs232.sendNumber(le.get());
        rs232.send(" R: "); 
        rs232.sendNumber(re.get()); 
        rs232.send("\r\nRange \r\nL: "); 
        rs232.wait();
        rs232.sendNumber(ReadRange(FINDER_LEFT));
        rs232.send("cm M: ");
        rs232.sendNumber(ReadRange(FINDER_MIDDLE));
        rs232.send("cm R: ");
        rs232.sendNumber(ReadRange(FINDER_RIGHT));
        rs232.send("cm\r\n"); 
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
        }
    }
    else if(moveflags & MOVE_BACKWARD)
    {
        if(moveflags & MOVE_LEFT)
            setMotorPower(-(speed-TURN_VALUE), -speed);
        else if(moveflags & MOVE_RIGHT)
            setMotorPower(-speed, -(speed-TURN_VALUE));
        else
        {
            le_cor.start();
            re_cor.start();
            le_cor.clear();
            re_cor.clear();
            setMotorPower(-speed, -speed);
        }
    }
    else if(moveflags & MOVE_LEFT)
        setMotorPower(-speed, speed);
    else if(moveflags & MOVE_RIGHT)
        setMotorPower(speed, -speed);
    else
    {
        setMotorPower(0, 0);
        le_cor.stop();
        re_cor.stop();
    }
}

void MovementCorrection()
{
    int speed_cor = moveflags == MOVE_FORWARD ? speed : -speed;
    int r = re_cor.get();
    int l = le_cor.get();
    if(fabs(r - l) >= 50)
    {
        if((l > r && moveflags == MOVE_FORWARD) || (l < r && moveflags == MOVE_BACKWARD))
            setMotorPower(0, speed_cor);
        else setMotorPower(speed_cor, 0);
    }
    else if((fabs(r - l) >= 30 && moveflags == MOVE_FORWARD) || fabs(r - l) >= 45)
        setMotorPower(speed_cor, speed_cor);
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
    speed = 100;
    le.clear();
    re.clear();
    recordTime.stop();
    recordTime.clear();
    uint32_t nextPlay = 0;
    uint32_t nextPlayBase = 0;
    state = 0;

    /*rs232.send("YuniRC program has started!\r\n"
        "Controls: W,A,S,D - movement, Space - read sensor values,");
    rs232.wait();
    rs232.send(" R - reset encoders, Q - On/Off engine correction, 1 2 3 - speed \r\n");
    rs232.wait();
    rs232.send("Engine correction is disabled.\r\n"); */

    char ch;
    while(true)
    {
        if(state & STATE_ERASING)
            continue;

        // Move correction
        if((state & STATE_CORRECTION) && (moveflags == MOVE_FORWARD || moveflags == MOVE_BACKWARD))
            MovementCorrection();

        if((state & STATE_PLAY) && getTickCount() - nextPlayBase >= nextPlay)
        {
            Record rec;
            read_mem(&rec, lastAdress);
            lastAdress += REC_SIZE;
            if((rec.key[0] == 0 && rec.key[1] == 0 && rec.time == 0) ||
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
            SetMovement(rec.key);
            nextPlayBase = getTickCount();
            nextPlay = (uint32_t(rec.time)*10000) * JUNIOR_WAIT_MUL / JUNIOR_WAIT_DIV;
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
            if(char(key[0]) == 'O' || char(key[0]) == 'P')
            {
                SetMovement(key);
                continue;
            }
            else if((state & STATE_RECORD) && char(key[0]) != 'C')
            {
                if(!recordTime.isRunning())
                {
                    recordTime.clear();
                    recordTime.start();
                }

                if(recordIter > 0)
                {
                    lastRec.time = recordTime.getTime()/10000;
                    write_mem(&lastRec, lastAdress);                   
                    lastAdress+=REC_SIZE;
                }
                if(recordIter < MEM_SIZE-1)
                {
                    while(key_itr < 2)
                    {
                        lastRec.key[key_itr] = key[key_itr];
                        ++key_itr;
                    }
                    key_itr = 0;
                    recordTime.clear();
                    ++recordIter;
                }
                else
                {
                    key[0] = uint8_t('C');
                    key[1] = uint8_t('d');
                    rs232.send("Memory full\r\n");
                    SetMovement(key);
                    continue;
                }
            };
            SetMovement(key);
        }
    }
}

