#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include <util/delay.h>

#include "usart.h"


void delayMS(uint16_t ms)
{
	while(ms--)
		_delay_ms(1);
}

volatile uint16_t g_range;
int main()
{
	init_rs232();

	//DIDR0 = (1<<ADC0D);
    ADCSRA = (1<<ADEN)|(1<<ADSC)|(1<<ADIF)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
    //ADCSRB = 0;

    //ADMUX = (1<<REFS0);
    PORTA |= (1 << 0);
	
	sei();

	while(true)
	{
		cli(); 
		rs232.sendNumber(uint16_t((2914.0f/(float(g_range)+5.0f)-1.0f)*10.0f));
//		rs232.sendNumber(29140/(g_range+5)-1);
		sei();
		rs232.send("\f");
		delayMS(500);
	}
	
	cli();
	return 0;
}

ISR(ADC_vect)
{
	static bool initSensor = false;
    if (initSensor)
    {
        uint8_t adcl = ADCL;
        uint8_t adch = ADCH;

        g_range = (adch << 8) | (adcl);
       // ADMUX = (1<<REFS0);
    }

    initSensor = !initSensor;

    // Start the next conversion
    ADCSRA |= (1<<ADSC);
}