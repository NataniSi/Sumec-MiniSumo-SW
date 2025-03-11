/**
 * This is very temporary. The user is the memory.
 */

#ifndef _NVMEMORY_H
#define _NVMEMORY_H


#warning "nVMemory: This version of this lib is temporary."


#include "..\nVMemoryAllocations\nVMemoryAllocations.h" // Include memory adresses.


class nVMem {
private:
public:
  void setup();
  void get16(uint16_t* val, uint16_t address);
  void put16(uint16_t val, uint16_t address);
};


/**
 * @brief Setup.
 */
void nVMem::setup() {
  Serial.begin(115200);
  Serial.println("DEBUG - nVMemory v0.0.1 setup, prepare your notepad.");  //DEBUG
}


/**
 * @param val Place to store the read val.
 * @param address The address to read from.
 * @brief Reads a value(2 bytes) from a non-volatile memory.
 */
void nVMem::get16(uint16_t* val, uint16_t address) {
  Serial.print("DEBUG - Complete this pls: " + String(address) + ": ");  // Request.
  for(uint8_t i = 0; i < Serial.available(); i++) {                      // Flush.
    Serial.read();
  }
  for(uint16_t i = 0; i < 6000; i++) {  //Timeout.
    if(Serial.available() >= 16) {
      goto ReadContinue;
    }
    delay(100);
  }

  return;
ReadContinue:
  *val = 0;  // Flush output.
  for(uint8_t i = 0; i < 16; i++) {
    uint8_t temp = Serial.read();  // Read char
    if(temp == '0') {
      *val = *val << 1;
    } else if(temp == '1') {
      *val = *val << 1;
      *val + 1;
    } else {                // Wrong char, retry.
      get16(val, address);  // Recursion.
      return;
    }
  }
}


/**
 * @param val Val to write.
 * @param address The address to write to.
 * @brief Writes a value(2 bytes) to a non-volatile memory.
 */
void nVMem::put16(uint16_t val, uint16_t address) {
  Serial.print("DEBUG - Remember this pls: " + String(address) + ": ");  // Request.
  for(uint8_t i = 0; i < 16; i++) {                                      // Print.
    Serial.print(((val >> i) & 1) + '0');
  }

  Serial.print("\n");
}



#undef EEPROM_SIZE


#endif
