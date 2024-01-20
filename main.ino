#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <TimerOne.h>
#include <Bounce2.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD I2C address

int timer1Countdown;  
int timer2Countdown;  
int storedTimer1Countdown;
int storedTimer2Countdown;

bool timer1Active = false;
bool timer2Active = false;
bool programmingMode = false;

const int buttonPin1 = 2;  // Timer1 button
const int buttonPin2 = 3;  // Timer2 button
const int programButtonPin = 4;  // Programming mode button
const int timer1UpButtonPin = 5;  // Timer1 increment
const int timer1DownButtonPin = 6; // Timer1 decrement
const int timer2UpButtonPin = 7;   // Timer2 increment
const int timer2DownButtonPin = 8; // Timer2 decrement

Bounce debouncer1 = Bounce(); 
Bounce debouncer2 = Bounce();
Bounce programDebouncer = Bounce();
Bounce timer1UpDebouncer = Bounce();
Bounce timer1DownDebouncer = Bounce();
Bounce timer2UpDebouncer = Bounce();
Bounce timer2DownDebouncer = Bounce();

void setup() {
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(programButtonPin, INPUT_PULLUP);
  pinMode(timer1UpButtonPin, INPUT_PULLUP);
  pinMode(timer1DownButtonPin, INPUT_PULLUP);
  pinMode(timer2UpButtonPin, INPUT_PULLUP);
  pinMode(timer2DownButtonPin, INPUT_PULLUP);

  debouncer1.attach(buttonPin1); debouncer1.interval(25);
  debouncer2.attach(buttonPin2); debouncer2.interval(25);
  programDebouncer.attach(programButtonPin); programDebouncer.interval(25);
  timer1UpDebouncer.attach(timer1UpButtonPin); timer1UpDebouncer.interval(25);
  timer1DownDebouncer.attach(timer1DownButtonPin); timer1DownDebouncer.interval(25);
  timer2UpDebouncer.attach(timer2UpButtonPin); timer2UpDebouncer.interval(25);
  timer2DownDebouncer.attach(timer2DownButtonPin); timer2DownDebouncer.interval(25);

  EEPROM.get(0, storedTimer1Countdown);
  EEPROM.get(sizeof(storedTimer1Countdown), storedTimer2Countdown);
  timer1Countdown = storedTimer1Countdown;
  timer2Countdown = storedTimer2Countdown;

  lcd.init();                      
  lcd.backlight();

  Timer1.initialize(1000000);       
  Timer1.attachInterrupt(timerISR); 

  lcd.setCursor(0, 0);
  lcd.print("Timer1:");
  lcd.setCursor(0, 1);
  lcd.print("Timer2:");
}

void loop() {
  debouncer1.update();
  debouncer2.update();
  programDebouncer.update();
  timer1UpDebouncer.update();
  timer1DownDebouncer.update();
  timer2UpDebouncer.update();
  timer2DownDebouncer.update();

  if (programDebouncer.read() == HIGH) {
    programmingMode = true;
  } else {
    programmingMode = false;
  }

  if (programmingMode) {
    adjustTimers();
  } else {
    startStopTimers();
  }
}

void adjustTimers() {
  if (timer1UpDebouncer.fell()) {
    timer1Countdown++;
  }
  if (timer1DownDebouncer.fell()) {
    timer1Countdown = max(timer1Countdown - 1, 0);
  }
  if (timer2UpDebouncer.fell()) {
    timer2Countdown++;
  }
  if (timer2DownDebouncer.fell()) {
    timer2Countdown = max(timer2Countdown - 1, 0);
  }
}

void startStopTimers() {
  if (debouncer1.fell()) {
    timer1Active = !timer1Active;
    if (timer1Active && timer1Countdown != storedTimer1Countdown) {
      EEPROM.put(0, timer1Countdown);
      storedTimer1Countdown = timer1Countdown;
    }
  }
  if (debouncer2.fell()) {
    timer2Active = !timer2Active;
    if (timer2Active && timer2Countdown != storedTimer2Countdown) {
      EEPROM.put(sizeof(timer1Countdown), timer2Countdown);
      storedTimer2Countdown = timer2Countdown;
    }
  }
}

void timerISR() {
  // Timer ISR code remains the same
}

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <TimerOne.h>
#include <Bounce2.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD I2C address

int timer1Countdown;  
int timer2Countdown;  
int storedTimer1Countdown;
int storedTimer2Countdown;

bool timer1Active = false;
bool timer2Active = false;
bool programmingMode = false;

const int buttonPin1 = 2;  // Timer1 button
const int buttonPin2 = 3;  // Timer2 button
// ... Other button definitions

const int fetPin1 = 9;  // FET control pin for Timer1
const int fetPin2 = 10; // FET control pin for Timer2

Bounce debouncer1 = Bounce(); 
Bounce debouncer2 = Bounce();
// ... Other debouncers

void setup() {
  // ... Setup code for buttons and LCD

  pinMode(fetPin1, OUTPUT);
  pinMode(fetPin2, OUTPUT);

  // ... Other setup code, including EEPROM
}

void loop() {
  // ... Loop code for button updates and mode handling

  if (programmingMode) {
    adjustTimers();
  } else {
    startStopTimers();
  }
}

void adjustTimers() {
  // ... Adjust timer logic
}

void startStopTimers() {
  if (debouncer1.fell()) {
    timer1Active = !timer1Active;
    digitalWrite(fetPin1, timer1Active ? HIGH : LOW); // Control FET for Timer1
    if (timer1Active && timer1Countdown != storedTimer1Countdown) {
      EEPROM.put(0, timer1Countdown);
      storedTimer1Countdown = timer1Countdown;
    }
  }
  if (debouncer2.fell()) {
    timer2Active = !timer2Active;
    digitalWrite(fetPin2, timer2Active ? HIGH : LOW); // Control FET for Timer2
    if (timer2Active && timer2Countdown != storedTimer2Countdown) {
      EEPROM.put(sizeof(timer1Countdown), timer2Countdown);
      storedTimer2Countdown = timer2Countdown;
    }
  }
}

void timerISR() {
  // Timer ISR code remains the same, but add FET control when timers end
  if (timer1Active) {
    timer1Countdown--;
    // ... Display update for Timer1
    if (timer1Countdown <= 0) {
      timer1Active = false;
      digitalWrite(fetPin1, LOW); // Turn off FET for Timer1
      // ... LCD update for Timer1 completion
    }
  }
  if (timer2Active) {
    timer2Countdown--;
    // ... Display update for Timer2
    if (timer2Countdown <= 0) {
      timer2Active = false;
      digitalWrite(fetPin2, LOW); // Turn off FET for Timer2
      // ... LCD update for Timer2 completion
    }
  }
}
