#include <Arduino.h>
#include <Keyboard.h>

#define BOARD_MAJOR_VERSION 1
#define BOARD_MINOR_VERSION 1
#define BOARD_PATCH_VERSION 0
#include "ArcadeBoardTypeA.h"

#define IFD if(debug)

bool debug = false;

/***********************************************************
 * ┌──────────────────── MuseDashCon ───────────────────┐
 * │┌────┐       ┌────┐    ┌────┐    ┌────┐       ┌────┐│
 * ││ 00 │       │ 01 │    │ 02 │    │ 03 │       │ 04 ││
 * │└────┘       └────┘    └────┘    └────┘       └────┘│
 * │ L10     ┌────┐   ┌────┐    ┌────┐   ┌────┐    L11  │
 * │         │ 05 │   │ 06 │    │ 07 │   │ 08 │         │
 * │         └────┘   └────┘    └────┘   └────┘         │
 * │                       ┌────┐                       │
 * │                       │ 09 │                       │
 * │                       └────┘                       │
 * └────────────────────────────────────────────────────┘
 **********************************************************/

uint8_t keyBinds[12] = { 'k', 'q', 'w', 'e', \
						' ', 'j', 'a', 'd', \
						'j', 's', KEY_ESC, KEY_TAB };

// Variables for Long Press Feature Implementation
uint8_t k0_t = 0;
uint8_t k1_t = 0;
bool k0_p = 0;
bool k1_p = 0;

// Function Prototypes
void keyEvent(uint8_t swidx, uint8_t type);
void tick_loop();

// Run setup() once
void setup() {
	// Powering on with pressing switch #0 to enter debug mode
	if(!digitalRead(PIN_SW0)) debug = true;

	pinMode(LED_BUILTIN, OUTPUT);
	delay(1000);

	// if debug mode enabled
	if(debug) {
		Serial.begin(9600);
		while(!Serial) {
			PORTC ^= (1 << 7);
			delay(100);
		}
		Serial.println(F("DEBUG MODE ACTIVATED"));
	}

	IFD Serial.println(F("_0x4d's Arcade Board Type A"));

	// Initiating LED Controller
	IFD Serial.print(F("Initiate LED controller..."));
	int r;
	if((r = LED.init(0x4d)) != 0) {
		IFD Serial.println(F("failed"));
		IFD Serial.println(r);
	}
	else {
		for(int i = 0; i < 10; i++) {
			LED.setValue(i, 0);
		}
		LED.update();
		IFD Serial.println(F("success"));
		delay(1000);
	}

	// Initiating Switch Controller
	IFD Serial.print(F("Initiate Switch Controller..."));
	delay(100);
	Switch.init(POLLING_RATE_1000HZ);
	IFD Serial.println(F("success"));

	// Testing LED
	IFD Serial.print(F("LED testing..."));
	for(int i = 0; i < 10; i++) {
		LED.setValue(i, 255);
		LED.update();
		delay(100);
		LED.setValue(i, 30);
		LED.update();
		delay(10);
	}
	IFD Serial.println(F("done"));

	for(int i = 0; i < 10; i++) LED.setValue(i, 30);
	LED.update();
	delay(100);

	// Starting keyboard service
	Keyboard.begin();
	delay(100);

	// Add callback function to Swtich Controller
	Switch.setCallback(keyEvent);
	Switch.setCallbackOnTick(tick_loop);
}

// Run loop() forever
void loop() {
	LED.update();
	delay(16);
}

void keyEvent(uint8_t swidx, uint8_t type) {
	switch(type) {
		case FALLING:
		Keyboard.press(keyBinds[swidx]);
		LED.setValue(swidx, 0xFF);
		if(swidx == 0) {
			k0_t = 125;
			k0_p = 1;
		}
		else if(swidx == 4) {
			k1_t = 125;
			k1_p = 1;
		}
		break;

		case RISING:
		Keyboard.release(keyBinds[swidx]);
		LED.setValue(swidx, 30);
		if(swidx == 0) {
			Keyboard.release(keyBinds[10]);
			k0_t = 125;
			k0_p = 0;
		}
		else if(swidx == 4) {
			Keyboard.release(keyBinds[11]);
			k1_t = 125;
			k1_p = 0;
		}
		break;
	}
}

void tick_loop() {
	static uint8_t t1 = 0;
	static uint8_t t2 = 10;
	static uint8_t t3 = 0;

	// on every 4 ticks
	if(++t1 == 4) {
		t1 = 0;
		if(k0_p == 1 && --k0_t == 0) Keyboard.press(keyBinds[10]);
		if(k1_p == 1 && --k1_t == 0) Keyboard.press(keyBinds[11]);
		
		// on every 4 * 250 ticks
		if(++t2 == 250) {
			PORTC &= ~(1 << 7);
		}
		// on every 4 * 250 ticks
		if(++t3 == 250) {
			PORTC |= (1 << 7);
		}
	}
}
