#include <SoftwareSerial.h>
#include <Button.h>

//pins
#define JOYSTICK_INPUT_PIN 1
#define TRIGGER_INPUT_PIN 2
#define MOTOR_OUTPUT_PIN 3

//"trip" values for joystick
#define JOYSTICK_INCRECMENT_VAL 490
#define JOYSTICK_DECREMENT_VAL 360

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
    int rawReading = (map(analogRead(JOYSTICK_INPUT_PIN), 0, 1023, 0, 500));

    if (rawReading > JOYSTICK_INCRECMENT_VAL) {     //up
        return 0;
    } else if (rawReading < JOYSTICK_DECREMENT_VAL) {      //down
        return 2;
    } else {    //neutral
        return 1;
    }
}

void toggleFireModes () {
    int joystickReading = adjustedJoystickReading();

//    Serial.println(reading);
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

void fire() {    
    if (fireMode == SAFETY) {
        Serial.println("Safety");
    } else if (fireMode == SINGLE_FIRE) {
        Serial.println("Single Fire");
    } else if (fireMode == BURST_FIRE) {
        Serial.println("Burst Fire");
    } else if (fireMode == FULL_AUTO) {
        Serial.println("Full Auto");
    }
}



