#include <avr/eeprom.h>
#include <avr/io.h>
#include <stdbool.h>

#include "millis.h"
#include "usb_keyboard.h"

#define BUTTON PIN3_bm
#define SWITCH PIN4_bm
#define LED PIN2_bm

#define DEBOUNCE_DELAY 50

bool lastButtonState = false;
bool buttonState = false;

unsigned long lastDebounceTime = 0;
unsigned long lastSerialTime = 0;

static uchar EEMEM _eepromKeyboardString[KEYBOARD_STR_LENGTH];
uchar keyboardString[KEYBOARD_STR_LENGTH];

void setup() {
  initMillis();

  // Pull up on button and switch
  PORTB.DIRCLR = BUTTON | SWITCH;
  PORTA.PIN3CTRL = PORT_PULLUPEN_bm;
  PORTA.PIN4CTRL = PORT_PULLUPEN_bm;

  // Set led to output
  // PORTB.DIRSET = LED;

  // There is a bug with <avr/eeprom.h> where NVM_STATUS doesn't exist for attiny416.
  while (!bit_is_clear(NVMCTRL.STATUS,NVMCTRL_EEBUSY_bp));

  eeprom_read_block(keyboardString, _eepromKeyboardString, KEYBOARD_STR_LENGTH);
  if (keyboardString[0] == 0 || keyboardString[0] == 255) {
    strcpy(keyboardString,
           "Hello World from #club-keyboard!\n"
           "I'm James Ridey, the maker of this custom circuit\n"
           "To tweak this message, see (github.com/AeroX2/single-key-keyboard)\n");
  }

  usbInitKeyboard();
}

void keyboardUpdate() {
  // read the state of the switch into a local variable:
  bool reading = (~PORTA.IN & BUTTON);
  
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

      if (buttonState == false) {
        usbSendKeyStroke(0, 0);
        usbPrint(keyboardString);
      }
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
}

void eepromUpdate() {
  if (received) {
    received = false;
    eeprom_write_block(keyboardString, _eepromKeyboardString, KEYBOARD_STR_LENGTH);
  }
}

void main(void) {
  setup();
  while (1) {
    // It's better to use DigiKeyboard.delay() over the regular Arduino delay()
    // if doing keyboard stuff because it keeps talking to the computer to make
    // sure the computer knows the keyboard is alive and connected
    usbDelay(50);

    keyboardUpdate();
    eepromUpdate();
  }
}