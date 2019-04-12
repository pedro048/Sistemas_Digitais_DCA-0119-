#include <avr/io.h>
#include <inttypes.h> //Defini��o de tipos



int main()
{

uint16_t valorAD=0;


//Configura��o do AD
	ADMUX  |= _BV(REFS0); //Utiliza VCC como refer�ncia 
	ADCSRA |= _BV(ADEN);  //Habilita o AD

	DDRB  |= 0b11111111;  //Pinos PDx como sa�da


while(1)
{
		ADCSRA |= _BV(ADSC); 
 		while(!(ADCSRA & 0x10)); 
		valorAD= ADC;

		float v = 10;

		float angulo = ADC * (v/1024);


		PORTB = valorAD;

}


}
