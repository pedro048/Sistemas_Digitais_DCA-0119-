#include <avr/io.h>
#include <inttypes.h> //Defini��o de tipos



int main()
{
    // valores iniciais dos sinais PWM
    OCR1A = 0;
    
    // configuracao do PWM
    //FAST PWM 10 bits (Modo 7) sem invers�o
    TCCR1A = _BV(COM1A1) | _BV(WGM10) | _BV(WGM11);
    TCCR1B = _BV(CS11) | _BV(WGM12);
    
    // PB1/OC1A como sa�da
    DDRB = _BV(PORTB1);
    
    // valor inicial da vari�vel auxiliar
    uint16_t valorAD=0;
    
    //Configura��o do AD
    ADMUX  |= _BV(REFS0); //Utiliza VCC como refer�ncia 
    ADCSRA |= _BV(ADEN);  //Habilita o AD
    
    DDRB  &= 0b11111110;  //Pino PB0 como entrada
    
    
    while(1)
    {
      if (PINB & 0x01)
      {
          ADCSRA |= _BV(ADSC); 
          while(!(ADCSRA & 0x10)); 
          valorAD = ADC;
          OCR1A = valorAD;
      }
    }
}