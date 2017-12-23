#include <Wire.h>

#define JOYPAD_BUTTONS_COUNT 10

#include <Joypad.h>

#define DEV_ADDR 0x52
#define CRYPT_KEY 0x17

//#define DEBUG // define to enable debug (no rly?)
//#define STICK_CALIBRATION // define to enable stick calibration. I don't even know if it works well.

#define KEY_GR 1<<4
#define KEY_RE 1<<6
#define KEY_YE 1<<3
#define KEY_BL 1<<5
#define KEY_OR 1<<7

byte stickCal[4];

void setup() {
  for (byte i=0; i<4; i++) {
    stickCal[i] = 0;
  }

#ifdef DEBUG
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Opened serial port!");
  Serial.print("Device address: ");
  Serial.println(DEV_ADDR, HEX);
#endif

  Wire.begin();
  #ifdef DEBUG
  Serial.println("Starting transmission ...");
  #endif
  Wire.beginTransmission(DEV_ADDR);
  #ifdef DEBUG
  Serial.print("Initialization ... ");
  #endif
  Wire.write(0xF0);
  Wire.write(0x55);
  Wire.endTransmission();
  Wire.beginTransmission(DEV_ADDR);
  Wire.write(0xFB);
  Wire.write(0x00);
  Wire.endTransmission();
  #ifdef DEBUG
  Serial.println("Done");

  Serial.print("Initializing USB gamepad ... ");
  #endif
  Joypad.begin();
  #ifdef DEBUG
  Serial.println("Done");
  #endif
}

void handshake() {
  Wire.beginTransmission(DEV_ADDR);
  Wire.write(0x00);
  Wire.endTransmission();
}

byte buffer[6];
#ifdef CHANGES_ONLY
byte lastBuffer[6];
#endif
byte cnt = 0;

void parse() {
  byte StickX = buffer[0]<<2; // only 6 bits
  byte StickY = buffer[1]<<2; // same

  byte touchBar = buffer[2] << 3; // only 5 bits
  byte whammy = buffer[3]<<3; // same
  
  #ifdef DEBUG
  Serial.print("SX: ");
  Serial.print(StickX, DEC);
  Serial.print("\tSY: ");
  Serial.print(StickY, DEC);
  Serial.print("\ttouchBar: ");
  Serial.print(touchBar, DEC);
  Serial.print("\twhammy: ");
  Serial.println(whammy, DEC);

  Serial.print(buffer[4], BIN);
  Serial.print(" ");
  Serial.println(buffer[5], BIN);
  #endif

  // Now we're talking ... on the USB
  Joypad.joystick(StickX, StickY);
  Joypad.rjoystick(touchBar, whammy);

  Joypad.releaseAll();
  // GRYBO: WHY AREN'T THEY IN THE SAME ORDER IN THE REGISTERS
  if ((buffer[5] & KEY_GR) == 0) Joypad.press(0);
  if ((buffer[5] & KEY_RE) == 0) Joypad.press(1);
  if ((buffer[5] & KEY_YE) == 0) Joypad.press(2);
  if ((buffer[5] & KEY_BL) == 0) Joypad.press(3);
  if ((buffer[5] & KEY_OR) == 0) Joypad.press(4);

  // strum
  if ((buffer[4] & 1<<6) == 0) Joypad.press(5); // down
  if ((buffer[5] & 1<<0) == 0) Joypad.press(6); // up

  // misc.
  if ((buffer[4] & 1<<2) == 0) Joypad.press(7); // +
  if ((buffer[4] & 1<<4) == 0) Joypad.press(8); // - and star power
  
  Joypad.update();
}

#ifdef DEBUG
bool statusLED = HIGH;
#endif

void loop() {
  Wire.requestFrom(DEV_ADDR, 6);
  while (Wire.available()) {
    buffer[cnt] = (Wire.read());// ^ CRYPT_KEY) + CRYPT_KEY;
    cnt++;
    #ifdef DEBUG
    digitalWrite(LED_BUILTIN, statusLED);
    statusLED = !statusLED;
    #endif
  }
  if (cnt >= 5) parse();

  cnt = 0;
  handshake();
  delay(2);
}
