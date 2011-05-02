enum Buttons
{
    BUTTON_START     = 0x01,
    BUTTON_PAWN      = 0x02,
    BUTTON_LEFT      = 0x04,
    BUTTON_RIGHT     = 0x08,
    BUTTON_BACK_LEFT = 0x10,
    BUTTON_BACK_RIGHT= 0x20,
};

inline void init_buttons()
{
    PORTC = 0xFF;
    PCMSK1 |= (1<<PCINT8)|(1<<PCINT9)|(1<<PCINT10)|(1<<PCINT11);
}

inline void clean_buttons()
{
    PORTC = 0;
    PCMSK1 &= ~((1<<PCINT8)|(1<<PCINT9)|(1<<PCINT10)|(1<<PCINT11));
}

inline void ButtonPressed(uint8_t address, bool pressed)
{
    Packet pkt(CMSG_BUTTON_STATUS, 2);
    pkt.m_data[0] = address;
    pkt.m_data[1] = uint8_t(pressed);
    sendPacket(&pkt);
}

ISR(PCINT8_vect)
{
    ButtonPressed(BUTTON_LEFT, !(PINC & (1<<PC0)));
}

ISR(PCINT9_vect)
{
    ButtonPressed(BUTTON_RIGHT, !(PINC & (1<<PC1)));
}

ISR(PCINT10_vect)
{
    ButtonPressed(BUTTON_BACK_LEFT, !(PINC & (1<<PC2)));
}

ISR(PCINT11_vect)
{
    ButtonPressed(BUTTON_BACK_RIGHT, !(PINC & (1<<PC3)));
} 