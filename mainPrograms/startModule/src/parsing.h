#ifndef _PARSING_H
#define _PARSING_H

#include <Arduino.h>

struct parsed {
  bool    toggle_bit   = 0;
  uint8_t command_type = 0;
  uint8_t id           = 0;
  bool    data         = 0;
};

parsed parse(uint32_t data) {
  parsed parsedObj;
  parsedObj.data         = data & 1;
  parsedObj.id           = (data >> 1) & 0b11111;
  parsedObj.command_type = (data >> 6) & 0b11111;
  parsedObj.toggle_bit   = (data >> 11) & 1;

  return parsedObj;
}

#endif
