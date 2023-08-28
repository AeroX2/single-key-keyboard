/*
 * Based on Obdev's AVRUSB code and under the same license.
 *
 * TODO: Make a proper file header. :-)
 * Modified for Digispark by Digistump
 */
#ifndef __USBKeyboardOrSerial_h__
#define __USBKeyboardOrSerial_h__

#include <Arduino.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>

extern "C" {
#include "usbdrv.h"
}
#include "scancode-ascii-table.h"

typedef uint8_t byte;

#define HIDSERIAL_INBUFFER_SIZE 8

static uchar idleRate;  // in 4 ms units

/* We use a simplifed keyboard report descriptor which does not support the
 * boot protocol. We don't allow setting status LEDs and but we do allow
 * simultaneous key presses.
 * The report descriptor has been created with usb.org's "HID Descriptor Tool"
 * which can be downloaded from http://www.usb.org/developers/hidpage/.
 * Redundant entries (such as LOGICAL_MINIMUM and USAGE_PAGE) have been omitted
 * for the second INPUT item.
 */
const PROGMEM char usbHidReportDescriptorKeyboard[35] = {
    /* USB report descriptor */
    0x05, 0x01,  // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,  // USAGE (Keyboard)
    0xa1, 0x01,  // COLLECTION (Application)
    0x05, 0x07,  //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,  //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,  //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,  //   LOGICAL_MINIMUM (0)
    0x25, 0x01,  //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,  //   REPORT_SIZE (1)
    0x95, 0x08,  //   REPORT_COUNT (8)
    0x81, 0x02,  //   INPUT (Data,Var,Abs)
    0x95, 0x01,  //   REPORT_COUNT (simultaneous keystrokes)
    0x75, 0x08,  //   REPORT_SIZE (8)
    0x25, 0x65,  //   LOGICAL_MAXIMUM (101)
    0x19, 0x00,  //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,  //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,  //   INPUT (Data,Ary,Abs)
    0xc0         // END_COLLECTION
};

const PROGMEM char usbHidReportDescriptorSerial[29] = {
    /* USB report descriptor */
    0x06, 0x00,
    0xff,        // USAGE_PAGE (Generic Desktop)
    0x09, 0x01,  // USAGE (Vendor Usage 1)
    0xa1, 0x01,  // COLLECTION (Application)
    0x15, 0x00,  //   LOGICAL_MINIMUM (0)
    0x26, 0xff,
    0x00,        //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,  //   REPORT_SIZE (8)
    0x95, 0x08,  //   REPORT_COUNT (8)
    0x09, 0x00,  //   USAGE (Undefined)
    0x82, 0x02,
    0x01,                           //   INPUT (Data,Var,Abs,Buf)
    0x95, HIDSERIAL_INBUFFER_SIZE,  //   REPORT_COUNT (32)
    0x09, 0x00,                     //   USAGE (Undefined)
    0xb2, 0x02,
    0x01,  //   FEATURE (Data,Var,Abs,Buf)
    0xc0   // END_COLLECTION
};

const PROGMEM char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
    /* USB report descriptor */
    0x05, 0x01,  // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,  // USAGE (Keyboard)
    0xa1, 0x01,  // COLLECTION (Application)
    0x05, 0x07,  //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,  //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,  //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,  //   LOGICAL_MINIMUM (0)
    0x25, 0x01,  //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,  //   REPORT_SIZE (1)
    0x95, 0x08,  //   REPORT_COUNT (8)
    0x81, 0x02,  //   INPUT (Data,Var,Abs)
    0x95, 0x01,  //   REPORT_COUNT (simultaneous keystrokes)
    0x75, 0x08,  //   REPORT_SIZE (8)
    0x25, 0x65,  //   LOGICAL_MAXIMUM (101)
    0x19, 0x00,  //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,  //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,  //   INPUT (Data,Ary,Abs)
    0xc0         // END_COLLECTION
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

class USBKeyboardOrSerialDevice : public Print {
 public:

  void setMode(bool _keyboardMode) { keyboardMode = _keyboardMode; }

  // void init() {
  USBKeyboardOrSerialDevice() {
    // memcpy((void *)usbHidReportDescriptor,
    //        keyboardMode ? usbHidReportDescriptorKeyboard
    //                     : usbHidReportDescriptorSerial,
    //        USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH);

    cli();
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

  void update() { usbPoll(); }

  // delay while updating until we are finished delaying
  void delay(long milli) {
    unsigned long last = millis();
    while (milli > 0) {
      unsigned long now = millis();
      milli -= now - last;
      last = now;
      update();
    }
  }

  // sendKeyStroke: sends a key press AND release
  void sendKeyStroke(byte keyStroke) { sendKeyStroke(keyStroke, 0); }

  // sendKeyStroke: sends a key press AND release with modifiers
  void sendKeyStroke(byte keyStroke, byte modifiers) {
    sendData(keyStroke, modifiers);
    // This stops endlessly repeating keystrokes:
    sendData(0, 0);
  }

  // sendKeyPress: sends a key press only - no release
  // to release the key, send again with keyPress=0
  void sendKeyPress(byte keyPress) { sendData(keyPress, 0); }

  // sendKeyPress: sends a key press only, with modifiers - no release
  // to release the key, send again with keyPress=0
  void sendData(byte keyPress, byte modifiers) {
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

  size_t write(uint8_t chr) {
    if (keyboardMode) {
      uint8_t data = pgm_read_byte_near(ascii_to_scan_code_table + (chr - 8));
      sendData(data & 0b01111111, data >> 7 ? MOD_SHIFT_RIGHT : 0);
      return 1;
    } else {
      sendData(0, chr);
      return 1;
    }
  }

  // write up to 8 characters
  size_t write8(const uint8_t *buffer, size_t size) {
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
  size_t write(const uint8_t *buffer, size_t size) {
    if (keyboardMode) {
      return this->write(buffer, size);
    }

    size_t count = 0;
    unsigned char i;
    for (i = 0; i < (size / 8) + 1; i++) {
      count +=
          write8(buffer + i * 8, (size < (count + 8)) ? (size - count) : 8);
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

  bool keyboardMode = true;

  uchar inBuffer[HIDSERIAL_INBUFFER_SIZE];
  uchar outBuffer[2];

  uchar received = 0;
  uchar reportId = 0;
  uchar bytesRemaining;
  uchar *pos;

  using Print::write;
};

USBKeyboardOrSerialDevice USBKeyboardOrSerial = USBKeyboardOrSerialDevice();

// USB_PUBLIC uchar usbFunctionWrite(uchar *data, uchar len) {
//   if (USBKeyboardOrSerial.reportId == 0) {
//     int i;
//     if (len > USBKeyboardOrSerial.bytesRemaining)
//       len = USBKeyboardOrSerial.bytesRemaining;
//     USBKeyboardOrSerial.bytesRemaining -= len;
//     // int start = (pos==inBuffer)?1:0;
//     for (i = 0; i < len; i++) {
//       if (data[i] != 0) {
//         *USBKeyboardOrSerial.pos++ = data[i];
//       }
//     }
//     if (USBKeyboardOrSerial.bytesRemaining == 0) {
//       USBKeyboardOrSerial.received = 1;
//       *USBKeyboardOrSerial.pos++ = 0;
//       return 1;
//     } else {
//       return 0;
//     }
//   } else {
//     return 1;
//   }
// }

USB_PUBLIC usbMsgLen_t usbFunctionSetup(uchar data[8]) {
  usbRequest_t *rq = (usbRequest_t *)((void *)data);

  usbMsgPtr = USBKeyboardOrSerial.outBuffer;
  USBKeyboardOrSerial.reportId = rq->wValue.bytes[0];
  if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
    /* class request type */

    if (rq->bRequest == USBRQ_HID_GET_REPORT) {
      return 0;
    } else if (rq->bRequest == USBRQ_HID_SET_REPORT) {
      // /* since we have only one report type, we can ignore the report-ID */
      // USBKeyboardOrSerial.pos = USBKeyboardOrSerial.inBuffer;
      // USBKeyboardOrSerial.bytesRemaining = rq->wLength.word;
      // if (USBKeyboardOrSerial.bytesRemaining >
      //     sizeof(USBKeyboardOrSerial.inBuffer))
      //   USBKeyboardOrSerial.bytesRemaining =
      //       sizeof(USBKeyboardOrSerial.inBuffer);
      // return USB_NO_MSG; /* use usbFunctionWrite() to receive data from host */
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

#endif  // __USBKeyboard_h__
