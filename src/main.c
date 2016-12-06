#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "misc.h"

#include "inits.h"
#include "usart.h"
#include "delay.h"
#include "onewire.h"

/*

int __io_putchar(int c)
{
 send_char(c);
 return c;
}*/


int main(void)
{
	//init functions
	NVIC_INIT();
	GPIO_INIT();
	USART_INIT();
	TIMER_INIT();
	oneWireInit();

	uint8_t data[9];
	//send_string("Sprawdzam tryb zasilania:\r\n");
	/*if( isParasitePower() ) send_string("Tryb pasozytniczy\r\n");
	else send_string("Osobna linia zasilania\r\n");

	send_string("Czytam dane z pamieci termometru...\r\n");*/
	//readData(data);
	/*int i;
	for(i=0;i<9;i++){
		send_raw(data[i]);
		send_string("\r\n");
	}*/
	/*float temp = getTemp(convertRawTempData(data));
	int res = (int)temp;
	int rem = (int)((temp-res) * 10000);
	printf2("Temperatura wynosi: %d.%04d\r\n",res, rem);*/


	//printf("Temperatura wynosi: %.4f\r\n", getTemp(data));


    while(1)
    {
    	/*USART_Write("Hello world!\r\n");
    	//usartReadBuffer.DATA[126]='\r';
    	//usartReadBuffer.DATA[127]='\n';
    	//USART_Write( usartReadBuffer.DATA );
    	USART_WriteFromBuffer();
    	*/
    	//if(oneWireReset()) blink();
    	interpretCommand();
    	blink();
    	if( doConvert ){
    		doConvert=0x00;
    		readData(data);
    		float temp = getTemp(convertRawTempData(data));
			int res = (int)temp;
			int rem = (int)((temp-res) * 10000);
			printf2("Temperatura wynosi: %d.%04d\r\n",res, rem);
    	}
    }
}
