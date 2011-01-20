
inline int16_t fabs(int16_t num)
{
    if(num < 0)
        return -num;
    return num;
}

inline void write_byte(uint16_t adress, uint8_t byte)
{
    while(EECR & (1<< EEPE)){}
    EEARH = uint8_t(adress >> 8);
    EEARL = uint8_t(adress);
    EEDR = byte;
    EECR |= (1<< EEMPE);
    EECR |= (1<< EEPE);
}

inline uint8_t read_byte(uint16_t adress)
{
    while(EECR & (1<< EEPE)){}
    EEARH = uint8_t(adress >> 8);
    EEARL = uint8_t(adress);
    EECR |= (1<< EERE);
    return EEDR;
}

void write_mem(Record *rec, uint16_t adress_beg)
{   
    adress_beg += memBegin;
    // write keys
    for(uint8_t i = 0; i < 2; ++i)
       write_byte(adress_beg+i, rec->key[i]);
    // write stop event
    write_byte(adress_beg+2, rec->end_event);
    // write event params
    write_byte(adress_beg+3, rec->event_param[0]);
    write_byte(adress_beg+4, rec->event_param[1]);
}

void read_mem(Record *rec, uint16_t adress_beg)
{
    adress_beg += memBegin;
    while(EECR & (1<< EEPE)){} // wait for prev
    for(uint8_t i = 0; i < 2; ++i)
       rec->key[i] = read_byte(adress_beg+i); 
    // end event
    rec->end_event = read_byte(adress_beg+2);
    // event params
    rec->event_param[0] = read_byte(adress_beg+3);
    rec->event_param[1] = read_byte(adress_beg+4);
    
}

inline void erase_eeprom()
{
	for(uint16_t i = 0; i < 512; ++i)
		write_byte(i, 0);
}

inline uint16_t ReadRange(uint8_t adress, uint8_t method = RANGE_CENTIMETRES)
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
    clean_i2c();
    i2c.clear();
    return range;
}

inline bool EventHappened(Record *rec, uint32_t *nextPlayBase, uint32_t *nextPlay) 
{
    switch(rec->end_event)
    {
	    case EVENT_NONE:
		    return true;
        case EVENT_TIME:
            return (getTickCount() - *nextPlayBase >= *nextPlay);
        case EVENT_SENSOR_LEVEL_HIGHER:
            //if((rec->event_param[0] == 7 || rec->event_param[0] == 6) && getTickCount() - startTime < 1500000)
            //    return false;

            if(rec->event_param[0] == 8 && g_emergency && getTickCount() - startTime >= (1000000 * JUNIOR_WAIT_MUL / JUNIOR_WAIT_DIV))
			    return true;
            return (getSensorValue(rec->event_param[0]) >= rec->event_param[1]);
        case EVENT_SENSOR_LEVEL_LOWER:
            //if((rec->event_param[0] == 7 || rec->event_param[0] == 6) && getTickCount() - startTime < 1250000)
            //    return false;
            if(rec->event_param[0] == 8 && !g_emergency && getTickCount() - startTime >= (1000000 * JUNIOR_WAIT_MUL / JUNIOR_WAIT_DIV))
			    return true;
            return (getSensorValue(rec->event_param[0]) <= rec->event_param[1]);
        case EVENT_RANGE_HIGHER:
        case EVENT_RANGE_LOWER:
            // Uncomment to set messure delay
            //if(getTickCount() - *nextPlayBase < *nextPlay)
            //    return false;
            //*nextPlayBase = getTickCount();
            return (rec->end_event == EVENT_RANGE_HIGHER) ?
                (ReadRange(rec->event_param[0]) >= rec->event_param[1]) :
                (ReadRange(rec->event_param[0]) <= rec->event_param[1]);
        case EVENT_DISTANCE:
            if((fabs(encoder_play_r.get()) + fabs(encoder_play_l.get())) / 2 >= (rec->getBigNum()*MM))
                return true;
            break;
        case EVENT_DISTANCE_LEFT:
            if(fabs(encoder_play_l.get()) >= (rec->getBigNum()*MM))
                return true;
            break;
        case EVENT_DISTANCE_RIGHT:
            if(fabs(encoder_play_r.get()) >= (rec->getBigNum()*MM))
                return true;
            break;
    }
    return false;
}

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
