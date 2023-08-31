#include <Adafruit_NeoPixel.h>
#include "SchedulerVTimer.h"

// Macro definitions
#define NEOPIXELPIN 6 
#define NUMPIXELS 2 
#define UNUSED 0
#define PIRPIN 2

// Object creation
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

// Function prototypes
void toggleEyes(void);
void ISR_PIR(void);
void allowPIRInterrupts(void);
void resetColor(void);

// Global variables
volatile bool eyesOn = false;
volatile uint32_t WHITE = pixels.Color(255, 255, 255);
volatile uint32_t RED = pixels.Color(255, 0, 0);
volatile uint32_t OFF = pixels.Color(0, 0, 0);
volatile uint32_t currentColor = WHITE;

void setup() {
  // Setup all tasks 
  setupTasks(vtimer, toggleEyes);
  setupTasks(vtimer, allowPIRInterrupts);
  setupTasks(vtimer, resetColor);
  // Initializion
  initSchedulerVTTimer();

  // Allows an interrupt to occur on the PIR sensor pin
  attachInterrupt(digitalPinToInterrupt(PIRPIN), ISR_PIR, FALLING);

  // Initialize NeoPixel object
  pixels.begin(); 

  for (uint8_t i = 0; i < NUMPIXELS; i++) 
    pixels.setPixelColor(i, currentColor);

  // Starts timer to toggle eyes every 1s
  startVTimer(toggleEyes, 1000, UNUSED);
}

void loop() {
  // Look for tasks to execute
  procTasks();
}

// Toggles the NeoPixel leds, using a global variable to check current state
void toggleEyes(void) {
  if (eyesOn) {
    for (uint8_t i = 0; i < NUMPIXELS; i++) 
      pixels.setPixelColor(i, OFF);
    pixels.show();
    eyesOn = false;
  } else {
    for (uint8_t i = 0; i < NUMPIXELS; i++) 
      pixels.setPixelColor(i, currentColor);
    pixels.show();
    eyesOn = true;
  }
  // Restarts timer to toggle eyes again in 1s
  startVTimer(toggleEyes, 1000, UNUSED);
}

// Handles an ISR interrupt, turning eyes red
void ISR_PIR(void) {
  // Turns eyes red
  currentColor = RED;
  for (uint8_t i = 0; i < NUMPIXELS; i++) 
    pixels.setPixelColor(i, currentColor);
  // Stops any interrupts on the PIR sensor pin
  detachInterrupt(digitalPinToInterrupt(PIRPIN));
  // Starts timer to allow interrupt again
  startVTimer(allowPIRInterrupts, 1000, UNUSED);
  // Starts timer to reset eye color to white
  startVTimer(resetColor, 10000, UNUSED);
}

// Resumes the interrupt handling on the PIR sensor pin
void allowPIRInterrupts(void) {
  attachInterrupt(digitalPinToInterrupt(PIRPIN), ISR_PIR, FALLING);
}

// Resets the eye color to white, on next toggleEyes timer expiry, color will change
void resetColor(void) {
  currentColor = WHITE;
  for (uint8_t i = 0; i < NUMPIXELS; i++) 
    pixels.setPixelColor(i, currentColor);
}

