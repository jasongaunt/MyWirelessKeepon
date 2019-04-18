
#include <Wire.h>
#include "Keepon.h"

#define cbi(sfr, bit) _SFR_BYTE(sfr) &= ~_BV(bit)
#define sbi(sfr, bit) _SFR_BYTE(sfr) |= _BV(bit)
#define MK_FREQ 49600L // Set clock to 50kHz (actualy 49.6kHz seems to work better)
#define SOUND (byte)0x52  // Sound controller (device ID 82).  Write to 0xA4, read from 0xA5.
#define BUTTON (byte)0x50 // Button controller (device ID 80). Write to 0xA0, read from 0xA1.
#define MOTOR (byte)0x55  // Motor controller (device ID 85).  Write to 0xAA, read from 0xAB.
#define BAUD 115200 // baud rate

//////////// Class Declaration ////////////////

class Keepon {
  public:
    Keepon();
    // Utilites
    void bootup();
    void keeParse(String& msg);
    void goHome();
    void sing();
    // Sound
    void soundPlay(int snd);
    void soundRepeat(int snd);
    void soundDelay(int msec);
    void soundStop();
    // Speed
    void speedPan(int spd);
    void speedTilt(int spd);
    void speedPONSide(int spd);
    // Move
    void movePan(int pos);
    void moveTilt(int pos);
    void moveSide(SideMove pos);
    void movePon(PONMove pos);
    void moveStop();
    // Mode
    void mode(Mode m);
  private:
    char msg[32];
    byte device, cmd[2];
    void writeCommand();
};

// Setup pins and serial connection
Keepon::Keepon() {
  pinMode(SDA, OUTPUT); // Data wire on My Keepon
  pinMode(SCL, OUTPUT); // Clock wire on My Keepon
  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);
  Serial.begin(BAUD);
  while(!Serial);
}

////////// Keepon Utilities /////////////

// Wait for keepon power on
void Keepon::bootup() {
  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);
  Serial.print("Waiting for My Keepon... ");
  while (analogRead(0) < 512); // Wait until we see voltage on A0 pin
  Serial.println("My Keepon detected.");
  delay(1000);
  Wire.begin();
  TWBR = ((F_CPU / MK_FREQ) - 16) / 2;
  Serial.write((byte)0);
}

void Keepon::keeParse(String& msg) {
  Serial.println(msg);
}

void Keepon::writeCommand() {
  int result = 1;
  int attempts = 0;
  while (result != 0 && attempts++ < 50) {
    Wire.beginTransmission(device);
    Wire.write((byte)cmd[0]);
    Wire.write((byte)cmd[1]);
    result = (int)Wire.endTransmission();
  }
}

void Keepon::goHome() {
  Serial.println("Returning to home position.");
  movePan(0);
  moveTilt(0);
  soundStop();
}

void Keepon::sing() {
  for (int i = 0; i < 64; i++) {
    soundPlay(i);
    delay(1000);
  }
}

////////// Keepon Commands ///////////////

// Sound 
void Keepon::soundPlay(int snd) {
  device = SOUND;
  cmd[0] = 0x01;
  cmd[1] = B10000000 | (63 & snd);
  writeCommand();
}

void Keepon::soundRepeat(int snd) {
  device = SOUND;
  cmd[0] = 0x01;
  cmd[1] = B11000000 | (63 & snd);
  writeCommand();
}

void Keepon::soundDelay(int msec) {
  device = SOUND;
  cmd[0] = 0x03;
  cmd[1] = (byte)msec;
  writeCommand();
}

void Keepon::soundStop() {
  device = SOUND;
  cmd[0] = 0x01;
  cmd[1] = B00000000;
  writeCommand();
}

// Speed

void Keepon::speedPan(int spd) {
  device = MOTOR;
  cmd[0] = 5;
  cmd[1] = (byte)spd;
  writeCommand();
}
void Keepon::speedTilt(int spd){
  device = MOTOR;
  cmd[0] = 3;
  cmd[1] = (byte)spd;
  writeCommand();
}
void Keepon::speedPONSide(int spd) {
  device = MOTOR;
  cmd[0] = 1;
  cmd[1] = (byte)spd;
  writeCommand();
}

// Move

void Keepon::movePan(int pos) {
  device = MOTOR;
  cmd[0] = 4;
  cmd[1] = (byte)(pos + 127);
  writeCommand();
}

void Keepon::moveTilt(int pos) {
  device = MOTOR;
  cmd[0] = 2;
  cmd[1] = (byte)(pos + 127);
  writeCommand();
}

void Keepon::moveSide(SideMove pos) {
  device = MOTOR;
  cmd[0] = 0;
  switch(pos) {
    case CYCLE:
      cmd[1] = 0;
      break;
    case CENTER_FROM_LEFT:
      cmd[1] = 1;
      break;
    case RIGHT:
      cmd[1] = 2;
      break;
    case CENTER_FROM_RIGHT:
      cmd[1] = 3;
      break;
    case LEFT:
      cmd[1] = 4;
      break;
  }
  writeCommand();
}

void Keepon::movePon(PONMove pos) {
  device = MOTOR;
  cmd[0] = 0;
  switch(pos) {
    case UP:
      cmd[1] = -1;
      break;
    case HALF_DOWN:
      cmd[1] = -2;
      break;
    case DOWN:
      cmd[1] = -3;
      break;
    case HALF_UP:
      cmd[1] = -4;
      break;
  }
  writeCommand();
}

void Keepon::moveStop() {
  device = MOTOR;
  cmd[0] = 6;
  cmd[1] = 16;
  writeCommand();
}

// Mode

void Keepon::mode(Mode m) {
  cmd[0] = 6;
  switch(m) {
    case DANCE:
      cmd[1] = 0;
      break;
    case TOUCH:
      cmd[1] = 1;
      break;
    case TEMPO:
      cmd[1] = 2;
      break;
    case SLEEP:
      cmd[1] = 240;
      break;
  }
  writeCommand();
}

////////////// Arduino Code /////////////

Keepon *keepon;

void setup() {
  keepon = new Keepon();
}

void loop() {
  keepon->bootup();
  keepon->goHome();
  while(true) {
    if (Serial.available() > 0) {
      String input = Serial.readStringUntil(';');
      Serial.println(input);
    }
  }
}
