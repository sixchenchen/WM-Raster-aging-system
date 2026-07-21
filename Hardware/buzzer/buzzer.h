#ifndef __BUZZER_H
#define __BUZZER_H

#include "gd32f10x.h"

/* buzzer PA15 define */
#define BUZZER_PORT GPIOA
#define BUZZER_PIN GPIO_PIN_15

/* initilize buzzer */
void Buzzer_Init(void);

/* buzzer on */
void Buzzer_On(void);

/* buzzer off */
void Buzzer_Off(void);

/* buzzer start */
void Buzzer_Start(uint32_t time_ms);

/* buzzer togger */
void Buzzer_Toggle(void);

/* buzzer beep blocking type */
void Buzzer_Beep(uint32_t time_ms);

/* buzzer alarm */
void Buzzer_Alarm(uint8_t times, uint32_t on_ms, uint32_t off_ms);

/*
    buzzer task
*/
void Buzzer_Task(void);

#endif
