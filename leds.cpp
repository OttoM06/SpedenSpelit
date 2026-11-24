#include "leds.h"

unsigned long ledTimer = 0;

void initializeLeds() {         //Sets the pins A2-A5 to output
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);

}

void setLed(byte ledNumber) {       //Turns on the designated led
  switch(ledNumber) {
  case 0:
    digitalWrite(A2, HIGH);
    break;
  case 1:
    digitalWrite(A3, HIGH);
    break;
  case 2:
    digitalWrite(A4, HIGH);
    break;
  case 3:
    digitalWrite(A5, HIGH);
    break;
  }

}

void clearAllLeds() {       //Turns off all the leds
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
  digitalWrite(A4, LOW);
  digitalWrite(A5, LOW);

}

void setAllLeds() {         //Turns on all the leds
  digitalWrite(A2, HIGH);
  digitalWrite(A3, HIGH);
  digitalWrite(A4, HIGH);
  digitalWrite(A5, HIGH);
}

//counts from 0 to 15 and shows them as their binary values
void show1() {
        const int pins[4] = {A2, A3, A4, A5};  //index 0 = least significant bit

        for (int n = 0; n < 16; n++) {
            for (int i = 0; i < 4; i++) {
                digitalWrite(pins[i], (n >> i) & 1);
            }
            delay(500);
            // numeb show
        }
    }

//Turns on the leds in order for a set amount of loops
// with the speed increasing at the end of each loop.
bool show2(int rounds) {
  static int round = 0;
  static int led = 0;
  static int delay = 400;
  static bool started = false;

  if(!started) {
    started = true;
    round = 0;
    led = 0;
    delay = 400;
    ledTimer = millis();

    clearAllLeds();
    setLed(led);
    return false;
  }

  if(!utils::wait(delay, ledTimer)) return false;
  led++;

  if(led >= 4) {
    led = 0;
    round++;
    delay = max(delay - 50, 50);

    if(round >= rounds) {
      started = false;
      return true;
    }
  }

  clearAllLeds();
  setLed(led);
  return false;
}