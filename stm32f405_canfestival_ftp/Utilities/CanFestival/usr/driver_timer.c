/**
 * @file driver_timer.c
 * @author Leo-jiahao (leo884823525@gmail.com)
 * @brief 有实时系统时创建一个定时器，通过互斥信号量管理定时器触发后的执行任务。
 * 该执行任务是CanFestival的协议接口，所以必须包括协议的timer.h，并调用
 * @version 0.1
 * @date 2023-08-02
 * 
 * @copyright 
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) <2023>  <Leo-jiahao> 
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 */

#include <stdlib.h>
#include "stm32f405_demo1_tim.h"


#include "applicfg.h"
#include "timer.h"



static TIMEVAL last_counter_val = 0;
static TIMEVAL elapsed_time = 0;

void TimerCleanup(void)
{
	/* only used in realtime apps */
}
/**
 * @brief 获取互斥锁,对于裸机，需要关闭中断
 * 
 */
void EnterMutex(void)
{
	__set_PRIMASK(1);
}
/**
 * @brief 退还互斥锁，对于裸机，需要开启中断
 * 
 */
void LeaveMutex(void)
{
	__set_PRIMASK(0);
}
/**
 * @brief 基础定时器中断处理函数
 * 
 */
void timer_notify()
{
	EnterMutex();
	last_counter_val = 0;
	elapsed_time = 0;
	TimeDispatch();
	LeaveMutex();

}
/**
 * @brief 基础定时器初始化，配置中断处理函数为timer_notify
 * 
 */
void TimerInit(void)
{
	BSP_TIM2_Init(84,0xffff);//第一次 1ms触发
	BSP_TIM2_SetCallBack(timer_notify);
}

/**
 * @brief Set the Timer object 重新设置下一次定时器的触发值
 * 
 * @param value 
 */
void setTimer(TIMEVAL value)
{
	uint32_t timer = BSP_TIM2_GetCounter();
	elapsed_time += timer - last_counter_val;
	last_counter_val = 0xffff - value;
	BSP_TIM2_SetCounter(0xffff - value);
}
/**
 * @brief Get the Elapsed Time object
 * 获取经过的时间
 * @return TIMEVAL 
 */
TIMEVAL getElapsedTime(void)
{
  	uint32_t timer = BSP_TIM2_GetCounter();        // Copy the value of the running timer
	if(timer < last_counter_val)
		timer += 0xffff;
	TIMEVAL elapsed = timer - last_counter_val + elapsed_time;
	return elapsed;
}

