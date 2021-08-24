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
  * @file    mos_misc.c
  * @version V1.0.0
  * @date    2021-08-01
  * @brief   �ں�����
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */

#include "mos_misc.h"


/* Public Fun-----------------------------------------------------------------*/
/**
 * @brief  ��ĳһ���ڴ��е�����ȫ������Ϊָ����ֵ
 * @param  Ŀ����ʼ��ַ
 * @param  Ҫ���õ�ֵ
 * @param  Ŀ���ַ����
 *
 * @return Ŀ����ʼ��ַ
 */
void *mos_memset(void *dst, mos_int32_t c, mos_int32_t len)
{
    if (NULL == dst || len <= 0)
    {
        return NULL;
    }

    char * tmp = (char *)dst;

    while(len-- > 0)
    {
        *tmp++ = c;
    }

    return dst;
}

/**
 * @brief  ��Դ�ڴ��ַ����ʼλ�ÿ�ʼ�������ɸ��ֽڵ�Ŀ���ڴ��ַ��
 * @param  Ŀ����ʼ��ַ
 * @param  Դ��ַ
 * @param  Ҫ�������ֽڳ���
 *
 * @return Ŀ����ʼ��ַ
 */
void *mos_memcpy(void *dst, const void *src, mos_int32_t len)
{
    if (NULL == dst || NULL == src || len <= 0)
    {
        return NULL;
    }

    char *d = (char *)dst;
    const char *s = (char *)src;

    if (d <= s || d > (s + len))
    {
        while(len--)
        {
            *d++ = *s++;
        }
    }
    else
    {
        d = (char *)dst + len - 1;
        s = (char *)src + len - 1;

        while(len--)
        {
            *d-- = *s--;
        }
    }

    return dst;
}

/**
 * @brief  ��ָ�����ȵ��ַ������Ƶ��ַ�������
 * @param  �ַ���������ʼ��ַ
 * @param  Դ��ַ
 * @param  Ҫ���Ƶ��ֽڳ���
 *
 * @return �ַ���������ʼ��ַ
 * @note   ���ַ������鳤�ȴ���ָ�����ȣ������ָ�����ȵ�ʣ����������Ϊ0
 */
char *mos_strncpy(char *dst, const char *src, mos_int32_t len)
{
    if (NULL == dst || NULL == src || len <= 0)
    {
        return NULL;
    }

    char *d = (char *)dst;
    const char *s = (char *)src;

    while (len-- > 0)
    {
        if ((*d++ = *s++) == 0)
        {
            break;
        }
    }

    while (len-- > 0)
    {
        *d++ = 0;
    }

    return dst;
}

/**
 * @brief  �����ַ������״γ����Ӵ��ĵ�ַ
 * @param  �ַ�����ʼ��ַ
 * @param  Ŀ���Ӵ�
 *
 * @return �״γ���Ŀ���Ӵ�����ʼ��ַ����û�ҵ�������NULL
 */
/**
 * @brief �����ַ������״γ���Ŀ���Ӵ��ĵ�ַ
 */
char *mos_strstr(const char *str1, const char *str2)
{
    char *ch = (char *)str1;
    char *s1, *s2;

    if (!*str2)
    {
        return ((char *) str1);
    }

    while (*ch)
    {
        s1 = ch;
        s2 = (char *)str2;

        while (*s1 && *s2 && (*s1 == *s2))
        {
            s1++;
            s2++;
        }

        if (!*s2) return ch;

        ch++;
    }

    return NULL;
}
