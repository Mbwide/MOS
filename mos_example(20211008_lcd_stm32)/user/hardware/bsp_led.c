/**
  ******************************************************************************
  * @file    led.c
  * @version V1.0.0
  * @date
  * @brief   led
  ******************************************************************************
  * @note
  *     硬件连接：
  *     PD2  LED1
  *     PA8  LED2
  *
  ******************************************************************************
  */
  
#include "stm32f10x.h"
#include "bsp_led.h"


/* Public Fun-----------------------------------------------------------------*/
/**
 * @brief 初始化LED1和LED2
 */
void led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_SetBits(GPIOD, GPIO_Pin_2);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);
}

/**
 * @brief LED1开启
 */
void led1_on(void)
{
    /* PD2 输出低 */
    GPIO_ResetBits(GPIOD, GPIO_Pin_2);
}

/**
 * @brief LED1关闭
 */
void led1_off(void)
{
    /* PD2 输出高 */
    GPIO_SetBits(GPIOD, GPIO_Pin_2);
}

/**
 * @brief LED2开启
 */
void led2_on(void)
{
    /* PA8 输出低 */
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
}

/**
 * @brief LED2关闭
 */
void led2_off(void)
{
    /* PA8 输出高 */
    GPIO_SetBits(GPIOA, GPIO_Pin_8);
}

