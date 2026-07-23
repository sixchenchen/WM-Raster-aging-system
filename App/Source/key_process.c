#include "key_process.h"

#include "key.h"
#include "setting.h"
#include "alarm.h"
#include "sensor.h"

/*
    key process task
    handle user operation
*/
void Key_Process_Task(void)
{
    Key_Event key;
    /*
        get key event
        if no key:
        return KEY_NONE
    */
    key = Key_Get_Event();
    switch (key)
    {
    /*
        SET button
        Function:
        1.enter setting mode
        2.switch editing position
    */
    case KEY_SET_EVENT:
        Setting_Set();
        break;
    /*
        UP button
        Increase current digit
    */
    case KEY_UP_EVENT:
        Setting_Up();
        break;
    /*
        DOWN button
        Decrease current digit
    */
    case KEY_DOWN_EVENT:
        Setting_Down();
        break;
    /*
        RESET short button
    */
    case KEY_RESET_SHORT_EVENT:

        Setting_Reset_Short();

        break;
    /*
        RESET long button
    */
    case KEY_RESET_LONG_EVENT:

        Setting_Reset_Long();

        break;
    /*
        no key
    */
    case KEY_NONE:
        break;
    default:
        break;
    }
}