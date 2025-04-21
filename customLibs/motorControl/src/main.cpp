#include <Arduino.h>
#include "pins.h"
#include "motor.h"


motor<2>
motorObjLeft(S_PIN_MOTOR_LEFT_PWM, S_PIN_MOTOR_LEFT_IN2, S_PIN_MOTOR_N_SLEEP, S_PIN_ENC_LEFT_A, S_PIN_ENC_LEFT_B, 0, 0);
motor<2>
motorObjRight(S_PIN_MOTOR_RIGHT_PWM, S_PIN_MOTOR_RIGHT_IN2, S_PIN_MOTOR_N_SLEEP, S_PIN_ENC_RIGHT_A, S_PIN_ENC_RIGHT_B, 1, 1);

void setup() {
  motorObjLeft.setPwm();
  motorObjRight;
}

void loop() {}
