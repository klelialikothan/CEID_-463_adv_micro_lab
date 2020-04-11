#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <header.h>

#define PIOA_ID 2
#define TC0_ID 17       
#define BUT_IDLE 0
#define BUT_PRESSED 1     
#define BUT_RELEASED 2   
#define LED_IDLE 0        
#define LED_FLASHING 1   

void FIQ_handler (void);

PIO *pioa = NULL;
AIC *aic = NULL;
TC *tc = NULL;

unsigned int button_state = BUT_IDLE;  // not functioned button
unsigned int led_state = LED_IDLE;     // not functioned led

int main (int argc, const char*argv[] ) {

	unsigned int gen;

	STARTUP;                   // system initialization 
	tc->Channel_0.RC = 8192;   // period = 1 second
	tc->Channel_0.CMR = 2084;  // slow clock, waveform, disable CLK on RC compare
	tc->Channel_0.IDR = 0xFF;  // disable all the switches
	tc->Channel_0.IER = 0x10;  // enable RC compare

	aic->FFER = (1<<PIOA_ID) | (1<<TC0_ID);  // switches 2,17 -> FIQ
	aic->IECR = (1<<PIOA_ID) | (1<<TC0_ID);  // PIOA & TC0 interrupts enabled
	pioa->PUER  = 0x01;						 // line 0: PULL−UP enabled
	pioa->ODR   = 0x01;						 // line 0: input mode
	pioa->CODR  = 0x02;						 // line 1: LOW voltage
	pioa->OER   = 0x02;						 // line 1: output mode 

	gen         = pioa->ISR;				 // clear PIOA
	pioa->PER   = 0x03;						 // lines 0,1: general purpose mode
	gen         = tc->Channel_0.SR;			 // clear TC0
	aic->ICCR   = (1<<PIOA_ID)|(1<<TC0_ID);  // clear AIC
	pioa->IER   = 0x01;						 // line 1: interrupts enabled

	char tmp; 								 // Initialization of tmp ADDED
	while ((tmp = getchar()) != 'e') {    	 // Exit condition

	}

	aic->IDCR = (1<<PIOA_ID) | (1<<TC0_ID);	 // AIC interrupts disabled
	tc->Channel_0.CCR = 0x02;				 // timer disabled
	CLEANUP;
	return 0;

}

void FIQ_handler(void) {

	unsigned int data_in = 0;
	unsigned int fiq = 0;
	unsigned int data_out;

	fiq = aic->IPR;				// identify interrupt source

	//if(! fiq & (1<<PIOA_ID) ){	// if interrupt source is PIOA
	if (fiq & (1<<PIOA_ID)) {	// if interrupt source is PIOA, removed NOT
		data_in = pioa->ISR;		// clear interrupt source
		aic->ICCR = (1<<PIOA_ID);	// clear AIC interrupt
		data_in = pioa->PDSR;		// read input value
		if (!(data_in & 0x01)) {    // CHANGED -> button pressed (input_en && input ==1)
			if(button_state == BUT_IDLE) {
				button_state = BUT_PRESSED;
				if(led_state == LED_IDLE){
					tc->Channel_0.CCR = 0x05;  // start counter
					led_state = LED_FLASHING;  // start flashing
				}
				else {
					tc->Channel_0.CCR = 0x02;  // stop counter
					led_state = LED_IDLE;	   // stop flashing
				}
			}
		}
		else {
			if (button_state == BUT_PRESSED) {
				button_state = BUT_IDLE;
			}
		}
	}

	if (fiq & (1<<TC0_ID)) {
		data_out = tc->Channel_0.SR;	// clear interrupt source
		aic->ICCR = (1<<TC0_ID);		// clear AIC interrupt
		data_out = pioa->ODSR;			// read output value
		pioa->SODR =  data_out | 0x02;	// CHANGED bitwise AND to OR
		pioa->CODR = data_out & 0x02;	// COMMENT!!!!!!!!!!!!!!!
		tc->Channel_0.CCR = 0x05;		// COMMENT!!!!!!!!!!!!!!!
	}
	
}












