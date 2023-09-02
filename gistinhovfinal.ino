// Ard.Pin - 328.pin - Device Pin    -   Device
// 00  RXD    PD0       USB               USB Cable
// 01  TXD    PD1       USB               USB Cable
// 02  INT0   PD2       PIR               PIR
// 03~ INT1   PD3       DFPlayer-Busy     DFMini Busy (16)
// 04         PD4       RX                DFMini TX
// 05~        PD5       TX                DFMini RX
// 06~        PD6       NeoPixel          NeoPixel(in) 
// 07         PD7       NeoPixel          NeoPixel(in)
// 08         PB0       
// 09~        PB1          
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
#include <SoftwareSerial.h>
#include "SchedulerVTimer.h"
#include "DFRobotDFPlayerMini.h"

// Macro definitions
#define NEOPIXELPINRIGHT 6 
#define NEOPIXELPINLEFT 7
#define PIRPIN 2
#define RXPIN 4
#define TXPIN 5
#define NUMPIXELS 2
#define NUMSONGS 11
#define UNUSED 0
#define clearBit(reg, bit) (reg &= ~(1 << bit))

// Object creation
Adafruit_NeoPixel pixelsRight(NUMPIXELS, NEOPIXELPINRIGHT, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsLeft(NUMPIXELS, NEOPIXELPINLEFT, NEO_GRB + NEO_KHZ800);
SoftwareSerial dfMiniSerial(RXPIN, TXPIN);

// Function prototypes
void toggleEyes(void);
void resetColor(void);
void allowINT0Interrupt(void);
void stopINT0Interrupt(void);
void allowINT1Interrupt(void);
void stopINT1Interrupt(void);
void playNextSong(void);

// Global variables
volatile bool eyesOn = false;
volatile uint32_t WHITE = pixelsRight.Color(255, 255, 255);
volatile uint32_t RED = pixelsRight.Color(255, 0, 0);
volatile uint32_t OFF = pixelsRight.Color(0, 0, 0);
volatile uint32_t currentColor = WHITE;
volatile uint8_t nextSong = 1;
volatile bool currentlyPlaying = false;

void setup() {
  // Start Serial
  dfMiniSerial.begin(9600);
  // Setup all tasks 
  setupTasks(vtimer, toggleEyes);
  setupTasks(vtimer, resetColor);
  setupTasks(app, playNextSong);
  // Virtual timer module initialization
  initSchedulerVTTimer();

  // Allows an interrupt to occur on the PIR sensor pin
  allowINT0Interrupt();

  // Initialize NeoPixel object
  pixelsRight.begin(); 
  pixelsLeft.begin();

  // Set NeoPixel initial colors
  for (uint8_t i = 0; i < NUMPIXELS; i++) 
    pixelsRight.setPixelColor(i, currentColor);
  for (uint8_t i = 0; i < NUMPIXELS; i++) 
    pixelsLeft.setPixelColor(i, currentColor);

  // Post all app tasks
  postTask(app, playNextSong, UNUSED);
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
      pixelsRight.setPixelColor(i, OFF);
    for (uint8_t i = 0; i < NUMPIXELS; i++) 
      pixelsLeft.setPixelColor(i, OFF);
    pixelsRight.show();
    pixelsLeft.show();
    eyesOn = false;
  } else {
    for (uint8_t i = 0; i < NUMPIXELS; i++) 
      pixelsRight.setPixelColor(i, currentColor);
    for (uint8_t i = 0; i < NUMPIXELS; i++) 
      pixelsLeft.setPixelColor(i, currentColor);
    pixelsRight.show();
    pixelsLeft.show();
    eyesOn = true;
  }
  // Restarts timer to toggle eyes again in 1s
  startVTimer(toggleEyes, 200, UNUSED);
}

// Handles an ISR interrupt, turning eyes red
ISR (INT0_vect) {
  // Turns eyes red
  currentColor = RED;
  for (uint8_t i = 0; i < NUMPIXELS; i++) 
    pixelsRight.setPixelColor(i, currentColor);
  for (uint8_t i = 0; i < NUMPIXELS; i++) 
    pixelsLeft.setPixelColor(i, currentColor);
  // Stops any interrupts on the PIR sensor pin
  stopINT0Interrupt();
  // PLays next song after turning eyes red on next loop pass
  postTask(app, playNextSong, UNUSED);
}

// Resets the eye color to white. On next toggleEyes timer expiry, color will change
void resetColor(void) {
  currentColor = WHITE;
  for (uint8_t i = 0; i < NUMPIXELS; i++) 
    pixelsRight.setPixelColor(i, currentColor);
  for (uint8_t i = 0; i < NUMPIXELS; i++) 
    pixelsLeft.setPixelColor(i, currentColor);
}

// Plays next song on DF Mini Player, sets next song to be played and changes flag to show it is currently playing
void playNextSong(void) {
  dfMiniSerial.play(nextSong);
  nextSong++;
  if (nextSong > NUMSONGS)
    nextSong = 1;
  currentlyPlaying = true;
  // Allows an interrupt to occur when the busy pin (pin 3) rises to HIGH after song ends
  allowINT1Interrupt();
}

// Handles a busy pin interrupt (song over) and resets eye color
ISR (INT1_vect) {
  currentlyPlaying = false;
  // Resets eye color to white after song finishes
  resetColor();
  // Stops an interrupt from occuring on the busy pin (pin 3)
  stopINT1Interrupt();
  // Allows for PIR interrupts to occur
  allowINT0Interrupt();
}

// Allows for INT0 (pin 2) interrupt to happen
void allowINT0Interrupt(void) {
  // The rising edge of INT0 generates an interrupt request.
  EICRA |= (1 << ISC01) | (1 << ISC00);   
  // External Interrupt Request 0 Enable
  EIMSK |= (1 << INT0); 
}

// Allows for INT1 (pin 3) interrupt to happen
void allowINT1Interrupt(void) {
  // The rising edge of INT1 generates an interrupt request.
  EICRA |= (1 << ISC11) | (1 << ISC10);   
  // External Interrupt Request 1 Enable
  EIMSK |= (1 << INT1); 
}

// Stops INT0 (pin 2) interrupt from happening
void stopINT0Interrupt(void) {
  // External Interrupt Request 1 Disable
  EIMSK &= ~(1 << INT0);
  // EIMSK = clearBit(EIMSK, INT0);
}

// Stops INT1 (pin 3) interrupt from happening
void stopINT1Interrupt(void) {
  // External Interrupt Request 1 Disable
  EIMSK &= ~(1 << INT1);
  // EIMSK = clearBit(EIMSK, INT1);
}

