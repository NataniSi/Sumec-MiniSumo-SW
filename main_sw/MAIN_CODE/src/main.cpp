
/*
*   Project Name: Sumec
*   Name: MAIN_CODE/Strong
*   Last change: 8.4.2025
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

//=================== DECLARING OBJECTs ===================

// motors objects
Motor MotorLeft(PIN_MOTOR_LEFT_NSLEEP, PIN_MOTOR_LEFT_ENABLE, PIN_MOTOR_LEFT_PHASE, MOTOR_LEFT_CHANNEL);
Motor MotorRight(PIN_MOTOR_RIGHT_NSLEEP, PIN_MOTOR_RIGHT_ENABLE, PIN_MOTOR_RIGHT_PHASE, MOTOR_RIGHT_CHANNEL);
Motion Move(MotorLeft, MotorRight);

// IR, RC-5 and led objects
DohyoIR Remote(PIN_IR);
LED LEDRed(PIN_LED_RED);
LED LEDOrange(PIN_LED_ORABGE);





//=================== DECLARING OBJECTs ===================

void setup()
{



    // hardware settings (Setup's):

    pinMode(PIN_Start, INPUT);
    Serial.begin(115200);

}

void loop()
{
 
    LEDRed.update();            //updates the red led
    LEDOrange.update();         //updates the orange led
    Remote.update();


    // Button
    bootonOld = digitalRead(button);


   

    //===========================Normal process===============================

    if(Remote.isStopped()) 
    {
        Move.stop();
        LEDOrange.blink(500);
    }


    if (Remote.hasDohyoID() && !Remote.isStarted())
    {
        LEDOrange.blink(500, 100);
    } else
    {
        LEDOrange.blink(500);
    }

    // after start comand, main code will start running
    if (Remote.isStarted())
    {
        LEDOrange.blink(100);
        Move.goForward(1.0);
    }


}