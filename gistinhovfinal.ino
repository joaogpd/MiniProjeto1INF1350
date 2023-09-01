// Ard.Pin - 328.pin - Tabuleiro Pin -   Device
// 00  RXD    PD0       USB               USB Cable
// 01  TXD    PD1       USB               USB Cable
// 02  INT0   PD2       PIR               PIR
// 03~ INT1   PD3       DFPlayer-Busy     DFMini Busy (16)
// 04         PD4       
// 05~        PD5       
// 06~        PD6       NanoPixel         NanoPixel(in) 
// 07         PD7       
// 08         PB0       
// 09~        PB1       TX2               DFMini RX (2)+ Resistor 1k    
// 10~        PB2       
// 11~        PB3       
// 12         PB4       
// 13         PB5       
// A0         PC0       
// A1         PC1       
// A2         PC2       
// A3         PC3       
// A4         PC4       
// A5         PC5       

#include <Adafruit_NeoPixel.h>
#include "SchedulerVTimer.h"

// Macro definitions
#define NEOPIXELPIN 6 
#define NEOPIXELPIN2
#define NUMPIXELS 2 
#define UNUSED 0
#define PIRPIN 2

// Object creation
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXELPIN2, NEO_GRB + NEO_KHZ800);

// Function prototypes
void toggleEyes(void);
void ISR_PIR(void);
void allowPIRInterrupts(void);
void resetColor(void);
void allowINT0Interrupt(void);
void stopINT0Interrupt(void);

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
  allowINT0Interrupt();

  // Initialize NeoPixel object
  pixels.begin(); 
  pixels2.begin();

  for (uint8_t i = 0; i < NUMPIXELS; i++) 
    pixels.setPixelColor(i, currentColor);
  for (uint8_t i = 0; i < NUMPIXELS; i++) 
    pixels2.setPixelColor(i, currentColor);

  // Starts timer to toggle eyes every 1s
  startVTimer(toggleEyes, 200, UNUSED);
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
    for (uint8_t i = 0; i < NUMPIXELS; i++) 
      pixels2.setPixelColor(i, OFF);
    pixels.show();
    pixels2.show();
    eyesOn = false;
  } else {
    for (uint8_t i = 0; i < NUMPIXELS; i++) 
      pixels.setPixelColor(i, currentColor);
    for (uint8_t i = 0; i < NUMPIXELS; i++) 
      pixels2.setPixelColor(i, currentColor);
    pixels.show();
    pixels2.show();
    eyesOn = true;
  }
  // Restarts timer to toggle eyes again in 1s
  startVTimer(toggleEyes, 200, UNUSED);
}

// Handles an ISR interrupt, turning eyes red
// void ISR_PIR(void) {
ISR (INT0_vect) {
  // Turns eyes red
  currentColor = RED;
  for (uint8_t i = 0; i < NUMPIXELS; i++) 
    pixels.setPixelColor(i, currentColor);
  for (uint8_t i = 0; i < NUMPIXELS; i++) 
    pixels2.setPixelColor(i, currentColor);
  // Stops any interrupts on the PIR sensor pin
  stopINT0Interrupt();
  // Starts timer to allow interrupt again
  startVTimer(allowPIRInterrupts, 1000, UNUSED);
  // Starts timer to reset eye color to white
  startVTimer(resetColor, 10000, UNUSED);
}

// Resumes the interrupt handling on the PIR sensor pin
void allowPIRInterrupts(void) {
  allowINT0Interrupt();
}

// Resets the eye color to white, on next toggleEyes timer expiry, color will change
void resetColor(void) {
  currentColor = WHITE;
  for (uint8_t i = 0; i < NUMPIXELS; i++) 
    pixels.setPixelColor(i, currentColor);
  for (uint8_t i = 0; i < NUMPIXELS; i++) 
    pixels2.setPixelColor(i, currentColor);
}

void allowINT0Interrupt(void) {
  // Setup interrupt on pin 2 (INT0)
  EICRA |= (1 << ISC01) | (1 << ISC00);   // Trigger interrupt on rising
  EIMSK |= (1 << INT0); 
}

void stopINT0Interrupt(void) {
  // Stop interrupt on pin 2 (INT0)
  EIMSK &= ~(1 << INT0);
}

