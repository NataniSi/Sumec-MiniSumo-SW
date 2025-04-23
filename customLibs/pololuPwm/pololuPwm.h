#ifndef _POLOLU_PWM_H
#define _POLOLU_PWM_H

#include <Arduino.h>


#define MAX_SENSORS 3


class pwmSensor {
private:
  uint8_t     pins[MAX_SENSORS];
  static void interruptCallback(void* args);
  uint16_t    microsToMm(int64_t distanceMicros);
  struct pwmSens {
    const uint8_t LEFT = 0, RIGHT = 1, MIDDLE = 2;
  };
  // Each sensors three values are for the rising, falling and a temp storage for the rising edge respectively.
  int64_t measuredTimes[MAX_SENSORS][3] = {0};

public:
  pwmSens sens;
  pwmSensor(const uint8_t* pins, uint8_t pinsSize);
  uint16_t pwmRead(const uint8_t sensor);
};



IRAM_ATTR void pwmSensor::interruptCallback(void* args) {
  const uint8_t sensor        = ((uint8_t)(((size_t*)args)[0]));
  const uint8_t pin           = *((uint8_t*)(((size_t*)args)[1]));
  int64_t**     measuredTimes = ((int64_t**)(((size_t*)args)[2]));

  const uint8_t val = digitalRead(pin);

  if(val == 1) {  // Rising.
    // This will overflow in 292k years, micros() would overflow in about 71 minutes.
    measuredTimes[sensor][3] = esp_timer_get_time();
  } else {                                                // Falling.
    measuredTimes[sensor][0] = measuredTimes[sensor][2];  // Rising time.
    measuredTimes[sensor][1] = esp_timer_get_time();      // Falling time.
  }
}


/**
 * @param distanceMicros Measured micros of the pwm.
 * @returns Calculated distance in mm.
 * @brief Converts micros to mm acording to the websites formula.
 */
uint16_t pwmSensor::microsToMm(int64_t distanceMicros) {
  return (distanceMicros - 1000) * 2;
}


/**
 * @param pins Array of pins in the format left, right, middle.
 * @param pinsSize Length of the pin array.
 * @brief Sets pinmodes, copies pins, attaches callbacks.
 */
pwmSensor::pwmSensor(const uint8_t* pins, uint8_t pinsSize) {
  if(pinsSize > MAX_SENSORS) {
    return;
  }

  for(uint8_t idx = 0; idx < pinsSize; idx++) {
    this->pins[idx] = pins[idx];
    pinMode(pins[idx], INPUT);

    uint32_t args[] = {(uint32_t)idx, (uint32_t)&pins[idx], (uint32_t)measuredTimes[idx]};
    attachInterruptArg(pins[idx], interruptCallback, (void*)args, CHANGE);
  }
}


/**
 * @param sensor Use pwmSensorObj->sens-><SENSOR>.
 * @returns Measured distance in mm or 0xffff if the sensors id is wrong.
 * @brief Reads a distance from the specified sensor and returns it in mm.
 */
uint16_t pwmSensor::pwmRead(const uint8_t sensor) {
  if(sensor >= MAX_SENSORS) {
    return -1;
  }

  return microsToMm(measuredTimes[sensor][1] - measuredTimes[sensor][0]);
}


// Example code.
#if 0
  #define N_OF_SENSORS 3  // Max is 3 for now.
const uint8_t pwmPins[N_OF_SENSORS] = {0xff, 0xff, 0xff};
pwmSensor     pwmSensorObj(pwmPins, N_OF_SENSORS);  // Constructor.
  #undef N_OF_SENSORS     // This macro is no longer needed.

void setup() {
  Serial.begin(115200);
}

void loop() {
  const uint16_t pwmDistanceLeft   = pwmSensorObj.pwmRead(pwmSensorObj.sens.LEFT); // Reading the distance.
  const uint16_t pwmDistanceRight  = pwmSensorObj.pwmRead(pwmSensorObj.sens.RIGHT);
  const uint16_t pwmDistanceMiddle = pwmSensorObj.pwmRead(pwmSensorObj.sens.MIDDLE);
  
  Serial.println("PWM sensors:");
  
  if(pwmDistanceLeft == 0xffff) {  // Check for error. Optional.
    Serial.println("Sensor left - error pwm101 - wrong sensor"); // Here the error has a name so it's easier to share with others.
  } else {
    Serial.print("Left:\t");
    Serial.printf("%05u", pwmDistanceLeft);
    Serial.println("mm");
  }
  
  if(pwmDistanceRight == 0xffff) {
    Serial.println("Sensor right - error pwm101 - wrong sensor");
  } else {
    Serial.print("Right:\t");
    Serial.printf("%05u", pwmDistanceRight);
    Serial.println("mm");
  }
  
  if(pwmDistanceMiddle == 0xffff) {
    Serial.println("Sensor middle - error pwm101 - wrong sensor");
  } else {
    Serial.print("Middle:\t");
    Serial.printf("%05u", pwmDistanceMiddle);
    Serial.println("mm");
  }
  
  Serial.print("\n");  // One more line.
  delay(500);
}
#endif


#undef MAX_SENSORS
#endif
