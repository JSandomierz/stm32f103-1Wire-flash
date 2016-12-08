#ifndef INITSH_INCLUDED
#define INITSH_INCLUDED

#define BUFFER_SIZE 128
struct roundBuffer{
	char DATA[BUFFER_SIZE];
	char lastWritePos, lastReadPos;
}usartReadBuffer, usartWriteBuffer;

#define CMD_BUFFER_SIZE 32
struct Command{
	uint8_t loadedChars;
	char buffer[CMD_BUFFER_SIZE];
	int value;
}pendingCommand;
enum commandCode {C_LED, C_CONVERT, C_HELP};

//check to do conversion and send temp
uint8_t doConvert;

void TIMER_INIT();
void SysTick_Handler(void);
void GPIO_INIT();
void NVIC_INIT(void);
void USART_INIT();

#endif
