#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include <util/delay.h>

#include "usart.h"

static const char hex[] = "0123456789ABCDEF";

void USART_Transmit( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR1A & (1<<UDRE)) )
	;
	/* Put data into buffer, sends the data */
	UDR1 = data;
	
}

void send_usart1(const char * str)
{
    for (; *str != 0; ++str)
        USART_Transmit(*str);
    /* Wait for empty transmit buffer */
	while (!( UCSR1A & (1<<TXC1)) )
	;
	UCSR1A |= (1<<TXC1);
}

uint8_t HexToInt(char hex)
{
	// 0 to 9
	if(hex > 47 && hex < 58)
		return hex - 48;
	// A to F
	else
		return hex - 55;
}


int main()
{
	UCSR1B = (1<<TXEN1)|(1<<RXCIE1);
	UCSR1C = (1<<UCSZ11)|(1<<UCSZ10);
	UBRR1H = 383 >> 8;
	UBRR1L = uint8_t(383);

	init_rs232();
    sei();

	for(uint16_t i = 0; i < 200; ++i)
		_delay_ms(5);
	char ch;

	send_usart1("=  (00 m ) !");
	
	UCSR1A= 0;
	UCSR1B &= ~((1<<TXEN1));
	UCSR1B |= ((1<<RXEN1));
	
	volatile uint8_t counter1 = 0;
	volatile uint8_t counter2 = 0;

	uint16_t result = 0;

	uint16_t calib[3] = { 108, 98, 144 };
	
	while(true)
	{
		if(usart1.peek(ch))
		{
			++counter2;

			if(counter2 == 1)
				result = (HexToInt(ch) << 8);
			if(counter2 == 2)
				result = (HexToInt(ch) << 4);
			else if(counter2 == 3)
			{
				result |= (HexToInt(ch));
				uint8_t res = (result >> 2);
				res = (uint16_t(float((result*100)/calib[counter1])*2.5)>>2);
				++counter1;
				switch(counter1)
				{
					case 1:
						rs232.sendCharacter('R');
						break;
					case 2:
						rs232.sendCharacter('G');
						break;
					case 3:
						rs232.sendCharacter('B');
						break;
				}
				counter2 = 0;
				rs232.sendCharacter(':');
				rs232.sendCharacter(' ');
				rs232.sendCharacter(res);
				rs232.sendNumber(result);
				rs232.sendCharacter('\n');
			}
			if(counter1 >= 3)
				counter1 = 0;
		}
	}
	cli();
	return 0;
}
