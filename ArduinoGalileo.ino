#include <avr/io.h>
#include <inttypes.h>

int main(void){
  DDRD |= 0b00000000; // PD6 (6) - verde_in, PD7 (7) - amarelo_in, 
  DDRB |= 0b00111100; // PB0 (8) - vermelho_in, PB1 (9) - buzzer_in, PB2 (10) - verde_out, PB3 (11) - amarelo_out, PB4 (12) - vermelho_out, PB5 (13) - buzzer_out
  
  while(1){

    if(PIND & 0b01000000){ // verde
      PORTB |= _BV(PORTB2); 
    }
    if(PIND & 0b10000000){ // amarelo
      PORTB |= _BV(PORTB3); 
    }
    if(PINB & 0b00000001){ // vermelho
      PORTB |= _BV(PORTB4); 
    }
    if(PINB & 0b00000010){ // Buzzer
      PORTB |= _BV(PORTB5); 
    }
  }
}
