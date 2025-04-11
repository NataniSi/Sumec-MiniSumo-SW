/**
 * https://www.instructables.com/ATTiny-Port-Manipulation/
 * https://www.instructables.com/ATtiny85-Interrupt-Barebones-Example/
 * https://en.wikipedia.org/wiki/RC-5
 */


/*#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>*/
#include <Arduino.h>


#define PIN_STATE_OUT DDB3
#define PIN_IR_INPUT  DDB4
#define PIN_LED1      DDB1
#define PIN_LED2      DDB0


uint64_t             buffer                 = 0;
static const uint8_t bufferSize             = sizeof(buffer) * 8;
uint32_t             timeBuffer[bufferSize] = {0};
uint32_t             timeSinceLastTrig          = 0;  // Millis.
uint8_t              buffIndex                  = 0;  // Micros.


void interrupt() {
  if(buffIndex + 1 >= bufferSize) {
    return;
  }

  timeSinceLastTrig = millis();
  
  buffIndex++;
}


void setup() {
  cli();
  DDRB |= 1 << PIN_STATE_OUT;  // Set 3 as output.
  DDRB &= 0 << PIN_IR_INPUT;   // Set 4 as input.
#if FINAL == 1
  DDRB |= 1 << PIN_LED1;  // Set 1 as output.
  DDRB |= 1 << PIN_LED2;  // Set 0 as output.
#endif
  attachInterrupt(PIN_IR_INPUT, interrupt, CHANGE);
  /*PORTB |= 1 << DDB3;
  PORTB &= 0 << DDB3;
  (PORTB >> DDB3) & 1;*/
  sei();
}


void loop() {
  delayMicroseconds(1000000);
}
