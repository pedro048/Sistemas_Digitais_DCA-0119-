#include <avr/io.h>
#include <math.h>
#include <avr/interrupt.h>
#include <inttypes.h>

volatile uint8_t x=0;
ISR(TIMER0_OVF_vect)
{
  x++;
  if(x < 61){
    PORTB |= 0b00100000; 
  }else if(x < 122){
    PORTB &= 0b11011111;
  }else 
    x = 0;
}


int main(void)
{

// valores iniciais dos sinais PWM
// configuracao do PWM
//FAST PWM 10 bits (Modo 7) sem inversão
TCCR1A = _BV(COM1A1) | _BV(WGM10) | _BV(WGM11);
TCCR1B = _BV(CS11) | _BV(WGM12);

// PB1/OC1A como saída
DDRB = 0b00100010; ;
//Valor do PWM OCR1A = 0 - 1023

ADMUX   |= 0b01000000;
ADCSRA  |= 0b10000111;

//CONTAR TEMPO COM O CONTADOR ZERO
TCCR0A |= 0b00000000;
TCCR0B |=_BV(CS02) | _BV(CS00);
TIMSK0 |= _BV(TOIE0);
sei();

while(1){
  ADCSRA |= 0b01000000;
  while(!(ADCSRA & 0b00010000));
  OCR1A = ADC; 
  
 }
}
