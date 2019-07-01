#include <avr/io.h>
#include <inttypes.h>

int main(){
  int cont = 0;
  DDRD |= 0b10000000;
  
  while(1){
    
    if((PIND & 0b00000100) && cont == 0){
      PORTD = _BV(PORTD7);
      cont = 1;
    }
    if((PIND & 0b00000100) && cont == 1){
      PORTD = 0b00000000;
      cont = 0;
    }
  }
}