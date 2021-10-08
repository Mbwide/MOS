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
  * @date    2021-10-06
  * @brief   �ں�����
  ******************************************************************************
  * @note
  *          2021-08-01 Mbwide:��ʼ�汾
  *          2021-10-06 Mbwide:���printf��ӳ����汾��ӡ
  ******************************************************************************
  */

#include "mos_misc.h"
#include "mos_port.h"
#include "mos_user_config.h"

/* define --------------------------------------------------------------------*/
/* �������ֵ */
#define ZEROPAD     (1 << 0)    /* pad with zero */
#define SIGN        (1 << 1)    /* unsigned/signed long */
#define PLUS        (1 << 2)    /* show plus */
#define SPACE       (1 << 3)    /* space if plus */
#define LEFT        (1 << 4)    /* left justified */
#define SPECIAL     (1 << 5)    /* 0x */
#define LARGE       (1 << 6)    /* use 'ABCDEF' instead of 'abcdef' */
/* ʹ�þ��� */
#define MOS_PRINTF_PRECISION

/* Private Fun----------------------------------------------------------------*/
/* �ж��Ƿ�Ϊ���� */
#define is_digit(c)  ((unsigned)((c) - '0') < 10)

/**
 * @brief  �ַ���ת��Ϊ����
 * @param  �ַ���
 *
 * @return ת���������
 */
MOS_INLINE mos_int32_t mos_atoi(const char **s)
{
    register mos_int32_t i = 0;

    while (is_digit(**s))
    {
        i = i * 10 + *((*s)++) - '0';
    }

    return i;
}

/**
 * @brief  ȡ����
 * @param  �������ݵĵ�ַ
 * @param  ����(10��16)
 *
 * @return ת���������
 */
MOS_INLINE mos_int32_t divide(mos_int32_t *n, mos_int32_t base)
{
    mos_int32_t res;

    if (base == 10)
    {
        res = ((mos_uint32_t) * n) % 10U;
        *n = ((mos_uint32_t) * n) / 10U;
    }
    else
    {
        res = ((mos_uint32_t) * n) % 16U;
        *n = ((mos_uint32_t) * n) / 16U;
    }

    return res;
}

#ifdef MOS_PRINTF_PRECISION
static char *print_number(char *buf,
                          char *end,
                          int   num,
                          int   base,
                          int   s,
                          int   precision,
                          int   type)
#else
static char *print_number(char *buf,
                          char *end,
                          long  num,
                          int   base,
                          int   s,
                          int   type)
#endif
{
    char c, sign;
#ifdef RT_PRINTF_LONGLONG
    char tmp[32];
#else
    char tmp[16];
#endif
    const char *digits;
    static const char small_digits[] = "0123456789abcdef";
    static const char large_digits[] = "0123456789ABCDEF";
    register int i;
    register int size;

    size = s;

    digits = (type & LARGE) ? large_digits : small_digits;

    if (type & LEFT)
        type &= ~ZEROPAD;

    c = (type & ZEROPAD) ? '0' : ' ';

    /* get sign */
    sign = 0;

    if (type & SIGN)
    {
        if (num < 0)
        {
            sign = '-';
            num = -num;
        }
        else if (type & PLUS)
            sign = '+';
        else if (type & SPACE)
            sign = ' ';
    }

    i = 0;

    if (num == 0)
        tmp[i++] = '0';
    else
    {
        while (num != 0)
            tmp[i++] = digits[divide(&num, base)];
    }

#ifdef MOS_PRINTF_PRECISION

    if (i > precision)
        precision = i;

    size -= precision;
#else
    size -= i;
#endif

    if (!(type & (ZEROPAD | LEFT)))
    {
        if ((sign) && (size > 0))
            size--;

        while (size-- > 0)
        {
            if (buf <= end)
                *buf = ' ';

            ++ buf;
        }
    }

    if (sign)
    {
        if (buf <= end)
        {
            *buf = sign;
            -- size;
        }

        ++ buf;
    }

    /* no align to the left */
    if (!(type & LEFT))
    {
        while (size-- > 0)
        {
            if (buf <= end)
                *buf = c;

            ++ buf;
        }
    }

#ifdef MOS_PRINTF_PRECISION

    while (i < precision--)
    {
        if (buf <= end)
            *buf = '0';

        ++ buf;
    }

#endif

    /* put number in the temporary buffer */
    while (i-- > 0)
    {
        if (buf <= end)
            *buf = tmp[i];

        ++ buf;
    }

    while (size-- > 0)
    {
        if (buf <= end)
            *buf = ' ';

        ++ buf;
    }

    return buf;
}

mos_int32_t mos_vsnprintf(char       *buf,
                          mos_base_t   size,
                          const char *fmt,
                          va_list     args)
{

    mos_uint32_t num;
    int i, len;
    char *str, *end, c;
    const char *s;

    mos_uint8_t base;            /* the base of number */
    mos_uint8_t flags;           /* flags to print number */
    mos_uint8_t qualifier;       /* 'h', 'l', or 'L' for integer fields */
    mos_int32_t field_width;     /* width of output field */

#ifdef MOS_PRINTF_PRECISION
    int precision;               /* min. # of digits for integers and max for a string */
#endif

    str = buf;
    end = buf + size - 1;

    /* Make sure end is always >= buf */
    if (end < buf)
    {
        end  = ((char *) - 1);
        size = end - buf;
    }

    for (; *fmt ; ++fmt)
    {
        if (*fmt != '%')
        {
            if (str <= end)
                *str = *fmt;

            ++ str;
            continue;
        }

        /* process flags */
        flags = 0;

        while (1)
        {
            /* skips the first '%' also */
            ++ fmt;

            if (*fmt == '-') flags |= LEFT;
            else if (*fmt == '+') flags |= PLUS;
            else if (*fmt == ' ') flags |= SPACE;
            else if (*fmt == '#') flags |= SPECIAL;
            else if (*fmt == '0') flags |= ZEROPAD;
            else break;
        }

        /* get field width */
        field_width = -1;

        if (is_digit(*fmt)) field_width = mos_atoi(&fmt);
        else if (*fmt == '*')
        {
            ++ fmt;
            /* it's the next argument */
            field_width = va_arg(args, int);

            if (field_width < 0)
            {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

#ifdef MOS_PRINTF_PRECISION
        /* get the precision */
        precision = -1;

        if (*fmt == '.')
        {
            ++ fmt;

            if (is_digit(*fmt)) precision = mos_atoi(&fmt);
            else if (*fmt == '*')
            {
                ++ fmt;
                /* it's the next argument */
                precision = va_arg(args, int);
            }

            if (precision < 0) precision = 0;
        }

#endif
        /* get the conversion qualifier */
        qualifier = 0;

        if (*fmt == 'h' || *fmt == 'l')
        {
            qualifier = *fmt;
            ++ fmt;

        }

        /* the default base */
        base = 10;

        switch (*fmt)
        {
            case 'c':
                if (!(flags & LEFT))
                {
                    while (--field_width > 0)
                    {
                        if (str <= end) *str = ' ';

                        ++ str;
                    }
                }

                /* get character */
                c = (mos_uint8_t)va_arg(args, int);

                if (str <= end) *str = c;

                ++ str;

                /* put width */
                while (--field_width > 0)
                {
                    if (str <= end) *str = ' ';

                    ++ str;
                }

                continue;

            case 's':
                s = va_arg(args, char *);

                if (!s) s = "(NULL)";

                len = mos_strlen(s);
#ifdef MOS_PRINTF_PRECISION

                if (precision > 0 && len > precision) len = precision;

#endif

                if (!(flags & LEFT))
                {
                    while (len < field_width--)
                    {
                        if (str <= end) *str = ' ';

                        ++ str;
                    }
                }

                for (i = 0; i < len; ++i)
                {
                    if (str <= end) *str = *s;

                    ++ str;
                    ++ s;
                }

                while (len < field_width--)
                {
                    if (str <= end) *str = ' ';

                    ++ str;
                }

                continue;

            case 'p':
                if (field_width == -1)
                {
                    field_width = sizeof(void *) << 1;
                    flags |= ZEROPAD;
                }

#ifdef MOS_PRINTF_PRECISION
                str = print_number(str, end,
                                   (long)va_arg(args, void *),
                                   16, field_width, precision, flags);
#else
                str = print_number(str, end,
                                   (long)va_arg(args, void *),
                                   16, field_width, flags);
#endif
                continue;

            case '%':
                if (str <= end) *str = '%';

                ++ str;
                continue;

            case 'o':
                base = 8;
                break;

            case 'X':
                flags |= LARGE;

            case 'x':
                base = 16;
                break;

            case 'd':
            case 'i':
                flags |= SIGN;

            case 'u':
                break;

            default:
                if (str <= end) *str = '%';

                ++ str;

                if (*fmt)
                {
                    if (str <= end) *str = *fmt;

                    ++ str;
                }
                else
                {
                    -- fmt;
                }

                continue;
        }

        if (qualifier == 'l')
        {
            num = va_arg(args, mos_uint32_t);

            if (flags & SIGN) num = (mos_int32_t)num;
        }
        else if (qualifier == 'h')
        {
            num = (mos_uint16_t)va_arg(args, mos_int32_t);

            if (flags & SIGN) num = (mos_int16_t)num;
        }
        else
        {
            num = va_arg(args, mos_uint32_t);

            if (flags & SIGN) num = (mos_int32_t)num;
        }

#ifdef MOS_PRINTF_PRECISION
        str = print_number(str, end, num, base, field_width, precision, flags);
#else
        str = print_number(str, end, num, base, field_width, flags);
#endif
    }

    if (str <= end) *str = '\0';
    else *end = '\0';


    return str - buf;
}

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
 * @brief  �ж��ַ�����С
 * @param  �ַ���1
 * @param  �ַ���2
 *
 * @return str1��str2��С
 */
int mos_strcmp(const char* str1, const char* str2)
{
    int ret = 0;

    while(!(*(char*)str1 - * (char*)str2) && *str1)
    {
        str1++;
        str2++;
    }

    ret = *(char*)str1 - * (char*)str2;

    if (ret < 0)
    {
        return -1;
    }
    else if (ret > 0)
    {
        return 1;
    }

    return 0;
}

/**
 * @brief  �����ַ������״γ����Ӵ��ĵ�ַ
 * @param  �ַ�����ʼ��ַ
 * @param  Ŀ���Ӵ�
 *
 * @return �״γ���Ŀ���Ӵ�����ʼ��ַ����û�ҵ�������NULL
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

/**
 * @brief  �����ַ�������
 * @param  �ַ�����ʼ��ַ
 *
 * @return �ַ�������
 */
mos_base_t mos_strlen(const char *s)
{
    mos_base_t len = 0;

    while(*s++ != '\0')
    {
        len++;
    }

    return len;
}

#if (MOS_CONFIG_USE_SHELL == YES || MOS_CONFIG_USE_DEBUG_PRINTF == YES)
/**
 * @brief  �ں˴�ӡ�ӿ�
 * @param  Ҫ��ӡ������
 */
void mos_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    mos_base_t length;
    static char mos_log_buf[MOS_CONFIG_DEBUG_TXBUF_SIZE];

    length = mos_vsnprintf(mos_log_buf, sizeof(mos_log_buf) - 1, fmt, args);

    if (length > MOS_CONFIG_DEBUG_TXBUF_SIZE - 1)
    {
        length = MOS_CONFIG_DEBUG_TXBUF_SIZE - 1;
    }
	
	/* �����ٽ�� */
    mos_base_t temp =  mos_port_entry_critical_temp();
    /* ͨ����Ӧ�Ľӿ�������� */
    mos_port_output(mos_log_buf);
	/* �˳��ٽ�� */
    mos_port_exit_critical_temp(temp);
	
    va_end(args);
    return;
}

/**
 * @brief  �汾��ӡ
 */
void mos_show_version(void)
{
    mos_printf("\n");
    mos_printf("  ��__��\n");
    mos_printf(" (��ء�) ----Hello World��\n");
    mos_printf(" ���ģ�(�ߣߣ�\n");
    mos_printf("����-(��MOS_��\n");
    mos_printf(" ������������\n");
    mos_printf("---Micro OS--- An Open Source RTOS\n");
    mos_printf("Version:%d.%d.%d build %s\n",
               MOS_VERSION, MOS_SUBVERSION, MOS_REVISION, __DATE__);
    mos_printf("\n");
}
#endif

