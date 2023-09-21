/*
 * Based on Obdev's AVRUSB code and under the same license.
 *
 */
#ifndef __USB_KEYBOARD_OR_SERIAL__
#define __USB_KEYBOARD_OR_SERIAL__

#include <avr/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>

#include "keycodes.h"
#include "usbdrv/scancode-ascii-table.h"
#include "usbdrv/usbdrv.h"

#define KEYBOARD_STR_LENGTH 150

const PROGMEM char usbDescriptorHidReport[] = {
    0x05, 0x01,        // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,        // USAGE (Keyboard)
    0xa1, 0x01,        // COLLECTION (Application)
    0x05, 0x07,        //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,        //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,        //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,        //   LOGICAL_MINIMUM (0)
    0x25, 0x01,        //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,        //   REPORT_SIZE (1)
    0x95, 0x08,        //   REPORT_COUNT (8)
    0x81, 0x02,        //   INPUT (Data,Var,Abs)
    0x95, 0x01,        //   REPORT_COUNT (simultaneous keystrokes)
    0x75, 0x08,        //   REPORT_SIZE (8)
    0x25, 0x65,        //   LOGICAL_MAXIMUM (101)
    0x19, 0x00,        //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,        //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,        //   INPUT (Data,Ary,Abs)
    0x95, KEYBOARD_STR_LENGTH,        //   REPORT_COUNT (32)
    0x09, 0x00,        //   USAGE (Undefined)
    0xb2, 0x02, 0x01,  //   FEATURE (Data,Var,Abs,Buf)
    0xc0               // END_COLLECTION
};

typedef struct keyboard_report_t {
  char modifiers;
  char keypress;
} keyboard_report_t;

keyboard_report_t currentReport;

uint8_t idleRate = 500 / 4;  // repeat rate for keyboards in 4 ms units

extern char keyboardString[KEYBOARD_STR_LENGTH];

uchar* stringPos;
uint8_t bytesRemaining;
bool received = false;

static inline void usbCalibrateOsc() {
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

  // 16.5MHz will be 2356 (we want to arrive at this point)
  // 16.0MHz will be 2284 (internal oscillator at 16MHz starts at about this
  // point) 12.8MHz will be 1827 (we want to arrive at this point)
  int16_t targetValue =
      2356;                            //(unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5); //10.5e6 is
                                       // 10500000 //targetValue should be 2356 for 16.5MHz
  uint8_t tmp = CLKCTRL_OSC20MCALIBA;  // oscilator default calibration value

  // trim to 16.5MHz or 12.8MHz
  //  https://www.silabs.com/community/interface/knowledge-base.entry.html/2004/03/15/usb_clock_tolerance-gVai
  //  http://vusb.wikidot.com/examples
  uint8_t sav = tmp;
  int16_t low = 32767;  // lowest saved value
  while (1) {           // normally solves in about 5 itterations
    int16_t cur =
        usbMeasureFrameLength() - targetValue;  // we expect cur to be negative
                                                // numbers until we overshoot

    int16_t curabs = cur;
    if (curabs < 0) {
      curabs = -curabs;
    }

    // make a positive number, so we know how far away from zero it is
    if (curabs < low) {  // current value is lower
      low = curabs;      // got a new lower value, save it
      sav = tmp;         // save the OSC CALB value
    } else {             // things just got worse, curabs > low, so saved value before this was the best value
      _PROTECTED_WRITE(CLKCTRL_OSC20MCALIBA, sav);
      return;
    }

    if (cur < 0) {  // freq too high
      tmp++;
    } else {  //+1 will increase frequency by about 1%
      // freq too low
      tmp--;
    }  //-1 will increase frequency by about 1%
    // apply change and check again
    _PROTECTED_WRITE(CLKCTRL_OSC20MCALIBA, tmp);
  }
}

void usbInitKeyboard() {
  cli();
  usbCalibrateOsc();

  usbDeviceDisconnect();
  _delay_ms(250);
  usbDeviceConnect();

  usbInit();

  sei();
}

// delay while updating until we are finished delaying
void usbDelay(long milli) {
  unsigned long last = millis();
  while (milli > 0) {
    unsigned long now = millis();
    milli -= now - last;
    last = now;
    usbPoll();
  }
}

// sendKeyPress: sends a key press only, with modifiers - no release
// to release the key, send again with keyPress=0
void _sendData(uint8_t keypress, uint8_t modifiers) {
  while (!usbInterruptIsReady()) {
    // Note: We wait until we can send keyPress
    //       so we know the previous keyPress was
    //       sent.
    usbPoll();
    _delay_ms(5);
  }

  currentReport.modifiers = modifiers;
  currentReport.keypress = keypress;

  usbSetInterrupt(&currentReport, sizeof(currentReport));
}

// sendKeyStroke: sends a key press AND release with modifiers
void usbSendKeyStroke(uint8_t keyStroke, uint8_t modifiers) {
  _sendData(keyStroke, modifiers);
  // This stops endlessly repeating keystrokes:
  _sendData(0, 0);
}

size_t _writeKey(uint8_t chr) {
  uint8_t data = (uint8_t)(ascii_to_scan_code_table[chr - 8]);
  usbSendKeyStroke(data & 0b01111111, data >> 7 ? MOD_SHIFT_RIGHT : 0);
  return 1;
}

void usbPrint(char* str) {
  size_t i = 0;
  char c;
  while (c = str[i], c != 0) {
    _writeKey(c);
    i++;
  }
}

USB_PUBLIC uchar usbFunctionWrite(uchar* data, uchar len) {
  received = true;

  if (len > bytesRemaining) len = bytesRemaining;
  bytesRemaining -= len;

  for (int i = 0; i < len; i++) {
    char c = data[i];
    *stringPos++ = c;
    if (c == 0)
      return 1;
  }

  if (bytesRemaining < 0) {
    *stringPos++ = 0;
    return 1;
  }

  return 0;
}

USB_PUBLIC usbMsgLen_t usbFunctionSetup(uchar data[8]) {
  usbRequest_t* rq = (usbRequest_t*)((void*)data);

  if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
    switch (rq->bRequest) {
      case USBRQ_HID_GET_REPORT:
        usbMsgPtr = (usbMsgPtr_t*)&currentReport;
        return sizeof(currentReport);
      case USBRQ_HID_SET_REPORT:
        bytesRemaining = sizeof(keyboardString) - 1;
        stringPos = &keyboardString;
        return USB_NO_MSG;
    }
  }

  return 0;
}

#endif  // __USB_KEYBOARD_OR_SERIAL__
