enum Buttons
{
    BUTTON_START     = 0x01,
    BUTTON_PAWN      = 0x02
};
#define FINDER_FRONT1    0xE0

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
const uint8_t finFor[] = {0xE0, 0xE8, 0xEA};
const uint8_t backFor[] = {0xE2, 0xE4, 0xE6};

inline uint16_t fabs(int16_t num)
{
    if(num < 0)
        return -num;
    return num;
}

inline void SendRangeReq()
{
    // uint8_t adr = FINDER_FRONT1;
     uint8_t data [2] = { 0, 0x51 };
     for(uint8_t i = 0; i < 3; ++i)
     {
         i2c.write(moveflags ==MOVE_FORWARD ? finFor[i] : backFor[i], &data[0], 2);
         if(i2c.get_result().result != 2)
            continue;
     }
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

bool checkRange()
{
    //uint8_t adr = FINDER_FRONT1;
    for(uint8_t i = 0; i < 3; ++i)
    {
        if(ReadRange(moveflags ==MOVE_FORWARD ? finFor[i] : backFor[i]) <= RANGE_THRESHOLD)
            return true;
    }
    return false;
}
