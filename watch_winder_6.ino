#include <TM1637Display.h>
#include <Stepper.h>
#define STEPS 2048 // the number of steps in one revolution of your motor (28BYJ-48) 
#define BUTTON 2
#define DISP_CLK 6
#define DISP_DIO 5

// *************************************
// ******* VARIABLE DEFINITIONS ********
// *************************************
const float revBig = 1.5; // revolutions factor for one case revolution
const float oneRevTime = 10.0; // time of one revolution
int revNumbers[] = {750, 1000, 1500, 1800}; // revolutions number to full tension
const float dailyFactor = 0.5; // part of daily tension vs. full tension
// *************************************

Stepper stepper(STEPS, 8, 10, 9, 11);
TM1637Display disp(DISP_CLK, DISP_DIO);
int revFull;
int revDaily;
int pauseMinutesDaily;
int tempRevFull;
int tempRevDaily;
int tempPauseMinutesDaily;
unsigned long tempPauseSecondDaily;
int menuStep = 0;
int mode = -1;
int revDirection = 1;
int revDirectionMode = -1;
int revNumberMode = -1;

uint8_t dispCont[] = {SEG_A | SEG_F | SEG_E | SEG_D, SEG_G | SEG_E | SEG_D | SEG_C, SEG_E | SEG_G | SEG_C, SEG_F | SEG_G | SEG_E | SEG_D};
uint8_t dispC[] = {SEG_A | SEG_F | SEG_E | SEG_D, 0, 0, 0};
uint8_t dispFull[] = {SEG_F | SEG_A | SEG_G | SEG_E, SEG_E | SEG_D | SEG_C, SEG_B | SEG_C, SEG_B | SEG_C};
uint8_t dispF[] = {SEG_F | SEG_A | SEG_G | SEG_E, 0, 0, 0};
uint8_t dispDirMode_0[] = {SEG_G | SEG_B | SEG_C, SEG_F | SEG_E | SEG_G, 0, 0};
uint8_t dispDirMode_1[] = {SEG_F | SEG_E | SEG_G, SEG_F | SEG_E | SEG_G, 0, 0};
uint8_t dispDirMode_2[] = {SEG_G | SEG_B | SEG_C, SEG_G | SEG_B | SEG_C, 0, 0};
uint8_t dispDone[] = {SEG_B | SEG_C | SEG_D | SEG_E | SEG_G, SEG_G | SEG_C | SEG_D | SEG_E, SEG_C | SEG_E | SEG_G, SEG_A | SEG_D | SEG_E | SEG_F | SEG_G};
uint8_t dispCw[] = {SEG_F | SEG_E | SEG_G, SEG_F | SEG_E | SEG_G, SEG_F | SEG_E | SEG_G, SEG_F | SEG_E | SEG_G};
uint8_t dispCCw[] = {SEG_G | SEG_B | SEG_C, SEG_G | SEG_B | SEG_C, SEG_G | SEG_B | SEG_C, SEG_G | SEG_B | SEG_C};
uint8_t dispBothSides[] = {SEG_G | SEG_B | SEG_C, SEG_G | SEG_B | SEG_C, SEG_F | SEG_E | SEG_G, SEG_F | SEG_E | SEG_G};
uint8_t dispHelo[] = {SEG_G | SEG_B | SEG_C, SEG_G | SEG_B | SEG_C, SEG_F | SEG_E | SEG_G, SEG_F | SEG_E | SEG_G};

void setup() {
  //Serial.begin(9600);
  stepper.setSpeed(10);
  disp.setBrightness(0x0A);
  pinMode(BUTTON, INPUT_PULLUP);
  revNumberMode = sizeof(revNumbers) / sizeof(revNumbers[0]) - 1;
  setTempValuables();
}

void loop() {
  delay(500);

  if (menuStep == 0) {
    setMode();
  } else if (menuStep == 1) {
    setRevolutionNumber();
  } else if (menuStep == 2) {
    setDirection();
  } else if (menuStep == 3) {
    if (mode == 0) {
      doDailyTension();
    } else if (mode == 1) {
      doFullTension();
    }
  }

  if (digitalRead(BUTTON) == LOW) {
    menuStep++;
    if (menuStep > 3) {
      menuStep = 0;
      revDirectionMode = -1;
      revNumberMode = -1;
      mode = -1;
    }
  }
}

void doRevolution() {
  stepper.step(revDirection * STEPS * revBig);
  if (revDirectionMode == 0) {
    revDirection = revDirection * -1;
  }
}

void setTempValuables() {
  revDaily = revFull * dailyFactor;
  pauseMinutesDaily = 1440 - revDaily * oneRevTime / 60; // pause during daily revolutions
  tempRevDaily = revDaily;
  tempRevFull = revFull;
  tempPauseMinutesDaily = pauseMinutesDaily;
  tempPauseSecondDaily = pauseMinutesDaily * 60UL;
}

void setMode() {
  mode++;
  if (mode > 2) {
    mode = 0;
  }

  if (mode == 0) {
    disp.setSegments(dispCont, 4, 0);
  } else if (mode == 1) {
    disp.setSegments(dispFull, 4, 0);
  }
  delay(2000);
}

void setRevolutionNumber() {
  revNumberMode++;
  if (revNumberMode > sizeof(revNumbers) / sizeof(revNumbers[0]) - 1) {
    revNumberMode = 0;
  }

  revFull = revNumbers[revNumberMode];
  setTempValuables();
  if (mode == 0) {
    disp.showNumberDec(revDaily, false, 4, 0);
  } else {
    disp.showNumberDec(revFull, false, 4, 0);
  }
  delay(2000);
}

void setDirection() {
  revDirectionMode++;
  if (revDirectionMode > 2) {
    revDirectionMode = 0;
  }

  if (revDirectionMode == 0) {
    disp.setSegments(dispBothSides, 4, 0);
    revDirection = 1;
  } else if (revDirectionMode == 1) {
    disp.setSegments(dispCw, 4, 0);
    revDirection = -1;
  } else if (revDirectionMode == 2) {
    disp.setSegments(dispCCw, 4, 0);
    revDirection = 1;
  }
  delay(2000);
}

void doDailyTension() {
  if (tempRevDaily > 0) {
    if (tempRevDaily % 2 == 1) {
      disp.setSegments(dispC, 2, 0);
      if (revDirectionMode == 0) {
        disp.setSegments(dispDirMode_0, 2, 2);
      } else if (revDirectionMode == 1) {
        disp.setSegments(dispDirMode_1, 2, 2);
      } else if (revDirectionMode == 2) {
        disp.setSegments(dispDirMode_2, 2, 2);
      }
    } else {
      disp.showNumberDec(tempRevDaily, false, 4, 0);
    }
    doRevolution();
    tempRevDaily--;
  } else if (tempPauseSecondDaily == 0) {
    setTempValuables();
    delay(500);
  } else {
    int hours = tempPauseMinutesDaily / 60;
    int minutes = tempPauseMinutesDaily % 60;
    disp.showNumberDecEx(hours, 64, true, 2, 0);
    disp.showNumberDec(minutes, true, 2, 2);
    tempPauseSecondDaily--;
    if (tempPauseSecondDaily % 60UL == 0) {
      tempPauseMinutesDaily--;
    }
    delay(500);
  }
}

void doFullTension() {
  if (tempRevFull > 0) {
    if (tempRevFull % 2 == 1) {
      disp.setSegments(dispF, 2, 0);
      if (revDirectionMode == 0) {
        disp.setSegments(dispDirMode_0, 2, 2);
      } else if (revDirectionMode == 1) {
        disp.setSegments(dispDirMode_1, 2, 2);
      } else if (revDirectionMode == 2) {
        disp.setSegments(dispDirMode_2, 2, 2);
      }
    } else {
      disp.showNumberDec(tempRevFull, false, 4, 0);
    }
    doRevolution();
    tempRevFull--;
  } else {
    disp.setSegments(dispDone, 4, 0);
    delay(3000);
    menuStep = 0;
    revDirectionMode = -1;
    revNumberMode = -1;
    mode = -1;
  }
}
