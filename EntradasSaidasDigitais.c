#include <inttypes.h> //Definição de tipos
#include <avr/io.h>

unsigned char x;

int main()
{


	DDRB  &= 0b1111100;  //Pinos PB0 e PB1 como entradas
	PORTB &= 0b1111110;  //Pino PB0 com Hi-Z
	PORTB |= 0b0000010;  //Pino PB1 com pull-up

	
	DDRC  |= 0b0001111;  //Pinos PC0,PC1, PC2 e PC3 como saída

	
	while (1)
	{
	
	//	PORTC = ((PINB & 0b00000001)<<0) | (PORTC & 0b11111110); //Pino PC0 utiliza o estado do pino PB0
		PORTC = (PINB & 1) | (PORTC & ~1); //Pino PC0 utiliza o estado do pino PB0
	//	PORTC = ((PINB & 0b00000001)<<1) | (PORTC & 0b11111101); //Pino PC1 utiliza o estado do pino PB0
		PORTC = ((PINB & 1)<<1) | (PORTC & ~(1<<1)); //Pino PC1 utiliza o estado do pino PB0

	//	PORTC = ((PINB & 0b00000010)<<1) | (PORTC & 0b11111011); //Pino PC2 utiliza o estado do pino PB1
		PORTC = ((PINB & 1<<1)<<1) | (PORTC & ~(1<<2)); //Pino PC2 utiliza o estado do pino PB1
	//	PORTC = ((PINB & 0b00000010)<<2) | (PORTC & 0b11110111); //Pino PC3 utiliza o estado do pino PB1
		PORTC = ((PINB & 1<<1)<<2) | (PORTC & ~(1<<3)); //Pino PC3 utiliza o estado do pino PB1
	}



}
