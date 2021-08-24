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
  * @brief   内核杂项
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */

#include "mos_misc.h"


/* Public Fun-----------------------------------------------------------------*/
/**
 * @brief  将某一块内存中的内容全部设置为指定的值
 * @param  目标起始地址
 * @param  要设置的值
 * @param  目标地址长度
 *
 * @return 目标起始地址
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
 * @brief  从源内存地址的起始位置开始拷贝若干个字节到目标内存地址中
 * @param  目标起始地址
 * @param  源地址
 * @param  要拷贝的字节长度
 *
 * @return 目标起始地址
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
 * @brief  将指定长度的字符串复制到字符数组中
 * @param  字符串数组起始地址
 * @param  源地址
 * @param  要复制的字节长度
 *
 * @return 字符串数组起始地址
 * @note   若字符串数组长度大于指定长度，则大于指定长度的剩余数据设置为0
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
 * @brief  返回字符串中首次出现子串的地址
 * @param  字符串起始地址
 * @param  目标子串
 *
 * @return 首次出现目标子串的起始地址，若没找到，返回NULL
 */
/**
 * @brief 返回字符串中首次出现目标子串的地址
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
