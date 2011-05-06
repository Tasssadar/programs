

void sendPacket(Packet *pkt)
{
    rs232.sendCharacter(0xFF);
    rs232.sendCharacter(MASTER_ADDRESS);
    rs232.sendCharacter(pkt->m_lenght);
    rs232.sendCharacter(pkt->m_opcode);
    rs232.sendBytes(pkt->m_data, pkt->m_lenght);
}


uint8_t pktItr = 0;
Packet pkt;
uint8_t startItr = 0;
Packet pong(CMSG_PONG, 0);

bool readPacket()
{
    char c;
    if(!rs232.peek(c))
        return false;
    
    if(!startItr && uint8_t(c) == 0xFF)
    {
        startItr = 1;
        return false;
    }
    
    if(startItr && uint8_t(c) == DEVICE_ADDRESS)
    {
        pktItr = 1;
        startItr = 0;
    }
    else if(pktItr == 2)
        pkt.m_lenght = uint8_t(c);
    else if(pktItr == 3)
        pkt.m_opcode = uint8_t(c);
    else if(pktItr >= 4 && pktItr < pkt.m_lenght+4)
        pkt.m_data[pktItr-4] = uint8_t(c);
    else 
        return false;
    ++pktItr;

    if(pktItr > 3 && pkt.m_opcode != 0 && pktItr == pkt.m_lenght+4)
    {
        pktItr = 0;
        return true;
    }
    return false;
}

void handlePacket(Packet *pkt)
{
    if((state & STATE_LOCKED) && pkt->m_opcode != SMSG_UNLOCK && 
        pkt->m_opcode != SMSG_CONNECT_REQ)
    {
        Packet lock(CMSG_LOCKED, 1);
        lock.m_data[0] = (state & STATE_PAUSED);
        sendPacket(&lock);
        return;
    }
    switch(pkt->m_opcode)
    {
        case SMSG_PING:
        {
            sendPacket(&pong);
#ifdef PING
            pingTimer = PING_TIME;
             if(checkRange && rangeLastAdr == 0)
            {
                rangeLastAdr = FINDER_FRONT1;
                SendRangeReq();
            }
#endif
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
        case SMSG_SET_EMERGENCY_INFO:
            sendEmergency = (pkt->m_data[0] == 1) ? true : false;
            break;
        case SMSG_SET_SERVO_VAL:
        {
            int16_t val = (pkt->m_data[3] == 1) ? -(pkt->readUInt16(1)) : pkt->readUInt16(1);
            setServoByFlags(pkt->m_data[0], val);
            break;
        }
        case SMSG_ENCODER_START:
        case SMSG_ENCODER_STOP:
            clearEnc(false);
            break;
        case SMSG_ENCODER_GET:
        {    
            Packet encoder(CMSG_ENCODER_SEND, 4);
            encoder.setUInt16(0, fabs(getLeftEnc()));
            encoder.setUInt16(2, fabs(getRightEnc()));
            sendPacket(&encoder);
            break;
        }
        case SMSG_ENCODER_SET_EVENT:
        {
            setEncEvent(pkt->m_data[0], pkt->readUInt16(1), pkt->readUInt16(3));
            if(pkt->m_data[5] == 1)
                clearEnc(false);
            break;
        }
        case SMSG_ENCODER_RM_EVENT:
            removeEncEvent(pkt->m_data[0]);
            if(pkt->m_data[1] == 1)
                clearEnc(false);
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
        case SMSG_STOP:
        {
            StopAll(true);
            Packet lock(CMSG_LOCKED, 1);
            lock.m_data[0] = 0;
            sendPacket(&lock);
            break;
        }
        case SMSG_UNLOCK:
            StartAll(true);
#ifdef PING
            pingTimer = PING_TIME;
#endif
            break;
        case SMSG_CONNECT_REQ:
        {
            Packet res(CMSG_CONNECT_RES, 1);
            res.m_data[0] = (state & STATE_LOCKED) ? 1 : 0;
            sendPacket(&res);
            state &= ~(STATE_PAUSED);
            break;
        }
        case SMSG_TEST:
        {
            Packet res(CMSG_TEST_RESULT, 5);
            res.m_data[0] = uint8_t(isStartButtonPressed());
            uint32_t result = test();
            res.setUInt16(1, (result >> 16));
            res.setUInt16(3, (result & 0xFFFF));
            sendPacket(&res);
            break;
         }
        case SMSG_SHUTDOWN_RANGE:
            checkRange = false;
            break;
    }
}

inline void conLost()
{
    setMotorPower(0, 0);
    //clearEnc();
    StopAll(true);
    state &= ~(STATE_CORRECTION2);
    state |= STATE_PAUSED;
}

void checkEncEvent(bool right)
{
    cli();
    moveCheckTimer = MOVE_CHECK_TIME;
    sei();

    for(uint8_t y = 0; y < 5; ++y)
    {
        if(enc_events[y].id == 0)
            continue;
        if((enc_events[y].left != 0 && enc_events[y].left <= fabs(getLeftEnc())) ||
            (enc_events[y].right != 0 && enc_events[y].right <= fabs(getRightEnc())) ||
            (enc_events[y].left == 0 && enc_events[y].right == 0))
        {
            cli();
            Packet encoder(CMSG_ENCODER_EVENT_DONE, 1);
            encoder.m_data[0] = enc_events[y].id;
            enc_events[y].id = 0;
            sendPacket(&encoder);
            moveflags = MOVE_NONE;
            SetMovementByFlags();
            sei();
        }
    }
}

void discButton()
{
    if((state & STATE_BUTTON))
        return;
    state |= STATE_BUTTON;
    Packet button(CMSG_BUTTON_STATUS, 2);
    button.m_data[0] = BUTTON_PAWN;
    button.m_data[1] = 0x01;
    sendPacket(&button);
    clean_buttons();
}

void StartMatch()
{
    Packet button(CMSG_BUTTON_STATUS, 2);
    button.m_data[0] = BUTTON_START;
    button.m_data[1] = 0x01;
    sendPacket(&button);
}
