#define JUNIOR_F_CPU 20000000

#define JUNIOR_BOOTLOADER_AWARE 1
#define JUNIOR_BOOTLOADER_SEQ() \
    asm volatile ( \
        "ldi r31, 0x1F\n" \
        "ldi r30, 0x81\n" \
        "ijmp" \
        )

#define JUNIOR_RESET_SEQ() \
    asm volatile ( \
        "ldi r31, 0x0\n" \
        "ldi r30, 0x0\n" \
        "ijmp" \
        )

#ifndef JUNIOR_H_AVR
#define JUNIOR_H_AVR

#ifdef JUNIOR_ON_IDLE
#define JUNIOR_DO_IDLE() JUNIOR_ON_IDLE()
#else
#define JUNIOR_DO_IDLE()
#endif

inline void nop()
{
    asm volatile ("nop");
}

#endif

#ifndef JUNIOR_H_COMMON
#define JUNIOR_H_COMMON

#ifndef __cplusplus
# error Nezapomente si v nastaveni AVR studia zapnout "-x c++" !
#endif

#if !defined(F_CPU) && !defined(JUNIOR_F_CPU)
# error Nemate nastavenou frekvenci procesoru!
#endif

#ifndef JUNIOR_F_CPU
# define JUNIOR_F_CPU F_CPU
#endif

#ifndef F_CPU
# define F_CPU JUNIOR_F_CPU
#endif

#if defined(F_CPU) && F_CPU != JUNIOR_F_CPU
# error Mate v nastaveni spatne nastavenou frekvenci procesoru !
#endif

#define JUNIOR_CONCAT2(x, y) x ## y
#define JUNIOR_CONCAT(x, y) JUNIOR_CONCAT2(x, y)

#endif
/*
template <typename T>
T load_eeprom(uint8_t address)
{
    T res;
    char * ptr = (char *) &res;
    char * pend = ptr + sizeof res;
    
    EEARH = 0;
    EEARL = address;

    while (ptr != pend)
    {
        EECR = (1<<EERE);
        *ptr++ = EEDR;
        ++EEARL;
    }
    
    return res;
}

template <typename T>
void store_eeprom(uint8_t address, T value)
{
    char * ptr = (char *) &value;
    char * pend = ptr + sizeof value;
    
    EEARH = 0;
    EEARL = address;

    while (ptr != pend)
    {
        EEDR = *ptr++;
        
        EECR = (1<<EEMPE);
        EECR = (1<<EEPE);
        
        while (EECR & (1<<EEPE))
        {
        }

        ++EEARL;
    }
}
 */
#define JUNIOR_SERVO_TOP  0xc000
#define JUNIOR_SERVO_ZERO 3750
#define JUNIOR_SERVO_MUL 10
#define JUNIOR_WAIT_MUL 5
#define JUNIOR_WAIT_DIV 2

// Define the following symbols to use this module:
//   - JUNIOR_SERVO_TOP  -- the value at which the servo's clock overflow
//   - JUNIOR_SERVO_ZERO -- the value that causes the servo to move to its central position
//   - JUNIOR_SERVO_MUL  -- the multiplier applied to the control signals (-127..127) to calculate the new pulse length

#ifndef JUNIOR_NO_TIMER

#if !defined(JUNIOR_SERVO_TOP) || !defined(JUNIOR_SERVO_ZERO) || !defined(JUNIOR_SERVO_MUL)
# error Nemate spravne nakonfigurovany modul serv. Kouknete do timer_servo.inc.
#endif

volatile uint32_t g_tickCounter = 0;

ISR(TIMER1_OVF_vect)
{
    g_tickCounter += JUNIOR_SERVO_TOP;
#ifdef JUNIOR_ON_TIMER
    JUNIOR_ON_TIMER();
#endif
}

inline uint32_t getTickCount()
{
    uint32_t res;
    uint8_t ctsl, ctsh, tmp;
    
    do
    {
        ctsl = TCNT1L;
        ctsh = TCNT1H;
        
        cli();
        res = g_tickCounter;
        sei();
                
        tmp = TCNT1L;
    }
    while (ctsh > TCNT1H);
    
    return res + (uint16_t(ctsh << 8) | uint16_t(ctsl));
}

inline void clearTickCounter()
{
    TCNT1H = 0;
    TCNT1L = 0;
    g_tickCounter = 0;
}

/*
 * Waits for a given number of wait-intervals.
 * Currently, a wait-interval is 128 us.
 */
inline void wait(uint32_t time)
{
    time = time * JUNIOR_WAIT_MUL / JUNIOR_WAIT_DIV;
    uint32_t base = getTickCount();
    while (getTickCount() - base < time)
    {
        JUNIOR_DO_IDLE();
    }
}

inline void waitForever()
{
    for (;;)
        asm volatile ("nop");
}

inline void init_timer_servo()
{
#ifndef JUNIOR_NO_SERVO
    DDRB |= (1<<1)|(1<<2);
#endif

    // Setup the PWM generation for the servo cotrollers.
    // Fast PWM with TOP=ICR1 and non-inverting compare match behavior.
    // Default value for the output compare match registers is 1500
    // so the duty cycle will be 1.5 ms.
    
    // Since the timer is shared with the global clock,
    // The period is set to 16 ms, or more precisely to 2**14.
    TCNT1H = 0;
    TCNT1L = 0;
    ICR1H  = (uint8_t) ((JUNIOR_SERVO_TOP - 1) >> 8);
    ICR1L  = (uint8_t) ((JUNIOR_SERVO_TOP - 1));

#ifndef JUNIOR_NO_SERVO
    OCR1AH = (uint8_t) ((JUNIOR_SERVO_TOP - 1) >> 8);
    OCR1AL = (uint8_t) ((JUNIOR_SERVO_TOP - 1));
    OCR1BH = (uint8_t) ((JUNIOR_SERVO_TOP - 1) >> 8);
    OCR1BL = (uint8_t) ((JUNIOR_SERVO_TOP - 1));
    TCCR1A = (1<<COM1A1)|(1<<COM1A0)|(1<<COM1B1)|(1<<COM1B0)|(1<<WGM11);
#else
    TCCR1A = (1<<WGM11);
#endif
    TCCR1B = (1<<WGM12)|(1<<WGM13)|(1<<CS11);
    TIMSK1 = (1<<TOIE1);
}

inline void clean_timer_servo()
{
    TCCR1B = 0;
    TCCR1A = 0;
#ifndef JUNIOR_NO_SERVO
    DDRB &= ~((1<<1)|(1<<2));
#endif
}

inline void stop_timer_servo()
{
#ifndef JUNIOR_NO_SERVO
    OCR1AH = (uint8_t) ((JUNIOR_SERVO_TOP - 1) >> 8);
    OCR1AL = (uint8_t) ((JUNIOR_SERVO_TOP - 1));
    OCR1BH = (uint8_t) ((JUNIOR_SERVO_TOP - 1) >> 8);
    OCR1BL = (uint8_t) ((JUNIOR_SERVO_TOP - 1));
#endif
}

#ifndef JUNIOR_NO_SERVO
namespace detail {

inline void stopLeftServo()
{
    OCR1AH = (uint8_t) ((JUNIOR_SERVO_TOP - 1) >> 8);
    OCR1AL = (uint8_t) ((JUNIOR_SERVO_TOP - 1));
}

inline void stopRightServo()
{
    OCR1BH = (uint8_t) ((JUNIOR_SERVO_TOP - 1) >> 8);
    OCR1BL = (uint8_t) ((JUNIOR_SERVO_TOP - 1));
}

inline void setLeftServo(int16_t left)
{
    uint16_t l = (JUNIOR_SERVO_TOP - 1) - (left * 4 + 3150);
    OCR1AH = (uint8_t) (l >> 8);
    OCR1AL = (uint8_t) (l);
}

inline void setRightServo(int16_t right)
{
    uint16_t r = (JUNIOR_SERVO_TOP - 1) - (right * 4 + 3150);
    OCR1BH = (uint8_t) (r >> 8);
    OCR1BL = (uint8_t) (r);
}

}

#endif

#else

inline void init_timer_servo()
{
}

inline void clean_timer_servo()
{
}

inline void stop_timer_servo()
{
}

#endif

#ifndef JUNIOR_NO_TIMER
#ifndef JUNIOR_NO_SERVO

inline void stopLeftServo()
{
    detail::stopLeftServo();
}

inline void stopRightServo()
{
    detail::stopRightServo();
}

inline void setServoPos(int16_t left, int16_t right)
{
    detail::setLeftServo(left);
    detail::setRightServo(right);
}

inline void setLeftServo(int16_t left)
{
    detail::setLeftServo(left);
}

inline void setRightServo(int16_t right)
{
    detail::setRightServo(right);
}

#endif
#endif



namespace detail {

inline void setLeftMotor(int8_t left)
{
    if (left == 0)
    {
        OCR2A = 0;
        OCR2B = 0;
    }
    else if (left < 0)
    {
        OCR2A = 0;
        OCR2B = -left * 2 + 1;
    }
    else
    {
        OCR2A = left * 2 + 1;
        OCR2B = 0;
    }
}

inline void setRightMotor(int8_t right)
{
    if (right == 0)
    {
        OCR0A = 0;
        OCR0B = 0;
    }
    else if (right > 0)
    {
        OCR0A = 0;
        OCR0B = right * 2 + 1;
    }
    else
    {
        OCR0A = -right * 2 + 1;
        OCR0B = 0;
    }
}

}

volatile int8_t g_req_speed_left = 0, g_req_speed_right = 0;

ISR(TIMER0_OVF_vect)
{
    static uint8_t prescaler = 0;
    
    if (++prescaler != 3)
        return;
        
    prescaler = 0;

    static int8_t speed_l = 0, speed_r = 0;
    
    int8_t reql = g_req_speed_left;
    if (speed_l != reql)
    {
        if (speed_l < reql)
            ++speed_l;
        else
            --speed_l;
        
        detail::setLeftMotor(speed_l);
    }

    int8_t reqr = g_req_speed_right;
    if (speed_r != reqr)
    {
        if (speed_r < reqr)
            ++speed_r;
        else
            --speed_r;
        
        detail::setRightMotor(speed_r);
    }
}

inline void setLeftMotor(int8_t left)
{
    g_req_speed_left = left;
//	detail::setLeftMotor(left);
}

inline void setRightMotor(int8_t right)
{
    g_req_speed_right = right;
//	detail::setRightMotor(right);
}

inline void stopLeftMotor()
{
//	OCR2A = 0;
//	OCR2B = 0;
    setLeftMotor(0);
}

inline void stopRightMotor()
{
//	OCR0A = 0;
//	OCR0B = 0;
    setRightMotor(0);
}

inline void setMotorPower(int8_t left, int8_t right)
{
    sendPowerReq(uint8_t(fabs(left)), uint8_t(fabs(right)));
    setLeftMotor(left);
    setRightMotor(right);
}

inline void init_dc_motor()
{
    OCR0A = 0;
    OCR0B = 0;
    TIMSK0 = (1<<TOIE0);
    TCCR0A = (1<<COM0A1)|(1<<COM0A0)|(1<<COM0B1)|(1<<COM0B0)|(1<<WGM00);

    OCR2A = 0;
    OCR2B = 0;
    TIMSK2 = 0;
    ASSR = 0;
    TCCR2A = (1<<COM2A1)|(1<<COM2A0)|(1<<COM2B1)|(1<<COM2B0)|(1<<WGM20);

    TCCR0B = (1<<CS01);//|(1<<CS00);
    TCCR2B = (1<<CS21);

    GTCCR = (1<<TSM)|(1<<PSRSYNC)|(1<<PSRASY);
    TCNT0 = 0;
    TCNT2 = 0;
    GTCCR = (1<<PSRSYNC)|(1<<PSRASY);
    
    DDRB |= (1<<3);
    DDRD |= (1<<3)|(1<<5)|(1<<6);
}

inline void clean_dc_motor()
{
    TCCR0A = 0;
    TCCR0B = 0;
    TCCR2A = 0;
    TCCR2B = 0;
}

inline void stop_dc_motor()
{
    OCR0A = 0;
    OCR0B = 0;
    OCR2A = 0;
    OCR2B = 0;
}


#define JUNIOR_VOLTAGE_SENSOR 6
#define JUNIOR_VOLTAGE_THRESHOLD 700

/*struct ground_sensors_t
{
    uint16_t value[8];
};*/

//volatile struct ground_sensors_t g_sensors;
uint16_t g_threshold = 512;

//inline void begin_emergency();
//inline void end_emergency();

inline void emergency(bool start);

#ifndef JUNIOR_VOLTAGE_SENSOR
#define JUNIOR_VOLTAGE_SENSOR 7
#endif

#ifndef JUNIOR_VOLTAGE_THRESHOLD
#define JUNIOR_VOLTAGE_THRESHOLD 737
#endif

ISR(ADC_vect)
{
    static uint8_t currentSensor = 0;
    static bool initSensor = false;
    //static const uint8_t sensorMap[8] = { 5, 4, 0, 1, 2, 3, 6, 7 };

    if (initSensor)
    {
        uint8_t adcl = ADCL;
        uint8_t adch = ADCH;

        uint16_t value = (adch << 8) | (adcl);
        //g_sensors.value[sensorMap[currentSensor]] = value;
        if(currentSensor == 0 && checkForStart && value - g_threshold == 511)
        {
            checkForStart = false;
            StartMatch();
        }

        /*if (currentSensor == JUNIOR_VOLTAGE_SENSOR)
        {
            emergency(value < JUNIOR_VOLTAGE_THRESHOLD);
            if (value < JUNIOR_VOLTAGE_THRESHOLD)
                begin_emergency();
            else
                end_emergency(); 
        }*/

        currentSensor = (currentSensor + 1) & 0x07;

        PORTC = (PORTC & ~((1<<0)|(1<<1)|(1<<2))) | currentSensor;
    }
    
    initSensor = !initSensor;
    
    // Start the next conversion
    if(checkForStart)
        ADCSRA |= (1<<ADSC);
}

inline void init_indirect_sensors()
{
    // Setup the AD converter.
    // AVCC with external capacitor at AREF pin is used as the voltage reference.
    // Voltage is read from ADC3.
    // The prescaler is set to 1/128 -- from 20.0 MHz system clock,
    // the ADC frequency is 156.25kHz.
    // The AD will be restarted from ISR after each conversion finished.
    // The first conversion will be started immediately.
    DIDR0 = (1<<3);
    ADMUX = (1<<REFS0)|3;
    ADCSRB = 0;
    ADCSRA = (1<<ADEN)|(1<<ADSC)|(1<<ADIF)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
    
    DDRC |= (1<<0)|(1<<1)|(1<<2);
    DDRC &= ~(1<<3); 
    
  /*  g_sensors.value[0] = 1024;
    g_sensors.value[1] = 1024;
    g_sensors.value[2] = 1024;
    g_sensors.value[3] = 1024;
    g_sensors.value[4] = 1024;
    g_sensors.value[5] = 1024;
    g_sensors.value[6] = 1024;
    g_sensors.value[7] = 1024; */
}

inline void clean_indirect_sensors()
{
    ADCSRA = 0;
    DDRC &= 0xf0;
}

inline void stop_indirect_sensors()
{
}

/*inline int16_t getSensorValue(uint8_t index)
{
    cli();
    while (g_sensors.value[index] == 1024)
    {
        sei();
        nop();
        cli();
    }
    int16_t res = g_sensors.value[index] - g_threshold;
    sei();
    nop();

    return res;
} 

inline void calibrate_sensors()
{
    uint32_t avg = getSensorValue(0);
    avg += getSensorValue(1);
    avg += getSensorValue(2);
    avg += getSensorValue(3);
    
    g_threshold = (uint16_t) (avg / 4);
}*/

#define JUNIOR_LED_PORT D
#define JUNIOR_LED_PIN 7

#ifndef JUNIOR_LED_PORT
#define JUNIOR_LED_PORT D
#endif

#ifndef JUNIOR_LED_PIN
#define JUNIOR_LED_PIN 7
#endif

volatile bool g_emergency = false;
volatile bool g_led_status = false;
volatile bool g_power_off = false;

inline void init_single_led_power_off()
{
    JUNIOR_CONCAT(DDR , JUNIOR_LED_PORT) &= ~(1<<JUNIOR_LED_PIN);
    JUNIOR_CONCAT(PORT, JUNIOR_LED_PORT) &= ~(1<<JUNIOR_LED_PIN);
}

inline void clean_single_led_power_off()
{
    JUNIOR_CONCAT(DDR , JUNIOR_LED_PORT) &= ~(1<<JUNIOR_LED_PIN);
    JUNIOR_CONCAT(PORT, JUNIOR_LED_PORT) &= ~(1<<JUNIOR_LED_PIN);
}

inline void setLed()
{
    JUNIOR_CONCAT(PORT, JUNIOR_LED_PORT) |= (1<<JUNIOR_LED_PIN);
    JUNIOR_CONCAT(DDR , JUNIOR_LED_PORT) |= (1<<JUNIOR_LED_PIN);
    g_led_status = true;
}

inline void clearLed()
{
   // JUNIOR_CONCAT(DDR , JUNIOR_LED_PORT) &= ~(1<<JUNIOR_LED_PIN);
     JUNIOR_CONCAT(DDR , JUNIOR_LED_PORT) |= (1<<JUNIOR_LED_PIN);
    JUNIOR_CONCAT(PORT, JUNIOR_LED_PORT) &= ~(1<<JUNIOR_LED_PIN);
    g_led_status = false;
}

inline void toggleLed()
{
    if (g_led_status)
        clearLed();
    else
        setLed();
}

/*inline void begin_emergency()
{
    g_emergency = true;
    if (g_power_off)
        return;
    
    static uint8_t phase = 0;
    
    if (phase < 32)
    {
        JUNIOR_CONCAT(PORT, JUNIOR_LED_PORT) |= (1<<JUNIOR_LED_PIN);
        JUNIOR_CONCAT(DDR , JUNIOR_LED_PORT) |= (1<<JUNIOR_LED_PIN);
    }
    else
    {
        JUNIOR_CONCAT(DDR , JUNIOR_LED_PORT) &= ~(1<<JUNIOR_LED_PIN);
        JUNIOR_CONCAT(PORT, JUNIOR_LED_PORT) &= ~(1<<JUNIOR_LED_PIN);
    }
    
    phase = (phase + 1) % 64;
}

inline void end_emergency()
{
    g_emergency = false;
    if (!g_power_off)
    {
        if (g_led_status)
            setLed();
        else
            clearLed();
    }
} */

inline void stop_single_led_power_off()
{
}

inline void power_off()
{
    g_power_off = true;
    JUNIOR_CONCAT(PORT, JUNIOR_LED_PORT) &= ~(1<<JUNIOR_LED_PIN);
    JUNIOR_CONCAT(DDR , JUNIOR_LED_PORT) |= (1<<JUNIOR_LED_PIN);
}


#ifndef JUNIOR_RS232_BPS
#define JUNIOR_RS232_BPS 38400
#endif

#ifndef JUNIOR_NO_RS232

#ifndef JUNIOR_BOOTLOADER_AWARE
# define JUNIOR_BOOTLOADER_AWARE 0
#endif

#ifndef JUNIOR_RS232_SIGNAL
#define JUNIOR_RS232_SIGNAL USART
#endif

#ifndef JUNIOR_RS232_TXBUF
# define JUNIOR_RS232_TXBUF 96
#endif

#ifndef JUNIOR_RS232_RXBUF
# define JUNIOR_RS232_RXBUF 32
#endif

#ifndef JUNIOR_RS232_BPS
# error Nastavte symbol JUNIOR_RS232_BPS na rychlost, s jakou chcete komunikovat (napr. 115200).
#endif

#if JUNIOR_BOOTLOADER_AWARE
void clean();
#endif

inline void force_wd_reset()
{
    cli();
#if defined(WDTCR)
    WDTCR = (1<<WDCE)|(1<<WDE);
    WDTCR = (1<<WDE);
#elif defined(WDTCSR)
    WDTCSR = (1<<WDCE)|(1<<WDE);
    WDTCSR = (1<<WDE);
#else
# error Unsupported Watchdog timer interface.
#endif
    for (;;)
    {
    }
}

#ifndef JUNIOR_H_QUEUE_H
#define JUNIOR_H_QUEUE_H

namespace detail {

template <typename T, uint8_t size>
class queue
{
public:
    queue()
        : m_rd_ptr(0), m_wr_ptr(0)
    {
    }

    bool push(T t)
    {
        uint8_t wr = m_wr_ptr;
        
        uint8_t new_ptr = inc(wr);

        if (new_ptr == m_rd_ptr)
            return false;

        m_elems[wr] = t;
        m_wr_ptr = new_ptr;
        return true;
    }

    bool empty() const
    {
        return m_rd_ptr == m_wr_ptr;
    }
    
    bool full() const
    {
        return inc(m_wr_ptr) == m_rd_ptr;
    }

    T top() const
    {
        return m_elems[m_rd_ptr];
    }

    void pop()
    {
        m_rd_ptr = inc(m_rd_ptr);
    }

private:
    T m_elems[size];
    volatile uint8_t m_rd_ptr;
    volatile uint8_t m_wr_ptr;
    
    uint8_t inc(uint8_t v) const
    {
        ++v;
        return v == size? 0: v;
    }
};

}

#endif


namespace detail {

template <int rxmax, int txmax>
class rs232_t
{
public:
    void data_in(char ch)
    {
        m_rxbuf.push(ch);
    }

    bool data_out(char & ch)
    {
        if (m_txbuf.empty())
            return false;
            
        ch = m_txbuf.top();
        m_txbuf.pop();
        return true;
    }

    char get()
    {
        char ch;

        while (!this->peek(ch))
        {
            JUNIOR_DO_IDLE();
        }

        return ch;
    }

    bool peek(char & ch)
    {
        if (m_rxbuf.empty())
            return false;

        ch = m_rxbuf.top();
        m_rxbuf.pop();
        return true;
    }

    void sendCharacter(char ch)
    {
        m_txbuf.push(ch);
        UCSR0B |= (1<<UDRIE0);
    }

    void sendBytes(uint8_t data[], uint8_t lenght)
    {
        for(uint8_t i = 0; i < lenght; ++i)
            sendCharacter(char(data[i]));
    }
    
    void send(const char * str)
    {
        for (; *str != 0; ++str)
            m_txbuf.push(*str);
        UCSR0B |= (1<<UDRIE0);
    }

    void sendHexByte(uint8_t byte)
    {
        static const char hexdigits[] = "0123456789ABCDEF";
        this->sendCharacter(hexdigits[byte >> 4]);
        this->sendCharacter(hexdigits[byte & 0x0f]);
    } 

    template <typename T>
    void sendNumberHex(T n, uint8_t width = 0)
    {
        char buf[32];
        uint8_t len = 0;

        if (n != 0)
        {
            T a = (n < 0)? -n: n;

            while (a > 0)
            {
                buf[len++] = '0' + (a & 0x0f);
                a = a >> 4;
            }

            if (n < 0)
                buf[len++] = '-';
        }
        else
            buf[len++] = '0';

        for (; width > len; --width)
            m_txbuf.push(' ');
        for (; len > 0; --len)
            m_txbuf.push(buf[len-1]);
        UCSR0B |= (1<<UDRIE0);
    }

    template <typename T>
    void sendNumber(T n, uint8_t width = 0)
    {
        char buf[32];
        uint8_t len = 0;

        if (n != 0)
        {
            T a = (n < 0)? -n: n;

            while (a > 0)
            {
                T b = a / 10;
                buf[len++] = '0' + (a - b * 10);
                a = b;
            }

            if (n < 0)
                buf[len++] = '-';
        }
        else
            buf[len++] = '0';

        for (; width > len; --width)
            m_txbuf.push(' ');
        for (; len > 0; --len)
            m_txbuf.push(buf[len-1]);
        UCSR0B |= (1<<UDRIE0);
    }
    
    template <typename T>
    void dumpNumber(T n)
    {
        sendNumber(n);
        sendCharacter(' ');
    }
    
    void wait()
    {
        while (!m_txbuf.empty())
        {
            JUNIOR_DO_IDLE();
        }
    }
    
    bool txempty() const
    {
        return m_txbuf.empty();
    }

private:
    detail::queue<char, rxmax> m_rxbuf;
    detail::queue<char, txmax> m_txbuf;
};

}

detail::rs232_t<JUNIOR_RS232_RXBUF, JUNIOR_RS232_TXBUF> rs232;

inline void init_rs232()
{
    // Initialize the RS232 line
    UCSR0A = (1<<U2X0);
    UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
    
#define JUNIOR_UBRR ((F_CPU + 4 * JUNIOR_RS232_BPS) / (8 * JUNIOR_RS232_BPS) - 1)
    
    UBRR0H = JUNIOR_UBRR >> 8;
    UBRR0L = JUNIOR_UBRR;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
}

inline void stop_rs232()
{
}

inline void clean_rs232()
{
    UCSR0B = 0;
    UCSR0A = 0;
    UCSR0C = 0;
}

ISR(JUNIOR_CONCAT(JUNIOR_RS232_SIGNAL, _RX_vect))
{
    unsigned char ch = UDR0;

#if JUNIOR_BOOTLOADER_AWARE
    {
        static const unsigned char bootSeq[] = { 0x74, 0x7E, 0x7A, 0x33 };
        static uint8_t state = 0;

        if (ch == bootSeq[state])
        {
            if (++state == 4)
                force_wd_reset();
        }
        else
            state = 0;
    }
#endif

#if JUNIOR_RS232_STOP
//	inline void stop();

    {
        static const unsigned char bootSeq[] = { 27, 91, 75 };
        static uint8_t state = 0;

        if (ch == bootSeq[state])
        {
            if (++state == 3)
            {
                clean();
                JUNIOR_RESET_SEQ();
            }
        }
        else
            state = 0;
    }
#endif

    rs232.data_in(ch);
}

ISR(JUNIOR_CONCAT(JUNIOR_RS232_SIGNAL, _UDRE_vect))
{
    char ch;
    if (rs232.data_out(ch))
        UDR0 = ch;
    else
        UCSR0B &= ~(1<<UDRIE0);
}

#else

void init_rs232()
{
}

void stop_rs232()
{
}

void clean_rs232()
{
}

#endif


#ifndef JUNIOR_I2C_BPS
#define JUNIOR_I2C_BPS 400000
#endif

#ifndef JUNIOR_I2C_TXBUF
# define JUNIOR_I2C_TXBUF 32
#endif

#ifndef JUNIOR_I2C_RXBUF
# define JUNIOR_I2C_RXBUF 32
#endif

#ifndef JUNIOR_I2C_BPS
# error Nastavte rychlost i2c linky!
#endif

#ifndef JUNIOR_H_QUEUE_H
#define JUNIOR_H_QUEUE_H

namespace detail {

template <typename T, uint8_t size>
class queue
{
public:
    queue()
        : m_rd_ptr(0), m_wr_ptr(0)
    {
    }

    bool push(T t)
    {
        uint8_t wr = m_wr_ptr;
        
        uint8_t new_ptr = inc(wr);

        if (new_ptr == m_rd_ptr)
            return false;

        m_elems[wr] = t;
        m_wr_ptr = new_ptr;
        return true;
    }

    bool empty() const
    {
        return m_rd_ptr == m_wr_ptr;
    }
    
    bool full() const
    {
        return inc(m_wr_ptr) == m_rd_ptr;
    }

    T top() const
    {
        return m_elems[m_rd_ptr];
    }

    void pop()
    {
        m_rd_ptr = inc(m_rd_ptr);
    }

private:
    T m_elems[size];
    volatile uint8_t m_rd_ptr;
    volatile uint8_t m_wr_ptr;
    
    uint8_t inc(uint8_t v) const
    {
        ++v;
        return v == size? 0: v;
    }
};

}

#endif


struct i2c_transaction
{
    uint8_t address;
    uint8_t length;
    uint8_t result;
};

namespace detail {

class i2c_t
{
public:
    i2c_t()
        : m_on_slave_tx(0), m_max_slave_rx_len(0)
    {
        m_current_transaction.length = 0;
    }

    bool handle_slave_states(uint8_t state)
    {
        switch (state)
        {
        case 0x60:    // Own SLA+W has been received
//		case 0x68:    // Arbitration lost in SLA as Master, own SLA+W has been received
        case 0x70:    // GCA has been received
//		case 0x78:    // Arbitration lost in SLA as Master, GCA has been received
            m_current_transaction.address = 0x01;
            m_current_transaction.length = m_max_slave_rx_len;
            m_current_transaction.result = 0;

            if (m_current_transaction.result == m_current_transaction.length)
                TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
            else
                TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
            return true;

        case 0x80:    // Data has been received, SLA+W
        case 0x90:    // Data has been received, GCA
            if (!m_rx_queue.push(TWDR))
                m_current_transaction.length = m_current_transaction.result;
            else
                ++m_current_transaction.result;

            if (m_current_transaction.result == m_current_transaction.length)
                TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
            else
                TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
            return true;

        case 0x88:    // Addressed with SLA+W, data received, NACK returned
        case 0x98:    // Addressed with GCA, data received, NACK returned
        case 0xA0:    // Stop condition received
            m_results.push(m_current_transaction);
            TWCR = construct_twcr();
            return true;

        case 0xC0:    // Data byte has been transmitted, NACK was received
            TWCR = construct_twcr();
            return true;

        case 0xA8:    // Own SLA+R received, ACK returned
        case 0xB8:    // Data byte has been transmitted, ACK was received
            if (m_on_slave_tx == 0)
                TWDR = 0xff;
            else
                TWDR = m_on_slave_tx(m_on_slave_tx_data);
            TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
            return true;
        }
        return false;
    }

    bool handle_master_states(uint8_t state)
    {
        switch (state)
        {
        case 0x08:    // A START condition has been transmitted
        case 0x10:    // A repeated START condition has been transmitted
            if (m_master_actions.empty())
                return false;

            m_current_transaction = m_master_actions.top();
            m_master_actions.pop();
            TWDR = m_current_transaction.address;
            TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
            return true;

        case 0x18:    // SLA+W has been transmitted, ACK
        case 0x20:    // SLA+W has been transmitted, NACK
        case 0x28:    // Data byte has been transmitted, ACK
        case 0x30:    // Data byte has been transmitted, NACK
            if (m_current_transaction.result == m_current_transaction.length)
            {
                release_transmission();
                TWCR = construct_twcr(true);
            }
            else
            {
                TWDR = m_tx_queue.top();
                TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
                m_tx_queue.pop();
                ++m_current_transaction.result;
            }

            return true;

//		case 0x38:    // Arbitration lost
//			TWCR = construct_twcr();
//			return true;

        case 0x50:    // Data byte received, returned ACK
            if (!m_rx_queue.push(TWDR))
                return false;

            ++m_current_transaction.result;
            // no break
            
        case 0x40:    // SLA+R has been transmitted, ACK was received
            if (m_current_transaction.result + 1 < m_current_transaction.length)
                TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
            else
                TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);

            return true;

        case 0x58:    // Data byte received, returned NACK
            if (!m_rx_queue.push(TWDR))
                return false;

            ++m_current_transaction.result;
            // no break

        case 0x48:    // SLA+R has been transmitted, NACK was received
            m_results.push(m_current_transaction);
            TWCR = construct_twcr(true);
            return true;
        }
        return false;
    }

    void write(uint8_t address, uint8_t data)
    {
        this->write(address, &data, 1);
    }

    void write(uint8_t address, uint8_t * data, uint8_t length)
    {
        i2c_transaction tr = { address & 0xfe, 0, 0 };

        for (; tr.length < length; ++tr.length)
            if (!m_tx_queue.push(*data++))
                break;

        m_master_actions.push(tr);

        // FIXME: Force a START condition only if we're not already transmitting.
        TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWSTA)|(1<<TWEN)|(1<<TWIE);
    }

    void read(uint8_t address, uint8_t length)
    {
        i2c_transaction tr = { address | 1, length, 0 };
        m_master_actions.push(tr);

        // FIXME: Force a START condition only if we're not already transmitting.
        TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWSTA)|(1<<TWEN)|(1<<TWIE);
    }

    void max_slave_rx(uint8_t v)
    {
        m_max_slave_rx_len = v;
    }

    void on_slave_tx(uint8_t (*handler)(void * data), void * data)
    {
        m_on_slave_tx = handler;
        m_on_slave_tx_data = data;
    }

    uint8_t rx_get()
    {
        if(m_rx_queue.empty())
            return 255;
        uint8_t res = m_rx_queue.top();
        m_rx_queue.pop();
        return res;
    }

    void wait_for_result()
    {
        while (m_results.empty())
        {
        }
    }

    bool has_results() const
    {
        return !m_results.empty();
    }

    i2c_transaction get_result()
    {
        wait_for_result();
        i2c_transaction res = m_results.top();
        m_results.pop();
        return res;
    }

    void address(uint8_t address)
    {
        TWAR = address & 0xfe;
    }

    void clear()
    {
        while(!m_tx_queue.empty())
            m_tx_queue.pop();
        while(! m_rx_queue.empty())
              m_rx_queue.pop();
        while(!m_results.empty())
            m_results.pop();
        while(!m_master_actions.empty())
            m_master_actions.pop();
    }

private:
    uint8_t construct_twcr(bool active = false)
    {
        uint8_t res = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);

        if (!m_master_actions.empty())
            res |= (1<<TWSTA);
        else
            if (active)
                res |= (1<<TWSTO);

        return res;
    }

    void release_transmission()
    {
        for (uint8_t i = m_current_transaction.result; i < m_current_transaction.length; ++i)
            m_tx_queue.pop();

        m_results.push(m_current_transaction);
    }

    detail::queue<uint8_t, 64> m_tx_queue;
    detail::queue<uint8_t, 64> m_rx_queue;

    detail::queue<i2c_transaction, 16> m_master_actions;
    detail::queue<i2c_transaction, 16> m_results;

    uint8_t (* volatile m_on_slave_tx)(void * data);
    void * volatile m_on_slave_tx_data;

    uint8_t m_max_slave_rx_len;

    i2c_transaction m_current_transaction;
};

}

detail::i2c_t i2c;

inline void init_i2c()
{
#define JUNIOR_TWBR ((JUNIOR_F_CPU + JUNIOR_I2C_BPS) / (2 * JUNIOR_I2C_BPS) - 8)
    TWBR = JUNIOR_TWBR;
    TWSR = 0;
    TWAMR = 0;
    TWAR = 0;
    TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
}

inline void stop_i2c()
{
}

inline void clean_i2c()
{
    TWCR = 0;
}

ISR(TWI_vect)
{
    uint8_t state = TWSR & 0xf8;

    if (!i2c.handle_master_states(state)
        && !i2c.handle_slave_states(state))
    {
        // Bus error or an unexpected state,
        // reset the TWI module and indicate an error.
        TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    }
}



#ifndef JUNIOR_NO_TIMER
inline void wait(uint32_t);

inline void waitForStartButton()
{
    while ((PINB & (1<<0)) != 0)
        JUNIOR_DO_IDLE();
    wait(100000);
    while ((PINB & (1<<0)) == 0)
        JUNIOR_DO_IDLE();
}
#endif


inline bool isStartButtonPressed()
{
    return (PINB & (1<<0)) == 0;
}

inline void init_buttons()
{
    PORTB |= (1<<0)|(1<<4);
   // PCMSK0 |= (1<<PCINT0)|(1<<PCINT4);
}

inline void clean_buttons()
{
   // PORTB &= ~(1<<0);
     PCMSK0 &= ~(1<<PCINT0);
}

inline void stop_buttons()
{
}

namespace detail
{
    inline void waitForStartSignal(bool anychar, char ch)
    {
        char inch;
        
        for (;;)
        {
#ifndef JUNIOR_NO_TIMER
            if ((PINB & (1<<0)) == 0)
                break;
#endif
                
            if (rs232.peek(inch) && (anychar || inch == ch))
                return;
            
            JUNIOR_DO_IDLE();
        }

#ifndef JUNIOR_NO_TIMER
        wait(100000);

        for (;;)
        {
            if ((PINB & (1<<0)) != 0)
                break;
                
            if (rs232.peek(inch) && (anychar || inch == ch))
                return;
            
            JUNIOR_DO_IDLE();
        }
#endif
    }
}

inline void waitForStartSignal(char ch)
{
    detail::waitForStartSignal(false, ch);
}

inline void waitForStartSignal()
{
    detail::waitForStartSignal(true, 0);
}

#ifndef JUNIOR_NO_TIMER

#ifndef JUNIOR_H_ASYNC_COUNTER_H
#define JUNIOR_H_ASYNC_COUNTER_H

template <typename Getter>
class async_counter
{
public:
    async_counter()
        : m_value(Getter::get()), m_running(true)
    {
    }

    void start()
    {
        if (!m_running)
        {
            m_value = Getter::get() - m_value;
            m_running = true;
        }
    }

    void stop()
    {
        if (m_running)
        {
            m_value = Getter::get() - m_value;
            m_running = false;
        }
    }

    void clear()
    {
        if (m_running)
            m_value = Getter::get();
        else
            m_value = 0;
    }

    int32_t get() const
    {
        int32_t res;
        if (m_running)
            res = Getter::get() - m_value;
        else
            res = m_value;
        return res;
    }
    
    int32_t get_and_clear()
    {
        int32_t res;
        res = get();
        (*this) -= res;
        return res;
    }
    
    async_counter & operator -= (int32_t value)
    {
        if (m_running)
            m_value += value;
        else
            m_value -= value;
        return *this;
    }
    bool isRunning()
    {
        return m_running;
    }

private:
    int32_t m_value;
    bool m_running;
};

#endif


struct tickcount_getter
{
    static int32_t get()
    {
        return getTickCount() * JUNIOR_WAIT_DIV / JUNIOR_WAIT_MUL;
    }
};

class stopwatch : public async_counter<tickcount_getter>
{
public:
    int32_t getTime() const
    {
        return this->get();
    }
};

#endif


#define JUNIOR_ENCODER_LEFT_PORT D
#define JUNIOR_ENCODER_LEFT_PCI 2
#define JUNIOR_ENCODER_LEFT_vect PCINT2_vect
#define JUNIOR_ENCODER_LEFT_PIN1 2
#define JUNIOR_ENCODER_LEFT_PIN2 4
                       
#define JUNIOR_ENCODER_RIGHT_PORT B
#define JUNIOR_ENCODER_RIGHT_PCI 0
#define JUNIOR_ENCODER_RIGHT_vect PCINT0_vect
#define JUNIOR_ENCODER_RIGHT_PIN1 5
#define JUNIOR_ENCODER_RIGHT_PIN2 4
/*
typedef int32_t encoder_t;

volatile encoder_t g_left_encoder = 0;
volatile encoder_t g_right_encoder = 0;

ISR(JUNIOR_ENCODER_LEFT_vect)
{
    uint8_t port = JUNIOR_CONCAT(PIN, JUNIOR_ENCODER_LEFT_PORT);
    if (((port & (1<<JUNIOR_ENCODER_LEFT_PIN1)) != 0) == ((port & (1<<JUNIOR_ENCODER_LEFT_PIN2)) != 0))
        --g_left_encoder;
    else
        ++g_left_encoder;
}

ISR(JUNIOR_ENCODER_RIGHT_vect)
{
    uint8_t port = JUNIOR_CONCAT(PIN, JUNIOR_ENCODER_RIGHT_PORT);
    if (((port & (1<<JUNIOR_ENCODER_RIGHT_PIN1)) != 0) == ((port & (1<<JUNIOR_ENCODER_RIGHT_PIN2)) != 0))
        --g_right_encoder;
    else
        ++g_right_encoder;
}

encoder_t getLeftEncoder()
{
    cli();
    encoder_t res = g_left_encoder;
    sei();
    return res;
}

encoder_t getRightEncoder()
{
    cli();
    encoder_t res = g_right_encoder;
    sei();
    return res;
}*/

void init_encoders()
{
    JUNIOR_CONCAT(PCMSK, JUNIOR_ENCODER_LEFT_PCI) |= (1<<JUNIOR_ENCODER_LEFT_PIN1);
    JUNIOR_CONCAT(PCMSK, JUNIOR_ENCODER_RIGHT_PCI) |= (1<<JUNIOR_ENCODER_RIGHT_PIN1);
    PCIFR = (1<<JUNIOR_ENCODER_RIGHT_PCI)|(1<<JUNIOR_ENCODER_LEFT_PCI);
    PCICR = (1<<JUNIOR_ENCODER_RIGHT_PCI)|(1<<JUNIOR_ENCODER_LEFT_PCI);
}

void stop_encoders()
{
}

void clean_encoders()
{
    PCICR = 0;
    JUNIOR_CONCAT(PCMSK, JUNIOR_ENCODER_LEFT_PCI) = 0;
    JUNIOR_CONCAT(PCMSK, JUNIOR_ENCODER_RIGHT_PCI) = 0;
}

/*
#ifndef JUNIOR_H_ASYNC_COUNTER_H
#define JUNIOR_H_ASYNC_COUNTER_H

template <typename Getter>
class async_counter
{
public:
    async_counter()
        : m_value(Getter::get()), m_running(true)
    {
    }

    void start()
    {
        if (!m_running)
        {
            m_value = Getter::get() - m_value;
            m_running = true;
        }
    }

    void stop()
    {
        if (m_running)
        {
            m_value = Getter::get() - m_value;
            m_running = false;
        }
    }

    void clear()
    {
        if (m_running)
            m_value = Getter::get();
        else
            m_value = 0;
    }

    int32_t get() const
    {
        int32_t res;
        if (m_running)
            res = Getter::get() - m_value;
        else
            res = m_value;
        return res;
    }
    
    int32_t get_and_clear()
    {
        int32_t res;
        res = get();
        (*this) -= res;
        return res;
    }
    
    async_counter & operator -= (int32_t value)
    {
        if (m_running)
            m_value += value;
        else
            m_value -= value;
        return *this;
    }

private:
    int32_t m_value;
    bool m_running;
};

#endif


struct leftenc_getter
{
    static encoder_t get()
    {
        return getLeftEncoder();
    }
};

struct rightenc_getter
{
    static encoder_t get()
    {
        return getRightEncoder();
    }
};

typedef async_counter<leftenc_getter> left_encoder;
typedef async_counter<rightenc_getter> right_encoder;
*/

void run();

inline void init()
{
    init_timer_servo();
    init_dc_motor();
    init_indirect_sensors();
    init_single_led_power_off();
    init_rs232();
    init_i2c();
    init_buttons();
    init_encoders();
}

inline void clean()
{
    clean_encoders();
    clean_buttons();
    clean_i2c();
    clean_rs232();
    clean_single_led_power_off();
    clean_indirect_sensors();
    clean_dc_motor();
    clean_timer_servo();
}

inline void stop()
{
    stop_timer_servo();
    stop_dc_motor();
   // stop_indirect_sensors();
    stop_single_led_power_off();
    stop_rs232();
    stop_i2c();
    stop_buttons();
    stop_encoders();
}

int main()
{
    init();
    sei();
    run();
    cli();
    clean();

#ifdef JUNIOR_BOOTLOADER_AWARE
    JUNIOR_BOOTLOADER_SEQ();
#endif
}

