#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

/*
Aluno: Pedro Victor Andrade Alves
Matricula: 20190001079
Curso: Engenharia da Computacao
Prova da 1º unidade
disciplina: Sistemas Digitais

SECADOR DE GRAOS
*/

/*


ESQUEMATICO:
			    T		 Um
				|		 |
			PC0/ADC0  PC1/ADC1
			-----------------
 L1	<---PD0	|				|
 L2	<---PD1	|				| PB1/OC1A ----> D1 (ventilador)
 L3	<---PD2	|	atmega328p	|
			|				| PB3/OC2A ----> D2 (aquecedor)
			|				|
			-----------------
					| PD5
					|  _
				Hiz	 _   _ terra
*/

/*
sensor de temperatura:
0 - 200ºC => T
0 - 1023  => ADC0/PC0

sensor de umidade:
0 - 100% => Um
0 - 1023 => ADc1/PC1

ventilador:
0 - 100 CFM => D1
0 - 1023	=> OC1A/PB1

aquecedor:
0 - 100 W => D2
0 - 1023  => OC2A/PB3

*/

volatile int cont = 0; // ajuda na contagem do tempo 
volatile uint16_t T = 0; // recebe o valor do sensor de temperatura
volatile uint16_t Um = 0; // recebe o valor do sensor de umidade
			
ISR(TIMER0_OVF_vect){ /* rotina de interrupcao de overflow que ajuda 
						 a verificar os sensores de temperatura e 
						 umidade a cada 16 ms
					  */
	cont++;
	/*
	 clk = 8Mhz;
	 ciclo de maquina = 1/(8*10^(6)) = 1,25*10^(-7) s
	 Estouro = TIMER0*prescale*(ciclo de maquina)
	 Estouro = 256*256*(1,25*10^(-7))
	 Estouro = 8.2 ms

	 Temporizacao = 16 ms (valor escolhido entre 1 e 20 ms)
	 Temporizacao = Estouro*(valor de comparacao)
	 16*10^(-3)= 8.2*10^(-3)*(valor de comparacao)
	 (valor de comparacao) = 16/(8.2)
	 (valor de comparacao) = 2
	
	*/
	if(cont == 2){ // verifica os sensores de temperatura e umidade a cada 16 ms
		T = lerSensor(0);
		Um = lerSensor(1);
		cont = 0;
	}
}

volatile uint16_t lerSensor(int id); // prototipo da funcao que seleciona o ADC a ser usado

int main(void){
	DDRB |= _BV(PORTB1) | _BV(PORTB3); /* coloca o PB1(OC1A) => D1(vel. fluxo de ar CFM) e
										  PB3(OC2A) => D2(fluxo de calor W) como saida
										  D1 = ventilador
										  D2 = aquecedor
									   */
	DDRC |= 0b00000000; 			   /* serao usados o PC0(ADC0) => sensor de temperatura e o 
										  PC1(ADC1) => sensor de umidade (entradas)	
									   */
	DDRD |= _BV(PORTD2) | _BV(PORTD1) | _BV(PORTD0); /* PD5(CH1) => entrada
														PD2(L3) => saida
														PD1(L2) => saida
														PD0(L1) => saida
													 */	
	PORTD |= 0b00000000; // PD5(CH1) como Hiz e os LEDs comecam desligados
	
	//configurando PWM
	//PWM do D1 
	TCCR1A |= _BV(COM1A1) | _BV(WGM11) | _BV(WGM10); 
	TCCR1B |= _BV(WGM12) | _BV(CS12); 				 /* o modo definido foi o fast PWM 7
														
														a frequencia de D1 deveria estar entre 100hz e 100Khz
													    e com: CS12 = 1, CS11 = 0, CS10 = 0 foi definida como 
														31.25 KHz
													 */
    //PWM do D2		
	TCCR2A |= _BV(COM2A1) | _BV(WGM21) | _BV(WGM20); 
	TCCR2B |= _BV(CS22);							 /* o modo definido foi o fast PWM 3
	
														a frequencia de D2 deveria estar entre 100hz e 100Khz
													    e com: CS22 = 1, CS21 = 0, CS20 = 0 foi definida como 
														31.25 KHz
													 */	
	//configurando o temporizador com TIMER0 
	cli(); // desabilita as interrupcoes globais
	TCNT0 |= 0x00; // a contagem comeca em zero
	TCCR0A |= 0x00; // nao usa PWM
	TCCR0B |= _BV(CS02); // configura o TIMER0 com prescale de 256 (valor usado no calculo do estouro)
	TIMSK0 |= _BV(TOIE0); //habilita interrupcao por estouro de TIMER0
	sei(); // habilita as interrupcoes globais
	
	

	while(1){
		
		if(PIND & 0b00100000){ /* CH1 he retentiva
					
								  verifica se a CH1 foi acionada e liga o sistema de secagem 
								  se a temperatura lida pelo sensor for maior ou igual a 20 e																
								  menor ou igual a 120, ligando o LED L3. No caso de nao obedecer 
								  esse criterio o sistema nao liga e os LEDs L1 e L2 sao acionados
								  nos, respectivos, casos: temperatura menor que vinte e temperatura 
								  maior que cento e viente. Os LEDs L1, L2, L3 sao apagados quando a
								  a CH1 he desligada. CH1 pode ser desligada a qualquer momento, sendo  
								  que apos o fim da secagem, obrigatoriamente, o sistema deve parar e 
								  esperar CH1 ser religada 
								*/
			
			if(T < 102){ // temperatura < 20ºC
				PORTD |= _BV(PORTD0); // liga o LED L1
				// o sistema de secagem nao comeca a funcionar
				OCR1A |= 0; // D1(ventilador) desligado 
				OCR2A |= 0; // D2(aquecedor) desligado
			}
			
			if(T > 614){ // temperatura > 120ºC
				PORTD |= _BV(PORTD1); // liga o LED L2
				// o sistema de secagem nao comeca a funcionar
				OCR1A |= 0; // D1(ventilador) desligado 
				OCR2A |= 0; // D2(aquecedor) desligado
			}
			
			if(T>= 102 && T<= 614){ /* o sistema de secagem comeca a funcionar
									   quando a temperatura for igual ou estiver 
									   entre 20ºC e 120ºC
									*/
				PORTD |= _BV(PORTD2); // liga o LED L3
				if(Um == 1023){
					OCR1A &= 256;  // valor aplicado ao ventilador D1
					OCR2A &= 1023; // valor aplicado ao aquecedor D2
				}
				if(Um >= 512 && Um < 1023){
					OCR1A &= 512;	// valor aplicado ao ventilador D1
					OCR2A &= 512;	// valor aplicado ao aquecedor D2
				}
				if(Um >= 256 && Um < 512){
					OCR1A &= 1023;	// valor aplicado ao ventilador D1
					OCR2A &= 256;	// valor aplicado ao aquecedor D2
				}
				//FIM DA SECAGEM
				if(Um >= 0 && Um < 256){ /* com esse valor de umidade os 
										    graos ficam totalmente secos
											(secagem termina) 
										 */
					OCR1A &= 0; // desliga o vetilador D1
					OCR2A &= 0; // desliga o aquecedor D2
					continue; // secagem termina e he preciso esperar o acionamento de CH1
				}
			}		
		}else{
			PORTD &= 0b00000000; // desliga L1, L2, L3 (CH1 desligada)
		}	
	}
}

volatile uint16_t lerSensor(int id){
	switch(id){
		case 0:
			ADMUX &= _BV(REFS0); // AREF = Vcc e usa o ADC0(sensor de temperatura) 
		case 1:
			ADMUX &= _BV(REFS0) | _BV(MUX0); // AREF = Vcc e usa o ADC1(sensor de umidade) 
		default:
	}
	ADCSRA |= _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); /* habilita a possibilidade de ocorrer conversoes AD(ADEN)
																   define o fator de divisao como 128 => 8MHz/128 = 62.5Khz
																 */
	ADCSRA |= _BV(ADSC); // inicia a conversao AD
	while(!(ADCSRA & 0b00010000)); // a conversao so termina quando ADIF vai para 1
	return ADC; // retorna o valor da conversao, pode ser relacionado a temperatura ou umidade (usando ADC0 => temperatura ou ADC1 => umidade)
}

