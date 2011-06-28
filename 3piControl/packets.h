enum moveFlags
{
    MOVE_NONE         = 0x00,
    MOVE_FORWARD      = 0x01,
    MOVE_BACKWARD     = 0x02,
    MOVE_LEFT         = 0x04,
    MOVE_RIGHT        = 0x08,
    MOVE_LEFT_WHEEL   = 0x10,
    MOVE_RIGHT_WHEEL  = 0x20,
};

#define TURN_VALUE 50

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

volatile uint16_t speed = 0;
volatile uint8_t moveflags = 0;
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

void SetMovementByFlags();

void handlePacket(Packet *pkt)
{
    switch(pkt->m_opcode)
    {
        case 4:
            setRightMotor(pkt->readInt16(0));
            setLeftMotor(pkt->readInt16(2));
            break;
    }
}