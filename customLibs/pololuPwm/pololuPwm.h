/**
 * Hardware timer based pulse reading lib.
 * Intended for reading pwm pulses, though it can be used for reading length of basically any pulse as long as it's not too long.
 *  - RNS 12025/04/18
 */



 #include <Arduino.h>



 #define PWM_ERROR_OK            0
 #define PWM_ERROR_TIMER_IS_NULL 1
 #define PWM_ERROR_TIMEOUT       2
 
 
 class pwmSensor {
 private:
   hw_timer_t* timerHandle = NULL;
   static void interruptCallback(void* args);
   uint16_t    microsToMm(uint64_t distanceMicros);
 
 public:
   pwmSensor(uint8_t timerChannel);
   uint8_t pwmReadBlocking(const uint8_t pin, uint16_t* const measuredDistance, uint16_t timeout, bool invertTrigger);
 };
 
 
 
 IRAM_ATTR void pwmSensor::interruptCallback(void* args) {
   const uint8_t     pin              = *((uint8_t*)(((size_t*)args)[0]));
   const uint8_t     val              = digitalRead(pin);
   hw_timer_t* const timerHandle      = ((hw_timer_t*)(((size_t*)args)[1]));
   uint16_t* const   measuredDistance = ((uint16_t*)(((size_t*)args)[2]));
   uint8_t* const    readCompleteFlag = ((uint8_t*)(((size_t*)args)[3]));
   uint8_t* const    timerRunning     = ((uint8_t*)(((size_t*)args)[4]));
   const uint8_t     invertTrigger    = *((uint8_t*)(((size_t*)args)[5]));
 
 
   if(val == 1 - invertTrigger) {
     timerStart(timerHandle);  // Start.
     *timerRunning = 1;
   } else {
     if(*timerRunning != 1) {  // Reading not started yet.
       return;
     }
 
     timerStop(timerHandle);                      // Stop.
     *measuredDistance = timerRead(timerHandle);  // Read.
     *readCompleteFlag = 1;
     timerRestart(timerHandle);  // Set to 0.
     detachInterrupt(pin);       // Unattach self.
   }
 }
 
 
 /**
  * @param distanceMicros Measured micros of the pwm.
  * @returns Calculated distance in mm.
  * @brief Converts micros to mm acording to the websites formula.
  */
 uint16_t pwmSensor::microsToMm(uint64_t distanceMicros) {
   return (distanceMicros - 1000) * 2;
 }
 
 
 /**
  * @param timer Id of the hardware timer used. usually between 0 and 4.
  */
 pwmSensor::pwmSensor(uint8_t timerChannel) {
   timerHandle = timerBegin(timerChannel, 80, 1);
 }
 
 
 /**
  * @param pin Gpio to measure pwm on.
  * @param measuredDistance Read distance goes here.
  * @param timeout How long should this wait for a reading. In seconds * 10^-5.
  * @param invertTrigger Whether to trigger on rising and end on falling (0) or trigger on falling and stop on rising (1).
  * @returns Error (0|1|2) - (all oki|timer is null|timeout).
  * @brief Does a single read of the pwm pulse. Blocks until finished(max ~11ms) or after timeout.
  * @note Error 1 might be a wrong timer channel.
  * @note Error 2 might be disconnected sensor.
  * @note "As objects approach the sensor, the output pulse width will approach 1.0 ms, while an object detected at 130 cm will produce a 1.65 ms pulse width. The sensor uses a pulse width of 2.0 ms to indicate no detection." - https://www.pololu.com/product/4071..
  * @note "In our tests, the sensor was able to reliably detect a white sheet of paper out to its maximum range of 130 cm, and it could reliably detect a hand out to around 80 cm away." - https://www.pololu.com/product/4071.
  * @warning PinModes not included.
  */
 uint8_t
 pwmSensor::pwmReadBlocking(const uint8_t pin, uint16_t* const measuredDistance, const uint16_t timeout, bool invertTrigger) {
   if(timerHandle == NULL) {
     return PWM_ERROR_TIMER_IS_NULL;
   }
 
   uint8_t  readCompleteFlag = 0;
   uint8_t  timerRunning     = 0;
   uint64_t measureDistanceMicros;
   size_t   interruptParams[] = {
   (size_t)&pin,          (size_t)timerHandle,   (size_t)&measureDistanceMicros, (size_t)&readCompleteFlag,
   (size_t)&timerRunning, (size_t)&invertTrigger};
   attachInterruptArg(pin, this->interruptCallback, (void*)interruptParams, CHANGE);
 
   for(uint16_t waitLoopIdx = 0; waitLoopIdx < timeout; waitLoopIdx++) {
     if(readCompleteFlag == 1) {
       break;
     }
     delayMicroseconds(10);
   }
 
   if(readCompleteFlag == 1) {
     *measuredDistance = microsToMm(measureDistanceMicros);
     return PWM_ERROR_OK;
   } else {
     return PWM_ERROR_TIMEOUT;
   }
 }
 