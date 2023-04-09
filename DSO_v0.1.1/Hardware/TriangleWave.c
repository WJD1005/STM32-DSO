#include "stm32f10x.h"                  // Device header

void TriangleWave_Init(uint16_t ARR, uint16_t PSC)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC | RCC_APB1Periph_TIM6, ENABLE);		//使能DAC和TIM6时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;	//模拟输入
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	DAC_InitTypeDef DAC_InitStructure;
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;					//TIM6_TRGO触发
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_Triangle;		//三角波生成器
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_TriangleAmplitude_4095;	//屏蔽/幅值选择器
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;			//不开启输出缓存
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = ARR;
	TIM_TimeBaseInitStructure.TIM_Prescaler = PSC;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStructure);
	
	TIM_ClearFlag(TIM6, TIM_FLAG_Update);
	
	TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);	//TRGO为溢出更新触发
	
	DAC_Cmd(DAC_Channel_1, ENABLE);		//开启DAC
	DAC_SetChannel1Data(DAC_Align_12b_R, 0);	//12位右对齐，基准电压初始化为0
	TIM_Cmd(TIM6, ENABLE);				//开启定时器
}


