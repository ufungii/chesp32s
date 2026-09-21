#ifndef INPUTS_H
#define INPUTS_H

#include <Arduino.h>
#include <XPT2046_Touchscreen.h>

// Joystick Pins
#define JOY_X_PIN        34  // ADC1_CH6
#define JOY_Y_PIN        35  // ADC1_CH7
#define JOY_SW_PIN       32  // Joystick Pushbutton (Active LOW)

// Action / Navigation Buttons
#define BTN_SEL          JOY_SW_PIN
#define BTN_OPT1         13  // Cancel / Deselect
#define BTN_BACK         14 // Return to Home Menu
#define BTN_OPT2         33 // Flip board / Reset (long press)

// ADC Thresholds
#define JOY_THRESH_LOW   1100
#define JOY_THRESH_HIGH  2700
#define BAT_SENSE_PIN 39 // Analog Input Only 

// Function Prototypes
void initInputs();
void processInputs();
int getBatteryPercentage();

#endif // INPUTS_H