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
;  * @brief   �û�ϵͳ֧�֣����ʵ��
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
MOS_NVIC_SYSPRI2             EQU     0xE000ED20;���ȼ��Ĵ�����ַ
MOS_NVIC_PENDSV_PRI          EQU     0xF0F00000;����PendSV��SysTick���ж����ȼ�Ϊ���
MOS_NVIC_PENDSVSET           EQU     0x10000000

	
;*************************************************************************
;                              �������ָ��
;*************************************************************************
    AREA    |.text|, CODE, READONLY, ALIGN=2 
    THUMB
    REQUIRE8
    PRESERVE8
	  
mos_port_start_first_task  PROC
	
    ;����PendSV�ж����ȼ�Ϊ���
    LDR     r0, =MOS_NVIC_SYSPRI2
    LDR     r1, =MOS_NVIC_PENDSV_PRI
    STR     R1, [R0]
	
    ;ʹ��ȫ���ж� 
    CPSIE I
    CPSIE F
    ;DSB
    ;ISB
	
    ;����SVCȥ������һ������ 
    SVC 0  
	
    ENDP

SVC_Handler    PROC
    LDR    r3, =g_cur_task_tcb
	
    LDR    r1, [r3]                                 
    LDR    r0, [r1]        ;��ʱr0��ֵΪ��ջָ��                       

    LDMFD  r0!, {r4 - r11} ;���߳�ջָ��r1(����֮ǰ�ȵݼ�)ָ������ݼ��ص�CPU�Ĵ���r4~r11
    MSR    psp, r0         ;��r0��ֵ���������ջָ����µ�psp
    ISB
    MOV    r0, #0
    ;MOV   lr, #0xFFFFFFF9
    ORR    lr, #0xd        ;����SVC�жϷ����˳�ǰ,ͨ����r14�Ĵ������4λ��λ����0x0D��ȷ���쳣����ʹ�õĶ�ջָ����PSP����LR�Ĵ�����λ2ҪΪ1
                           ;ʹ��Ӳ�����˳�ʱʹ�ý��̶�ջָ��PSP��ɳ�ջ���������غ�����߳�ģʽ������Thumb״̬ */
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
    LDR    r2, [r3]            ;����g_cur_task_tcb��r2,��ǰ����spָ��
	
    STMDB  r0!, {r4-r11}       ;��CPU�Ĵ���r4~r11��ֵ�洢��r0ָ��ĵ�ַ 
    STR    r0, [r2]            ;������ջ���µ�ջ��ָ��洢����ǰ����TCB�ĵ�һ����Ա����spָ�� 				
                        
    STMDB  sp!, {r1, r3, r14}  ;��r1, R3��R14��ʱѹ���ջ����Ϊ�������ú���mos_task_switch_context,
                               ;���ú���ʱ,���ص�ַ�Զ����浽R14��,����һ�����÷���,R14��ֵ�ᱻ����,�����Ҫ��ջ����;
                               ;R3����ĵ�ǰ���������TCBָ���ַ,�������ú���õ�,���ҲҪ��ջ����
    
	BL     mos_task_switch_context
	
    LDMIA  sp!, {r1, r3, r14}  ;�ָ�r1,r3��r14
    LDR    r2,  [r3]
    LDR    r0,  [r2]           ;��ǰ���������TCB��һ����������ջ��ջ��,����ջ��ֵ����R0
    LDMIA  r0!, {r4-r11}       ;��ջ 
    MSR    psp, r0
	
    ;�ָ��ж�
    MSR    PRIMASK, r1
    ;MOV   lr, #0xFFFFFFF9
    ORR    lr, lr, #0x04       ;ȷ���쳣����ʹ�õĶ�ջָ����PSP����LR�Ĵ�����λ2ҪΪ1
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

		
		
		
		
		
		
		
		
		