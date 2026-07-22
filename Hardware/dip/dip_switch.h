#ifndef __DIP_SWITCH_H
#define __DIP_SWITCH_H

#include "gd32f10x.h"

/*
    initialize dip switch
*/
void Dip_Switch_Init(void);

/*
    read 5 bit address

    0~31
*/
uint8_t DIP_Read_Address(void);

/*
    read mode switch

    PA8 PA9 PA10

    0~7

*/
uint8_t DIP_Read_Mode(void);

#endif
