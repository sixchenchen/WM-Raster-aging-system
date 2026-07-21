#include "icc.h"
#include "delay.h"

#define IIC_Delay() Delay_us(5)

/*
    SCL = 1
*/
static void SCL_High(void)
{
    gpio_bit_set(IIC_PORT, IIC_SCL_PIN);
}

/*
    SCL = 0
*/
static void SCL_Low(void)
{
    gpio_bit_reset(IIC_PORT, IIC_SCL_PIN);
}

/*
    SDA = 1
*/
static void SDA_High(void)
{
    gpio_bit_set(IIC_PORT, IIC_SDA_PIN);
}

/*
    SDA = 0
*/
static void SDA_Low(void)
{
    gpio_bit_reset(IIC_PORT, IIC_SDA_PIN);
}

/*
    SDA read
*/
static uint8_t SDA_Read(void)
{
    return gpio_input_bit_get(IIC_PORT, IIC_SDA_PIN);
}

/*
    icc initial
*/

void IIC_Init(void)
{

    rcu_periph_clock_enable(RCU_GPIOB);

    /*
        open-drain output
        The IIC must have open-drain output.
    */

    gpio_init(IIC_PORT,
              GPIO_MODE_OUT_OD,
              GPIO_OSPEED_50MHZ,
              IIC_SCL_PIN |
                  IIC_SDA_PIN);

    SCL_High();

    SDA_High();
}

/*
    Start

    SDA:
    1 -> 0

    SCL keep hight

*/

void IIC_Start(void)
{

    SDA_High();

    SCL_High();

    IIC_Delay();

    SDA_Low();

    IIC_Delay();

    SCL_Low();
}

/*
    Stop

    SDA:
    0 -> 1

*/

void IIC_Stop(void)
{

    SDA_Low();

    SCL_High();

    IIC_Delay();

    SDA_High();

    IIC_Delay();
}

/*
    waiting ACK

    return:
    0 ACK
    1 NACK

*/

uint8_t IIC_WaitAck(void)
{

    uint8_t timeout = 0;

    SDA_High();

    SCL_High();

    IIC_Delay();

    while (SDA_Read())
    {

        timeout++;

        if (timeout > 250)
        {

            IIC_Stop();

            return 1;
        }
    }

    SCL_Low();

    return 0;
}

/*
    send ACK
*/

void IIC_SendAck(void)
{

    SDA_Low();

    IIC_Delay();

    SCL_High();

    IIC_Delay();

    SCL_Low();
}

/*
    send NACK
*/

void IIC_SendNack(void)
{

    SDA_High();

    IIC_Delay();

    SCL_High();

    IIC_Delay();

    SCL_Low();
}

/*
    IIC Send byte

*/

void IIC_SendByte(uint8_t data)
{

    uint8_t i;

    for (i = 0; i < 8; i++)
    {

        SCL_Low();

        if (data & 0x80)
        {
            SDA_High();
        }
        else
        {
            SDA_Low();
        }

        data <<= 1;

        IIC_Delay();

        SCL_High();

        IIC_Delay();
    }

    SCL_Low();
}

/*
    read a byte
*/

uint8_t IIC_ReadByte(uint8_t ack)
{

    uint8_t i;

    uint8_t data = 0;

    SDA_High();

    for (i = 0; i < 8; i++)
    {

        SCL_Low();

        IIC_Delay();

        SCL_High();

        data <<= 1;

        if (SDA_Read())
        {
            data++;
        }

        IIC_Delay();
    }

    SCL_Low();

    if (ack)
    {
        IIC_SendAck();
    }
    else
    {
        IIC_SendNack();
    }

    return data;
}
