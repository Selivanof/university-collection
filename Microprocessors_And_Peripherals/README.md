<div align="center">
<h3 align="center">Microprocessors And Peripherals</h3>

  <a href=""> ![Static Badge](https://img.shields.io/badge/Language-C-teal)</a>
  <a href=""> ![Static Badge](https://img.shields.io/badge/Language-Assembly-purple)</a>
  <a href=""> ![Static Badge](https://img.shields.io/badge/Semester-8-red)</a>
</div>

This repository contains the assignments that I completed for the Microprocessors and Peripherals 2023-2024 Course, as part of my undergraduate studies at Aristotle University of Thessaloniki. This was a team assignment, in collaboration with:
- [Lamprinos Chatziioannou](https://github.com/chatziiola)

|   Course Information     |                                  |
|--------------------------|----------------------------------|
| Semester                 | 8                                |
| Attendance Year          | 2023-2024                        |
| Project Type             | Mandatory                        |
| Team Project             | Yes                              |
| Language Used            | ARM Assembly, C                  |
| Software Used            | Keil uVision, Visual Studio Code |

Team Members: 

## Description

During this course, 3 assignments were given to us:

### 1. ARM Assembly Routines

A C function receives a string from the user using UART. Then 3 ARM Assembly routines are called that perform the following actions:
- Calculate a hash from the string
- Calculate the digital root of the hash
- Calculate the sum (N + N-1 + ... + 2 + 1) of the digital root (N)

### 2. GPIO, Timers and Interrupts

A C function is always ready to receive a number from the user using UART. 
Based on the UART input:
- If the number is odd, set a timer to blink the LED (0.5s interval)
- If the number is even, stop the blink timer and keep the current state

When an external button is pressed:

- Stop the blink timer, toggle the LED and print the total number of presses using UART

### 3. Final Project: Sensors

This project required a driver for the DHT11 sensor to be written by us, using the information available on its datasheet.

A C function is ready to receive a number from the user using UART at the start of the program (once). 

Every 2 seconds, the temperature is measured and printed using UART. Based on the measurement:

- If the temperature is greater than 25C, turn on the LED.
- If the temperature is less than 20C, turn off the LED.
- If the temperature is between 20C and 25C, blink the LED.

If the touch sensor is activated, change the temperature measuring interval based on the initial input of the user.

## Notes

Some headers that are used in our .c files (e.g. gpio.h, uart.h, ...) were provided to us by the professor. These files are **not** included in this repository.