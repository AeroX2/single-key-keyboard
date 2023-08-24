#include <Arduino.h>
#include <USBKeyboard.h>

static inline void use16MHzOsc() {
	//setup main clk freq to 16MHz, we trim to 16.5MHz later
	//programming fuses will set BOD, and 16MHz or 20MHz
	//datasheet pg550 BODLEVEL2 only guaranteed to 10MHz
	//use 0x02 (OSCCFG) value must be 0x02, for 16MHz fuse must be 0x01
	_PROTECTED_WRITE(CLKCTRL_MCLKCTRLB,0x00);            //no prescaler = 0x00, prescaler enable = CLKCTRL_PEN_bm (default prescaler is div2)
	while((CLKCTRL_MCLKSTATUS & CLKCTRL_OSC20MS_bm)==0); //pg88 wait for OSC20MS to become stable	
	
	// VUSB will trim to 16.5MHz	
}

#define BUTTON PIN_PA3
#define SWITCH PIN_PA4
#define LED PIN_PB2

void setup() {
  // use16MHzOsc();
}

void loop() {
  // // put your main code here, to run repeatedly:
  // // this is generally not necessary but with some older systems it seems to
  // // prevent missing the first character after a delay:
  // USBKeyboard.sendKeyStroke(0);

  // // Type out this string letter by letter on the computer (assumes US-style
  // // keyboard)
  // USBKeyboard.println("Hello Digispark!");

  // // It's better to use DigiKeyboard.delay() over the regular Arduino delay()
  // // if doing keyboard stuff because it keeps talking to the computer to make
  // // sure the computer knows the keyboard is alive and connected
  // USBKeyboard.delay(5000);

  digitalWrite(LED, HIGH);
  delay(1000);
  digitalWrite(LED, LOW);
  delay(1000);
}