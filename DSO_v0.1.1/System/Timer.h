#ifndef __TIMER_H
#define __TIMER_H

extern uint8_t SampleFlag;
extern uint8_t KEY0_Flag;
extern uint8_t KEY1_Flag;
extern uint8_t WKUP_Flag;

void TIM2_Init(uint16_t ARR, uint16_t PSC);
void TIM3_Init(uint16_t ARR, uint16_t PSC);

#endif
