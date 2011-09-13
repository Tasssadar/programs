#include "3piLibPack.h"

#define SENSOR_COUNT 5
#define SENSOR_CENTER 2
//#define MAX_SPEED 100

volatile uint8_t speeds[3] =
{
    150, // max speed
    70,
    0,
};

void follow();

volatile bool stopped = true;
uint8_t cur_speed[2] = {0, 0};
volatile int8_t lastMax = SENSOR_CENTER;
uint16_t followTimer = 0;

void toggleStart()
{
    stopped = !stopped;
    if(!stopped)
    {
        delay(1000);
        resetCalibration();

        clean_rs232(); // so the red LED does not interfere

        for(uint8_t counter = 0; counter < 80; ++counter)
        {
            if(counter < 20 || counter >= 60)
                setMotorPower(40,-40);
            else
                setMotorPower(-40,40);
            calibrate_sensors();

            // 80*20 = 1600 ms.
            delay(20);
        }

        init_rs232();

        /*for(uint8_t i = 0; i < PI_GRND_SENSOR_COUNT; ++i)
        {
            rs232.dumpNumber(g_calibratedMinimum[i]);
            rs232.dumpNumber(g_calibratedMaximum[i]);
            rs232.send("\r\n");
            rs232.wait();
        }*/
    }
    setMotorPower(0, 0);
}

void showSpeed(bool tors232 = true)
{
    if(tors232)
        rs232.dumpNumber(speeds[0]);

    display.gotoXY(0,0);
    display.printNumber(speeds[0]);
    display.print(" ");
    display.printNumber(getBatteryVoltage());
}

void checkStart()
{
    if(isPressed(BUTTON_B))
    {
        waitForRelease(BUTTON_B);
        toggleStart();
    }
    else if(isPressed(BUTTON_A))
    {
        waitForRelease(BUTTON_A);
        speeds[0] += 10;
        showSpeed();
    }
    else if(isPressed(BUTTON_C))
    {
        waitForRelease(BUTTON_C);
        speeds[0] -= 10;
        showSpeed();
    }

    char c;
    if(rs232.peek(c))
    {
        switch(c)
        {
            case 's':
                toggleStart();
                break;
            case 'q':
                speeds[0] += 10;
                showSpeed();
                break;
            case 'w':
                speeds[0] -= 10;
                showSpeed();
                break;
        }
    }
}

inline uint8_t abs_my(int8_t num)
{
    return num > 0 ? num : -num;
}
