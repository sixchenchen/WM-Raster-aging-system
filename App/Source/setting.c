#include "setting.h"
#include "key.h"
#include "alarm.h"
#include "sensor.h"

// The current value displayed on the digital tube
static uint16_t setting_value = 0;

// current editor location
static uint8_t edit_pos = 0;

/*
    current editor mode
    1：modify parameter
    0：normal display
*/
static uint8_t edit_mode = 0;

// position
static uint16_t digit_weight[4] = {
    1000,
    100,
    10,
    1};

// rest seg value
static void Rest_Set_Value(void)
{
    setting_value = 0;
    edit_pos = 0;
    edit_mode = 0;
}

/*
    save value
*/
static void Setting_Save(void)
{
    // TODO
    /*
        EEPROM_Write(setting_value);
    */
}

/*
    Add the current position
*/
static void Setting_Add(void)
{
    setting_value += digit_weight[edit_pos];
    if (setting_value > 9999)
    {
        setting_value = 0;
    }
}

/*
    Reduce the current position
*/
static void Setting_Sub(void)
{
    if (setting_value >= digit_weight[edit_pos])
    {
        setting_value -= digit_weight[edit_pos];
    }
    else
    {
        setting_value = 9999;
    }
}

// initialize value
void Setting_Init(void)
{
    Rest_Set_Value();
}

/*
    Raster trigger counting
    +1 for each trigger
    Maximum 9999
*/
void Setting_AddCount(void)
{
    setting_value++;
    if (setting_value > 9999)
    {
        setting_value = 0;
    }
}

// get seg value
uint16_t Setting_Get_Value(void)
{
    return setting_value;
}
// get seg pos
uint8_t Setting_Get_Pos(void)
{
    return edit_pos;
}
// get seg edit
uint8_t Setting_Is_Edit(void)
{
    return edit_mode;
}

/*
    SET key
        First press:
        Enter Settings
    Again:
        Bit switch
    Fourth time:
        Exit Settings
*/

void Setting_Set(void)
{

    if (edit_mode == 0)
    {
        edit_mode = 1;
        edit_pos = 0;
    }
    else
    {
        edit_pos++;
        if (edit_pos >= 4)
        {
            edit_pos = 0;
            edit_mode = 1;
        }
    }
}

/*
    UP
    Current bit increment
*/

void Setting_Up(void)
{
    if (edit_mode == 0)
        return;
    setting_value += digit_weight[edit_pos];
    if (setting_value > 9999)
    {
        setting_value = 0;
    }
}

/*
    DOWN
    Current bit reduction
*/

void Setting_Down(void)
{
    if (edit_mode == 0)
        return;
    if (setting_value >= digit_weight[edit_pos])
    {
        setting_value -= digit_weight[edit_pos];
    }
    else
    {
        setting_value = 9999;
    }
}

/*
    RESET short
    exit
*/

void Setting_Reset_Short(void)
{
    edit_mode = 0;
    edit_pos = 0;
}

/*
    RESET long
    clear seg value
*/
void Setting_Reset_Long(void)
{
    setting_value = 0;
}
