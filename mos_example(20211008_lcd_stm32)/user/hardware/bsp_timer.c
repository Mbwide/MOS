/**
  ******************************************************************************
  * @file    bsp_timer.c
  * @version 
  * @date    
  * @brief   定时器
  ******************************************************************************
  * @note
  ******************************************************************************
  */
  
#include "bsp_timer.h"

/* Public Fun-----------------------------------------------------------------*/
/**
 * @brief  TIM3初始化
 * @param  自动重装值
 * @param  时钟预分频数
 */
void TIM3_Int_Init(u16 arr, u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
    
	/* 定时器TIM3初始化 */
    TIM_TimeBaseStructure.TIM_Period = arr;                    //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
    TIM_TimeBaseStructure.TIM_Prescaler = psc;                 //设置用来作为TIMx时钟频率除数的预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;    //设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//TIM向上计数模式
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);            //根据指定的参数初始化TIMx的时间基数单位
    
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE );                //使能指定的TIM3中断,允许更新中断

	/* 中断优先级NVIC设置 */
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;            //TIM3中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;  //先占优先级4级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;         //从优先级0级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;            //IRQ通道被使能
    NVIC_Init(&NVIC_InitStructure);            

    TIM_Cmd(TIM3, ENABLE);  
}


/**
 * @brief  TIM5初始化
 * @param  自动重装值
 * @param  时钟预分频数
 */
void TIM5_Int_Init(u16 arr, u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE); //时钟使能

    /* 定时器TIM5初始化 */
    TIM_TimeBaseStructure.TIM_Period = arr; 					//设置在下一个更新事件装入活动的自动重装载寄存器周期的值
    TIM_TimeBaseStructure.TIM_Prescaler = psc; 					//设置用来作为TIMx时钟频率除数的预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 	//设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM向上计数模式
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure); 			//根据指定的参数初始化TIMx的时间基数单位
    
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
    TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE ); 			     //使能指定的TIM5中断,允许更新中断

	/* 中断优先级NVIC设置 */
    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;  			//TIM5中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;  	//先占优先级5级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  		//从优先级0级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 			//IRQ通道被使能
    NVIC_Init(&NVIC_InitStructure);  							//初始化NVIC寄存器

    TIM_Cmd(TIM5, ENABLE);  									//使能TIM5
}











