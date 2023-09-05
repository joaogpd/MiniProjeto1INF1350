/***************************************************************************
*
*  INF1350 - Sistemas Reativos
*  Prof.: Adriano Branco
*  Monitor: Pablo Nascimento
*  Semestre: 2023.2
*  Grupo 1: Cristiane Guimarães
*           João Gabriel Peixoto
*           Luiz Fernando Costa
*
*  PROJETO 1 - GISTEIN
*
***************************************************************************/


#ifndef DEFS_H
#define DEFS_H

#define NEOPIXELPINRIGHT 6 
#define NEOPIXELPINLEFT 7
#define PIRPIN 2
#define RXPIN 4
#define TXPIN 5
#define PIN2 0
#define PIN3 1
#define NUMPIXELS 1
#define NUMSONGS 11
#define UNUSED 0
#define clearBit(reg, bit) (reg &= ~(1 << bit))
#define ISCx1(x) ISC ## x ## 1
#define ISCx0(x) ISC ## x ## 0
#define INTx(x) INT ## x

#endif
