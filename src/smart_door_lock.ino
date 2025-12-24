/*
  Smart Door Lock System (Arduino Uno)
  -----------------------------------
  Second-year friendly, professional structure.

  Features:
  - PIR motion detection (wake the system)
  - 4x4 keypad password entry (8 chars), masked as '*'
  - I2C 16x2 LCD status messages
  - Dual servo door actuation (open/close)
  - Green/Red LED feedback
  - Buzzer tones (success/error/lockout)
  - Lockout after 3 failed attempts
  - Idle timeout during password entry

  Wiring (default pins):
  - PIR: D4
  - Keypad rows: D6 D7 D8 D9
  - Keypad cols: D10 D11 D12 D13
  - Servo 1: A2
  - Servo 2: A3
  - Green LED: D3
  - Red LED: D2
  - Buzzer: D5
  - LCD: I2C (SDA/SCL)
*/

#include <Arduino.h>
#include <Keypad.h>
#include <Servo.h>
#include <Adafruit_LiquidCrystal.h>

// ----------------------- Pin map -----------------------
const int PIN_PIR      = 4;
const int PIN_LED_RED  = 2;
const int PIN_LED_GRN  = 3;
const int PIN_BUZZER   = 5;

const int PIN_SERVO_1  = A2;
const int PIN_SERVO_2  = A3;

// Keypad (4x4) pins (D6-D13)
const byte ROWS = 4;
const byte COLS = 4;

byte rowPins[ROWS] = {6, 7, 8, 9};
byte colPins[COLS] = {10, 11, 12, 13};

// Typical 4x4 keypad layout
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// LCD (I2C)
Adafruit_LiquidCrystal lcd(0);

// Servos
Servo servo1;
Servo servo2;

// ----------------------- Config -----------------------
const char PASSWORD[] = "12345678";          // 8-char password (change this)
const int  PASS_LEN   = 8;

const unsigned long IDLE_TIMEOUT_MS = 10000; // 10s timeout for keypad entry
const unsigned long LOCKOUT_MS      = 15000; // 15s lockout after 3 failed attempts

const int MAX_ATTEMPTS = 3;

// Servo angles (adjust to your build)
const int SERVO1_CLOSED = 0;
const int SERVO1_OPEN   = 90;

const int SERVO2_CLOSED = 180;
const int SERVO2_OPEN   = 90;

const int SERVO_STEP_DELAY_MS = 10;          // smooth movement speed

// ----------------------- State -----------------------
enum class State {
  IDLE,
  PROMPT,
  ENTRY,
  GRANTED,
  DENIED,
  LOCKED
};

State state = State::IDLE;

char inputBuffer[PASS_LEN + 1]; // +1 for null terminator
int  inputIndex = 0;

int failedAttempts = 0;

unsigned long entryStartMs = 0;
unsigned long lockoutStartMs = 0;

// ----------------------- Helpers -----------------------
void ledsOff() {
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_GRN, LOW);
}

void ledGranted() {
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_GRN, HIGH);
}

void ledDenied() {
  digitalWrite(PIN_LED_GRN, LOW);
  digitalWrite(PIN_LED_RED, HIGH);
}

void beepSuccess() {
  tone(PIN_BUZZER, 1200, 150);
  delay(170);
  tone(PIN_BUZZER, 1500, 150);
  delay(200);
  noTone(PIN_BUZZER);
}

void beepError() {
  tone(PIN_BUZZER, 300, 300);
  delay(350);
  noTone(PIN_BUZZER);
}

void beepLockout() {
  for (int i = 0; i < 3; i++) {
    tone(PIN_BUZZER, 250, 250);
    delay(300);
  }
  noTone(PIN_BUZZER);
}

void lcdHeader(const char* line1, const char* line2 = "") {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

bool isMotionDetected() {
  return digitalRead(PIN_PIR) == HIGH;
}

void resetEntry() {
  inputIndex = 0;
  inputBuffer[0] = '\0';
  entryStartMs = millis();
}

void printMaskedProgress() {
  lcd.setCursor(0, 1);
  for (int i = 0; i < PASS_LEN; i++) {
    lcd.print(i < inputIndex ? '*' : ' ');
  }
}

bool isPasswordCorrect() {
  for (int i = 0; i < PASS_LEN; i++) {
    if (inputBuffer[i] != PASSWORD[i]) return false;
  }
  return true;
}

void moveServoSmooth(Servo& s, int fromDeg, int toDeg) {
  if (fromDeg == toDeg) return;
  int step = (toDeg > fromDeg) ? 1 : -1;
  for (int pos = fromDeg; pos != toDeg; pos += step) {
    s.write(pos);
    delay(SERVO_STEP_DELAY_MS);
  }
  s.write(toDeg);
}

void openDoor() {
  moveServoSmooth(servo1, SERVO1_CLOSED, SERVO1_OPEN);
  moveServoSmooth(servo2, SERVO2_CLOSED, SERVO2_OPEN);
}

void closeDoor() {
  moveServoSmooth(servo1, SERVO1_OPEN, SERVO1_CLOSED);
  moveServoSmooth(servo2, SERVO2_OPEN, SERVO2_CLOSED);
}

// ----------------------- State transitions -----------------------
void goIdle() {
  ledsOff();
  closeDoor();
  resetEntry();
  state = State::IDLE;
  lcdHeader("System Idle", "Waiting motion");
}

void goPrompt() {
  ledsOff();
  resetEntry();
  state = State::PROMPT;
  lcdHeader("Enter Password:", "");
  printMaskedProgress();
}

void goGranted() {
  state = State::GRANTED;
  ledGranted();
  lcdHeader("Access Granted", "Opening...");
  beepSuccess();
  openDoor();
  delay(2000);
  lcdHeader("Door Open", "Closing soon");
  delay(2000);
  closeDoor();
  delay(500);
  goIdle();
}

void goDenied() {
  state = State::DENIED;
  ledDenied();
  lcdHeader("Access Denied", "Try again");
  beepError();
  delay(1500);

  failedAttempts++;
  if (failedAttempts >= MAX_ATTEMPTS) {
    lockoutStartMs = millis();
    state = State::LOCKED;
    lcdHeader("LOCKED OUT", "Wait 15 sec");
    beepLockout();
    ledDenied();
  } else {
    goPrompt();
  }
}

void handleKeypad() {
  char key = keypad.getKey();
  if (!key) return;

  entryStartMs = millis();

  if (key == '*') {
    lcdHeader("Cancelled", "Returning...");
    delay(800);
    goIdle();
    return;
  }

  if (key == '#') {
    if (inputIndex < PASS_LEN) {
      lcdHeader("Password short", "Enter 8 chars");
      delay(1000);
      lcdHeader("Enter Password:", "");
      printMaskedProgress();
      return;
    }
    if (isPasswordCorrect()) {
      failedAttempts = 0;
      goGranted();
    } else {
      goDenied();
    }
    return;
  }

  if (inputIndex < PASS_LEN) {
    inputBuffer[inputIndex++] = key;
    if (inputIndex == PASS_LEN) inputBuffer[PASS_LEN] = '\0';
    printMaskedProgress();
  } else {
    tone(PIN_BUZZER, 600, 50);
  }
}

void handleTimeout() {
  if (state == State::PROMPT || state == State::ENTRY) {
    if (millis() - entryStartMs > IDLE_TIMEOUT_MS) {
      lcdHeader("Timeout", "Returning...");
      delay(800);
      goIdle();
    }
  }
}

void handleLockout() {
  unsigned long elapsed = millis() - lockoutStartMs;
  if (elapsed >= LOCKOUT_MS) {
    failedAttempts = 0;
    lcdHeader("Unlocked", "Try again");
    delay(800);
    goIdle();
  } else {
    unsigned long remaining = (LOCKOUT_MS - elapsed) / 1000;
    lcd.setCursor(0, 1);
    lcd.print("Wait ");
    lcd.print(remaining);
    lcd.print(" sec   ");
  }
}

void setup() {
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GRN, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  ledsOff();
  noTone(PIN_BUZZER);

  servo1.attach(PIN_SERVO_1);
  servo2.attach(PIN_SERVO_2);
  servo1.write(SERVO1_CLOSED);
  servo2.write(SERVO2_CLOSED);

  lcd.begin(16, 2);
  goIdle();
}

void loop() {
  if (state == State::LOCKED) {
    handleLockout();
    return;
  }

  if (state == State::IDLE) {
    if (isMotionDetected()) {
      lcdHeader("Motion Detected", "Authenticating");
      delay(700);
      goPrompt();
      state = State::ENTRY;
    }
    return;
  }

  if (state == State::PROMPT || state == State::ENTRY) {
    handleKeypad();
    handleTimeout();
    return;
  }
}
