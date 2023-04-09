#include "stm32f10x.h"                  // Device header

//本项目定时器使用：
//TIM1：PWM波生成
//TIM2：采样中断
//TIM3：按键扫描中断
//TIM6：触发DAC产生三角波
//TIM7：触发DAC产生噪声



//触发中断开始采样标志位，不在中断函数中进行采样，不直接触发ADC是因为不方便管理
uint8_t SampleFlag = 0;
//按键按下标志位，0无按下，1短按，2长按
uint8_t KEY0_Flag = 0, KEY1_Flag = 0, WKUP_Flag = 0;

/**
  * @brief	TIM2初始化，使用内部时钟，定时中断
  * @param	ARR: 自动重装寄存器值，为实际计数次数减1，范围：0~65535
  * @param	PSC: 预分频器值，为分频数减1，范围：0~65535
  * @retval	无
  */
void TIM2_Init(uint16_t ARR, uint16_t PSC)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	//开启时钟
	
	TIM_InternalClockConfig(TIM2);	//时基单元使用内部时钟
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;	//滤波器采样频率1分频
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;	//向上计数
	TIM_TimeBaseInitStructure.TIM_Period = ARR;	//自动重装值
	TIM_TimeBaseInitStructure.TIM_Prescaler = PSC;	//预分频值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;	//重复计数器（高级定时器）
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
	
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);	//清除TIM_TimeBaseInit()执行时给中断标志置的1
	
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);	//开启更新中断接入
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//分组2
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM2, ENABLE); //开启定时器
}

/**
  * @brief	TIM3初始化，使用内部时钟，定时中断
  * @param	ARR: 自动重装寄存器值，为实际计数次数减1，范围：0~65535
  * @param	PSC: 预分频器值，为分频数减1，范围：0~65535
  * @retval	无
  */
void TIM3_Init(uint16_t ARR, uint16_t PSC)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//开启时钟
	
	TIM_InternalClockConfig(TIM3);	//时基单元使用内部时钟
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;	//滤波器采样频率1分频
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;	//向上计数
	TIM_TimeBaseInitStructure.TIM_Period = ARR;	//自动重装值
	TIM_TimeBaseInitStructure.TIM_Prescaler = PSC;	//预分频值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;	//重复计数器（高级定时器）
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
	
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);	//清除TIM_TimeBaseInit()执行时给中断标志置的1
	
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);	//开启更新中断接入
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//分组2
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM3, ENABLE); //开启定时器
}

/**
  * @brief	TIM2中断函数，用于定时将采样标志位置1
  * @param	无
  * @retval	无
  */
void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		SampleFlag = 1;
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	//清除中断标志位
	}
}

/**
  * @brief	TIM3中断函数，用于处理按键
  * @param	无
  * @retval	无
  */
void TIM3_IRQHandler(void)
{
	static uint8_t KEY0 = 0, KEY1 = 0, WKUP = 0;
	static uint8_t Time = 0;
	
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		//WKUP
		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == SET)	//按下
		{
			WKUP = 1;
			Time++;
		}
		else					//松开
		{
			if(WKUP == 1)		//之前是按下的
			{
				if(Time > 50)	//长按
				{
					WKUP_Flag = 2;
				}
				else			//短按
				{
					WKUP_Flag = 1;
				}
				Time = 0;
				WKUP = 0;
			}
		}
		//KEY1
		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15) == RESET)	//按下
		{
			KEY1 = 1;
			Time++;
		}
		else					//松开
		{
			if(KEY1 == 1)		//之前是按下的
			{
				if(Time > 50)	//长按
				{
					KEY1_Flag = 2;
				}
				else			//短按
				{
					KEY1_Flag = 1;
				}
				Time = 0;
				KEY1 = 0;
			}
		}
		//KEY0
		if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) == RESET)	//按下
		{
			KEY0 = 1;
			Time++;
		}
		else					//松开
		{
			if(KEY0 == 1)		//之前是按下的
			{
				if(Time > 50)	//长按
				{
					KEY0_Flag = 2;
				}
				else			//短按
				{
					KEY0_Flag = 1;
				}
				Time = 0;
				KEY0 = 0;
			}
		}
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);	//清除中断标志位
	}
}
