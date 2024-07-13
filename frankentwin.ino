 #include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <msTask.h>
#include <Bounce2.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16 chars and 2 line display

// Pin definitions
const int pinSwitch1 = 2;
const int pinSwitch2 = 3;
const int pinProgMode = 4;
const int pin1Up = 5;
const int pin1Dn = 6;
const int pin2Up = 7;
const int pin2Dn = 8; 
const int pinOutput1 = 11;
const int pinOutput2 = 12;

Bounce2::Button buttonSwitch1 = Bounce2::Button();
Bounce2::Button buttonSwitch2 = Bounce2::Button();
Bounce2::Button buttonProgMode = Bounce2::Button();
Bounce2::Button button1Up = Bounce2::Button();
Bounce2::Button button1Dn = Bounce2::Button();
Bounce2::Button button2Up = Bounce2::Button();
Bounce2::Button button2Dn = Bounce2::Button();

// Event durations (in milliseconds)
unsigned long defaultDuration = 5000;
unsigned long maxDuration = 99900;

unsigned long output1Duration;
unsigned long output2Duration;

unsigned long output1Start = 0;
unsigned long output2Start = 0;

unsigned long output1Remaining = 0;
unsigned long output2Remaining = 0;

// State variables
bool output1Active = false;
bool output2Active = false;
bool progMode = false;
int output1Mode = 0;
int output2Mode = 0;
int output1ModeDisp = 0;
int output2ModeDisp = 0;

void stopOutput1();
void stopOutput2();

msTask timer1Task(defaultDuration, stopOutput1);
msTask timer2Task(defaultDuration, stopOutput2);

void setup() {
  pinMode(pinOutput1, OUTPUT);
  pinMode(pinOutput2, OUTPUT);  
  
  buttonSwitch1.attach(pinSwitch1,INPUT_PULLUP);
  buttonSwitch2.attach(pinSwitch2,INPUT_PULLUP);
  buttonProgMode.attach(pinProgMode,INPUT_PULLUP);
  button1Up.attach(pin1Up,INPUT_PULLUP);
  button1Dn.attach(pin1Dn,INPUT_PULLUP);
  button2Up.attach(pin2Up,INPUT_PULLUP);
  button2Dn.attach(pin2Dn,INPUT_PULLUP);
  
  buttonSwitch1.interval(25);
  buttonSwitch2.interval(25);
  buttonProgMode.interval(25);
  button1Up.interval(25);
  button1Dn.interval(25);
  button2Up.interval(25);
  button2Dn.interval(25);
  
  buttonSwitch1.setPressedState(LOW); 
  buttonSwitch2.setPressedState(LOW);
  buttonProgMode.setPressedState(LOW);
  button1Up.setPressedState(LOW);
  button1Dn.setPressedState(LOW);
  button2Up.setPressedState(LOW);
  button2Dn.setPressedState(LOW);

  EEPROM.get(0, output1Duration);
  EEPROM.get(sizeof(output1Duration), output2Duration);
  EEPROM.get(sizeof(output1Duration)+sizeof(output2Duration), output1Mode);
  EEPROM.get(sizeof(output1Duration)+sizeof(output2Duration)+sizeof(output1Mode), output2Mode);

  if ((output1Duration < 0) || (output1Duration > maxDuration)) {
    output1Duration = defaultDuration;
  }
  
  if ((output2Duration < 0) || (output2Duration > maxDuration)) {
    output2Duration = defaultDuration;
  }

  if ((output1Mode < 0) || (output1Mode > 2)) {
    output1Mode = 0;
  }
  
  if ((output2Mode < 0) || (output2Mode > 2)) {
    output2Mode = 0;
  }


  msTask::init();
  timer1Task.start();
  timer2Task.start();
  
  lcd.init(); // Initialize the LCD
  lcd.backlight();

  updateLCD(progMode, output1Mode, output1Duration, output2Mode, output2Duration); // Update the LCD in each loop iteration
}

void loop() {
  buttonSwitch1.update();
  buttonSwitch2.update();

  output1Active = digitalRead(pinOutput1);
  output2Active = digitalRead(pinOutput2);

  // Output 1 start
  if (buttonSwitch1.pressed() && !output1Active && !progMode) {
    output1Start = millis();
    digitalWrite(pinOutput1, HIGH);
    if (output1Mode == 0) { // Timed mode
      timer1Task.setPeriod(output1Duration);
      timer1Task.start();
    } else {
      timer1Task.setPeriod(maxDuration);
      timer1Task.start();
    }
  } else if (buttonSwitch1.released() && output1Active && output1Mode == 1) {
    stopOutput1();
  } else if (buttonSwitch1.pressed() && output1Active && output1Mode == 2) {
    stopOutput1();
  }

  // Output 2 start
  if (buttonSwitch2.pressed() && !output2Active && !progMode) {
    output2Start = millis();
    digitalWrite(pinOutput2, HIGH);
    if (output2Mode == 0) { // Timed mode
      timer2Task.setPeriod(output2Duration);
      timer2Task.start();
    } else {
      timer2Task.setPeriod(maxDuration);
      timer2Task.start();
    }
  } else if (buttonSwitch2.released() && output2Active && output2Mode == 1) {
    stopOutput2();
  } else if (buttonSwitch2.pressed() && output2Active && output2Mode == 2) {
    stopOutput2();
  }
  
  button1Up.update();
  button1Dn.update();
  button2Up.update();
  button2Dn.update();
  buttonProgMode.update();
  
  // Toggle programming mode
  if (buttonProgMode.pressed() && !output1Active && !output2Active) {
    progMode = !progMode;
    EEPROM.put(0, output1Duration);
    EEPROM.put(sizeof(output1Duration), output2Duration); 
  }

  // Toggle continuous mode
  if (button1Up.pressed() && !output1Active && !progMode) {
    if (output1Mode == 1) {
      output1Mode = 2;
    } else {
      output1Mode = 1;
    }
  } else if (button1Dn.pressed() && !output1Active && !progMode) {
    output1Mode = 0;
  }

  // Toggle continuous mode
  if (button2Up.pressed() && !output2Active && !progMode) {
    if (output2Mode == 1) {
      output2Mode = 2;
    } else {
      output2Mode = 1;
    }
  } else if (button2Dn.pressed() && !output2Active && !progMode) {
    output2Mode = 0;
  }

  EEPROM.put(sizeof(output1Duration)+sizeof(output2Duration), output1Mode);
  EEPROM.put(sizeof(output1Duration)+sizeof(output2Duration)+sizeof(output1Mode), output2Mode);

  // Only adjust durations in programming mode
  if (progMode) {
    if (button1Up.pressed()) {
      adjustDuration(output1Duration, 100);
    } else if ((button1Up.read() == LOW) && (button1Up.currentDuration() > 1000) && (millis() % 100 == 0)) {
        adjustDuration(output1Duration, 250);
    } else if (button1Dn.pressed()) {
      adjustDuration(output1Duration, -100);
    } else if ((button1Dn.read() == LOW) && (button1Dn.currentDuration() > 1000) && (millis() % 100 == 0)) {
      adjustDuration(output1Duration, -250);
    }

    if (button2Up.pressed()) {
      adjustDuration(output2Duration, 100);
    } else if ((button2Up.read() == LOW) && (button2Up.currentDuration() > 1000) && (millis() % 100 == 0)) {
      adjustDuration(output2Duration, 250);
    } else if (button2Dn.pressed()) {
      adjustDuration(output2Duration, -100);
    } else if ((button2Dn.read() == LOW) && (button2Dn.currentDuration() > 1000) && (millis() % 100 == 0)) {
      adjustDuration(output2Duration, -250);
    }
  }

  if (output1Active && output1Mode == 0) { // Timer mode
    output1Remaining = output1Duration-(millis()-output1Start);
  } else { // Idle
    output1Remaining = output1Duration;
  }

  if (output2Active && output2Mode == 0) { // Timer mode
    output2Remaining = output2Duration-(millis()-output2Start);
  } else { // Idle
    output2Remaining = output2Duration;
  }

  if (millis() % 100 == 0) {
    if (progMode) {
      output1ModeDisp = 0;
      output2ModeDisp = 0;
    } else {
      output1ModeDisp = output1Mode;
      output2ModeDisp = output2Mode;
    }
    updateLCD(progMode, output1ModeDisp, output1Remaining, output2ModeDisp, output2Remaining); // Update the LCD in each loop iteration
  }
}
 
void adjustDuration(unsigned long &eventDur, int change) { 
  if (eventDur+change < 0){
    eventDur = maxDuration;
  } else if (eventDur+change > maxDuration) {
    eventDur = 0;
  } else {
    eventDur = eventDur+change;
  }
}

void stopOutput1() {
  digitalWrite(pinOutput1, LOW);
}

void stopOutput2() {
  digitalWrite(pinOutput2, LOW);
}

void updateLCD(bool progMode, int mode1, unsigned long rem1, int mode2, unsigned long rem2) {
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
  if (mode1 == 0 && rem1 <= maxDuration) {
    formatTime(time1, rem1);
    lcd.print(time1);
  } else if (mode1 == 1) {
    lcd.print(" CONT");
  } else if (mode1 == 2) {
    lcd.print(" HOLD");
  }

  // Display Timer 2 status
  lcd.setCursor(11, 1); // Bottom right corner
  if (mode2 == 0 && rem2 <= maxDuration) {
    formatTime(time2, rem2);
    lcd.print(time2);
  } else if (mode2 == 1) {
    lcd.print(" CONT");
  } else if (mode2 == 2) {
    lcd.print(" HOLD");
  }
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
