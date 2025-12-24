# Smart Door Lock System (Arduino)

## Project Documentation

A microcontroller-based **smart door lock system** developed using an **Arduino Uno**.  
The system integrates **motion detection**, **keypad-based authentication**, **servo-controlled door actuation**, and **audio-visual feedback** to simulate a real-world electronic access control system.

**Control Loop:**  
Sense → Decide → Actuate

---

## 1. Project Overview

This project demonstrates a secure access control mechanism using an Arduino microcontroller.

When motion is detected using a PIR sensor, the system prompts the user to enter an **8-character password** through a 4×4 keypad. Based on the input, the system either grants or denies access while providing clear feedback via LEDs, a buzzer, and an LCD.

---

## 2. Features

### Authentication
- 8-character password-based access control
- Masked password entry using `*`
- Cancel (`*`) and submit (`#`) keypad controls
- Idle timeout during password entry

### Security
- Motion-triggered authentication
- Lockout after **3 failed attempts**
- **1-minute system lockout** after repeated failures

### Feedback & Actuation
- Dual servo motors for door opening and closing
- Green LED for access granted
- Red LED for locked/denied state
- Buzzer tones for success, error, and lockout
- 16×2 I²C LCD for system messages

---

## 3. Tech Stack

- **Programming Language:** Arduino C/C++
- **Microcontroller:** Arduino Uno
- **Sensors:** PIR motion sensor
- **Input Devices:** 4×4 matrix keypad
- **Actuators:** Servo motors
- **Output Devices:** LEDs, buzzer, I²C LCD

---

## 4. Project Structure

```text
smart-door-lock/
├─ src/
│  └─ smart_door_lock.ino
├─ docs/
│  └─ IO Systems.docx
├─ assets/
│  └─ images/
├─ README.md
├─ .gitignore
└─ LICENSE
```

---

## 5. Installation & Setup

### Hardware Requirements
- Arduino Uno
- PIR motion sensor
- 4×4 keypad
- 2 × Servo motors
- Buzzer
- Red & green LEDs
- 16×2 I²C LCD
- Breadboard and jumper wires

### Software Setup
1. Install **Arduino IDE**
2. Connect the Arduino board via USB
3. Install required libraries:
   - Servo
   - Keypad
   - Adafruit LiquidCrystal
4. Open the file:
   ```bash
   src/smart_door_lock.ino
   ```
5. Upload the sketch to the Arduino

---

## 6. Usage

1. Power the system.
2. Wait for motion detection.
3. Enter the password using the keypad.
4. Observe system response:
   - Access granted → door opens.
   - Access denied → warning feedback.
5. After inactivity or completion, system returns to standby.

---

## 7. Conclusion

The Smart Door Lock System successfully demonstrates how embedded systems combine **sensor input**, **decision logic**, and **physical actuation** to solve real-world security problems.

This project reflects practical skills in:
- Embedded programming
- Hardware interfacing
- Secure logic design
- User interaction and feedback

---

## License

Educational / Academic Use

