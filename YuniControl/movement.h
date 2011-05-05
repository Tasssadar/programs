#define TURN_VALUE 40


enum servoFlags
{
    SERVO_DOORS        = 0x01,
    SERVO_BRUSHES      = 0x02,
    SERVO_REEL         = 0x04,
};

struct EncoderEvent
{
    uint8_t id;
    uint16_t left;
    uint16_t right;
};

uint8_t speed;
uint32_t startTime;
uint8_t correction_treshold = 5;
EncoderEvent enc_events[5];

volatile int32_t g_left_encoder = 0;
volatile int32_t g_right_encoder = 0;

volatile int32_t g_left_cor = 0;
volatile int32_t g_right_cor = 0;
void StopAll(bool lock = true);
void StartAll(bool unlock = true);

int32_t getLeftEnc(bool cor = false)
{
    cli();
    int32_t res = cor ? g_left_cor : g_left_encoder;
    sei();
    return res; 
}

int32_t getRightEnc(bool cor = false)
{
    cli();
    int32_t res = cor ? g_right_cor : g_right_encoder;
    sei();
    return res; 
}

void checkEncEvent(bool right);

ISR(JUNIOR_ENCODER_LEFT_vect)
{
    uint8_t port = JUNIOR_CONCAT(PIN, JUNIOR_ENCODER_LEFT_PORT);
    if (((port & (1<<JUNIOR_ENCODER_LEFT_PIN1)) != 0) == ((port & (1<<JUNIOR_ENCODER_LEFT_PIN2)) != 0))
    {
        --g_left_encoder;
        --g_left_cor;
    }
    else
    {
        ++g_left_encoder;
        ++g_left_cor;
    }
    checkEncEvent(false);
}

ISR(PCINT0_vect)
{
    uint8_t port = JUNIOR_CONCAT(PIN, JUNIOR_ENCODER_RIGHT_PORT);
    if (((port & (1<<JUNIOR_ENCODER_RIGHT_PIN1)) != 0) == ((port & (1<<JUNIOR_ENCODER_RIGHT_PIN2)) != 0))
    {
        --g_right_encoder;
        --g_right_cor;
    }
    else
    {
        ++g_right_encoder;
        ++g_right_cor;
    }
    checkEncEvent(true);
}

void clearEnc(bool correction = true)
{
    cli();
    if(correction)
    {
        g_left_cor = 0;
        g_right_cor = 0;
    }
    else
    {
        g_right_encoder = 0;
        g_left_encoder = 0;
    }
    sei();
}

inline void SetMovementByFlags()
{
    if(moveflags == MOVE_NONE)
    {
        startTime = getTickCount();
        setMotorPower(0, 0);
        clearEnc();
        state &= ~(STATE_CORRECTION2); 
    }
    else if(moveflags & MOVE_FORWARD)
    {
        if(moveflags & MOVE_LEFT)
            setMotorPower(speed-TURN_VALUE, speed);
        else if(moveflags & MOVE_RIGHT)
            setMotorPower(speed, speed-TURN_VALUE);
        else
        {
            clearEnc();
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
            clearEnc();
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
   /* else if(moveflags & MOVE_LEFT_WHEEL)
    {
        setMotorPower(-speed, 0);
    }
    else if(moveflags & MOVE_RIGHT_WHEEL)
    {
        setMotorPower(0, -speed);
    }*/
    else
    {
        startTime = getTickCount();
        setMotorPower(0, 0);
        clearEnc();
        state &= ~(STATE_CORRECTION2);
    }
}

void MovementCorrection()
{
    int8_t speed_cor = moveflags == MOVE_FORWARD ? speed : -speed;
    int8_t speed_cor_low = moveflags == MOVE_FORWARD ? speed-1 : -(speed-1);
    int32_t r = fabs(getRightEnc(true));
    int32_t l = fabs(getLeftEnc(true));
    if(!(state & STATE_CORRECTION2) && fabs(r - l) >= correction_treshold)
    {
        if(l > r)
            setMotorPower(speed_cor_low, speed_cor);
        else
            setMotorPower(speed_cor, speed_cor_low);
        state |= STATE_CORRECTION2;
    }
    else if((state & STATE_CORRECTION2) && fabs(r - l) >= correction_treshold)
    {
        setMotorPower(speed_cor, speed_cor);
        state &= ~(STATE_CORRECTION2);
    }
    else return;
    clearEnc();
   // rs232.send("g ");
   // rs232.dumpNumber(l);
   // rs232.dumpNumber(r);
}

void setServoByFlags(uint8_t flags, int16_t val)
{
    if(flags & SERVO_DOORS)
        setLeftServo(val);
    if(flags & SERVO_BRUSHES)
        setRightServo(val);
#ifdef PING
    if(flags & SERVO_REEL)
    {
        setLed();
        doReel = true;
    }
#endif
}

void setEncEvent(uint8_t id, uint16_t left, uint16_t right)
{
    for(uint8_t y = 0; y < 5; ++y)
    {
        if(enc_events[y].id == id)
            return;

        if(enc_events[y].id != 0)
            continue;
        cli();
        enc_events[y].id = id;
        enc_events[y].left = left;
        enc_events[y].right = right;
        sei();
        break;
    }
}

void removeEncEvent(uint8_t id)
{
    for(uint8_t y = 0; y < 5; ++y)
    {
        if(enc_events[y].id == id)
        {
            cli();
            enc_events[y].id = 0;
            sei();
            break;
        }
    }
}

void StopAll(bool lock)
{
    if(lock)
        state |= STATE_LOCKED;
    setMotorPower(0, 0);

    clearLed();
    clean_indirect_sensors();
    clean_dc_motor();
}

void StartAll(bool unlock)
{
    if(unlock)
        state &= ~(STATE_LOCKED);
         
    init_timer_servo();
    init_dc_motor();
   // init_indirect_sensors();
   SetMovementByFlags();
}

uint32_t test()
{
    uint32_t encRes = 0;
    setServoByFlags(SERVO_DOORS, 405);
    setServoByFlags(SERVO_BRUSHES, 0);
    setServoByFlags(SERVO_REEL, 1);
    wait(1000000);
    setServoByFlags(SERVO_DOORS, -312);
    setServoByFlags(SERVO_BRUSHES, 690);

    clearEnc(false);
    setMotorPower(127, 127);
    wait(800000);
    setMotorPower(0, 0);
    encRes = uint16_t(getLeftEnc() << 16);
    encRes |= uint16_t(getRightEnc());
    clearEnc(false);
    return encRes;
}

