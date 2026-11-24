#include "display.h"

// The values of the variables below tell us the number of the Arduino digital pin in which the referenced shift register pin is connected to.
uint8_t dataPin = 8; // A variable for the Serial Data Input pin (SER). The next bit value to be written to the shift register is 0 (LOW) or 1 (HIGH).
uint8_t oePin = 9; // A variable for the Output Enable pin (QA-QH). LOW = QA-QH outputs are used, HIGH = QA-QH outputs are not used.
uint8_t latchPin = 10; // A variable for the Register Clock pin (RCLK). LOW -> HIGH = Copy everything inside the shift register to the QA-QH outputs.
uint8_t clockPin = 11; // A variable for the Shift Register Clock pin (SRCLK). LOW -> HIGH = The value determined by the SER pin is moved to the shift register.
uint8_t resetPin = 12; // A variable for the Shift Register Clear pin (SRCLR). LOW = All bits are reset, HIGH = Normal bit activity.

byte displayNumbers[10] = { // A byte type array. Every binary number in it lights up a specific number (0-9) in the segment display.
  B00111111, // 0
  B00000110, // 1
  B01011011, // 2
  B01001111, // 3
  B01100110, // 4
  B01101101, // 5
  B01111101, // 6
  B00000111, // 7
  B01111111, // 8
  B01101111 // 9
};

static const byte chars[][9] {
//char a,b,c,d,e,f,g,dot
  {'a',1,1,1,0,1,1,1,0}, {'b',0,0,1,1,1,1,1,0},
  {'c',0,0,0,1,1,0,1,0}, {'d',0,1,1,1,1,0,1,0},
  {'e',1,0,0,1,1,1,1,0}, {'f',1,0,0,0,1,1,1,0},
  {'g',1,0,1,1,1,1,0,0}, {'h',0,1,1,0,1,1,1,0},
  {'i',0,1,1,0,0,0,0,0}, {'j',0,1,1,1,1,0,0,0},
  {'l',0,0,0,1,1,1,0,0}, {'n',0,0,1,0,1,0,1,0},
  {'o',0,0,1,1,1,0,1,0}, {'p',1,1,0,0,1,1,1,0},
  {'r',0,0,0,0,1,0,1,0}, {'s',1,0,1,1,0,1,1,0},
  {'t',0,0,0,1,1,1,1,0}, {'u',0,0,1,1,1,0,0,0},
  {'y',0,1,1,1,0,1,1,0},
};

void initializeDisplay(void) {
  // Set the states of Arduino digital pins 8-12 to output.
  for (int i = 8; i < 13; i++) {
    pinMode(i, OUTPUT);
  }
}

void writeByte(uint8_t bits,bool last) {
  if (bits > 9) return;

  digitalWrite(oePin, LOW);                     // Enable the output pins that are connected to the display segments.
  digitalWrite(resetPin, HIGH);                 // Set the SRCLR pin state to high so that the bits work correctly.
  digitalWrite(latchPin, LOW);                  // Set the RCLK pin state to low so data can be collected to the register.

  byte value = displayNumbers[bits];            // A byte type variable to store a specific displayNumbers array element.

  // Send the bits of the byte in the MSB order (bits 7...0).
  for (int i = 7; i >= 0; i--) {
    digitalWrite(clockPin, LOW);                // Set the SRCLK pin state to low to wait for the clock pulse.
    digitalWrite(dataPin, (value >> i) & 1);    // Set the SER pin, LOW if (value >> i) is 0 and HIGH if (value >> i) is 1.

    // Set the SRCLK pin state to high: the clock pulse happens and the data determined by
    // the dataPin moves into the shift register on the edge of the clock.
    digitalWrite(clockPin, HIGH);
  }

  // If the last parameter is true, set the RCLK pin state to high to update the QA-QH outputs at the same time.
  if(last) {
    digitalWrite(latchPin, HIGH);
  }
}

void writeHighAndLowNumber(uint8_t tens, uint8_t ones) {
  writeByte(tens, false);
  writeByte(ones, true);
}

static void writeCharByte(byte pattern, bool last) {
  digitalWrite(oePin, LOW);
  digitalWrite(resetPin, HIGH);
  digitalWrite(latchPin, LOW);

  for (int i = 7; i >= 0; i--) {
    digitalWrite(clockPin, LOW);
    digitalWrite(dataPin, (pattern >> i) & 1);
    digitalWrite(clockPin, HIGH);
  }

  if (last) {
    digitalWrite(latchPin, HIGH);
  }
}

bool printChar(char character, bool last) {
  if(character == ' ') {
    writeCharByte(0, last);
    return true;
  }

  for (int i = 0; i < sizeof(chars) / sizeof(chars[0]); i++) {
    if (character != chars[i][0]) continue;

    byte pattern = 0;

    for (int bit = 0; bit < 8; bit++) {
      pattern |= chars[i][bit + 1] << bit;
    }

    writeCharByte(pattern, last);
    return true;
  }

  return false;
}

void printString(const char* str) {
  static unsigned long timer = 0;
  static unsigned int index = 0;
  static const char* previousText = nullptr;
  static bool scrolling = false;

  if(previousText != str) {
    previousText = str;
    index = 0;
    scrolling = strlen(str) > 2;
    timer = millis();
  }

  if(scrolling && utils::wait(500, timer)) {
    timer = millis();
    index++;
  }

  if(index >= strlen(str) + 1) {
    index = 0;
  }

  char first = ' ';
  char second = ' ';

  if(index < strlen(str)) first = str[index];
  if(index + 1 < strlen(str)) second = str[index + 1];

  printChar(first, false);
  printChar(second, true);
}

void showResult(byte number) {
  uint8_t tens = 0;
  uint8_t ones = 0;
  ones = number % 10;     // Take the remainder from the given number when divided by ten and store it to ones.
  tens = number / 10;     // The number is then divided by ten to get the tens.
  Serial.print(tens);
  Serial.println(ones);
  writeHighAndLowNumber(tens, ones);
}