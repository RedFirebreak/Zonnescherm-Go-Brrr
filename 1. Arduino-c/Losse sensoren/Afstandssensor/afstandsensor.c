/*
 * Project.c
 *
 * Created: 11/8/2020 4:57:03 PM
 * Author : Datboi
 */ 

/* 
 * HC-SR04
 * trigger to sensor : uno 0 (PD0) output
 * echo from sensor  : uno 3 (PD3 = INT1) input
 * 
 * DIO : uno 8  (PB0) data
 * CLK : uno 9  (PB1) clock
 * STB : uno 10 (PB2) strobe
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16E6
#include <util/delay.h>
#include "distance.h"

volatile uint16_t gv_counter; // 16 bit counter value
volatile uint8_t gv_echo; // a flag

//********** start display ***********

void reset_display()
{
    // clear memory - all 16 addresses
    sendCommand(0x40); // set auto increment mode
    write(strobe, LOW);
    shiftOut(0xc0);   // set starting address to 0
    for(uint8_t i = 0; i < 16; i++)
    {
        shiftOut(0x00);
    }
    write(strobe, HIGH);
    sendCommand(0x89);  // activate and set brightness to medium
}

void show_distance(uint16_t cm)
{
                        /*0*/ /*1*/ /*2*/ /*3*/ /*4*/ /*5*/ /*6*/ /*7*/ /*8*/ /*9*/
    uint8_t digits[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};
    uint8_t ar[8] = {0};
    uint8_t digit = 0, i = 0;
    uint8_t temp, nrs, spaces;
    
    // cm=1234 -> ar[0..7] = {4,3,2,1,0,0,0,0}
    while (cm > 0 && i < 8) {
        digit = cm % 10;
        ar[i] = digit;
        cm = cm / 10;
        i++;
    }

    nrs = i;      // 4 digits
    spaces = 8-i; // 4 leading spaces  
    
    // invert array -> ar[0..7] = {0,0,0,0,1,2,3,4}
    uint8_t n = 7;
    for (i=0; i<4; i++) {
        temp = ar[i];
        ar[i] = ar[n];
        ar[n] = temp;
        n--;
    }
    
    write(strobe, LOW);
    shiftOut(0xc0); // set starting address = 0
    // leading spaces
    for (i=0; i<8; i++) {
        if (i < spaces) {
            shiftOut(0x00);
        } else {
            shiftOut(digits[ar[i]]);
        }           
        shiftOut(0x00); // the dot
    }
    
    write(strobe, HIGH);
}

void sendCommand(uint8_t value)
{
    write(strobe, LOW);
    shiftOut(value);
    write(strobe, HIGH);
}

// write value to pin
void write(uint8_t pin, uint8_t val)
{
    if (val == LOW) {
        PORTB &= ~(_BV(pin)); // clear bit
    } else {
        PORTB |= _BV(pin); // set bit
    }
}

// shift out value to data
void shiftOut (uint8_t val)
{
    uint8_t i;
    for (i = 0; i < 8; i++)  {
        write(clock, LOW);   // bit valid on rising edge
        write(data, val & 1 ? HIGH : LOW); // lsb first
        val = val >> 1;
        write(clock, HIGH);
    }
}

//********** end display ***********

void init_ports(void)
{
	DDRD = (1 << 0);
	DDRB = 0xff;
}

void init_timer(void)
// prescaling : max time = 2^16/16E6 = 4.1 ms, 4.1 >> 2.3, so no prescaling required
// normal mode, no prescale, stop timer
{
    TCCR1A = 0;
    TCCR1B = 0;
}

void init_ext_int(void)
{
    // any change triggers ext interrupt 1
    EICRA = (1 << ISC10);
    EIMSK = (1 << INT1);
}


uint16_t calc_cm(uint16_t counter)
{
	// counter 0 ... 65535, f = 16 MHz
	// prescaler = 1024 so multiply with 1024
	// devide by 16 because FCPU = 16MHz, buildin * 1000 * 1000 for uS
	// divide uS by 58 to convert to cm

	return ((uint16_t) (counter*1024/16)/58);
}

int main(void)
{
	uint16_t cm = 0;

	init_ports();
	reset_display();
	init_timer();
	init_ext_int();
	sei();

	_delay_ms(50);
	while(1) {
		gv_echo = BEGIN; // set flag for ISR
		// start trigger pulse lo -> hi (D0)
		PORTD |= _BV(0);
		_delay_us(12); // micro sec
		// stop trigger pulse hi -> lo (D0)
		PORTD = 0x00;
		// wait 30 milli sec, gv_counter == timer1 (read in ISR)
		_delay_ms(30);
		cm = calc_cm(gv_counter);
		show_distance(cm);
		_delay_ms(500); // wait 0.5 sec
	}
}

ISR (INT1_vect)
{
	if (gv_echo == BEGIN) {
		// set timer1 value to zero and start counting
		// C Compiler automatically handles 16-bit I/O read/write operations in the correct order
		TCNT1 = 0;
		TCCR1B |= (1<<CS10) | (0<<CS11) | (1<<CS12); // set prescaler to 1024
		// clear flag
		gv_echo = END;
		} else {
		// stop counting and read value timer1
		TCCR1B = 0;
		gv_counter = TCNT1;
	}
}

