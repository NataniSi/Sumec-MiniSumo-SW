/**
 * PINS
 * pin  | gpio  | meta    | other   | i2c   | remote
 * 1    | 5     | reset   | adc0    |       | reset (also known as reset)
 * 2    | 3     | -       | adc3    |       | dataOut
 * 3    | 4     | -       | adc2    |       | dataIn
 * 4    | gnd   | gnd     | -       |       | gnd
 * 5    | 0     | mosi    | pwm0    | sda   | led2 (yellow)
 * 6    | 1     | miso    | pwm1    |       | led1 (blue)
 * 7    | 2     | sck     | adc1    | scl   | 
 * 8    | vcc   | vcc     | -       |       | vcc
 */
/**
 * RC5 DATA
 * 11<toggle_bit:1><command_type:5><id:5><data:1>
 *  - toggle_bit - helps differentiate between an intended and unintended signal interruption
 *  - command_type - either 7 or 11
 *    - 7 - robot state
 *    - 11 - set id
 *  - id - the id to either verify or set, same as the dip switches on the remote
 *  - data - an additional bit that does stuff in pair with the command_type
 *    - command_type == 7 && data == 1 - start
 *    - command_type == 7 && data == 0 - stop
 *    - command_type == 11 - always 0
 */
/**
 * MODULE OUTPUT
 *  - 0 - stop
 *  - 1 - go
 */

/** TODO: Add the led stuff */

#include <Arduino.h>
#include "decoding.h"
#include "parsing.h"

#define PIN_STATE_OUT 3
#define PIN_IR_INPUT  4
#define PIN_LED1      1
#define PIN_LED2      0



enum State { IDLE, READING, DECODING, PARSING, SETTING_OUTPUT };
enum CommandType { ROBOT_STATE = 7, SET_ID = 11 };


#define TX PIN_LED2

#define SHORT_GAP_MAX 1400
#define READ_TIMEOUT  25000

#define LED1_BLINK_TIME 1000
#define LED2_BLINK_TIME 200



void ioInnit() {
  pinMode(PIN_IR_INPUT, INPUT);
  pinMode(PIN_STATE_OUT, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_LED1, OUTPUT);
  digitalWrite(PIN_IR_INPUT, 0);
  digitalWrite(PIN_STATE_OUT, 0);
  digitalWrite(PIN_LED1, 0);
  digitalWrite(PIN_LED2, 0);
}


void setup() {
  ioInnit();
}

void loop() {
  static State    state      = State::IDLE;
  static uint32_t trigMicros = 0, trigMicrosPrev = 0;
  uint32_t        microsNow = 0;
  static uint32_t buffer;
  bool            irVal     = 0;
  static bool     irValPrev = 0;

  static uint16_t decodedBits = 0;
  static parsed   parsedObj;

  static uint8_t id     = 0;
  static bool    output = 0, outputPrev = 0;
  static bool    outputL1 = 0, outputL1Prev = 0;
  static bool    outputL2 = 0, outputL2Prev = 0;

  static uint8_t led2BlinkCount = 0;
  static uint32_t led2BlinkPrev = 0;
  static uint32_t led1BlinkPrev = 0;

  uint32_t millisNow = 0;


  switch(state) {
    case State::IDLE:
      microsNow = micros();
      irVal     = !digitalRead(PIN_IR_INPUT);
      if(irVal == 1) {
        trigMicros = microsNow;
        state      = State::READING;
      }
      break;

    case State::READING:
      microsNow = micros();
      irVal     = !digitalRead(PIN_IR_INPUT);
      if(irVal != irValPrev) {
        irValPrev      = irVal;
        trigMicrosPrev = trigMicros;
        trigMicros     = microsNow;
        if(trigMicros - trigMicrosPrev > SHORT_GAP_MAX) {
          buffer = buffer << 1;
          buffer = buffer + 1;
        } else {
          buffer = buffer << 1;
        }
      } else if(microsNow - trigMicros > READ_TIMEOUT) {
        state = State::DECODING;
      }
      break;

    case State::DECODING:
      decodedBits = decodeTheMess(buffer);
      state       = State::PARSING;
      break;

    case State::PARSING:
      parsedObj = parse(decodedBits);
      state     = State::SETTING_OUTPUT;
      break;

    case State::SETTING_OUTPUT:
      switch(parsedObj.command_type) {
        case CommandType::ROBOT_STATE:
          if(parsedObj.id == id) {
            output = parsedObj.data;
          }
          break;

        case CommandType::SET_ID:
          id = parsedObj.id;
          outputL2 = 0;
          led2BlinkCount = 4;
          break;

        default:
          break;
      }
      buffer = 0;
      state  = State::IDLE;
      break;

    default:
      break;
  }

  millisNow = millis();

  if(led2BlinkCount > 0) {
    if(millisNow - led2BlinkPrev > LED2_BLINK_TIME) {
      led2BlinkPrev = millisNow;
      outputL2 = !outputL2;
      led2BlinkCount--;
    }
  }
  if(millisNow - led1BlinkPrev > LED1_BLINK_TIME) {
    led1BlinkPrev = millisNow;
    outputL1 = !outputL1;
  }


  if(output != outputPrev) {
    outputPrev = output;
    digitalWrite(PIN_STATE_OUT, output);
    outputL2 = output;
  }
  if(outputL1 != outputL1Prev) {
    outputL1Prev = outputL1;
    digitalWrite(PIN_LED1, outputL1);
  }
  if(outputL2 != outputL2Prev) {
    outputL2Prev = outputL2;
    digitalWrite(PIN_LED2, outputL2);
  }
}
