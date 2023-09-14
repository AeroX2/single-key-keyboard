#ifndef __MILLIS_H__
#define __MILLIS_H__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
 
void init_millis();
uint32_t millis();
uint32_t micros();
 
#endif /* TIMER_H_ */
