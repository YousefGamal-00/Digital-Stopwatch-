# Digital Stopwatch with Dual Mode

This project implements a **Digital Stopwatch** with two modes—**Increment Mode** and **Countdown Mode**—using an ATmega32 microcontroller and multiplexed seven-segment displays.

## 📋 **Project Features**

- **Increment Mode (Default):** Stopwatch counts up from zero.
- **Countdown Mode:** Counts down from a user-defined starting time.
- **Controls:**
  - **Reset**: Resets the stopwatch to its initial state.
  - **Pause/Resume**: Pauses and resumes time tracking.
  - **User Time Adjustment**: Easily set hours, minutes, and seconds.
- **Visual Feedback:**
  - **LED Indicators**: Display status changes.
  - **Buzzer**: Alarm sound when countdown reaches zero.

## 🛠️ **Hardware Used**
- ATmega32 Microcontroller
- Six multiplexed seven-segment displays
- Push buttons for control
- Buzzer and LED indicators

## ⚙️ **Technologies & Concepts Applied**
- Timer configuration in AVR microcontroller
- Multiplexing of seven-segment displays
- External interrupts for button controls
- Power optimization techniques

## 📖 **How It Works**
1. The stopwatch starts in **Increment Mode** and begins counting from 00:00:00.
2. The user can switch to **Countdown Mode** and set a custom start time.
3. Buttons allow for pausing, resuming, and resetting the timer, while the buzzer signals when the countdown reaches zero.
4. LED indicators provide visual feedback for different states (paused, running, or alarm).

## 🏗️ **Build Instructions**
1. Set up the ATmega32 microcontroller and connect it to the seven-segment displays.
2. Implement the button inputs using external interrupts.
3. Program the microcontroller using the provided C code for handling timer operations and multiplexing.
4. Connect the buzzer and LEDs for visual and audio feedback.

## 💡 **Lessons Learned**
- Effective use of timers and external interrupts in embedded systems.
- Optimizing performance while controlling multiple seven-segment displays using multiplexing.
- Designing a user-friendly interface with minimal hardware resources.

## 🙌 **Acknowledgments**
Special thanks to **Mohamed Tarek** for his guidance throughout this project.
