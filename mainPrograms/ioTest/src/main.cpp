/**
 * This is meant to be use for testing all of our io stuff.
 * TODO:
 * Check for //DEBUG
 * Check for //TODO
 * Test qre(s)
 * Test sharp bin
 * Memory test?
 * 
 * Usefull stuff
 *  - https://en.wikipedia.org/wiki/ANSI_escape_code
 * Not really that usefull stuff
 *  - https://en.wikipedia.org/wiki/Printf
 *  - https://en.wikipedia.org/wiki/C_data_types
 */

#include <Arduino.h>


// What stuff should be tested.
#define TEST_QRE   0
#define TEST_SHARP 0
#define TEST_PWM   1
#define TEST_I2C   1



// Includes
#include "Wire.h"
#include <vector>
#include "..\..\..\ioPins\ioPins.h"                          // Include pins.
#include "..\..\..\customLibs\nVMemory\nVMemory\nVMemory.h"  // Include non-volatile memory.
#if TEST_QRE == 1
  #include "..\..\..\customLibs\qre\qre.h"  // Include qre(s).
#endif
#if TEST_SHARP == 1
  #include "..\..\..\customLibs\sharpBinary\sharpBinary.h"  // Include binary sharp sensor.
#endif
#if TEST_PWM == 1
  #include "..\..\..\customLibs\pololuPwm\pololuPwm.h"  // Include pwm read lib.
  #define PWM_LEFT_CHANNEL  4                           // Define rmt channels from range 4-7(inclusive).
  #define PWM_RIGHT_CHANNEL 5
#endif



// non-volatile memory object.
nVMem nVMemObj;

// Qre object.
#if TEST_QRE == 1
qre qreObj(&nVMemObj, S_PIN_BUTTON_3, S_PIN_LED_1, S_PIN_LED_2, S_PIN_LED_3);
#endif
// Sharp object
#if TEST_SHARP == 1
binarySensor sharpBinObj;
#endif
// Pwm read.
#if TEST_PWM == 1
pwmSensor pwmSensorLeftObj(S_PIN_POL_PWM_LEFT, PWM_LEFT_CHANNEL);  // Create objects.
pwmSensor pwmSensorRightObj(S_PIN_POL_PWM_RIGHT, PWM_RIGHT_CHANNEL);
#endif

uint64_t deltaTime() {
  static uint64_t loopTime[2];
  static uint8_t  loopTimeIdx = 0;
  loopTimeIdx                 = loopTimeIdx ^ 1;
  loopTime[loopTimeIdx]       = millis();
  return (loopTime[loopTimeIdx] - loopTime[loopTimeIdx ^ 1]);
}


void setup() {
  Serial.begin(115200);
  Serial.print("\e[1;1H");  // Set cursor (row,col), (min 1,1).
  Serial.print("\e[2J");    // Clear console.

  Wire.begin(S_PIN_SDA, S_PIN_SCL, I2C_FREQ);  // I2C.
  // non-volatile memory setup.
  nVMemObj.setup();

  // Qre setup.
#if TEST_QRE == 1
  qreObj.setup(S_PIN_QRE_A, S_PIN_QRE_B, S_PIN_QRE_C, true);
#endif
#if TEST_SHARP == 1
  sharpBinObj.setup(S_PIN_SHARP_1, S_PIN_SHARP_2);  //TODO TEST - Sensor pins might be reversed.
#endif
#if TEST_PWM == 1
  pwmSensorLeftObj.pwmSetup();  // Set sensors up.
  pwmSensorRightObj.pwmSetup();
#endif
}


void loop() {
  const char loadingChar[] = {'/', '-', '\\', '|'};
  uint64_t   deltaTimeVar  = deltaTime();

#if TEST_QRE == 1
  bool qreLeft, qreMid, qreRight;
  char qreLeftChar, qreMidChar, qreRightChar;

  if(qreObj.read(qreObj.sensorQre1, &qreLeft) == 1) {  // Set the char to '0' or '1', unless the read fails, then set to '-'.
    // qreLeftChar = qreLeft + '0';  // Not like this.
    qreLeftChar = (qreLeft == 0 ? '0' : '1');
  } else {
    qreLeftChar = '-';
  }
  if(qreObj.read(qreObj.sensorQre2, &qreMid) == 1) {
    qreMidChar = (qreMid == 0 ? '0' : '1');
  } else {
    qreMidChar = '-';
  }
  if(qreObj.read(qreObj.sensorQre3, &qreRight) == 1) {
    qreRightChar = (qreRight == 0 ? '0' : '1');
  } else {
    qreRightChar = '-';
  }
#endif

#if TEST_SHARP == 1
  bool sharpLeft, sharpRight;
  char sharpLeftChar, sharpRightChar;

  if(sharpBinObj.read(sharpBinObj.sensorLeft, &sharpLeft) == 1) {
    sharpLeftChar = (sharpLeft == 0 ? '0' : '1');
  } else {
    sharpLeftChar = '-';
  }
  if(sharpBinObj.read(sharpBinObj.sensorRight, &sharpRight) == 1) {
    sharpRightChar = (sharpRight == 0 ? '0' : '1');
  } else {
    sharpRightChar = '-';
  }
#endif

#if TEST_PWM == 1
  uint16_t retrievedItemLeft  = 0;
  uint16_t retrievedItemRight = 0;
  pwmSensorLeftObj.pwmRead(&retrievedItemLeft);  // Read sensors.
  pwmSensorRightObj.pwmRead(&retrievedItemRight);
  uint32_t pwmDistLeft = pwmSensorLeftObj.pwmToMm(retrievedItemLeft); // Convert to mm.
  uint32_t pwmDistRight = pwmSensorRightObj.pwmToMm(retrievedItemRight);
#endif

#if TEST_I2C == 1
  static std::vector<uint8_t> respondingAdresses;

  for(uint8_t addr = 0; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if(Wire.endTransmission() == 0) {
      respondingAdresses.push_back(addr);
    }
  }
#endif

  Serial.print("\e[1;1H");                                                           // Set cursor (row,col), (min 1,1).
  Serial.printf("Sumec i/o testing pannel: %c", loadingChar[(millis() / 250) % 4]);  // Header text.

  Serial.print("\e[2;1H");
  Serial.printf("Delta time: %04llu\t fps: %.2f", deltaTimeVar, 1.0 / deltaTimeVar * 1000);  // Time per frame and fps.

  Serial.print("\e[3;1H");
  Serial.printf("running for %010lums", millis());  // Time running.

  Serial.print("\e[4;1H");
  Serial.print("Qre: ");  // Qre stuff
#if TESR_QRE == 1
  Serial.printf("(Left|Mid|Right) - (%c|%c|%c)", qreLeftChar, qreMidChar, qreRightChar);
#else
  Serial.print("[DISABLED]");
#endif

  Serial.print("\e[5;1H");
  Serial.print("Sharp binary: ");  // Sharp binary stuff
#if TEST_SHARP == 1
  Serial.printf("(Left|Right) - (%c|%c)", sharpLeftChar, sharpRightChar);
  #else
  Serial.print("[DISABLED]");
  #endif
  
  Serial.print("\e[6;1H");
  Serial.printf("PWM sensor:");  // Pwm sensor stuff.
  #if TEST_PWM == 1
  Serial.printf("(Left|Right) - (%010u|%010u)", pwmDistLeft, pwmDistRight);
  #else
  Serial.print("[DISABLED]");
  #endif


  Serial.print("\e[7;1H");
  Serial.printf("I2C addresses:");  // Connected I2C devices.
  #if TEST_I2C == 1
  for(uint8_t idx = 0; idx < respondingAdresses.size(); idx++) {
    Serial.printf("\t%x", respondingAdresses.at(idx));
  }
  #else
  Serial.print("\t[DISABLED]");
  #endif
  

  delay(20);
}
