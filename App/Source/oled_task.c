#include "oled_task.h"
#include "OLED.h"
#include "systick.h"
#include "system_config.h"

void OLED_Task(void)
{
    uint8_t i;
    uint8_t line = 2; // 从第三行开始显示设备
    /*
        清屏
    */
    OLED_Clear();
    /*
        第一行固定显示公司名称
    */
    OLED_ShowString(30, 0, "沃米科技", OLED_8X16);
    /*
        显示在线设备
    */
    for (i = 0; i < MAX_SLAVE_ADDRESS; i++)
    {
        /*
            只显示在线设备
        */
        if (slave_list[i].online)
        {
            uint8_t y = line * 8;
            /*
                设备编号
                设备1
            */
            OLED_ShowString(0, y, "ID", OLED_6X8);

            OLED_ShowNum(24, y, i + 1, 1, OLED_6X8);

            /*
                在线状态               
            */
            OLED_ShowString(40, y, "Online", OLED_6X8);

            /*
                触发次数
            */
            OLED_ShowNum(80, y, slave_list[i].trigger_count, 4, OLED_6X8);
            /*
                下一行
            */
            line++;
            /*
                OLED高度限制
                64像素最多8行6X8
            */
            if (line >= 8)
            {
                break;
            }
        }
    }
    OLED_Update();
}