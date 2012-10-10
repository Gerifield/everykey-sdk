// This is a test program to make sure that all pins work and that none
// of them are short circuited.
// Upon loading the program and resetting the key, all gpio pins of the
// processor that are connected to pads along the edge of the board are
// high.
// Pressing the key will turn on one pad after the next in numerical
// order:
//
           26     (...)     14
				+---------------------+
				|                     |
			1	|  O                  |
	(...)	|                    +---
				|     TOP            | U
				|                    | S
				|    +----+          | B
				|    |    |          +---
				|    |Proc|           |
		 13 |    +----+           |
				+---------------------+

// Pads are "numbered" counterclockwise, starting at the left (looking a
// the key form the top, i.e processor, side so left pads (top to
// bottom) are 1 - 13 and the top pads (from left to right) 14-26

// MOST of the pads are connected to GPIO pins, with the following
// exceptions:
//
// Pad
//  5				Open drain
// 12				Open drain
// 17       External Power or USB Power
// 18       Regulated 3.3V used as ADC reference
// 19       Ground
//
// So ... Pads 5 and 12 WON'T light up (if you are testing like we do
// with a LED connected to GND on the cathode and trying the pads with
// the anode) .  They will be low (draining) when off.
//
// Finally, Pad 20 is connected to the key, it will be turn to high
// after pad 26, of course afterwards the key will no longer work and
// you'll need to reset it to test the key again.

#include "pressanykey/pressanykey.h"


#define KEY_PORT 1
#define KEY_PIN 4

#define LED_PORT 0
#define LED_PIN 7

# define SYSTICK_CNTR 0x000AFC7F



// port and pin are stuck together in on byte ...
const uint8_t pinports[] = {
	0x17, // 1_7
	0x16, // 1_6
	0x01, // 1_0
	0x20, // 2_0
	0x04,
	0x10,      
	0x06,
	0x0B,
	0x0A,
	0x02,
	0x07,
	0x05,
	0x21,

	0x22,
	0x11,
	0x12,
	//0, // ext bpower
	//0, // vdd
	//0, // gnd
	//(1<<4 & 4), // // button special case. pad 20/19
	0x23,
	0x13,
	0x15,
	0x2B,
	0x09,
	0x08
};




#define NUM_PIN_PORT 22
void set_functions() {
	// some pads aren't 
	GPIO_SETFUNCTION(1,7,PIO,IOCON_IO_ADMODE_DIGITAL)
	GPIO_SETFUNCTION(1,6,PIO,IOCON_IO_ADMODE_DIGITAL)
	GPIO_SETFUNCTION(0,1,PIO,IOCON_IO_ADMODE_DIGITAL)
	GPIO_SETFUNCTION(2,0,PIO,IOCON_IO_ADMODE_DIGITAL)
	GPIO_SETFUNCTION(0,4,PIO,IOCON_IO_ADMODE_DIGITAL)
	GPIO_SETFUNCTION(1,0,PIO,IOCON_IO_ADMODE_DIGITAL)
	GPIO_SETFUNCTION(0,6,PIO,IOCON_IO_ADMODE_DIGITAL)
	GPIO_SETFUNCTION(0,11,PIO,IOCON_IO_ADMODE_DIGITAL)
	GPIO_SETFUNCTION(0,10,PIO,IOCON_IO_ADMODE_DIGITAL)
	GPIO_SETFUNCTION(0,2,PIO,IOCON_IO_ADMODE_DIGITAL)
	GPIO_SETFUNCTION(0,7,PIO,IOCON_IO_ADMODE_DIGITAL)
	GPIO_SETFUNCTION(0,5,PIO,IOCON_IO_ADMODE_DIGITAL)
	GPIO_SETFUNCTION(2,1,PIO,IOCON_IO_ADMODE_DIGITAL)

	GPIO_SETFUNCTION(2,2,PIO,IOCON_IO_ADMODE_DIGITAL)
	GPIO_SETFUNCTION(1,1,PIO,IOCON_IO_ADMODE_DIGITAL)
	GPIO_SETFUNCTION(1,2,PIO,IOCON_IO_ADMODE_DIGITAL)
	// pwr
	// vdd
	// gnd
	// key
	GPIO_SETFUNCTION(2,3,PIO,IOCON_IO_ADMODE_DIGITAL)
	GPIO_SETFUNCTION(1,3,PIO,IOCON_IO_ADMODE_DIGITAL)
	GPIO_SETFUNCTION(1,5,PIO,IOCON_IO_ADMODE_DIGITAL)
	GPIO_SETFUNCTION(2,11,PIO,IOCON_IO_ADMODE_DIGITAL)
	GPIO_SETFUNCTION(0,9,PIO,IOCON_IO_ADMODE_DIGITAL)
	GPIO_SETFUNCTION(0,8,PIO,IOCON_IO_ADMODE_DIGITAL)
};

void initialize_ports (void) {
	int i;
	uint8_t pp, pin, port;
	set_functions();
	for (i=0; i!=NUM_PIN_PORT; ++i) {
		pp = pinports[i];
		port = 0x0f & (pp>>4);
		pin = 0x0f & pp;
		
		GPIO_SetDir(port,pin, GPIO_Output);
		GPIO_WriteOutput(port, pin, true);
	}
	GPIO_SetDir(KEY_PORT, KEY_PIN, GPIO_Input);
	GPIO_SETPULL(KEY_PORT, KEY_PIN, IOCON_IO_PULL_UP);
}

bool key_state;
int pad;


void set_pad(int * pad) {
	uint8_t i, port, pin;
	uint8_t pp; 
	bool val;
	

	for (i = 0; i!= NUM_PIN_PORT; ++i) {
		pp = pinports[i];
		port = 0x0f & (pp >> 4);
		pin  = 0x0f & pp;

		if (i == *pad) {
			val = true;	
		} else {
			val = false;	
		}
		GPIO_WriteOutput(port, pin, val);
	}

	if (*pad == NUM_PIN_PORT) {
		GPIO_SetDir(KEY_PORT, KEY_PIN, GPIO_Output);
		GPIO_WriteOutput(KEY_PORT, KEY_PIN, true);
	} 
	
	*pad += 1;
	
}

#define pressed false
void systick(void) {
	bool curr_state;	

	curr_state = GPIO_ReadInput(KEY_PORT, KEY_PIN);

	if (key_state != curr_state) {
		if (curr_state == pressed) {
			GPIO_WriteOutput(LED_PORT, LED_PIN, true);
		} else {
			GPIO_WriteOutput(LED_PORT, LED_PIN, false);
			set_pad(&pad);
		}
	}
	key_state = curr_state;
}

void main (void) {
	key_state = true;
	pad = 0;
	initialize_ports();
  SYSCON_StartSystick(SYSTICK_CNTR);
}
