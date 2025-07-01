#ifndef _DECODING_H
#define _DECODING_H
// oki it's 2am, I figured out this way of doing things

// received data
// 00000000000001000010000100001100 => 11{0}00111000100 - stop
// 00000000000000010010000100001110 => 11{#}00111000101 - start
// 00000000000000010011100100001100 => 11{0}01011000100 - set


// 00000000000000010010000100001110 - start
// 00000000000000011001100001100001111110 // duplincate 1
// 011001100001100001111110               // remove leading 0s except one
// 01 10 01 10 00 01 10 00 01 11 11 10    // split to pairs starting at the front
// 01 10 01 10 00 01 10 00 01 11 11 10    // if the last pair is not full, add a 0
// 1 0 1 0 0 1 0 0 1 1 1 0                // remove the first digit of each pair
// 0 1 0 1 0 0 1 0 0 1 1 1                // shift right, this is the "flip mask"
// 1                                      // mark the first digit as 1
// 1 0 0 1 1 1 0 0 0 1 0 1                // each next digit is based on the prevoius one. if the digit's "flip mask" bit is 0 this digit is the same as the prevoius, if it's 1 this digit is flipped
// 100111000101                           // merge them
// 1 00111 00010 1                        // fplit them starting from the right to 1-5-5-1 and discart the rest
// 1 7     2     1
// toggle_bit = 1, command_type = 7, id = 2, data = 1
// robot state, id 2, go

// 00000000000001000010000100001100 - stop
// 0000000000000110000110000110000111100
// 0110000110000110000111100
// 01 10 00 01 10 00 01 10 00 01 11 10 0
// 01 10 00 01 10 00 01 10 00 01 11 10 00
// 1 0 0 1 0 0 1 0 0 1 1 0 0
// 0 1 0 0 1 0 0 1 0 0 1 1 0
// 1
// 1 0 0 0 1 1 1 0 0 0 1 0 0
// 1000111000100
// 0 00111 00010 0
// 0 7     2     0
// toggle_bit = 0, command_type = 7, id = 2, data = 0
// robot state, id 2, stop

// 00000000000000010011100100001100 - set
// 000000000000000110011111100110000111100
// 0110011111100110000111100
// 01 10 01 11 11 10 01 10 00 01 11 10 0
// 01 10 01 11 11 10 01 10 00 01 11 10 00
// 1 0 1 1 1 0 1 0 0 1 1 0 0
// 0 1 0 1 1 1 0 1 0 0 1 1 0
// 1
// 1 0 0 1 0 1 1 0 0 0 1 0 0
// 1001011000100
// 0 01011 00010 0
// toggle_bit = 0, command_type = 11, id = 2, data = 0
// set id, id 2

#include <Arduino.h>


uint16_t decodeTheMess(uint32_t theMess) {
  //Serial.print("theMess:");
  //Serial.println(theMess);  // 270604 - 01000010000100001100 - oki


  uint64_t theBiggerMess = 0;
  for(uint8_t idx = 0; idx < sizeof(theBiggerMess) * 8; idx++) {
    if((theMess >> ((sizeof(theBiggerMess) * 8) - idx - 1)) & 1) {
      theBiggerMess = theBiggerMess << 2;
      theBiggerMess = theBiggerMess | 0b11;
    } else {
      theBiggerMess = theBiggerMess << 1;
    }
  }


  //uint32_t* tempVar1;
  //tempVar1 = (uint32_t*)(&theBiggerMess);
  //Serial.print("theBiggerMess:");
  //Serial.print(tempVar1[1]);  // 0
  //Serial.print(":");
  //Serial.println(tempVar1[0]);  // 12782652 - 0110000110000110000111100 - oki
  //Serial.print(";");


  uint8_t firstDigitOffset = 0;
  for(uint8_t idx = 0; idx < sizeof(theBiggerMess) * 8; idx++) {
    if((theBiggerMess >> (sizeof(theBiggerMess) * 8 - idx - 1)) & 1) {
      firstDigitOffset = (sizeof(theBiggerMess) * 8 - idx - 1) + 1;
      break;
    }
  }


  //Serial.print("offset:");
  //Serial.println(firstDigitOffset);  // 24 - oki


  if(firstDigitOffset % 2 == 0) {
    theBiggerMess = theBiggerMess << 1;  // 01100111111001100001111000
    firstDigitOffset++;
  }
  firstDigitOffset++;  // now this means the number of bits :3.



  uint32_t flipMask = 0;
  for(uint8_t idx = 0; idx < (firstDigitOffset) / 2; idx++) {
    flipMask |= ((theBiggerMess >> (idx * 2)) & 1) << idx;
  }

  flipMask = flipMask >> 1;

  //Serial.print("flipMask:");
  //Serial.println(flipMask);  // 2342 - 0100100100110 - oki



  uint16_t output  = 1;
  bool     lastBit = 1;
  for(uint8_t idx = 0; idx < (firstDigitOffset) / 2 - 1; idx++) {
    output = output << 1;
    if((flipMask >> ((firstDigitOffset / 2 - 1) - idx - 1)) & 1) {
      lastBit = !lastBit;
    }
    output |= lastBit;
  }


  return output;
}

#endif
