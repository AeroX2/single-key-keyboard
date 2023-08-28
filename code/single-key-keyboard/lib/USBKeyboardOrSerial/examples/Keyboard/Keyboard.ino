#include "USBKeyboard.h"

void setup() {
  // don't need to set anything up to use USBKeyboard
}


void loop() {
  // this is generally not necessary but with some older systems it seems to
  // prevent missing the first character after a delay:
  USBKeyboardOrSerial.sendKeyStroke(0);
  
  // Type out this string letter by letter on the computer (assumes US-style
  // keyboard)
  USBKeyboardOrSerial.println("Hello Digispark!");
  
  // It's better to use USBKeyboard.delay() over the regular Arduino delay()
  // if doing keyboard stuff because it keeps talking to the computer to make
  // sure the computer knows the keyboard is alive and connected
  USBKeyboardOrSerial.delay(5000);
}
