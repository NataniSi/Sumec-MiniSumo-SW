/**
 * A lib for the qre line sensing sensors.
 * Uses the LEDs, BUTTON_3 and nVMemory.
 * TODO:
 * check for TODO
 */

#ifndef _QRE_H
#define _QRE_H
#ifdef _NVMEMORY_H



  #define NUM_OF_SENSORS              3     // Number of max sensors.
  #define NUM_OF_LEDS                 3     // Sumec hardware based constant. Number of debug leds.
  #define LIB_ID_QRE                  1     // Library id. Probably won't be implemented anywhere else.
  #define START_RECALIBRATION_TIMEOUT 1000  // Time to confirm the recalibration, else load thresholds from memory. In ms/10.
  #define RECALIBRATION_TIMEOUT       400   // How many samples should be taken for each recalibration.
  #define BLACK_WHITE_MIN_DELTA                                                                                                \
    300  // Minimal difference between black and white samples. Tune this until it's somewhat reliable.
  #define CALIBRATION_BLACK_BIAS            5  // How much closer will the threshold be to the black 0 - 10.
  #define DELAY_BETWEEN_SENSOR_CALIBRATIONS 500



class qre {
private:
  typedef uint8_t sensorQre;
  nVMem*          nVMemObjQre = NULL;
  uint8_t         nOfSensors  = 0;  // Two or three sensor mode. Also used for the re-setup prevention.
  uint8_t         sensorPins[NUM_OF_SENSORS];
  bool            loadThresholds(bool attemptRecalibration);
  uint16_t        thresholdLeft;
  uint16_t        thresholdRight;
  uint16_t        thresholdMid;
  uint16_t        thresholds[NUM_OF_SENSORS] = {thresholdLeft, thresholdRight, thresholdMid};
  uint8_t         buttonPin1;
  uint8_t         ledPins[NUM_OF_LEDS];
  bool            useLeds;
  void            setLedsBinary(uint8_t binNum, bool enable, uint8_t* ledPinsArray, uint8_t ledPinsArrayLen);
  bool            calibrate(uint8_t sensorPin, uint16_t* threshold, uint8_t bias);

public:
  const sensorQre sensorQre1 = 0;
  const sensorQre sensorQre2 = 1;
  const sensorQre sensorQre3 = 2;
  qre(nVMem* nVMemoryObj, uint8_t buttonPin, uint8_t ledPin1, uint8_t ledPin2, uint8_t ledPin3);
  qre(nVMem* nVMemoryObj, uint8_t buttonPin);
  bool setup(uint8_t pinLeft, uint8_t pinRight, bool attemptRecalibration);
  bool setup(uint8_t pinLeft, uint8_t pinRight, uint8_t pinMid, bool attemptRecalibration);
  bool read(const sensorQre sensor, bool* value);
};



/**
 * @param nVMemoryObj Reference to a nVMemory object.
 * @param buttonPin1 Pin for a button.
 * @param ledPin1 Pin for one of the leds.
 * @param ledPin2 Pin for another one of the leds.
 * @param ledPin3 Pin for a third one of the leds.
 * @brief Constructor.
 */
qre::qre(nVMem* nVMemoryObj, uint8_t buttonPin1, uint8_t ledPin1, uint8_t ledPin2, uint8_t ledPin3) {
  if(nVMemoryObj == NULL) {
    return;
  }

  nVMemObjQre      = nVMemoryObj;
  this->buttonPin1 = buttonPin1;
  useLeds          = true;
  ledPins[0]       = ledPin1;
  ledPins[1]       = ledPin2;
  ledPins[2]       = ledPin3;
}


/**
 * @param nVMemoryObj Reference to a nVMemory object.
 * @param buttonPin1 Pin for a button.
 * @param ledPin1 Pin for one of the leds.
 * @param ledPin2 Pin for another one of the leds.
 * @param ledPin3 Pin for a third one of the leds.
 * @brief Constructor.
 */
qre::qre(nVMem* nVMemoryObj, uint8_t buttonPin1) {
  if(nVMemoryObj == NULL) {
    return;
  }

  nVMemObjQre      = nVMemoryObj;
  this->buttonPin1 = buttonPin1;
  useLeds          = false;
}


/**
 * @param pinLeft Pin for the left sensor.
 * @param pinRight Pin for The right sensor.
 * @param attemptRecalibration Attempt to set new thresholds?
 * @returns True if the calibration happened.
 * @brief Setup for just the front sensors.
 */
bool qre::setup(uint8_t pinLeft, uint8_t pinRight, bool attemptRecalibration) {
  if(nVMemObjQre == NULL) {  // No nVMem object.
    return 0;
  }

  if(nOfSensors != 0) {  // Re-setup protection?.
    return 0;
  }

  nOfSensors = 2;
  pinMode(pinLeft, INPUT);
  pinMode(pinRight, INPUT);
  return loadThresholds(attemptRecalibration);  // Obtain thresholds.
}


/**
 * @param pinLeft Pin for the left sensor.
 * @param pinRight Pin for The right sensor.
 * @param pinMid Pin for The middle/rear sensor.
 * @param attemptRecalibration Attempt to set new thresholds?
 * @returns True if the calibration happened.
 * @brief Setup for all 3 sensors.
 */
bool qre::setup(uint8_t pinLeft, uint8_t pinRight, uint8_t pinMid, bool attemptRecalibration) {
  if(nVMemObjQre == NULL) {  // No nVMem object.
    return 0;
  }

  if(nOfSensors != 0) {  // Re-setup protection?.
    return 0;
  }

  nOfSensors = 3;
  pinMode(pinLeft, INPUT);
  pinMode(pinRight, INPUT);
  pinMode(pinMid, INPUT);
  return loadThresholds(attemptRecalibration);  // Obtain thresholds.
}


/**
 * @param attemptRecalibration If true, this will wait a few seconds for a button press and start recalibration.
 * @brief Either loads thresholds from nVMem or recalibrates sensors and saves new thresholds to nVMem.
 */
bool qre::loadThresholds(bool attemptRecalibration) {
  pinMode(S_PIN_BUTTON_3, INPUT);
  bool recalibrate = false;
  if(attemptRecalibration == true) {                             // Attempt calibration.
    for(uint16_t i = 0; i < START_RECALIBRATION_TIMEOUT; i++) {  // Check every 10ms for a button press.
      if(useLeds == true) {
        setLedsBinary(LIB_ID_QRE, (i % 20 < 10), ledPins, NUM_OF_LEDS);  // Blink the number LIB_ID_QRE on the leds every 200ms.
      }
      if(digitalRead(S_PIN_BUTTON_3) == 1) {  // Button pressed.
        recalibrate = true;                   // Set flag.
        break;
      }
      delay(10);
    }
  }

  if(attemptRecalibration != true || recalibrate != true) {  // Don't recalibrate or recalibration begin failed.
    // Load thresholds from nVMem.
    // Load sides.
    (*nVMemObjQre).get16(&thresholdLeft, EEPROM_ADDR_QRE_THRE_LEFT);
    (*nVMemObjQre).get16(&thresholdRight, EEPROM_ADDR_QRE_THRE_RIGHT);
    if(nOfSensors > 2) {  // Load mid sensor.
      (*nVMemObjQre).get16(&thresholdMid, EEPROM_ADDR_QRE_THRE_MID);
    }
  } else {  // Calibrate.

    uint16_t thresholdsNew[NUM_OF_SENSORS] = {0};
    bool     calibrationCanceled           = false;
    for(uint8_t sensorId = 0; sensorId < nOfSensors; sensorId++) {
      if(useLeds == true) {
        setLedsBinary(sensorId, 1, ledPins, NUM_OF_LEDS);
      }
      if(calibrate(sensorPins[sensorId], &thresholdsNew[sensorId], CALIBRATION_BLACK_BIAS) == false) {
        calibrationCanceled = true;
        break;
      } else {
        delay(DELAY_BETWEEN_SENSOR_CALIBRATIONS);
      }
    }

    if(calibrationCanceled == false) {
      for(uint8_t i = 0; i < NUM_OF_SENSORS; i++) {  // Copy values.
        thresholds[i] = thresholdsNew[i];
      }

      // Save sides.
      (*nVMemObjQre).put16(thresholds[0], EEPROM_ADDR_QRE_THRE_LEFT);
      (*nVMemObjQre).put16(thresholds[1], EEPROM_ADDR_QRE_THRE_RIGHT);
      // Save mid sensor.
      if(nOfSensors > 2) {
        (*nVMemObjQre).put16(thresholds[2], EEPROM_ADDR_QRE_THRE_MID);
      }
      if(useLeds == true) {
        setLedsBinary(0, 0, ledPins, NUM_OF_LEDS);
      }
      return 1;
    }
  }
  if(useLeds == true) {
    setLedsBinary(0, 0, ledPins, NUM_OF_LEDS);
  }
  return 0;
}


/**
 * @param sensorPin Pin of the sensor to calibrate.
 * @param threshold A place to save the measured threshold.
 * @param bias  Black bias (0 - 10).
 * @returns True if success, false if canceled r timed out.
 * @brief Continuously takes samples from the sensor and compares each to the previous sample. If the difference is big enough, calculate average of those two values and save that as the threshold.
 */
bool qre::calibrate(uint8_t sensorPin, uint16_t* threshold, uint8_t bias) {
  uint16_t measuredData[2]   = {0};
  uint8_t  measuredDataIdx   = 0;
  int16_t  measuredDataDelta = 0;
  bool     thresholdOptained = false;
  for(uint8_t i = 0; i < RECALIBRATION_TIMEOUT; i++) {      // Timeout n*50ms = ~20s.
    measuredData[measuredDataIdx] = analogRead(sensorPin);  // Take a sample.
    measuredDataDelta             = measuredData[measuredDataIdx] - measuredData[1 - measuredDataIdx];
    if(i != 0) {
      if(measuredDataDelta > BLACK_WHITE_MIN_DELTA) {  // Delta is enough.
        *threshold =
        (bias * measuredData[measuredDataIdx] + (10 - bias) * measuredData[1 - measuredDataIdx]) / 20;  // Set threshold.
        thresholdOptained = true;                                                                       // Set flag.
        break;
      } else if(measuredDataDelta < BLACK_WHITE_MIN_DELTA) {  // Delta is enough, inverted bias.
        *threshold =
        ((10 - bias) * measuredData[measuredDataIdx] + bias * measuredData[1 - measuredDataIdx]) / 20;  // Set threshold.
        thresholdOptained = true;                                                                       // Set flag.
        break;
      }
    }

    measuredDataIdx = measuredDataIdx ^ 1;  // Move index.
    if(digitalRead(buttonPin1) == 1) {      // Cancel.
      thresholdOptained = false;
      break;
    }

    delay(50);  // Take ~20 samples per second.
  }

  if(useLeds == true) {
    if(thresholdOptained == true) {
      setLedsBinary(0xff, 1, ledPins, NUM_OF_LEDS);
      delay(200);
      setLedsBinary(0xff, 0, ledPins, NUM_OF_LEDS);
      delay(200);
    }

    setLedsBinary(0xff, 1, ledPins, NUM_OF_LEDS);
    delay(200);
    setLedsBinary(0xff, 0, ledPins, NUM_OF_LEDS);
  } else {
    /** TODO: use UDP here */
    if(thresholdOptained == true) {
      // Success.
    } else {
      // Failure.
    }
  }

  return thresholdOptained;
}


/**
 * @param binNum Number to display.
 * @param enable Enable for the leds.
 * @param ledPinsArray Pins for the number. LSB last.
 * @param ledPinsArrayLen Number of leds.
 * @brief Displays a binary number on a list of leds.
 */
void qre::setLedsBinary(uint8_t binNum, bool enable, uint8_t* ledPinsArray, uint8_t ledPinsArrayLen) {
  for(uint8_t i = 0; i < ledPinsArrayLen; i++) {
    if(enable == 1) {
      digitalWrite(ledPinsArray[i], (binNum >> (ledPinsArrayLen - i - 1)) & 1);
    } else {
      digitalWrite(ledPinsArray[i], 0);
    }
  }
}


/**
 * @param sensor Sensor id.
 * @param value (0|1) - (black|white).
 * @returns 1 for ok, 0 for error.
 * @brief Measures a value nad compares to a threshold.
 */
bool qre::read(const sensorQre sensor, bool* value) {
  if(nVMemObjQre == NULL) {  // No nVMem object.
    return 0;
  }

  if(sensor > nOfSensors - 1) {  // Value out of bounds. Sensor doesn't exist.
    return 0;
  }

  *value = (analogRead(sensorPins[sensor]) > thresholds[sensor]);  // Read from sensor, compare to threshold, write.
  return 1;
}



  #undef NUM_OF_SENSORS
  #undef NUM_OF_LEDS
  #undef LIB_ID_QRE
  #undef START_RECALIBRATION_TIMEOUT
  #undef RECALIBRATION_TIMEOUT
  #undef BLACK_WHITE_MIN_DELTA
  #undef CALIBRATION_BLACK_BIAS
  #undef DELAY_BETWEEN_SENSOR_CALIBRATIONS


#else
  #error "Requires non-volatile memory (vNMemory)."
#endif
#endif
