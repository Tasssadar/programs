
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

    int16_t readInt16(uint8_t index) const { return ((((int8_t)m_data[index]) << 8) | m_data[index+1]); }
    uint16_t readUInt16(uint8_t index) const { return ((m_data[index] << 8) | m_data[index+1]); }
    void setUInt16(uint8_t index, uint16_t val)
    {
        m_data[index] = uint8_t(val >> 8);
        m_data[index+1] = uint8_t(val & 0xFF);
    }
};

Packet pkt;
volatile uint8_t startItr = 0;
volatile uint8_t pktItr = 0;

bool readPacket()
{
    char c;
    if(!rs232.peek(c))
        return false;

    if(!startItr && !pktItr && uint8_t(c) == 0xFF)
    {
        startItr = 1;
        return false;
    }

    if(startItr && uint8_t(c) == 0x01)
    {
        pktItr = 1;
        startItr = 0;
    }
    else if(pktItr == 2)
        pkt.m_lenght = uint8_t(c)-1;
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

uint16_t abs(int16_t n) {return (n < 0) ? -n : n; } 

uint8_t GetNumWidth(int16_t num)
{
    uint8_t ret = num < 0 ? 1 : 0;
    num = abs(num);
    for(;num >= 10; ++ret)
        num /= 10;
    return ret+1;
}

void handlePacket(Packet *pkt)
{
    switch(pkt->m_opcode)
    {
        case 4:
        {
            setRightMotor(pkt->readInt16(0));
            setLeftMotor(pkt->readInt16(2));
            display.printNumToXY(pkt->readInt16(2), 0, 1);
            uint8_t spaces = 8-(GetNumWidth(pkt->readInt16(2))+GetNumWidth(pkt->readInt16(0)));
            for(;spaces > 0; --spaces)
                display.send_data(' ');
            display.printNumber(pkt->readInt16(0));
            break;
        }
    }
}