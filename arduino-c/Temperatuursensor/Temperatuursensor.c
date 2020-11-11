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
void uart_init();
void init_adc();
uint8_t get_adc_value(uint8_t pin);

void init_adc() {
	// Source: https://medium.com/@jrejaud/arduino-to-avr-c-reference-guide-7d113b4309f7
	// 16Mhz / 128 = 125kHz ADC reference clock
	ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));
	
	// Voltage reference from AVcc (5V on ATMega328p)
	ADMUX |= (1<<REFS0);
	
	ADCSRA |= (1<<ADEN);    // Turn on ADC
	ADCSRA |= (1<<ADSC);    // Do a preliminary conversion
}

uint8_t get_adc_value(uint8_t pin) {
	// Source: https://medium.com/@jrejaud/arduino-to-avr-c-reference-guide-7d113b4309f7
	ADMUX &= 0xF0;    // Clear previously read channel
	ADMUX |= pin;    // Define new ADC Channel to read, analog pins 0 to 5 on ATMega328p
	
	ADCSRA |= (1<<ADSC);    // New Conversion
	ADCSRA |= (1<<ADSC);    // Do a preliminary conversion
	
	// Wait until conversion is finished
	while(ADCSRA & (1<<ADSC));
	
	// Return ADC value
	return ADCW;
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
    transmit(get_adc_value(0));
}

float get_temperatuur() {
	return 0.48828125*get_adc_value(0)-50;
}

void send_temperature_info() {
	transmit((int8_t)get_temperatuur());
}

int main(void)
{
    init_adc();
    uart_init();
    
    while (1) 
    {
        send_temperature_info();
        _delay_ms(1000); // 1 sec
    }
}