#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"
#include "misc.h"

#include "inits.h"

void TIMER_INIT(){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseInitTypeDef tim;

	TIM_TimeBaseStructInit(&tim);
	tim.TIM_CounterMode = TIM_CounterMode_Up;
	tim.TIM_Prescaler = 0;
	tim.TIM_Period = 63;
	TIM_TimeBaseInit(TIM2, &tim);

	//TIM_SetCompare1(TIM2, 100);//calculate the channel I for USART send

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	TIM_Cmd(TIM2, ENABLE);

	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = TIM2_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	SysTick_Config(SystemCoreClock / 1000);
}

void SysTick_Handler(void){
	currentms++;
}

void GPIO_INIT(){
	blinkActivated=0;
	blinkms=0;
	currentms=0;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	//pa5 for led
	GPIO_InitTypeDef gpio;
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOA, &gpio);

	//pc6 for onewire
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	gpio.GPIO_Mode = GPIO_Mode_Out_OD;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOC, &gpio);
}

void NVIC_INIT(void){
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void USART_INIT(){
	doConvert=0x00;
	pendingCommand.loadedChars=0;

	usartReadBuffer.lastReadPos=0;
	usartReadBuffer.lastWritePos=0;
	usartWriteBuffer.lastReadPos=0;
	usartWriteBuffer.lastWritePos=0;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);


	GPIO_InitTypeDef gpio;

	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = GPIO_Pin_2;
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &gpio);

	gpio.GPIO_Pin = GPIO_Pin_3;
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &gpio);

	USART_InitTypeDef uart;

	USART_StructInit(&uart);
	//uart.USART_BaudRate = 9600;
	//uart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &uart);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);

	USART_Cmd(USART2, ENABLE);
}

void blink(){
	if(blinkActivated){
		if(blinkms > 10){
			if(currentms > blinkms){
				currentms=0;
				if(blinkActivated==1){
					GPIO_SetBits(GPIOA, GPIO_Pin_5);
					blinkActivated=2;
				}else{
					GPIO_ResetBits(GPIOA, GPIO_Pin_5);
					blinkActivated=1;
				}
			}
		}else GPIO_SetBits(GPIOA, GPIO_Pin_5);
	}else GPIO_ResetBits(GPIOA, GPIO_Pin_5);
}
