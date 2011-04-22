
enum Buttons
{
    BUTTON_START     = 0x01,
    BUTTON_PAWN      = 0x02
};

#define FINDER_FRONT1    0xE0

inline uint16_t fabs(int16_t num)
{
    if(num < 0)
        return -num;
    return num;
}

inline void SendRangeReq()
{
     uint8_t adr = FINDER_FRONT1;
     uint8_t data [2] = { 0, 0x51 };
     for(uint8_t i = 0; i < 6; ++i, adr +=2)
     {
         i2c.write(adr, &data[0], 2);
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
    uint8_t adr = FINDER_FRONT1;
    for(uint8_t i = 0; i < 6; ++i, adr +=2)
    {
        if(ReadRange(adr) <= RANGE_THRESHOLD)
            return true;
    }
    return false;
}
