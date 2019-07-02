#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

/*

Author: Pedro Victor Andrade Alves
computer enginner student
02/07/2019

	Sistema responsavel por monitorar
a temperatura de ambientes. No caso de 
incedio he acionada a vazao de agua e 
um alarme. Em todo momento o estado da
temperatura he mostrado ao usuario.

*/

/*	
					   ESQUEMATICO
	
				  ______________________
				 |					 PB3|------>LED_PWM (saida de agua)
botao_lig ------>|PD2				 	|
				 | 		Atmega328p   PD7|------>LED_verde (temperatura normal)
botao_desl------>|PD4				 PB0|------>LED_amarelo (temperatura um pouco elevada)
				 |					 PB4|------>LED_vermelho (temperatura alta)
sesor_Temp------>|PC0					|
				 |___________PB5________|
							  |
							  |
						    Buzzer (alarme)
*/

volatile int cont1 = 0; // variavel contadora que ajuda na temporização do estado amarelo
volatile int cont2 = 0; // variavel contadora que ajuda na temporização do estado vermelho
volatile int aux1 = 0;
volatile int aux2 = 0;
uint16_t valor_PWM = 0;
uint16_t valor_temp = 0;
int aux3 = 0; // ajuda no acionamentoe no desacionamento

ISR(TIMER0_OVF_vect){ /* rotina de interrupção por estouro de TIMER0 (overflow)
						 utilizada para fazer os espacamentos dos bips do buzzer (alarme)
					  */
	cont1++;
	cont2++;
	/*
	 clk = 8Mhz;
	 ciclo de maquina = 1/(8*10^(6)) = 1,25*10^(-7) s
	 Estouro = TIMER0*prescale*(ciclo de maquina)
	 Estouro = 256*256*(1,25*10^(-7))
	 Estouro = 8.2 ms
	 Temporizacao = 5
	 Temporizacao = Estouro*(valor de comparacao)
	 5= 8.2*10^(-3)*(valor de comparacao)
	 (valor de comparacao) = 5/(8.2*10^(-3))
	 (valor de comparacao) = 610
	*/
	
	if(cont1 == 610){ // 5s (bips mais espacados)
		if(aux1 == 0){
			aux1 = 1;
		}else{
			aux1 = 0;
        }
		cont1 = 0;	
	}
	
	/*
	 Temporizacao = 2
	 Temporizacao = Estouro*(valor de comparacao)
	 2= 8.2*10^(-3)*(valor de comparacao)
	 (valor de comparacao) = 2/(8.2*10^(-3))
	 (valor de comparacao) = 244
	*/
	
	if(cont2 == 244){ // 2s (bips mais constantes)
		if(aux2 == 0){
			aux2 = 1;
		}else{
			aux2 = 0;
        }
		cont2 = 0;	
	}
}

int main(){
	DDRD |= 0b10000000; /* saidas: PD7 (LED_verde)
						   entradas: PD2, PD4 (botoes: lig, desl)	
						*/
	DDRB |= 0b00111001; // saidas: PB0, PB3, PB4, PB5 (LED_amarelo, LED_PWM, LED_vermelho, buzzer)
	DDRC |= 0b00000000; // PC0 como entrada (sesor de temperatura)
	
	// O sistema inicializa desligado
	PORTD &= 0b00000000;
	PORTB &= 0b00000000;
	
	//configuracao do PWM
	//FAST PWM 10 bits (Modo 7) sem inversão: WGM10 = 1, WGM11 = 1, WGM12 = 1  
 
	TCCR2A |= _BV(COM2A1) | _BV(WGM20) | _BV(WGM21);
	TCCR2B |= _BV(CS21); //| _BV(WGM22); // clk_io/8 (prescale)
 
	// configuracao do ADC
	ADMUX   |= 0b01000000; //AREF com o valor do Vcc e usando o ADC0
	ADCSRA  |= 0b10000111; //ativa o ADEN (permite a conversao A/D) e define o fator de divisao como 128

  
	//CONTAR TEMPO COM O CONTADOR ZERO
	cli();                //Desabilita a interrupção global
	TCNT0 =  0x00;        //Inicia o timer0 em 0
	TCCR0B = 0x04;        //Configura o prescaler para 1:256
	TIMSK0 = 0x01;        //Habilita a interrupção por estouro do TMR0
	sei();                //Habilitar a interrupção global
	
	while(1){
		ADCSRA |= _BV(ADSC); 	/*O ADSC vai para 1 quando a conversao inicia e para 0 quando ela termina, 
								  levando o ADIF(interrupcao de fim de conversao) para 1
								*/
		while(!(ADCSRA & 0b00010000)); //enquanto ADIF nao for 1, a conversao ainda esta acontecendo
		valor_PWM = (255*ADC)/1023;
		valor_temp = ADC;
		
		if(PIND & 0b00000100){ // ligar
			aux3 = 1;
		}
		if(PIND & 0b00010000){ // desl
			aux3 = 0;
		}
		
		if(aux3 == 1){ // sistema em funcionamento
			OCR2A = valor_PWM; // saida de agua
			
			if(valor_temp >= 0 && valor_temp < 100){
				PORTD |= _BV(PORTD7); // acende o LED_verde
				PORTB &= (PORTB & 0b11111110); // desliga o LED_amarelo
				PORTB &= (PORTB & 0b11101111); // desliga o LED_vermelho
				PORTB &= (PORTB & 0b11011111); // desliga o buzzer
			}
			
			if(valor_temp >= 100 && valor_temp < 200){
				PORTD &= (PORTD & 0b01111111);   // desliga o LED_verde
				PORTB |= _BV(PORTB0);    		 // acende o LED_amarelo
				PORTB &= (PORTB & 0b11101111);   // desliga o LED_vermelho
				// bips de 5s 
				if(aux1 == 0){
					PORTB |= _BV(PORTB5); // liga o buzzer
				}else{
					PORTB &= (PORTB & 0b11011111); // desliga o buzzer
				}
			}
			
			if(valor_temp >= 200 && valor_temp < 1024){
				PORTD &= (PORTD & 0b01111111);   // desliga o LED_verde
				PORTB &= (PORTB & 0b11111110);   // desliga o LED_amarelo 
				PORTB |= _BV(PORTB4);            // acende o LED_vermelho
				// bips de 2s
				if(aux2 == 0){
					PORTB |= _BV(PORTB5); // liga o buzzer
				}else{
					PORTB &= (PORTB & 0b11011111); // desliga o buzzer
				}
			}
		}else{
			OCR2A = 0; 		  				 // sem vazao de agua
			PORTD &= (PORTD & 0b01111111);   // desliga o LED_verde
			PORTB &= (PORTB & 0b11111110);   // desliga o LED_amarelo 
			PORTB &= (PORTB & 0b11101111);   // desliga o LED_vermelho
			PORTB &= (PORTB & 0b11011111);   // desliga o buzzer
		}
	
	}
}