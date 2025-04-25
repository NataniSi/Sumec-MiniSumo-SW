#include <Motion.h>
#include <Arduino.h>

//============================= PROGRAM VARIABLES ===============================

#define Range 20                 // Length senzor range[cm] for decision, is enybodey there?
#define Type_of_starting 0       // 0 = IR led, 1 = IR modul
#define tests 0                  // tests: 0 = off, 1 = on
#define Searching_rotate 0.7
#define AnotherStarting 1

// Logic variables:
bool back_on_line = 0;          //0 = normal state, 1 = Sumec's back starting on line
bool starting_direction = 0;


// States
int tipe_of_strategy = 290;
uint16_t state = 0;             // variable who decides, what is in progress
uint16_t LINEstate = 0;
uint8_t startState;
uint16_t saveState;

// Start variables
bool isStarted = 0;             // variable for starting with button
int START = 0; 


// sensors:

// Value of line sonzors
uint8_t QREleft;
uint8_t QREright;
uint8_t QREback;

// Value of length sonzors
int LUNAleft;
int LUNAright;
int LUNAmiddle;

// Value of side sonzors
bool SHARPleft;
bool SHARPright;

// Start time
bool bootonOld;


//============================= PROGRAM VARIABLES ===============================

// structure tick, who have all tick variables 
struct TICK
{
    int tickTime;
    uint32_t tickNumber = 100;
    uint32_t lastNumberTick = 0;
    uint32_t lastTick = 0;
};

// function for updateing tick's
void Tick_managing(int time, uint32_t value, uint32_t last, uint32_t *return_last, uint32_t *return_value)
{
    if (millis() - last >= time)
    {
        value++;
        last = millis();
    }

    *return_last = last;
    *return_value = value;
}

// using tics

TICK Tick_QRE;
TICK Tick_Sharp;
TICK Tick_Start;
TICK Tick_free;


// Function for calibration

float A = 0;
float B = 0;
int count;

float calibration(float volueA, float volueB)
{
  float sum;

  if(volueA > volueB) sum = volueB;
  else sum = volueA;

  return abs((volueA - volueB)/2)+sum;
}
