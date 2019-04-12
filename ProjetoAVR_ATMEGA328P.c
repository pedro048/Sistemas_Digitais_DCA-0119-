#include <avr/io.h>
#include <inttypes.h> //Definição de tipos



int main()
{

uint16_t valorAD=0;


//Configuração do AD
	ADMUX  |= _BV(REFS0); //Utiliza VCC como referência 
	ADCSRA |= _BV(ADEN);  //Habilita o AD

	DDRB  |= 0b11111111;  //Pinos PDx como saída


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
