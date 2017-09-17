#include <SoftwareSerial.h>
#include <Button.h>

//pins
#define IR_GATE_PIN 0           //analog
#define JOYSTICK_INPUT_PIN 2    //analog
#define TRIGGER_INPUT_PIN 2     //digital
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

Button trigger (TRIGGER_INPUT_PIN, false, false, 20);    

void setup () {
    Serial.begin(9600);
    
    pinMode(MOTOR_OUTPUT_PIN, OUTPUT);
    digitalWrite(MOTOR_OUTPUT_PIN, LOW);
}

void loop () {
    toggleFireModes();

    trigger.read();
    if (trigger.isPressed()) {
        fire();
    }

}

//returns value based on up, down, or neutral
//up = 0
//neutral/middle = 1
//down = 2
int adjustedJoystickReading () {
    
    return ((map(analogRead(JOYSTICK_INPUT_PIN), 0, 1023, 0, 500)) > JOYSTICK_INCRECMENT_VAL ? 0 : ((map(analogRead(JOYSTICK_INPUT_PIN), 0, 1023, 0, 500)) < JOYSTICK_DECREMENT_VAL ? 2 : 1));

    //int rawReading = (map(analogRead(JOYSTICK_INPUT_PIN), 0, 1023, 0, 500));
    // if (rawReading > JOYSTICK_INCRECMENT_VAL) {     //up
    //     return 0;
    // } else if (rawReading < JOYSTICK_DECREMENT_VAL) {      //down
    //     return 2;
    // } else {    //neutral
    //     return 1;
    // }
}

//switch between the various modes
void toggleFireModes () {
    int joystickReading = adjustedJoystickReading();
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
    if ((map(analogRead(IR_GATE_PIN), 0, 1023, 0, 100) > IR_GATE_TRIP) ){
        numOfDartsFired++;  //keep track of how may darts fired
    }
}



