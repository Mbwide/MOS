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
  * @brief   内核初始化
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */

#include "mos_init.h"
#include "mos_task.h"
#include "mos_port.h"
#include "mos_heap.h"
#include "mos_tick.h"
#include "mos_shell.h"


/* Public Fun-----------------------------------------------------------------*/
/**
 * @brief 内核初始化函数
 */
void mos_init_core(void)
{
    /* 硬件层初始化 */
    mos_port_bsp_init();

    /* SysTick定时器初始化 */
    mos_tick_systick_init();

    /* 内存池初始化 */
    mos_heap_init();

    /* 任务调度器初始化 */
    mos_task_scheduler_init();

#if (MOS_CONFIG_USE_SHELL == YES)
    /* shell初始化 */
    mos_shell_init();
#endif

#if (MOS_CONFIG_USE_DEBUG_PRINTF == YES)
    /* 内核打印版本 */
    mos_show_version();
    mos_printf("Core Init Success!\n");
#endif
}

