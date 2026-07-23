#include "seg.h"
#include "systick.h"
#include "delay.h"
#include "setting.h"

#define SEG_DELAY(value)              \
    do                                \
    {                                 \
        uint32_t delay_count = value; \
        while (delay_count--)         \
        {                             \
            __NOP();                  \
        }                             \
    } while (0)

static uint8_t brightness = 7;
/*
    flash control
*/
static uint8_t flash_flag = 0;

/*
    0~9 Digital tube segment code
*/
static const uint8_t SEG_TABLE[10] =
    {
        0x3F,
        0x06,
        0x5B,
        0x4F,
        0x66,
        0x6D,
        0x7D,
        0x07,
        0x7F,
        0x6F};

/* Set CLK high. */
static void CLK_H(void)
{
    gpio_bit_set(SEG_PORT, SEG_CLK_PIN);
}

/* Set CLK low. */
static void CLK_L(void)
{
    gpio_bit_reset(SEG_PORT, SEG_CLK_PIN);
}

/* Set DIO high. */
static void DIO_H(void)
{
    gpio_bit_set(SEG_PORT, SEG_DIO_PIN);
}

/* Set DIO low. */
static void DIO_L(void)
{
    gpio_bit_reset(SEG_PORT, SEG_DIO_PIN);
}

/* Send the TM1637 start condition. */
static void SEG_Start(void)
{
    SEG_DIO_GPIO_OUTPUT();
    CLK_H();
    DIO_H();
    SEG_DELAY(6);
    DIO_L();
}

/* Send the TM1637 stop condition. */
static void SEG_Stop(void)
{
    CLK_L();
    SEG_DELAY(5);
    DIO_L();
    SEG_DELAY(5);
    CLK_H();
    SEG_DELAY(5);
    DIO_H();
}

/* Write one byte and leave the bus ready for the ACK phase. */
static void SEG_WriteByte(uint8_t data)
{
    uint8_t i;

    SEG_DIO_GPIO_OUTPUT();

    for (i = 0; i < 8; i++)
    {
        CLK_L();
        if (data & 0x01)
            DIO_H();
        else
            DIO_L();
        SEG_DELAY(3);
        CLK_H();
        SEG_DELAY(3);
        data >>= 1;
    }
}

/* Read the TM1637 ACK bit and restore DIO to output mode. */
static uint8_t SEG_IsAck(void)
{
    uint8_t time = 60;
    uint8_t ack = 0;

    CLK_L();
    SEG_DIO_GPIO_INPUT();

    while (time--)
    {
        if (RESET == gpio_input_bit_get(SEG_DIO_GPIO_PORT, SEG_DIO_GPIO_PIN))
        {
            ack = 1;
            break;
        }
    }

    CLK_H();
    SEG_DELAY(6);
    CLK_L();
    SEG_DIO_GPIO_OUTPUT();

    return ack;
}

/* Send a command byte, including its ACK phase. */
static uint8_t SEG_SendCommand(uint8_t cmd)
{
    SEG_Start();
    SEG_WriteByte(cmd);
    if (!SEG_IsAck())
    {
        SEG_Stop();
        return 0;
    }
    SEG_Stop();
    return 1;
}

/* Initialize the digital tube  */
void SEG_Init(void)
{
    // 1.configure the clock enable
    SEG_CLK_GPIO_CLK_ENABLE();
    // 2.configure CLK,DIO mode
    SEG_CLK_GPIO_OUTPUT();
    SEG_DIO_GPIO_OUTPUT();
    CLK_H();
    DIO_H();
    Delay_ms(10);
    // open display
    SEG_Clear();
    SEG_SendCommand(0x88 | brightness);
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
    // Automatic address incrementation: 0x40 -> 0x41 -> 0x42 -> 0x43.
    if (!SEG_SendCommand(0x40))
    {
        return;
    }

    SEG_Start();
    SEG_WriteByte(0XC0);
    if (!SEG_IsAck())
    {
        SEG_Stop();
        return;
    }

    for (i = 0; i < 4; i++)
    {
        SEG_WriteByte(data[i]);
        if (!SEG_IsAck())
        {
            SEG_Stop();
            return;
        }
    }
    SEG_Stop();

    // Display control and brightness.
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

void SEG_Task(void)
{
    static uint32_t flash_tick = 0;
    static uint32_t seg_tick = 0;

    static uint16_t last_value = 0xffff;
    static uint8_t last_pos = 0xff;
    static uint8_t last_edit = 0xff;

    uint16_t value;
    uint8_t pos;
    uint8_t edit;

    if (GetTimeElapsed(flash_tick) >= 500)
    {
        flash_tick = GetTick();
        flash_flag = !flash_flag;
    }

    if (GetTimeElapsed(seg_tick) >= 10)
    {
        seg_tick = GetTick();

        value = Setting_Get_Value();
        pos = Setting_Get_Pos();
        edit = Setting_Is_Edit();

        if (value != last_value ||
            pos != last_pos ||
            edit != last_edit ||
            edit)
        {

            last_value = value;
            last_pos = pos;
            last_edit = edit;

            uint8_t digit[4];

            digit[0] = SEG_TABLE[value / 1000 % 10];
            digit[1] = SEG_TABLE[value / 100 % 10];
            digit[2] = SEG_TABLE[value / 10 % 10];
            digit[3] = SEG_TABLE[value % 10];

            if (edit && flash_flag)
            {
                digit[pos] = SEG_OFF;
            }

            SEG_DisplayDigits(digit);
        }
    }
}
