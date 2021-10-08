;/**
;  ******************************************************************************
;  * Micro OS V1.0 - Copyright (C) 2021 Real Time Engineers Ltd.
;  * All rights reserved.
;  * This file is part of Micro OS.
;  *
;  * This program is free software; you can redistribute it and/or modify
;  * it under the terms of the GNU General Public License as published by
;  * the Free Software Foundation; either version 2 of the License, or
;  * (at your option) any later version.
;  *
;  * This program is distributed in the hope that it will be useful,
;  * but WITHOUT ANY WARRANTY; without even the implied warranty of
;  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  * GNU General Public License for more details.
;  ******************************************************************************
;  */
;/**
;  ******************************************************************************
;  * @file    mos_port_asm.s
;  * @version V1.0.0
;  * @date    2021-08-20
;  * @brief   用户系统支持，汇编实现
;  ******************************************************************************
;  * @note
;  *
;  ******************************************************************************
;  */


        EXPORT SVC_Handler
        EXPORT PendSV_Handler
			
        EXPORT mos_port_start_first_task
        EXPORT mos_port_task_scheduler
        EXPORT mos_port_interrupt_disable
        EXPORT mos_port_interrupt_enable
        EXPORT mos_port_interrupt_disable_temp
        EXPORT mos_port_interrupt_enable_temp
		
        IMPORT g_cur_task_tcb
        IMPORT mos_task_switch_context
MOS_NVIC_INT_CTRL            EQU     0xE000ED04
MOS_NVIC_SYSPRI2             EQU     0xE000ED20;优先级寄存器地址
MOS_NVIC_PENDSV_PRI          EQU     0xF0F00000;配置PendSV和SysTick的中断优先级为最低
MOS_NVIC_PENDSVSET           EQU     0x10000000

	
;*************************************************************************
;                              代码产生指令
;*************************************************************************
    AREA    |.text|, CODE, READONLY, ALIGN=2 
    THUMB
    REQUIRE8
    PRESERVE8
	  
mos_port_start_first_task  PROC
	
    ;配置PendSV中断优先级为最低
    LDR     r0, =MOS_NVIC_SYSPRI2
    LDR     r1, =MOS_NVIC_PENDSV_PRI
    STR     R1, [R0]
	
    ;使能全局中断 
    CPSIE I
    CPSIE F
    ;DSB
    ;ISB
	
    ;调用SVC去启动第一个任务 
    SVC 0  
	
    ENDP

SVC_Handler    PROC
    LDR    r3, =g_cur_task_tcb
	
    LDR    r1, [r3]                                 
    LDR    r0, [r1]        ;此时r0的值为堆栈指针                       

    LDMFD  r0!, {r4 - r11} ;将线程栈指针r1(操作之前先递减)指向的内容加载到CPU寄存器r4~r11
    MSR    psp, r0         ;将r0的值，即任务的栈指针更新到psp
    ISB
    MOV    r0, #0
    ;MOV   lr, #0xFFFFFFF9
    ORR    lr, #0xd        ;当从SVC中断服务退出前,通过向r14寄存器最后4位按位或上0x0D，确保异常返回使用的堆栈指针是PSP，即LR寄存器的位2要为1
                           ;使得硬件在退出时使用进程堆栈指针PSP完成出栈操作并返回后进入线程模式、返回Thumb状态 */
    BX     lr 
	
    ENDP
		
mos_port_task_scheduler    PROC
    LDR    R0, =MOS_NVIC_INT_CTRL
    LDR    R1, =MOS_NVIC_PENDSVSET
    STR    R1, [R0]
    BX     LR
	
    NOP
    ENDP

PendSV_Handler    PROC
    MRS    r1, PRIMASK
    CPSID  I
	
    MRS    r0, psp
    ISB
	
    LDR    r3, =g_cur_task_tcb
    LDR    r2, [r3]            ;加载g_cur_task_tcb到r2,当前任务sp指针
	
    STMDB  r0!, {r4-r11}       ;将CPU寄存器r4~r11的值存储到r0指向的地址 
    STR    r0, [r2]            ;将任务栈的新的栈顶指针存储到当前任务TCB的第一个成员，即sp指针 				
                        
    STMDB  sp!, {r1, r3, r14}  ;将r1, R3和R14临时压入堆栈，因为即将调用函数mos_task_switch_context,
                               ;调用函数时,返回地址自动保存到R14中,所以一旦调用发生,R14的值会被覆盖,因此需要入栈保护;
                               ;R3保存的当前激活的任务TCB指针地址,函数调用后会用到,因此也要入栈保护
    
	BL     mos_task_switch_context
	
    LDMIA  sp!, {r1, r3, r14}  ;恢复r1,r3和r14
    LDR    r2,  [r3]
    LDR    r0,  [r2]           ;当前激活的任务TCB第一项保存了任务堆栈的栈顶,现在栈顶值存入R0
    LDMIA  r0!, {r4-r11}       ;出栈 
    MSR    psp, r0
	
    ;恢复中断
    MSR    PRIMASK, r1
    ;MOV   lr, #0xFFFFFFF9
    ORR    lr, lr, #0x04       ;确保异常返回使用的堆栈指针是PSP，即LR寄存器的位2要为1
    BX     lr         
	
    ENDP
	

mos_port_interrupt_disable_temp  PROC
    MRS    R0, PRIMASK
    CPSID  I
    BX     LR
    ENDP

mos_port_interrupt_enable_temp	 PROC
    MSR    PRIMASK, r0
    BX     LR
	
    NOP
    ENDP

mos_port_interrupt_disable	 PROC
    CPSID  I
    BX     LR
    ENDP

mos_port_interrupt_enable	 PROC
    CPSIE  I
    BX     LR
    ENDP
	
    END

		
		
		
		
		
		
		
		
		