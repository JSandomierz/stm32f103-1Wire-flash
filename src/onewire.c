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

uint8_t CRC_LUT[256]= {
 0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
 157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
 35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
 190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
 70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
 219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
 101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
 248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
 140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
 17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
 175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
 50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
 202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
 87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
 233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
 116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53};

 uint8_t calculateROMCRC(uint8_t* data, unsigned int dataLen){
	unsigned int i;
	uint8_t crcByte=0x00;
	for(i=0;i<8;++i){
        //printf("CRC: %d\n", crcByte);
        crcByte = CRC_LUT[data[i] ^ crcByte ];
	}
	return crcByte;
 }

uint8_t readData(uint8_t* data){
	int i=0;
	for(;i<9;i++) data[i] = 0x00;
	//printf2("RESET!\r\n");
	if( oneWireReset() ){
		writeByte(0xCC);
		writeByte(0x44);
		delay_ms(1);
		for( i=0;i<3;i++ ){
			if( oneWireReset() ){
				writeByte(0xCC);
				writeByte(0xBE);
				for(i=0;i<9;i++) data[i] = readByte();
				//printf2("CLC: %0.2X\r\n", calculateROMCRC(data, 8));
				//printf2("CRC: %0.2X\r\n", data[8]);
				if(!(calculateROMCRC(data, 8) ^ data[8])) return 1;
			}else return 0;
		}
	}
	return 0;
}

uint16_t convertRawTempData(uint8_t* data){
    uint16_t result= 0x0000;
    int i;
    for(i=7;i>=0;i--){
        if( data[0] & (0x01 << (7-i)) ) result |= (0x0001  << (7-i));
        if( data[1] & (0x01 << (i)) ) result |= (0x0100 << (i));
    }
    return result;
}

float getTemp(uint16_t rawTemp){
	float result=0;
    float x = 0.0625;
    uint16_t mask = 0x0001;

    int i;
	if(rawTemp & 0xF000){
        result = -64;
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
	return result;
}

uint8_t isParasitePower(){
	oneWireReset();
	writeByte(0xCC);
	writeByte(0xB4);
	return readBit();
}

