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
  * @file    mos_sys.h
  * @version V1.0.0
  * @date    2021-08-20
  * @brief   ϵͳ֧��
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */

#ifndef _MOS_SYS_H
#define _MOS_SYS_H
#include "mos_typedef.h"

/* Public Fun-----------------------------------------------------------------*/
/* �жϷ���������ʱ����øú������ж�Ƕ�׼����� */
void mos_sys_interrupt_enter(void);
/* �жϷ������˳�ʱ����øú������ж�Ƕ�׼����� */
void mos_sys_interrupt_leave(void);
/* ��ȡ�ж�Ƕ�׼��� */
mos_uint8_t mos_sys_get_interrupt_nest_count(void);

#endif /* _MOS_SYS_H */
