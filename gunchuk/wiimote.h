/*
 * Copyright (c) 2011 Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * Bare-bones Nintendo Wiimote library for Arduino, tricking the Wiimote into
 * treating the Arduino as a Classic Controller.  Communication works both to and from
 * the Arduino.
 *
 * Using insights into the communication between Wiimote and Nunchuk found
 * at http://www.ccsinfo.com/forum/viewtopic.php?p=91094.  The wmgui tool that
 * comes with cwiid is good for experimenting and testing.
 *
 * Here's how to connect the Arduino to the Wiimote:
 *      Arduino    Wiimote
 *      GND        GND  (white cable)
 *      AREF       3.3V (red cable)
 *      A4         SDA  (green cable)
 *      A5         SCL  (yellow cable)
 *
 * It doesn't look like it's possible to power the Arduino from the Wiimote.
 *
 * Note that some online instructions for using a Wiimote extension 
 * with Arduino call for changes to twi.h.  
 * Fortunately, this modification is no longer
 * necessary.  Just use a current version of the Arduino development
 * environment.
 */
 /* Modified to work with Gunchuk extension */
 
#ifndef __WIIMOTE_H__
#define __WIIMOTE_H__

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <Wire.h>
#include "wm_crypto.h"
#include "config.h"

const int SDA0 = 0;
const int SCL0 = 1;
const int SDA1 = 26;
const int SCL1 = 27;

// Identification sequence for Gunchuck.
static byte idbuf[] = { 0x00, 0x00, 0xa4, 0x20, 0x67, 0x67 };

// Gunchuck calibration data. Include 'ideal' calibration values for nunchuk accelerometer.
static byte calbuf[16] = { 0x7F, 0x7F, 0x7F, 0x00, 0xB4, 0xB4, 0xB4, 0x00, 0xFF, 0x00, 0x7F, 0xFF, 0x00, 0x7F };  // Checksum will be calculated later...

// Gunchuck analog stick. Initialized to calibration values.
static byte sx = 0x7F;
static byte sy = 0x7F;

// Nunchuk accelerometers. Initialized to calibration values.
static byte accelX = 0x80;
static byte accelY = 0x80;
static byte accelZ = 0x80;

// Gunchuck button state.
static bool buttonStates[13];

// Gunchuck outputs state.
static bool outputStates[4];

// Receiving nunchuk data.
static bool nunchukReady = false;

static byte outbuf[6];
static byte *curbuf = outbuf;
static byte state = 0;
static byte crypt_setup_done = 0;

/*
 * This function pointer informs the Arduino that a byte has been
 * written to one of the registers corresponding to the Extension.
 * Specifically, if you assign the function foo to this pointer and
 * you write yy bytes starting at the register address 04a400xx of
 * the Wiimote, then this library will call foo(xx, yy).  The bytes
 * are written to the global array wiimote_registers.
 *
 * Note that count will always be less than eight.  If you write
 * eight or more bytes to the Wiimote, this callback will be
 * invoked several times.
 */
static void (*wiimote_receive)(byte offset, byte count) = NULL;

/*
 * Callback for streaming bytes from the Arduino to the Wiimote.
 * If this function pointer is not null, then it will be called
 * right _after_ the current output buffer has been sent to the
 * Wiimote, and the implementing function has the choice of either
 * overwriting the given buffer and returning it or returning
 * another buffer that contains the next six bytes to be sent.
 *
 * The return value of this function becomes the new output buffer.
 * The idea is much like the one behind wiimote_swap_buffers.  You
 * probably want to use only one of them; either wiimote_stream
 * for streaming data, or wiimote_swap_buffers for more sporadic
 * updates.
 *
 * There is one exception to this rule: Since this function is
 * called after the current output buffer has been sent, the very
 * first buffer will be all zero unless you initialize it with
 * wiimote_set_byte or wiimote_swap_buffers.  (This approach may
 * seem awkward, but it minimizes delays in I2C communication, and
 * in most cases an all-zero initial buffer is not a problem.)
 */
static byte *(*wiimote_stream)(byte *buffer) = NULL;

/*
 * Global array for storing and accessing register values written
 * by the Wiimote.  Those are the 256 registers corresponding to
 * the Extension, starting at 04a40000.
 */
static byte wiimote_registers[0x100];

/*
 * Start Wiimote <-> Extension communication encryption,
 * if requested by the application.
 */
static void setup_encryption() {
	int i;

	for (i = 0x40; i <= 0x49; i++) {
		wm_rand[9 - (i - 0x40)] = wiimote_registers[i];
	}

	for (i = 0x4A; i <= 0x4F; i++) {
		wm_key[5 - (i - 0x4A)] = wiimote_registers[i];
	}

	wm_gentabs();

	crypt_setup_done = 1;
}

/*
 * Set calibration data on Wiimote registers.
 * Calibration data is stored in addresses 0x20 and 0x30.
 */
static void set_caldata(byte caldata[16] = calbuf) {
	byte calchecksum = 0;
  
  // Fix calibration data checksum, just in case...
	for(int i = 0; i < 14; i++) {
		calchecksum += calbuf[i];
	}
	caldata[14] = calchecksum + 0x55;
	caldata[15] = calchecksum + 0xAA;

	// Set calibration data on registers
	for (int i = 0x20; i <= 0x2F; i++) {
		wiimote_registers[i] = caldata[i - 0x20]; // 0x20
		wiimote_registers[i + 0x10] = caldata[i - 0x20]; // 0x30
	}
}

/*
 * Generic function for sending data via Wire.write().
 * Supports Wiimote encryption, if enabled.
 */
static void send_data(uint8_t* data, uint8_t size, uint8_t addr) {
	static uint8_t buffer[8];
	int i;

	if (wiimote_registers[0xF0] == 0xAA && crypt_setup_done) {
		for (i = 0; i < size; i++) {
			buffer[i] = (data[i] - wm_ft[(addr + i) % 8]) ^ wm_sb[(addr + i)
					% 8];
		}

		Wire.write(buffer, size);
	} else {
		Wire.write(data, size);
	}
}

static void receive_bytes(int count) {
	byte crypt_keys_received = 0;

	if (count == 1) {
		state = Wire.read();
	} else if (count > 1) {
		byte addr = Wire.read();
		byte curr = addr;

		for (int i = 1; i < count; i++) {
			byte d = Wire.read();

			// Wii is trying to disable encryption...
			if(addr == 0xF0 && d == 0x55) {
				crypt_setup_done = 0;
			}

			if (wiimote_registers[0xF0] == 0xAA && crypt_setup_done) {
				// Decrypt
				wiimote_registers[curr] = (d ^ wm_sb[curr % 8]) + wm_ft[curr
						% 8];
				curr++;
			} else {
				wiimote_registers[curr++] = d;
			}

			// Check if last crypt key setup byte was received...
			if (curr == 0x50) {
				crypt_keys_received = 1;
			}

		}
		if (wiimote_receive) {
			wiimote_receive(addr, (byte) count - 1);
		}
	}
  if (state != 0)
    Serial.println(state);
	// Setup encryption
	if (crypt_keys_received) {
		setup_encryption();
	}

}

static void handle_request() {

	static byte last_state = 0xFF;
	static byte offset = 0;

	switch (state) {

	case 0x00:
		send_data(curbuf, 6, 0x00);

		if (wiimote_stream) {
			curbuf = wiimote_stream(curbuf);
		}

		break;

	case 0xFA:
		send_data(wiimote_registers + state, 6, state);

		break;

	case 0x20:
	case 0x30:
		if(last_state == state) {
			offset += 8;
      last_state = 0xFF;
		} else {
			last_state = state;
			offset = 0;
		}
    Serial.println(offset);
		send_data(wiimote_registers + state + offset, 8, state + offset);
		break;

	default:
		if(last_state == state) {
			offset += 8;
		} else {
			last_state = state;
			offset = 0;
		}

		send_data(wiimote_registers + state + offset, 8, state + offset);
		break;
	}
}

/*
 * Sets the given value of the current buffer.
 *
 * index of the value to be set, 0 <= index < 6
 * value new value of the byte at the given index
 *
 * Make sure to read the documentation of wiimote_stream if you
 * intend to use both wiimote_stream and this function.
 */
static void wiimote_set_byte(int index, byte value) {
	curbuf[index] = value;
}

/*
 * Makes the given buffer the current buffer and returns the
 * previous buffer.  Use this function if you want to change
 * several values at once.
 *
 * buffer must be a byte buffer of length at least 6.
 *
 * Returns the previous buffer, suitable for reuse.
 *
 * Make sure to read the documentation of wiimote_stream if you
 * intend to use both wiimote_stream and this function.
 */
static byte *wiimote_swap_buffers(byte *buffer) {
	byte *tmp = curbuf;
	curbuf = buffer;
	return tmp;
}

/*
 * Takes joystick, and button values and encodes them
 * into a buffer.
 *
 * Buffer encoding details:
 * http://wiibrew.org/wiki/Wiimote/Extension_Controllers/Classic_Controller
 */
static void wiimote_write_buffer(byte *buffer) {
	buffer[0] = (accelZ & 0xC0) | (sx >> 2);
	buffer[1] = ((accelZ & 0x30) << 2) | (sy >> 2);
	buffer[2] = ((accelZ & 0x08) << 4) | (accelX >> 1);
	buffer[3] = ((accelZ & 0x04) << 5) | (accelY >> 1);

	buffer[4] = (buttonStates[5] << 7) | (buttonStates[7] << 6) | (nunchukReady << 5) | (buttonStates[8] << 4) | 
      (buttonStates[9] << 3) | (buttonStates[10] << 2) | (accelZ & 0x02);

	buffer[5] = buttonStates[12] << 7 | (buttonStates[1] << 6) | (buttonStates[3] << 5) | (buttonStates[0] << 4) | 
      (buttonStates[2] << 3) | buttonStates[11] << 2 | (buttonStates[4] << 1) | buttonStates[6];

	buffer[4] ^= 0x3F;
	buffer[5] = ~buffer[5];
}

/*
 * Initializes Wiimote connection. Call this function in your
 * setup function.
 */
static void wiimote_init() {
	memset(wiimote_registers, 0xFF, 0x100);

	// Set extension id on registers
	for (int i = 0xFA; i <= 0xFF; i++) {
		wiimote_registers[i] = idbuf[i - 0xFA];
	}

  // Set calibration data on registers
	set_caldata();

	// Initialize curbuf, otherwise, "Up+Right locked" bug...
	wiimote_write_buffer(curbuf);

	// Encryption disabled by default
	wiimote_registers[0xF0] = 0x55;
	wiimote_registers[0xFB] = 0x00;

  Wire.setSDA(SDA0);
  Wire.setSCL(SCL0);
  
	// Join I2C bus
	Wire.begin(0x52);

	Wire.onReceive(receive_bytes);
	Wire.onRequest(handle_request);
}

#endif