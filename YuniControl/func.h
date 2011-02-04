inline int16_t fabs(int16_t num)
{
    if(num < 0)
        return -num;
    return num;
}


inline uint16_t ReadRange(uint8_t adress)//, uint8_t method = RANGE_CENTIMETRES)
{
    uint16_t range = 0;
    uint8_t data [2] = { 0, 0x51 };
    i2c.write(adress, &data[0], 2);
    if(i2c.get_result().result != 2)
        return 0;
    wait(70000); // maybe we can use smaller value
    i2c.write(adress, 0x02); // range High-byte
    if(i2c.get_result().result != 1)
        return 0;
    i2c.read(adress, 1);
    i2c.get_result();
    range = (8 << TWDR);
    //range = TWDR << 8;
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
