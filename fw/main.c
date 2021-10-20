/**
 * program rc spínače
 * program by mel zapínat a vypínat výkonový výstup podle vstupního
 * signálu servo pulsů
 **/

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

#define IN (P2IN&BIT1)  // P2.1
#define NEG (P2IN&BIT0) // P2.0

#define POUT (1<<2) // P1.2 = PA.2
#define LED (1<<3) // P1.3 = PA.3

#define ON() do{PAOUT|=(POUT|LED);}while(0)
#define OFF() do{PAOUT&=~(POUT|LED);}while(0)

#define MS2P(x) (x/8)

#define PULSE_MAX (MS2P(2500)) // 2.5ms
#define PULSE_MIN (MS2P(500)) // 0.5ms

#define NO_PULSE_MAX (MS2P(50000)) // 50ms

#define PULSE_TH_LOW (MS2P(1300)) // 1.3ms
#define PULSE_TH_HIGH (MS2P(1700)) // 1.7ms
#define PULSE_HYST (MS2P(20)) // 0.02ms


int main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // vypnout bafana
	
	// IOs 
	PADIR = POUT | LED;
	PAOUT = 0;
	P2REN = 1;
    PM5CTL0 &= ~LOCKLPM5; // aktivace nastaveni IO (magie)
	
	// nastaveni citace
	TBCTL = 0x02E0; // SMCLK, Continuous mode, 1/8

	bool in = false;
	uint16_t pstart = 0;
	
	// detekce negovacího vstupu
	bool neg = false;
	if (NEG) {
		neg = true;
	}
	else {
		PAOUT = LED;
		pstart = TBR;
		while ((TBR - pstart) < 1000) {};
		if (NEG) neg = true;
		PAOUT = 0;
		while ((TBR - pstart) < 5000) {};
	}

	// smyčka
	while(1) {
		uint16_t now = TBR;
		
		bool nin = IN;
		// detekce pulsu
		if (nin != in) {
			if (in) { // sestupna hrana
				uint16_t pulse;
				pulse = now - pstart;
				// validizace pulsu
				if ((pulse < PULSE_MAX) && (pulse > PULSE_MIN)) {
					if (neg) {
						if (pulse < (PULSE_TH_LOW - PULSE_HYST)) ON();
						else if (pulse > (PULSE_TH_LOW + PULSE_HYST)) OFF();
					}
					else {
						if (pulse > (PULSE_TH_HIGH + PULSE_HYST)) ON();
						else if (pulse < (PULSE_TH_HIGH - PULSE_HYST)) OFF();
					}
				}
			}
			else { // nabezna hrana
				pstart = now;
			}
		}
		in = nin;
		
		// vypnout vystup kdyz nejsou pulsy
		if ((now - pstart) > NO_PULSE_MAX) OFF();
	}
}
