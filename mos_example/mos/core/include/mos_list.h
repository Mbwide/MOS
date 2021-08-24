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
  * @file    mos_list.h
  * @version V1.0.0
  * @date    2021-08-20
  * @brief   双向链表，用于各种列表实现
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */
#ifndef _MOS_LIST_H
#define _MOS_LIST_H
#include "mos_typedef.h"


/* define --------------------------------------------------------------------*/
/**
 *获取成员member在结构体类型type的偏移量
 */
#define MOS_OFFSET_OF(type, member) \
    ((unsigned long)(&((type *)0)->member))

/**
 *根据链表类型member的实例地址ptr获取包含该成员的type型结构体实例的入口地址
 */
#define MOS_LIST_ENTRY(ptr, type, member) \
    ((type *)((char *)ptr - MOS_OFFSET_OF(type, member)))

/**
 *双向链表头初始化
 */
#define MOS_LIST_HEAD(list) \
    mos_list_t list = { &(list), &(list) }


/* Public Fun-----------------------------------------------------------------*/
/**
 *双向链表结构体
 */
typedef struct mos_list_node
{
    struct mos_list_node *next; /* 指向后一个节点 */
    struct mos_list_node *prev; /* 指向前一个节点 */
} mos_list_t;

/**
 * @brief 双向链表初始化
 */
MOS_INLINE void mos_list_init(mos_list_t *list)
{
    list->next = list;
    list->prev = list;
}

/**
 * @brief 双向链表头部插入一个结点
 */
MOS_INLINE void mos_list_head_insert(mos_list_t *list, mos_list_t *node)
{
    node->next = list->next;
    node->prev = list;

    list->next->prev = node;
    list->next = node;
}

/**
 * @brief 双向链表尾部插入一个结点
 */
MOS_INLINE void mos_list_tail_insert(mos_list_t *list, mos_list_t *node)
{
    node->next = list;
    node->prev = list->prev;

    list->prev->next = node;
    list->prev = node;
}

/**
 * @brief 双向链表删除一个结点
 */
MOS_INLINE void mos_list_node_delete(mos_list_t *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;

    node->prev = node->next = node;

}

/**
 * @brief 判断链表是否为空
 */
MOS_INLINE mos_bool_t mos_list_is_empty(mos_list_t *node)
{
    return (mos_bool_t)(node->next == node);
}

#endif /* _MOS_LIST_H */

