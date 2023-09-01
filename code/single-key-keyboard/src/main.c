// #include <Arduino.h>
// #include <EEPROM.h>
#include <avr/io.h>
#include "USBKeyboardOrSerial/USBKeyboardOrSerial.h"

#define BUTTON PIN3_bm
#define SWITCH PIN4_bm
#define LED PIN2_bm

#define DEBOUNCE_DELAY 50

int buttonState;
int lastButtonState = 0;
// bool keyboardMode = false;

unsigned long lastDebounceTime = 0;

char keyboardString[90];

void setup() {
  // Pull up on button and switch
  PORTB.DIRCLR = BUTTON | SWITCH;
  PORTA.PIN3CTRL = PORT_PULLUPEN_bm;
  PORTA.PIN4CTRL = PORT_PULLUPEN_bm;

  // Set led to output
  PORTB.DIRSET = LED;

  keyboardMode = (~PORTA.IN & SWITCH);
  if (keyboardMode) {
    // uint8_t c = EEPROM.read(0);
    strcpy(keyboardString, "Hello World from #club-keyboard!");
    // if (c == 0xFF) {
    // } else {
    //   // EEPROM.get(0, keyboardString);
    // }
  }

  setMode(keyboardMode);
  init();
}

void keyboardUpdate() {
  // read the state of the switch into a local variable:
  int reading = (~PORTA.IN & BUTTON);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  // if (reading != lastButtonState) {
  //   // reset the debouncing timer
  //   lastDebounceTime = millis();
  // }

  // if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
  //   if (reading != buttonState) {
  //     buttonState = reading;

  //     if (buttonState == LOW) {
        sendKeyStroke(0, 0);
  //       USBKeyboardOrSerial.println(keyboardString);
  //     }
  //   }
  // }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
}

void hidSerialUpdate() {
  // if (millis() - lastSerialTime > 20000) {
  //   USBKeyboardOrSerial.print("Current string: ");
  //   USBKeyboardOrSerial.println(keyboardString);
  //   USBKeyboardOrSerial.print("Enter a new string: ");
  // }

  while (available()) {
    unsigned char* buffer[90];
    read(buffer);

    // EEPROM.put(0, buffer);

  //   USBKeyboardOrSerial.print("New string is: ");
    // USBKeyboardOrSerial.println((char*)buffer);
    strcpy(keyboardString, (char*)buffer);
    // USBKeyboardOrSerial.println("Please reboot");
  }
}

void main(void) __attribute__((noreturn));  void main(void) {
  setup();
  while (1) {
  if (keyboardMode) {
    keyboardUpdate();
  } else {
    hidSerialUpdate();
  }
  }

  // It's better to use DigiKeyboard.delay() over the regular Arduino delay()
  // if doing keyboard stuff because it keeps talking to the computer to make
  // sure the computer knows the keyboard is alive and connected
  // USBKeyboardOrSerial.delay(10);
}