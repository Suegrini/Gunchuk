#include "wiimote.h"
#include "profiles.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define recoilState outputStates[0]
#define rumbleState outputStates[1]
#define ext1State outputStates[2]
#define ext2State outputStates[3]

int8_t buttons[11] = {pinA, pinB, pinX, pinY, pinLeft, pinRight, pinUp, pinDown, pinMinus, pinHome, pinPlus};
int8_t outputs[7] = {pinRecoil, pinRumble, pinExt1, pinExt2, pinRLED, pinGLED, pinBLED};

uint8_t playerId, battery, lives, ammo, profile, ledR, ledG, ledB;
bool vOut1, vOut2, vOut3, vOut4;

#ifdef USE_TEMP
const uint16_t tempNormal = 50;
const uint16_t tempWarning = 60;
const uint16_t lockLowTime = 600;
const uint16_t lockHighTime = 45;
#endif

#ifdef USE_NUNCHUK
byte nunchukCal[16];
int nunX;
int nunY;

// Nunchuk initialization.
bool nunchukInit() {
  // Initialize extension unencrypted.
  Wire1.beginTransmission(0x52);
  Wire1.write(0xF0);
  Wire1.write(0x55);
  if (Wire1.endTransmission() != 0) return false;
  delay(10);
  Wire1.beginTransmission(0x52);
  Wire1.write(0xFB);
  Wire1.write(0x00);
  if (Wire1.endTransmission() != 0) return false;
	delay(10);
  
  // Verify if connected extension is nunchuk.
  Wire1.beginTransmission(0x52);
  Wire1.write(0xFA);
  if (Wire1.endTransmission() != 0) return false;
	delay(10);
  Wire1.requestFrom(0x52, 6);
  uint8_t nunchukID[6] = {0x00, 0x00, 0xA4, 0x20, 0x00, 0x00};
  for (int i = 0; i < 6; i++) {
    uint8_t readValue = Wire1.read();
    if (readValue != nunchukID[i]) return false;
  }
	delay(10);
  
  // Get nunchuk joystick calibration data.
  Wire1.beginTransmission(0x52);
  Wire1.write(0x20);
  if (Wire1.endTransmission() != 0) return false;
	delay(10);
  Wire1.requestFrom(0x52, 16);
  // Set nunchuk calibration data on registers
	for (int i = 0; i < 16; i++) {
		nunchukCal[i] = Wire1.read();
	}
  set_caldata(nunchukCal);

  return true;
}
#endif

// Wiimote button data stream.
byte *stream_callback(byte *buffer) {
	wiimote_write_buffer(buffer);
	return buffer;
}

// Store received data from buffer.
static void receive_data(byte off, byte count) {
  // Check if 6 bytes were received in address 0xe0.
  if (off == 0xe0 && count == 6) {
    playerId = (wiimote_registers[off] & 0x03) + 1;
    battery = (wiimote_registers[off] & 0xFC);

    recoilState = bitRead(wiimote_registers[off + 1], 0);
    rumbleState = bitRead(wiimote_registers[off + 1], 1);
    ext1State = bitRead(wiimote_registers[off + 1], 2);
    ext2State = bitRead(wiimote_registers[off + 1], 3);
    
    vOut1 = bitRead(wiimote_registers[off + 1], 4);
    vOut2 = bitRead(wiimote_registers[off + 1], 5);
    vOut3 = bitRead(wiimote_registers[off + 1], 6);
    vOut4 = bitRead(wiimote_registers[off + 1], 7);

    ledR = (wiimote_registers[off + 2] & 0x0F) * 255 / 15;    
    ledG = ((wiimote_registers[off + 2] & 0xF0) >> 4) * 255 / 15;
    ledB = (wiimote_registers[off + 3] & 0x0F) * 255 / 15;

    lives = wiimote_registers[off + 3] >> 4;
    ammo = wiimote_registers[off + 4];

    profile = wiimote_registers[off + 5];
  }
}

void setup() {
	// Set push button pins as input, turning pull-up on.
  uint32_t inputPins = 0;
  inputPins |= (pinA >= 0) ? (1 << pinA) : 0;
  inputPins |= (pinB >= 0) ? (1 << pinB) : 0;
  inputPins |= (pinX >= 0) ? (1 << pinX) : 0;
  inputPins |= (pinY >= 0) ? (1 << pinY) : 0;
  inputPins |= (pinLeft >= 0) ? (1 << pinLeft) : 0;
  inputPins |= (pinRight >= 0) ? (1 << pinRight) : 0;
  inputPins |= (pinUp >= 0) ? (1 << pinUp) : 0;
  inputPins |= (pinDown >= 0) ? (1 << pinDown) : 0;
  inputPins |= (pinMinus >= 0) ? (1 << pinMinus) : 0;
  inputPins |= (pinHome >= 0) ? (1 << pinHome) : 0;
  inputPins |= (pinPlus >= 0) ? (1 << pinPlus) : 0;
  gpio_init_mask(inputPins);
  gpio_set_dir_in_masked(inputPins);
  gpio_pull_up(inputPins);

  // Set analog stick pins as analog inputs.
  #ifdef USE_STICK
  adc_init();
  adc_gpio_init(26);
  adc_gpio_init(27);
  #endif

  // Set temperature sensor as analog input.
  #ifdef USE_TEMP
  adc_init();
  adc_gpio_init(28);
  #endif
  
	// Set output pins as output.
  uint32_t outputPins = 0;
  outputPins |= (pinRecoil >= 0) ? (1 << pinRecoil) : 0;
  outputPins |= (pinRumble >= 0) ? (1 << pinRumble) : 0;
  outputPins |= (pinExt1 >= 0) ? (1 << pinExt1) : 0;
  outputPins |= (pinExt2 >= 0) ? (1 << pinExt2) : 0;
  outputPins |= (pinRLED >= 0) ? (1 << pinRLED) : 0;
  outputPins |= (pinGLED >= 0) ? (1 << pinGLED) : 0;
  outputPins |= (pinBLED >= 0) ? (1 << pinBLED) : 0;
  gpio_init_mask(outputPins);
  gpio_set_dir_out_masked(outputPins);
  gpio_clr_mask(outputPins);

	// Prepare wiimote communications.
	wiimote_stream = stream_callback;
  wiimote_receive = receive_data;
	wiimote_init();
}

// Setup output peripherials and nunchuk.
void setup1() {
  #ifdef USE_NEOPIXEL
  // Initialize neopixels.
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  #endif

  #if defined(USE_DISPLAY) || defined(USE_NUNCHUK)
  // Initialize 2nd I2C bus.
  Wire1.setSDA(SDA1);
  Wire1.setSCL(SCL1);
  Wire.setClock(400000);
  Wire1.begin();
  #endif

  #ifdef USE_NUNCHUK
  // Set extension detect pin as input, turn pull-up on.
  gpio_init(nunchukDetect);
  gpio_pull_up(nunchukDetect);
  #endif

  #ifdef USE_DISPLAY
  // Initialize OLED display.
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(SSD1306_WHITE);
  oled.display();
  #endif
  
  // Setup array of profile functions.
  setupDisplayProfiles();
}

void loop() {
  // Read state of all button pins.
  int i = 0;
  uint32_t inputPins = gpio_get_all();
  for (i; i < 11; i++)
    buttonStates[i] = (buttons[i] >= 0) && (inputPins & (1 << buttons[i]));

  #ifdef USE_TEMP
  static unsigned long lastTime = 0;
  // Read analog input from sensor, convert to Celsius.
  adc_select_input(2);
  int temp =  ((adc_read() * (3300 / 4096)) - 500) / 10;

  unsigned long currTime = millis();
  if (temp < tempNormal) {
    // If solenoid temperature below tempNormal operate normally.
    gpio_put(pinRecoil,recoilState);
    lastTime = currTime;
  }
  else if (temp < tempWarning) {
    // If temperature above normal throttle activation.
    if (gpio_get(pinRecoil) && (!recoilState || currTime - lastTime >= lockHighTime))
    {
      gpio_put(pinRecoil, false);
      lastTime = currTime;
    }
    else if (currTime - lastTime >= lockLowTime && recoilState)
    {
      gpio_put(pinRecoil,true);
      lastTime = currTime;
    }
  }
  else {
    // If above warning temperature disable recoil.
    gpio_put(pinRecoil, false);
  }
  // If temperature sensor enabled skip setting solenoid output state.
  i = 1;
  #else
  i = 0;
  #endif

  // Set states for all inputs.
  for (i; i < 4; i++) {
    if (outputs[i] >= 0)
        gpio_put(outputs[i], outputStates[i]);
  }
  
  #if defined(USE_STICK) || defined(USE_NUNCHUK)
  if (!nunchukReady) {
    #ifndef USE_STICK
    // If USE_STICK is disabled and nunchuk is disconnected set stick to center values.
    sx = 0x7F;
    sy = 0x7F;
    #else
    // Read analog stick values.
    adc_select_input(0);
    sx = adc_read() >> 4;
    adc_select_input(1);
    sy = adc_read() >> 4;
    #endif
  }
  else
  {
    #ifdef USE_NUNCHUK
    // If reading from nunchuk set analog stick values from nunchuk.
    sx = nunX;
    sy = nunY;
    #endif
  }
  #endif

	delay(50);
}

void loop1() {
  #ifdef USE_NUNCHUK
  static bool wasNunchukDetected = false;
  bool nunchukDetected = !gpio_get(nunchukDetect);
  if (nunchukReady && nunchukDetected) {
    // If nunchuk initialized, read data from 2nd I2C bus.
    Wire1.beginTransmission(0x52);  
    Wire1.write(0x00);
    if (Wire1.endTransmission() == 0) {
      delay(10);
      Wire1.requestFrom(0x52, 6);
      byte nunchukData[6];
      for (int i = 0; i < 6; i++) {
        nunchukData[i] = Wire1.read();
      }

      nunX = nunchukData[0];
      nunY = nunchukData[1];
      
      accelX = nunchukData[2];
      accelY = nunchukData[3];
      accelZ = nunchukData[4];

      buttonStates[11] = !(nunchukData[5] & 0x02);
      buttonStates[12] = !(nunchukData[5] & 0x01);
    }
  }
  else if (nunchukDetected) {
    // If extension connected attempt to initialize nunchuk.
    nunchukReady = nunchukInit();
  }
  else if (wasNunchukDetected) {
    nunchukReady = false;
    set_caldata();
  }
  wasNunchukDetected = nunchukDetected;
  #endif

  // Run the currently selected profile if it's not null.
  if (displayProfiles[profile] != nullptr) displayProfiles[profile]();
}