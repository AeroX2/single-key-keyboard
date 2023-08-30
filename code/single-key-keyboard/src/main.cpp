#include <Arduino.h>
#include <EEPROM.h>
#include <USBKeyboardOrSerial.h>

#define BUTTON PIN_PA3
#define SWITCH PIN_PA4
#define LED PIN_PB2

#define DEBOUNCE_DELAY 50

bool buttonState;
bool lastButtonState = LOW;
bool keyboardMode = false;

unsigned long lastDebounceTime = 0;
unsigned long lastSerialTime = 0;

char keyboardString[90];

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(SWITCH, INPUT_PULLUP);

  // keyboardMode = digitalRead(SWITCH);

  // if (keyboardMode) {
  //   char c = EEPROM.read(0);
  //   if (c == 0) {
      // strcpy(keyboardString, "Hello World from #club-keyboard!");
  //   } else {
  //     short i = 0;
  //     while (i < 255 && c != 0) {
  //       c = EEPROM.read(i++);
  //       keyboardString[i] = c;
  //     }
  //     keyboardString[i] = 0;
  //   }
  // }

  // USBKeyboardOrSerial.setMode(keyboardMode);
  // USBKeyboardOrSerial.init();
}

void keyboardUpdate() {
  // read the state of the switch into a local variable:
  int reading = digitalRead(BUTTON);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        USBKeyboardOrSerial.sendKeyStroke(0);
        USBKeyboardOrSerial.println(keyboardString);
      }
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
}

void hidSerialUpdate() {
  if (millis() - lastSerialTime > 20000) {
    USBKeyboardOrSerial.print("Current string: ");
    USBKeyboardOrSerial.println(keyboardString);
    USBKeyboardOrSerial.print("Enter a new string: ");
  }

  while (USBKeyboardOrSerial.available()) {
    unsigned char* buffer = new unsigned char[90];
    USBKeyboardOrSerial.read(buffer);

    short c;
    for (c = 0; c < 90 && buffer[c] != 0; c++) {
      EEPROM.write(c, buffer[c]);
    }
    EEPROM.write(c, 0);

    USBKeyboardOrSerial.print("New string is: ");
    USBKeyboardOrSerial.println((char*)buffer);
    USBKeyboardOrSerial.println("Please reboot");
  }

  lastSerialTime = millis();
}

void loop() {
  // It's better to use DigiKeyboard.delay() over the regular Arduino delay()
  // if doing keyboard stuff because it keeps talking to the computer to make
  // sure the computer knows the keyboard is alive and connected
  digitalWrite(LED, HIGH);
  USBKeyboardOrSerial.delay(2500);
  digitalWrite(LED, LOW);
  USBKeyboardOrSerial.delay(2500);

  USBKeyboardOrSerial.sendKeyStroke(0);

  USBKeyboardOrSerial.println("Frog");
}