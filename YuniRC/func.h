
void write_mem(Record *rec, uint16_t adress_beg)
{   
    // write keys
    for(uint8_t i = 0; i < 2; ++i)
    {
       while(EECR & (1<< EEPE)){} // wait for prev
       EEARH = uint8_t(adress_beg+i >> 8);
       EEARL = uint8_t(adress_beg+i);
       EEDR = rec->key[i];
       EECR |= (1<< EEMPE);
       EECR |= (1<< EEPE);
    }
    while(EECR & (1<< EEPE)){}
    //write time
    EEARH = uint8_t(adress_beg+2 >> 8);
    EEARL = uint8_t(adress_beg+2);
    EEDR = uint8_t((rec->time >> 8));
    EECR |= (1<< EEMPE);
    EECR |= (1<< EEPE);
    while(EECR & (1<< EEPE)){}
    EEARH = uint8_t(adress_beg+3 >> 8);
    EEARL = uint8_t(adress_beg+3);
    EEDR = uint8_t(rec->time & 0xFF);
    EECR |= (1<< EEMPE);
    EECR |= (1<< EEPE);
    while(EECR & (1<< EEPE)){}
    // and filled
 /*   EEARH = uint8_t(adress_beg+4 >> 8);
    EEARL = uint8_t(adress_beg+4);
    EEDR = uint8_t(rec->filled);
    EECR |= (1<< EEMPE);
    EECR |= (1<< EEPE); */
}

void read_mem(Record *rec, uint16_t adress_beg)
{
    while(EECR & (1<< EEPE)){} // wait for prev
    for(uint8_t i = 0; i < 2; ++i)
    {
       EEARH = uint8_t(adress_beg+i >> 8);
       EEARL = uint8_t(adress_beg+i);
       EECR |= (1<< EERE);
       rec->key[i] = EEDR; 
    }
    // time...
    EEARH = uint8_t(adress_beg+2 >> 8);
    EEARL = uint8_t(adress_beg+2);
    EECR |= (1<< EERE);  
    rec->time = (EEDR << 8);
    EEARH = uint8_t(adress_beg+3 >> 8);
    EEARL = uint8_t(adress_beg+3);
    EECR |= (1<< EERE);
    rec->time |= (EEDR & 0xFF);
    //filled
  /*  EEARH = uint8_t(adress_beg+4 >> 8);
    EEARL = uint8_t(adress_beg+4);
    EECR |= (1<< EERE);
    rec->filled = bool(EEDR); */
    
}

uint16_t ReadRange(uint8_t adress, uint8_t method = RANGE_CENTIMETRES)
{
    uint16_t range = 0;
    uint8_t data [2] = { 0, method };
    i2c.write(adress, &data[0], 2);
    if(i2c.get_result().result != 2)
        return 0;
    wait(10000); // maybe we can use smaller value
    i2c.write(adress, 0x02); // range High-byte
    if(i2c.get_result().result != 1)
        return 0;
    i2c.read(adress, 1);
    i2c.get_result();
    range = (8 << TWDR);
    i2c.write(adress, 0x03);
    if(i2c.get_result().result != 1)
        return 0;
    i2c.read(adress, 1);
    i2c.get_result();
    range |= TWDR;
    return range;
}

inline uint8_t findSpace(char key[])
{
    for(uint8_t i = 0; i < 10; ++i)
        if(key[i] == ' ')
            return i;
    return 0;
}
