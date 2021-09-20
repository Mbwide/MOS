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
  * @file    mos_heap.c
  * @version V1.0.0
  * @date    2021-09-10
  * @brief   动态内存管理
  ******************************************************************************
  * @note
  *
  ******************************************************************************
  */

#include "mos_user_config.h"
#include "mos_heap.h"
#include "mos_port.h"

/* 内存块最小值 */
#define BLOCK_MIN_SIZE ((mos_ubase_t )(sizeof(mos_heap_head_t) << 1))

/* 内存块数据头大小 */
#define HEAD_T_SIZE sizeof(mos_heap_head_t)

/* 内存块数据头 */
typedef struct mos_heap_head
{
	mos_ubase_t used;
	mos_ubase_t block_size;
	struct mos_heap_head * next;
} mos_heap_head_t;


/* Private -------------------------------------------------------------------*/
/* 内存池 */
static mos_uint8_t heap[MOS_CONFIG_HEAP_SIZE];
/* 内存块起始数据头（不属于内存池），heap_head_start.next指向第一个未使用的内存块数据头*/
static mos_heap_head_t heap_head_start;
/* 内存块结束数据尾指针（属于内存池）*/
static mos_heap_head_t *heap_head_end;
/* 内存总大小 */
static mos_ubase_t heap_total_size;
/* 剩余内存大小 */
static mos_ubase_t heap_free_size;

/* 放回内存池 */
static void block_insert_free_list(mos_heap_head_t *new_block);

/* Public Fun-----------------------------------------------------------------*/
/**
 * @brief  内存池初始化
 */
void mos_heap_init(void)
{
	/* 指向内存池第一个内存块数据头（属于内存池） */
	mos_heap_head_t * first_block;
	
	/* 用于计算内存开始和结束地址数值 */
	mos_ubase_t add_begin;
	mos_ubase_t add_end;
	
	/* 内存对齐 */
	add_begin = MOS_ALIGN_UP((mos_uint32_t)heap, MOS_CONFIG_ALIGN_SIZE);
	add_end   = MOS_ALIGN_DOWN((mos_uint32_t)(heap + MOS_CONFIG_HEAP_SIZE), MOS_CONFIG_ALIGN_SIZE);
	
	/* 内存总大小与剩余内存大小 */
	heap_total_size = add_end - add_begin - HEAD_T_SIZE;
	heap_free_size = heap_total_size;
	
	/* 初始化内存块起始数据头 */
	heap_head_start.used = 0;
	heap_head_start.block_size = 0;
	heap_head_start.next = (mos_heap_head_t *)add_begin;
	
	/* 初始化内存块结束数据头 */
	heap_head_end = (mos_heap_head_t *)(add_end - HEAD_T_SIZE);
	heap_head_end->used = 0;
	heap_head_end->block_size = 0;
	heap_head_end->next = NULL;
	
	/* 初始化第一个内存块数据头 */
	first_block = (mos_heap_head_t *)add_begin;
	first_block->used = 0;
	first_block->block_size = heap_free_size;
	first_block->next = heap_head_end;
}

/**
 * @brief  申请内存
 * @param  申请的内存大小，单位为byte
 *
 * @return 返回申请到的内存首地址，若申请失败，返回NULL
 */
void *mos_malloc(mos_ubase_t need_size)
{
	void *return_ptr = NULL;
	mos_heap_head_t *pre_block;
	mos_heap_head_t *cur_block;
	mos_heap_head_t *new_block;
	
	if (need_size == 0) return NULL;
	
	/* 申请大小需要字节对齐 */
	need_size = MOS_ALIGN_UP(need_size, MOS_CONFIG_ALIGN_SIZE);
	/* 申请大小超出剩余内存大小返回NULL */
	if (need_size > heap_free_size) return NULL;
	/* 申请大小+数据头大小 */
	need_size += HEAD_T_SIZE;
	/* 内存块最小值 */
	if (need_size < BLOCK_MIN_SIZE) need_size = BLOCK_MIN_SIZE;
	
	/* 进入临界区 */
	mos_port_entry_critical();
	
	pre_block = &heap_head_start;
	cur_block = heap_head_start.next;
	
	/* 寻找符合大小的内存块 */
	while((cur_block->block_size < need_size) && cur_block->next != NULL)
	{
		pre_block = cur_block;
		cur_block = cur_block->next;
	}
	
	/* 找到了 */
	if (cur_block != heap_head_end)
	{
		/* 返回地址要去掉内存块数据头 */
		return_ptr = (void *)((mos_uint8_t *)pre_block->next + HEAD_T_SIZE);
		/* 将要返回的内存块数据头从可用内存的内存块数据头链表删除*/
		pre_block->next = cur_block->next;
		
		/* 内存块太大，切掉多余的放回内存池 */
		if ((cur_block->block_size - need_size) > BLOCK_MIN_SIZE)
		{
			/* 获取要放回内存池的内存块数据头 */
			new_block = (mos_heap_head_t *)((mos_ubase_t)(cur_block) + need_size);
			new_block->used = 0;
			new_block->block_size = cur_block->block_size - need_size;
            
			/* 放回内存池 */
			block_insert_free_list(new_block);
			
			/* 修改要返回的内存块数据头 */
			cur_block->used = 1;
			cur_block->block_size = need_size;
		
		}
	}
	
	/* 修改剩余内存大小 */
	heap_free_size -= cur_block->block_size;
	
	/* 退出临界区 */
	mos_port_exit_critical();
	
	return return_ptr;
}

/**
 * @brief  内存施放
 * @param  要施放的内存首地址
 */
void mos_free(void *mem_add)
{
	mos_heap_head_t *cur_block;
	mos_uint8_t * p_block;
	
	p_block = (mos_uint8_t *)mem_add;
	
	if (mem_add != NULL) 
	{
		p_block -= HEAD_T_SIZE;
		cur_block = (mos_heap_head_t *)p_block;
		
		if (cur_block->used == 1) 
		{
			cur_block->used = 0;
			
			/* 进入临界区 */
	        mos_port_entry_critical();
			
			/* 修改剩余内存大小 */
			heap_free_size += cur_block->block_size;
			
			/* 放回内存池 */
			block_insert_free_list(cur_block);			
			
			/* 退出临界区 */
	        mos_port_exit_critical();
		}
	}
}


/* Private Fun----------------------------------------------------------------*/
/**
 * @brief  放回内存池
 * @param  要放回内存池的内存块数据头指针
 */
static void block_insert_free_list(mos_heap_head_t *new_block)
{
	mos_heap_head_t *cur_block;
	mos_uint8_t *p_block;
	
	cur_block = &heap_head_start;
	
	/* 根据地址寻找插入点 */
	while (cur_block->next < new_block)
	{
		cur_block = cur_block->next;
	}
	
	/* 向前合并 */
	p_block = (mos_uint8_t *)cur_block;
    if ((p_block + cur_block->block_size) == (mos_uint8_t *)new_block)
	{
		/* 新节点改为合并后的结点，并修改该节点大小 */
		cur_block->block_size += new_block->block_size;
		new_block = cur_block;
	}
	
	/* 向后合并 */
	p_block = (mos_uint8_t *)new_block;
	if((p_block + new_block->block_size ) == (mos_uint8_t *)cur_block->next)
	{
		/* 如果可以合并并且当前节点的下一个节点不是尾部 */
	    if(cur_block->next != heap_head_end)
		{
			new_block->block_size += cur_block->next->block_size;
			new_block->next = cur_block->next->next;
		}
		/* 下一个节点是尾部，新节点指向尾节点 */
		else
		{
			new_block->next = heap_head_end;
		}
	}
	else
	{
		/* 没有向后合并，新节点向后连接到当前节点的下一节点*/
		new_block->next = cur_block->next;
	}
	
	/* 没有向前合并，新节点向前连接到当前节点 */
	if(cur_block != new_block)
	{
		cur_block->next = new_block;
	}
}


