
#include <avr/io.h>
#include <inttypes.h>
#include <math.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#define BAUD   9600    //taxa de 9600 bps
#define MYUBRR  F_CPU/16/BAUD-1

volatile uint16_t tempo = 0; // Contador de segundos decorridos desde o início do programa
volatile uint8_t x = 0;
uint16_t valorADC = 0;

ISR(USART_RX_vect) {
	while (!(UCSR0A & (1 << RXC0))); //espera o dado ser recebido
	if (UDR0 == 's') {
		escreve_USART("valor da saida PWM: \0");
		escreve_USART(OCR1A);
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
	UBRR0H = (unsigned char)(ubrr0 >> 8); //Ajusta a taxa de transmissão
	UBRR0L = (unsigned char)ubrr0;

	UCSR0A = 0;//desabilitar velocidade dupla (no Arduino é habilitado por padrão)
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); //Habilita a transmissão e a recepção
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00) | (1 << RXC0); /*modo assíncrono, 8 bits de dados, 1 bit de parada, sem paridade*/
}

 ISR(TIMER0_OVF_vect){
	 	 
}

int main(void){
	USART_Inic(MYUBRR);
	OCR1A = 0;
	DDRD |= 0b11100000; //PD0 = liga; PD1 = desl; PD2 = autom; PB5 = vermelho; PB6 = Amarelo; PB7 = verde;
	DDRC |= 0b00000000; //usa o PC0/ADC0 como entrada para o potenciometro
	DDRB |= 0b00000010; // usar PB1/OC1A (PWM)
	
	//configuracao do PWM
	//FAST PWM 10 bits (Modo 7) sem inversão
	TCCR1A = _BV(COM1A1) | _BV(WGM10) | _BV(WGM11);
	TCCR1B = _BV(CS11) | _BV(WGM12);
	
	//usar para contagem de tempo TIMER0
	TCCR0A |= 0b00000000; 
	//TCCR0B |= 0b00000011;
	TCCR0B |= _BV(CS02) | _BV(CS00);
	TIMSK0 |= 0b00000001; // ativa o TOIE0 (interrupção por Overflow)
	sei(); // habilita interrupções
	
	//conversor A/D
	ADMUX |= _BV(REFS0); //Utiliza VCC como referência
	ADCSRA |= _BV(ADEN);  //Habilita o A/D
	
    while (1){
		// modo ligado
		if(PIND & 0b00000001){ 
			ADCSRA |= _BV(ADSC);
			while(!(ADCSRA & 0b00010000));
			OCR1A = ADC;
			
			/*
			valorADC = (ADC*150)/1023;
			
			if(valorADC >= 0 && valorADC < 50){
				PORTD |= _BV(PORTD7); // LED verde ligado
			}
			
			if(valorADC >= 50 && valorADC < 100){
				PORTD |= _BV(PORTD6); // LED amarelo ligado
			}
			
			if(valorADC >= 100 && valorADC < 150){
				PORTD |= _BV(PORTD5); // LED vermelho ligado
			}
			*/
		}
		
		// modo desligado
		if(PIND & 0b00000010){ 
			OCR1A = 0; // desliga o motor
		}
    }
}

