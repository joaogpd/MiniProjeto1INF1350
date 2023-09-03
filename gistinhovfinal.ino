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
#include "defs.h"

// Object creation
Adafruit_NeoPixel pixelsRight(NUMPIXELS, NEOPIXELPINRIGHT, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsLeft(NUMPIXELS, NEOPIXELPINLEFT, NEO_GRB + NEO_KHZ800);
SoftwareSerial dfMiniSerial(RXPIN, TXPIN);

// Function prototypes
void toggleEyes(void);
void resetColor(void);
void playNextSong(void);
void allowPinInterrupt(uint8_t source);
void stopPinInterrupt(uint8_t source);

// Global variables
volatile bool eyesOn = false;
volatile const uint32_t WHITE = pixelsRight.Color(255, 255, 255);
volatile const uint32_t RED = pixelsRight.Color(255, 0, 0);
volatile const uint32_t OFF = pixelsRight.Color(0, 0, 0);
volatile uint32_t currentColor = WHITE;
volatile uint8_t nextSong = 1;
volatile bool currentlyPlaying = false;

void setup() {
  // Start Serial
  dfMiniSerial.begin(9600);
  // Setup all tasks 
  setupTasks(vtimer, toggleEyes);
  setupTasks(app, playNextSong);
  // Virtual timer module initialization
  initSchedulerVTTimer();

  // Allows an interrupt to occur on the PIR sensor pin
  allowPinInterrupt(PIN2);

  // Initialize NeoPixel object
  pixelsRight.begin(); 
  pixelsLeft.begin();

  // Set NeoPixel initial colors
  pixelsRight.setPixelColor(0, currentColor);
  pixelsLeft.setPixelColor(0, currentColor);

  // Post all app tasks
  postTask(app, playNextSong, UNUSED);
  // Starts timer to toggle eyes every 1s
  startVTimer(toggleEyes, 200, UNUSED);
}

void loop() {
  // Looks for tasks to execute
  procTasks();
}

// Toggles the NeoPixel leds, using a global variable to check current state
void toggleEyes(void) {
  if (eyesOn) { 
    pixelsRight.setPixelColor(0, OFF);
    pixelsLeft.setPixelColor(0, OFF);
    pixelsRight.show();
    pixelsLeft.show();
    eyesOn = false;
  } else {
    pixelsRight.setPixelColor(0, currentColor);
    pixelsLeft.setPixelColor(0, currentColor);
    pixelsRight.show();
    pixelsLeft.show();
    eyesOn = true;
  }
  // Restarts timer to toggle eyes again in 1s
  startVTimer(toggleEyes, 200, UNUSED);
}

// Resets the eye color to white. On next toggleEyes timer expiry, color will change
void resetColor(void) {
  currentColor = WHITE;
  pixelsRight.setPixelColor(0, currentColor);
  pixelsLeft.setPixelColor(0, currentColor);
}

// Plays next song on DF Mini Player, sets next song to be played and changes flag to show it is currently playing
void playNextSong(void) {
  dfMiniSerial.play(nextSong);
  nextSong++;
  if (nextSong > NUMSONGS)
    nextSong = 1;
  currentlyPlaying = true;
  // Allows an interrupt to occur when the busy pin (pin 3) rises to HIGH after song ends
  allowPinInterrupt(PIN3);
}

// Handles an ISR interrupt, turning eyes red
ISR (INT0_vect) {
  // Turns eyes red
  currentColor = RED;
  pixelsRight.setPixelColor(0, currentColor);
  pixelsLeft.setPixelColor(0, currentColor);
  // Stops any interrupts on the PIR sensor pin
  stopPinInterrupt(PIN2);
  // Plays next song after turning eyes red on next loop pass
  postTask(app, playNextSong, UNUSED);
}

// Handles a busy pin interrupt (song over) and resets eye color
ISR (INT1_vect) {
  currentlyPlaying = false;
  // Resets eye color to white after song finishes
  resetColor();
  // Stops an interrupt from occuring on the busy pin (pin 3)
  stopPinInterrupt(PIN3);
  // Allows for PIR interrupts to occur
  allowPinInterrupt(PIN2);
}

// Allow for INT0 or INT1 interrupts to happen
void allowPinInterrupt(uint8_t source) {
  // Interrupt on rising 
  EICRA |= (1 << ISCx1(source)) | (1 << ISCx0(source));
  EIMSK |= (1 << INTx(source));
}

// Stops INT0 or INT1 from happening
void stopPinInterupt(uint8_t source) {
  EIMSK = clearBit(EIMSK, INTx(source));
}
