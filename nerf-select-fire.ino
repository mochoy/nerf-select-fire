#include <SoftwareSerial.h>

#define JOY_INPUT_PIN 1

void setup () {
    Serial.begin(9600);
}

int treatValue(int data) {
    return (data * 9 / 1024) + 48;
}

void loop () {
    int reading = (map(analogRead(JOY_INPUT_PIN), 0, 1023, 0, 500));

//    Serial.println(reading);
    
    if (reading > 490) {
        Serial.println("up!");
    } else if (reading < 360) {
        Serial.println("down!");
    }

    delay(50);
}

