#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU 16000000UL
#define BAUD   9600    //taxa de 9600 bps
#define MYUBRR  F_CPU/16/BAUD-1

#define   echo      PORTD4    //bit para leitura do pulso de Echo
#define   trigger   PORTD3    //bit para gerar trigger para o sensor ultrassônico

#define   set_bit(reg,bit)    (reg |= (1<<bit))   //macro para setar um bit de determinado registrador
#define   clr_bit(reg,bit)    (reg &= ~(1<<bit))    //macro para limpar um bit de determinado registrador

uint16_t ldrValor = 0;
int counter = 0x00;
int cont = 0;
int aux = 0;

uint16_t pegaPulsoEcho();    //função para ler pulso de Echo
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

  // -- Configura Interrupção do Timer0 --
  //
  // T0_OVF = (256 - timer0) x prescaler x ciclo de máquina
  //        = (256 -    0  ) x    256    x      62,5E-9
  //        =~ 4 ms
  //
  // Para 60 ms: 4ms x 15
  
  counter++; 
  if(counter == 15){             
    HCSR04Trig();               //pulso de trigger
    counter = 0; 
  }
  
} //end ISR Timer0


//Comunicacao USART
ISR(USART_RX_vect) {
  while (!(UCSR0A & (1 << RXC0))); //espera o dado ser recebido
  if(UDR0 == 'a'){
      if(aux == 1){
        escreve_USART("acesso liberado\n\0");
        escreve_USART("\n\0");
      }else{
        escreve_USART("acesso bloqueado\n\0");
        escreve_USART("\n\0");
      }
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
  UBRR0H = (unsigned char)(ubrr0 >> 8); //Ajusta a taxa de transmissão
  UBRR0L = (unsigned char)ubrr0;

  UCSR0A = 0;//desabilitar velocidade dupla (no Arduino é habilitado por padrão)
  UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); //Habilita a transmissão e a recepção
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00) | (1 << RXC0); /*modo assíncrono, 8 bits de dados, 1 bit de parada, sem paridade*/
}

int main(void){
  USART_Inic(MYUBRR);
  DDRB |= _BV(PORTB3);  //coloca o PB3(OC2A) como saida (PWM)
  DDRC |= 0b00000000; //define o PC0(ADC0) como entrada (A\D)
  DDRD |= _BV(PORTD7) | _BV(trigger); //define PD7 como saida (servo) e PD3 (trigger) tambem
  PORTD |= 0b00000000; //o servo inicia parado

 //configuracao do PWM
 //FAST PWM 10 bits (Modo 7) sem inversão: WGM10 = 1, WGM11 = 1, WGM12 = 1  
 
 TCCR2A |= _BV(COM2A1) | _BV(WGM20) | _BV(WGM21);
 TCCR2B |= _BV(CS21); //| _BV(WGM22); // clk_io/8 (prescale)
 
 // configuracao do ADC
  ADMUX   |= 0b01000000; //AREF com o valor do Vcc e usando o ADC0
  ADCSRA  |= 0b10000111; //ativa o ADEN (permite a conversao A/D) e define o fator de divisao como 128

  
  //CONTAR TEMPO COM O CONTADOR ZERO
  cli();                //Desabilita a interrupção global
  TCNT0 =  0x00;            //Inicia o timer0 em 0
  TCCR0B = 0x04;            //Configura o prescaler para 1:256
  TIMSK0 = 0x01;            //Habilita a interrupção por estouro do TMR0
  sei();                //Habilitar a interrupção global 
  

  /*
   Queremos um período de 8s
   
   Troca de estado de PD0, 4s
   
   Ciclo de Máquina
   
   AVR 1/1600000
   
   Ciclo de máquina: 1/16Mhz = 62,5ns
   
   Estouro = timer0 x prescaler x ciclo de máquina = 4,08ms
   
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
    OCR2A = ldrValor;
    
    if(aux == 0){
      servo0graus();
    }
    if((PIND & 0b00000100) || (pegaPulsoEcho()<200)){ 
      servo180graus();    
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


uint16_t pegaPulsoEcho(){                //Função para captura do pulso de echo gerado pelo sensor de ultrassom

  uint32_t i, resultado;                //Variáveis locais auxiliares
  
  for(i=0;i<600000;i++)               //Laço for para aguardar o início da borda de subida do pulso de echo
  {
    if(!(PIND & (1<<echo)))             //Pulso continue em nível baixo?
    continue;                   //Aguarda
    else                                            //Pulso em nível alto?
    break;                      //Interrompe
  } //end for
  
  
  //Configuração do Timer2 para contar o tempo em que o pulso de echo permanecerá em nível lógico alto
  
  TCCR1A = 0x00;                    //Desabilita modos de comparação A e B, Desabilita PWM
  TCCR1B = (1<<CS11);                 //Configura Prescaler (F_CPU/8)
  TCNT1  = 0x00;                    //Inicia contagem em 0
  
  
  for(i=0;i<600000;i++)               //Laço for para aguardar que ocorra a borda de descida do pulso de echo
  {
    if(PIND & (1<<echo))              //Pulso continua em nível alto?
    {
      if(TCNT1 > 60000) break;          //Interrompe se TCNT2 atingir o limite da contagem
      else continue;                //Senão, continua
    }
    else
    break;                      //Interrompe quando encontrar a borda de descida
    
  } //end for
  
  resultado = TCNT1;                  //Salva o valor atual de TCNT2 na variável resultado (tempo que o echo ficou em high)
  
  TCCR1B = 0x00;                    //Interrompe Timer
  
  
  return (resultado>>1);                //Função retornará o tempo em microssegundos
  
  
} //end pegaPulsoEcho


void HCSR04Trig()                //gera pulso de Trigger
{
  
  set_bit(PORTD,trigger);
  _delay_us(10);
  clr_bit(PORTD,trigger);
  
} //end HCSR04Trig



