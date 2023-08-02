/**
 * @file stm32f405_demo1_tim.c
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
#include "stm32f405_demo1_tim.h"

static uint8_t tim2_PreemptPrio = 5;
static uint8_t tim2_SubPrio = 0; 
static TIM_HandleTypeDef htim2;
static BSP_TIMx_CallBack tim2_callback = NULL;

/**
 * @brief 84Mhz定时器2初始化
 * 
 * @param psc 
 * @param arr 
 */
bool BSP_TIM2_Init(uint32_t psc, uint32_t arr)
{
    
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = psc-1;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = arr-1;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        return false;
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
    {
        return false;
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
    {
        return false;
    }
    if(HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
	{
		return false;
	}
    return true;
}
/**
 * @brief 
 * 
 * @return true 
 * @return false 
 */
bool BSP_TIM2_DeInit(void)
{
    tim2_callback = NULL;
    if(HAL_TIM_Base_DeInit(&htim2) != HAL_OK)
    {
        return false;
    }
    return true;
}
/**
 * @brief 
 * 
 * @param callback 
 */
bool BSP_TIM2_SetCallBack(BSP_TIMx_CallBack callback)
{
    tim2_callback = callback;
    return true;
}
/**
 * @brief 设置定时器2的当前计数值，定时器2 最大支持u32计数值，但保留使用 u16
 * 
 * @param counter 
 */
void BSP_TIM2_SetCounter(uint16_t counter)
{
  __HAL_TIM_SET_COUNTER(&htim2, counter);
}
/**
 * @brief 获取定时器2的当前计数值
 * 
 * @return uint32_t 
 */
uint32_t BSP_TIM2_GetCounter(void)
{
  return __HAL_TIM_GET_COUNTER(&htim2);
}
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM2)
  {

    /* TIM2 clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();

    /* TIM2 interrupt Init */
    HAL_NVIC_SetPriority(TIM2_IRQn, tim2_PreemptPrio, tim2_SubPrio);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
  
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM2)
  {

    /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();

    /* TIM2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM2_IRQn);

  }
}
/**
 * @brief 定时器2中断服务函数
 * 
 */
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}
/**
 * @brief 定时器更新中断回调函数
 * 
 * @param htim 
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	/**
     * @brief  Period elapsed callback in non blocking mode
     * @note   This function is called  when TIM1 interrupt took place, inside
     * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
     * a global variable "uwTick" used as application time base.
     * @param  htim : TIM handle
     * @retval None
     */
        
    if (htim->Instance == TIM1) {
        HAL_IncTick();
    }
        
    if(htim->Instance == TIM2)
    {
		if(tim2_callback)
		{
			tim2_callback();
		}
    }
}
