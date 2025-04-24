/**
 * GPIOs for the SUMEC MK5.
 */

#define S_PIN_LED_1 01
#define S_PIN_LED_2 37
#define S_PIN_LED_3 44
#define S_PIN_LED_4 36

#define S_PIN_BUTTON_3      04
#define S_PIN_BUTTON_3_MODE INPUT_PULLUP
#define S_PIN_BUTTON_4      05
#define S_PIN_BUTTON_4_MODE INPUT_PULLUP


#define S_PIN_QRE_L 14
#define S_PIN_QRE_R 13
#define S_PIN_QRE_B 16

#define S_PIN_SHARP_L 38
#define S_PIN_SHARP_R 10


// Will be 35 after the pcb patch.
#define S_PIN_POL_PWM_L 11
#define S_PIN_POL_PWM_R 12
// Small mistakes were made this is supposed to be 11 but the left one is missing it's pin and it's moved like this.
#define S_PIN_POL_PWM_M 0xff


#define S_PIN_SDA 42
#define S_PIN_SCL 41


#define S_PIN_IR_MODULE 37


#define S_PIN_ENC_LEFT_A 0xff
#define S_PIN_ENC_LEFT_B 0xff

#define S_PIN_ENC_RIGHT_A 0xff
#define S_PIN_ENC_RIGHT_B 0xff

#define S_PIN_MOTOR_N_SLEEP  17
// Pwm.
#define S_PIN_MOTOR_LEFT_EN  07
// In2.
#define S_PIN_MOTOR_LEFT_PH  06
// Pwm.
#define S_PIN_MOTOR_RIGHT_EN 08
// In2.
#define S_PIN_MOTOR_RIGHT_PH 18
