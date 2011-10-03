#include "LineFollower.h"

void run()
{
    setSoftAccel(false);
    showSpeed();

    while(true)
    {
        checkStart();
        if(stopped)
            delayMicroseconds(1000);
        else
        {
            if(!followTimer)
                follow();
            else
            {
                delayMicroseconds(1);
                --followTimer;
            }
        }
    }
    return;
}

void follow()
{
    bool on_line = false;
    uint8_t i;
    uint16_t value_cur;
    uint32_t avg = 0;
    uint16_t sum = 0;

    static uint16_t last_value = 0;

    for(i = 0; i < SENSOR_COUNT; ++i)
    {
        value_cur = getSensorValue(i, true);

        if(value_cur == 1024)
            value_cur = 0;
        else if(value_cur > 910)
            on_line = true;

        if(value_cur > 50)
        {
            avg += uint32_t(value_cur) * (i * 1000);
            sum += value_cur;
        }
        rs232.sendNumber(value_cur, 4);
        rs232.sendCharacter(' ');
    }

    if(sum == 0)
        return;

    uint16_t position = 0;
    if(!on_line)
    {
        // If it last read to the left of center, return 0.
        if(last_value < 2000)
            position = 0;
        // If it last read to the right of center, return the max.
        else
            position = 4000;
    }
    else
    {
        last_value = avg/sum;
        position = last_value;
    }

    rs232.sendNumber(position, 4);
    rs232.sendCharacter(' ');
    rs232.sendNumber(last_value, 4);
    rs232.send("\r\n");
    rs232.wait();
    
    followTimer = 1;//10;

    cur_speed[0] = speeds[0];
    cur_speed[1] = speeds[0];
    
    if(position < 1000)
    {
        followTimer = 50;
        cur_speed[0] = speeds[2];
    }
    else if(position > 3000)
    {
        followTimer = 50;
        cur_speed[1] = speeds[2];
    }

    setMotorPower(cur_speed[0], cur_speed[1]);
}













