/*
 * eh_demo.c
 *
 * Created: 10/15/2012 6:44:30 PM
 *  Author: Crispytronics
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/delay.h>

#define F_CPU 1000000UL
#define TX_PERIOD_SEC 60
#define VOUT2_EN 5
#define PGD 4

char pgd_interrupt = 0;
unsigned int seconds = 0;
int last_tx = -1;

void pulse_vout2en();
void sleep();
void init();
void init_timer();

int main() {
	init();
	init_timer();
	while(1) {
		if((pgd_interrupt || PINC & (1 << PGD)) && (last_tx < 0 || last_tx + TX_PERIOD_SEC <= seconds)) {
			pgd_interrupt = 0;
			pulse_vout2en();
			last_tx = seconds;
		}
		sleep();
	}
	return 0;
}

void init_timer() {
	TIMSK2 |= (0 << OCIE2B) | (0 << OCIE2B) | (0 << TOIE2);
	ASSR |= (1 << AS2);
	TCNT2 = 0;
	OCR2A = 0;
	OCR2B = 0;
	TCCR2A |= (0 << COM2A1) | (0 << COM2A0) | (0 << COM2B1) | (0 << COM2B0) | (0 << WGM21) | (0 << WGM20);
	TCCR2B |= (0 << FOC2A) | (0 << FOC2B) | (0 << WGM22) | (1 << CS22) | (1 << CS21) | (1 << CS20);
	while(ASSR & ((1 << TCN2UB) | (1 << OCR2AUB) | (1 << OCR2AUB) | (1 << TCR2AUB) | (1 << TCR2BUB))) {
	}
	TIFR2 |= (1 << OCF2B) | (1 << OCF2A) | (1 << TOV2);
	TIMSK2 |= (0 << OCIE2B) | (0 << OCIE2B) | (1 << TOIE2);
}

void init() {
	PRR = (1 << 7) | (0 << 6) | (1 << 5) | (1 << 3) | (0 << 2) | (1 << 1) | (1 << 0);
	DDRC |= (1 << VOUT2_EN);
	PCMSK1 |= (1 << PGD);
	PCICR |= (1 << 1);
	SMCR |= (1 << 0);
	SMCR |= (0 << 3) | (1 << 2) | (1 << 1);
	sei();
}

ISR(PCINT1_vect) {
	cli();
	if(PINC & (1 << PGD)) {
		pgd_interrupt = 1;
	}	
	sei();
}	

ISR(TIMER2_OVF_vect) {
	cli();
	unsigned int temp = seconds;
	seconds += 8;
	//check counter for rollover
	if(temp > seconds) {
		last_tx = -1;
	}
	sei();
}

void pulse_vout2en() {
	cli();
	PORTC |= (1 << VOUT2_EN);
	_delay_ms(1500);
	PORTC &= ~(1 << VOUT2_EN);
	sei();
}

void sleep() {
	asm("sleep");
}