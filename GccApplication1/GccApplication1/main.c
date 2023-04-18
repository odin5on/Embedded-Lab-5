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

#define __DELAY_BACKWARD_COMPATIBLE__

#include <avr/io.h>
#include <util/delay.h>
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

void write_voltage(uint8_t address, uint8_t channel, uint8_t value){
	i2c_start(address);
	i2c_write(channel);
	i2c_write(value);
	i2c_stop();
}

const int sine[64] = {128, 141, 153, 165, 177, 188, 199, 209, 219, 227, 234, 241, 246, 250 ,254 ,255 ,255 ,255 ,254 ,250 ,246 ,241 ,234, 227, 219, 209, 199, 188, 177, 165, 153, 141, 128, 115, 103, 91, 79, 68, 57, 47, 37, 29, 22, 15, 10, 6, 2 ,1 ,0, 1, 2 ,6 ,10 ,15, 22, 29, 37, 47 ,57 ,68 ,79, 91, 103, 115};

int main(void)
{

	stdout = stdin = &uart_io;
		
	USART_Init(MYUBRR);
	
	ADMUX |= 0b01 << REFS0;
	ADCSRA = 1 << ADEN | 0b111 << ADPS0;
	
	i2c_init();
	
	uint8_t addr = 0b01011000;
	
	char c[20];
	char command;
	int channel, numofwaves = 0;
	float third = 0;
    while (1) 
    {
		scanf("%19s", c);
		sscanf(c, "%c,%d,%f,%d", &command, &channel, &third, &numofwaves);
		printf("%c , %d , %f , %d\n", command, channel, third, numofwaves);
		if(command == 'G'){
			printf("v=%.3f\n", read_adc());
		}
		else if(command == 'S'){
			int a = convert_voltage(third);
			write_voltage(addr, channel, a);
			printf("DAC channel %d set to %.2f V (%dd)\n",channel,third,a);
		}
		else if(command == 'W'){
			printf("Generating %d sine wave cycles with f=%.0f Hz on DAC channel %d\n", numofwaves, third, channel);
			double delay = 1000.0/(72.0*third);
			for(int i=0; i<numofwaves; i++){
				for(int j=0; j<64; j++){
					write_voltage(addr, channel, sine[j]);
					_delay_ms(delay);
				}
			}
		}
    }
}


