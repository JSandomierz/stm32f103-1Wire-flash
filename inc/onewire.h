#ifndef ONEWIREH_INCLUDED
#define ONEWIREH_INCLUDED

//GPIO_InitTypeDef onewiregpio;

struct simple_float{
	int num;
	int rem;
};

void oneWireInit();
uint8_t oneWireReset();

void writeBit(uint8_t bit);
uint8_t readBit();

void writeByte(uint8_t byte);
uint8_t readByte();

uint8_t checkCRC(uint8_t *binData);
uint8_t readData(uint8_t* data);

uint8_t isParasitePower();

uint16_t convertRawTempData(uint8_t* data);
float getTemp(uint16_t rawTemp);

#endif
