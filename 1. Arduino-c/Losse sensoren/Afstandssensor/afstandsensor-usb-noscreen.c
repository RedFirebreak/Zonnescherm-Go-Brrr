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
#define UBBRVAL 51

volatile uint16_t gv_counter; // 16 bit counter value
volatile uint8_t gv_echo; // a flag


//********** start usb ***********
void uart_init() {
	// set the baud rate
	UBRR0H = 0;
	UBRR0L = UBBRVAL;
	// disable U2X mode
	UCSR0A = 0;
	
	// enable transmitter and receiver
	// WERKT NIET || UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
	// WERKT NIET || UCSR0B = 0x18;
	UCSR0B = _BV(TXEN0);
	
	// set frame format : asynchronous, 8 data bits, 1 stop bit, no parity
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
}

void transmit(uint8_t data) {
	// wait for an empty transmit buffer
	// UDRE is set when the transmit buffer is empty
	loop_until_bit_is_set(UCSR0A,UDRE0);
	// send the data
	UDR0 = data;
}

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
	uart_init();
	init_ports();
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
		_delay_ms(500); // wait 0.5 sec
		transmit(cm);
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

