#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

#include "onewire.h"
#include "usart.h"

void oneWireInit(){
	GPIO_InitTypeDef onewiregpio;
	onewiregpio.GPIO_Speed = GPIO_Speed_50MHz;
	onewiregpio.GPIO_Pin = GPIO_Pin_6;
	onewiregpio.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOC, &onewiregpio);
}

uint8_t oneWireReset(){
	uint8_t result = RESET;

	//GPIO_SetBits(GPIOC, GPIO_Pin_6);
	GPIO_ResetBits(GPIOC, GPIO_Pin_6);
	delay_us(480);

	GPIO_SetBits(GPIOC, GPIO_Pin_6);
	delay_us(70);
	if( GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6) == RESET ){
		result = SET;
	}
	delay_us(410);
	return result;
}

void writeBit(uint8_t bit){
	GPIO_ResetBits(GPIOC, GPIO_Pin_6);
	//delay_us(5);
	if(bit == RESET) delay_us(60);
	else{
		GPIO_SetBits(GPIOC, GPIO_Pin_6);
		delay_us(55);
	}
	GPIO_SetBits(GPIOC, GPIO_Pin_6);
}

uint8_t readBit(){
	uint8_t result;
	GPIO_ResetBits(GPIOC, GPIO_Pin_6);
	GPIO_SetBits(GPIOC, GPIO_Pin_6);
	delay_us(15);
	result = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6);
	delay_us(45);
	return result;
}

void writeByte(uint8_t byte){
	uint8_t mask = 0x01;
	int i=0;
	for(;i<8;i++){
		if( byte & (mask<<i) ) writeBit(SET);
		else writeBit(RESET);
	}
}

uint8_t readByte(){
	uint8_t result = 0x00;
	uint8_t mask = 0x01;
	int i=0;
	for(;i<8;i++){
		if( readBit()==SET ) result |= (mask<<i);
	}
	return result;
}

void readData(uint8_t* data){
	int i=0;
	for(;i<9;i++) data[i] = 0x00;

	//send_string("Rozpoczynam konwersje temperatury!\r\n");

	oneWireReset();
	writeByte(0xCC);
	writeByte(0x44);

	//while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6) == RESET){};
	//send_string("Konwersja zakonczona, pobieram dane!\r\n");
	delay_ms(1);

	oneWireReset();
	writeByte(0xCC);
	writeByte(0xBE);

	for(i=0;i<9;i++) data[i] = readByte();
}

uint16_t convertRawTempData(uint8_t* data){
    uint16_t result= 0x0000;
    int i;
    for(i=7;i>=0;i--){
        if( data[0] & (0x01 << (7-i)) ) result |= (0x0001  << (7-i));
        if( data[1] & (0x01 << (i)) ) result |= (0x0100 << (i));
    }
    /*for(i=7;i>=0;i--){
        if( data[1] & (0x01 << (i)) ) result |= (0x0100 << (7-i));
    }*/
    return result;
}

float getTemp(uint16_t rawTemp){
	float result=0;
    float x = 0.0625;
    uint16_t mask = 0x0001;

    int i;
	if(rawTemp & 0xF000){
        result = -64;
        //printf("Minus Temp\n");
        for(i=0;i<10;i++){
            if( rawTemp & (mask<<i) ) result+=x;
            x*=2;
        }
	}else{
	    for(i=0;i<11;i++){
            if( rawTemp & (mask<<i) ) result+=x;
            x*=2;
        }
	}
	printf2("rawTemp: %X\r\n", rawTemp);
	return result;
}

uint8_t isParasitePower(){
	oneWireReset();
	writeByte(0xCC);
	writeByte(0xB4);
	return readBit();
}

