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
  * @file    mos_hw.c
  * @version V1.0.0
  * @date    2021-08-20
  * @brief   用户系统硬件支持
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */

#include "mos_hw.h"	
#include "mos_misc.h"		


/* Private Fun----------------------------------------------------------------*/
static void mos_hw_task_exit_error( void )
{
    /* 函数停止在这里 */
    while(1);
}


/* Public Fun-----------------------------------------------------------------*/
void *mos_hw_stack_init(void *tentry, 
						mos_uint32_t parameter,
						mos_uint32_t stack_bottom)
{	
	task_hw_context_t 	*task_context;
	mos_uint8_t 		*sp;
	stack_bottom = MOS_ALIGN_DOWN(stack_bottom, 4);
	
	sp = (mos_uint8_t *)(stack_bottom - sizeof(task_hw_context_t) + 4);
	task_context = (task_hw_context_t *)(sp);

	mos_memset(task_context, 0xCC, sizeof(task_hw_context_t));
	
	task_context->r4  = 0x04040404L;
    task_context->r5  = 0x05050505L;
    task_context->r6  = 0x06060606L;
    task_context->r7  = 0x07070707L;
    task_context->r8  = 0x08080808L;
    task_context->r9  = 0x09090909L;
    task_context->r10 = 0x10101010L;
    task_context->r11 = 0x11111111L;
    
    task_context->r0  = parameter;
    task_context->r1  = 0x01010101L;
    task_context->r2  = 0x02020202L;
    task_context->r3  = 0x03030303L;
    task_context->r12 = 0x12121212L;
    task_context->lr  = (mos_uint32_t)mos_hw_task_exit_error;
    task_context->pc  = (mos_uint32_t)tentry & 0xfffffffeUL;
    task_context->psr = 0x01000000L;
	return (void *)sp;
}
