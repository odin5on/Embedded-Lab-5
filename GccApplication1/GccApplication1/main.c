/*
 * GccApplication1.c
 *
 * Created: 4/10/2023 12:28:02 PM
 * Author : dbodn
 */ 

#ifndef F_CPU
#define F_CPU 16000000UL // 16 MHz clock
#endif

#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1


#include <avr/io.h>
#include <stdio.h>
#include "i2cmaster.h"
#include "twimaster.c"


void USART_Init(unsigned int ubrr){
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	/*Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

void uart_putchar( unsigned char data, FILE* uart)
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) );
	/* Put data into buffer, sends the data */
	UDR0 = data;
}

unsigned char uart_getchar( FILE* uart)
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) )
	;
	/* Get and return received data from buffer */
	return UDR0;
}

static float read_adc(){
	ADMUX = (ADMUX & 0xf0) | 0x00;
	ADCSRA |= 1 << ADSC;
	
	while(!(ADCSRA & (1 << ADIF)));
	ADCSRA |= 1 << ADIF;
	
	
	return 5.0f * (ADCL + (ADCH<<8)) / 1023.0f;
}

static FILE uart_io = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

unsigned char convert_voltage(float v){
	uint8_t val = v*51;
	return val;
}

int main(void)
{

	stdout = stdin = &uart_io;
		
	USART_Init(MYUBRR);
	
	ADMUX |= 0b01 << REFS0;
	ADCSRA = 1 << ADEN | 0b111 << ADPS0;
	
	i2c_init();
	
	unsigned char channel0 = 00000000;
	unsigned char channel1 = 00000001;
	unsigned char addr = 01011000;
	unsigned char volt = 10000001;
	
	char c[20];
	char command;
	int channel, waveforms = 0;
	float third = 0;
    while (1) 
    {
		scanf("%19s", c);
		sscanf(c, "%c,%d,%f,%d", &command, &channel, &third, &waveforms);
		printf("%c , %d , %f , %d\n", command, channel, third, waveforms);
		if(command == 'G'){
			printf("v=%.3f\n", read_adc());
		}
		else if(command == 'S'){
			printf("%d\n",convert_voltage(third));
			int a = i2c_start(addr);
			printf("start success?: %d\n", a);
			if(channel == 1){
				a = i2c_write(channel1);
				printf("command write success?: %d\n", a);
			} else {
				a = i2c_write(channel0);
				printf("command write success?: %d\n", a);
			}
			a = i2c_write(0xFF);
			printf("write voltage success?:%d\n", a);
			i2c_stop();
			printf("after stop\n");
		}
		else if(command == 'W'){
		
		}
    }
}


