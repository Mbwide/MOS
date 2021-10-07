/**
  ******************************************************************************
  * Micro OS V1.0 - Copyright (C) 2021 Real Time Engineers Ltd.
  * All rights reserved.
  * This file is part of Micro OS.
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 2 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  ******************************************************************************
  */

/**
  ******************************************************************************
  * @file    mos_port.c
  * @version 
  * @date    
  * @brief   �û�ϵͳ���֧��
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */

#include "mos_port.h"
#include "mos_tick.h"
#include "mos_sys.h"
#include "mos_shell.h"
#include "mos_user_config.h"	

#include "bsp_usart.h"

/* Public Fun-----------------------------------------------------------------*/
#if (MOS_CONFIG_USE_SHELL == YES || MOS_CONFIG_USE_DEBUG_PRINTF == YES)  
/**
 * @brief  shell/debug����ӿ�
 * @param  Ҫ���������
 */
void mos_port_output(const char *string)
{	
	/* ֱ���ַ������� */
    while (*string!='\0')
	{
		/* ���� */
        if (*string=='\n')
		{
			USART_SendData(MOS_SHELL_DEBUG_PORT, '\r'); 
			while (USART_GetFlagStatus(MOS_SHELL_DEBUG_PORT, USART_FLAG_TXE) == RESET);
		}

		USART_SendData(MOS_SHELL_DEBUG_PORT, *string++); 				
		while (USART_GetFlagStatus(MOS_SHELL_DEBUG_PORT, USART_FLAG_TXE) == RESET);	
	}	
}
#endif

#if (MOS_CONFIG_USE_SHELL == YES)
/**
 * @brief ����1�ж�(shell����ӿ�)
 */
void USART1_IRQHandler(void)  
{
	/* �����ٽ�� */
    mos_base_t temp =  mos_port_entry_critical_temp();

    if (USART_GetITStatus(MOS_SHELL_DEBUG_PORT, USART_IT_RXNE) != RESET)
    {
        USART_ClearITPendingBit(MOS_SHELL_DEBUG_PORT, USART_IT_RXNE);
		mos_shell_process(USART_ReceiveData(MOS_SHELL_DEBUG_PORT));
    }
	
	/* �˳��ٽ�� */
    mos_port_exit_critical_temp(temp);
}

#endif
/**
 * @brief Ӳ�����ʼ��
 */
void mos_port_bsp_init(void)
{
	
#if ((MOS_CONFIG_USE_SHELL == YES) || (MOS_CONFIG_USE_DEBUG_PRINTF == YES))
	/* Ӳ�����ʼ�� */
    shell_uart1_init(115200);
#endif
	
}

/**
 * @brief ϵͳʱ�ӳ�ʼ��
 */
void mos_port_systick_init(void)
{
    /* SysTick�ж�Ƶ������ */
    SysTick_Config(MOS_CONFIG_CPU_FREQUENCY / MOS_CONFIG_TICK_PER_SECOND);
}

/**
 * @brief SysTick �жϷ�����
 */
void SysTick_Handler(void)
{
    mos_tick_systick_handler();
}



//void SVC_Handler(void)
//{
//
//}

//void PendSV_Handler(void)
//{
//	s_PendSV_Handler();
//}
