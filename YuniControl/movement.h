#define TURN_VALUE 40

enum moveFlags
{
    MOVE_NONE         = 0x00,
    MOVE_FORWARD      = 0x01,
    MOVE_BACKWARD     = 0x02,
    MOVE_LEFT         = 0x04,
    MOVE_RIGHT        = 0x08,
};

uint8_t moveflags;
uint8_t speed;
left_encoder le_cor;
right_encoder re_cor;
uint32_t startTime;
uint8_t correction_treshold = 5;

inline void SetMovementByFlags()
{
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
}


void MovementCorrection()
{
    int8_t speed_cor = moveflags == MOVE_FORWARD ? speed : -speed;
    int8_t speed_cor_low = moveflags == MOVE_FORWARD ? speed-1 : -(speed-1);
    int r = re_cor.get();
    int l = le_cor.get();
    if(!(state & STATE_CORRECTION2) && fabs(r - l) >= correction_treshold)
    {
        if((l > r && moveflags == MOVE_FORWARD) || (l < r && moveflags == MOVE_BACKWARD))
            setMotorPower(speed_cor_low, speed_cor);
        else setMotorPower(speed_cor, speed_cor_low);
        state |= STATE_CORRECTION2;
    }
    else if((state & STATE_CORRECTION2) && fabs(r - l) >= correction_treshold)
    {
        setMotorPower(speed_cor, speed_cor);
        state &= ~(STATE_CORRECTION2);
    }
    else return;
    re_cor.clear();
    le_cor.clear();
    
}
