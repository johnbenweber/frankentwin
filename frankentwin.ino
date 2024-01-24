#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <msTask.h>
#include <MsTimer2.h>
#include <Bounce2.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16 chars and 2 line display

// Pin definitions
const int buttonStartEvent1 = 3;
const int buttonStartEvent2 = 2;
const int buttonProgrammingMode = 4;
const int buttonIncreaseEvent1 = 5;
const int buttonDecreaseEvent1 = 6;
const int buttonIncreaseEvent2 = 7;
const int buttonDecreaseEvent2 = 8;
const int ledEvent1 = 13;
const int ledEvent2 = 13;

Bounce2::Button debouncer1 = Bounce2::Button();
Bounce2::Button debouncer2 = Bounce2::Button();
Bounce2::Button programDebouncer = Bounce2::Button();
Bounce2::Button timer1UpDebouncer = Bounce2::Button();
Bounce2::Button timer1DownDebouncer = Bounce2::Button();
Bounce2::Button timer2UpDebouncer = Bounce2::Button();
Bounce2::Button timer2DownDebouncer = Bounce2::Button();

// Event durations (in milliseconds)
unsigned long durationEvent1 = 5000;
unsigned long durationEvent2 = 5000;
unsigned long maxDuration = 99900;

unsigned long event1Start = 0;
unsigned long event2Start = 0;

unsigned long remaining1 = 0;
unsigned long remaining2 = 0;

// State variables
volatile bool event1Active = false;
volatile bool event2Active = false;
bool event1ActiveTemp = false;
bool event2ActiveTemp = false;
bool programmingMode = false;

void stopEvent1();
msTask task(durationEvent1, stopEvent1);

void setup() {
  debouncer1.attach(buttonStartEvent1,INPUT_PULLUP);
  debouncer2.attach(buttonStartEvent2,INPUT_PULLUP);
  programDebouncer.attach(buttonProgrammingMode,INPUT_PULLUP);
  timer1UpDebouncer.attach(buttonIncreaseEvent1,INPUT_PULLUP);
  timer1DownDebouncer.attach(buttonDecreaseEvent1,INPUT_PULLUP);
  timer2UpDebouncer.attach(buttonIncreaseEvent2,INPUT_PULLUP);
  timer2DownDebouncer.attach(buttonDecreaseEvent2,INPUT_PULLUP);
  
  debouncer1.interval(25);
  debouncer2.interval(25);
  programDebouncer.interval(25);
  timer1UpDebouncer.interval(25);
  timer1DownDebouncer.interval(25);
  timer2UpDebouncer.interval(25);
  timer2DownDebouncer.interval(25);
  
  debouncer1.setPressedState(LOW); 
  debouncer2.setPressedState(LOW);
  programDebouncer.setPressedState(LOW);
  timer1UpDebouncer.setPressedState(LOW);
  timer1DownDebouncer.setPressedState(LOW);
  timer2UpDebouncer.setPressedState(LOW);
  timer2DownDebouncer.setPressedState(LOW);

  msTask::init();
  
  lcd.init(); // Initialize the LCD
  lcd.backlight();

  updateLCD(programmingMode, durationEvent1, durationEvent2); // Update the LCD in each loop iteration
}

void loop() {
  debouncer1.update();
  debouncer2.update();

  // Event 1 start
  if (debouncer1.pressed() && !event1ActiveTemp && !programmingMode) {
      event1Active = true;
      startEvent1(event1Start, durationEvent1);
  }

  // Event 2 start
  if (debouncer2.pressed() && !event2ActiveTemp && !programmingMode) {
      event2Active = true;
      startEvent2(event2Start, durationEvent2);
  }

  noInterrupts();
  event1ActiveTemp = event1Active;
  event2ActiveTemp = event2Active;
  interrupts();
  
  timer1UpDebouncer.update();
  timer1DownDebouncer.update();
  timer2UpDebouncer.update();
  timer2DownDebouncer.update();
  programDebouncer.update();
  
  // Toggle programming mode
  if (programDebouncer.pressed() && event1ActiveTemp == false && event2ActiveTemp == false) {
      programmingMode = !programmingMode;
  }

  // Only adjust durations in programming mode
  if (programmingMode) {
    if (timer1UpDebouncer.pressed()) {
      adjustDuration(durationEvent1, 100);
    } else if (timer1DownDebouncer.pressed()) {
      adjustDuration(durationEvent1, -100);
    }

    if (timer2UpDebouncer.pressed()) {
      adjustDuration(durationEvent2, 100);
    } else if (timer2DownDebouncer.pressed()) {
      adjustDuration(durationEvent2, -100);
    }
  }

  if (event1ActiveTemp) {
    remaining1 = durationEvent1-(millis()-event1Start);
  } else {
    remaining1 = durationEvent1;
  }

  if (event2ActiveTemp) {
    remaining2 = durationEvent2-(millis()-event2Start);
  } else {
    remaining2 = durationEvent2;
  }

  if (millis() % 100 == 0) {
    updateLCD(programmingMode, remaining1, remaining2); // Update the LCD in each loop iteration
  }
}

void adjustDuration(unsigned long &eventDur, int change) {
  eventDur = max(0, min(eventDur+change, maxDuration)); // Ensure minimum 0
}

void startEvent1(unsigned long &e1Start, unsigned long durEvent1) {
  e1Start = millis();
  digitalWrite(ledEvent1, HIGH);
  task.setPeriod(durEvent1);
  task.start();
}

void stopEvent1() {
  event1Active = false;
  digitalWrite(ledEvent1, LOW);
  task.stop();
}

void startEvent2(unsigned long &e2Start, unsigned long durEvent2) {
  e2Start = millis();
  digitalWrite(ledEvent2, HIGH);
  MsTimer2::set(durEvent2, stopEvent2); // Set duration
  MsTimer2::start();
}

void stopEvent2() {
  event2Active = false;
  digitalWrite(ledEvent2, LOW);
  MsTimer2::stop();
}

void updateLCD(bool progMode, unsigned long rem1, unsigned long rem2) {
    
  lcd.setCursor(6, 0); // Set cursor to middle of the top line
  if (progMode) {
      lcd.print("PROG");
  } else {
      lcd.print("    ");
  }

  char time1[5];
  char time2[5];
  
  // Display Timer 1 status
  lcd.setCursor(0, 1); // Bottom left corner
  formatTime(time1, rem1);
  lcd.print(time1);

  // Display Timer 2 status
  lcd.setCursor(11, 1); // Bottom right corner
  formatTime(time2, rem2);
  lcd.print(time2);
}

void formatTime(char* timeStr, unsigned long milliseconds) {
  int sec = milliseconds/1000;
  int digit1 = sec/10;
  int digit2 = sec%10;
  int decimal = (milliseconds%1000)/100;

  if (digit1 != 0) {
    sprintf(timeStr, "%d%d.%ds", digit1, digit2, decimal);
  } else {
    sprintf(timeStr, " %d.%ds", digit2, decimal);
  }
}

