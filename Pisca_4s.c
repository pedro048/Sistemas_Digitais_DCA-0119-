#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

volatile int cont = 0;
volatile int aux = 0;

ISR(TIMER0_OVF_vect){
  cont++;
  /*
   clk = 8Mhz;
   ciclo de maquina = 1/(8*10^(6)) = 1,25*10^(-7) s
   Estouro = TIMER0*prescale*(ciclo de maquina)
   Estouro = 256*256*(1,25*10^(-7))
   Estouro = 8.2 ms

   Temporizacao = 4s
   Temporizacao = Estouro*(valor de comparacao)
   4 = 8.2*10^(-3)*(valor de comparacao)
   (valor de comparacao) = 4/(8.2*10^(-3))
   (valor de comparacao) = 488
  */

  if(cont == 488){
      if(aux==0){
        aux=1;
      }else{
        aux=0;
      }
      cont = 0;
  }
}

int main(){
  
  // PD7 (LED) 
  DDRD |= 0b10000000;
  //configurando o temporizador com TIMER0 
  cli(); // desabilita as interrupcoes globais
  TCNT0 |= 0x00; // a contagem comeca em zero
  TCCR0A |= 0x00; // nao usa PWM
  TCCR0B |= _BV(CS02); // configura o TIMER0 com prescale de 256 (valor usado no calculo do estouro)
  TIMSK0 |= _BV(TOIE0); //habilita interrupcao por estouro de TIMER0
  sei(); // habilita as interrupcoes globais
  
  while(1){
      if(aux == 0){
        PORTD = 0b00000000;
      }else{
        PORTD = _BV(PORTD7);
      }
    }
}