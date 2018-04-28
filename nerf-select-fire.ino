/*----------------------------------------------------------------------*
 * nerf-select-fire v1.0                                                *
 * Monty Choy, Nov 2017                                                 *
 *                                                                      *
 * Arduino sketch for saftey, single shot, three round burst, and full  *
 * auto. Schematics can also be found in the repository.                *
 *----------------------------------------------------------------------*/

#include <Button.h>                                                     //library to deal with buttons easier
#include <SoftwareSerial.h>

//pins
#define IR_GATE_PIN 0                                                     //analog input
#define TRIGGER_PIN 11                                                    //digital input
#define DART_COUNTER_SWITCH_PIN 4                                         //digital input
#define MOTOR_OUTPUT_PIN 3                                                //digital output
#define JOYSTICK_X_PIN 1
#define JOYSTICK_Y_PIN 2

#define MAPPED_HIGH_JOYSTICK_TRIP 490 
#define MAPPED_LOW_JOYSTICK_TRIP 360

//for buttons/switches
#define PULLUP true                                                       //internal pullup, so we dont need to wire resistor
#define INVERT true                                                       //invert required for proper readings with pullup
#define DEBOUNCE_MS 20                                                    //check btn time every 20ms

#define IR_GATE_TRIP 90                                                   //'trip' value for IR gate          

//code for fire modes. 4 modes total
#define SAFETY 0                                                          //SAFTEY is mode 0
#define SINGLE_FIRE 1                                                     //singe fire is mode 1
#define BURST_FIRE 2                                                      //burst fire is mode 2
#define FULL_AUTO 3                                                       //full auto is mode 3


byte fireMode = 0;                                                        //keep track of fire modes. 
byte lastFireMode = 0;
byte dartsFired = 0;                                                      //keep track of how many darts fire
bool isCheckingForDartsFired = false;                                     //some modes need to check if a certain number of darts to fire

Button trigger (TRIGGER_PIN, PULLUP, INVERT, DEBOUNCE_MS);                            //trigger button, using the library   
Button dartCountingSwitch (DART_COUNTER_SWITCH_PIN, PULLUP, INVERT, DEBOUNCE_MS);     //dart counting button, using the library

void setup () {   
    Serial.begin(9600);
    pinMode(MOTOR_OUTPUT_PIN, OUTPUT);                                    //set motor output pin to an output pin
    digitalWrite(MOTOR_OUTPUT_PIN, LOW);                                  //make sure motor is off
    resetDartsFired();                                                    //reset all dart firing values so they dont get messed up later
}

void loop () {
    toggleFireModes();                                                    //constantly check for changes in firemodes
    fire();                                                               //constantly check if dart is fired
    checkForDartsFired();                                                 //do stuff if dart is fired
    selectFire();                                                         //do fancy select-fire stuff
}

//switch between the various modes
void toggleFireModes () {
  bool hasStateChanged = false;

 
  if (lastFireMode != 0 && analogRead(JOYSTICK_X_PIN) > 800) {   //safety
    lastFireMode = fireMode = 0;
    hasStateChanged = true;
  } else if (lastFireMode != 2 && analogRead(JOYSTICK_X_PIN) < 100) {  //burst
    lastFireMode = fireMode = 2;
    hasStateChanged = true;
  } else if (lastFireMode != 1 && analogRead(JOYSTICK_Y_PIN) > 900) {  //single shot 
    lastFireMode = fireMode = 1;
    hasStateChanged = true;
  } else if (lastFireMode != 3 && analogRead(JOYSTICK_Y_PIN) < 100) {  //full auto
    lastFireMode = fireMode = 3;
    hasStateChanged = true;
  }

  if (hasStateChanged) {
    resetDartsFired();                                                    //reset darts fired stuff so it doesn't get messed up later
  }
}

//when dart fired
void fire() {
  dartCountingSwitch.read();                                              //read button
  if (dartCountingSwitch.wasReleased()) {
    dartsFired++;
    Serial.println("shot fired!");
  }
}

void checkForDartsFired () {      
  dartCountingSwitch.read();      
  if (isCheckingForDartsFired &&                                          //if checking for darts being fired. Not all 
   (fireMode == SINGLE_FIRE || fireMode == BURST_FIRE)) {                 // modesneed to check if a dart is fired
    byte dartsToFire = (fireMode == SINGLE_FIRE ? 1 : 3);                 //determine max amounts of darts to be fired
    if (dartsFired < dartsToFire) {                                       //if can still fire (hasn't reached threshold of
      digitalWrite(MOTOR_OUTPUT_PIN, HIGH);                               //how many darts can fire), power pusher motor
//      Serial.println("shootin!");
    } else if (dartCountingSwitch.isPressed() &&                          //if can't fire anymore darts and pusher 
     dartsFired >= dartsToFire) {                                         //retracted
//      Serial.println("not shooting!");

      resetDartsFired();                                                  //Reset darts fired stuff so it can happen again
      
    }
  }
}

//do all the fancy select fire stuff
void selectFire () {
    trigger.read();                                                       //read trigger
    if (trigger.wasPressed() && (fireMode == SINGLE_FIRE || fireMode == BURST_FIRE)) {   //if in burst fire or single shot mode
        isCheckingForDartsFired = true;                               //allow for darts to be fired, handled elsewhere
    } else if (trigger.isPressed()) {                                            //check of trigger is pressed
        if (fireMode == SAFETY) {                                         //if in safety mode, turn off motor
            digitalWrite(MOTOR_OUTPUT_PIN, LOW);                          
        } else if (fireMode == FULL_AUTO) {                               //if full auto turn on motor
            digitalWrite(MOTOR_OUTPUT_PIN, HIGH);                         
        }
    } else if (!trigger.isPressed()) {                                    //if trigger isn't pressed
        if (fireMode == FULL_AUTO || fireMode == SAFETY) {                //if firemode is fullauto or safety, turn off motor
            digitalWrite(MOTOR_OUTPUT_PIN, LOW);                          
        } else if ( !isCheckingForDartsFired                              //if all darts fired
         && (fireMode == SINGLE_FIRE || fireMode == BURST_FIRE) ) {       //and in burstfire 
          resetDartsFired();                                              //reset darts fired stuff
        }   
    }
}

void resetDartsFired () {
  digitalWrite(MOTOR_OUTPUT_PIN, LOW);                                    //turn of motor
  dartsFired = 0;                                                         //darts fired set to 0
  isCheckingForDartsFired = false;                                        //no longer checking if darts are being fired
}
