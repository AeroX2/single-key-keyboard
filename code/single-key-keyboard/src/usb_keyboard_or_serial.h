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

#include "usbdrv/usbdrv.h"
#include "usbdrv/scancode-ascii-table.h"

typedef uint8_t byte;

#define HIDSERIAL_INBUFFER_SIZE 8

static uchar idleRate;  // in 4 ms units

const PROGMEM char
    usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
        /* USB report descriptor */
 0x05, 0x01,                    // USAGE_PAGE (Generic Desktop) 
  0x09, 0x06,                    // USAGE (Keyboard) 
  0xa1, 0x01,                    // COLLECTION (Application) 
  0x05, 0x07,                    //   USAGE_PAGE (Keyboard) 
  0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl) 
  0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI) 
  0x15, 0x00,                    //   LOGICAL_MINIMUM (0) 
  0x25, 0x01,                    //   LOGICAL_MAXIMUM (1) 
  0x75, 0x01,                    //   REPORT_SIZE (1) 
  0x95, 0x08,                    //   REPORT_COUNT (8) 
  0x81, 0x02,                    //   INPUT (Data,Var,Abs) 
  0x95, 0x01,           //   REPORT_COUNT (simultaneous keystrokes) 
  0x75, 0x08,                    //   REPORT_SIZE (8) 
  0x25, 0x65,                    //   LOGICAL_MAXIMUM (101) 
  0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated)) 
  0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application) 
  0x81, 0x00,                    //   INPUT (Data,Ary,Abs) 
  0xc0                           // END_COLLECTION 

        // 0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined 0xFF00)
        // 0x09, 0x01,        // Usage (0x01)
        // 0xA1, 0x01,        // Collection (Application)
        // 0x85, 0x02,  // Report ID
        // 0x15, 0x00,        //   Logical Minimum (0)
        // 0x26, 0xFF, 0x00,  //   Logical Maximum (255)
        // 0x75, 0x08,        //   Report Size (8)
        // 0x95, 0x08,        //   Report Count (8)
        // 0x09, 0x00,        //   Usage (0x00)
        // 0x82, 0x02, 0x01,  //   Input (Data,Var,Abs,No Wrap,Linear,Preferred
        // State,No Null Position,Buffered Bytes) 0x95, 0x08,        //   Report
        // Count (8) 0x09, 0x00,        //   Usage (0x00) 0xB2, 0x02, 0x01,  //
        // Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null
        // Position,Non-volatile,Buffered Bytes) 0xC0,              // End
        // Collection
};

/* Keyboard usage values, see usb.org's HID-usage-tables document, chapter
 * 10 Keyboard/Keypad Page for more codes.
 */
#define MOD_CONTROL_LEFT (1 << 0)
#define MOD_SHIFT_LEFT (1 << 1)
#define MOD_ALT_LEFT (1 << 2)
#define MOD_GUI_LEFT (1 << 3)
#define MOD_CONTROL_RIGHT (1 << 4)
#define MOD_SHIFT_RIGHT (1 << 5)
#define MOD_ALT_RIGHT (1 << 6)
#define MOD_GUI_RIGHT (1 << 7)

#define KEY_A 4
#define KEY_B 5
#define KEY_C 6
#define KEY_D 7
#define KEY_E 8
#define KEY_F 9
#define KEY_G 10
#define KEY_H 11
#define KEY_I 12
#define KEY_J 13
#define KEY_K 14
#define KEY_L 15
#define KEY_M 16
#define KEY_N 17
#define KEY_O 18
#define KEY_P 19
#define KEY_Q 20
#define KEY_R 21
#define KEY_S 22
#define KEY_T 23
#define KEY_U 24
#define KEY_V 25
#define KEY_W 26
#define KEY_X 27
#define KEY_Y 28
#define KEY_Z 29
#define KEY_1 30
#define KEY_2 31
#define KEY_3 32
#define KEY_4 33
#define KEY_5 34
#define KEY_6 35
#define KEY_7 36
#define KEY_8 37
#define KEY_9 38
#define KEY_0 39

#define KEY_ENTER 40

#define KEY_SPACE 44

#define KEY_F1 58
#define KEY_F2 59
#define KEY_F3 60
#define KEY_F4 61
#define KEY_F5 62
#define KEY_F6 63
#define KEY_F7 64
#define KEY_F8 65
#define KEY_F9 66
#define KEY_F10 67
#define KEY_F11 68
#define KEY_F12 69

#define KEY_ARROW_UP 82
#define KEY_ARROW_DOWN 81
#define KEY_ARROW_LEFT 80
#define KEY_ARROW_RIGHT 79

uchar inBuffer[HIDSERIAL_INBUFFER_SIZE];
uchar outBuffer[2];

uchar received = 0;
uchar reportId = 0;
uchar bytesRemaining;
uchar *pos;

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
      2356;  //(unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5); //10.5e6 is
             // 10500000 //targetValue should be 2356 for 16.5MHz
  uint8_t tmp = CLKCTRL_OSC20MCALIBA;  // oscilator default calibration value

  // trim to 16.5MHz or 12.8MHz
  //  https://www.silabs.com/community/interface/knowledge-base.entry.html/2004/03/15/usb_clock_tolerance-gVai
  //  http://vusb.wikidot.com/examples
  uint8_t sav = tmp;
  int16_t low = 32767;  // lowest saved value
  while (1)             // normally solves in about 5 itterations
  {
    int16_t cur =
        usbMeasureFrameLength() -
        targetValue;  // we expect cur to be negative numbers until we overshoot

    int16_t curabs = cur;
    if (curabs < 0) {
      curabs = -curabs;
    }  // make a positive number, so we know how far away from zero it is
    if (curabs < low)  // current value is lower
    {
      low = curabs;  // got a new lower value, save it
      sav = tmp;     // save the OSC CALB value
    } else  // things just got worse, curabs > low, so saved value before this
            // was the best value
    {
      _PROTECTED_WRITE(CLKCTRL_OSC20MCALIBA, sav);
      return;
    }

    if (cur < 0)  // freq too high
    {
      tmp++;
    }     //+1 will increase frequency by about 1%
    else  // freq too low
    {
      tmp--;
    }  //-1 will increase frequency by about 1%
    // apply change and check again
    _PROTECTED_WRITE(CLKCTRL_OSC20MCALIBA, tmp);
  }
}

void usbInitKeyboardSerial() {
  cli();
  usbCalibrateOsc();

  usbDeviceDisconnect();
  _delay_ms(250);
  usbDeviceConnect();

  usbInit();

  sei();

  // TODO: Remove the next two lines once we fix
  //       missing first keystroke bug properly.
  memset(outBuffer, 0, sizeof(outBuffer));
  usbSetInterrupt(outBuffer, sizeof(outBuffer));
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
void _sendData(byte keyPress, byte modifiers) {
  while (!usbInterruptIsReady()) {
    // Note: We wait until we can send keyPress
    //       so we know the previous keyPress was
    //       sent.
    usbPoll();
    _delay_ms(5);
  }

  memset(outBuffer, 0, sizeof(outBuffer));

  outBuffer[0] = modifiers;
  outBuffer[1] = keyPress;

  usbSetInterrupt(outBuffer, sizeof(outBuffer));
}

// sendKeyStroke: sends a key press AND release with modifiers
void usbSendKeyStroke(byte keyStroke, byte modifiers) {
  _sendData(keyStroke, modifiers);
  // This stops endlessly repeating keystrokes:
  _sendData(0, 0);
}

size_t _write(uint8_t chr) {
  uint8_t data = (uint8_t)(ascii_to_scan_code_table[chr - 8]);
  usbSendKeyStroke(data & 0b01111111, data >> 7 ? MOD_SHIFT_RIGHT : 0);
  return 1;
}

size_t usbPrint(char* str) {
  size_t i = 0;
  char c;
  while (c = str[i], c != 0) {
    _write(c);
    i++;
  }
}

size_t usbPrintln(char* str) {
  usbPrint(str);
  usbPrint("\n");
}

// write up to 8 characters
size_t _write8(const uint8_t *buffer, size_t size) {
  unsigned char i;
  while (!usbInterruptIsReady()) {
    usbPoll();
  }
  memset(outBuffer, 0, 8);
  for (i = 0; i < size && i < 8; i++) {
    outBuffer[i] = buffer[i];
  }
  usbSetInterrupt(outBuffer, 8);
  return i;
}

// write a string
size_t writeT(const uint8_t *buffer, size_t size) {
  size_t count = 0;
  unsigned char i;
  for (i = 0; i < (size / 8) + 1; i++) {
    count +=
        _write8(buffer + i * 8, (size < (count + 8)) ? (size - count) : 8);
  }
  return count;
}

uchar available() { return received; }

uchar read(uchar *buffer) {
  if (received == 0) return 0;
  int i;
  for (i = 0; inBuffer[i] != 0 && i < HIDSERIAL_INBUFFER_SIZE; i++) {
    buffer[i] = inBuffer[i];
  }
  inBuffer[0] = 0;
  buffer[i] = 0;
  received = 0;
  return i;
}

USB_PUBLIC uchar usbFunctionWrite(uchar *data, uchar len) {
  if (reportId == 0) {
    int i;
    if (len > bytesRemaining) len = bytesRemaining;
    bytesRemaining -= len;
    // int start = (pos==inBuffer)?1:0;
    for (i = 0; i < len; i++) {
      if (data[i] != 0) {
        *pos++ = data[i];
      }
    }
    if (bytesRemaining == 0) {
      received = 1;
      *pos++ = 0;
      return 1;
    } else {
      return 0;
    }
  }
  return 1;
}

USB_PUBLIC usbMsgLen_t usbFunctionSetup(uchar data[8]) {
  usbRequest_t *rq = (usbRequest_t *)((void *)data);

  usbMsgPtr = outBuffer;
  // reportId = rq->wValue.bytes[0];
  if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
    /* class request type */

    if (rq->bRequest == USBRQ_HID_GET_REPORT) {
      return 0;
    } else if (rq->bRequest == USBRQ_HID_SET_REPORT) {
      /* since we have only one report type, we can ignore the report-ID */
      pos = inBuffer;
      bytesRemaining = rq->wLength.word;
      if (bytesRemaining > sizeof(inBuffer)) bytesRemaining = sizeof(inBuffer);
      return USB_NO_MSG; /* use usbFunctionWrite() to receive data from host
                          */
    } else if (rq->bRequest == USBRQ_HID_GET_IDLE) {
      return 0;
    } else if (rq->bRequest == USBRQ_HID_SET_IDLE) {
      idleRate = rq->wValue.bytes[1];
    }
  } else {
    /* no vendor specific requests implemented */
  }

  return 0;
}

#endif  // __USB_KEYBOARD_OR_SERIAL__
