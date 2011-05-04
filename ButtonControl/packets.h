enum Opcodes
{
    /*SMSG_PING                = 0x01,
    CMSG_PONG                = 0x02,
    SMSG_SET_MOVEMENT        = 0x03,
    SMSG_SET_CORRECTION_VAL  = 0x04,
    CMSG_RANGE_BLOCK         = 0x05,
    CMSG_RANGE_BLOCK_GONE    = 0x06,
    CMSG_EMERGENCY_START     = 0x07,
    CMSG_EMERGENCY_END       = 0x08,
    SMSG_SET_EMERGENCY_INFO  = 0x09,
    SMSG_SET_SERVO_VAL       = 0x10,
    SMSG_ENCODER_START       = 0x11,
    SMSG_ENCODER_GET         = 0x12,
    CMSG_ENCODER_SEND        = 0x13,
    SMSG_ENCODER_STOP        = 0x14,
    SMSG_ENCODER_SET_EVENT   = 0x15,
    CMSG_ENCODER_EVENT_DONE  = 0x16,
    CMSG_LASER_GATE_STAT     = 0x17,
    SMSG_LASER_GATE_SET      = 0x18,*/
    CMSG_BUTTON_STATUS       = 0x19,
    /*SMSG_ADD_STATE           = 0x20,
    SMSG_REMOVE_STATE        = 0x21,
    SMSG_STOP                = 0x22,
    CMSG_LOCKED              = 0x23,
    SMSG_UNLOCK              = 0x24,
    SMSG_CONNECT_REQ         = 0x25,
    CMSG_CONNECT_RES         = 0x26,*/
    SMSG_TEST                = 0x27,
    CMSG_TEST_RESULT         = 0x28,
    //SMSG_ENCODER_RM_EVENT    = 0x29,
};

struct Packet
{
    Packet(uint8_t opcode = 0, uint8_t lenght = 0)
    {
        m_opcode = opcode;
        m_lenght = lenght;
    }

    uint8_t m_opcode;
    uint8_t m_lenght;
    uint8_t m_data[10];

    uint16_t readUInt16(uint8_t index) const { return ((m_data[index] << 8) | (m_data[index+1] & 0xFF)); }
    void setUInt16(uint8_t index, uint16_t val)
    {
        m_data[index] = uint8_t(val >> 8);
        m_data[index+1] = uint8_t(val & 0xFF);
    }
};

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
    switch(pkt->m_opcode)
    {
        case SMSG_TEST:
            //TODO
            break;
    }
}
