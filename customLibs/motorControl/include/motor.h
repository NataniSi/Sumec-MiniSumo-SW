/**
 * encoder has 12 edges total per one rotation of the motor (before gearbox).
 */

#include <Arduino.h>

#define LEDC_FREQ 32000

template <uint8_t bufferSize> class motor {
private:
  uint8_t               pinMotorPwm, pinMotorIn2, pinMotorNSleep, pinEncoderA, pinEncoderB, ledcChannel;
  bool                  invert;
  const uint8_t         trigTimeBufferSize         = bufferSize;
  uint64_t              trigTimeBuffer[bufferSize] = {0};
  uint8_t               trigTimeBufferIndex        = 0;
  void                  setupPins();
  static IRAM_ATTR void motor::encoderCallback(void* args);

public:
  motor(const uint8_t pinMotorPwm,
        const uint8_t pinMotorIn2,
        const uint8_t pinMotorNSleep,
        const uint8_t pinEncoderA,
        const uint8_t pinEncoderB,
        const uint8_t ledcChannel,
        const bool    invert);
  void setPwm();
};



/**
 * @brief Sets up hardware i/o.
 */
template <uint8_t bufferSize> void motor<bufferSize>::setupPins() {
  pinMode(pinMotorPwm, OUTPUT);
  pinMode(pinMotorIn2, OUTPUT);
  pinMode(pinMotorNSleep, OUTPUT);
  pinMode(pinEncoderA, INPUT);
  pinMode(pinEncoderB, INPUT);
  ledcSetup(ledcChannel, LEDC_FREQ, 8);
  ledcAttachPin(pinMotorPwm, ledcChannel);
}


/**
 * @param pinMotorPwm 
 * @param pinMotorIn2 
 * @param pinMotorNSleep 
 * @param pinEncoderA 
 * @param pinEncoderB 
 * @param ledcChannel 
 * @param invert 
 * @brief 
 */
template <uint8_t bufferSize>
motor<bufferSize>::motor(const uint8_t pinMotorPwm,
                         const uint8_t pinMotorIn2,
                         const uint8_t pinMotorNSleep,
                         const uint8_t pinEncoderA,
                         const uint8_t pinEncoderB,
                         const uint8_t ledcChannel,
                         const bool    invert) {
  this->pinMotorPwm    = pinMotorPwm;
  this->pinMotorIn2    = pinMotorIn2;
  this->pinMotorNSleep = pinMotorNSleep;
  this->pinEncoderA    = pinEncoderA;
  this->pinEncoderB    = pinEncoderB;
  this->ledcChannel    = ledcChannel;
  this->invert         = invert;

  setupPins();

  uint32_t encCallArgsA[] = {(uint32_t)&pinEncoderA, (uint32_t)};
  uint32_t encCallArgsB[] = {(uint32_t)&pinEncoderB, (uint32_t)};
  attachInterruptArg(pinEncoderA, encoderCallback, (void*)encCallArgsA, CHANGE);
  attachInterruptArg(pinEncoderB, encoderCallback, (void*)encCallArgsB, CHANGE);
}


template <uint8_t bufferSize> IRAM_ATTR void motor<bufferSize>::encoderCallback(void* args) {
  uint8_t pin = *((uint8_t*)(((uint32_t*)args)[0]));

  esp_timer_get_time();
}

#undef LEDC_FREQ
