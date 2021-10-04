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
  * @file    mos_port.h
  * @version V1.0.0
  * @date    2021-08-20
  * @brief   �û�ϵͳ���֧��
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */
#ifndef _MOS_PORT_H
#define _MOS_PORT_H
#include "mos_typedef.h"


/* define --------------------------------------------------------------------*/
/* �����ٽ���,irq */
#define mos_port_entry_critical_irq() \
    mos_port_interrupt_disable_temp()
/* �˳��ٽ���,irq */
#define mos_port_exit_critical_irq(x) \
    mos_port_interrupt_enable_temp(x)

/* �����ٽ��� */
#define mos_port_entry_critical() \
    mos_port_interrupt_disable()
/* �˳��ٽ��� */
#define mos_port_exit_critical() \
    mos_port_interrupt_enable()

/* Public Fun-----------------------------------------------------------------*/
/* ��ʼ��һ������ */
void mos_port_start_first_task(void);
/* ������� */
void mos_port_task_scheduler(void);

/* ϵͳʱ�ӳ�ʼ�� */
void mos_port_systick_init(void);

/* SVC �жϷ����� */
extern void SVC_Handler(void);
/* SysTick �жϷ����� */
extern void SysTick_Handler(void);
/* PendSV �жϷ����� */
extern void PendSV_Handler(void);

/* �ر��ж�,����־λ */
mos_base_t mos_port_interrupt_disable_temp(void);
/* �����ж�,����־λ */
void mos_port_interrupt_enable_temp(mos_base_t primask_value);

/* �ر��ж� */
void mos_port_interrupt_disable(void);
/* �����ж� */
void mos_port_interrupt_enable(void);


#endif /* _MOS_PORT_H */

