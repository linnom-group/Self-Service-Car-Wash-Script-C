#include <Wire.h> 
#include <TM1637Display.h>
#include <OLED_I2C.h>


OLED myOLED(SDA, SCL, 8);
extern uint8_t RusFont[];
extern uint8_t MegaNumbers[];
extern uint8_t MediumNumbers[];
extern uint8_t SmallFont[];
extern uint8_t logo[];
extern uint8_t logo2[];
extern uint8_t mybee[];

const int waterPin = 8;
const int foamPin = 10;
const int pausePin = 9;
#define RELAY_WATER 4
#define RELAY_FOAM 5

const int CLK_PIN = 2;
const int DIO_PIN = 3;
TM1637Display display(CLK_PIN, DIO_PIN);

String inputString = "";
bool isInputComplete = false;

int inputValue = 0;
int vtime = 0;
static unsigned long startTime = 0;
bool isWaterActive = false;
bool isFoamActive = false;
bool isPauseActive = false;
bool isPauseCounting = false;
int pauseTime = 0;
bool isPauseFinished = false;
int savedTime = 0;

const int WATER_FUNCTION = 1;
const int FOAM_FUNCTION = 2;
int interruptedFunction = 0;
bool isPauseInterrupted = false; // added variable



void setup() { 
  Serial.begin(9600);
  myOLED.begin();
  myOLED.setBrightness(127); 
  pinMode(waterPin, INPUT_PULLUP);
  pinMode(foamPin, INPUT_PULLUP);
  pinMode(pausePin, INPUT_PULLUP);
  pinMode(RELAY_WATER, OUTPUT);
  pinMode(RELAY_FOAM, OUTPUT);
  display.setBrightness(7);
  display.clear(); 
  myOLED.clrScr();
  myOLED.drawBitmap(0, 0, mybee, 128, 64);
  myOLED.update();
  delay(3000);
  myOLED.clrScr();
  myOLED.drawBitmap(0, 0, logo, 128, 64);
  myOLED.update();
}

void loop() {
  int waterState = digitalRead(waterPin);
  int foamState = digitalRead(foamPin);
  int pauseState = digitalRead(pausePin);

  if (Serial.available() > 0) {
    char incomingChar = Serial.read();
    if (incomingChar == '\n') {
      isInputComplete = true;
    } else {
      inputString += incomingChar;
    }
  }

  if (isInputComplete) {
    inputValue = inputString.toInt();
    vtime = inputValue * 9;
    display.showNumberDec(vtime); 
    myOLED.clrScr();
    myOLED.setFont(MegaNumbers);
    myOLED.printNumI(vtime, CENTER, 20);  
    myOLED.update();
    inputString = "";
    isInputComplete = false;
  }

  if (waterState == LOW && !isWaterActive && !isFoamActive && !isPauseActive) {
    activateWater();
  }

  if (foamState == LOW && !isWaterActive && !isFoamActive && !isPauseActive) {
    activateFoam();
  }

  if (pauseState == LOW && !isWaterActive && !isFoamActive && !isPauseActive && !isPauseCounting) {
    activatePause();
  }
}

void activateWater() {
  startTime = millis();
  isWaterActive = true;
  digitalWrite(RELAY_WATER, HIGH);
  digitalWrite(RELAY_FOAM, LOW);

  while (vtime > 0 && isWaterActive) {
    unsigned long currentTime = millis();
    if (currentTime - startTime >= 1000) {
      startTime = currentTime;
      vtime--;
      display.showNumberDec(vtime);
      myOLED.setFont(RusFont);
      myOLED.print("DJLF", CENTER, 1);
      myOLED.setFont(MegaNumbers);
      myOLED.printNumI(vtime, CENTER, 20);  
      myOLED.update();
      myOLED.clrScr();
    }

    if (digitalRead(foamPin) == LOW || digitalRead(pausePin) == LOW) {
      isWaterActive = false;
      isPauseInterrupted = true;
      interruptedFunction = WATER_FUNCTION;
      break;
    }
  }

  if (isWaterActive) {
    digitalWrite(RELAY_WATER, LOW);
    display.showNumberDec(0);
    isWaterActive = false;
  }
}

void activateFoam() {
  startTime = millis();
  isFoamActive = true;
  digitalWrite(RELAY_WATER, LOW);
  digitalWrite(RELAY_FOAM, HIGH);

  while (vtime > 0 && isFoamActive) {
    unsigned long currentTime = millis();
    if (currentTime - startTime >= 400) {
      startTime = currentTime;
      vtime--;
      display.showNumberDec(vtime);
      myOLED.setFont(RusFont);
      myOLED.print("GTYF", CENTER, 1);
      myOLED.setFont(MegaNumbers);
      myOLED.printNumI(vtime, CENTER, 20);  
      myOLED.update();
      myOLED.clrScr();
    }

    if (digitalRead(waterPin) == LOW || digitalRead(pausePin) == LOW) {
      isFoamActive = false;
      isPauseInterrupted = true;
      interruptedFunction = FOAM_FUNCTION;
      break;
    }
  }

  if (isFoamActive) {
    digitalWrite(RELAY_FOAM, LOW);
    display.showNumberDec(0);
    isFoamActive = false;
  }
}

void activatePause() {
  isPauseActive = true;
  digitalWrite(RELAY_WATER, LOW);
  digitalWrite(RELAY_FOAM, LOW);
  isPauseCounting = true;
  unsigned long pauseStartTime = millis();
  pauseTime = 5;
  display.showNumberDec(pauseTime);

  while (isPauseCounting) {
    unsigned long currentTime = millis();
    if (currentTime - pauseStartTime >= 1000) {
      pauseStartTime = currentTime;
      pauseTime--;
      display.showNumberDec(pauseTime);
      myOLED.clrScr();
      myOLED.setFont(RusFont);
      myOLED.print("GFEPF", CENTER, 1);
      myOLED.setFont(MegaNumbers);
      myOLED.printNumI(pauseTime, CENTER, 20);  
      myOLED.update();
      myOLED.clrScr();
    }

    if (digitalRead(waterPin) == LOW || digitalRead(foamPin) == LOW) {
      isPauseActive = false;
      isPauseCounting = false;
      isPauseInterrupted = false;
      break;
    }

    if (pauseTime == 0) {
      isPauseCounting = false;
      isPauseFinished = true;
    }
  }

  if (isPauseActive && isPauseFinished) {
    isPauseActive = false;
    isPauseFinished = false;
    display.showNumberDec(0);
    myOLED.setFont(MegaNumbers);
    myOLED.print("0", CENTER, 20);  
    myOLED.update();
    myOLED.clrScr();
    if (!isPauseInterrupted) {
      if (savedTime > 0) {
        vtime = savedTime;
        savedTime = 0;

        if (interruptedFunction == WATER_FUNCTION) {
          activateWater();
        } else if (interruptedFunction == FOAM_FUNCTION) {
          activateFoam();
        }
      }
    } else {
      if (interruptedFunction == WATER_FUNCTION) {
        activateWater();
      } else if (interruptedFunction == FOAM_FUNCTION) {
        activateFoam();
      }
    }
  }
}
