
/*
*   Project Name: Sumec
*   Name: MAIN_CODE
*   Last change: 1.3.2025
*/

#include <Data_holder.h>
#include <Pins.h>
#include <Motor.h>
#include <Motion.h>
#include <DohyoIR.h>
#include <LED.h>
#include <QRE1113.h>
#include <variablesForMAIN.h>
#include <TfLunaEsp32S3.h>
#include <Sharp_GP2Y0D810Z0F.h>
#include "pwmSensor.h"

//=================== DECLARING OBJECTs ===================

// motors objects
Motor MotorLeft(PIN_MOTOR_LEFT_NSLEEP, PIN_MOTOR_LEFT_ENABLE, PIN_MOTOR_LEFT_PHASE, MOTOR_LEFT_CHANNEL);
Motor MotorRight(PIN_MOTOR_RIGHT_NSLEEP, PIN_MOTOR_RIGHT_ENABLE, PIN_MOTOR_RIGHT_PHASE, MOTOR_RIGHT_CHANNEL);
Motion Move(MotorLeft, MotorRight);

// IR, RC-5 and led objects
DohyoIR Remote(PIN_IR);
LED LEDRed(PIN_LED_RED);
LED LEDOrange(PIN_LED_ORABGE);

// Line sonzors objects
QRE qreLeft('L');
QRE qreRight('R');
QRE qreBack('B');

// side sonzors objects
Sharp sharpLeft('R');
Sharp sharpRight('L');

// new
#define pwmSensorPinArrayLen_var 2  // Number of sensors.
const uint8_t pwmSensorPinArray_var[pwmSensorPinArrayLen_var] = {5, 15};  // Sensor pins (pcb_v.4 left = 5, right = 16).
uint8_t measuredDistanceArray_var[pwmSensorPinArrayLen_var];  // Array that stores the measured distances.

void serialSetCursorPos(uint16_t row, uint16_t col) 
{
    Serial.print("\033[");  // Begin of escape sequence.
    Serial.print(row+1);  // Row number (begins with 1).
    Serial.print(";");
    Serial.print(col+1);  // Column (begins with 1).
    Serial.print("H");
}

//=================== DECLARING OBJECTs ===================

void setup()
{
    
    // logic settings:
    Tick_QRE.tickTime = 10;         //this replaces delay
    Tick_Sharp.tickTime = 10;       //this replaces delay
    Tick_Start.tickTime = 10;       //this replaces delay
    Tick_free.tickTime = 1;       //this replaces delay

    // hardware settings (Setup's):
    TfL_Setup();
    pinMode(button, INPUT);
    pinMode(PIN_Start, INPUT);
    Serial.begin(115200);
    UDP_Setup();

    qreLeft.Threshold = 3000;
    qreRight.Threshold = 3000;

    //new
    pwmSensor(pwmSensorPinArray_var, pwmSensorPinArrayLen_var, measuredDistanceArray_var);
}

void loop()
{

    LEDRed.update();            //updates the red led
    LEDOrange.update();         //updates the orange led
    Remote.update();


    //=========================Writeing value from sensors to variables=============

    // Button
    bootonOld = digitalRead(button);

    // Line sonzors
    QREleft = qreLeft.get();
    QREright = qreRight.get();
    QREback = qreBack.get();

    // Length senzors
    //LUNAleft = TfL_Get(TfL_Addr1);
    //LUNAright = TfL_Get(TfL_Addr3);
    LUNAmiddle = TfL_Get(TfL_Addr2);


    // side sonzors
    SHARPleft = sharpLeft.get();
    SHARPright = sharpRight.get();

    // Writeing value to TICK
    Tick_managing(Tick_QRE.tickTime, Tick_QRE.tickNumber, Tick_QRE.lastTick, &Tick_QRE.lastTick, &Tick_QRE.tickNumber);
    Tick_managing(Tick_Sharp.tickTime, Tick_Sharp.tickNumber, Tick_Sharp.lastTick, &Tick_Sharp.lastTick, &Tick_Sharp.tickNumber);
    Tick_managing(Tick_Start.tickTime, Tick_Start.tickNumber, Tick_Start.lastTick, &Tick_Start.lastTick, &Tick_Start.tickNumber);
    Tick_managing(Tick_free.tickTime, Tick_free.tickNumber, Tick_free.lastTick, &Tick_free.lastTick, &Tick_free.tickNumber);



    /*if(Remote.isStarted() && !Remote.isStopped())
    {
        Move.goForward(1.0);
    }
    else if(Remote.hasDohyoID() && !Remote.isStarted())
    {
        Move.stop();
    }*/

    /*Serial.println(LUNAmiddle);
    delay(100);*/

    


    //Move.goForward(1.0);



    //Serial.println("1st sensor | 2nd sensor");
    Serial.print(measuredDistanceArray_var[0]);
    Serial.print(" | ");
    Serial.println(measuredDistanceArray_var[1]);
    //Serial.print("Millis: ");
    //Serial.println(millis());
    //delay(100);
}