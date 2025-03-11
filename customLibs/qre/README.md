# QRE LIB

## Use in code
1) Create the qre object. nVMemoryObj and buttonPin are important the rest is more for debug.
2) Call qre::setup in setup. Pass the qre pins and whether the recalibration should happen.
3) Call qre::read with qre::sensorQre type constant to read from the sensor.

## Use in runtime
1) If the calibration is enabled, perform the calibration.
2) That's it. It should work, returning the value of the floor under each sensor.

## Calibration
1) Wait for blinking binary 1 on the leds or a UDP log message.
> [!NOTE]
> UDP not implemented yet.
2) Press debug button specified by the pin in constructor.
3) Repeat for each sensor, in order left, right, middle. Sensor is also indicated by a binary number displayed by the debug LEDs.
   1) Slowly move the sensor over the blackwhite edge.
   2) Look for a flash on all LEDs or check the UDP log. Two blinks mean success, one if failure due to timeout or cancelation by pressing the button.
