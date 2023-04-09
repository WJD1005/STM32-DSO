#include "stm32f10x.h"                  // Device header

/**
  * @brief  PWM输出初始化，输出端口为PA8（TIM1_CH1），控制PWM波频率与分辨率
  * @param  ARR: 自动重装寄存器值，PWM分辨率=1/(ARR+1)
  * @param	PSC: 预分频寄存器值，PWM频率=CK_PSC/(PSC+1)/(ARR+1)
  * @retval 无
  */
void PWM_Init(uint16_t ARR, uint16_t PSC)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);	//开启TIM1时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//开启GPIOA时钟
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;	//操作PA8
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	TIM_InternalClockConfig(TIM1);	//时基单元使用内部时钟
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;	//滤波器采样频率1分频
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;	//向上计数
	TIM_TimeBaseInitStructure.TIM_Period = ARR;	//自动重装值
	TIM_TimeBaseInitStructure.TIM_Prescaler = PSC;	//预分频值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;	//重复计数器（高级定时器）
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);
	
	TIM_ClearFlag(TIM1, TIM_FLAG_Update);	//清除TIM_TimeBaseInit()执行时给中断标志置的1
	
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);	//结构体成员比较多，先将结构体写入默认参数，再按需求改
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	//PWM模式1
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;	//输出低极性，也就是有效电平时输出低电平
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0x00;	//捕获/比较寄存器值
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);	//通道1初始化
	
	TIM_CtrlPWMOutputs(TIM1, ENABLE);	//MOE主输出使能
	
	TIM_Cmd(TIM1, ENABLE);	//开启定时器
}

/**
  * @brief  设置CCR捕获/比较寄存器值，控制PWM波占空比
  * @param  CCR1: TIM1_CCR1的值，在PWM模式1向上计数时PWM占空比=CCR/(ARR+1)
  * @retval 无
  */
void PWM_SetCompare(uint16_t CCR)
{
	TIM_SetCompare1(TIM1, CCR);
}
