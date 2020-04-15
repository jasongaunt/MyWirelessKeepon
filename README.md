# My Keepon Arduino controller - now with added Wireless connectivity courtesy of ESP8266!

## Credits

* Original code by Marek Michalowski (BeatBots) with help from  Kyle Machulis (Nonpolynomial Labs) & Mark Gasson (Wow! Stuff)
* Tweaks to BPM code by Tyler Krupicka (2019)
* Wireless conversion by Jason Gaunt (2020)

## Original release info

Copyright © 2012 BeatBots LLC

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

For a copy of the GNU General Public License, see
http://www.gnu.org/licenses/gpl.html

For support, please post issues on the Github project page at
http://github.com/BeatBots/MyKeepon

Learn more about My Keepon at
http://mykeepon.beatbots.net

Keepon® is a trademark of BeatBots LLC.


## Original release instructions

Connect the following pins with a 4-wire ribbon cable:

```
My Keepon  <->  Arduino
---------       -------
V               A0
Cl              A5
Da              A4
G               GND
```

Then compile this code to your Arduino.  (Recommended & tested: Arduino Nano v3.)
  
With My Keepon powered off, use the Arduino Serial Monitor (or otherwise connect to the Arduino with your own application, e.g. Max patch MyKeepon.maxpat or standalone MyKeepon.app, which we provide here) at 115200 baud.

Turn on your My Keepon.  You should receive a "My Keepon detected" message.
  
Allowable commands (the closing semicolon is required):
```
    SOUND PLAY <0...63>;
    SOUND REPEAT <0...63>;
    SOUND DELAY <msec>;
    SOUND STOP;
    SPEED [PAN, TILT, PONSIDE] <0...255>;
    MOVE PAN <-100...100>;
    MOVE TILT <-100...100>;
    MOVE SIDE [CYCLE, CENTERFROMLEFT, RIGHT, CENTERFROMRIGHT, LEFT];
    MOVE PON [UP, HALFDOWN, DOWN, HALFUP];
    MOVE STOP;
    MODE [DANCE, TOUCH];
    MODE TEMPO;
    MODE SLEEP;
```

Strings that the Arduino can send back to you:
```
    BUTTON [DANCE, TOUCH] [OFF, ON]
    BUTTON [HEAD, FRONT, BACK, RIGHT, LEFT] [OFF, ON]
    MOTOR [PAN, TILT, SIDE, PON] FINISHED
    MOTOR [PAN, TILT, SIDE, PON] STALLED
    ENCODER TILT [NOREACH, FORWARD, BACK, UP]
    ENCODER PON [HALFDOWN, UP, DOWN, HALFUP]
    ENCODER SIDE [CENTER, RIGHT, LEFT]
    ENCODER PAN [BACK, RIGHT, LEFT, CENTER]
    EMF [PAN, TILT, PONSIDE] [-127...127]
    POSITION [PAN, TILT, PONSIDE] [VAL]
    AUDIO TEMPO [67, 80, 100, 133, 200] (if BPM cannot be detected, this is estimated from power spectral density response)
    AUDIO MEAN [0...64] (the mean of the envelope over a 1.28sec window, max around 64 for very loud music, not updated when motors are moving)
    AUDIO RANGE [0...64] (dynamic range, max 64 for shouting, not updated when motors moving)
    AUDIO ENVELOPE [0...127] (near instantaneous log of the audio amplitude; commented out in code for reduction of data transfer)
    AUDIO BPM [VAL] (estimated beat interval in multiples of 5msec)
```
