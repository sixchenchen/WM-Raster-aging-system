#ifndef __SETTING_H
#define __SETTING_H

#include "gd32f10x.h"

/*
    setting initialize
*/
void Setting_Init(void);

/*
    Grating trigger increased
*/
void Setting_AddCount(void);

/*
    obtain current value
*/
uint16_t Setting_Get_Value(void);

/*
    obtain current setting bit
    0:k_bit
    1:h_bit
    2:t_bit
    3:u_bit
*/
uint8_t Setting_Get_Pos(void);

/*
    currently editing
*/
uint8_t Setting_Is_Edit(void);

/*
    Set button processing
*/
void Setting_Set(void);

void Setting_Up(void);

void Setting_Down(void);

void Setting_Reset_Short(void);

void Setting_Reset_Long(void);

#endif
