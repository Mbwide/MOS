# MOS

### 1 简介

**介绍：**一款开源的实时操作系统MicroOS，支持 ARM 系列 M3 内核架构和 M4 内核架构（暂不支持FPU），基础框架已完成**任务调度、中断管理以及内存管理**，已经在STM32F103芯片上移植并且顺利运行

**特性：**目前支持静态任务创建，动态任务创建，多优先级，支持时间片轮转调度，持续开发中

### 2 源码目录

1. **mos_user_config.h  用户配置文件**
2. **core 内核文件**
   - include：头文件
     - mos_init.h
     - mos_list.h 链表实现
     - mos_misc.h
     - mos_sys.h
     - mos_task.h
     - mos_tick.h
     - mos_heap.h
   - mos_init.c       内核初始化
   - mos_misc.c    内核杂项（字符处理等）
   - mos_sys.c       系统支持(临界区等)
   - mos_task.c     任务与调度（任务创建，延时，调度器开启等）
   - mos_tick.c      定时器
   - mos_heap.c   动态内存管理
3. **ports  硬件接口文件**
   - mos_hw.c 	系统硬件支持（栈初始化等）
   - mos_hw.h 
   - mos_port_asm.s 系统软件支持，汇编实现（PendSV中断服务函数，SVC中断服务函数）
   - mos_port.c  系统软件支持（滴答定时间配置，滴答定时器中断等）
   - mos_port.h
   - mos_typedef.c  数据类型重定义

### 3 设计框架

> 设计框架按层分类，由三层组成，**用户层，内核层以及硬件支持层**

##### 3.1 用户层

1. config.h，可裁剪，配置相关需求

##### 3.2 内核层

1. 任务管理
   - 任务创建
   - 任务延时
   - 任务调度
   - 任务挂起与恢复
2. 中断管理
   - 开关中断
   - 中断嵌套管理
3. 内存管理
   - 动态内存管理
4. 任务间通信

##### 3.3 硬件支持层

1. 硬件支持的汇编实现（临界区，任务切换的上下文保存部分）
2. 硬件支持的接口初始化（任务栈，定时器）
3. 异常服务函数

### 4 移植过程（STM32F103标准库）

#### 4.1 静态方法

##### 4.1.1 修改接口文件

1. 屏蔽标准库中断服务函数
   - SVC_Handler
   - PendSV_Handler
   - SysTick_Handler
2. 配置mos_user_config.h 
   - 设置CPU频率
   - 设置MOS时钟周期
   - **#define MOS_CONFIG_USE_DYNAMIC_HEAP		    (NO)**
3. 引入头文件 mos_init.h mos_task.h mos_port.h

##### 4.1.2任务创建

1. 设置系统中断优先级分组
2. 硬件初始化
3. 进入临界区
4. 内核初始化
5. 任务创建
   - 任务控制块指针
   - 任务入口函数
   - 任务优先级
   - 任务堆栈大小，单位为字
   - 任务栈起始地址
6. 退出临界区
7. 任务调度开启

#### 4.2 动态方法

##### 4.2.1 修改接口文件

1. 屏蔽标准库中断服务函数
   - SVC_Handler
   - PendSV_Handler
   - SysTick_Handler
2. 配置mos_user_config.h 
   - 设置CPU频率
   - 设置MOS时钟周期
   - **#define MOS_CONFIG_USE_DYNAMIC_HEAP		    (YES)**
3. 引入头文件 mos_init.h mos_task.h mos_port.h

##### 4.2.2任务创建

1. 设置系统中断优先级分组
2. 硬件初始化
3. 进入临界区
4. 内核初始化
5. 任务创建：需要以下参数
   - 任务控制块指针
   - 任务入口函数
   - 任务优先级
   - 任务堆栈大小，单位为字
6. 退出临界区
7. 任务调度开启



