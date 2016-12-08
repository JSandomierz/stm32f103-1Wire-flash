#ifndef DELAYH_INCLUDED
#define DELAYH_INCLUDED

//globals
volatile uint32_t usCount;

void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

//blinking LED
uint8_t blinkActivated;
uint16_t blinkms;
uint16_t currentms;

#endif
