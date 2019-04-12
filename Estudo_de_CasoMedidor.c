#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#define CHAVE_ON 0b00000001

#define F_CPU 16000000
#define BAUDRATE 9600
#define BAUD_PRESCALLER F_CPU/16/BAUDRATE - 1

#define NUMERO_INT 2500// 1 segundo esta para 250000
#define TENSAOMAXIMA 230
#define CORRENTEMAXIMA 20
#define dt NUMERO_INT/250000

void intUSART();
void txByte(uint8_t info);
uint8_t  rxByte();
void USART_Flush(void);

volatile uint16_t contagemOver = 0;
volatile uint8_t tempoamostra = 0;

 ISR(TIMER0_OVF_vect)
{
  contagemOver++;
  if (contagemOver == NUMERO_INT){ // contagem de 0.1s, 100ms 
    tempoamostra = 1;
  }

  
}
  int main(void)
{
  unsigned char *chptr;
  
  uint16_t valor_tensao = 0;
  uint16_t valor_corrente = 0;

  float valor_tensaof = 0;
  float valor_correntef = 0;
  float pot_inst = 0;
  float pot = 0;

  intUSART();
  
  ADMUX   |= 0b01000000;
  ADCSRA  |= 0b10000111;
  
  TCCR0A |= 0b00000000;
  TCCR0B |= 0b00000011;
  TIMSK0 |= 0b00000001;
  
  
  while(1)
  {
    if (PINB & CHAVE_ON){
        sei();
        if(tempoamostra == 1)
        {
            ADMUX  &= 0b11110000;
            ADMUX  |= 0b00000010;
            ADCSRA |= 0b01000000;
            while(!(ADCSRA & 0b00010000));
            valor_tensao = ADC;
        
            ADMUX  &= 0b11110000;
            ADMUX  |= 0b00000100;
            ADCSRA |= 0b01000000;
            while(!(ADCSRA & 0b00010000));
            valor_corrente = ADC;
            
            tempoamostra = 0;
            
            // valor da tensão em valores reais
            valor_tensaof = valor_tensao*TENSAOMAXIMA/1023;
    
            // valor da corrente em valores reais
            valor_correntef = valor_corrente*CORRENTEMAXIMA/1023;
    
            //calculo da potencia instatanea
            pot_inst = valor_tensao * valor_corrente;
    
            // integrando por Euler
            pot = pot + dt*pot_inst;

            chptr = (unsigned char *) &pot;
            txByte(*chptr++);
            txByte(*chptr++);
            txByte(*chptr++);
            txByte(*chptr);
    
            
        }
        else 
          cli();
          pot = 0;
          pot_inst = 0;
          valor_correntef = 0;
          valor_tensaof = 0;
          valor_tensao = 0;
          valor_corrente = 0;
          TCNT0 = 0;
    }
  }
}
void intUSART()
{
  // Configurado para 38.4kbps
  UBRR0L = 25;
  UBRR0H = 0;

  // U2X=1 - Dobra a velocidade
  //UCSRA = (1<<U2X);

  // UCSZ2 = 0 - 8 bits
  UCSR0B |= _BV(RXEN0)|_BV(TXEN0);


  // UCSZ1 = 1 e UCSZ0 = 1 -> 8 bits
  // USBS0 = 0 -> 1 - stop bits
  // UPM0 = 0 e UMP0 = 0 -> sem bit de paridade
  UCSR0C |= _BV(UCSZ01)|_BV(UCSZ00);
}

void txByte(uint8_t info)
{
  // Transmissão de dados
  //Bit UDRE indica quando o buffer de tx pode enviar dados
  while(!(UCSR0A & (1<<UDRE0)));
  UDR0 = info;
}

uint8_t  rxByte()
{
  //Bit RXC sinaliza quando existem bytes não lidos no buffer
  while(!(UCSR0A & (1<<RXC0)));
  return UDR0;
}
void USART_Flush( void )
{
 unsigned char dummy;
 while ( UCSR0A & (1<<RXC0) ) dummy = UDR0;
}

