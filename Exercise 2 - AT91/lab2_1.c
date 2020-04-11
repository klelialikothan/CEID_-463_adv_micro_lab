#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <header.h>

PIO *pioa = NULL;
AIC *aic = NULL;

int main (int argc, const char*argv[] ) {
	unsigned int gen;

	STARTUP;                                     // system initialisation 

	aic->FFER   = (1<<PIOA_ID);  			     // switches 2,17 -> FIQ
	pioa->CODR  = 0x7F;					 	     // lines 0-6: LOW voltage to init
	pioa->OER   = 0x7F;					         // lines 0-6: output mode 
	pioa->PER   = 0x7F;					         // lines 0-6: general purpose mode

	int cycles=1;
	int off_cycles=0;
	char tmp;

	while ((tmp = getchar()) != 'e') {    	     // exit condition
		if (tmp=='u'){							 // increase ratio
			if ((off_cycles-1)>=0){				 // off_cycles>=0     
		        off_cycles-=1;				     // adjust for next cycle
		    }
		    else {
		      off_cycles=0;						 // retain peak brightness
		    }
		}
		else if (tmp=='d'){					     // decrease ratio
			if ((off_cycles+1)<=100){			 // off_cycles<=100 
        		off_cycles+=1;				 	 // adjust for next cycle
        	}
      		else {								 
        		off_cycles=100;					 // remain at 0% brightness
      		}
		}
		for (cycles=1; cycles<=100; cycles++){   // display segments On/Off
		    if (cycles<=100-off_cycles){
		      pioa->CODR  = 0x7F;				 // segments On (Active Low)
		    }
		    else {
		      pioa->SODR  = 0x7F;				 // segments Off
		    }
		}
	}

	aic->IDCR = (1<<PIOA_ID);       	         // AIC interrupts disabled
	CLEANUP;
	return 0;
}
