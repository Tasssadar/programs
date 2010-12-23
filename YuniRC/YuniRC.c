#include "yunimin3.h"

#define TURN_VALUE 40

enum moveFlags
{
    MOVE_NONE         = 0x00,
    MOVE_FORWARD      = 0x01,
    MOVE_BACKWARD     = 0x02,
    MOVE_LEFT         = 0x04,
    MOVE_RIGHT        = 0x08,
};
enum rangeFinderPos
{
    FINDER_MIDDLE     = 0xE0,
    FINDER_LEFT       = 0xE2,
    FINDER_RIGHT      = 0xE4
};
#define RANGE_CENTIMETRES 0x51
#define MEM_SIZE 38
#define REC_SIZE 13 // sizeof(Record)

/*enum rangeMethods 
{
    RANGE_INCHES      = 0x50,
    RANGE_CENTIMETRES = 0x51,
    RANGE_MS          = 0x52,

    RANGE_INCHES_FAKE = 0x56,
    RANGE_CENTIMETRES_FAKE=0x57,
    RANGE_MS_FAKE     = 0x58,
};*/

uint8_t moveflags;
uint8_t speed;
left_encoder le;
right_encoder re;
left_encoder le_cor;
right_encoder re_cor;
bool correction = false;
volatile bool record = false;
bool play = false;
bool erasing = false;
stopwatch recordTime;
struct Record
{
    char key[10];
    uint16_t time;
    bool filled;
};
Record lastRec;
volatile uint8_t recordIter;
uint16_t lastAdress;

#include "func.h"

void SetMovement(char key[])
{
    // Set Movement Flags
    if(key[1] == ' ') // <- one character keys
    {
        bool down = key[3] == 'd';
        // only down keys
        if(down)
        {
            switch(key[0])
            {
                //speed
                case '1': speed = 50;  break;
                case '2': speed = 100; break;
                case '3': speed = 127; break;
                case 'R':   // reset encoders
                    re.clear();
                    le.clear();
                    break;
                case 'Q':   // on/off correction
                    correction = !correction;
                    rs232.send("Engine correction is ");
                    if(correction) {rs232.send("enabled \r\n");}
                    else {rs232.send("disabled \r\n");}
                    break;
                case 'C':
                    record = !record;
                    rs232.wait();
                    setMotorPower(0, 0);
                    le_cor.stop();
                    re_cor.stop();
                    moveflags = MOVE_NONE;
                    if(record)
                    {
                        recordIter = 0;
                        lastAdress = 0;
                        rs232.send("Erasing EEPROM...");
                        erasing = true;
                        for(uint8_t i = 0; i < MEM_SIZE+1; ++i)
                        {
                            Record rec;
                            rec.filled = false;
                            write_mem(&rec, lastAdress);
                            lastAdress+= REC_SIZE;
                        }
                        erasing = false;
                        rs232.send("done\r\n");
                        lastAdress = 0;
                    }
                    else
                    {
                        recordTime.stop();
                        recordTime.clear();
                        recordIter = 0;
                    }
                    rs232.send("Trace recording is ");
                    if(record){ rs232.send("enabled \r\n");}
                    else {rs232.send("disabled \r\n");}
                    break;
                case 'P':
                    if(!play)
                    {  
                        rs232.send("Playing..\r\n");   
                        recordIter = 0; 
                        lastAdress = 0;              
                        play = true;
                        record = false;
                    }
                    else
                    {
                        rs232.send("Playback stopped\r\n");
                        play = false;
                        setMotorPower(0, 0);
                        le_cor.stop();
                        re_cor.stop();
                        moveflags = MOVE_NONE;
                    }
                    break;
                case 'O':
                    play = !play;
                    rs232.send("Playback ");
                    if(play) rs232.send("unpaused\r\n");
                    else
                    {
                        setMotorPower(0, 0);
                        le_cor.stop();
                        re_cor.stop();
                        moveflags = MOVE_NONE;
                        rs232.send("paused\r\n");
                    }
                    break;
           }
        }
        // Movement
        switch(key[0])
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
        
    }
    // Sensors
    else if(key[0] == 'S' && key[4] == 'e' && key[7] == 'd') // Space
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
    
    char key[10];
    uint8_t key_itr = 0;
    while(key_itr < 10)
        key[++key_itr] = '0';
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

    /*rs232.send("YuniRC program has started!\r\n"
        "Controls: W,A,S,D - movement, Space - read sensor values,");
    rs232.wait();
    rs232.send(" R - reset encoders, Q - On/Off engine correction, 1 2 3 - speed \r\n");
    rs232.wait();
    rs232.send("Engine correction is disabled.\r\n"); */

    char ch;
    while(true)
    {
        if(erasing)
            continue;

        // Move correction
        if(correction && (moveflags == MOVE_FORWARD || moveflags == MOVE_BACKWARD))
            MovementCorrection();

        if(play && getTickCount() - nextPlayBase >= nextPlay)
        {
            Record rec;
            read_mem(&rec, lastAdress);
            lastAdress += REC_SIZE;
            if(!rec.filled)
            {
                play = false;
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

        key[key_itr] = ch;
        ++key_itr;
        if((key[0] == 'O' || key[0] == 'P') && key_itr >= 4)
        {
            while(key_itr < 10)
                key[++key_itr] = '0';
            key_itr = 0;
            SetMovement(key);
        }
        else if((ch == 'd'  || ch == 'u') && key_itr >= 4 && !play)
        {
            while(key_itr < 10)
                key[++key_itr] = '0';
            key_itr = 0;
             
            if(record && key[0] != 'C')
            {
                if(!recordTime.isRunning())
                {
                    recordTime.clear();
                    recordTime.start();
                }
                // FIXME: ignore two or more keys at once
                if(lastRec.key[findSpace(lastRec.key)+2] == 'd' && key[findSpace(key)+2] != 'u')
                {
                    while(key_itr < 10)
                        key[++key_itr] = '0';
                    key_itr = 0;
                    continue;
                }

                if(recordIter > 0)
                {
                    lastRec.time = recordTime.getTime()/10000;
                    write_mem(&lastRec, lastAdress);                   
                    lastAdress+=REC_SIZE;
                }
                if(recordIter < MEM_SIZE)
                {
                    while(key_itr < 10)
                    {
                        lastRec.key[key_itr] = key[key_itr];
                        ++key_itr;
                    }
                    key_itr = 0;
                    lastRec.filled = true;
                    recordTime.clear();
                    ++recordIter;
                }
                else
                {
                    key[0] = 'C';
                    key[1] = ' ';
                    key[2] = '-';
                    key[3] = 'd';
                    rs232.send("Memory full\r\n");
                    SetMovement(key);
                    continue;
                }
            };
            SetMovement(key);
        }
    }
}

