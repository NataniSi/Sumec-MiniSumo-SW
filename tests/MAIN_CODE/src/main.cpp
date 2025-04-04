
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
#include "pololuPwm.h"

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

// distance senzor
pwmSensor pwmL(16, 4);
pwmSensor pwmR(15, 5);

uint16_t pulseL;
uint16_t pulseR;

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
    pwmL.pwmSetup();
    pwmR.pwmSetup();
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
    pwmL.pwmRead(&pulseL);
    pwmR.pwmRead(&pulseR);
    LUNAleft = pwmL.pwmToMm(pulseL);
    LUNAright = pwmR.pwmToMm(pulseR);
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

    
    Serial.println(LUNAleft);

    //Move.goForward(1.0);



    //Serial.println("1st sensor | 2nd sensor");
    
    //Serial.print("Millis: ");
    //Serial.println(millis());
    //delay(100);
}