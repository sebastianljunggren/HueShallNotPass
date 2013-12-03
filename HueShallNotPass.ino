/*
  Hue Shall Not Pass

  Unlock your computer in a safe-like way, using a potentiometer.

  This code is not safe since it requires storage of passwords in plaintext.
 */

#include <Bounce.h>

// The amount of LEDs for showing unlock progress.
const int progressLedCount = 5;
// The colors ordered from right to left.
const unsigned char colors[5][3] = {
    {255, 0, 0},  // Red
    {0, 100, 0},  // Green
    {0, 0, 255},  // Blue
    {255, 20, 0}, // Orange
    {255, 0, 255} // Purple
};
const unsigned char black[] = {0, 0, 0};
const unsigned char white[] = {127, 127, 127};

// Pins for showing which color is active
const int colorLedCount = 5;
const int colorLeds[] = {3, 4, 5, 6, 7};

const byte potPin = A3;

// The button is debounced using the Bounce-library.
const byte buttonPin = 9;
Bounce button = Bounce(buttonPin, 5);
boolean buttonReleased = false;

boolean locked = false;
int potValue = 0;
int progress = 0;
int activeColor = 0;

// The code is red, green, blue, red, purple.
int code[] = {0, 1, 2, 0, 4};
int enteredCode[] = {-1,-1,-1,-1,-1};
boolean litLeds[] = {true, true, true, true, true};

// Some variables needed for ShiftPWM.
const int ShiftPWM_latchPin=8;
#define SHIFTPWM_NOSPI
const int ShiftPWM_dataPin = 11;
const int ShiftPWM_clockPin = 13;
const bool ShiftPWM_invertOutputs = true;
const bool ShiftPWM_balanceLoad = false;
#include <ShiftPWM.h>
unsigned char maxBrightness = 255;
unsigned char pwmFrequency = 75;
int numRegisters = 2;
int numRGBleds = 5;

void setup() {  
     
    Serial.begin(9600);

    // Sets the number of 8-bit registers that are used.
    ShiftPWM.SetAmountOfRegisters(numRegisters);

    ShiftPWM.Start(pwmFrequency,maxBrightness);      
    for (int i = 0; i < colorLedCount; i++) {
        pinMode(colorLeds[i], OUTPUT);
    }
    pinMode(potPin, INPUT);
    pinMode(buttonPin, INPUT);
}

void loop() {
    // Check if computer has changed status
    if( Serial.available() > 0) {
        int status = Serial.read();
        if (status == 'l') {
            locked = true;
        } else {
            locked = false;
        }
    }

    // Se if the button has been released
    button.update();
    buttonReleased = button.risingEdge();

    // Check potentiometer
    potValue = map(analogRead(potPin), 0, 1023, 0, colorLedCount);
    if(locked && potValue != colorLedCount) {
        // User is trying to unlock with potentiometer
        tryToUnlock();
    } else if (locked && potValue == colorLedCount) {
        // Wait for the user to start trying to unlock
        // Ensure no leds are lit and that progress is zero
        progress = 0;
        for (int i=0; i < progressLedCount; i++) {
            litLeds[i] = false;
        }
        delay(10);
    } else if (!locked && potValue == colorLedCount) {
        // User wants to lock computer
        activeColor = -1;
        locked = true;
        Serial.print('l');
    }
    handleLeds();
    delay(100);
}

void handleLeds() {
    // The RGB LEDs.
    for(int i=0; i < progressLedCount; i++) {
        if (locked && potValue < colorLedCount) {
            if (i < progress) {
                ShiftPWM.SetRGB(i, white[0], white[1], white[2]);
            } else if (i == progress) {
                ShiftPWM.SetRGB(i, colors[activeColor][0], colors[activeColor][1], colors[activeColor][2]);
            } else {
                ShiftPWM.SetRGB(i, black[0], black[1], black[2]);
            }
        } else if (locked && potValue == colorLedCount) {
            ShiftPWM.SetRGB(i, black[0], black[1], black[2]);
        } else {
            ShiftPWM.SetRGB(i, white[0], white[1], white[2]);
        }   
    }

    // The color LEDs.
    for(int i=0; i<colorLedCount; i++){
        digitalWrite(colorLeds[i], potValue == i); 
                
    }  
}

void tryToUnlock() {
    activeColor = potValue;

    // Only do something if button has been release
    if (buttonReleased) {
        litLeds[progress] = true;
        enteredCode[progress] = activeColor;
        progress++;
        // Check if full code has been entered.
        if (progress == progressLedCount) {
            boolean correctCode = true;
            for(int i=0; i < progressLedCount; i++) {
                if (enteredCode[i] != code[i]) {
                    correctCode = false;
                }    
            }
            if (correctCode) {
                // Unlock the computer
                locked = false;
                Serial.print('u');
            } else {
                // Start over
                progress = 0;
                for(int i=0; i<progressLedCount; i++) {
                    enteredCode[i] = -1;
                }
            }

        }
    }
}
