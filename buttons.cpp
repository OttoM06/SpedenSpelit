#include "buttons.h"
volatile byte first_buttonpressed = 0; //stores the value of first button
volatile byte second_buttonpressed = 0; //stores the value of second button
volatile byte third_buttonpressed = 0; //stores the value of third button
volatile byte fourth_buttonpressed  = 0; //stores the value of fourth button
volatile byte buttonstatus = 0; //stores the current button status

byte buttonpin[]={2,3,4,5}; //stores pin number of the buttons
int buttonstate[4]= {HIGH,HIGH,HIGH,HIGH}; //sets all initial states of all the buttons to HIGH
int lastButtonState[4] = {HIGH,HIGH,HIGH,HIGH}; //sets all the initial previou state of all the buttons to HIGH               
unsigned long lastDebounceTime[4] = {0,0,0,0}; // Sets the initial debounce time of all four buttons to 0
unsigned long debounceDelay = 50; // debounce time 50ms, waits for button state to stabilize before accepting the change
void initButtonsAndButtonInterrupts(void){

  // See requirements for this function from buttons.h
  pinMode(2,INPUT_PULLUP); //sets pin 2 as input with internal pull up resistor
  pinMode(3,INPUT_PULLUP); //sets pin 3 as input with internal pull up resistor
  pinMode(4,INPUT_PULLUP); //sets pin 4 as input with internal pull up resistor
  pinMode(5,INPUT_PULLUP); //sets pin 5 as input with internal pull up resistor

  PCICR |= (1 << PCIE2); // enables set pin-change interrupt for Pins 0-7 on PORT D

  PCMSK2 |= (1 << PCINT18); //sets mask to look for PCINT18(PD2)
  PCMSK2 |= (1 << PCINT19); //sets mask to look for PCINT19(PD3)
  PCMSK2 |= (1 << PCINT20); //sets mask to look for PCINT20(PD4)
  PCMSK2 |= (1 << PCINT21); //sets mask to look for PCINT21(PD5) 
}


ISR(PCINT2_vect) {
   /*
     Here you implement logic for handling
	 interrupts from 2,3,4,5 pins for Game push buttons
	 and for pin 6 for start Game push button.
   */
  buttonstatus = PIND;
  first_buttonpressed  = (buttonstatus & (1 << PD2)) == 0; // checks if D2 is pressed LOW means button pressed
  second_buttonpressed   = (buttonstatus & (1 << PD3)) == 0; // checks if D3 is pressed LOW means button pressed
  third_buttonpressed = (buttonstatus & (1 << PD4)) == 0; // checks if D4 is pressed LOW means button pressed
  fourth_buttonpressed = (buttonstatus & (1 << PD5)) == 0; // checks if D5 is pressed LOW means button pressed
}
void debounce(void){
  for(int i = 0;i < 4;i++) {
    int reading = digitalRead(buttonpin[i]);

    if(reading != lastButtonState[i]){ // != operator checks if current buttonstate is different from previous state
      lastDebounceTime[i] = millis(); //saves the time when buttonstate changed
                                      //saves the button state change time to variable 
    }
    if((millis() - lastDebounceTime[i]) > debounceDelay){ // checks if enoungh time has passed when since buttonstate last changed
      if(reading != buttonState[i]){ // checks if the button state has changed
        buttonState[i] = reading; // saves the current button state buttonsate[i]
        if (buttonState[i] == LOW) { // checks if button is pressed
          Serial.println(buttonpin[i]); //prints the pin number of the pressed to serial monitor
        }
      }
    }
    lastButtonState[i] = reading;//saves the current reading as to previousstate for the next loop 
  }
}

int getButtonState(int buttonIndex){
  if(buttonIndex < 0 || buttonIndex >= 4){ //checks if buttonIndex is smaller than 0 or greater than or equel to 4
    return HIGH; // returns HIGH if button index inavlid
  }
  return buttonState[buttonIndex]; // returns button state
}