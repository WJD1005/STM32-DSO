#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Timer.h"

/**
  * @brief	开启按键对应的GOIO时钟与配置GPIO模式，开启TIM3每20ms扫描一次按键状态
  * @param	无
  * @retval	无
  */
void Key_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	//创建GPIO参数结构体
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);	//开启GPIOA和GPIOC的时钟
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);	//关闭JTAG（避免PA15冲突），使能SWD，可以用SWD模式调试

	//KEY0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;	//操作PC5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//上拉输入模式
	GPIO_Init(GPIOC, &GPIO_InitStructure);	//按结构体参数配置PC5为上拉输入
	
	//KEY1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;	//操作PA15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//上拉输入模式
	GPIO_Init(GPIOA, &GPIO_InitStructure);	//按结构体参数配置PA15为上拉输入
	
	//WK_UP
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	//操作PA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;	//下拉输入模式
	GPIO_Init(GPIOA, &GPIO_InitStructure);	//按结构体参数配置PA0为下拉输入
	
	TIM3_Init(19999, 71);					//20ms定时中断扫描
}
