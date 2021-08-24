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
  * @file    mos_hw.h
  * @version V1.0.0
  * @date    2021-08-20
  * @brief   用户系统硬件支持
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */
  
#ifndef _MOS_HW_H
#define _MOS_HW_H
#include "mos_typedef.h"


typedef struct task_hw_context
{
    /* 手动出入栈寄存器 */
    mos_uint32_t r4;
    mos_uint32_t r5;
    mos_uint32_t r6;
    mos_uint32_t r7;
    mos_uint32_t r8;
    mos_uint32_t r9;
    mos_uint32_t r10;
    mos_uint32_t r11;
	
	/* 自动出入栈寄存器 */
	mos_uint32_t r0;
    mos_uint32_t r1;
    mos_uint32_t r2;
    mos_uint32_t r3;
    mos_uint32_t r12;
    mos_uint32_t lr;
    mos_uint32_t pc;
    mos_uint32_t psr;
    
} task_hw_context_t;


/* Public Fun-----------------------------------------------------------------*/
void *mos_hw_stack_init(void *tentry, 
						mos_uint32_t parameter,
						mos_uint32_t stack_top);



#endif /* _MOS_HW_H */

