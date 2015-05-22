/*******************************************************************************
Precision tachometer
by Anna Jõgi a.k.a Libahunt
*******************************************************************************/
#include <LiquidCrystal.h>

/* SETTINGS */
const int roundsToCount = 30;/* 30 rounds is roughly 1.5 seconds around 1100rpm */
const unsigned long debounceTime = 5000;/* needs experimentation, probably upper limit is 5000*/

/* variables */
boolean falling = false;
int i, rpm, rounds, offPercentage, currState, prevState;
unsigned long debounceStart, intervalsSum, averageInterval, diff, maxDiff;
unsigned long fallingTimes[roundsToCount];
unsigned long intervals[roundsToCount-1];


/* initialize library objects */
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  
  DDRB = DDRB & ~0x01;//pin mode D8 is input
  Serial.begin(9600);
  Serial.println("#Carburettor adjustment tachometer#");
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.write("#Carb adj tach#");
  
}

void loop() {

  /*listen for falling edges for number of time specified in roundsToCount,
  fill fallingTimes array with microsecond timestamps*/
  rounds=0;
  while (rounds < roundsToCount) {
    currState = PINB & 0x01;//digitalRead pin 8;
    if (currState == 0 && prevState == 0x01) {
      falling = true;
      debounceStart = micros();
    }
    if (falling) {
      if ( micros()-debounceStart >= debounceTime ) { 
        fallingTimes[rounds] = debounceStart;
        rounds++;
        falling= false;
      }
    }
    prevState = currState;
  }

  /*calculate intervals between falling edges, average interval,
  maximum difference in percentages and the rpm */
  intervalsSum = 0;
  maxDiff = 0;
  for (i=1; i<roundsToCount; i++) {
    intervals[i-1] = fallingTimes[i] - fallingTimes[i-1];
    intervalsSum = intervalsSum + intervals[i-1];
  }
  averageInterval = intervalsSum/(roundsToCount-1);
  for (i=0; i<roundsToCount-1; i++) {
    if (intervals[i] < averageInterval) {
      diff = averageInterval - intervals[i];
    }
    else {
      diff = intervals[i] - averageInterval;
    }
    if (diff > maxDiff) {
      maxDiff = diff;
    }
    offPercentage = (int) diff*100/averageInterval;
    
    rpm = (int) (60000000/averageInterval);
    
  }
  
  /*output the numbers to Serial port*/
  Serial.print(rpm);
  Serial.print(" rpm ¬");
  Serial.print(offPercentage);
  Serial.println("%");
  
  /*output RPM to lcd */
  lcd.setCursor(0,1);
  lcd.print(rpm, DEC);
  
  delay(30);
  
}
    
