#include <Button.h>
#include <SoftwareSerial.h>

//pins
#define IR_GATE_PIN 0               //analog
#define JOYSTICK_INPUT_PIN 6        //analog
#define TRIGGER_PIN 11              //digital
#define DART_COUNTER_SWITCH_PIN 4   //digital
#define MOTOR_OUTPUT_PIN 3          //digital PWM

//for buttons/switches
#define PULLUP true        
#define INVERT true      
#define DEBOUNCE_MS 20 

//"trip" values for joystick
#define JOYSTICK_INCRECMENT_VAL 490
#define JOYSTICK_DECREMENT_VAL 360

//'trip' value for IR gate
#define IR_GATE_TRIP 90

//code for fire modes
#define SAFETY 0
#define SINGLE_FIRE 1
#define BURST_FIRE 2
#define FULL_AUTO 3

//keep track of fire modes
byte fireMode = 2;   //0 = safe, 1 = single shot, 2 = burst, 3 = full auto

//keep track of debouncing joystick
int lastJoystickReading, debounceDelay = 50;
double lastTime; 

//keep track of how many darts fire
byte dartsFired = 0;

//check if burst cycle can reset (resets after firing 3 or 1 darts, dependig on the mode)
bool canReset = false;

//make sure checking for darts being fired. In some modes, the amount of darts being fired doesnt matter
bool isCheckingForDartsFired = false;

Button trigger (TRIGGER_PIN, PULLUP, INVERT, DEBOUNCE_MS);    
Button dartCountingSwitch (DART_COUNTER_SWITCH_PIN, PULLUP, INVERT, DEBOUNCE_MS);

void setup () {   
    Serial.begin(9600);

    //setup pin for motor control
    pinMode(MOTOR_OUTPUT_PIN, OUTPUT);
    digitalWrite(MOTOR_OUTPUT_PIN, LOW);        //make sure motor is off
}

void loop () {
    // toggleFireModes();
    fire();
    checkForDartsFired();
    selectFire();
}

//switch between the various modes
void toggleFireModes () {
    int joystickReading = ((map(analogRead(JOYSTICK_INPUT_PIN), 0, 1023, 0, 500)) > JOYSTICK_INCRECMENT_VAL ? 0 : ((map(analogRead(JOYSTICK_INPUT_PIN), 0, 1023, 0, 500)) < JOYSTICK_DECREMENT_VAL ? 2 : 1));   //up = 0, neutral/middle = 1, down = 2

    //joystick debouncing
    if ((lastJoystickReading != joystickReading) && (millis() >= lastTime + debounceDelay)) {   //make sure joystick actually moved and check once every 50 milis
        if (joystickReading == 0) {     // joystick moved up
            fireMode = ((fireMode == 3) ? 0 : fireMode + 1);    //increment fireMode
        } else if (joystickReading == 2) {      // joystick moved dowm
            fireMode = ((fireMode == 0) ? 3 : fireMode - 1);    //decrement fireMode
        }
        dartsFired = 0;        //reset num of darts fire so next time it loops back to 3rd burst/single shot, the dart firings don't get messed up 
        lastTime = millis();
    }
    
    lastJoystickReading = joystickReading;
}

//when dart fired
void fire() {
    dartCountingSwitch.read();

    //detect if dart is fired based on IR gate or switc
    dartsFired += ( (isCheckingForDartsFired && 
    	( (map(analogRead(IR_GATE_PIN), 0, 1023, 0, 100) > IR_GATE_TRIP) ||
    	 dartCountingSwitch.wasPressed()) )
    	 ? 1 : 0);        
}

void checkForDartsFired () {
  if (isCheckingForDartsFired && (fireMode == SINGLE_FIRE || fireMode == BURST_FIRE)) {
    byte dartsToFire = (fireMode == SINGLE_FIRE ? 1 : 3);
    if (dartsFired < dartsToFire) {
      digitalWrite(MOTOR_OUTPUT_PIN, HIGH);
    } else if (dartCountingSwitch.isPressed() && dartsFired >= dartsToFire) {
      digitalWrite(MOTOR_OUTPUT_PIN, LOW);
      canReset = true;
    }
  }
}

//do all the fancy select fire stuff
void selectFire () {
    trigger.read();
    if (trigger.isPressed()) {      //check of trigger is pressed
        if (fireMode == SAFETY) {       //if safety, turn off motor
            digitalWrite(MOTOR_OUTPUT_PIN, LOW);
        } else if (fireMode == SINGLE_FIRE || fireMode == BURST_FIRE) {
            isCheckingForDartsFired = true;
            canReset = false;
        } else if (fireMode == FULL_AUTO) {     //if full auto, turn on motor
            digitalWrite(MOTOR_OUTPUT_PIN, HIGH);
        }
    } else if (!trigger.isPressed()) {    //trigger isn't pressed
        if (fireMode == FULL_AUTO || fireMode == SAFETY) {
            //turn off motor if the mode is safety or full auto. Full auto only shoots when trigger is pulled
            digitalWrite(MOTOR_OUTPUT_PIN, LOW);

        //check to see if mode is single shot or burst. 
        //If all darts haven't been fired yet when the trigger is let go of, then some darts still need to be fired to complete cycle    
        } else if ( canReset && (fireMode == SINGLE_FIRE || fireMode == BURST_FIRE) ) {     
            digitalWrite(MOTOR_OUTPUT_PIN, LOW);
            dartsFired = 0;
            canReset = false;
            isCheckingForDartsFired = false;
        }
    }
}
