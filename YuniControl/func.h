enum Buttons
{
    BUTTON_START     = 0x01,
    BUTTON_PAWN      = 0x02
};
#define FINDER_FRONT1    0xE0


enum Opcodes
{
    SMSG_PING                = 0x01,
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
    SMSG_LASER_GATE_SET      = 0x18,
    CMSG_BUTTON_STATUS       = 0x19,
    SMSG_ADD_STATE           = 0x20,
    SMSG_REMOVE_STATE        = 0x21,
    SMSG_STOP                = 0x22,
    CMSG_LOCKED              = 0x23,
    SMSG_UNLOCK              = 0x24,
    SMSG_CONNECT_REQ         = 0x25,
    CMSG_CONNECT_RES         = 0x26,
    SMSG_TEST                = 0x27,
    CMSG_TEST_RESULT         = 0x28,
    SMSG_ENCODER_RM_EVENT   = 0x29,
    CMSG_RANGE_VALUE        = 0x30,
    SMSG_SHUTDOWN_RANGE     = 0x31,
    CMSG_DEADEND_DETECTED   = 0x32,
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
void sendPacket(Packet *pkt);
uint8_t rangeLastAdr = 0;
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
uint8_t moveflags;

inline uint16_t fabs(int16_t num)
{
    if(num < 0)
        return -num;
    return num;
}

inline void SendRangeReq()
{
     uint8_t data [2] = { 0, 0x51 };
     i2c.write(rangeLastAdr, &data[0], 2);
     i2c.get_result();
     rangeTimer = RANGE_TIME;
     checkRangeNow = true;
}

inline uint16_t ReadRange(uint8_t adress)//, uint8_t method = RANGE_CENTIMETRES)
{
    uint16_t range = 0;
    i2c.write(adress, 0x02); // range High-byte
    if(i2c.get_result().result != 1)
        return range;

    i2c.read(adress, 1);
    i2c.get_result();
    range = (8 << TWDR);
    i2c.write(adress, 0x03);
    if(i2c.get_result().result != 1)
        return range;
    i2c.read(adress, 1);
    i2c.get_result();
    range |= TWDR;
    clean_i2c();
    i2c.clear();
    return range;
}

bool checkRangeFunc()
{
    Packet pkt(CMSG_RANGE_VALUE, 3);
    uint16_t range = 0;
    bool pass = false;
    range = ReadRange(rangeLastAdr);
    pkt.m_data[0] = rangeLastAdr;
    pkt.setUInt16(1, range);
    sendPacket(&pkt);
    if(range <= RANGE_THRESHOLD)
        pass = true;
    rangeLastAdr +=2;
    if(rangeLastAdr <= 0xE8)
        SendRangeReq();
    else
        rangeLastAdr = 0;
    
    return pass;
}
