#ifndef __DSO_H
#define __DSO_H

void DSO_Init(void);

void DSO_SetStatus(uint8_t Sta);

void DSO_SetVertical(uint8_t VerticalLevel);
void DSO_SetHorizontal(uint8_t HorizontalTime);

void DSO_Trigger_None(void);

void DSO_Run(void);


#endif
