#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "delay.h"

void delay_us(uint32_t us){
	usCount=0;
	if( us>3 ) us*=0.38461538;
	//us*=0.5;
	while(usCount <= us){};
}
void delay_ms(uint32_t ms){
	while(--ms > 0) delay_us(1000);
}

void TIM2_IRQHandler()
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
        usCount++;
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}
