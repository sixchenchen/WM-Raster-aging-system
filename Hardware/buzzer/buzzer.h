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

/* buzzer togger */
void Buzzer_Toggle(void);

#endif
