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
  * @brief   定时器
  ******************************************************************************
  * @note
  *          2021-08-20 Mbwide:初始版本
  *          2021-10-04 Mbwide:结构优化
  ******************************************************************************
  */

#ifndef _MOS_TICK_H
#define _MOS_TICK_H
#include "mos_typedef.h"
#include "mos_task.h"

/* Public Fun-----------------------------------------------------------------*/
/* 系统时钟初始化 */
void mos_tick_systick_init(void);
/* 系统时钟中断服务函数 */
void mos_tick_systick_handler(void);
/* 获取当前时钟计数值 */
mos_tick_t mos_tick_get_cur_tick_count(void);
/* 任务延时处理 */
void mos_tick_delay_process(mos_tcb_t *mos_tcb, const mos_uint32_t tick);

#endif /* _MOS_TICK_H */

