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
	
	//PRADC = 0x00;
	//ADSC = 0x01;
	
}
static FILE uart_io = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

int main(void)
{

	stdout = stdin = &uart_io;
		
	USART_Init(MYUBRR);
	
	ADMUX |= 0b01 << REFS0;
	ADCSRA = 1 << ADEN | 0b111 << ADPS0;
	
	//UCSR0A = 1 << U2X0;
	//UBRR0L = (F_CPU / (8UL * BAUD)) - 1;
	//UCSR0B = 1 << TXEN0 
	
	//unsigned char tmp;
	//
	//DDRC = 0x00;
	//
	//tmp = PINC
	//tmp = tmp & (1 << 0)
	//
	//REFS1 = 0;
	//REFS0 = 1;
	
	char c;
    while (1) 
    {
		c = "";
		c = scanf("%c", &c);
		if(strcmp(c, "G")){
			printf("v=%.3f\n", read_adc());
		}
    }
}


