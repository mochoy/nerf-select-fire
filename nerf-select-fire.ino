/*----------------------------------------------------------------------*
 * nerf-select-fire v1.0                        			                  *
 * Monty Choy, Nov 2017													                        *
 *                                                                      *
 * Arduino sketch for saftey, single shot, three round burst, and full  *
 * auto. Schematics can also be found in the repository. 								*
 *----------------------------------------------------------------------*/

#include <Button.h>																											//library to deal with buttons easier

//pins
#define IR_GATE_PIN 0																											//analog input
#define TRIGGER_PIN 11              																			//digital input
#define DART_COUNTER_SWITCH_PIN 4   																			//digital input
#define MOTOR_OUTPUT_PIN 3          																			//digital output
#define JOYSTICK_X_PIN 2
#define JOYSTICK_Y_PIN 3

#define MAPPED_HIGH_JOYSTICK_TRIP 490 
#define MAPPED_LOW_JOYSTICK_TRIP 360

//for buttons/switches
#define PULLUP true        																								//internal pullup, so we dont need to wire resistor
#define INVERT true      																									//invert required for proper readings with pullup
#define DEBOUNCE_MS 20 																										//check btn time every 20ms

#define IR_GATE_TRIP 90																										//'trip' value for IR gate					

//code for fire modes. 4 modes total
#define SAFETY 0																													//SAFTEY is mode 0
#define SINGLE_FIRE 1																											//singe fire is mode 1
#define BURST_FIRE 2																											//burst fire is mode 2
#define FULL_AUTO 3																												//full auto is mode 3


byte fireMode = 0;   																											//keep track of fire modes. 
byte dartsFired = 0;																											//keep track of how many darts fire
bool isCheckingForDartsFired = false;																			//some modes need to check if a certain number of darts to fire

Button trigger (TRIGGER_PIN, PULLUP, INVERT, DEBOUNCE_MS);														//trigger button, using the library   
Button dartCountingSwitch (DART_COUNTER_SWITCH_PIN, PULLUP, INVERT, DEBOUNCE_MS);			//dart counting button, using the library

void setup () {   
    pinMode(MOTOR_OUTPUT_PIN, OUTPUT);																		//set motor output pin to an output pin
    digitalWrite(MOTOR_OUTPUT_PIN, LOW);        													//make sure motor is off
    resetDartsFired();																										//reset all dart firing values so they dont get messed up later
}

void loop () {
    toggleFireModes();																										//constantly check for changes in firemodes
    fire();																																//constantly check if dart is fired
    checkForDartsFired();																									//do stuff if dart is fired
    selectFire();																													//do fancy select-fire stuff
}

//switch between the various modes
void toggleFireModes () {
  bool hasStateChanged = false;
  
  int joystickXHelped = joystickHelper(map(analogRead(JOYSTICK_X_PIN), 0, 1023, 0, 500));
  if (joystickXHelped > 0) {
    if (joystickXHelped == 1) {
      fireMode = 0;
    } else if (joystickXHelped == 2) {
      fireMode = 1;
    }

    hasStateChanged = true;
  }

  int joystickYHelped = joystickHelper(map(analogRead(JOYSTICK_Y_PIN), 0, 1023, 0, 500));
  if (joystickYHelped > 0) {
    if (joystickYHelped == 1) {
      fireMode = 2;
    } else if (joystickYHelped == 2) {
      fireMode = 3;
    }

    hasStateChanged = true;
  }

  if (hasStateChanged) {
    resetDartsFired();																										//reset darts fired stuff so it doesn't get messed up later
  }
}

//method to check if joystick moved up or down
//returns 0 if no move, 1 if up, 2 if down
int joystickHelper (int reading) {
  if (reading > MAPPED_HIGH_JOYSTICK_TRIP) {
    return 1;
  } else if (reading < MAPPED_LOW_JOYSTICK_TRIP) {
    return 2;
  } else {
    return 0;
  }
}

//when dart fired
void fire() {
  dartCountingSwitch.read();																							//read button
  dartsFired += ( (isCheckingForDartsFired && 														//detect and keep track if dart is fired through
  	( (map(analogRead(IR_GATE_PIN), 0, 1023, 0, 100) > IR_GATE_TRIP) ||		    //switch or IR gate. 
  	 dartCountingSwitch.wasPressed()) )
  	 ? 1 : 0);        
}

void checkForDartsFired () {						
  if (isCheckingForDartsFired && 																					//if checking for darts being fired. Not all 
   (fireMode == SINGLE_FIRE || fireMode == BURST_FIRE)) {									// modesneed to check if a dart is fired
    byte dartsToFire = (fireMode == SINGLE_FIRE ? 1 : 3);									//determine max amounts of darts to be fired
    if (dartsFired < dartsToFire) {																				//if can still fire (hasn't reached threshold of
      digitalWrite(MOTOR_OUTPUT_PIN, HIGH);																//how many darts can fire), power pusher motor
    } else if (dartCountingSwitch.isPressed() && 													//if can't fire anymore darts and pusher 
     dartsFired >= dartsToFire) {																					//retracted
      resetDartsFired();																									//Reset darts fired stuff so it can happen again
    }
  }
}

//do all the fancy select fire stuff
void selectFire () {
    trigger.read();																												//read trigger
    if (trigger.isPressed()) {      																			//check of trigger is pressed
        if (fireMode == SAFETY) {       																	//if in safety mode, turn off motor
            digitalWrite(MOTOR_OUTPUT_PIN, LOW);													
        } else if (fireMode == SINGLE_FIRE || fireMode == BURST_FIRE) {		//if in burst fire or single shot mode
            isCheckingForDartsFired = true;																//allow for darts to be fired, handled elsewhere
        } else if (fireMode == FULL_AUTO) {     													//if full auto turn on motor
            digitalWrite(MOTOR_OUTPUT_PIN, HIGH);													
        }
    } else if (!trigger.isPressed()) {    																//if trigger isn't pressed
        if (fireMode == FULL_AUTO || fireMode == SAFETY) {								//if firemode is fullauto or safety, turn off motor
            digitalWrite(MOTOR_OUTPUT_PIN, LOW);													
        } else if ( !isCheckingForDartsFired 															//if all darts fired
         && (fireMode == SINGLE_FIRE || fireMode == BURST_FIRE) ) {     	//and in burstfire 
        	resetDartsFired();																							//reset darts fired stuff
        }		
    }
}

void resetDartsFired () {
	digitalWrite(MOTOR_OUTPUT_PIN, LOW);																		//turn of motor
	dartsFired = 0;																													//darts fired set to 0
	isCheckingForDartsFired = false;																				//no longer checking if darts are being fired
}
