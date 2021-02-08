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
 * │ L10     ┌────┐   ┌────┐    ┌────┐   ┌────┐    D11  │
 * │         │ 05 │   │ 06 │    │ 07 │   │ 08 │         │
 * │         └────┘   └────┘    └────┘   └────┘         │
 * │                       ┌────┐                       │
 * │                       │ 09 │                       │
 * │                       └────┘                       │
 * └────────────────────────────────────────────────────┘
 **********************************************************/
// Long Press 00: L10
// Double Press 04: D11

uint8_t keyBinds[12] = { 'k', 'q', 'w', 'e', \
						' ', 'j', 'a', 'd', \
						'j', 's', KEY_ESC, KEY_TAB };

// Variables for Long Press Feature Implementation
uint8_t k0_t = 0;
uint8_t k1_t = 0;
uint8_t k1_t2 = 0;
bool k0_p = 0;
bool k1_p = 0;
bool k1_lp = false;
uint8_t idle_led_val = 100;
uint8_t b_level = 0;

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
	Switch.setInputDelay(20);
}

// Run loop() forever
void loop() {
	for(uint8_t i = 0; i < 16; i++) {
		if(LED.getValue(i) != 255) LED.setValue(i, idle_led_val);
	}
	LED.update();
	Serial.print(k1_p);
	Serial.print(' ');
	Serial.println(k1_t);
	delay(16);
}

void keyEvent(uint8_t swidx, uint8_t type) {
	switch(type) {
		case FALLING:
		LED.setValue(swidx, 0xFF);
		if(swidx == 4) {
			if(k1_t > 0) { // BTN04 Double Tap
				k1_t = 0;
				k1_p = false;
				Keyboard.press(keyBinds[11]);
				Keyboard.release(keyBinds[11]);
			}
			else {
				k1_t = 64;
				k1_p = true;
			}
		}
		else {
			Keyboard.press(keyBinds[swidx]);
			if(swidx == 0) {
				k0_t = 125;
				k0_p = 1;
			}
		}
		break;

		case RISING:
		LED.setValue(swidx, idle_led_val);

		if(swidx == 4) {
			k1_p = false;
			if(k1_lp) {
				Keyboard.release(keyBinds[4]);
				k1_lp = false;
			}
		}
		else {
			Keyboard.release(keyBinds[swidx]);
			if(swidx == 0) {
				Keyboard.release(keyBinds[10]);
				k0_t = 125;
				k0_p = 0;
			}
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
		if(k1_t > 0) {
			if(--k1_t == 0) { // Timeout Reached
				if(!k1_p) { // BTN04 Single Tap
					Keyboard.press(keyBinds[4]);
					k1_t2 = 5;
				}
				else if(k1_p) { // BTN04 Long Press
					Keyboard.press(keyBinds[4]);
					k1_lp = true;
				}
			}
		}
		if(k1_t2 > 0) {
			if(--k1_t2 == 0) {
				Keyboard.release(keyBinds[4]);
			}
		}
		
		// on every 4 * 250 ticks
		if(++t2 == 250) {
			PORTC &= ~(1 << 7);
			if(abs(b_level - CdS.getBrightnessLevel()) > 0) {
				if(b_level > CdS.getBrightnessLevel()) b_level--;
				else b_level++;
			}
			idle_led_val = b_level * 7;
		}
		// on every 4 * 250 ticks
		if(++t3 == 250) {
			PORTC |= (1 << 7);
		}
	}
}
