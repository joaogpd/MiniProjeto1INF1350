// Ard.Pin - 328.pin - Device Pin    -   Device
// 00  RXD    PD0       USB               USB Cable
// 01  TXD    PD1       USB               USB Cable
// 02  INT0   PD2       PIR               PIR
// 03~ INT1   PD3       DFPlayer-Busy     DFMini Busy (16)
// 04         PD4       
// 05~        PD5       
// 06~        PD6       NeoPixel          NeoPixel(in) 
// 07         PD7       NeoPixel          NeoPixel(in)
// 08         PB0       
// 09~        PB1          
// 10~        PB2       RX                DFMini TX
// 11~        PB3       TX                DFMini RX
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

// Instanciação de objetos
Adafruit_NeoPixel pixelsRight(NUMPIXELS, NEOPIXELPINRIGHT, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsLeft(NUMPIXELS, NEOPIXELPINLEFT, NEO_GRB + NEO_KHZ800);
SoftwareSerial dfMiniSerial(RXPIN, TXPIN);
DFRobotDFPlayerMini myDFPlayer;

// Protótipos de funções
void toggleEyes(void);
void resetColor(void);
void playNextSong(void);
void allowPinInterrupt(uint8_t source);
void stopPinInterrupt(uint8_t source);

// Variáveis globais
volatile bool eyesOn = false;
volatile const uint32_t WHITE = pixelsRight.Color(255, 255, 255);
volatile const uint32_t RED = pixelsRight.Color(255, 0, 0);
volatile const uint32_t OFF = pixelsRight.Color(0, 0, 0);
volatile uint32_t currentColor = WHITE;
volatile uint8_t nextSong = 1;
volatile bool currentlyPlaying = false;

void setup() {
  // Inicializa SWSerial para DF Mini Player
  dfMiniSerial.begin(9600);
  // Inicializa DFPMini Player
  myDFPlayer.begin(dfMiniSerial, true, false);
  // Realiza setup de tasks dos tipos app e vtimer no scheduler 
  setupTasks(vtimer, toggleEyes);
  setupTasks(app, playNextSong);
  // Inicializa o módulo de timer virtual
  initSchedulerVTTimer();

  // Permite uma interrupção no pino do sensor PIR
  allowPinInterrupt(PIN2);

  // Inicializa objetos do Neopixel LED
  pixelsRight.begin(); 
  pixelsLeft.begin();

  // Determina a cor inicial dos NeoPixel
  for (int i = 0; i < NUMPIXELS; i++)
    pixelsRight.setPixelColor(i, currentColor);
  for (int i = 0; i < NUMPIXELS; i++)
    pixelsLeft.setPixelColor(i, currentColor);

  // Inicializa um timer para piscar os olhos a cada 200ms
  startVTimer(toggleEyes, 200, UNUSED);
}

void loop() {
  // Busca tasks para executar 
  procTasks();
}

/**
  * Pisca os LEDs dos olhos. Caso esteja ligado, desliga, caso esteja desligado, liga.
  * No final da função reinicializa o timer de 200ms para piscar os olhos.
  */
void toggleEyes(void) {
  if (eyesOn) { 
    for (int i = 0; i < NUMPIXELS; i++)
      pixelsRight.setPixelColor(i, OFF);
    for (int i = 0; i < NUMPIXELS; i++)
      pixelsLeft.setPixelColor(i, OFF);
    pixelsRight.show();
    pixelsLeft.show();
    eyesOn = false;
  } else {
    for (int i = 0; i < NUMPIXELS; i++)
      pixelsRight.setPixelColor(i, currentColor);
    for (int i = 0; i < NUMPIXELS; i++)
      pixelsLeft.setPixelColor(i, currentColor);
    pixelsRight.show();
    pixelsLeft.show();
    eyesOn = true;
  }
  // Reinicializa o timer para piscar os olhos de novo em 200ms
  startVTimer(toggleEyes, 200, UNUSED);
}

/**
  * Altera a cor dos olhos para branco. Na próxima vez que o timer 
  * de toggleEyes expirar, a cor irá mudar.
  */.
void resetColor(void) {
  currentColor = WHITE;
  for (int i = 0; i < NUMPIXELS; i++)
    pixelsRight.setPixelColor(i, currentColor);
  for (int i = 0; i < NUMPIXELS; i++)
    pixelsLeft.setPixelColor(i, currentColor);
}

/** Toca a próxima música no DF Mini Player. Marca a próxima música para ser executada
  * e altera uma flag para mostrar que ela está tocando no momento. 
  */
void playNextSong(void) {
  myDFPlayer.play(nextSong);
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
  for (int i = 0; i < NUMPIXELS; i++)
    pixelsRight.setPixelColor(i, currentColor);
  for (int i = 0; i < NUMPIXELS; i++)
    pixelsLeft.setPixelColor(i, currentColor);
  // Make changes take effect
  pixelsLeft.show();
  pixelsRight.show();
  // Eyes are now turned on
  eyesOn = true;
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
  if (!source) {
    EICRA |= (1 << ISC01) | (1 << ISC00);
    EIMSK |= (1 << INT0);
  } else {
    EICRA |= (1 << ISC11) | (1 << ISC10);
    EIMSK |= (1 << INT1);
  }
}

// Stops INT0 or INT1 from happening
void stopPinInterrupt(uint8_t source) {
  if (!source)
    EIMSK = clearBit(EIMSK, INT0);
  else
    EIMSK = clearBit(EIMSK, INT1);
}
