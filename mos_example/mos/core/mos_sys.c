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
  * @file    mos_sys.c
  * @version V1.0.0
  * @date    2021-08-20
  * @brief   ϵͳ֧��
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */

#include "mos_sys.h"
#include "mos_port.h"


/* Public --------------------------------------------------------------------*/
volatile mos_uint8_t g_mos_interrupt_nest;


/* Public Fun-----------------------------------------------------------------*/
/**
 * @brief �жϷ���������ʱ����øú������ж�Ƕ�׼�����
 *
 * @note ��Ҫ��Ӧ�ó����е��øú���
 */
void mos_sys_interrupt_enter(void)
{
    mos_base_t temp;

    /* �����ٽ��� */
    temp = mos_port_entry_critical_irq();

    /* �ж�Ƕ�׼�����++ */
    g_mos_interrupt_nest++;

    /* �˳��ٽ��� */
    mos_port_exit_critical_irq(temp);
}

/**
 * @brief �жϷ������˳�ʱ����øú������ж�Ƕ�׼�����
 *
 * @note ��Ҫ��Ӧ�ó����е��øú���
 */
void mos_sys_interrupt_leave(void)
{
    mos_base_t temp;

    /* �����ٽ��� */
    temp = mos_port_entry_critical_irq();

    /* �ж�Ƕ�׼�����++ */
    g_mos_interrupt_nest--;

    /* �˳��ٽ��� */
    mos_port_exit_critical_irq(temp);
}

/**
 * @brief ��ȡ�ж�Ƕ�׼���
 *
 * @return �ж�Ƕ�׼���ֵ
 *
 * @note ��Ҫ��Ӧ�ó����е��øú���
 */
mos_uint8_t mos_sys_get_interrupt_nest_count(void)
{
    return g_mos_interrupt_nest;
}