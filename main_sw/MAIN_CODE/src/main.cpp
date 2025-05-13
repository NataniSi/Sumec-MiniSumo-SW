
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

//=================== DECLARING OBJECTs ===================

// motors objects
Motor MotorLeft(PIN_MOTOR_LEFT_NSLEEP, PIN_MOTOR_LEFT_ENABLE, PIN_MOTOR_LEFT_PHASE, MOTOR_LEFT_CHANNEL);
Motor MotorRight(PIN_MOTOR_RIGHT_NSLEEP, PIN_MOTOR_RIGHT_ENABLE, PIN_MOTOR_RIGHT_PHASE, MOTOR_RIGHT_CHANNEL);
Motion Move(MotorLeft, MotorRight);

// IR, RC-5 and led objects
DohyoIR Remote(PIN_IR, AnotherStarting);
LED LEDRed(PIN_LED_RED);
LED LEDOrange(PIN_LED_ORABGE);

// Line sonzors objects
QRE qreLeft('L');
QRE qreRight('R');
QRE qreBack('B');

// side sonzors objects
Sharp sharpLeft('R');
Sharp sharpRight('L');



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


    qreLeft.Threshold = 3000;
    qreRight.Threshold = 3000;
}

void loop()
{
 
    LEDRed.update();            //updates the red led
    LEDOrange.update();         //updates the orange led
    Remote.update();
    START = digitalRead(PIN_Start);

    
    if(/*Remote.isStopped()*/!START) 
    {
        state = 0;
        Move.stop();
        LEDRed.blink(500);
    }

    //=========================Writeing value from sensors to variables=============

    // Button
    bootonOld = digitalRead(button);

    // Line sonzors
    QREleft = qreLeft.get();
    QREright = qreRight.get();
    QREback = qreBack.get();

    // Length senzors
    LUNAleft = TfL_Get(TfL_Addr1);
    LUNAright = TfL_Get(TfL_Addr3);
    LUNAmiddle = TfL_Get(TfL_Addr2);


    // side sonzors
    SHARPleft = sharpLeft.get();
    SHARPright = sharpRight.get();

    // Writeing value to TICK
    Tick_managing(Tick_QRE.tickTime, Tick_QRE.tickNumber, Tick_QRE.lastTick, &Tick_QRE.lastTick, &Tick_QRE.tickNumber);
    Tick_managing(Tick_Sharp.tickTime, Tick_Sharp.tickNumber, Tick_Sharp.lastTick, &Tick_Sharp.lastTick, &Tick_Sharp.tickNumber);
    Tick_managing(Tick_Start.tickTime, Tick_Start.tickNumber, Tick_Start.lastTick, &Tick_Start.lastTick, &Tick_Start.tickNumber);
    Tick_managing(Tick_free.tickTime, Tick_free.tickNumber, Tick_free.lastTick, &Tick_free.lastTick, &Tick_free.tickNumber);


    //==========================Out of line Process==============================


    switch (LINEstate)
    {
        //QRE
        case 0:     

            if((QREleft || QREright) && state != 0 && state != 2 && state != 3)
            {
                saveState = state;   //saved last state
                state = 001;
                LINEstate = LINEstate+2;

                Tick_QRE.lastTick = millis();
                Tick_QRE.tickNumber = 0;
                Move.stop();
            }

            break;

        //Go backward
        case 2:     

            if(Tick_QRE.tickNumber < 10)
            {
                Move.goBackward(1.0);
            }
            else
            {
                LINEstate = 3;
            }

            break;

        //STOP after backward
        case 3:     

            Move.stop();

            state = saveState;
            LINEstate = 0;

            break;

        //Go forward    -   I dont know, what it doing -> delete this
        case 4:     

            if(Tick_QRE.tickNumber < 10)
            {
                Move.goForward(1.0);
            }
            else
            {
                LINEstate = 3;
            }

            break;

    }

    

    //===========================Normal process===============================

    // printing values from senzors to chack, is it ok
    if(tests)
    {    
        Serial.print(LUNAleft);
        Serial.print(" ");
        Serial.print(LUNAmiddle);
        Serial.print(" ");
        Serial.println(LUNAright);
       Serial.print("    ");
        Serial.print(QREright);
        Serial.print(" ");
        Serial.println(qreBack.getRaw());
        Serial.print("    ");
        Serial.print(state);
    }
    //Serial.println(starting_direction);

    switch (state)
    {
    case 000:       // INIT
        
        if(bootonOld < digitalRead(button)) //state = 003;
        {
            back_on_line = (back_on_line+1)%2;

            if(back_on_line) LEDOrange.setOn();
            else LEDOrange.setOff();
        }
        
        //===========================Checking IR state===============================
        if (Remote.hasDohyoID() && !Remote.isStarted())
        {
            LEDRed.blink(500, 100);

            Tick_Sharp.lastTick = millis();
            Tick_Sharp.tickNumber = 0;
        } else
        {
            LEDRed.blink(500);

            if(SHARPleft)
            { 
                starting_direction = 0;
                LEDOrange.setOff();
            }
            if(SHARPright) 
            {
                LEDOrange.blink(500, 100);
                starting_direction = 1;
            }
        }

        // after start comand, main code will start running
        if (Remote.isStarted())
        {
            delay(5000);
            state = 002;
            Tick_Start.tickNumber = 0;
            LINEstate = 0;
        }

        LINEstate = 0;
        
        break;
    case 001:       // IDLE
        
        //nothing - program is stopped

        break;

    /*=============START FUNCTION=============*/

    case 002:

        //Out of line disabled
        LINEstate = 0;                                                                  

        //protect in back on line start
        if(back_on_line)                                                                
        {
            QREleft = 0;
            QREright = 0;
        }

        //setting startState only in one time
        if(startState == 0) startState = QREleft*1 + QREright*3 + back_on_line*5;       
        


        switch (startState)
        {

        //Sumec's left side starting on the line
        case 1:                                                                         

            if(Tick_Start.tickNumber < 20)
            {
                Move.turnRight(1.0);
            }
            else if(Tick_Start.tickNumber < 45)
            {
                Move.goForward(1.0);
            }
            else
            {
                state = tipe_of_strategy;   
            }
            break;

        //Sumec's right side starting on the line
        case 3:                                                                        

            if(Tick_Start.tickNumber < 20)
            {
                Move.turnLeft(1.0);
            }
            else if(Tick_Start.tickNumber < 45)
            {
                Move.goForward(1.0);
            }
            else
            {
                state = tipe_of_strategy;
            }
            break;

        //Sumec's back side starting on the line
        case 5:                                                                         

            if(Tick_Start.tickNumber < 40)
            {
                Move.goForward(1.0);
            }
            else
            {
                state = tipe_of_strategy;
            }
            break;

        //Sumec's front side starting on the line
        case 4:                                                                         

            if(Tick_Start.tickNumber < 20)
            {
                Move.goBackward(1.0);
            }
            else
            {
                state = tipe_of_strategy;
            }

        //Sumec starting inside the ring
        default:                                                                         
            
            if(starting_direction == 1){

                Move.turnLeft(1.0);
                Serial.println("left");
                
            } else{

                Move.turnRight(1.0);
                Serial.println("right");
            }

            if(LUNAleft < Range || LUNAright < Range) state = tipe_of_strategy;

            break;
        }      
        break;

    /*=============CALIBRATION STATE=============*/
    case 003:
 
        LINEstate = 0;
        LEDRed.setOff();

        if(digitalRead(button) && count == 0)
        {
            B = 0;
            A = analogRead(10);
            Serial.print("A: ");
            Serial.println(A);
            count = 1;
        }
        else if(count == 1 && !digitalRead(button))
        {
            count = 2;
        }
        else if(digitalRead(button) && count ==2)
        {
            B = analogRead(10);
            Serial.print("B: ");
            Serial.println(B);
            count = 3;
        }
        else if(count == 3 && !digitalRead(button))
        {
            Serial.println(calibration(A, B));
            
            count = 0;
            state = 000;
        }
        break;

    case 230:                                                                           

        // Searching with smoller speed
        if(LUNAright > Range) Move.turnRight(0.8*Searching_rotate);
        // Make movement
        else Move.turnRight(0.8);
        
        // Chacking left senzor
        if(LUNAleft < Range && LUNAmiddle > Range)
        {
            state = 260;
        }

        // Chacking middle senzor
        if(LUNAmiddle < Range)
        {
            state = 290;

            Tick_free.lastTick = millis();
            Tick_free.tickNumber = 0;
        }

        // Chacking sides senzor
        if(SHARPleft || SHARPright)
        {
            Tick_Sharp.lastTick = millis();
            Tick_Sharp.tickNumber = 0;

            if(SHARPleft)
            {
                state = 330;
            }
            else if(SHARPright)
            {
                state = 360;
            }
        }

        break;
    case 260:                                                                           

        // Searching with smoller speed
        if(LUNAleft > Range) 
        {
            Move.turnLeft(0.8*Searching_rotate);

            //Return to the searching mode, if left senzor see nothing, or middle or right senzors see something
            if(LUNAright < Range || LUNAmiddle < Range) 
            {   
                state = 230;
            }
        }
        // Make movement
        else Move.turnLeft(0.8);
        

        // Chacking sides senzor
        if(SHARPleft || SHARPright)
        {
            Tick_Sharp.lastTick = millis();
            Tick_Sharp.tickNumber = 0;

            if(SHARPleft)
            {
                state = 330;
            }
            else if(SHARPright)
            {
                state = 360;
            }
        }       
        break;
    case 290:                                                                           

        // Make movement
        Move.goForward(1.0);

        // twitching after 2s
        if(Tick_free.tickNumber >= 2000)
        {
            if(Tick_free.tickNumber < 2025)
            {
                Move.turnRight(1.0);
            }
            else if(Tick_free.tickNumber < 2075)
            {
                Move.turnLeft(1.0);
            }
            else
            {

                if(LUNAmiddle > Range)    
                {
                    state = 230;
                }

                Tick_free.tickNumber = 2000;
            }
        }

        // Return to the searching mode, if middle senzor see nothing
        else
        {

            if(LUNAmiddle > Range)    
            {
                state = 230;
            }

        }

        break;


    /*=============Sharps -> Emergenci mode=============*/

    case 330:                                                                            

        if(LUNAmiddle < Range || LUNAleft < Range || LUNAright < Range) state = 230;

        Move.turnRight(1.0);
        

        break;
    case 360:                                                                           

        if(LUNAmiddle < Range || LUNAleft < Range || LUNAright < Range) state = 230;

        Move.turnLeft(1.0);
        
        
        break;
    }    
}