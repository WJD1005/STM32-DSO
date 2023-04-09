#include "stm32f10x.h"                  // Device header
#include "TFTLCD.h"
#include "DSO.h"
#include "ADC.h"
#include "Key.h"
#include "PWM.h"
#include "TriangleWave.h"
#include "NoiseSignal.h"

int main(void)
{
	uint8_t VerticalLevel = 7;
	uint8_t HorizontalTime = 4;
	
	
	LCD_Init(1);		//横屏
	Key_Init();
	DSO_Init();
	
	//测试用PWM（PA8）
	PWM_Init(99, 287);		//1KHz，分辨率1%
	PWM_SetCompare(50);
	//测试用三角波（PA4）
	TriangleWave_Init(1, 8);	//9分频，计数1次8000KHz触发频率，产生近似1KHz的0~3.3V的三角波
	//测试用噪声信号（PA5）
	NoiseSignal_Init(999, 71);	//1KHz变化的噪声信号
	
	while(1)
	{
		//按键部分
		if(WKUP_Flag == 1)		//WKUP短按暂停
		{
			WKUP_Flag = 0;
			DSO_SetStatus(0);
			//暂停单独由循环卡住，免得运行时要判断运行状态
			while(1)
			{
				if(WKUP_Flag == 1)		//WKUP再次短按开始
				{
					WKUP_Flag = 0;
					KEY1_Flag = 0;		//清除暂停期间产生的无用标志位
					KEY0_Flag = 0;
					DSO_SetStatus(1);
					break;
				}
			}
		}
		if(KEY1_Flag == 1)		//KEY1短按切换垂直灵敏度
		{
			KEY1_Flag = 0;
			if(VerticalLevel > 1)
			{
				VerticalLevel--;
			}
			else
			{
				VerticalLevel = 7;
			}
			DSO_SetVertical(VerticalLevel);
		}
		if(KEY0_Flag == 1)		//KEY0短按切换水平时基
		{
			KEY0_Flag = 0;
			if(HorizontalTime > 1)
			{
				HorizontalTime--;
			}
			else
			{
				HorizontalTime = 13;
			}
			DSO_SetHorizontal(HorizontalTime);
		}
		
		DSO_Run();
	}
}
