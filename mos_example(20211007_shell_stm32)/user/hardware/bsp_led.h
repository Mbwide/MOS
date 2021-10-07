/**
  ******************************************************************************
  * @file    bsp_led.h
  * @version V1.0.0
  * @date
  * @brief   led
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */

#ifndef _BSP_LED_H
#define _BSP_LED_H

/* Public Fun-----------------------------------------------------------------*/
void led_init(void);              
void led1_on(void);
void led1_off(void);

void led2_on(void);
void led2_off(void);

int  led1_state(void);

#endif /* _BSP_LED_H */
