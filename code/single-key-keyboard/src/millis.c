#include "millis.h"

volatile static uint32_t timerMillis;

#define TIMER_TOP (F_CPU / 1000 - 1)  // Overflow after 1 ms

// Initializes the use of the timer functions by setting up the TCA timer.
void init_millis() {
  TCA0.SINGLE.PER = TIMER_TOP;
  TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;  // Enable overflow interrupt
  TCA0.SINGLE.CTRLA =
      TCA_SINGLE_ENABLE_bm;  // Start without prescaler, at full CPU clock speed
}

// TCA overflow handler, called every millisecond.
ISR(TCA0_OVF_vect) {
  timerMillis++;
  // Clear the interrupt flag (to reset TCA0.CNT)
  TCA0.SINGLE.INTFLAGS |= TCA_SINGLE_OVF_bm;
}

// Gets the milliseconds of the current time.
uint32_t millis() {
  uint32_t m;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { m = timerMillis; }
  return m;
}

// Gets the microseconds of the current time.
uint32_t micros() {
  uint32_t ms;
  uint16_t cnt;
  uint8_t flags;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    ms = timerMillis;
    cnt = TCA0.SINGLE.CNT;
    flags = TCA0.SINGLE.INTFLAGS;
  }
  // If the timer has overflowed, and the ISR hasn't run yet (it clears the
  // overflow flag), and CNT is in its first half (read CNT after overflow is
  // more likely than overflow after read CNT), then add 1 ms to compensate that
  // timerMillis wasn't updated yet
  if ((flags & TCA_SINGLE_OVF_bm) && cnt < TIMER_TOP / 2) ms++;
  // First convert milliseconds to microseconds
  // Then add timer value converted to microseconds
  return ms * 1000 + cnt / (F_CPU / 1000000L);
}