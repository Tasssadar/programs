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
enum rangeMethods 
{
    RANGE_INCHES      = 0x50,
    RANGE_CENTIMETRES = 0x51,
    RANGE_MS          = 0x52,

    RANGE_INCHES_FAKE = 0x56,
    RANGE_CENTIMETRES_FAKE=0x57,
    RANGE_MS_FAKE     = 0x58,
};

uint8_t moveflags;
uint8_t speed;
left_encoder le;
right_encoder re;
left_encoder le_cor;
right_encoder re_cor;
bool correction = false;

inline int16_t fabs(int16_t num)
{
    if(num < 0)
        return -num;
    return num;
}
uint16_t ReadRange(uint8_t adress, uint8_t method = RANGE_CENTIMETRES)
{
    uint16_t range = 0;
    uint8_t data [2] = { 0, method };
    i2c.write(adress, &data[0], 2);
    if(i2c.get_result().result != 2)
        return 0;
    wait(10000); // maybe we can use smaller value
    i2c.write(adress, 0x02); // range High-byte
    if(i2c.get_result().result != 1)
        return 0;
    i2c.read(adress, 1);
    i2c.get_result();
    range = (8 << TWDR);
    i2c.write(adress, 0x03);
    if(i2c.get_result().result != 1)
        return 0;
    i2c.read(adress, 1);
    i2c.get_result();
    range |= TWDR;
    return range;
}

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
                    if(correction) rs232.send("enabled \r\n");
                    else rs232.send("disabled \r\n");
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
        rs232.send("\r\nSensors: \r\n LeftTop: ");
        rs232.sendNumber(getSensorValue(0));
        rs232.send("\r\n LeftMiddle: ");
        rs232.sendNumber(getSensorValue(4));
        rs232.send("\r\n RightMiddle: ");
        rs232.sendNumber(getSensorValue(5));
        rs232.send("\r\n RightTop: ");
        rs232.sendNumber(getSensorValue(1));
        rs232.wait();
        rs232.send("\r\n LeftBottom: ");
        rs232.sendNumber(getSensorValue(2));
        rs232.send("\r\n RightBottom: ");
        rs232.sendNumber(getSensorValue(3));
        rs232.wait();
        rs232.send("\r\n LeftEncoder: ");
        rs232.sendNumber(le.get());
        rs232.send("\r\n RightEncoder: ");
        rs232.sendNumber(re.get());
        rs232.send("\r\n average: ");
        rs232.sendNumber((re.get()+le.get())/2);
        // 0xE2 left, 0xE0 middle, 0xE4 right
        rs232.send("\r\n RangeLeft: ");
        rs232.sendNumber(ReadRange(FINDER_LEFT));
        rs232.send(" cm\r\n RangeMiddle: ");
        rs232.sendNumber(ReadRange(FINDER_MIDDLE));
        rs232.send(" cm\r\n RangeRight: ");
        rs232.sendNumber(ReadRange(FINDER_RIGHT));
        rs232.send(" cm\r\n");
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
    speed = 100;
    le.clear();
    re.clear();
    rs232.send("YuniRC program has started!\r\n"
        "Controls: W,A,S,D - movement, Space - read sensor values,");
    rs232.wait();
    rs232.send(" R - reset encoders, Q - On/Off movement correction, 1 2 3 - speed \r\n");
    rs232.wait();
    rs232.send("Movement correction is disabled.\r\n");

    char ch;
    while(true)
    {
        // Move correction
        if(correction && (moveflags == MOVE_FORWARD || moveflags == MOVE_BACKWARD))
            MovementCorrection();

        //Read command
        if(!rs232.peek(ch))
            continue;

        key[key_itr] = ch;
        ++key_itr;
        if((ch == 'd'  || ch == 'u') && key_itr >= 4)
        {
            while(key_itr < 10)
                key[++key_itr] = '0';
            key_itr = 0;
            SetMovement(key);
            //rs232.send(key);
            //rs232.send("\r\n");
        }
    }
}

