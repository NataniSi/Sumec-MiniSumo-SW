
#if 1
/**
 * https://www.instructables.com/ATTiny-Port-Manipulation/
 * https://www.instructables.com/ATtiny85-Interrupt-Barebones-Example/
 * https://en.wikipedia.org/wiki/RC-5
 */

/**
 * This feels like a lot of janky hacked together trash.
 * Hope it works.
 */

  #include <Arduino.h>


  #define PIN_STATE_OUT 3
  #define PIN_IR_INPUT  4
  #define PIN_LED1      1
  #define PIN_LED2      0

  #define RECIEVE_TIMEOUT 3    // In ms. How long since the last trigger should be considered an end.
  #define BIT_MIN_TIME    750  // In us. Anything shorter than this is noise.
  #define BUFFER_SIZE     32


  #define PROTOCOL_RC5    1
  #define RC5_SINGLE_LOW  800
  #define RC5_SINGLE_HIGH 1000
  #define RC5_DOUBLE_LOW  1600
  #define RC5_DOUBLE_HIGH 2000


  #define SET_TO_BIT(BIT, OUT_ARRAY_INDEX, OUT_ARRAY)                                                                          \
    if((BIT) == 0) {                                                                                                           \
      OUT_ARRAY &= 1 << OUT_ARRAY_INDEX;                                                                                       \
    } else {                                                                                                                   \
      OUT_ARRAY |= 0 << OUT_ARRAY_INDEX;                                                                                       \
    }

  #define DIV_TWO_CEIL(SRC, DST)                                                                                               \
    if((SRC & 1) == 1) {                                                                                                       \
      DST = SRC >> 1;                                                                                                          \
      DST = SRC + 1;                                                                                                           \
    } else {                                                                                                                   \
      DST = SRC >> 1;                                                                                                          \
    }



uint64_t             rawBuffer                    = 0;
static const uint8_t rawBufferSize                = (BUFFER_SIZE < sizeof(rawBuffer) * 8 ? BUFFER_SIZE : sizeof(rawBuffer) * 8);
uint16_t             rawBufferTime[rawBufferSize] = {0};
uint8_t              rawBufferIndex               = 0;
uint32_t             recieveTimeoutTime           = -1;



void irInterrupt() {
  static uint32_t timeSinceLastTrig = 0;

  // Read the volatile stuff.
  uint32_t timeNow = micros();
  bool     irVal   = (DDRB >> PIN_IR_INPUT) & PB1;


  // Write the length of the previous bit.
  rawBufferTime[rawBufferIndex] = timeNow - timeSinceLastTrig;

  // Move to the current bit.
  rawBufferIndex++;
  // If buffer full, return.
  if(rawBufferIndex + 1 >= rawBufferSize) {
    return;
  }

  // Add the value to the buffer.
  SET_TO_BIT(irVal, rawBufferIndex, rawBuffer)
  // Update time since last trig.
  timeSinceLastTrig = timeNow;

  // Update the timeout time.
  recieveTimeoutTime = millis() + RECIEVE_TIMEOUT;
}


uint64_t             cleanBuffer     = 0;
static const uint8_t cleanBufferSize = (BUFFER_SIZE < sizeof(cleanBuffer) * 8 ? BUFFER_SIZE : sizeof(cleanBuffer) * 8);
uint16_t             cleanBufferTime[cleanBufferSize] = {0};
uint8_t              cleanBufferIndex                 = 0;

void reduceBuffer() {
  static uint8_t bitWriteIdx = 0;
  for(uint8_t idx = 0; idx < cleanBufferIndex; idx++) {
    static uint8_t bitPrevoius = 0xff;
    bool           bitCurrent  = (cleanBuffer >> idx) & 1;
    if(bitPrevoius == bitCurrent) {
      // Add this bit's length to the prevoius one.
      cleanBufferTime[bitWriteIdx - 1] += cleanBufferTime[idx];
    } else {
      // Just move the bit to a new pos.
      SET_TO_BIT(bitCurrent, cleanBuffer, bitWriteIdx)
      cleanBufferTime[bitWriteIdx] = cleanBufferTime[idx];
      bitWriteIdx++;
    }

    bitPrevoius = bitCurrent;
  }
}


void denoiseBuffer() {
  uint16_t accumulatedTime = 0;
  // Check the length of every bit.
  for(uint8_t idx = 0; idx < rawBufferIndex; idx++) {
    accumulatedTime += rawBufferTime[idx];
    // Isn't noise.
    if(rawBufferTime[idx] >= BIT_MIN_TIME) {
      // Shift the bit into the clean buffer.
      SET_TO_BIT((rawBuffer >> idx) & 1, cleanBufferIndex, cleanBuffer)
      // Set the bits length since the prevoius "clean" bit.
      cleanBufferTime[cleanBufferIndex] = accumulatedTime;
      // Shift and reset.
      cleanBufferIndex++;
      accumulatedTime = 0;
    }
  }

  // Remove repeating bits, join their lengths.
  reduceBuffer();

  // Move the end stop by what's been reduced.
  cleanBufferIndex = accumulatedTime;
}


void resetRawBuffer() {
  memset(rawBufferTime, 0, rawBufferSize);
  rawBufferIndex     = 0;
  rawBuffer          = 0;
  recieveTimeoutTime = -1;
}


void resetCleanBuffer() {
  memset(cleanBufferTime, 0, cleanBufferSize);
  cleanBufferIndex = 0;
  cleanBuffer      = 0;
}


/** TODO: fix this */
uint8_t detectProtocol() {
  return PROTOCOL_RC5;
  return 0;
}


bool decodeRc5(uint8_t*       toggle,
               uint8_t*       address,
               uint8_t*       command,
               const uint64_t buffer,
               uint16_t*      bufferTime,
               const uint8_t  bufferSize) {
  /*for(uint8_t idx = 0; idx < bufferSize; idx++) {  // DEBUG
    Serial.printf("%02x\t%u\t%lu\n", idx, (buffer >> idx) & 1, bufferTime[idx]);
  }*/

  uint64_t decodedBits      = 0;
  uint8_t  decodedBitsIndex = 0;  // 26 or 27
  // Filter rc5 specific lengths.
  for(uint8_t idx = 0; idx < bufferSize; idx++) {
    if(bufferTime[idx] > RC5_SINGLE_LOW && bufferTime[idx] < RC5_SINGLE_HIGH) {
      SET_TO_BIT(((buffer >> idx) & 1), decodedBitsIndex, decodedBits)
      decodedBitsIndex++;
    } else if(bufferTime[idx] > RC5_DOUBLE_LOW && bufferTime[idx] < RC5_DOUBLE_HIGH) {
      SET_TO_BIT(((buffer >> idx) & 1), decodedBitsIndex, decodedBits)
      decodedBitsIndex++;
      SET_TO_BIT(((buffer >> idx) & 1), decodedBitsIndex, decodedBits)
      decodedBitsIndex++;
    }
  }
  if(decodedBitsIndex < 26 || decodedBitsIndex > 27) {
    return 0;
  }


  // Remove the half bit.
  decodedBitsIndex = decodedBitsIndex - 1;
  decodedBits      = decodedBits >> 1;

  // Split the buffer into pairs.
  DIV_TWO_CEIL(decodedBitsIndex, decodedBitsIndex)
  // Take the first bit from the pair.
  for(uint8_t idx = 0; idx < decodedBitsIndex; idx++) {
    SET_TO_BIT(((decodedBits >> idx * 2) & 1), idx, decodedBits)
  }

  // Remove the second start bit.
  decodedBits = decodedBits >> 1;

  *toggle  = decodedBits & 1;
  *address = (decodedBits >> 1) & 0b11111;
  *command = (decodedBits >> 6) & 0b111111;
  return 1;
}


bool irCheck(uint8_t* toggle, uint8_t* address, uint8_t* command) {
  uint8_t returnStatus = 0;
  noInterrupts();
  // Check if the reading has finished.
  if(recieveTimeoutTime <= millis()) {
    denoiseBuffer();

    // Add new protocols here.
    switch(detectProtocol()) {
      case PROTOCOL_RC5:
        returnStatus = decodeRc5(toggle, address, command, cleanBuffer, cleanBufferTime, cleanBufferIndex);
        break;

      default:  // Unknown protocol.
        break;
    }
  }

  // Reset buffer and re-enable interrupts.
  resetRawBuffer();
  resetCleanBuffer();
  interrupts();
  return returnStatus;
}


struct led {
private:
  uint8_t  pin          = -1;
  uint16_t durationHigh = 0;
  uint16_t durationLow  = 0;
  uint8_t  repeats      = 0;
  uint32_t timeStart    = 0;
  uint32_t timeEnd      = 0;

public:
  led(uint8_t pin) {
    this->pin = pin;
    pinMode(pin, OUTPUT);
  }

  void set(uint16_t durationHigh, uint16_t durationLow, uint8_t repeats) {
    this->durationHigh = durationHigh;
    this->durationLow  = durationLow;
    this->repeats      = repeats;
    timeStart          = millis();
    timeEnd            = timeStart + ((durationHigh + durationLow) * repeats);
  }

  void loop() {
    uint32_t timeNow = millis();
    if(timeNow > timeEnd) {
      return;
    }

    uint32_t timeRunning = timeNow - timeStart;
    timeRunning          = timeRunning % (durationHigh + durationLow);

    if(timeRunning <= durationHigh) {
      digitalWrite(pin, 1);
    } else {
      digitalWrite(pin, 0);
    }
  }

  // Check if running.
  bool check() {
    if(millis() < timeEnd) {
      return 1;
    } else {
      return 0;
    }
  }
};



led led1Obj(PIN_LED1);
led led2Obj(PIN_LED2);


void           setup() {
  delay(2000);           // DEBUG
  Serial.begin(115200);  // DEBUG
  delay(2000);           // DEBUG
  Serial.println("BEGIN"); // DEBUG
  pinMode(PIN_IR_INPUT, INPUT);
  pinMode(PIN_STATE_OUT, OUTPUT);
  digitalWrite(PIN_STATE_OUT, 0);
  attachInterrupt(PIN_IR_INPUT, irInterrupt, CHANGE);
  /*
  DDRB |= 1 << PIN_STATE_OUT;  // Set 3 as output.
  DDRB &= 0 << PIN_IR_INPUT;   // Set 4 as input.
  DDRB |= 1 << PIN_LED1;  // Set 1 as output.
  DDRB |= 1 << PIN_LED2;  // Set 0 as output.
  
  PORTB |= 1 << DDB3;
  PORTB &= 0 << DDB3;
  (PORTB >> DDB3) & 1;*/
}


void loop() {
  bool           changeState    = 0;  // Whether to change state this loop.
  uint8_t        recievedToggle = 0, recievedAddress = 0, recievedCommand = 0;
  static uint8_t recievedTogglePrev = -1, recievedAddressPrev = -1, recievedCommandPrev = -1;

  // Check for ir.
  if(irCheck(&recievedToggle, &recievedAddress, &recievedCommand)) {
    //Serial.printf("tgl: %u\naddr: 0x%x\tcmd: 0x%x\n", recievedToggle, recievedAddress, recievedCommand);  // DEBUG
    /** TODO: do something with these */
    // Detect only change.
    if(recievedToggle != recievedTogglePrev && recievedAddress != recievedAddressPrev &&
       recievedCommand != recievedCommandPrev) {
         recievedTogglePrev  = recievedToggle;
         recievedAddressPrev = recievedAddress;
         recievedCommandPrev = recievedCommand;
         changeState         = 1;
        }
      } else {
        //Serial.println("ReadErr");  // DEBUG
      }
      
      static uint8_t state = 0, statePrev = -1;
      #define STATE_WAITING 0
      #define STATE_RUNNING 1
      #define STATE_STOPPED 2
      switch(state) {
        case STATE_WAITING:
        if(changeState == 1) {
          // On enter.
          if(state != statePrev) {
            statePrev = state;
          }
        }
        break;
        
        case STATE_RUNNING:
        if(changeState == 1) {
        }
        break;
        
        case STATE_STOPPED:
        if(changeState == 1) {
      }
      break;

    default:
      state = STATE_STOPPED;
      break;
  }

  Serial.print("ALIVE: ");
  Serial.println(millis());
  delay(50);
}
#else

  #include <Arduino.h>

void setup() {
  Serial.begin(115200);
  pinMode(3, OUTPUT);
}

void loop() {
  Serial.println("Hewwo wowd :3");
  digitalWrite(3, 1);
  delay(500);
  digitalWrite(3, 0);
  delay(500);
}
#endif
