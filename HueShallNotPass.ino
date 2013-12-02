/*
  Hue Shall Not Pass

  Unlock your computer in a safe-like way, using a potentiometer.

  This code is not safe since it requires storage of passwords in plaintext.
 */

// The amount of LEDs for showing unlock progress.
const int progressLedCount = 5;
const unsigned char colors[5][3] = {
    {255, 0, 0},
    {0, 100, 0},
    {0, 0, 255},
    {255, 20, 0},
    {255, 0, 255}
};
const unsigned char black[] = {0, 0, 0};
const unsigned char white[] = {255, 255, 255};

const int potPin = A3;
const int colorLedCount = 5;
const int colorLeds[] = {3, 4, 5, 6, 7};

boolean locked = false;
int potValue = 0;
int progress = 0;
int activeColor = 0;
unsigned long activeSince = 0;
int code[] = {0, 1, 2, 0, 4};
int enteredCode[] = {-1,-1,-1,-1,-1};
boolean litLeds[] = {true, true, true, true, true};

// You can choose the latch pin yourself.
const int ShiftPWM_latchPin=8;

// ** uncomment this part to NOT use the SPI port and change the pin numbers. This is 2.5x slower **
#define SHIFTPWM_NOSPI
const int ShiftPWM_dataPin = 11;
const int ShiftPWM_clockPin = 13;


// If your LED's turn on if the pin is low, set this to true, otherwise set it to false.
const bool ShiftPWM_invertOutputs = true;

// You can enable the option below to shift the PWM phase of each shift register by 8 compared to the previous.
// This will slightly increase the interrupt load, but will prevent all PWM signals from becoming high at the same time.
// This will be a bit easier on your power supply, because the current peaks are distributed.
const bool ShiftPWM_balanceLoad = false;

#include <ShiftPWM.h>   // include ShiftPWM.h after setting the pins!

// Here you set the number of brightness levels, the update frequency and the number of shift registers.
// These values affect the load of ShiftPWM.
// Choose them wisely and use the PrintInterruptLoad() function to verify your load.
// There is a calculator on my website to estimate the load.

unsigned char maxBrightness = 255;
unsigned char pwmFrequency = 75;
int numRegisters = 2;
int numRGBleds = 5;

void setup() {  
     
    Serial.begin(9600);

    // Sets the number of 8-bit registers that are used.
    ShiftPWM.SetAmountOfRegisters(numRegisters);

    // SetPinGrouping allows flexibility in LED setup. 
    // If your LED's are connected like this: RRRRGGGGBBBBRRRRGGGGBBBB, use SetPinGrouping(4).
    ShiftPWM.SetPinGrouping(1); //This is the default, but I added here to demonstrate how to use the funtion

    ShiftPWM.Start(pwmFrequency,maxBrightness);      
    for (int i = 0; i < colorLedCount; i++) {
        pinMode(colorLeds[i], OUTPUT);
    }
    pinMode(potPin, INPUT);
}

// the loop routine runs over and over again forever:
void loop() {
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
    for(int i=0; i<colorLedCount; i++){
        digitalWrite(colorLeds[i], potValue == i); 
                
    }  
}

void tryToUnlock() {
    if (potValue == activeColor && (millis() - activeSince) >= 1800 && progress < progressLedCount) {
        litLeds[progress] = true;
        enteredCode[progress] = activeColor;
        progress++;
        activeSince = millis();
        // Check if full code has been entered.
        if (progress == progressLedCount) {
            boolean correctCode = true;
            for(int i=0; i < progressLedCount; i++) {
                if (enteredCode[i] != code[i]) {
                    correctCode = false;
                }    
            }
            if (correctCode) {
                locked = false;
                Serial.print('u');
            } else {
                activeSince = millis();
                progress = 0;
                for(int i=0; i<progressLedCount; i++) {
                    enteredCode[i] = -1;
                }
            }

        }
    } else if (activeColor != potValue) {
        activeSince = millis();
        activeColor = potValue;
    }
}
