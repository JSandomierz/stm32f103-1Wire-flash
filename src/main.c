#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"
#include "misc.h"

#include "inits.h"
#include "usart.h"
#include "delay.h"
#include "onewire.h"
#include "flash.h"

void readTemp(){
	if(blinkms > 10){
		if(currentms > blinkms){
			currentms=0;
			uint8_t data[9];
			uint16_t rawTemp = 0x00;
			if(readData(data)){
				rawTemp = convertRawTempData(data);
				float temp = getTemp(rawTemp);
				int res = (int)temp;
				int rem = (int)((temp-res) * 10000);
				//printf2("\r\nTemperatura wynosi: %d.%04d\r\n",res, rem);
				//printf2("%04X, %d.%04d\r\n", rawTemp,  res, rem);
				writeNextHalfWord( rawTemp );
			}
		}
	}
}

int main(void)
{
	//init functions
	NVIC_INIT();
	GPIO_INIT();
	USART_INIT();
	TIMER_INIT();
	flashInit();
	oneWireInit();

    while(1)
    {
    	interpretCommand();
    	readTemp();
    }
}
