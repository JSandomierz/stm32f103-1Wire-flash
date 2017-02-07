#ifndef FLASHH_INCLUDED
#define FLASHH_INCLUDED

#define FLASH_PAGE_SIZE ((uint16_t)0x400)
#define FLASH_USER_START ((uint32_t)0x08008000)
#define FLASH_USER_END ((uint32_t)0x0801FFFF)
//#define FLASH_MAX_READINGS 10

struct FlashDataStruct{
	uint32_t nextAddress;
	uint16_t id;
}FlashData;

uint32_t readFlashPageWord(uint32_t address);
uint16_t readFlashPageHalfWord(uint32_t address);

void flashInit();
void flashReset();
void writeNextWord(uint16_t data);
void erasePage(uint32_t address);

#endif
