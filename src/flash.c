#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include <string.h>
#include "flash.h"

uint32_t readFlashPageWord(uint32_t address){
	return (*(__IO uint32_t*) (address));
}

uint16_t readFlashPageHalfWord(uint32_t address){
	return (*(__IO uint16_t*) (address));
}

void flashInit(){
	FlashData.id=0;
	FlashData.nextAddress=FLASH_USER_START+2;
	//determine lastData position
	uint32_t i=FlashData.nextAddress;
	uint32_t tmp;
	uint8_t found=0x00;
	while(i<=FLASH_USER_END){
		tmp = readFlashPageHalfWord( i );
		if(tmp == 0xFFFF){
			found=0xFF;
			break;
		}
		i+=2;
	}
	if(!found){//wszystkie strony zajete, kasuje pierwsza.
		erasePage( FLASH_USER_START );
		FlashData.nextAddress=FLASH_USER_START;
	}
	//printf2("INIT addr: %08X\r\n", i);
	FlashData.nextAddress=i;
}

void writeNextHalfWord(uint16_t data){
	FLASH_UnlockBank1();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

	if(FlashData.nextAddress>FLASH_USER_END){//powrot do poczatku
		FlashData.nextAddress=FLASH_USER_START;
		erasePage( FLASH_USER_START );
	}else{
		if(FlashData.nextAddress%FLASH_PAGE_SIZE==0){
			erasePage( FlashData.nextAddress );
		}
	}
	FLASH_Status status;
	/*while( (status=FLASH_ProgramHalfWord( FlashData.nextAddress, data ))!=FLASH_COMPLETE ){
		switch(status){
		case FLASH_BUSY:
			printf2("FLASH BUSY, %08X\r\n", FlashData.nextAddress);
			break;
		case FLASH_ERROR_PG:
			printf2("FLASH ERROR PG, %08X\r\n", FlashData.nextAddress);
			break;
		case FLASH_ERROR_WRP:
			printf2("FLASH ERROR WRP, %08X\r\n", FlashData.nextAddress);
			break;
		case FLASH_TIMEOUT:
			printf2("FLASH TIMEOUT, %08X\r\n", FlashData.nextAddress);
			break;
		}
		delay_ms(500);
	}
	if(status==FLASH_COMPLETE) printf2("FLASH COMPLETE, %08X\r\n", FlashData.nextAddress);*/
	status=FLASH_ProgramHalfWord( FlashData.nextAddress, data );
	FLASH_LockBank1();

	if(status==FLASH_COMPLETE){
		//printf2("FLASH COMPLETE, %08X\r\n", FlashData.nextAddress);
		FlashData.nextAddress+=2;
	}
}

void erasePage(uint32_t address){
	FLASH_UnlockBank1();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	while( FLASH_ErasePage(FLASH_USER_START)!=FLASH_COMPLETE ){delay_ms(1);}
	FLASH_LockBank1();
}

void flashReset(){//wymazuje wszystkie strony uzytkownika
	FLASH_UnlockBank1();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	/*uint32_t i=FLASH_USER_START;
	for(;i<FLASH_USER_END;i+=FLASH_PAGE_SIZE)
		erasePage(i);*/
	erasePage(FLASH_USER_START);
	FLASH_LockBank1();
}
