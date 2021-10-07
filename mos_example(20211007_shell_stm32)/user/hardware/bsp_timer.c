/**
  ******************************************************************************
  * @file    bsp_timer.c
  * @version 
  * @date    
  * @brief   ��ʱ��
  ******************************************************************************
  * @note
  ******************************************************************************
  */
  
#include "bsp_timer.h"

/* Public Fun-----------------------------------------------------------------*/
/**
 * @brief  TIM3��ʼ��
 * @param  �Զ���װֵ
 * @param  ʱ��Ԥ��Ƶ��
 */
void TIM3_Int_Init(u16 arr, u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��
    
	/* ��ʱ��TIM3��ʼ�� */
    TIM_TimeBaseStructure.TIM_Period = arr;                    //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
    TIM_TimeBaseStructure.TIM_Prescaler = psc;                 //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;    //����ʱ�ӷָ�:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//TIM���ϼ���ģʽ
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);            //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
    
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE );                //ʹ��ָ����TIM3�ж�,��������ж�

	/* �ж����ȼ�NVIC���� */
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;            //TIM3�ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;  //��ռ���ȼ�4��
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;         //�����ȼ�0��
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;            //IRQͨ����ʹ��
    NVIC_Init(&NVIC_InitStructure);            

    TIM_Cmd(TIM3, ENABLE);  
}


/**
 * @brief  TIM5��ʼ��
 * @param  �Զ���װֵ
 * @param  ʱ��Ԥ��Ƶ��
 */
void TIM5_Int_Init(u16 arr, u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE); //ʱ��ʹ��

    /* ��ʱ��TIM5��ʼ�� */
    TIM_TimeBaseStructure.TIM_Period = arr; 					//��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
    TIM_TimeBaseStructure.TIM_Prescaler = psc; 					//����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 	//����ʱ�ӷָ�:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM���ϼ���ģʽ
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure); 			//����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
    
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
    TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE ); 			     //ʹ��ָ����TIM5�ж�,��������ж�

	/* �ж����ȼ�NVIC���� */
    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;  			//TIM5�ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;  	//��ռ���ȼ�5��
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  		//�����ȼ�0��
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 			//IRQͨ����ʹ��
    NVIC_Init(&NVIC_InitStructure);  							//��ʼ��NVIC�Ĵ���

    TIM_Cmd(TIM5, ENABLE);  									//ʹ��TIM5
}











