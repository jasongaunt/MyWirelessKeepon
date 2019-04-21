
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
    void parseCommand(char const *msg);
    void readResponse();
    void goHome();
    void sing();
    void setAudioLogs(boolean enable);
    // Sound
    void soundPlay(int snd);
    void soundRepeat(int snd);
    void soundDelay(int msec);
    void soundStop();
    // Speed
    void speedPan(int spd);
    void speedTilt(int spd);
    void speedPonSide(int spd);
    // Move
    void movePan(int pos);
    void moveTilt(int pos);
    void moveSide(SideMove pos);
    void movePon(PonMove pos);
    void moveStop();
    // Mode
    void mode(Mode m);
    // Strings
    bool startsWith(char const *msg, char const *cmp);
    char const *findNextWord(char const *msg);
  private:
    boolean audioLogs;
    char msg[32];
    byte device, cmd[2];
    void writeCommand();
};

// Setup pins and serial connection
Keepon::Keepon() {
  audioLogs = false;
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
void Keepon::speedPonSide(int spd) {
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

void Keepon::movePon(PonMove pos) {
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

///////////////// Strings!  //////////////

bool Keepon::startsWith(char const *msg, char const *cmp) {
  int i = 0;
  while (*(cmp + i)) {
  if (*(msg + i) != *(cmp + i)) return false;
    i++;
  }
  return true;
}

char const* Keepon::findNextWord(char const *msg) {
  while (*msg && *msg != ' ' && *msg != ';') msg++;
  while (*msg == ' ' || *msg == ';') msg++;
  return msg;
}

void Keepon::parseCommand(char const *msg) {
  if (startsWith(msg, "SOUND")) {
    if (startsWith(findNextWord(msg), "PLAY")) {
      int sound = atoi(findNextWord(msg + 10));
      soundPlay(sound);
    } else if (startsWith(findNextWord(msg), "REPEAT")) {
      int sound = atoi(findNextWord(msg + 12));
      soundRepeat(sound);
    } else if (startsWith(findNextWord(msg), "DELAY")) {
      int msec = atoi(findNextWord(msg + 11));
      soundDelay(msec);
    } else if (startsWith(findNextWord(msg), "STOP")) {
      soundStop();
    } else {
      Serial.println("Unknown SOUND Command");
    }
  } else if (startsWith(msg, "SPEED")) {
    if (startsWith(findNextWord(msg), "PAN")) {
      int spd = atoi(findNextWord(msg + 8));
      speedPan(spd);
    } else if (startsWith(findNextWord(msg), "TILT")) {
      int spd = atoi(findNextWord(msg + 9));
      speedTilt(spd);
    } else if (startsWith(findNextWord(msg), "PONSIDE")) {
      int spd = atoi(findNextWord(msg + 12));
      speedPonSide(spd);
    } else {
      Serial.println("Unknown SPEED Command");
    }
  } else if (startsWith(msg, "MOVE")) {
    if (startsWith(findNextWord(msg), "PAN")) {
      int pos = atoi(findNextWord(msg + 7));
      movePan(pos);
    } else if (startsWith(findNextWord(msg), "TILT")) {
      int pos = atoi(findNextWord(msg + 8));
      moveTilt(pos);
    } else if (startsWith(findNextWord(msg), "SIDE")) {
      if(startsWith(findNextWord(msg + 8), "CYCLE")) {
        moveSide(SideMove(CYCLE));
      } else if(startsWith(findNextWord(msg + 8), "CENTERFROMLEFT")) {
        moveSide(SideMove(CENTER_FROM_LEFT));
      } else if(startsWith(findNextWord(msg + 8), "RIGHT")) {
        moveSide(SideMove(RIGHT));
      } else if(startsWith(findNextWord(msg + 8), "CENTERFROMRIGHT")) {
        moveSide(SideMove(CENTER_FROM_RIGHT));
      } else if(startsWith(findNextWord(msg + 8), "LEFT")) {
        moveSide(SideMove(LEFT));
      }
    } else if (startsWith(findNextWord(msg), "PON")) {
      if(startsWith(findNextWord(msg + 7), "UP")) {
        movePon(PonMove(UP));
      } else if(startsWith(findNextWord(msg + 7), "HALFDOWN")) {
        movePon(PonMove(HALF_DOWN));
      } else if(startsWith(findNextWord(msg + 7), "DOWN")) {
        movePon(PonMove(DOWN));
      } else if(startsWith(findNextWord(msg + 7), "HALFUP")) {
        movePon(PonMove(HALF_UP));
      } 
    } else if (startsWith(findNextWord(msg), "STOP")) {
      moveStop();
    } 
    else {
      Serial.println("Unknown MOVE Command");
    }
  } if (startsWith(msg, "MODE")) {
    if (startsWith(findNextWord(msg), "DANCE")) {
      mode(Mode(DANCE));
    } else if (startsWith(findNextWord(msg), "TOUCH")) {
      mode(Mode(TOUCH));
    } else if (startsWith(findNextWord(msg), "TEMPO")) {
      mode(Mode(TEMPO));
    } else if (startsWith(findNextWord(msg), "SLEEP")) {
      mode(Mode(SLEEP));
    } else {
      Serial.println("Unknown MODE Command");
    }
  } else {
    Serial.print("Unknown command: ");
    Serial.println(msg);
  }
}

////////////// Read Response ////////////

boolean buttonState[8];
char* buttonName[] = {
  "DANCE", "", "HEAD", "TOUCH",
  "RIGHT", "FRONT", "LEFT", "BACK"};

boolean motorState[8];
char* motorName[] = {
  "PON FINISHED", "SIDE FINISHED", "TILT FINISHED", "PAN FINISHED",
  "PON STALLED", "SIDE STALLED", "TILT STALLED", "PAN STALLED"};

int encoderState[4], audioState[5], emfState[3], positionState[3];
char* encoderName[] = {
  "TILT NOREACH", "TILT FORWARD", "TILT BACK", "TILT UP",
  "PON HALFDOWN", "PON UP", "PON DOWN", "PON HALFUP",
  "SIDE CENTER", "SIDE LEFT", "SIDE RIGHT", "SIDE CENTER",
  "PAN BACK", "PAN RIGHT", "PAN LEFT", "PAN CENTER"};

unsigned long updatedButton = 0, updatedMotor = 0;

void Keepon::setAudioLogs(boolean enabled) {
  audioLogs = enabled;
}

void Keepon::readResponse() {
  int i;
  byte buttonResponse, motorResponse;
  int intResponse;

  if (millis() - updatedButton > 100) {
    updatedButton = millis();
    Wire.requestFrom((int)BUTTON, 1);
    if (Wire.available() >= 1) {
      buttonResponse = Wire.read();
      for (i = 0; i < 8; i++) {
        if (i != 1) {
          if (buttonResponse & (1<<i)) {
            if (!buttonState[i]) {
              Serial.print("BUTTON ");
              Serial.print(buttonName[i]);
              Serial.println(" ON");
              buttonState[i] = 1;
            }
          }
          else if (buttonState[i]) {
            Serial.print("BUTTON ");
            Serial.print(buttonName[i]);
            Serial.println(" OFF");
            buttonState[i] = 0;
          }
        }
      }
    }
  }

  if (millis() - updatedMotor > 300) {
    updatedMotor = millis();
    Wire.requestFrom((int)MOTOR, 13);
    if (Wire.available() >= 13) {
      motorResponse = Wire.read();
      for (i = 0; i < 8; i++) {
        if (motorResponse & (1<<i)) {
          if (!motorState[i]) {
            Serial.print("MOTOR ");
            Serial.println(motorName[i]);
            motorState[i] = 1;
          }
        } 
        else if (motorState[i]) {
          motorState[i] = 0;
        }
      }
      motorResponse = Wire.read();
      if (motorResponse != audioState[0]) {
        Serial.print("AUDIO TEMPO ");
        Serial.println(motorResponse);
        audioState[0] = motorResponse;
      }
      motorResponse = Wire.read();
      if (motorResponse != audioState[1]) {
        if (audioLogs) {
          Serial.print("AUDIO MEAN ");
          Serial.println(motorResponse);
        }
        audioState[1] = motorResponse;
      }
      motorResponse = Wire.read();
      if (motorResponse != audioState[2]) {
        if (audioLogs) {
          Serial.print("AUDIO RANGE ");
          Serial.println(motorResponse);
        }
        audioState[2] = motorResponse;
      }
      motorResponse = Wire.read();
      for (i = 0; i < 4; i++) {
        if ((motorResponse & (3<<(2*i))) != encoderState[i]) {
          encoderState[i] = motorResponse & (3<<(2*i));
          Serial.print("ENCODER ");
          Serial.println(encoderName[4*i+(encoderState[i]>>(2*i))]);
        }
      }
      motorResponse = Wire.read();
      intResponse = motorResponse;
      if (intResponse > 0) intResponse -= 127;
      if (intResponse != emfState[0]) {
        Serial.print("EMF PONSIDE ");
        Serial.println(intResponse);
        emfState[0] = intResponse;
      }
      motorResponse = Wire.read();
      intResponse = motorResponse;
      if (intResponse > 0) intResponse -= 127;
      if (intResponse != emfState[1]) {
        Serial.print("EMF TILT ");
        Serial.println(intResponse);
        emfState[1] = intResponse;
      }
      motorResponse = Wire.read();
      intResponse = motorResponse;
      if (intResponse > 0) intResponse -= 127;
      if (intResponse != emfState[2]) {
        Serial.print("EMF PAN ");
        Serial.println(intResponse);
        emfState[2] = intResponse;
      }
      motorResponse = Wire.read();
      if (motorResponse != audioState[3]) {
        //      Serial.print("AUDIO ENVELOPE ");
        //      Serial.println(motorResponse);
        audioState[3] = motorResponse;
      }
      motorResponse = Wire.read();
      if (motorResponse != audioState[4]) {
        Serial.print("AUDIO BPM ");
        Serial.println(motorResponse);
        audioState[4] = motorResponse;
      }
      motorResponse = Wire.read();
      intResponse = motorResponse - 127;
      if (intResponse != positionState[0]) {
        Serial.print("POSITION PONSIDE ");
        Serial.println(intResponse);
        positionState[0] = intResponse;
      }
      motorResponse = Wire.read();
      intResponse = motorResponse - 127;
      if (intResponse != positionState[1]) {
        Serial.print("POSITION TILT ");
        Serial.println(intResponse);
        positionState[1] = intResponse;
      }
      motorResponse = Wire.read();
      intResponse = motorResponse - 127;
      if (intResponse != positionState[2]) {
        Serial.print("POSITION PAN ");
        Serial.println(intResponse);
        positionState[2] = intResponse;
      }
    }
  }
}

////////////// Arduino Code /////////////

Keepon *keepon;

void setup() {
  keepon = new Keepon();
}

void loop() {
  char input[50];
  
  keepon->bootup();
  keepon->goHome();
  
  while(true) {
    keepon->readResponse();
    if (Serial.available() > 0) {
      Serial.readStringUntil(';').toCharArray(input, 50);
      Serial.println(input);
      keepon->parseCommand(input);
    }
  }
}
