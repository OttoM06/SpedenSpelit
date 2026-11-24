#include "display.h"
#include "buttons.h"
#include "leds.h"
#include "SpedenSpelit.h"

// meininki

const unsigned long LED_BLANK_TIME = 100;

volatile int buttonNumber = -1;           // for buttons interrupt handler
volatile bool newTimerInterrupt = false;  // for timer interrupt handler
volatile unsigned int isrCallIndex = 0;   // this index is raised every time the isr is called.

static bool ledBlanking = false;
static byte nextLed = 0;
static unsigned long ledBlankTimer = 0;

int numbers[100];                         // array for the numbers that are generated
int score = 0;

volatile bool gameStarted = false;

bool showingResults = false;
unsigned long resultTimer = 0;

void setup() {
  Serial.begin(9600);

  randomSeed(analogRead(0));              // Random seed from analog pin 0s noise.

  // Initialize hardware
  initializeLeds();
  initializeDisplay();
  initButtonsAndButtonInterrupts();

  writeByte(0, true);

  // Timer setup
  noInterrupts();
  initializeTimer();

  interrupts();
}

void loop() {
  if(showingResults && utils::wait(2000, resultTimer)) showingResults = false;
  if(!gameStarted && !showingResults) idle();

  gameInput();

  if(!gameStarted && buttonNumber != -1) startTheGame();
  if(buttonNumber >= 0 && buttonNumber < 4) checkGame(buttonNumber);
  buttonNumber = -1;                          // Reset button number for next cycle

  if(newTimerInterrupt == true) {
    clearAllLeds();

    noInterrupts();
    unsigned int callIndex = isrCallIndex;    // Copy to a variable to avoid reading a broken int if the isr modifies the value while it's being read
    interrupts();

    nextLed = numbers[(callIndex - 1) % 100]; // isr increments isrCallIndex before this is called so the first number is 1.
                                              // Also % 100 to keep the idx between 0,..,99 even if the actual index is way above that

    ledBlankTimer = millis();
    ledBlanking = true;

    newTimerInterrupt = false;
  }

  if(ledBlanking && utils::wait(LED_BLANK_TIME, ledBlankTimer)) {
    setLed(nextLed);
    ledBlanking = false;
  }
}

void initializeTimer(void) {
  TCCR1A = 0;     // Clear these so there are no surprises ;)
  TCCR1B = 0;
  TCNT1 = 0;      // Counter starts from zero. Using CTC mode does this already but there could be some garbage left over.

  /*
    With this formula a timer for a desired frequency can be made easily :P

    OCR = CPU / (PRESCALER * FREQ) - 1 = OUTPUT.

    Since the ATMegaWhatevr in the arduino is 16 Mhz, if that 
    is divided by the prescaler value times 1 (hz) - 1 (because counting starts at zero so at count 15624 there has been 15625 counts...) we get 1 hz.

    So...

    OCR = 16000000 / (1024 * 1) - 1 = 15624.

    If the prescaler value is too small the output can be too large
    to fit into a 16 bit unsigned integer (65500 or somethng) so by using a bigger prescaler that doesn't happen.
    It does affect the resolution of the timer but this game is so slow it doesn't matter... Probably 乁( ˙ ω˙乁).
  */
  OCR1A = 15624;

  TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10); // CTC mode (count up to OCR1A and then reset to 0), prescaler 1024 (CS12 and CS10 set).
  TIMSK1 |= (1 << OCIE1A);                            // no OCIE1A, no isr :P when counter reaches OCR1A the isr func is run.
}

ISR(TIMER1_COMPA_vect) {
  if(gameStarted) {
    isrCallIndex++;

    newTimerInterrupt = true;
    
    if(isrCallIndex % 5 == 0) {
      // Increase speed

      if(OCR1A > 6000) OCR1A -= 1000;   // Clamp the speed-up
    }
  }
}

void gameInput() {
  // Taking input through the serial monitor for testing
  if(Serial.available() > 0) {
    int input = Serial.parseInt();

    if(input <= 4 && input >= 0) {
      buttonNumber = input;

      Serial.print("Pressed: ");
      Serial.println(input);
    }
  }

  debounce();

  // Select the first button that is pressed.
  // Kinda doesn't work if the player presses multiple buttons
  // But that isn't the idea of the game anyways...
  static int previouslyPressed = -1;
  int currentlyPressed = -1;

  for(int i = 0; i < 4; i++) {
    if(getButtonState(i) == LOW) {
      currentlyPressed = i;
      break;
    }
  }

  if(previouslyPressed == -1 && currentlyPressed != -1) {
    buttonNumber = currentlyPressed;
  }

  previouslyPressed = currentlyPressed;
}

void checkGame(byte lastButtonPress) {
  if(!gameStarted) return;

  if(numbers[score] != lastButtonPress) {
    Serial.println("Game over");
    int btnsPressed = score;

    endTheGame(btnsPressed);

  } else {
    score++;

    if(score >= 100) {
      Serial.println("You won!");
      
      endTheGame(100);
      return;
    }

    writeHighAndLowNumber(score / 10, score % 10);

    Serial.print("Score: ");
    Serial.println(score);
  }
}

void initializeGame() {
  clearAllLeds();

  for(int i = 0; i < 100; i++) {
    // Each index is a random number between 0 and 3 which 
    // correspoonds to the leds that are lit up. 0 = A2, 1 = A3, 2 = A4, 3 = A5

    numbers[i] = random(0, 4);
  }

  Serial.println("Game initialized");
}

void startTheGame() {
  initializeGame();

  TCNT1 = 0;

  gameStarted = true;
  showingResults = false;
  newTimerInterrupt = false;
  buttonNumber = -1;
}

// Actually more like reset game state but this name shall do
void endTheGame(int scoreReached) {
  // Reset stuff
  gameStarted = false;
  buttonNumber = -1;
  newTimerInterrupt = false;
  isrCallIndex = 0;
  score = 0;
  OCR1A = 15624;
  TCNT1 = 0;

  // Clear the display and leds
  clearAllLeds();
  writeByte(0, true);

  // Wait before going back to idle so the player sees their score
  // Done as an if statement at the top of loop()
  showResult(scoreReached);
  resultTimer = millis();
  showingResults = true;
}

void idle() {
  static byte step = 0;
  static unsigned long timer = 0;

  printString("speden spelit");

  if(step == 4) {
    if(show2(2)) {
      step = (step + 1) % 7;
    }
    return;
  }

  if(!utils::wait(500, timer)) return;
  clearAllLeds();

  switch(step) {
    case 0: setLed(1); setLed(3); break;
    case 1: setLed(0); setLed(2); break;
    case 2: setLed(1); setLed(3); break;
    case 3: setLed(0); setLed(2); break;
    case 5: clearAllLeds(); break;
    case 6: setAllLeds(); break;
  }

  // Every 500 ms the step is incremented until it reaches 6 when it "goes back" to 0
  step = (step + 1) % 7;
}