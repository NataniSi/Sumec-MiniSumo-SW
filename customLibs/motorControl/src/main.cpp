#include <Arduino.h>
#include <pins.h>


#define MC_FREQ (uint32_t)



class motorCtrl {
private:
const uint32_t freq = 32000;
  bool    invertMotor = 0;
  uint8_t ledcChannel;
  uint8_t pinNSleep;
  uint8_t pinPwm;
  uint8_t pinIn2;
  uint8_t pinEncA;
  uint8_t pinEncB;

public:
  motorCtrl(const bool    invertMotor,
            const uint8_t ledcChannel,
            const uint8_t pinNSleep,
            const uint8_t pinPwm,
            const uint8_t pinIn2,
            const uint8_t pinEncA,
            const uint8_t pinEncB);
  void setup();
  void setPwm(uint8_t pwm, bool dir);
};



motorCtrl::motorCtrl(const bool    invertMotor,
                     const uint8_t ledcChannel,
                     const uint8_t pinNSleep,
                     const uint8_t pinPwm,
                     const uint8_t pinIn2,
                     const uint8_t pinEncA,
                     const uint8_t pinEncB) {
  this->invertMotor = invertMotor;
  this->ledcChannel = ledcChannel;
  this->pinNSleep = pinNSleep;
  this->pinPwm       = pinPwm;
  this->pinIn2       = pinIn2;
  this->pinEncA     = pinEncA;
  this->pinEncB     = pinEncB;
}



void motorCtrl::setup() {
  pinMode(pinEncA, INPUT);
  pinMode(pinEncB, INPUT);

  pinMode(pinNSleep, OUTPUT);
  pinMode(pinPwm, OUTPUT);
  pinMode(pinIn2, OUTPUT);
  digitalWrite(pinNSleep, 1);
  ledcSetup(ledcChannel, freq, 8);
  ledcAttachPin(pinPwm, ledcChannel);
}

void motorCtrl::setPwm(uint8_t pwm, bool dir) {
  ledcWrite(ledcChannel, pwm);
  digitalWrite(pinIn2, dir);
}



#define CHANNEL1 1
#define CHANNEL2 2

//motorCtrl motorCtrlLObj(0, CHANNEL1, S_PIN_MOTOR_N_SLEEP, S_PIN_MOTOR_LEFT_PWM, S_PIN_MOTOR_LEFT_IN2, S_PIN_ENC_LEFT_A, S_PIN_ENC_LEFT_B);
motorCtrl motorCtrlRObj(1, CHANNEL2, S_PIN_MOTOR_N_SLEEP, S_PIN_MOTOR_RIGHT_PWM, S_PIN_MOTOR_RIGHT_IN2, S_PIN_ENC_RIGHT_A, S_PIN_ENC_RIGHT_B);

void setup() {
  motorCtrlRObj.setup();
  motorCtrlRObj.setPwm(16, 0);
}

void loop() {
  delay(1000);
}
