#include "stm32f10x.h"
#include "stm32f10x_usart.h"

#include "inits.h"
#include "usart.h"
#include "delay.h"
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
		if( !strcmp(command, "#tempverb") ){
			sscanf(pendingCommand.buffer, "%s %s", command, command);
			if( !strcmp(command, "on") ){
				printf2("\nTemp. verbose is on.\r\n");
			}else if( !strcmp(command, "off") ){
				printf2("\nTemp. verbose is off.\r\n");
			}else printf2("\nInvalid parameter for tempverb cmd\r\n");
		}else if( !strcmp(command, "#led") ){
			sscanf(pendingCommand.buffer, "%s %s %d", command, command, &value);
			if( !strcmp(command, "on") ){
				printf2("\nLED ON, blink: %d\r\n", value);
				blinkms = value;
				blinkActivated=1;
			}else if( !strcmp(command, "off") ){
				printf2("\nLED OFF\r\n");
				blinkActivated=0;
			}else printf2("\nInvalid parameter for LED cmd\r\n");
		}else if( !strcmp(command, "#convert") ){
			sscanf(pendingCommand.buffer, "%s %d", command, &value);
			printf2("\nCONVERT ON, t: %d\r\n", value);
			doConvert=0x01;
		}else if( !strcmp(command, "#read") ){
			sscanf(pendingCommand.buffer, "%s %d", command, &value);
			printf2("Sending %d readings...\r\n", value);
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
/*void USART_ReadCommand(){//interpret the data
	if(pendingCommand.loadedChars >= CMDLEN){
		//pendingCommand.loadedChars=0;
		strcpy(pendingCommand.buffer, "(00ABC123)");
		pendingCommand.buffer[CMDLEN] = '\0';
		int i;
		for(i=0;i<CMDLEN;i++){
			//printf2("\r\n%c\r\n", usartReadBuffer.DATA[usartReadBuffer.lastReadPos]);
			pendingCommand.buffer[ i ] = usartReadBuffer.DATA[usartReadBuffer.lastReadPos++];
			if(usartReadBuffer.lastReadPos>=BUFFER_SIZE) usartReadBuffer.lastReadPos=0;
			//if(usartReadBuffer.lastReadPos==usartReadBuffer.lastWritePos) return;//ERROR!
		}
		pendingCommand.loadedChars-=CMDLEN;
		printf2("\r\nMessage received: %s\r\n", pendingCommand.buffer);
		if( pendingCommand.buffer[0]=='(' && pendingCommand.buffer[CMDLEN-1]==')' ){
			char tmp[CMDLEN];
			tmp[2]='\0';
			tmp[3]='\0';

			strncpy( tmp, pendingCommand.buffer+1, 2 );
			if( !sscanf(tmp, "%d", &pendingCommand.no) ) pendingCommand.no=101;

			strncpy( tmp, pendingCommand.buffer + 3, 3 );
			if( !strcmp(tmp, "CON") ) pendingCommand.code=2;
			if( !strcmp(tmp, "SND") ) pendingCommand.code=3;
			if( !strcmp(tmp, "LED") ) pendingCommand.code=255;

			strncpy( tmp, pendingCommand.buffer + 6, 3 );
			if( !sscanf(tmp, "%d", &pendingCommand.value) ) pendingCommand.no=102;

			printf2("CMD: %d no %d, value: %d\r\n", pendingCommand.code, pendingCommand.no, pendingCommand.value);
		}else printf2("Nieprawidlowy format komendy - IDCMDVAL, XXCMDXXX\r\n");
	}
}*/
