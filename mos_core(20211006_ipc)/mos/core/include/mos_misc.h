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
  * @file    mos_misc.h
  * @version V1.0.0
  * @date    2021-08-01
  * @brief   �ں�����
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */
#ifndef _MOS_MISC_H
#define _MOS_MISC_H

#include "mos_typedef.h"

/* Public Fun-----------------------------------------------------------------*/
/* ��ĳһ���ڴ��е�����ȫ������Ϊָ��ֵ */
void *mos_memset(void *str, mos_int32_t c, mos_int32_t len);

/* ��Դ�ڴ��ַ����ʼλ�ÿ�ʼ�������ɸ��ֽڵ�Ŀ���ڴ��ַ�� */
void *mos_memcpy(void *dst, const void *src, mos_int32_t len);

/* ��ָ�����ȵ��ַ������Ƶ��ַ������� */
char *mos_strncpy(char *dst, const char *src, mos_int32_t len);

/* �����ַ������״γ����Ӵ��ĵ�ַ */
char *mos_strstr(const char *str1, const char *str2);

#endif
