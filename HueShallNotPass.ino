/*
  Hue Shall Not Pass

  Unlock your computer in a safe-like way, using a potentiometer.

  This code is not safe since it requires storage of passwords in plaintext.
 */

// The amount of LEDs for showing unlock progress.
const int progressLedCount = 5;
const int progressLeds[] = {7, 6, 5, 4, 3};
const int potPin = A3;
const int colorLedCount = 5;
const int colorLeds[] = {8, 9, 10, 11, 12};

boolean locked = false;
int potValue = 0;
int progress = 0;
int activeColor = 0;
unsigned long activeSince = 0;
int code[] = {0, 1, 2, 0, 4};
int enteredCode[] = {-1,-1,-1,-1,-1};
boolean litLeds[] = {true, true, true, true, true};
// the setup routine runs once when you press reset:
void setup() {                
    for (int i = 0; i < progressLedCount; i++) {
        pinMode(progressLeds[i], OUTPUT);
    }
    for (int i = 0; i < progressLedCount; i++) {
        pinMode(colorLeds[i], OUTPUT);
    }
    pinMode(potPin, INPUT);
    Serial.begin(9600);
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
        digitalWrite(progressLeds[i], litLeds[i]); 
                
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
