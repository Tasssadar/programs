enum Opcodes
{
    SMSG_PING                = 0x01,
    CMSG_PONG                = 0x02,
    SMSG_SET_MOVEMENT        = 0x03,
    SMSG_SET_CORRECTION_VAL  = 0x04,
    SMSG_GET_RANGE_VAL       = 0x05,
    CMSG_GET_RANGE_VAL       = 0x06,
    CMSG_EMERGENCY_START     = 0x07,
    CMSG_EMERGENCY_END       = 0x08,
    SMSG_SET_EMERGENCY_INFO  = 0x09,
    SMSG_SET_SERVO_VAL       = 0x10,
    SMSG_ENCODER_START       = 0x11,
    SMSG_ENCODER_GET         = 0x12,
    CMSG_ENCODER_SEND        = 0x13,
    SMSG_ENCODER_STOP        = 0x14,
    CMSG_LASER_GATE_STAT     = 0x15,
    SMSG_LASER_GATE_SET      = 0x16,
    CMSG_BUTTON_STATUS       = 0x17,
    SMSG_ADD_STATE           = 0x18,
    SMSG_REMOVE_STATE        = 0x19,
};

struct Packet
{
    Packet(uint8_t opcode = 0, uint8_t lenght = 0, uint8_t data[50] = 0)
    {
        m_opcode = opcode;
        m_lenght = lenght;
        for(uint8_t i = 0; i < lenght; ++i)
            m_data[i] = data[i];
    }

    uint8_t m_opcode;
    uint8_t m_lenght;
    uint8_t m_data[50];

    uint16_t readUInt16(uint8_t index) const { return ((m_data[index] << 8) | (m_data[index+1] & 0xFF)); }
    void setUInt16(uint8_t index, uint16_t val)
    {
        m_data[index] = uint8_t(val >> 8);
        m_data[index+1] = uint8_t(val & 0xFF);
    }
};

void sendPacket(Packet *pkt)
{
    rs232.sendCharacter(1);
    rs232.sendCharacter(pkt->m_opcode);
    rs232.sendCharacter(pkt->m_lenght);
    rs232.sendBytes(pkt->m_data, pkt->m_lenght);
}


uint8_t pktItr = 0;
Packet pkt;

bool readPacket()
{
    char c = rs232.get();
    
    if(pktItr == 1)
        pkt.m_opcode = uint8_t(c);
    else if(pktItr == 2)
        pkt.m_lenght = uint8_t(c);
    else if(pktItr >= 3 && pktItr < pkt.m_lenght+3)
        pkt.m_data[pktItr-3] = uint8_t(c);

    ++pktItr;

    if(pktItr > 2 && pkt.m_opcode != 0 && pktItr == pkt.m_lenght+3)
    {
        pktItr = 0;
        return true;
    }
    return false;
}

void handlePacket(Packet *pkt)
{
    switch(pkt->m_opcode)
    {
        case SMSG_PING:
        {
            Packet pong(CMSG_PONG, 0);
            sendPacket(&pong);
            break;
        }
        case SMSG_SET_MOVEMENT:
        {
            speed = pkt->m_data[0];
            moveflags = pkt->m_data[1];
            SetMovementByFlags();
            break;
        }
        case SMSG_SET_CORRECTION_VAL:
            correction_treshold = pkt->m_data[0];
            break;
        case SMSG_GET_RANGE_VAL:
        {
            Packet range(CMSG_GET_RANGE_VAL, 3);
            range.m_data[0] = pkt->m_data[0];
            range.setUInt16(1, ReadRange(pkt->m_data[0]));
            sendPacket(&range);
            break;
        }
        case SMSG_SET_EMERGENCY_INFO:
            sendEmergency = (pkt->m_data[0] == 1) ? true : false;
            break;
        case SMSG_SET_SERVO_VAL:
            setServoByFlags(pkt->m_data[0], pkt->m_data[1]);
            break;
        case SMSG_ENCODER_START:
            le.start();
            re.start();
            break;
        case SMSG_ENCODER_GET:
        {    
            Packet encoder(CMSG_ENCODER_SEND, 4);
            encoder.setUInt16(0, le.get());
            encoder.setUInt16(2, re.get());
            sendPacket(&encoder);
            break;
        }
        case SMSG_ENCODER_STOP:
            le.stop();
            re.stop();
            if(pkt->m_data[0] == 1)
            {
                le.clear();
                re.clear();
            }
            break;
        case SMSG_ADD_STATE:
            state |= pkt->m_data[0];
            break;
        case SMSG_REMOVE_STATE:
            state &= ~(pkt->m_data[0]);
            break;
        case SMSG_LASER_GATE_SET:
            //TODO implement
            break;
            
    }
}

inline void emergency(bool start)
{
    if((start && g_emergency) || (!start && !g_emergency))
        return;
    g_emergency = !g_emergency;


    if(!sendEmergency)
        return;

    if((emergencySent && !start) || (getTickCount() - startTime >= (1000000 * JUNIOR_WAIT_MUL / JUNIOR_WAIT_DIV) && start))
    {
        /*Packet emergency;
        emergency.m_opcode = start ? CMSG_EMERGENCY_START : CMSG_EMERGENCY_END;
        emergency.m_lenght = 0;
        sendPacket(&emergency);
       // startTime = getTickCount();
        emergencySent = start;*/
    }
      static uint8_t phase = 0;
    if(start)
    {
       
    
        if (phase < 32)
        {
            JUNIOR_CONCAT(PORT, JUNIOR_LED_PORT) |= (1<<JUNIOR_LED_PIN);
            JUNIOR_CONCAT(DDR , JUNIOR_LED_PORT) |= (1<<JUNIOR_LED_PIN);
        }
        else
        {
            JUNIOR_CONCAT(DDR , JUNIOR_LED_PORT) &= ~(1<<JUNIOR_LED_PIN);
            JUNIOR_CONCAT(PORT, JUNIOR_LED_PORT) &= ~(1<<JUNIOR_LED_PIN);
        }
    
        phase = (phase + 1) % 64;
    
    }
    else
        clearLed();
}
