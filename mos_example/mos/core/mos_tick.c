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
  * @file    mos_tick.c
  * @version V1.0.0
  * @date    2021-08-20
  * @brief   定时器
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */
  
#include "mos_tick.h"
#include "mos_sys.h"
#include "mos_task.h"

/* Public Fun-----------------------------------------------------------------*/
void mos_tick_systick_handler(void)
{
	/* 进入中断 */ 
	mos_sys_interrupt_enter();
	
	/* 时基更新 */ 
	mos_task_tickcount_increase();
	
	/* 离开中断 */
	mos_sys_interrupt_leave();
}


