#include "seg.h"
#include "systick.h"
#include "delay.h"
#include "setting.h"

#define SEG_Delay_us() Delay_us(5);

static uint8_t brightness = 7;
/*
    flash control
*/
static uint8_t flash_flag = 0;

/*
    0~9 Digital tube segment code
*/
static const uint8_t SEG_TABLE[10] = {
    0x3f, // 0
    0x06, // 1
    0x5b, // 2
    0x4f, // 3
    0x66, // 4
    0x6d, // 5
    0x7d, // 6
    0x07, // 7
    0x7f, // 8
    0x6f  // 9
};

/* set CLK hight */
static void CLK_HIGH(void)
{
    gpio_bit_set(SEG_PORT, SEG_CLK_PIN);
}

/* set CLK low */
static void CLK_LOW(void)
{
    gpio_bit_reset(SEG_PORT, SEG_CLK_PIN);
}

/* set DIO hight */
static void DIO_HIGH(void)
{
    gpio_bit_set(SEG_PORT, SEG_DIO_PIN);
}

/* set DIO low */
static void DIO_LOW(void)
{
    gpio_bit_reset(SEG_PORT, SEG_DIO_PIN);
}

/* SEG start */
static void SEG_Start(void)
{
    DIO_HIGH();
    CLK_HIGH();
    SEG_Delay_us();
    DIO_LOW();
}

/* SEG Stop */
static void SEG_Stop(void)
{
    CLK_LOW();
    SEG_Delay_us();
    DIO_LOW();
    CLK_HIGH();
    SEG_Delay_us();
    DIO_HIGH();
}

/* Initialize the digital tube  */
void SEG_Init(void)
{
    // 1.configure the clock enable
    rcu_periph_clock_enable(RCU_GPIOB);
    // 2.configure CLK,DIO mode
    gpio_init(SEG_PORT, GPIO_MODE_OUT_OD, GPIO_OSPEED_50MHZ, SEG_CLK_PIN | SEG_DIO_PIN);
    CLK_HIGH();
    DIO_HIGH();
    SEG_Clear();
}

/* Write data to TIM1647 */
static void SEG_WriteByte(uint8_t data)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        CLK_LOW();
        if (data & 0x01)
            DIO_HIGH();
        else
            DIO_LOW();
        SEG_Delay_us();
        CLK_HIGH();
        SEG_Delay_us();
        data >>= 1;
    }
    CLK_LOW();
}

/* Send command to TIM1647 */
static void SEG_SendCommand(uint8_t cmd)
{
    SEG_Start();
    SEG_WriteByte(cmd);
    SEG_Stop();
}

/*
    display number
    0~9999
*/
void SEG_DisplayNumber(uint16_t num)
{
    uint8_t digit[4];
    digit[0] = SEG_TABLE[num / 1000 % 10];
    digit[1] = SEG_TABLE[num / 100 % 10];
    digit[2] = SEG_TABLE[num / 10 % 10];
    digit[3] = SEG_TABLE[num % 10];
    SEG_DisplayDigits(digit);
}

/*
    display a 4-digit array
    eg:1234
*/
void SEG_DisplayDigits(uint8_t *data)
{
    uint8_t i;
    // automatic address incrementation 0x40 -> 0x41 -> 0x42 -> 0x43
    SEG_SendCommand(0x40);
    SEG_Start();
    /*
        0XC0:Starting from the ARM0 segment address
        0xC0:RAM0 → first
        0xC1:RAM1 → second
        0xC2:RAM2 → third
        0xC3:RAM3 → fourth
    */
    SEG_WriteByte(0XC0);
    for (i = 0; i < 4; i++)
    {
        SEG_WriteByte(data[i]);
    }
    SEG_Stop();
    // display control
    SEG_SendCommand(0x88 | brightness);
}

/* clear the digital tube */
void SEG_Clear(void)
{
    uint8_t buff[4] = {0, 0, 0, 0};
    SEG_DisplayDigits(buff);
}

/*
    set the brightness of the digital tube
    0~7
*/
void SEG_SetBrightness(uint8_t b)
{
    if (b > 7)
    {
        b = 7;
    }
    brightness = b;
}

/*
    display with decimal point
    position:0~3
*/
void SEG_DisplayDecimal(uint16_t num, uint8_t position)
{
    uint8_t digit[4];
    digit[0] = SEG_TABLE[num / 1000 % 10];
    digit[1] = SEG_TABLE[num / 100 % 10];
    digit[2] = SEG_TABLE[num / 10 % 10];
    digit[3] = SEG_TABLE[num % 10];
    if (position < 4)
    {
        digit[position] |= 0x80;
    }
    SEG_DisplayDigits(digit);
}

// seg task
void SEG_Task(void)
{
    static uint32_t flash_tick = 0;
    static uint32_t seg_tick = 0;
    // flash 500ms

    if (GetTick() - flash_tick >= 500)
    {
        flash_tick = GetTick();
        flash_flag = !flash_flag;
    }
    // Refresh 10 ms
    if (GetTick() - seg_tick >= 10)
    {
        seg_tick = GetTick();
        uint16_t value;
        uint8_t pos;
        uint8_t edit;
        uint8_t digit[4];
        // Retrieve value via setting
        value = Setting_Get_Value();
        pos = Setting_Get_Pos();
        edit = Setting_Is_Edit();
        // decompose the value
        digit[0] = SEG_TABLE[value % 10];
        digit[1] = SEG_TABLE[value / 10 % 10];
        digit[2] = SEG_TABLE[value / 100 % 10];
        digit[3] = SEG_TABLE[value / 1000 % 10];
        if (edit)
        {
            /* code */
            if (flash_flag)
            {
                /* code */
                digit[pos] = 0;
            }
        }
        SEG_DisplayDigits(digit);
    }
}
