enum Opcodes
{
    SMSG_PING                = 0x01,
    CMSG_PONG                = 0x02,
    SMSG_SET_MOVEMENT        = 0x03,
    SMSG_SET_CORRECTION_VAL  = 0x04,
    SMSG_GET_RANGE_VAL       = 0x05,
    CMSG_GET_RANGE_VAL       = 0x06,
};

struct Packet
{
   /* Packet(uint8_t opcode, uint8_t lenght, uint8_t data[50])
    {
        m_opcode = opcode;
        m_lenght = lenght;
        for(uint8_t i = 0; i < 50; ++i)
            m_data[i] = data[i];
    }*/

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
            Packet pong;
            pong.m_opcode = CMSG_PONG;
            pong.m_lenght = 0;
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
            Packet range;
            range.m_opcode = CMSG_GET_RANGE_VAL;
            range.m_lenght = 3;
            range.m_data[0] = pkt->m_data[0];
            range.setUInt16(1, ReadRange(pkt->m_data[0]));
            sendPacket(&range);
            break;
    }
}
