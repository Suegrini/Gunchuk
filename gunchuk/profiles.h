/*
 * profiles.h
 * 
 * This header file defines different display profiles for the Gunchuk.
 * Each profile function can update the display, LEDs, and other peripherals based on received data or button states.
 * Includes default profiles and placeholders for custom profiles that can be added as needed.
 * 
 * Profiles:
 * - defaultProfile: Sets LEDs to received or static color, displays battery and player information.
 * - inGame: Extends defaultProfile by adding life and ammo counters on the display.
 * - Custom profiles: Users can add their own profiles for specific needs.
 * - Profiles are selected frome the profile data received by the Gunmote.
 * - Up to 256 profiles can be selected, including the default profile. 
 *
 * To use this file:
 * - Define your custom profile functions.
 * - Assign profile index in setupDisplayProfiles().
 */

#ifndef PROFILES_H
#define PROFILES_H

#include "wiimote.h"

#define ba buttonStates[0]
#define bb buttonStates[1]
#define bx buttonStates[2]
#define by buttonStates[3]
#define bdl buttonStates[4]
#define bdr buttonStates[5]
#define bdu buttonStates[6]
#define bdd buttonStates[7]
#define bminus buttonStates[8]
#define bhome buttonStates[9]
#define bplus buttonStates[10]
#define bc buttonStates[11]
#define bz buttonStates[12]

#ifdef USE_NEOPIXEL
#include <FastLED.h>
CRGB leds[NUM_LEDS];
#endif

#ifdef USE_DISPLAY
#include <Adafruit_SSD1306.h>
#define OLED_RESET -1
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, OLED_RESET);
#endif

extern uint8_t playerId, battery, lives, ammo, mode, ledR, ledG, ledB;
extern bool vOut1, vOut2, vOut3, vOut4;

typedef void (*Profile)();
static Profile displayProfiles[256];

// Set LEDs to received or static color, display battery and player information.
static void defaultProfile() {
  #ifdef USE_NEOPIXEL
  #ifndef USE_STATIC_NEOPIXEL
  // If using neopixels and static color not set, set all LEDs to received color.
  fill_solid(leds, NUM_LEDS, CRGB(ledR, ledG, ledB));
  FastLED.show();
  #else
  // Set all LEDs to static color.
  fill_solid(leds, NUM_LEDS, CRGB(sNeoR, sNeoG, sNeoB));
  FastLED.show();
  #endif
  #endif

  #ifdef USE_ANALOGRGB
  #ifndef USE_STATIC_ANALOGRGB
  // If using 4 pin LED and static color not set, set LEDs to received color.
  analogWrite(pinRLED, ledR);
  analogWrite(pinGLED, ledG);
  analogWrite(pinBLED, ledB);
  #else
  // Set LED to static color.
  analogWrite(pinRLED, sAnalogR);
  analogWrite(pinGLED, sAnalogG);
  analogWrite(pinBLED, sAnalogB);
  #endif
  #endif

  #ifdef USE_DISPLAY
  oled.clearDisplay();

  #if defined(SCREEN_HEIGHT) && (SCREEN_HEIGHT == 64)
  // Display mode for 128x64 displays.
  oled.setTextSize(2);
  if (playerId > 0) {
    oled.setCursor(104, 0);
    oled.print("P");
    oled.print(playerId);
  }

  // Display battery indicator.
  oled.drawRect(0, 2, 26, 12, SSD1306_WHITE);
  oled.fillRect(27, 4, 3, 8, SSD1306_WHITE);
  int fillWidth = map(battery, 0, 100, 0, 24);
  
  oled.fillRect(1, 3, fillWidth, 10, SSD1306_WHITE);
  oled.fillRect(0, 18, 128, 2, SSD1306_WHITE);

  #elif defined(SCREEN_HEIGHT) && (SCREEN_HEIGHT == 32)
  // Display mode for 128x32 displays.
  oled.setTextSize(1);
  if (playerId > 0) {
    oled.setCursor(116, 0);
    oled.print("P");
    oled.print(playerId);
  }

  // Display battery indicator.
  oled.drawRect(0, 1, 18, 6, SSD1306_WHITE);
  oled.fillRect(19, 2, 2, 4, SSD1306_WHITE);
  int fillWidth = map(battery, 0, 100, 0, 16);
  oled.fillRect(1, 2, fillWidth, 4, SSD1306_WHITE);

  oled.drawFastHLine(0, 9, 128, SSD1306_WHITE);
  #endif
  
  oled.display();
  #endif
}

// Same as defaultProfile + life and ammo counters in display.
static void inGame() {
  #ifdef USE_NEOPIXEL
  #ifndef USE_STATIC_NEOPIXEL
  // If using neopixels and static color not set, set all LEDs to received color.
  fill_solid(leds, NUM_LEDS, CRGB(ledR, ledG, ledB));
  FastLED.show();
  #else
  // Set all LEDs to static color.
  fill_solid(leds, NUM_LEDS, CRGB(sNeoR, sNeoG, sNeoB));
  FastLED.show();
  #endif
  #endif

  #ifdef USE_ANALOGRGB
  #ifndef USE_STATIC_ANALOGRGB
  // If using 4 pin LED and static color not set, set LEDs to received color.
  analogWrite(pinRLED, ledR);
  analogWrite(pinGLED, ledG);
  analogWrite(pinBLED, ledB);
  #else
  // Set LED to static color.
  analogWrite(pinRLED, sAnalogR);
  analogWrite(pinGLED, sAnalogG);
  analogWrite(pinBLED, sAnalogB);
  #endif
  #endif

  #ifdef USE_DISPLAY
  oled.clearDisplay();
  
  #if defined(SCREEN_HEIGHT) && (SCREEN_HEIGHT == 64)
  // Display mode for 128x64 displays.
  oled.setTextSize(2);
  if (playerId > 0) {
    oled.setCursor(104, 0);
    oled.print("P");
    oled.print(playerId);
  }

  // Display battery indicator.
  oled.drawRect(0, 2, 26, 12, SSD1306_WHITE);
  oled.fillRect(27, 4, 3, 8, SSD1306_WHITE);
  int fillWidth = map(battery, 0, 100, 0, 24);
  oled.fillRect(1, 3, fillWidth, 10, SSD1306_WHITE);

  oled.fillRect(0, 18, 128, 2, SSD1306_WHITE);
  oled.fillRect(64, 20, 2, 45, SSD1306_WHITE);

  // Display Lives data.
  char text[4];
  oled.setCursor(8, 24);
  oled.println("Life");
  snprintf(text, sizeof(text), "%d", lives);
  oled.setCursor((64 - (strlen(text) * 12)) / 2, 44); // Center lives counter.
  oled.print(lives);

  // Display Ammo data.
  oled.setCursor(76, 24);
  oled.println("Ammo");
  snprintf(text, sizeof(text), "%d", ammo);
  oled.setCursor(68 + ((64 - (strlen(text) * 12)) / 2), 44); // Center ammo counter.
  oled.print(ammo);

  #elif defined(SCREEN_HEIGHT) && (SCREEN_HEIGHT == 32)
  // Display mode for 128x32 displays.
  oled.setTextSize(1);
  if (playerId > 0) {
    oled.setCursor(116, 0);
    oled.print("P");
    oled.print(playerId);
  }

  // Display battery indicator.
  oled.drawRect(0, 1, 18, 6, SSD1306_WHITE);
  oled.fillRect(19, 2, 2, 4, SSD1306_WHITE);
  int fillWidth = map(battery, 0, 100, 0, 16);

  oled.fillRect(1, 2, fillWidth, 4, SSD1306_WHITE);
  oled.drawFastHLine(0, 9, 128, SSD1306_WHITE);

  // Display Lives data.
  char text[4];
  oled.setCursor(0, 13);
  oled.println("Life");
  snprintf(text, sizeof(text), "%d", lives);
  oled.setCursor((64 - (strlen(text) * 6)) / 2, 24); // Center lives counter.
  oled.print(lives);

  // Display Ammo data.
  oled.setCursor(64, 13);
  oled.println("Ammo");
  snprintf(text, sizeof(text), "%d", ammo);
  oled.setCursor(64 + ((64 - (strlen(text) * 6)) / 2), 24);  // Center ammo counter.
  oled.print(ammo);
  #endif

  oled.display();
  #endif
}

// Add custom profile functions here.
/*
static void customProfile() {
  // Your profile.
}
*/

// Assign index for custom profiles here
static void setupDisplayProfiles() {
    displayProfiles[0] = defaultProfile;
    displayProfiles[1] = inGame;
    // displayProfiles[2] = customProfile;
}

#endif // PROFILES_H