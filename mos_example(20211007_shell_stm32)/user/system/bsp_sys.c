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

/* 执行汇编指令WFI */
void WFI_SET(void)
{
	__ASM volatile("wfi");		  
}

/* 关闭所有中断 */
void INTX_DISABLE(void)
{		  
	__ASM volatile("cpsid i");
}

/* 开启所有中断 */
void INTX_ENABLE(void)
{
	__ASM volatile("cpsie i");		  
}

/* 设置栈顶地址 */
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			
    BX r14
}
