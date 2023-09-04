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
/**> Objeto referente ao NeoPixel localizado no olho direito */
Adafruit_NeoPixel pixelsRight(NUMPIXELS, NEOPIXELPINRIGHT, NEO_GRB + NEO_KHZ800);
/**> Objeto referente ao NeoPixel localizado no olho esquerdo */
Adafruit_NeoPixel pixelsLeft(NUMPIXELS, NEOPIXELPINLEFT, NEO_GRB + NEO_KHZ800);
/**> Objeto de Software Serial usado pelo DFPlayerMini */
SoftwareSerial dfMiniSerial(RXPIN, TXPIN);
/**> Objeto do DFPlayerMini */
DFRobotDFPlayerMini myDFPlayer;

// Protótipos de funções
void toggleEyes(void);
void resetColor(void);
void playNextSong(void);
void allowPinInterrupt(uint8_t source);
void stopPinInterrupt(uint8_t source);

// Variáveis globais
/**> Variável que determina se os LEDs dos olhos estão acesos ou não no momento */
volatile bool eyesOn = false;
/**> Valor que representa a cor branca na biblioteca do NeoPixel */
volatile const uint32_t WHITE = pixelsRight.Color(255, 255, 255);
/**> Valor que representa a cor vermelha na biblioteca do NeoPixel */
volatile const uint32_t RED = pixelsRight.Color(255, 0, 0);
/**> Valor que representa a ausência de cor na biblioteca do NeoPixel */
volatile const uint32_t OFF = pixelsRight.Color(0, 0, 0);
/**> Variável que guarda a cor atual dos olhos */
volatile uint32_t currentColor = WHITE;
/**> Contador da próxima música que deve tocar */
volatile uint8_t nextSong = 1;

/**
 * Função de setup do modelo de programação do Arduino, 
 * roda somente uma vez no início da execução.
 */
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

/**
 * Função de loop do modelo de programação do Arduino, 
 * roda enquanto o programa está ativo.
 */
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
 */
void resetColor(void) {
  currentColor = WHITE;
  for (int i = 0; i < NUMPIXELS; i++)
    pixelsRight.setPixelColor(i, currentColor);
  for (int i = 0; i < NUMPIXELS; i++)
    pixelsLeft.setPixelColor(i, currentColor);
}

/** 
 * Toca a próxima música no DF Mini Player. Marca a próxima música para ser executada
 * e altera uma flag para mostrar que ela está tocando no momento. Permite interrupção
 * no pino 3 (BUSY pin), que deve ser gerada quando a música terminar de tocar 
 */
void playNextSong(void) {
  myDFPlayer.play(nextSong);
  nextSong++;
  if (nextSong > NUMSONGS)
    nextSong = 1;
  // Permite uma interrupção ocorrer quando o pino busy (pino 3) muda para HIGH após a música terminar
  allowPinInterrupt(PIN3);
}

/**
 * Lida com uma interrupção de INT0 (pino 2). Muda a cor dos olhos para vermelho, para interrupções do 
 * sensor PIR e enfileira a task de tocar a próxima música.
 */
ISR (INT0_vect) {
  // Muda cor dos olhos para vermelho
  currentColor = RED;
  for (int i = 0; i < NUMPIXELS; i++)
    pixelsRight.setPixelColor(i, currentColor);
  for (int i = 0; i < NUMPIXELS; i++)
    pixelsLeft.setPixelColor(i, currentColor);
  // Faz mudanças tomarem efeito
  pixelsLeft.show();
  pixelsRight.show();
  // Olhos agora estão ligados
  eyesOn = true;
  // Para qualquer interrupção no pino do sensor PIR
  stopPinInterrupt(PIN2);
  // Toca a próxima música na próxima passada do loop
  postTask(app, playNextSong, UNUSED);
}


/**
 * Lida com uma interrupção de INT1 (pino 3). Reseta cor do olho para branco.
 */
// Handles a busy pin interrupt (song over) and resets eye color
ISR (INT1_vect) {
  // Muda a cor dos olhos para branco
  resetColor();
  // Para interrupções no busy pin (pino 3)
  stopPinInterrupt(PIN3);
  // Permite interrupções de PIR
  allowPinInterrupt(PIN2);
}

/**
 * Permite interrupções de INT0 ou INT1.
 * @param source fonte da interrupção, pode ser 0 ou 1
 */
void allowPinInterrupt(uint8_t source) {
  // Interrupção em rising
  if (!source) {
    EICRA |= (1 << ISC01) | (1 << ISC00);
    EIMSK |= (1 << INT0);
  } else {
    EICRA |= (1 << ISC11) | (1 << ISC10);
    EIMSK |= (1 << INT1);
  }
}

/**
 * Impede interrupções de INT0 ou INT1.
 * @param source fonte da interrupção, pode ser 0 ou 1
 */
void stopPinInterrupt(uint8_t source) {
  if (!source)
    EIMSK = clearBit(EIMSK, INT0);
  else
    EIMSK = clearBit(EIMSK, INT1);
}
