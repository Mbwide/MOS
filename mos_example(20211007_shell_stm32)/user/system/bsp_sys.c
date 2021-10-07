#include "bsp_sys.h"
/**
  ******************************************************************************
  * @file    bsp_sys.c
  * @version 
  * @date    
  * @brief   bsp_sys
  ******************************************************************************
  * @note
  ******************************************************************************
  */

/* ִ�л��ָ��WFI */
void WFI_SET(void)
{
	__ASM volatile("wfi");		  
}

/* �ر������ж� */
void INTX_DISABLE(void)
{		  
	__ASM volatile("cpsid i");
}

/* ���������ж� */
void INTX_ENABLE(void)
{
	__ASM volatile("cpsie i");		  
}

/* ����ջ����ַ */
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			
    BX r14
}
