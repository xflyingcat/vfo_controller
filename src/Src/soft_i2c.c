#include "includes.h"
#include "stm32f1xx_hal.h"

#define SCL_LOW()     HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET)
#define SCL_HIGH()    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET)

#define SDA_LOW()     HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET)
#define SDA_HIGH()    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET)

#define IN_SDA_PIN()   HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10)

static void soft_i2c_send_bit(char bit);
static int soft_i2c_read_bit(void);
static void soft_i2c_delay ( void );


void soft_i2c_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull =GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    //GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);    
    
    SCL_HIGH();
    SDA_HIGH();


}

void soft_i2c_deinit(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}


void soft_i2c_start(void)
{
    SCL_HIGH();
    SDA_HIGH();
    soft_i2c_delay();
    SDA_LOW();
    soft_i2c_delay();
    SCL_LOW();
    soft_i2c_delay();
}


int soft_i2c_write(unsigned char bt)
{
    char i;
    unsigned char b;
    for(i=0; i<8; i++)
    {
        soft_i2c_send_bit(bt & (0x80));
        bt = bt << 1;
    }
    b = soft_i2c_read_bit();
    soft_i2c_delay();
    soft_i2c_delay();
    return b;
}

unsigned char soft_i2c_read(unsigned char ack)
{
    char i;
    unsigned char bt = 0;
    SDA_HIGH();
    for(i=0; i<8; i++)
    {
        bt = bt << 1;
        bt |= soft_i2c_read_bit();
    }
    soft_i2c_delay();
    if(ack)
    {
        SDA_LOW();
    }
    else
    {
        SDA_HIGH();
    }
    soft_i2c_delay();
    SCL_HIGH();
    soft_i2c_delay();
    SCL_LOW();
    soft_i2c_delay();
    return bt;
}


void soft_i2c_stop(void)
{
    SCL_LOW();
    SDA_LOW();
    soft_i2c_delay();
    SDA_LOW();
    soft_i2c_delay();
    SCL_HIGH();
    soft_i2c_delay();
    SDA_HIGH();
    soft_i2c_delay();
    soft_i2c_delay();
}


static void soft_i2c_send_bit(char bit)
{
    if(bit)
    {
        SDA_HIGH();
    }
    else
    {
        SDA_LOW();
    }
    soft_i2c_delay();
    SCL_HIGH();
    soft_i2c_delay();
    SCL_LOW();
    soft_i2c_delay();
}


static int soft_i2c_read_bit(void)
{
    unsigned long in;
    SDA_HIGH();
    soft_i2c_delay();
    SCL_HIGH();
    soft_i2c_delay();
    in = IN_SDA_PIN();
    soft_i2c_delay();
    SCL_LOW();
    soft_i2c_delay();
    if(in)
        return 1;
    else
        return 0;
}

static void soft_i2c_delay (void)
{
    volatile unsigned int    i ;
    for ( i = 0 ; (i < 50 ); i++ ) ;
}





