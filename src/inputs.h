#ifndef INPUTS_H
#define INPUTS_H

#include <Arduino.h>
#include <XPT2046_Touchscreen.h>

#define TOUCH_CS  0

// Directional & Main Action Pins
#define BTN_UP    13
#define BTN_DOWN  14
#define BTN_LEFT  27
#define BTN_RIGHT 26
#define BTN_SEL   25

// Extra Action Pins
#define BTN_OPT1  32
#define BTN_OPT2  4
#define BTN_BACK  33

#define MIN_PRESSURE 400

#define TS_MINX 300
#define TS_MAXX 3700
#define TS_MINY 300
#define TS_MAXY 3800

extern XPT2046_Touchscreen ts;

void initInputs();
void processInputs();

#endif