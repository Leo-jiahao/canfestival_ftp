/**
 * @file stm32f405_demo1_tim.h
 * @author Leo-jiahao (leo884823525@gmail.com)
 * @brief 
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
#ifndef __STM32F405_DEMO1_TIM_H__
#define __STM32F405_DEMO1_TIM_H__
#include "stm32f4xx_hal.h"
#include <stdbool.h>

typedef void (*BSP_TIMx_CallBack)(void);

bool             BSP_TIM2_Init(uint32_t psc, uint32_t arr);
void             BSP_TIM2_SetCounter(uint16_t counter);
uint32_t         BSP_TIM2_GetCounter(void);
bool             BSP_TIM2_DeInit(void);
bool             BSP_TIM2_SetCallBack(BSP_TIMx_CallBack callback);
#endif
