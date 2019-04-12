#include <avr/io.h>
#include <inttypes.h> //Definição de tipos



int main()
{
    // valores iniciais dos sinais PWM
    OCR1A = 0;
    
    // configuracao do PWM
    //FAST PWM 10 bits (Modo 7) sem inversão
    TCCR1A = _BV(COM1A1) | _BV(WGM10) | _BV(WGM11);
    TCCR1B = _BV(CS11) | _BV(WGM12);
    
    // PB1/OC1A como saída
    DDRB = _BV(PORTB1);
    
    // valor inicial da variável auxiliar
    uint16_t valorAD=0;
    
    //Configuração do AD
    ADMUX  |= _BV(REFS0); //Utiliza VCC como referência 
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