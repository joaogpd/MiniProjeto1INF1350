#ifndef DEFS_H
#define DEFS_H

/**> Pino dos LEDs NeoPixel do olho direito */
#define NEOPIXELPINRIGHT 6 
/**> Pino dos LEDs NeoPixel do olho esquerdo */
#define NEOPIXELPINLEFT 7
/**> Pino do sensor PIR */
#define PIRPIN 2
/**> Pino do TX do DFMini Player (RX para o Arduino) */
#define RXPIN 10
/**> Pino do RX do DFMini Player (TX para o Arduino) */
#define TXPIN 11
/**> Valor da interrupção referente ao pino 2 (INT0) */
#define PIN2 0
/**> Valor da interrupção referente ao pino 3 (INT1) */
#define PIN3 1
/**> Número de LEDS NeoPixel em cada pino */
#define NUMPIXELS 2
/**> Número de músicas no cartão SD */
#define NUMSONGS 46
/**> Valor usado para ignorar o parâmetro de argumento da biblioteca SchedulerVTimer */
#define UNUSED 0
/**> Macro que limpa um bit de um valor recebido */
#define clearBit(reg, bit) (reg &= ~(1 << bit))

#endif
