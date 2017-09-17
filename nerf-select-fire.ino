#include <SoftwareSerial.h>
#include <Button.h>

//pins
#define IR_GATE_PIN 0           //analog
#define JOYSTICK_INPUT_PIN 2    //analog
#define TRIGGER_PIN 2     //digital
#define MOTOR_OUTPUT_PIN 3      //digital PWM

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
int fireMode = 0;   //0 = safe, 1 = single shot, 2 = burst, 3 = full auto

//keep track of debouncing joystick
int lastJoystickReading, debounceDelay = 50;
double lastTime; 

//keep track of how many darts fire
int numOfDartsFired = 0;

Button trigger (TRIGGER_PIN, true, false, 20);    

void setup () {
    Serial.begin(9600);
    

    //setup pin for motor control
    pinMode(MOTOR_OUTPUT_PIN, OUTPUT);
    digitalWrite(MOTOR_OUTPUT_PIN, LOW);        //make sure motor is off
}

void loop () {
    toggleFireModes();
    fire();
    selectFire();
}

//switch between the various modes
void toggleFireModes () {
    int joystickReading = ((map(analogRead(JOYSTICK_INPUT_PIN), 0, 1023, 0, 500)) > JOYSTICK_INCRECMENT_VAL ? 0 : ((map(analogRead(JOYSTICK_INPUT_PIN), 0, 1023, 0, 500)) < JOYSTICK_DECREMENT_VAL ? 2 : 1));   //up = 0, neutral/middle = 1, down = 2

    //joystick debouncing
    if ((lastJoystickReading != joystickReading) && (millis() >= lastTime + debounceDelay)) {   //make sure joystick actually moved and check once every 50 milis
        if (joystickReading == 0) {     //up
            fireMode = ((fireMode == 3) ? 0 : fireMode + 1);    //increment fireMode
        } else if (joystickReading == 2) {      //down
            fireMode = ((fireMode == 0) ? 3 : fireMode - 1);    //decrement fireMode
        } 

        lastTime = millis();
    }
    
    lastJoystickReading = joystickReading;
}

//when dart fired
void fire() { 
    numOfDartsFired += ((map(analogRead(IR_GATE_PIN), 0, 1023, 0, 100) > IR_GATE_TRIP) ? 1 : 0);  
}

//do all the fancy select fire stuff
void selectFire () {
    if (trigger.read()) {      //check of trigger is pressed
        if (fireMode == SAFETY) {       //if safety, turn off motor
            digitalWrite(MOTOR_OUTPUT_PIN, LOW);
        } else if (fireMode == SINGLE_FIRE || fireMode == BURST_FIRE) {
            if (((fireMode == SINGLE_FIRE) ? 1 : 3) <= numOfDartsToFire) {       
                digitalWrite(MOTOR_OUTPUT_PIN, HIGH);
            } else {
                digitalWrite(MOTOR_OUTPUT_PIN, LOW);
            }
        } else if (fireMode == FULL_AUTO) {     //if full auto, turn on motor
            digitalWrite(MOTOR_OUTPUT_PIN, HIGH);
        }
    } else {    //trigger isn't pressed
        if (fireMode == SINGLE_FIRE || fireMode == SAFETY) {
            digitalWrite(MOTOR_OUTPUT_PIN, LOW);
        } else if (fireMode == SINGLE_FIRE || fireMode == BURST_FIRE) {
            if (((fireMode == SINGLE_FIRE) ? 1 : 3) >= numOfDartsToFire) {      //if can stop firing, because already fired 1 or 3 darts
                digitalWrite(MOTOR_OUTPUT_PIN, LOW);        //turn off motor
                numOfDartsFired = 0;                        //reset numOfDarts fired so it can fire again on next trigger pull
            } else {        //if still needs to fire, because havent fired 1 or 3 darts
                digitalWrite(MOTOR_OUTPUT_PIN, HIGH);
            }
        }
    }
}

#define SAFETY 0
#define SINGLE_FIRE 1
#define BURST_FIRE 2
#define FULL_AUTO 3