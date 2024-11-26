/*
 * config.h
 * 
 * This header file contains the configuration settings for the Gunchuk.
 * 
 * Configurable Features:
 * - USE_STICK: Enable analog stick support.
 * - USE_ANALOGRGB: Enable support for 4 pin RGB LEDs.
 * - USE_NEOPIXEL: Enable support for WS2812 addressable LEDs (Neopixels).
 * - USE_DISPLAY: Enable support for SSD1306 OLED displays.
 * - USE_TEMP: Enable support for solenoid temperature sensor.
 * - USE_NUNCHUK: Enable nunchuk compatibility (untested).
 * 
 * Pin Settings:
 * - Define input pins, set to -1 if not used.
 * - Define output pins, set to -1 if not used.
 * 
 * Joystick Calibration:
 * - Define min, max, and mid values for the joystick.
 * 
 * Neopixel Settings:
 * - Define the number of LEDs and data pin for Neopixels.
 * 
 * OLED Display:
 * - Define screen dimensions (only 128x64 and 128x32 displays currently supported).
 * 
 * Static LED Colors:
 * - Optionally set static color values for 4 pin RGB LEDs and Neopixels.
 * 
 * To use this file:
 * - Uncomment the features you need.
 * - Set the corresponding pin numbers and calibration values as needed.
 */

#ifndef CONFIG_H
#define CONFIG_H

// Enable peripherials, comment to disable.
#define USE_STICK
#define USE_ANALOGRGB
#define USE_NEOPIXEL
#define USE_DISPLAY
#define USE_TEMP
#define USE_NUNCHUK

// Set input pins, to unset set to -1.
const int pinA          = 15;
const int pinB          = -1;
const int pinX          = -1;
const int pinY          = -1;
const int pinLeft       = -1;
const int pinRight      = -1;
const int pinUp         = -1;
const int pinDown       = -1;
const int pinMinus      = -1;
const int pinHome       = -1;
const int pinPlus       = -1;
const int nunchukDetect = 14; // Set if nunchuk enabled

// Set output pins, to unset set to -1.
const int pinRecoil     = -1;
const int pinRumble     = -1;
const int pinExt1       = -1;
const int pinExt2       = -1;
const int pinRLED       = -1;
const int pinGLED       = -1;
const int pinBLED       = -1;

// Set Neopixel number of leds and data pin.
const int NUM_LEDS      =  1;
const int DATA_PIN      = 16;

// Set OLED display dimensions.
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Set static color for 4 pin RGB LEDs (can be overriden by custom profiles).
//#define USE_STATIC_NEOPIXEL

// Set static color values for 4 pin RGB LEDs.
#ifdef USE_STATIC_NEOPIXEL
const int sNeoR = 255;
const int sNeoG = 255;
const int sNeoB = 255;
#endif

// Set static color for Neopixels (can be overriden by custom profiles).
//#define USE_STATIC_ANALOGRGB

// Set static color values for Neopixels.
#ifdef USE_STATIC_ANALOGRGB
const int sAnalogR = 255;
const int sAnalogG = 255;
const int sAnalogB = 255;
#endif

#endif // CONFIG_H
