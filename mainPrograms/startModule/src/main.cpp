/**
 * https://www.instructables.com/ATTiny-Port-Manipulation/
 * https://www.instructables.com/ATtiny85-Interrupt-Barebones-Example/
 * https://en.wikipedia.org/wiki/RC-5
 */


/*#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>*/
#include <Arduino.h>



struct ringBuffer {
private:
  uint16_t      buffer     = 0;
  uint8_t       indexRead  = 0;
  uint8_t       indexWrite = 0;
  const uint8_t bufferSize = 14;

public:
};



void setup() {
  DDRB |= 1 << DDB3;  // Set 3 as output.
  DDRB &= 0 << DDB4;  // Set 4 as input.

  PORTB |= 1 << DDB3;
  PORTB &= 0 << DDB3;
  (PORTB >> DDB3) & 1;
  sei();
}


void loop() {
  delayMicroseconds(1000000);
}
