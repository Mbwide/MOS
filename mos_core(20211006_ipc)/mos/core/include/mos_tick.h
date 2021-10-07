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
  * @file    mos_tick.h
  * @version V1.0.1
  * @date    2021-10-04
  * @brief   ��ʱ��
  ******************************************************************************
  * @note
  *          2021-08-20 Mbwide:��ʼ�汾
  *          2021-10-04 Mbwide:�ṹ�Ż�
  ******************************************************************************
  */

#ifndef _MOS_TICK_H
#define _MOS_TICK_H
#include "mos_typedef.h"
#include "mos_task.h"

/* Public Fun-----------------------------------------------------------------*/
/* ϵͳʱ�ӳ�ʼ�� */
void mos_tick_systick_init(void);
/* ϵͳʱ���жϷ����� */
void mos_tick_systick_handler(void);
/* ��ȡ��ǰʱ�Ӽ���ֵ */
mos_tick_t mos_tick_get_cur_tick_count(void);
/* ������ʱ���� */
void mos_tick_delay_process(mos_tcb_t *mos_tcb, const mos_uint32_t tick);

#endif /* _MOS_TICK_H */

