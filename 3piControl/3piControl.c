#include "lib.h"
#include "packets.h"

int main()
{
    init();
    //rs232.send("aaa");
    //setLeftMotor(255);
    //setRightMotor(255);
    while(true)
    {
        if(readPacket())
            handlePacket(&pkt);
    }
    return 0;

}
