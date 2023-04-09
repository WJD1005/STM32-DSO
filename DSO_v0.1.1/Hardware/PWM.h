#ifndef __PWM_H
#define __PWM_H

void PWM_Init(uint16_t ARR, uint16_t PSC);
void PWM_SetCompare(uint16_t CCR);

#endif
