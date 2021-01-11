// --------------------------------------
//        ArcadeBoardTypeA v1.0.0
// --------------------------------------
// https://github.com/factoryal/Arcade-Board

#pragma once

#include "Arduino.h"
#include <Wire.h>

#define PIN_SW0 0    // PD2
#define PIN_SW1 1    // PD3
#define PIN_SW2 4    // PD4
#define PIN_SW3 12   // PD6
#define PIN_SW4 6    // PD7
#define PIN_SW5 SCK  // PB1
#define PIN_SW6 MOSI // PB2
#define PIN_SW7 MISO // PB3
#define PIN_SW8 8    // PB4
#define PIN_SW9 9    // PB5
#define PIN_SW10 10  // PB6
#define PIN_SW11 11  // PB7
#define PIN_SW12 A5  // PF0
#define PIN_SW13 A4  // PF1
#define PIN_SW14 A3  // PF4
#define PIN_SW15 A2  // PF5

#define PIN_LIGHT_SENSOR A1

#define POLLING_RATE_125HZ 2000
#define POLLING_RATE_250HZ 1000
#define POLLING_RATE_500HZ 500
#define POLLING_RATE_1000HZ 250
#define POLLING_RATE_2000HZ 125
#define CUSTOM_POLLING_RATE(x) (25000 / x)


// http://ww1.microchip.com/downloads/en/devicedoc/atmel-7766-8-bit-avr-atmega16u4-32u4_datasheet.pdf

typedef uint16_t POLLING_RATE_TYPEDEF;

class SwitchController {
private:
	uint16_t sw_state_now = 0xFFFF;
	uint16_t sw_state_before = 0xFFFF;
	void (*_event_callback)(uint8_t, uint8_t) = NULL;
	void (*_tick_callback)() = NULL;

public:
	SwitchController() {

	}

	void init(POLLING_RATE_TYPEDEF polling_rate) {
		cli();
		TCCR3A = (1 << WGM31);
		TCCR3B = (1 << WGM33) | (1 << WGM32) | (1 << CS31) | (1 << CS30);
		TIMSK3 = (1 << TOIE3);
		ICR3 = polling_rate - 1;
		sei();
	}

	void update() {
		if(_tick_callback) _tick_callback();

		sw_state_now = \
					((uint16_t)(PINF & 0x30) << 10) | ((uint16_t)(PINF & 0x03) << 12) |\
					((uint16_t)(PINB & 0xFE) << 4) |\
					((PIND & 0xC0) >> 3) | ((PIND & 0x1C) >> 2);

		uint16_t changes = sw_state_now ^ sw_state_before;
		if(changes) {
			uint16_t mask = 0x0001;
			for(uint8_t i = 0; i < 16; i++, mask = mask << 1) {
				if(_event_callback && (changes & mask)) {
					_event_callback(i, ((sw_state_now >> i) & 0x0001) | 0x0002);
					
				}
			}
		}
		sw_state_before = sw_state_now;
	}

	void setCallback(void (*callback)(uint8_t swidx, uint8_t type)) {
		_event_callback = callback;
	}

	void setCallbackOnTick(void (*callback)()) {
		_tick_callback = callback;
	}

} Switch;


// https://www.nxp.com/docs/en/data-sheet/PCA9635.pdf

class LEDController {
private:
	uint8_t brightness[16] = { 0 };
	uint8_t addr;

public:
	LEDController() {
		Wire.begin();
		Wire.setTimeout(100);
	}

	int init(uint8_t address) {
		addr = address;
		Wire.beginTransmission(addr);
		int8_t r = Wire.endTransmission();
		if(!r) {
			setRegister(addr, 0x00, 0x00);
			setRegister(addr, 0x01, 0x14);
			for(uint8_t i = 0; i < 4; i++) {
				setRegister(addr, 0x14 + i, 0xFF);
			}
		}
		return r;
	}

	void setValue(uint8_t idx, uint8_t val) {
		brightness[idx] = val;
	}

	uint8_t getValue(uint8_t idx) {
		return brightness[idx];
	}

	int update() {
		Wire.beginTransmission(addr);
		Wire.write(0xA2);
		for(uint8_t i = 0; i < 16; i++) {
			Wire.write(brightness[i]);
		};                                                                
		return Wire.endTransmission();
	}

private:
	static uint8_t setRegister(uint8_t address, uint8_t reg, uint8_t val) {
		Wire.beginTransmission(address);
		Wire.write(reg);
		Wire.write(val);
		return Wire.endTransmission();
	}

} LED;

ISR(TIMER3_OVF_vect) {
	Switch.update();
}