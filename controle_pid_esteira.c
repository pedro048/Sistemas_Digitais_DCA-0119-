#include <avr\io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

/*
 Projeto para controlar velocidade de um motor
 com potenciometro
*/
#define NUMERO_INT 2500// 1 segundo esta para 250000
#define dt NUMERO_INT/250000

#define VEL_MAX_W 500 // valor maximo de velocidade angular
#define SETPOINT 2000 // valor em rpm

float rpm=2.0, erro=0.0, erro_dif=0.0, erro_int=0.0, kp, ki, kd, dt;

volatile uint16_t fim_contagem = 0;
volatile uint8_t tempoamostra = 0;

ISR(TIMER0_OVF_vect){
	 fim_contagem++;
	 if (fim_contagem == NUMERO_INT){ // contagem de 0.1s, 100ms
		 tempoamostra = 1;
		 fim_contagem = 0;
	 }
}

float pid(float rpm){
	
	erro = SETPOINT - rpm;
	erro_dif = (erro - erro_ant)/dt;
	erro_int += erro_ant*dt;
	float saida = erro*kp + erro_dif*kd + erro_int*ki;
	
	float erro_ant = erro;
	
	if(saida > VEL_MAX_W){
		saida = VEL_MAX_W;
		}else if(saida < 0.0){
		saida = 0.0;
	}
	return saida;
}

double eqDiferencialCorrente(double W, double Wkw){
	//Wkw he o produto da velocidade angular(saida do pid) com uma constante kw
	float Ra = 0.371;
	float La = (0.161)*10^(-3);
	float Va = 5.0;
	
	float kw = 0.116;
	
	return  (W - (Ia * Ra + Wkw + Va))/La
	// Retorna uma corrente
	
}

double eqDiferencialVelocidade(double Iaki, double Tc, double W){
	// Iaki he o produto da saida da 1 EDO com a constante ki
	float b = (0.0214)*10^(-3);
	float Tae = 48*10^(-3);
	float J = 1460*10^(-3);
	
	float ki = 0.116;
	
	// Tc he o torque de carga um valor que vem do ADC (potenciometro)
	
	return  (Iaki - (W * b + Tc + Tae))/J;
	//retorna uma velocidade angular que se tornara rpm
}



float vel_rpm(float vel_W){
	return vel_W*60;
}


int main(void){
	//uint16_t valorAD=0;
	//uint16_t vel_motor=0;
	
	kp = 1.0;
	ki = 0.2;
	kd = 0.02;
	dt = 0.1;
	
	// valores iniciais dos sinais PWM
	OCR1A = 0; //Valor do PWM OCR1A = 0 - 1023
	// configuracao do PWM
	//FAST PWM 10 bits (Modo 7) sem inversão
	TCCR1A = _BV(COM1A1) | _BV(WGM10) | _BV(WGM11);
	TCCR1B = _BV(CS11) | _BV(WGM12);
	DDRC |= 0b00000000; // PC0 como entrada (ADC0)
	DDRB = _BV(PORTB1); // PB1 como siada (OC1A) PWM
	ADMUX |= _BV(REFS0); //AREF = Vcc, usando ADC0 = PC0
	ADCSRA |= _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0) ;  //Habilita o AD e configura o fator de divisao como 128
	
	//CONTAR TEMPO COM O CONTADOR ZERO
	TCCR0A |= 0b00000000; // A contagem nao he usada para PWM
	TCCR0B |= 0b00000011; //clkI/O/64
	TIMSK0 |= _BV(TOIE0); // ativa o TOIE0
	
	while(1){
		sei(); // chama interrupcoes globais
		if(tempoamostra == 1){
			ADCSRA |= _BV(ADSC);
			while(!(ADCSRA & 0b00010000));
			/*
			valorAD = ADC; 
			vel_motor = (valorAD*15)/1023;
			OCR1A = vel_motor;
			OCR1A = valorAD;
			*/
			OCR1A = ADC;  
			tempoamostra = 0;
			
		}
	}
}

