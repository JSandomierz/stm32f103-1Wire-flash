#ifndef USARTH_INCLUDED
#define USARTH_INCLUDED

void send_char(uint8_t c);
void send_raw(uint8_t byte);
void send_string(const char* s);

//void convertInt(int x);
void printf2(const char *fmt, ...);
void USART_Write(char* dataString);
void USART_WriteFromBuffer();
uint8_t USART_ReadCommand();

#endif
