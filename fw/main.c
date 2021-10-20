/**
 * program waxingu
 * program by mel tocit motorem pokud vidi na vstupu pulsy
 *
 * status:
 * nastaveni vystupu, krokovani motoru, rampa rychlosti, nastavitelna rychlost,
 * 3 rychlosti v zavislosti na frekvenci pulsu
 *      20ms ~ cca 10ot/min
 *      25ms ~ cca 20ot/min
 *      30ms ~ cca 30ot/min
 * 
 * todo:
 * nastavitelna rychlost (pomoci makra)
 * nekolik rychlosti v zavislosti na vstupu
 **/

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

#define IN (P2IN&BIT6)

#define EN (1<<7) // P1.7 = PA.7
#define DIR (1<<8) // P2.0 = PA.8
#define STEP (1<<9) // P2.1 = PA.9

#define OUT_DISABLED 0x4004
#define OUT_STEP_UP (OUT_DISABLED | EN | STEP)
#define OUT_STEP_DOWN (OUT_DISABLED | EN)

#define MOTOR_STEP() do{PAOUT=OUT_STEP_UP;PAOUT=OUT_STEP_DOWN;}while(0)
#define MOTOR_ENABLE() do{PAOUT=OUT_STEP_DOWN;}while(0)
#define MOTOR_DISABLE() do{PAOUT=OUT_DISABLED;}while(0)

#define MS2P(x) (x/8)

#define PULSE_MAX (MS2P(2000)) // 2ms
#define PULSE_MIN (MS2P(400)) // 0.4ms

#define NO_PULSE_MAX (MS2P(65000)) // 65ms

#define PERIOD_TH_LOW (22500/8) // 22.5ms
#define PERIOD_TH_HIGH (27500/8) // 27.5ms


#define ACCEL_TIME 300 // ms
//#define TICK_FREQV 2000L // Hz (cca 17ot/min)
#define TICK_FREQV 4000L // Hz (cca 32ot/min)
#define CPU_FREQV 1000000L
#define TICK_CNT CPU_FREQV/8/TICK_FREQV
#define SPEED_MAX TICK_FREQV*ACCEL_TIME/1000
#define SPEED_MIN SPEED_MAX/10

#define SPEED_HIGH SPEED_MAX
#define SPEED_MID (SPEED_MAX*2/3)
#define SPEED_LOW (SPEED_MAX/3)

#define HOME(x) ((x&0x7F)==0) // motor home position

uint16_t paout;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // vypnout bafana
	
	// IOs (P1.7, P2.0, P2.1 OUT, P2.6 IN)
	PADIR = 0x08;
	PAOUT = 0x08;
    PM5CTL0 &= ~LOCKLPM5; // aktivace nastaveni IO (magie)
	
	// nastaveni citace
	TBCTL = 0x02E0; // SMCLK, Continuous mode, 1/8

	while(1) {};

	while(1) {
		static uint16_t tim = 0; // casovac kroku
		static uint16_t tp = 0; // citac mereni pulsu
		static uint8_t in = 0; // vstup (posledni stav)
		static bool run = false; // beh
		static uint16_t spd = 0; // aktualni rychlost (0 - 1000)
		static uint16_t stp = 0; // citac krokove frekvence
		static uint16_t speed = 0; // prednastavena rychlost
		static uint16_t step_cnt = 0; // citac kroku
		
		static uint16_t preset = 4000;
		
		uint16_t now = TBR;
		
		uint8_t nin = IN;
		if (nin != in) {
			if (nin) {
				// preset rychlosti
				uint16_t period = now - tp;
				preset = SPEED_HIGH;
				if (period > PERIOD_TH_HIGH)
					preset = SPEED_MID;
				if (period > PERIOD_TH_LOW)
					preset = SPEED_LOW;
				// zacatek pulsu
				tp = now;
			}
			else {
				// konec pulsu - kontrola sirky pulsu
				uint16_t pulse;
				pulse = now - tp;
				if ((pulse < PULSE_MAX) && (pulse > PULSE_MIN))
					run = true;
			}
			in = nin;
		}
		
		// detekce zmizeni pulsu
		if ((now - tp) > NO_PULSE_MAX)
			run = false;
		
		// zapnuti motoru kdyz je priznak "run"
		if (run) {
			MOTOR_ENABLE();
			speed = preset;
		}
		else {
			speed = SPEED_MIN;
			// vypnuti motoru kdyz je rychlost SPEED_MIN a motor je v HOME pozici
			if ((spd <= speed) && (HOME(step_cnt))) {
				speed = 0;
				spd = 0;
				tim = now;
				step_cnt = 0;
				MOTOR_DISABLE();
			}
		}
		
		// krokovani a rampy
		if ((now - tim) > TICK_CNT) {
			tim = now;
			if (spd < speed)
				spd += 1;
			if (spd > speed)
				spd -= 1;
			stp += spd;
			if (stp >= SPEED_MAX) {
				MOTOR_STEP();
				step_cnt += 1;
				stp -= SPEED_MAX;
			}
		}
	}
}
