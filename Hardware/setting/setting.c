#include "key.h"
#include "seg.h"

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

// initialize value
void Setting_Init(void)
{
    Rest_Set_Value();
}

// setting task

void Setting_Task(void)
{
    Key_Event key;
    key = Key_Get_Event();
    switch (key)
    {
    case KEY_SET:
        /* code */
        edit_mode = 1;
        edit_pos++;
        if (edit_pos >= 4)
        {
            edit_pos = 0;
        }
        break;
    case KEY_UP:
        /* code */
        if (edit_mode)
        {
            /* code */
            setting_value += digit_weight[edit_pos];
            if (setting_value > 9999)
            {
                setting_value = 0;
            }
        }

        break;
    case KEY_DOWN:
        /* code */
        if (edit_mode)
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
        break;
    case KEY_RESET:
        /* code */
        Rest_Set_Value();
        break;

    default:
        break;
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
