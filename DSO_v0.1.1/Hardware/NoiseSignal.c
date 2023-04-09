#include "stm32f10x.h"                  // Device header

void NoiseSignal_Init(uint16_t ARR, uint16_t PSC)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC | RCC_APB1Periph_TIM7, ENABLE);		//使能DAC和TIM7时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;	//模拟输入
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	DAC_InitTypeDef DAC_InitStructure;
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T7_TRGO;					//TIM7_TRGO触发
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_Noise;		//噪声生成器
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bits11_0;	//屏蔽/幅值选择器
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;			//不开启输出缓存
	DAC_Init(DAC_Channel_2, &DAC_InitStructure);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = ARR;
	TIM_TimeBaseInitStructure.TIM_Prescaler = PSC;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseInitStructure);
	
	TIM_ClearFlag(TIM7, TIM_FLAG_Update);
	
	TIM_SelectOutputTrigger(TIM7, TIM_TRGOSource_Update);	//TRGO为溢出更新触发
	
	DAC_Cmd(DAC_Channel_2, ENABLE);		//开启DAC
	TIM_Cmd(TIM7, ENABLE);				//开启定时器
}
