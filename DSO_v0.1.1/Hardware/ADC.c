#include "stm32f10x.h"                  // Device header

/**
  * @brief	ADC1初始化，PA1引脚，规则组，单ADC非扫描非连续模式，无外部触发
  * @param	无
  * @retval	无
  */
void ADC1_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);		//开启GPIOA和ADC1的时钟
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);	//ADC时钟预分频器6分频
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;	//模拟输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;		//PA1
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_1Cycles5);		//通道1接入ADC1，规则组序列一，采样时间为1.5个ADC周期
	
	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;		//单ADC独立模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;		//无外部触发（软件触发）
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//数据右对齐
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;			//非扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;		//非连续模式
	ADC_InitStructure.ADC_NbrOfChannel = 1;					//扫描模式通道数目
	ADC_Init(ADC1, &ADC_InitStructure);
	
	ADC_Cmd(ADC1, ENABLE);		//使能ADC1
	
	ADC_ResetCalibration(ADC1);								//复位校准
	while(ADC_GetResetCalibrationStatus(ADC1) == SET);		//检查复位校准是否完成
	ADC_StartCalibration(ADC1);								//开始校准
	while(ADC_GetCalibrationStatus(ADC1) == SET);			//检查校准是否完成
}

/**
  * @brief	进行单次AD转换并返回AD值
  * @param	无
  * @retval	AD值
  */
uint16_t ADC1_GetValue(void)
{
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//软件触发
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);		//等待EOC标志位置1
	return ADC_GetConversionValue(ADC1);		//读取AD值，同时会把EOC清零
}
