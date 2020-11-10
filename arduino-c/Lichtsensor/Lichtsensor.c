/*
 * temperatuurTest.c
 *
 * Created: 8-11-2020 22:48:53
 * Author : Stefan
 */ 

#include <avr/io.h>
#include <string.h>
#include <avr/sfr_defs.h>
#define F_CPU 16E6
#include <util/delay.h>
#define UBBRVAL 51

// Function prototypes
void transmit(uint8_t data);
void send_adc_info();
void init_adc();
uint8_t get_adc_value();
void uart_init();

void init_adc() {
    // ref=Vcc, left adjust the result (8-bit resolution),
    // select channel 0 (PC0 = input)
    ADMUX = (1<<REFS0)|(1<<ADLAR);
    // enable the ADC & prescale= 128
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

uint8_t get_adc_value() {
	ADCSRA |= (1 << ADSC); // start conversion
	loop_until_bit_is_clear(ADCSRA, ADSC);
	// Save a value between 0 and 255 to sensorData.
	return ADCH;
}

void uart_init() {
    // set the baud rate
    UBRR0H = 0;
    UBRR0L = UBBRVAL;
    // disable U2X mode
    UCSR0A = 0;
    // enable transmitter and receiver
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
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

void send_adc_info() {
    transmit(get_adc_value());
}

int main(void)
{
    init_adc();
    uart_init();
    
    while (1) 
    {
        send_adc_info();
        _delay_ms(1000); // 1 sec
    }
}