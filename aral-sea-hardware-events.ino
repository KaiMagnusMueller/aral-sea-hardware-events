/*******************************************************************************

  Bare Conductive HID keyboard demo
  ---------------------------------

  HID_Keyboard.ino - simple MPR121 touch detection to faked keyboard output

  Bare Conductive code written by Stefan Dzisiewski-Smith and Peter Krige.

  This work is licensed under a MIT license https://opensource.org/licenses/MIT

  Copyright (c) 2016, Bare Conductive

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*******************************************************************************/

// compiler error handling
#include "Compiler_Errors.h"

// touch includes
#include <MPR121.h>
#include <MPR121_Datastream.h>
#include <Wire.h>

// keyboard includes
#include <Keyboard.h>

// keyboard variables
char key;
char deactivateKey;

// keyboard behaviour constants
const bool HOLD_KEY = false;  // set this to false if you want to have a single quick keystroke, true means the key is pressed and released when you press and release the electrode respectively
//const char KEY_MAP[12] = {'J', 'U', 'H', 'Y', 'G', 'T', 'F', 'D', 'E', 'S', 'W', 'A'};
// const char KEY_MAP[12] = {KEY_LEFT_ARROW, KEY_RIGHT_ARROW, KEY_UP_ARROW, KEY_DOWN_ARROW, ' ', KEY_ESC, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_PAGE_UP, KEY_PAGE_DOWN};  // more keys at http://arduino.cc/en/Reference/KeyboardModifiers
const char ACTIVATE_KEYS[7] = {'1', '2', '3', '4', '5', '6', '7'};  // for memory game
const char DEACTIVATE_KEYS[7] = {'q', 'w', 'e', 'r', 't', 'z', 'u'};  // for memory game

// touch constants
const uint32_t BAUD_RATE = 9600;
const uint8_t MPR121_ADDR = 0x5C;
const uint8_t MPR121_INT = 4;

// MPR121 datastream behaviour constants
const bool MPR121_DATASTREAM_ENABLE = false;

void setup() {
  Serial.begin(BAUD_RATE);
  pinMode(LED_BUILTIN, OUTPUT);

  if (!MPR121.begin(MPR121_ADDR)) {
    Serial.println("error setting up MPR121");
    switch (MPR121.getError()) {
      case NO_ERROR:
        Serial.println("no error");
        break;
      case ADDRESS_UNKNOWN:
        Serial.println("incorrect address");
        break;
      case READBACK_FAIL:
        Serial.println("readback failure");
        break;
      case OVERCURRENT_FLAG:
        Serial.println("overcurrent on REXT pin");
        break;
      case OUT_OF_RANGE:
        Serial.println("electrode out of range");
        break;
      case NOT_INITED:
        Serial.println("not initialised");
        break;
      default:
        Serial.println("unknown error");
        break;
    }
    while (1);
  }

  MPR121.setInterruptPin(MPR121_INT);


  MPR121.setTouchThreshold(800);
  MPR121.setReleaseThreshold(100);

  MPR121.setFFI(FFI_10);
  MPR121.setSFI(SFI_10);
  MPR121.setGlobalCDT(CDT_4US);  // reasonable for larger capacitances

  digitalWrite(LED_BUILTIN, HIGH);  // switch on user LED while auto calibrating electrodes
  delay(1000);
  MPR121.autoSetElectrodes();  // autoset all electrode settings
  digitalWrite(LED_BUILTIN, LOW);

  Keyboard.begin();


}

char TOUCHES[7] = {'1', '2', '3', '4', '5', '6', '7'};
uint8_t pinStates[7] = {};
uint8_t pinStatesPrev[7] = {};

void loop() {
  MPR121.updateAll();

  //   Serial.println(MPR121.getFilteredData(0));
  //   Serial.println(MPR121.getBaselineData(0));
  //

  for (int i = 0; i < 7; i++) { // check which electrodes were pressed

    key = ACTIVATE_KEYS[i];
    deactivateKey = DEACTIVATE_KEYS[i];


    uint8_t previousValue = pinStatesPrev[i];

    uint8_t currentValue;
    currentValue = MPR121.getFilteredData(i);


    if (currentValue > previousValue + 30) {
      Serial.print(i);
      Serial.print(" was removed with value ");
      Serial.print(currentValue);
      Serial.print(", ");
      Serial.println(previousValue);

      Keyboard.press(deactivateKey);  // press the appropriate key on the "keyboard" output

      if (!HOLD_KEY) {
        Keyboard.release(deactivateKey);  // if we don't want to hold the key, immediately release it
      }
    } else if (currentValue < previousValue - 30) {
      Serial.print(i);
      Serial.print(" was inserted with value ");
      Serial.print(currentValue);
      Serial.print(", ");
      Serial.println(previousValue);

      Keyboard.press(key);  // press the appropriate key on the "keyboard" output

      if (!HOLD_KEY) {
        Keyboard.release(key);  // if we have a new release and we were holding a key, release it
      }
    }

    pinStatesPrev[i] = currentValue;

    uint8_t status;
    status = MPR121.getTouchData(i);
    TOUCHES[i] = status;

    uint8_t state;
    state = MPR121.getFilteredData(i);
    pinStates[i] = state;





//    if (MPR121.isNewTouch(i)) {
//      //digitalWrite(LED_BUILTIN, HIGH);
//      Keyboard.press(key);  // press the appropriate key on the "keyboard" output
//
//      if (!HOLD_KEY) {
//        Keyboard.release(key);  // if we don't want to hold the key, immediately release it
//      }
//    } else {
//      if (MPR121.isNewRelease(i)) {
//        //digitalWrite(LED_BUILTIN, LOW);
//        Keyboard.press(deactivateKey);  // press the appropriate key on the "keyboard" output
//
//        if (!HOLD_KEY) {
//          Keyboard.release(deactivateKey);  // if we have a new release and we were holding a key, release it
//        }
//      }
//    }
  }

//  if (MPR121_DATASTREAM_ENABLE) {
//    MPR121_Datastream.update();
//  }

  // for (int i = 0; i < sizeof(TOUCHES); i++) Serial.print(TOUCHES[i], HEX);
  // Serial.println();
  //
  for (int i = 0; i < sizeof(pinStates); i++) {
    Serial.print(pinStates[i]);
    Serial.print(",");

  }
  Serial.println();

  delay(500);
}
