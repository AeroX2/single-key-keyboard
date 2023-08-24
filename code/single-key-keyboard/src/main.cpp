#include <Arduino.h>
#include <USBKeyboard.h>

void setup() {
}

void loop() {
  // put your main code here, to run repeatedly:
  // this is generally not necessary but with some older systems it seems to
  // prevent missing the first character after a delay:
  USBKeyboard.sendKeyStroke(0);

  // Type out this string letter by letter on the computer (assumes US-style
  // keyboard)
  USBKeyboard.println("Hello Digispark!");

  // It's better to use DigiKeyboard.delay() over the regular Arduino delay()
  // if doing keyboard stuff because it keeps talking to the computer to make
  // sure the computer knows the keyboard is alive and connected
  USBKeyboard.delay(5000);
}