#include "stm32f10x.h"                  // Device header
#include "math.h"
#include "TFTLCD.h"
#include "Timer.h"
#include "ADC.h"

char Version[6] = "V0.1.1";
/*
----------更新日志----------

V0.1.0	无触发模式除频率计算外基本完成
V0.1.1	增加了100us档采样数据组数，降低波形失真
*/

uint16_t Wave[299] = {0};	//100us/Div只采样150个，其他均采样299个
uint16_t pWave = 0;
uint16_t Wavey[299] = {0};
uint8_t Status= 1;		//1:Running 0:Holding
uint8_t TrigMode = 0;	//0:None 1:Rising 2:Falling
float TrigVal = 0;
uint8_t Vertical = 7;	//1:10mV 2: 20mV 3:50mV 4:0.1V 5:0.2V 6:0.5V 7:1V
uint8_t Horizontal = 4;	//1:100us 2:200us 3:500us 4:1ms 5:2ms 6:5ms 7:10ms 8:20ms 9:50ms 10:0.1s 11:0.2s 12:0.5s 13:1s
uint16_t Freq = 0;
float Max = 0;
float Min = 0;
float Avg = 0;
float Rms = 0;

/**
  * @brief	示波器初始化
  * @param	无
  * @retval	无
  */
void DSO_Init(void)
{
	uint16_t i;
	
	ADC1_Init();
	
	//采样定时器设置
	if(Horizontal == 1)
	{
		TIM2_Init(9, 47);
	}
	else if(Horizontal > 1 && Horizontal < 13)
	{
		if(Horizontal % 3 == 1)
		{
			TIM2_Init(pow(10, (Horizontal - 1) / 3 + 1) * 1 - 1, 23);
		}
		else if(Horizontal % 3 == 2)
		{
			TIM2_Init(pow(10, (Horizontal - 1) / 3 + 1) * 2 - 1, 23);
		}
		else if(Horizontal % 3 == 0)
		{
			TIM2_Init(pow(10, (Horizontal - 1) / 3 + 1) * 5 - 1, 23);
		}
	}
	else if(Horizontal == 13)
	{
		TIM2_Init(9999, 239);
	}
	//UI
	//背景色
	BACK_COLOR = BLACK;
	LCD_Clear(BACK_COLOR);
	//说明
	POINT_COLOR = WHITE;
	LCD_ShowString(2, 5, 24, 12, "DSO_", 12, 0);
	//版本号
	LCD_ShowString(26, 5, 36, 12, Version, 12, 0);
	//触发模式
	POINT_COLOR = WHITE;
	LCD_ShowString(132, 5, 54, 12, "TrigMode:", 12, 0);
	switch(TrigMode)
	{
		case 0:
			LCD_ShowString(186, 5, 42, 12, "None   ", 12, 0);
			break;
		case 1:
			LCD_ShowString(186, 5, 42, 12, "Rising ", 12, 0);
			break;
		case 2:
			LCD_ShowString(186, 5, 42, 12, "Falling", 12, 0);
			break;
	}
	//触发值
	LCD_ShowString(240, 5, 48, 12, "TrigVal:", 12, 0);
	LCD_ShowString(288, 5, 36, 12, " .  V", 12, 0);
	LCD_ShowNum(288, 5, (int)TrigVal, 1, 12, 0x00);			//个位
	LCD_ShowNum(300, 5, (int)(TrigVal * 100) % 100, 2, 12, 0x10);	//2位小数
	//波形显示区域（算上外框301*181）
	POINT_COLOR = WHITE;
	LCD_DrawRectangle(10, 22, 310, 202);
	//网格线
	for(i=1;i<=9;i++)
	{
		if(i == 5)
		{
			POINT_COLOR = WHITE;
		}
		else
		{
			POINT_COLOR = GRAY;
		}
		LCD_DrawLine(10 + i * 30, 23, 10 + i * 30, 201);
	}
	for(i=1;i<=5;i++)
	{
		if(i == 3)
		{
			POINT_COLOR = WHITE;
		}
		else
		{
			POINT_COLOR = GRAY;
		}
		LCD_DrawLine(11, 22 + i * 30, 309, 22 + i* 30);
	}
	//垂直灵敏度
	POINT_COLOR = WHITE;
	LCD_ShowString(2, 210, 54, 12, "Vertical:", 12, 0);
	switch(Vertical)
	{
		case 1:
			LCD_ShowString(56, 210, 48, 12, "10mV/Div", 12, 0);
			break;
		case 2:
			LCD_ShowString(56, 210, 48, 12, "20mV/Div", 12, 0);
			break;
		case 3:
			LCD_ShowString(56, 210, 48, 12, "50mV/Div", 12, 0);
			break;
		case 4:
			LCD_ShowString(56, 210, 48, 12, "0.1V/Div", 12, 0);
			break;
		case 5:
			LCD_ShowString(56, 210, 48, 12, "0.2V/Div", 12, 0);
			break;
		case 6:
			LCD_ShowString(56, 210, 48, 12, "0.5V/Div", 12, 0);
			break;
		case 7:
			LCD_ShowString(56, 210, 48, 12, "1V/Div  ", 12, 0);
			break;
	}
	//水平时基
	LCD_ShowString(116, 210, 66, 12, "Horizontal:", 12, 0);
	switch(Horizontal)
	{
		case 1:
			LCD_ShowString(182, 210, 54, 12, "100us/Div", 12, 0);
			break;
		case 2:
			LCD_ShowString(182, 210, 54, 12, "200us/Div", 12, 0);
			break;
		case 3:
			LCD_ShowString(182, 210, 54, 12, "500us/Div", 12, 0);
			break;
		case 4:
			LCD_ShowString(182, 210, 54, 12, "1ms/Div  ", 12, 0);
			break;
		case 5:
			LCD_ShowString(182, 210, 54, 12, "2ms/Div  ", 12, 0);
			break;
		case 6:
			LCD_ShowString(182, 210, 54, 12, "5ms/Div  ", 12, 0);
			break;
		case 7:
			LCD_ShowString(182, 210, 54, 12, "10ms/Div ", 12, 0);
			break;
		case 8:
			LCD_ShowString(182, 210, 54, 12, "20ms/Div ", 12, 0);
			break;
		case 9:
			LCD_ShowString(182, 210, 54, 12, "50ms/Div ", 12, 0);
			break;
		case 10:
			LCD_ShowString(182, 210, 54, 12, "0.1s/Div ", 12, 0);
			break;
		case 11:
			LCD_ShowString(182, 210, 54, 12, "0.2s/Div ", 12, 0);
			break;
		case 12:
			LCD_ShowString(182, 210, 54, 12, "0.5s/Div ", 12, 0);
			break;
		case 13:
			LCD_ShowString(182, 210, 54, 12, "1s/Div   ", 12, 0);
			break;
	}
	//频率
	POINT_COLOR = WHITE;
	LCD_ShowString(248, 210, 72, 12, "Freq:     Hz", 12, 0);
	LCD_ShowNum(278, 210, Freq, 5, 12, 0);
	//最值
	LCD_ShowString(2, 222, 54, 12, "Max: .  V", 12, 0);
	LCD_ShowNum(26, 222, (int)Max, 1, 12, 0x00);
	LCD_ShowNum(38, 222, (int)(Max * 100) % 100, 2, 12, 0x10);
	LCD_ShowString(68, 222, 54, 12, "Min: .  V", 12, 0);
	LCD_ShowNum(92, 222, (int)Min, 1, 12, 0x00);
	LCD_ShowNum(104, 222, (int)(Min * 100) % 100, 2, 12, 0x10);
	//均值
	LCD_ShowString(134, 222, 54, 12, "Avg: .  V", 12, 0);
	LCD_ShowNum(158, 222, (int)Avg, 1, 12, 0x00);
	LCD_ShowNum(170, 222, (int)(Avg * 100) % 100, 2, 12, 0x10);
	//均方根
	LCD_ShowString(200, 222, 54, 12, "Rms: .  V", 12, 0);
	LCD_ShowNum(224, 222, (int)Rms, 1, 12, 0x00);
	LCD_ShowNum(236, 222, (int)(Rms * 100) % 100, 2, 12, 0x10);
	//运行标志
	switch(Status)
	{
		case 0:
			POINT_COLOR = RED;
			LCD_ShowString(272, 222, 42, 12, "Holding", 12, 0);
			break;
		case 1:
			POINT_COLOR = GREEN;
			LCD_ShowString(272, 222, 42, 12, "Running", 12, 0);
			break;
	}
}

/**
  * @brief	设置示波器触发模式（未实现）
  * @param	Mode: 触发模式：0，Node；1，Rising；2，Falling
  * @retval	无
  */
void DSO_SetTrigMode(uint8_t Mode)
{
	
}

/**
  * @brief	设置示波器在Rising或Falling触发模式时的触发值（未实现）
  * @param	Val: 触发值，0~3.3
  * @retval	无
  */
void DSO_SetTrigVal(float Val)
{
	
}

/**
  * @brief	清除已经画出的波形
  * @param	无
  * @retval	无
  */
void DSO_ClearWave(void)
{
	uint16_t i, y1, y2;
	
	POINT_COLOR = BACK_COLOR;
	if(Horizontal == 1)
	{
		for(i=0;i<149;i++)
		{
			y1 = Wavey[i];
			y2 = Wavey[i + 1];
			if(y1 > 22 && y1 < 202 && y2 > 22 && y2 < 202)	//在范围内则连线（不太好）
			{
				LCD_DrawLine(11 + i * 2, y1, 13 + i * 2, y2);
			}
		}
	}
	else
	{
		for(i=0;i<298;i++)
		{
			y1 = Wavey[i];
			y2 = Wavey[i + 1];
			if(y1 > 22 && y1 < 202 && y2 > 22 && y2 < 202)	//在范围内则连线（不太好）
			{
				LCD_DrawLine(11 + i, y1, 12 + i, y2);
			}
		}
	}
	//补画网格线
	for(i=1;i<=9;i++)
	{
		if(i == 5)
		{
			POINT_COLOR = WHITE;
		}
		else
		{
			POINT_COLOR = GRAY;
		}
		LCD_DrawLine(10 + i * 30, 23, 10 + i * 30, 201);
	}
	for(i=1;i<=5;i++)
	{
		if(i == 3)
		{
			POINT_COLOR = WHITE;
		}
		else
		{
			POINT_COLOR = GRAY;
		}
		LCD_DrawLine(11, 22 + i * 30, 309, 22 + i* 30);
	}
}

/**
  * @brief	由波形数据计算画波形的纵坐标
  * @param	i: Wave[]数组索引
  * @retval	无
  */
void DSO_Calculatey(uint16_t i)
{
	switch(Vertical)
	{
		case 1:
			//10mV垂直灵敏度下1像素为(0.01/30)V
			if(i == 0)
			{
				Wavey[i] = 112 - ((Wave[i] - 2048) / 4096.0 * 3.3 / (0.01 / 30.0));
			}
			Wavey[i + 1] = 112 - ((Wave[i + 1] - 2048) / 4096.0 * 3.3 / (0.01 / 30.0));
			break;
		case 2:
			//20mV垂直灵敏度下1像素为(0.02/30)V
			if(i == 0)
			{
				Wavey[i] = 112 - ((Wave[i] - 2048) / 4096.0 * 3.3 / (0.02 / 30.0));
			}
			Wavey[i + 1] = 112 - ((Wave[i + 1] - 2048) / 4096.0 * 3.3 / (0.02 / 30.0));
			break;
		case 3:
			//50mV垂直灵敏度下1像素为(0.05/30)V
			if(i == 0)
			{
				Wavey[i] = 112 - ((Wave[i] - 2048) / 4096.0 * 3.3 / (0.05 / 30.0));
			}
			Wavey[i + 1] = 112 - ((Wave[i + 1] - 2048) / 4096.0 * 3.3 / (0.05 / 30.0));
			break;
		case 4:
			//0.1V垂直灵敏度下1像素为(0.1/30)V
			if(i == 0)
			{
				Wavey[i] = 112 - ((Wave[i] - 2048) / 4096.0 * 3.3 / (0.1 / 30.0));
			}
			Wavey[i + 1] = 112 - ((Wave[i + 1] - 2048) / 4096.0 * 3.3 / (0.1 / 30.0));
			break;
		case 5:
			//0.2V垂直灵敏度下1像素为(1/30)V
			if(i == 0)
			{
				Wavey[i] = 112 - ((Wave[i] - 2048) / 4096.0 * 3.3 / (0.2 / 30.0));
			}
			Wavey[i + 1] = 112 - ((Wave[i + 1] - 2048) / 4096.0 * 3.3 / (0.2 / 30.0));
			break;
		case 6:
			//0.5V垂直灵敏度下1像素为(1/30)V
			if(i == 0)
			{
				Wavey[i] = 112 - ((Wave[i] - 2048) / 4096.0 * 3.3 / (0.5 / 30.0));
			}
			Wavey[i + 1] = 112 - ((Wave[i + 1] - 2048) / 4096.0 * 3.3 / (0.5 / 30.0));
			break;
		case 7:
			//1V垂直灵敏度下1像素为(1/30)V
			if(i == 0)
			{
				Wavey[i] = 112 - ((Wave[i] - 2048) / 4096.0 * 3.3 / (1 / 30.0));
			}
			Wavey[i + 1] = 112 - ((Wave[i + 1] - 2048) / 4096.0 * 3.3 / (1 / 30.0));
			break;
	}
}

/**
  * @brief	画波形
  * @param	无
  * @retval	无
  */
void DSO_ShowWave(void)
{
	uint16_t i;
	int16_t y1, y2;
	
	POINT_COLOR = YELLOW;
	if(Horizontal == 1)
	{
		for(i=0;i<149;i++)
		{
			//坐标换算
			DSO_Calculatey(i);
			//读出坐标
			y1 = Wavey[i];
			y2 = Wavey[i + 1];
			if(y1 > 22 && y1 < 202 && y2 > 22 && y2 < 202)	//在范围内则连线（不太好）
			{
				LCD_DrawLine(11 + i * 2, y1, 13 + i * 2, y2);
			}
		}

	}
	else
	{
		for(i=0;i<298;i++)
		{
			//坐标换算
			DSO_Calculatey(i);
			//读出坐标
			y1 = Wavey[i];
			y2 = Wavey[i + 1];
			if(y1 > 22 && y1 < 202 && y2 > 22 && y2 < 202)	//在范围内则连线（不太好）
			{
				LCD_DrawLine(11 + i, y1, 12 + i, y2);
			}
		}
	}
}

/**
  * @brief	设置垂直灵敏度
  * @param	VerticalLevel: 垂直灵敏度：1:10mV 2: 20mV 3:50mV 4:0.1V 5:0.2V 6:0.5V 7:1V
  * @retval	无
  */
void DSO_SetVertical(uint8_t VerticalLevel)
{
	//擦除波形
	DSO_ClearWave();
	Vertical = VerticalLevel;
	switch(Vertical)
	{
		case 1:
			LCD_ShowString(56, 210, 48, 12, "10mV/Div", 12, 0);
			break;
		case 2:
			LCD_ShowString(56, 210, 48, 12, "20mV/Div", 12, 0);
			break;
		case 3:
			LCD_ShowString(56, 210, 48, 12, "50mV/Div", 12, 0);
			break;
		case 4:
			LCD_ShowString(56, 210, 48, 12, "0.1V/Div", 12, 0);
			break;
		case 5:
			LCD_ShowString(56, 210, 48, 12, "0.2V/Div", 12, 0);
			break;
		case 6:
			LCD_ShowString(56, 210, 48, 12, "0.5V/Div", 12, 0);
			break;
		case 7:
			LCD_ShowString(56, 210, 48, 12, "1V/Div  ", 12, 0);
			break;
	}
}

/**
  * @brief	设置水平灵敏度
  * @param	HorizontalLevel: 水平灵敏度：1:100us 2:200us 3:500us 4:1ms 5:2ms 6:5ms 7:10ms 8:20ms 9:50ms 10:0.1s 11:0.2s 12:0.5s 13:1s
  * @retval	无
  */
void DSO_SetHorizontal(uint8_t HorizontalTime)
{
	//擦除波形
	DSO_ClearWave();
	Horizontal = HorizontalTime;
	switch(Horizontal)
	{
		case 1:
			LCD_ShowString(182, 210, 54, 12, "100us/Div", 12, 0);
			break;
		case 2:
			LCD_ShowString(182, 210, 54, 12, "200us/Div", 12, 0);
			break;
		case 3:
			LCD_ShowString(182, 210, 54, 12, "500us/Div", 12, 0);
			break;
		case 4:
			LCD_ShowString(182, 210, 54, 12, "1ms/Div  ", 12, 0);
			break;
		case 5:
			LCD_ShowString(182, 210, 54, 12, "2ms/Div  ", 12, 0);
			break;
		case 6:
			LCD_ShowString(182, 210, 54, 12, "5ms/Div  ", 12, 0);
			break;
		case 7:
			LCD_ShowString(182, 210, 54, 12, "10ms/Div ", 12, 0);
			break;
		case 8:
			LCD_ShowString(182, 210, 54, 12, "20ms/Div ", 12, 0);
			break;
		case 9:
			LCD_ShowString(182, 210, 54, 12, "50ms/Div ", 12, 0);
			break;
		case 10:
			LCD_ShowString(182, 210, 54, 12, "0.1s/Div ", 12, 0);
			break;
		case 11:
			LCD_ShowString(182, 210, 54, 12, "0.2s/Div ", 12, 0);
			break;
		case 12:
			LCD_ShowString(182, 210, 54, 12, "0.5s/Div ", 12, 0);
			break;
		case 13:
			LCD_ShowString(182, 210, 54, 12, "1s/Div   ", 12, 0);
			break;
	}
	//更改采样定时器
	if(Horizontal == 1)
	{
		TIM2_Init(9, 47);
	}
	else if(Horizontal > 1 && Horizontal < 13)
	{
		if(Horizontal % 3 == 1)
		{
			TIM2_Init(pow(10, (Horizontal - 1) / 3 + 1) * 1 - 1, 23);
		}
		else if(Horizontal % 3 == 2)
		{
			TIM2_Init(pow(10, (Horizontal - 1) / 3 + 1) * 2 - 1, 23);
		}
		else if(Horizontal % 3 == 0)
		{
			TIM2_Init(pow(10, (Horizontal - 1) / 3 + 1) * 5 - 1, 23);
		}
	}
	else if(Horizontal == 13)
	{
		TIM2_Init(9999, 239);
	}
	//重新开始采样一页波形
	pWave = 0;
}

/**
  * @brief	设置运行状态
  * @param	Sta: 运行状态：0，Holding；1，Running
  * @retval	无
  */
void DSO_SetStatus(uint8_t Sta)
{
	if(Sta)
	{
		Status = 1;
		POINT_COLOR = GREEN;
		LCD_ShowString(272, 222, 42, 12, "Running", 12, 0);
	}
	else
	{
		Status = 0;
		POINT_COLOR = RED;
		LCD_ShowString(272, 222, 42, 12, "Holding", 12, 0);
	}
}

/**
  * @brief	波形数据采样一次
  * @param	无
  * @retval	0，采样失败；1，采样成功
  */
uint8_t DSO_Sample(void)
{
	if(SampleFlag)
	{
		Wave[pWave] = ADC1_GetValue();
		pWave++;
		SampleFlag = 0;
		return 1;
	}
	else
	{
		return 0;
	}
}

/**
  * @brief	计算波形频率（未实现）
  * @param	无
  * @retval	无
  */
void DSO_GetFrequency(void)
{
	
}

//int16_t QuickSort_GetMid(uint16_t Arr[], int16_t Left, int16_t Right)
//{
//	uint16_t Pivot = Arr[Left];	//Choose First
//	
//	while(Left < Right)
//	{
//		while(Arr[Right] >= Pivot && Left < Right)
//		{
//			Right--;
//		}
//		Arr[Left] = Arr[Right];
//		while(Arr[Left] <= Pivot && Left < Right)
//		{
//			Left++;
//		}
//		Arr[Right] = Arr[Left];
//	}
//	Arr[Left] = Pivot;
//	return Left;
//}

//void QuickSort(uint16_t Arr[], int16_t Left, int16_t Right)
//{
//	if(Left < Right)
//	{
//		int16_t Mid = QuickSort_GetMid(Arr, Left, Right);
//		QuickSort(Arr, Left, Mid - 1);
//		QuickSort(Arr, Mid + 1, Right);
//	}
//}

/**
  * @brief	数组排序，采用双向冒泡排序
  * @param	Arr[]: 待排序数组
  * @param	n: 数组数据个数
  * @retval	无
  */
void Sort(uint16_t Arr[], uint16_t n)
{
	uint16_t Left = 0, Right = n - 1, Temp, i;

	while(Left < Right)
	{
		for(i=Left;i<Right;i++)		//正向冒泡，找到最大者
		{
			if(Arr[i] > Arr[i+1])
			{
				Temp = Arr[i];
				Arr[i]=Arr[i+1];
				Arr[i+1]=Temp;
			}
		}
		Right--;
		for(i=Right;i>Left;i--)	//反向冒泡，找到最小者
		{
			if(Arr[i] < Arr[i-1])
			{
				Temp = Arr[i];
				Arr[i]=Arr[i-1];
				Arr[i-1]=Temp;
			}
		}
		Left++;
	}
}

/**
  * @brief	显示波形计算后的信息
			内含排序，一定要在画图之后使用
  * @param	无
  * @retval	无
  */
void DSO_ShowInfo(void)
{
	uint16_t i;
	uint32_t Temp = 0;
	
	POINT_COLOR = WHITE;
	
	//目前均值和均方值均是由本页数据得出的，所以若页周期不是波形周期的整数倍将会有波动，所以要么调到适合的水平时基，如果没有适合的话尽可能让时基大一点减弱不完整波形周期的影响，但不可太大，应以能看清波形细节的最大时基为准
	//以后可以考虑加入周期抽离
	if(Horizontal == 1)
	{
		//最值
		Sort(Wave, 76);
		Min = Wave[0] / 4096.0 * 3.3;
		Max = Wave[75] / 4096.0 * 3.3;
		//均值（取75组）
		for(i=0;i<150;i+=2)
		{
			Temp += Wave[i];
			Avg = (Temp / 4096.0 * 3.3) / 75.0;
		}
		//均方根（取75组）
		for(i=0;i<150;i+=2)
		{
			Temp += Wave[i] * Wave[i];
			Rms = sqrt(Temp / 75.0) / 4096.0 * 3.3;
		}
	}
	else
	{
		//最值
		Sort(Wave, 299);
		Min = Wave[0] / 4096.0 * 3.3;
		Max = Wave[298] / 4096.0 * 3.3;
		//均值（取60组）
		for(i=0;i<299;i+=5)
		{
			Temp += Wave[i];
			Avg = (Temp / 4096.0 * 3.3) / 60.0;
		}
		//均方根（取60组）
		for(i=0;i<299;i+=5)
		{
			Temp += Wave[i] * Wave[i];
			Rms = sqrt(Temp / 60.0) / 4096.0 * 3.3;
		}
	}
	//最值
	LCD_ShowNum(26, 222, (int)Max, 1, 12, 0x00);
	LCD_ShowNum(38, 222, (int)(Max * 100) % 100, 2, 12, 0x10);
	LCD_ShowNum(92, 222, (int)Min, 1, 12, 0x00);
	LCD_ShowNum(104, 222, (int)(Min * 100) % 100, 2, 12, 0x10);
	//均值
	LCD_ShowNum(158, 222, (int)Avg, 1, 12, 0x00);
	LCD_ShowNum(170, 222, (int)(Avg * 100) % 100, 2, 12, 0x10);
	//均方根
	LCD_ShowNum(224, 222, (int)Rms, 1, 12, 0x00);
	LCD_ShowNum(236, 222, (int)(Rms * 100) % 100, 2, 12, 0x10);
}

/**
  * @brief	触发模式为None时的总体功能实现
  * @param	无
  * @retval	无
  */
void DSO_Trigger_None(void)
{
	if(DSO_Sample())
	{
		if(Horizontal == 1)	//100us档只采样150组数据，也是极限速度
		{
			if(pWave == 150)
			{
				DSO_ClearWave();
				DSO_ShowWave();
				DSO_ShowInfo();
				pWave = 0;
			}
		}
		else				//其余挡位正常采样量，其中200us挡刚好是极限速度
		{
			if(pWave == 299)	//已完成一页采样
			{
				DSO_ClearWave();
				DSO_ShowWave();
				DSO_ShowInfo();
				pWave = 0;
			}
		}
	}
}

/**
  * @brief	示波器整体运行函数
  * @param	无
  * @retval	无
  */
void DSO_Run(void)
{
	//触发模式为None
	if(TrigMode == 0)
	{
		DSO_Trigger_None();
	}
}


