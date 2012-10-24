/*
 * eh_demo.c
 *
 * Created: 10/15/2012 6:44:30 PM
 *  Author: Kris
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/delay.h>
#define F_CPU 1000000UL

#define DELAY 5000
#define PULSE_PIN 5
#define INT_PIN 4

char pcint = 0;
int seconds = 0;

void pulse();
void sleep();
void init();
void init_timer();

int main() {
	init();
	//init_timer();
	while(1) {

		while(pcint || (PINC & (1 << INT_PIN))) {
			pcint = 0;
			pulse();
		}
		sei();
		sleep();
	}
	return 0;
}

void init_timer() {
	//disable timer interrupts (OCIE2x, TOIE2x)
	TIMSK2 |= (0 << OCIE2B) | (0 << OCIE2B) | (0 << TOIE2);
	//select clock source (AS2)
	ASSR |= (1 << AS2);
	//write new values (TCNT2, OCR2x, TCCR2x)
	TCNT2 = 0;
	OCR2A = 0;
	OCR2B = 0;
	TCCR2A |= (0 << COM2A1) | (0 << COM2A0) | (0 << COM2B1) | (0 << COM2B0) | (0 << WGM21) | (0 << WGM20);
	TCCR2B |= (0 << FOC2A) | (0 << FOC2B) | (0 << WGM22) | (1 << CS22) | (0 << CS21) | (1 << CS20);
	//wait for (TCN2xUB, OCR2xUB, TCR2xUB)
	while(ASSR & ((1 << TCN2UB) | (1 << OCR2AUB) | (1 << OCR2AUB) | (1 << TCR2AUB) | (1 << TCR2BUB))) {
	}
	//clear interrupt flags
	TIFR2 |= (1 << OCF2B) | (1 << OCF2A) | (1 << TOV2);
	//enable interrupts
	TIMSK2 |= (0 << OCIE2B) | (0 << OCIE2B) | (1 << TOIE2);
}

void init() {
	PRR = (1 << 7) | (0 << 6) | (1 << 5) | (1 << 3) | (0 << 2) | (1 << 1) | (1 << 0);
	DDRC |= (1 << PULSE_PIN);  // set LED pin as an output
	PCMSK1 |= (1 << INT_PIN);
	PCICR |= (1 << 1);  // enable PCINT0
	SMCR |= (1 << 0);   // enable sleep
	SMCR |= (0 << 3) | (1 << 2) | (1 << 1);  // set sleep mode to power-save
	sei();   // enable interrupts
}

ISR(PCINT1_vect) {
	cli();
	pcint = 1;
	EIMSK |= 3;
	sei();
}	

ISR(TIMER2_OVF_vect) {
	cli();
	seconds++;
	sei();
}

void pulse() {
	cli();
	PORTC |= (1 << PULSE_PIN);
	_delay_ms(1000);
	PORTC &= ~(1 << PULSE_PIN);
	sei();
}

void sleep() {
	asm("sleep");
}