#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
  
#define ADPORTA 0
#define ALTURA_TANQUE 0.3

float pid(void);

float altura, erro, erro_dif, erro_int, setpoint, kp, ki, kd, dt;

int main (void) {
  
  kp=1.0;
  ki=0.2;
  kd=0.02;
  setpoint = ALTURA_TANQUE/2.0;
  dt = 0.1;
  erro = 0.0;
   
/**
* We will be using OCR1A as our PWM output which is the
* same pin as PB1.
*/
DDRB |= 0b00000010;
   

TCCR1A |= _BV(COM1A1) | _BV(WGM10);
TCCR1B |= _BV(CS10) | _BV(WGM12);
  
ADCSRA |= _BV(ADEN) | (1<<ADPS0) | (1<<ADPS1);;
  
ADMUX &= 0xf0;
ADMUX |= ADPORTA;
  
for(;;) {
    
  ADCSRA |= _BV(ADSC);
  while(!(ADCSRA & _BV(ADIF)));
    
    
    altura = ADC/315.0 * ALTURA_TANQUE;
      
    OCR1A = floor( pid()/ALTURA_TANQUE *250u);

   
_delay_ms(100);
}
   
}

float pid(){
  float erro_ant = erro;
  erro = setpoint - altura;
  erro_dif = (erro - erro_ant)/dt;
  erro_int += erro_ant*dt;
  float saida = erro*kp + erro_dif*kd + erro_int*ki;
  if(saida > ALTURA_TANQUE){
    saida = ALTURA_TANQUE;
  }else if(saida < 0.0){
    saida = 0.0;
  }
  return saida; 
}