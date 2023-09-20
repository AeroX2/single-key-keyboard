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

#define USBIFACE_INDEX_KEYBOARD 0
#define USBIFACE_INDEX_SERIAL 1

#define HW_CDC_BULK_OUT_SIZE 8
#define HW_CDC_BULK_IN_SIZE 8

#define UART_DEFAULT_BPS 4800

const PROGMEM char usbHidReportDescriptorKeyboard[] = {
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

const PROGMEM char usbDescriptorConfiguration[] = {
    // 9,               /* sizeof(usbDescriptorConfiguration): length of descriptor in bytes */
    // USBDESCR_CONFIG, /* descriptor type */
    // USB_CFG_DESCR_PROPS_CONFIGURATION, 0,
    // // /* total length of data returned (including inlined descriptors) */
    // 2,                         /* number of interfaces in this configuration */
    // 1,                         /* index of this configuration */
    // 0,                         /* configuration name string index */
    // (1 << 7),                  /* attributes */
    // USB_CFG_MAX_BUS_POWER / 2, /* max USB current in 2mA units */
    // // /* interface descriptor follows inline (keyboard): */
    // // 9,                          /* sizeof(usbDescrInterface): length of descriptor in bytes */
    // // USBDESCR_INTERFACE,         /* descriptor type */
    // // USBIFACE_INDEX_KEYBOARD,    /* index of this interface */
    // // 0,                          /* alternate setting for this interface */
    // // 1,                          /* endpoints excl 0: number of endpoint descriptors to follow */
    // // 3,                          /* USB interface class */
    // // USB_CFG_INTERFACE_SUBCLASS, /* USB interface subclass */
    // // USB_CFG_INTERFACE_PROTOCOL, /* USB interface protocol */
    // // 0,                          /* string index for interface */
    // // 9,                          /* sizeof(usbDescrHID): length of descriptor in bytes */
    // // USBDESCR_HID,               /* descriptor type: HID */
    // // 0x01, 0x01,                 /* BCD representation of HID version */
    // // 0x00,                       /* target country code */
    // // 0x01,                       /* number of HID Report (or other HID class) Descriptor infos to
    // //                                follow */
    // // 0x22,                       /* descriptor type: report */
    // // sizeof(usbHidReportDescriptorKeyboard),
    // // 0,                          /* total length of report descriptor */
    // // 7,                          /* sizeof(usbDescrEndpoint) */
    // // USBDESCR_ENDPOINT,          /* descriptor type = endpoint */
    // // (char)0x81,                 /* IN endpoint number 1 */
    // // 0x03,                       /* attrib: Interrupt endpoint */
    // // 8, 0,                       /* maximum packet size */
    // // USB_CFG_INTR_POLL_INTERVAL, /* in ms */
    // /* interface descriptor follows inline (serial): */
    // 9,                     /* sizeof(usbDescrInterface): length of descriptor in bytes */
    // USBDESCR_INTERFACE,    /* descriptor type */
    // 0, // USBIFACE_INDEX_SERIAL, /* index of this interface */
    // 0,                     /* alternate setting for this interface */
    // 1,                     /* endpoints excl 0: number of endpoint descriptors to follow */
    // 2,                     /* USB interface class */
    // 2,                     /* USB interface subclass */
    // 1,                     /* USB interface protocol */
    // 0,                     /* string index for interface */
    // /* CDC Class-Specific descriptor */
    // 5,    /* sizeof(usbDescrCDC_HeaderFn): length of descriptor in bytes */
    // 0x24, /* descriptor type */
    // 0,    /* header functional descriptor */
    // 0x10, 0x01,

    // 4,    /* sizeof(usbDescrCDC_AcmFn): length of descriptor in bytes    */
    // 0x24, /* descriptor type */
    // 2,    /* abstract control management functional descriptor */
    // 0x02, /* SET_LINE_CODING, GET_LINE_CODING, SET_CONTROL_LINE_STATE    */

    // 5,    /* sizeof(usbDescrCDC_UnionFn): length of descriptor in bytes  */
    // 0x24, /* descriptor type */
    // 6,    /* union functional descriptor */
    // 0,    /* CDC_COMM_INTF_ID */
    // 1,    /* CDC_DATA_INTF_ID */

    // 5,    /* sizeof(usbDescrCDC_CallMgtFn): length of descriptor in bytes */
    // 0x24, /* descriptor type */
    // 1,    /* call management functional descriptor */
    // 3,    /* allow management on data interface, handles call management by itself */
    // 1,    /* CDC_DATA_INTF_ID */

    // /* Endpoint Descriptor */
    // 7,                          /* sizeof(usbDescrEndpoint) */
    // USBDESCR_ENDPOINT,          /* descriptor type = endpoint */
    // 0x80 | USB_CFG_EP3_NUMBER,  /* IN endpoint number 3 */
    // 0x03,                       /* attrib: Interrupt endpoint */
    // 8, 0,                       /* maximum packet size */
    // 10, /* in ms */

    // /* Interface Descriptor  */
    // 9,                  /* sizeof(usbDescrInterface): length of descriptor in bytes */
    // USBDESCR_INTERFACE, /* descriptor type */
    // 1,                  /* index of this interface */
    // 0,                  /* alternate setting for this interface */
    // 2,                  /* endpoints excl 0: number of endpoint descriptors to follow */
    // 0x0A,               /* Data Interface Class Codes */
    // 0,
    // 0, /* Data Interface Class Protocol Codes */
    // 0, /* string index for interface */

    // /* Endpoint Descriptor */
    // 7,                       /* sizeof(usbDescrEndpoint) */
    // USBDESCR_ENDPOINT,       /* descriptor type = endpoint */
    // 0x01,                    /* OUT endpoint number 1 */
    // 0x02,                    /* attrib: Bulk endpoint */
    // HW_CDC_BULK_OUT_SIZE, 0, /* maximum packet size */
    // 0,                       /* in ms */

    // /* Endpoint Descriptor */
    // 7,                      /* sizeof(usbDescrEndpoint) */
    // USBDESCR_ENDPOINT,      /* descriptor type = endpoint */
    // 0x81,                   /* IN endpoint number 1 */
    // 0x02,                   /* attrib: Bulk endpoint */
    // HW_CDC_BULK_IN_SIZE, 0,
    // 0,                      /* in ms */

    

  9,          /* sizeof(usbDescrConfig): length of descriptor in bytes */
    USBDESCR_CONFIG,    /* descriptor type */
    48,
    0,          /* total length of data returned (including inlined descriptors) */
    2,          /* number of interfaces in this configuration */
    1,          /* index of this configuration */
    0,          /* configuration name string index */
#if USB_CFG_IS_SELF_POWERED
    (1 << 7) | USBATTR_SELFPOWER,       /* attributes */
#else
    (1 << 7),                           /* attributes */
#endif
    USB_CFG_MAX_BUS_POWER/2,            /* max USB current in 2mA units */

    /* interface descriptor follows inline: */
    9,          /* sizeof(usbDescrInterface): length of descriptor in bytes */
    USBDESCR_INTERFACE, /* descriptor type */
    0,          /* index of this interface */
    0,          /* alternate setting for this interface */
    USB_CFG_HAVE_INTRIN_ENDPOINT,   /* endpoints excl 0: number of endpoint descriptors to follow */
    USB_CFG_INTERFACE_CLASS,
    USB_CFG_INTERFACE_SUBCLASS,
    USB_CFG_INTERFACE_PROTOCOL,
    0,          /* string index for interface */

    /* CDC Class-Specific descriptor */
    // 5,           /* sizeof(usbDescrCDC_HeaderFn): length of descriptor in bytes */
    // 0x24,        /* descriptor type */
    // 0,           /* header functional descriptor */
    // 0x10, 0x01,

    // 4,           /* sizeof(usbDescrCDC_AcmFn): length of descriptor in bytes    */
    // 0x24,        /* descriptor type */
    // 2,           /* abstract control management functional descriptor */
    // 0x02,        /* SET_LINE_CODING, GET_LINE_CODING, SET_CONTROL_LINE_STATE    */

    // 5,           /* sizeof(usbDescrCDC_UnionFn): length of descriptor in bytes  */
    // 0x24,        /* descriptor type */
    // 6,           /* union functional descriptor */
    // 0,           /* CDC_COMM_INTF_ID */
    // 1,           /* CDC_DATA_INTF_ID */

    // 5,           /* sizeof(usbDescrCDC_CallMgtFn): length of descriptor in bytes */
    // 0x24,        /* descriptor type */
    // 1,           /* call management functional descriptor */
    // 3,           /* allow management on data interface, handles call management by itself */
    // 1,           /* CDC_DATA_INTF_ID */

    /* Endpoint Descriptor */
    7,           /* sizeof(usbDescrEndpoint) */
    USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
    0x80|USB_CFG_EP3_NUMBER,        /* IN endpoint number 3 */
    0x03,        /* attrib: Interrupt endpoint */
    8, 0,        /* maximum packet size */
    USB_CFG_INTR_POLL_INTERVAL,        /* in ms */

    /* Interface Descriptor  */
    9,           /* sizeof(usbDescrInterface): length of descriptor in bytes */
    USBDESCR_INTERFACE,           /* descriptor type */
    1,           /* index of this interface */
    0,           /* alternate setting for this interface */
    2,           /* endpoints excl 0: number of endpoint descriptors to follow */
    0x0A,        /* Data Interface Class Codes */
    0,
    0,           /* Data Interface Class Protocol Codes */
    0,           /* string index for interface */

    /* Endpoint Descriptor */
    7,           /* sizeof(usbDescrEndpoint) */
    USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
    0x01,        /* OUT endpoint number 1 */
    0x00,        /* attrib: Bulk endpoint */
    HW_CDC_BULK_OUT_SIZE, 0,        /* maximum packet size */
    0,           /* in ms */

    /* Endpoint Descriptor */
    7,           /* sizeof(usbDescrEndpoint) */
    USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
    0x81,        /* IN endpoint number 1 */
    0x00,        /* attrib: Bulk endpoint */
    HW_CDC_BULK_IN_SIZE, 0,        /* maximum packet size */
    0,           /* in ms */

};

typedef struct keyboard_report_t {
  char modifiers;
  char keypress;
} keyboard_report_t;

keyboard_report_t currentKeyboardReport;
uint8_t keyboardReportDirty;
static uint8_t keyboardBootProtocol = 1;  // 0 = boot protocol, 1 = report protocol
static uint8_t keyboardIdleRate = 125;    // repeat rate for keyboards in 4 ms units
static uint8_t keyboardIdleTime = 0;

enum {
  SEND_ENCAPSULATED_COMMAND = 0,
  GET_ENCAPSULATED_RESPONSE,
  SET_COMM_FEATURE,
  GET_COMM_FEATURE,
  CLEAR_COMM_FEATURE,
  SET_LINE_CODING = 0x20,
  GET_LINE_CODING,
  SET_CONTROL_LINE_STATE,
  SEND_BREAK
};

// typedef struct serial_report_t {
//   char outBuffer[HIDSERIAL_OUTBUFFER_SIZE]
// } serial_report_t;

// serial_report_t currentSerialReport;
// char serialInBuffer[HIDSERIAL_INBUFFER_SIZE];
// bool serialReceived = false;
// char serialBytesRemaining;
// char* serialPos;

bool sendEmptyFrame;
static uchar intr3Status; /* used to control interrupt endpoint transmissions */

static usbWord_t baud;

uint8_t serialReportDirty;
static uint8_t serialBootProtocol = 1;  // 0 = boot protocol, 1 = report protocol
static uint8_t serialIdleRate = 125;    // repeat rate for mouses in 4 ms units
static uint8_t serialIdleTime = 0;

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
  while (1)             // normally solves in about 5 itterations
  {
    int16_t cur =
        usbMeasureFrameLength() - targetValue;  // we expect cur to be negative
                                                // numbers until we overshoot

    int16_t curabs = cur;
    if (curabs < 0) {
      curabs = -curabs;
    }                  // make a positive number, so we know how far away from zero it is
    if (curabs < low)  // current value is lower
    {
      low = curabs;  // got a new lower value, save it
      sav = tmp;     // save the OSC CALB value
    } else           // things just got worse, curabs > low, so saved value before this
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

  currentKeyboardReport.modifiers = modifiers;
  currentKeyboardReport.keypress = keypress;

  usbSetInterrupt(&currentKeyboardReport, sizeof(currentKeyboardReport));
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

void usbKeyboardPrint(char* str) {
  size_t i = 0;
  char c;
  while (c = str[i], c != 0) {
    _writeKey(c);
    i++;
  }
}

void usbKeyboardPrintln(char* str) {
  usbKeyboardPrint(str);
  usbKeyboardPrint("\n");
}

// size_t _writeSerial(const uint8_t* buffer, size_t size) {
//   unsigned char i;
//   while (!usbInterruptIsReady()) {
//     usbPoll();
//     _delay_ms(5);
//   }
//   memset(currentSerialReport.outBuffer, 0, HIDSERIAL_OUTBUFFER_SIZE);
//   for (i = 0; i < size && i < HIDSERIAL_OUTBUFFER_SIZE; i++) {
//     currentSerialReport.outBuffer[i] = buffer[i];
//   }
//   usbSetInterrupt(currentSerialReport.outBuffer, HIDSERIAL_OUTBUFFER_SIZE);
//   return i;
// }

// size_t usbSerialPrint(char* str) {
//   size_t count = 0;
//   size_t i;
//   size_t size = strlen(str);
//   // TODO Append on zeros to last HIDSERIAL_OUTBUFFER_SIZE byte array
//   for (i = 0; i < (size / HIDSERIAL_OUTBUFFER_SIZE) + 1; i++) {
//     count += _writeSerial(str + i * 8, (size < (count + HIDSERIAL_OUTBUFFER_SIZE)) ? (size - count) : HIDSERIAL_OUTBUFFER_SIZE);
//   }
//   return count;
// }

// void usbSerialPrintln(char* str) {
//   usbSerialPrint(str);
//   usbSerialPrint("\n");
// }

// bool usbSerialAvailable() {
//   return serialReceived;
// }

USB_PUBLIC uchar usbFunctionRead(uchar* data, uchar len) {
  data[0] = baud.bytes[0];
  data[1] = baud.bytes[1];
  data[2] = 0;
  data[3] = 0;
  data[4] = 0;
  data[5] = 0;
  data[6] = 8;

  return 7;
}

USB_PUBLIC uchar usbFunctionWrite(uchar* data, uchar len) {
  /*    SET_LINE_CODING    */
  baud.bytes[0] = data[0];
  baud.bytes[1] = data[1];

  // TODO: new baud rate is baud.word;

  return 1;
}

USB_PUBLIC void usbFunctionWriteOut(uchar* data, uchar len) {
  // /*  usb -> rs232c:  transmit char    */
  // for (; len; len--) {
  //   uchar uwnxt;

  //   uwnxt = (uwptr + 1) & TX_MASK;
  //   if (uwnxt != irptr) {
  //     tx_buf[uwptr] = *data++;
  //     uwptr = uwnxt;
  //   }
  // }

  // /*  postpone receiving next data    */
  // if (uartTxBytesFree() <= HW_CDC_BULK_OUT_SIZE)
  //   usbDisableAllRequests();
}

USB_PUBLIC usbMsgLen_t usbFunctionDescriptor(struct usbRequest* rq) {
  // switch {

  // }
  // if (rq->wValue.bytes[1] == USBDESCR_HID) {
  //   /* ugly hardcoding data within usbDescriptorConfiguration */
  //   if (rq->wIndex.word == USBIFACE_INDEX_KEYBOARD) {
  //     usbMsgPtr = (usbMsgPtr_t)&usbDescriptorConfiguration[18]; /* point to the
  //                                                                  HID keyboard
  //                                                                */
  //   }
  // } else if (rq->wValue.bytes[1] == USBDESCR_HID_REPORT) {
  //   if (rq->wIndex.word == USBIFACE_INDEX_KEYBOARD) {
  //     usbMsgPtr = (usbMsgPtr_t)&usbHidReportDescriptorKeyboard[0];
  //     return sizeof(usbHidReportDescriptorKeyboard);
  //   }
  // }

      if(rq->wValue.bytes[1] == USBDESCR_DEVICE){
        usbMsgPtr = (uchar *)usbDescriptorDevice;
        return usbDescriptorDevice[0];
    }else{  /* must be config descriptor */
        usbMsgPtr = (uchar *)usbDescriptorConfiguration;
        return sizeof(usbDescriptorConfiguration);
    }

  return 0;
}

USB_PUBLIC usbMsgLen_t usbFunctionSetup(uchar data[8]) {
  usbRequest_t* rq = (usbRequest_t*)((void*)data);

  if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
    switch (rq->bRequest) {
      // case USBRQ_HID_GET_REPORT: {
      //   // send "no keys pressed" and "no mouse event" if asked here
      //   usbWord_t index = rq->wIndex;
      //   usbMsgPtr = (usbMsgPtr_t)&data[0];  // we only have this one
      //   memset((void*)usbMsgPtr, 0x00, sizeof(data));
      //   return (index.word == USBIFACE_INDEX_SERIAL)
      //              ? sizeof(currentSerialReport)
      //              : sizeof(currentKeyboardReport);
      // }
      // case USBRQ_HID_GET_IDLE: {
      //   // send idle rate to PC as required by spec
      //   usbMsgPtr = (rq->wIndex.word == USBIFACE_INDEX_SERIAL)
      //                   ? (usbMsgPtr_t)&serialIdleRate
      //                   : (usbMsgPtr_t)&keyboardIdleRate;
      //   return 1;
      // }
      // case USBRQ_HID_SET_IDLE: {
      //   // save idle rate as required by spec
      //   if (rq->wIndex.word == USBIFACE_INDEX_SERIAL) {
      //     serialIdleRate = rq->wValue.bytes[1];
      //   } else {
      //     keyboardIdleRate = rq->wValue.bytes[1];
      //   }
      //   return 0;
      // }
      // case USBRQ_HID_GET_PROTOCOL: {
      //   usbMsgPtr = (rq->wIndex.word == USBIFACE_INDEX_SERIAL)
      //                   ? (usbMsgPtr_t)&serialBootProtocol
      //                   : (usbMsgPtr_t)&keyboardBootProtocol;
      //   return 1;
      // }
      // case USBRQ_HID_SET_PROTOCOL: {
      //   // save idle rate as required by spec
      //   if (rq->wIndex.word == USBIFACE_INDEX_SERIAL) {
      //     serialBootProtocol = rq->wValue.bytes[0];
      //   } else {
      //     keyboardBootProtocol = rq->wValue.bytes[0];
      //   }
      //   return 0;
      // }
      case GET_LINE_CODING:
      case SET_LINE_CODING:
        return 0xff;
      case SET_CONTROL_LINE_STATE: {
        if (intr3Status == 0)
          intr3Status = 2;
      }
      default: {
        if ((rq->bmRequestType & USBRQ_DIR_MASK) == USBRQ_DIR_HOST_TO_DEVICE)
          sendEmptyFrame = true;
      }
    }

    return 0;
  }
}

#endif  // __USB_KEYBOARD_OR_SERIAL__
