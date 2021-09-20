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
  * @file    mos_init.c
  * @version V1.0.0
  * @date    2021-08-20
  * @brief   �ں˳�ʼ��
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */

#include "mos_init.h"
#include "mos_task.h"
#include "mos_port.h"
#include "mos_heap.h"

/* Public Fun-----------------------------------------------------------------*/
/**
 * @brief �ں˳�ʼ������
 */
void mos_init_core(void)
{
	/* �ڴ�س�ʼ�� */
    mos_heap_init();
	
    /* �����������ʼ�� */
    mos_task_scheduler_init();

    /* SysTick��ʱ����ʼ�� */
    mos_port_systick_init();
}

