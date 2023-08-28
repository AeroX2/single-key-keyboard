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

static inline void use16MHzOsc() {
  // setup main clk freq to 16MHz, we trim to 16.5MHz later
  // programming fuses will set BOD, and 16MHz or 20MHz
  // datasheet pg550 BODLEVEL2 only guaranteed to 10MHz
  // use 0x02 (OSCCFG) value must be 0x02, for 16MHz fuse must be 0x01
  _PROTECTED_WRITE(CLKCTRL_MCLKCTRLB, 0x00);
  // no prescaler = 0x00, prescaler enable =
  // CLKCTRL_PEN_bm (default prescaler is div2)
  while ((CLKCTRL_MCLKSTATUS & CLKCTRL_OSC20MS_bm) == 0)
    ;
  // pg88 wait for OSC20MS to become stable

  // VUSB will trim to 16.5MHz
}

void setup() {
  use16MHzOsc();
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
  USBKeyboardOrSerial.sendKeyStroke(0);

  USBKeyboardOrSerial.println("Frog");
  // if (keyboardMode) {
  //   keyboardUpdate();
  // } else {
  //   hidSerialUpdate();
  // }

  // It's better to use DigiKeyboard.delay() over the regular Arduino delay()
  // if doing keyboard stuff because it keeps talking to the computer to make
  // sure the computer knows the keyboard is alive and connected
  USBKeyboardOrSerial.delay(5000);
}