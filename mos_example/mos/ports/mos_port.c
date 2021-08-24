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
  * @version V1.0.0
  * @date    2021-08-20
  * @brief   用户系统软件支持
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */

#include "mos_port.h"
#include "mos_tick.h"
#include "mos_user_config.h"
#include "ARMCM4.h"

void mos_port_systick_init(void)
{
    /* SysTick中断频率设置 */
    SysTick_Config( SystemCoreClock / MOS_CONFIG_TICK_PER_SECOND );
}

//void SVC_Handler(void)
//{
//
//}


//void PendSV_Handler(void)
//{
//	mos_port_pendsv_handler();
//}


void SysTick_Handler(void)
{
    mos_tick_systick_handler();
}

