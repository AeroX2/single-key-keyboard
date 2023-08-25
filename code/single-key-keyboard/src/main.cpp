#include <Arduino.h>
#include <USBKeyboard.h>

static inline void use16MHzOsc() {
  // setup main clk freq to 16MHz, we trim to 16.5MHz later
  // programming fuses will set BOD, and 16MHz or 20MHz
  // datasheet pg550 BODLEVEL2 only guaranteed to 10MHz
  // use 0x02 (OSCCFG) value must be 0x02, for 16MHz fuse must be 0x01
  _PROTECTED_WRITE(CLKCTRL_MCLKCTRLB,
                   0x00);  // no prescaler = 0x00, prescaler enable =
                           // CLKCTRL_PEN_bm (default prescaler is div2)
  while ((CLKCTRL_MCLKSTATUS & CLKCTRL_OSC20MS_bm) == 0)
    ;  // pg88 wait for OSC20MS to become stable

  // VUSB will trim to 16.5MHz
}

#define BUTTON PIN_PA3
#define SWITCH PIN_PA4
#define LED PIN_PB2

// Variables will change:
int buttonState;            // the current reading from the input pin
int lastButtonState = LOW;  // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an
// int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time;

bool writeMode = false;

USBKeyboardDevice USBKeyboard;
// HIDDevice HIDDevice;

void setup() { 
  use16MHzOsc();
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(SWITCH, INPUT_PULLUP);

  delay(500);

  writeMode = digitalRead(SWITCH);

  if (writeMode) {
    USBKeyboard = USBKeyboardDevice();
  } else {
    // HIDDevice = HIDDevice();
  }
}

void loop() {
  if (writeMode) {
    keyboardUpdate();
  } else {
    hidSerialUpdate();
  } 
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

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        USBKeyboard.sendKeyStroke(0);
        USBKeyboard.println("Hello World!");
      }
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;

  // It's better to use DigiKeyboard.delay() over the regular Arduino delay()
  // if doing keyboard stuff because it keeps talking to the computer to make
  // sure the computer knows the keyboard is alive and connected
  USBKeyboard.delay(50);
}

void hidSerialUpdate() {

}