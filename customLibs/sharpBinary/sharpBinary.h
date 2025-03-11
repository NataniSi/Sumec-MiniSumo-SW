/**
 * A lib for the sharp binary sensors.
 * Just a wrapper for arduino digiRead, for now.
 */

#ifndef _SENSOR_BINARY_H
#define _SENSOR_BINARY_H



#define NUM_OF_SENSORS 2



class binarySensor {
private:
  typedef uint8_t sensorBin;
  uint8_t         sensorPins[NUM_OF_SENSORS];

public:
  const sensorBin sensorLeft  = 0;
  const sensorBin sensorRight = 1;
  void            setup(uint8_t pinLeft, uint8_t pinRight);
  bool            read(sensorBin sensor, bool* value);
};



/**
 * @param pinLeft Pin for the left sensor.
 * @param pinRight Pin for the right sensor.
 * @brief Sets up pins and stuff.
 */
void binarySensor::setup(uint8_t pinLeft, uint8_t pinRight) {
  sensorPins[0] = pinLeft;
  sensorPins[1] = pinRight;
  for(uint8_t idx = 0; idx < NUM_OF_SENSORS; idx++) {
    pinMode(sensorPins[idx], INPUT);
  }
}


/**
 * @param sensor Sensor id.
 * @param value (0|1) - (empty space|object).
 *  * @returns 1 for ok, 0 for error.
 * @brief Read the sensors value.
 */
bool binarySensor::read(sensorBin sensor, bool* value) {
  if(value == NULL) {
    return 0;
  }

  if(*value != sensorLeft && *value != sensorRight) {
    return 0;
  }


  *value = digitalRead(sensorPins[sensor]);
  return 1;
}



#undef NUM_OF_SENSORS


#endif
