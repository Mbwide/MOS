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
  * @file    mos_typedef.h
  * @version V1.0.0
  * @date    2021-08-20
  * @brief   ���������ض���
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */

#ifndef _MOS_TYPEDEF_H
#define _MOS_TYPEDEF_H


/*�������������ض���*/
typedef signed   char                   mos_int8_t;
typedef signed   short                  mos_int16_t;
typedef signed   int					mos_int32_t;
typedef unsigned char					mos_uint8_t;
typedef unsigned short					mos_uint16_t;
typedef unsigned int					mos_uint32_t;
typedef unsigned int					mos_bool_t;

typedef unsigned int                    mos_tick_t;
typedef unsigned int					mos_err_t;

typedef unsigned long					mos_ubase_t;
typedef signed   long					mos_base_t;


/* MOS ������ */
#define MOS_EOK        0    /* û�д���*/
#define MOS_ERROR      1    /* �д��� */
#define MOS_ETIMEOUT   2    /* ��ʱ */
#define MOS_EFULL      3    /* ��Դ�� */
#define MOS_EEMPTY     4    /* ��Դ�� */
#define MOS_ENOMEM     5    /* �ڴ治�� */
#define MOS_ENULL      6    /* �Ƿ���ַ */


/* MDK ������ */
#ifdef __CC_ARM
    #define MOS_INLINE						static __inline
    #define ALIGN(n)                    	__attribute__((aligned(n)))
#endif

/**
 *size���ϰ���align�ֽڶ��� MOS_ALIGN_UP(9, 4) return 12
 */
#define MOS_ALIGN_UP(data, align)      (((data) + (align) - 1) & ~((align) - 1))

/**
 *size���°���align�ֽڶ��� MOS_ALIGN_DOWN(7, 4) return 4
 */
#define MOS_ALIGN_DOWN(data, align)    ((data) & ~((align) - 1))


/*�����������Ͷ��� */
#define TRUE							(1)
#define FALSE							(0)

#define YES								(1)
#define NO								(0)

#define NULL							((void *)0)

#define MOS_MAX_DELAY					(0xffffffffUL)

#endif /* _MOS_TYPEDEF_H */
