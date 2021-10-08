#ifndef __BSP_USART_H
#define __BSP_USART_H

#include "stdio.h"	
#include "bsp_sys.h" 



void shell_uart1_init(u32 bound);
void uart_send_byte(USART_TypeDef * uart_x, uint8_t c);
void uart_send_string(USART_TypeDef * uart_x, char *string);

#endif


