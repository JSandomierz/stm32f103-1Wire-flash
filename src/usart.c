#include "stm32f10x.h"
#include "stm32f10x_usart.h"

#include "inits.h"
#include "usart.h"
#include "delay.h"
#include "flash.h"
#include "onewire.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

void USART2_IRQHandler(void){
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){
    	USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    	//read
		usartReadBuffer.DATA[usartReadBuffer.lastWritePos++] = USART_ReceiveData(USART2);
		if(usartReadBuffer.lastWritePos >= BUFFER_SIZE) usartReadBuffer.lastWritePos = 0;
    }
    if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET){
    	USART_ClearITPendingBit(USART2, USART_IT_TXE);
    	if(usartWriteBuffer.lastReadPos!=usartWriteBuffer.lastWritePos){
			USART_SendData(USART2, usartWriteBuffer.DATA[usartWriteBuffer.lastReadPos++]);
			if(usartWriteBuffer.lastReadPos>=BUFFER_SIZE) usartWriteBuffer.lastReadPos=0;
		}else{
			USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
		}
	}
}

void vprint(const char *fmt, va_list argp)
{
	char string[BUFFER_SIZE];
    if(0 < vsprintf(string,fmt,argp)) // build string
    {
    	USART_Write(string);
    }
}


void printf2(const char *fmt, ...) // custom printf() function
{
	//
    va_list argp;
    va_start(argp, fmt);
    vprint(fmt, argp);
    va_end(argp);
}

void USART_Write(char* dataString){
	int i;//use when needed to write data.
	USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	for(i=0;i<strlen(dataString);i++){
		usartWriteBuffer.DATA[usartWriteBuffer.lastWritePos++] = dataString[i];
		if(usartWriteBuffer.lastWritePos>=BUFFER_SIZE) usartWriteBuffer.lastWritePos=0;
	}
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
	if(USART_GetFlagStatus(USART2, USART_FLAG_TXE)){
		USART_SendData(USART2, usartWriteBuffer.DATA[usartWriteBuffer.lastReadPos++]);
		if(usartWriteBuffer.lastReadPos>=BUFFER_SIZE) usartWriteBuffer.lastReadPos=0;
	}
}

uint8_t USART_ReadCommand(){//interpret the data
	if(usartReadBuffer.lastReadPos!=usartReadBuffer.lastWritePos){
		if( usartReadBuffer.DATA[usartReadBuffer.lastReadPos] == '#' || pendingCommand.loadedChars>0){
			printf2("%c", usartReadBuffer.DATA[usartReadBuffer.lastReadPos]);
			while((usartReadBuffer.lastReadPos!=usartReadBuffer.lastWritePos)){
				if( usartReadBuffer.DATA[usartReadBuffer.lastReadPos] != '\r' ){
					pendingCommand.buffer[ pendingCommand.loadedChars++ ] = usartReadBuffer.DATA[usartReadBuffer.lastReadPos++];
					if(usartReadBuffer.lastReadPos>=BUFFER_SIZE) usartReadBuffer.lastReadPos=0;

					if( pendingCommand.loadedChars > CMD_BUFFER_SIZE-1 ){
						pendingCommand.loadedChars=0;
						break;
					}
				}else{
					pendingCommand.buffer[pendingCommand.loadedChars]='\0';
					pendingCommand.loadedChars=0;
					return 1;
				}
			}
		}else{
			usartReadBuffer.lastReadPos++;
			if(usartReadBuffer.lastReadPos>=BUFFER_SIZE) usartReadBuffer.lastReadPos=0;
			pendingCommand.buffer[0]='\0';
		}
	}
	return 0;
}

uint8_t interpretCommand(){
	if( USART_ReadCommand() ){
		char command[CMD_BUFFER_SIZE];
		int value=0;
		sscanf(pendingCommand.buffer, "%s", command);
		//printf2("\r\n%s\r\n", command);
		if( !strcmp(command, "#convert") ){
			blinkActivated=0x01;
			sscanf(pendingCommand.buffer, "%s %d", command, &value);
			printf2("\nCONVERT ON, t: %d\r\n", value);
			blinkms=value;
		}else if( !strcmp(command, "#read") ){
			//printf2("Sending %d readings...\r\n", value);
			int n;
			sscanf(pendingCommand.buffer, "%s %d", command, &n);
			int i;
			uint32_t currentAddress=FlashData.nextAddress;
			uint16_t currentData=0x00;
			printf2("\r\n");
			for(i=0;i<n;i++){
				currentAddress-=2;
				if( currentAddress<FLASH_USER_START ) currentAddress=FLASH_USER_END-1;
				currentData = readFlashPageHalfWord( currentAddress );
				if( currentData==0xFFFF ) break;
				float temp = getTemp(((uint16_t)currentData));
				int res = (int)temp;
				int rem = (int)((temp-res) * 10000);
				printf2("%08X: %04X, %d.%04d\r\n", currentAddress, currentData,  res, rem);
				delay_ms(30);//for proper display...
			}
		}else if( !strcmp(command, "#flash_reset") ){
			flashReset();
		}else if( !strcmp(command, "#help") ){
			sscanf(pendingCommand.buffer, "%s %s", command, command);
			if(strlen(command)==0) printf2("\nPolecenia: help, tempverb, convert, read, led.\r\n");
			else{
				if( !strcmp(command, "tempverb") ){
					printf2("\ntempverb on|off - informacje o temp. przy konwersji\r\n", value);
				}else if( !strcmp(command, "convert") ){
					printf2("\nconvert [czestotliwosc] - konwersja co n sekund.\r\n", value);
				}else if( !strcmp(command, "read") ){
					printf2("\nread [num] - pobranie n odczytow z pamieci.\r\n", value);
				}else if( !strcmp(command, "led") ){
					printf2("\nled [czestotliwosc] - miganie dioda co n ms.\r\n", value);
				}else printf2("\nPolecenia: help, tempverb, convert, read, led.\r\n");
			}
		}else printf2("\nNie rozpoznano polecenia!\r\n");
		pendingCommand.buffer[0] = '\0';
		pendingCommand.loadedChars=0;
	}
}
