#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU 16000000UL
#define BAUD   9600    //taxa de 9600 bps
#define MYUBRR  F_CPU/16/BAUD-1

#define   echo      PORTD4    //bit para leitura do pulso de Echo
#define   trigger   PORTD3    //bit para gerar trigger para o sensor ultrass�nico

uint16_t ldrValor = 0;
int counter = 0x00;
int cont = 0;
int aux = 0;

uint16_t pegaPulsoEcho();    //fun��o para ler pulso de Echo
void HCSR04Trig();
void servo0graus();
void servo90graus();
void servo180graus();

ISR(TIMER0_OVF_vect){
  cont++;
  if(cont == 980){
    if(aux == 1){
      aux = 0;
    }
    cont = 0;
  }
  
  counter++; 
  if(counter == 15){             
    HCSR04Trig();               //pulso de trigger
    counter = 0; 
  }
  
} //end ISR Timer0

//Comunicacao USART
ISR(USART_RX_vect) {
  while (!(UCSR0A & (1 << RXC0))); //espera o dado ser recebido
  if(cont == 1){
      escreve_USART("acesso liberado\n\0");
      escreve_USART("\n\0");
  }else{
      escreve_USART("acesso bloqueado\n\0");
      escreve_USART("\n\0");
  }
}

void USART_Transmite(unsigned char dado) {
  while (!( UCSR0A & (1 << UDRE0)) ); //espera o dado ser enviado
  UDR0 = dado;          //envia o dado
}

void escreve_USART(char *c) {   //escreve String (RAM)
  for (; *c != 0; c++) USART_Transmite(*c);
}

unsigned char USART_Recebe() {
  while (!(UCSR0A & (1 << RXC0))); //espera o dado ser recebido
  return UDR0;        //retorna o dado recebido
}

void USART_Inic(unsigned int ubrr0) {
  UBRR0H = (unsigned char)(ubrr0 >> 8); //Ajusta a taxa de transmiss�o
  UBRR0L = (unsigned char)ubrr0;

  UCSR0A = 0;//desabilitar velocidade dupla (no Arduino � habilitado por padr�o)
  UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); //Habilita a transmiss�o e a recep��o
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00) | (1 << RXC0); /*modo ass�ncrono, 8 bits de dados, 1 bit de parada, sem paridade*/
}

int main(void){
  USART_Inic(MYUBRR);
  DDRB |= _BV(PORTB1);  //coloca o PB1(OC1A) como saida (PWM)
  DDRC |= 0b00000000; //define o PC0(ADC0) como entrada (A\D)
  DDRD |= _BV(PORTD7) | _BV(trigger); //define PD7 como saida (servo) e PD3 (trigger) tambem
  PORTD |= 0b00000000; //o servo inicia parado

 // configuracao do PWM
 //FAST PWM 10 bits (Modo 7) sem invers�o: WGM10 = 1, WGM11 = 1, WGM12 = 1  
  TCCR1A |= _BV(COM1A1) | _BV(WGM10) | _BV(WGM11);
  TCCR1B |= _BV(CS11) | _BV(WGM12); // clk_io/8 (prescale)

 // configuracao do ADC
  ADMUX   |= 0b01000000; //AREF com o valor do Vcc e usando o ADC0
  ADCSRA  |= 0b10000111; //ativa o ADEN (permite a conversao A/D) e define o fator de divisao como 128
  
  
  //CONTAR TEMPO COM O CONTADOR ZERO
  cli();                //Desabilita a interrup��o global
  TCNT0 =  0x00;            //Inicia o timer0 em 0
  TCCR0B = 0x04;            //Configura o prescaler para 1:256
  TIMSK0 = 0x01;            //Habilita a interrup��o por estouro do TMR0
  sei();                //Habilitar a interrup��o global 

  /*
   Queremos um per�odo de 8s
   
   Troca de estado de PD0, 4s
   
   Ciclo de M�quina
   
   AVR 1/1600000
   
   Ciclo de m�quina: 1/16Mhz = 62,5ns
   
   Estouro = timer0 x prescaler x ciclo de m�quina = 4,08ms
   
   Troca de estado = Estouro x Counter
   
   4s = 4,08ms x counter
   
   counter = 4/(0.00408) = 980 

*/
  
  while(1){
    ADCSRA |= _BV(ADSC); /*o ADSC vai para 1 quando a conversao inicia e para 0 quando ela termina, 
                          levando o ADIF(interrupcao de fim de conversao) para 1
                          */
    while(!(ADCSRA & 0b00010000)); //enquanto ADIF nao for 1, a conversao ainda esta acontecendo
    ldrValor = (255*ADC)/1023;
    OCR1A = ldrValor;
    
    if(aux == 0){
      servo0graus();
    }
    if(PIND & 0b00000100){ // considerando uma distancia de 10 cm (10*58=580)
      servo180graus();    //move o servo para a posi��o 90� por 2 segundos
      aux = 1; 
    } 
  }
}

void servo0graus(){         //Posiciona o servo em 0 graus
  PORTD = _BV(PORTD7);  //pulso do servo
  _delay_ms(1.0);      //0.6ms
  PORTD = 0b00000000;   //completa periodo do servo
  for(int i=0;i<32;i++) _delay_ms(0.6);
  // 20ms = 20000us
  // 20000us - 600us = 19400us
  // 19400/600 = ~~32
} //end servo0graus

void servo90graus(){            //Posiciona o servo em 90 graus
  PORTD = _BV(PORTD7);  //pulso do servo
  _delay_ms(1.5);     //1.5ms
  PORTD = 0b00000000;   //completa periodo do servo
  for(int i=0;i<12;i++) _delay_ms(1.5);
  // 20ms = 20000us
  // 20000us - 1500us = 18500us
  // 18500/1500 = ~~12
} //end servo90graus

void servo180graus(){             //Posiciona o servo em 180 graus
  PORTD = _BV(PORTD7);  //pulso do servo
  _delay_ms(2.4);     //2.4ms
  PORTD = 0b00000000;   //completa periodo do servo
  for(int i=0;i<7;i++) _delay_ms(2.4);
  // 20ms = 20000us
  // 20000us - 2400us = 17600us
  // 17600/2400 = ~~7
} //end servo180graus


uint16_t pegaPulsoEcho(){                //Fun��o para captura do pulso de echo gerado pelo sensor de ultrassom

  uint32_t i, resultado;                //Vari�veis locais auxiliares
  
  for(i=0;i<600000;i++)               //La�o for para aguardar o in�cio da borda de subida do pulso de echo
  {
    if(!(PIND & (1<<echo)))             //Pulso continue em n�vel baixo?
    continue;                   //Aguarda
    else                                            //Pulso em n�vel alto?
    break;                      //Interrompe
  } //end for
  
  
  //Configura��o do Timer2 para contar o tempo em que o pulso de echo permanecer� em n�vel l�gico alto
  
  TCCR2A = 0x00;                    //Desabilita modos de compara��o A e B, Desabilita PWM
  TCCR2B = (1<<CS11);                 //Configura Prescaler (F_CPU/8)
  TCNT2  = 0x00;                    //Inicia contagem em 0
  
  
  for(i=0;i<600000;i++)               //La�o for para aguardar que ocorra a borda de descida do pulso de echo
  {
    if(PIND & (1<<echo))              //Pulso continua em n�vel alto?
    {
      if(TCNT2 > 60000) break;          //Interrompe se TCNT2 atingir o limite da contagem
      else continue;                //Sen�o, continua
    }
    else
    break;                      //Interrompe quando encontrar a borda de descida
    
  } //end for
  
  resultado = TCNT2;                  //Salva o valor atual de TCNT2 na vari�vel resultado (tempo que o echo ficou em high)
  
  TCCR2B = 0x00;                    //Interrompe Timer
  
  
  return (resultado>>1);                //Fun��o retornar� o tempo em microssegundos
  
  
} //end pegaPulsoEcho


void HCSR04Trig()                //gera pulso de Trigger
{
  
  PORTD |= _BV(trigger);
  _delay_us(10);
  PORTD |= 0b00000000;
  
} //end HCSR04Trig

